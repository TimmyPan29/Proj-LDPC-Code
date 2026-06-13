/*
 * decoder.c -- Sum-product / min-sum belief propagation.
 */
#include "decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* A magnitude large enough to act as the box-plus identity element. */
#define BOX_INF 1.0e30

/*
 * Box-plus of two LLRs.
 *   a [+] b = sign(a)sign(b) * min(|a|,|b|)  + [ log(1+e^-|a+b|) - log(1+e^-|a-b|) ]
 * The bracketed correction term is included for SPA and dropped for min-sum.
 */
static inline double box_plus(double a, double b, int use_correction)
{
    double mag = fabs(a) < fabs(b) ? fabs(a) : fabs(b);
    double r   = ((a < 0.0) ^ (b < 0.0)) ? -mag : mag;
    if (use_correction)
        r += log1p(exp(-fabs(a + b))) - log1p(exp(-fabs(a - b)));
    return r;
}

static double **alloc_2d_double(int rows, int cols)
{
    double **m = malloc((size_t)rows * sizeof(double *));
    if (!m) return NULL;
    for (int i = 0; i < rows; i++) {
        m[i] = malloc((size_t)cols * sizeof(double));
        if (!m[i]) return NULL;
    }
    return m;
}

int decoder_ws_alloc(decoder_ws *ws, const ldpc_code *code)
{
    memset(ws, 0, sizeof(*ws));
    /* record dimensions first so decoder_ws_free is safe on a partial failure */
    ws->n_rows_v2c = code->n;
    ws->n_rows_c2v = code->num_checks;

    ws->L_v2c = alloc_2d_double(code->n,          code->col_weight);
    ws->L_c2v = alloc_2d_double(code->num_checks, code->row_weight);
    ws->hard  = malloc((size_t)code->n * sizeof(int));
    ws->in    = malloc((size_t)code->row_weight       * sizeof(double));
    ws->fwd   = malloc((size_t)(code->row_weight + 1) * sizeof(double));
    ws->bwd   = malloc((size_t)(code->row_weight + 1) * sizeof(double));

    if (!ws->L_v2c || !ws->L_c2v || !ws->hard || !ws->in || !ws->fwd || !ws->bwd) {
        decoder_ws_free(ws);
        return -1;
    }
    return 0;
}

void decoder_ws_free(decoder_ws *ws)
{
    if (ws->L_v2c) { for (int i = 0; i < ws->n_rows_v2c; i++) free(ws->L_v2c[i]); free(ws->L_v2c); }
    if (ws->L_c2v) { for (int i = 0; i < ws->n_rows_c2v; i++) free(ws->L_c2v[i]); free(ws->L_c2v); }
    free(ws->hard);
    free(ws->in);
    free(ws->fwd);
    free(ws->bwd);
    ws->L_v2c = ws->L_c2v = NULL;
    ws->hard = NULL;
    ws->in = ws->fwd = ws->bwd = NULL;
}

int ldpc_decode(const ldpc_code *code, const double *llr_ch,
                int algorithm, int max_iter, decoder_ws *ws)
{
    const int use_corr = (algorithm == ALGO_SPA);
    const int rw = code->row_weight;
    const int cw = code->col_weight;

    /* initialise every variable->check message with the channel LLR */
    for (int v = 0; v < code->n; v++)
        for (int k = 0; k < cw; k++)
            ws->L_v2c[v][k] = llr_ch[v];

    for (int iter = 1; iter <= max_iter; iter++) {

        /* ---- check-node update (horizontal step) ---- */
        for (int c = 0; c < code->num_checks; c++) {
            const int *vars = code->check_to_var[c];
            const int *slot = code->check_edge_slot[c];

            for (int p = 0; p < rw; p++)
                ws->in[p] = ws->L_v2c[vars[p]][slot[p]];

            ws->fwd[0] = BOX_INF;
            for (int p = 0; p < rw; p++)
                ws->fwd[p + 1] = box_plus(ws->fwd[p], ws->in[p], use_corr);
            ws->bwd[rw] = BOX_INF;
            for (int p = rw - 1; p >= 0; p--)
                ws->bwd[p] = box_plus(ws->bwd[p + 1], ws->in[p], use_corr);

            for (int p = 0; p < rw; p++)   /* extrinsic: exclude edge p */
                ws->L_c2v[c][p] = box_plus(ws->fwd[p], ws->bwd[p + 1], use_corr);
        }

        /* ---- variable-node update (vertical step) + hard decision ---- */
        for (int v = 0; v < code->n; v++) {
            const int *checks = code->var_to_check[v];
            const int *slot   = code->var_edge_slot[v];

            double total = llr_ch[v];
            for (int k = 0; k < cw; k++)
                total += ws->L_c2v[checks[k]][slot[k]];
            for (int k = 0; k < cw; k++)   /* extrinsic: exclude edge k */
                ws->L_v2c[v][k] = total - ws->L_c2v[checks[k]][slot[k]];

            ws->hard[v] = (total > 0.0) ? 0 : 1;
        }

        /* ---- syndrome check (early termination) ---- */
        int valid = 1;
        for (int c = 0; c < code->num_checks && valid; c++) {
            int parity = 0;
            const int *vars = code->check_to_var[c];
            for (int p = 0; p < rw; p++)
                parity ^= ws->hard[vars[p]];
            if (parity) valid = 0;
        }
        if (valid) return iter;
    }
    return max_iter;
}
