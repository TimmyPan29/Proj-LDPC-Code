/*
 * decoder.h -- Iterative belief-propagation decoder.
 *
 * Two check-node rules share the same message-passing skeleton:
 *   SPA (sum-product) -- exact box-plus with the log correction term;
 *   MSA (min-sum)     -- the sign x min-magnitude approximation.
 *
 * The check-node update uses a forward/backward sweep so producing all
 * "exclude-one" extrinsic messages for a degree-d node costs O(d) rather
 * than O(d^2). Decoding stops as soon as the hard decision satisfies every
 * parity check (a valid codeword), or after `max_iter` iterations.
 */
#ifndef DECODER_H
#define DECODER_H

#include "ldpc.h"

typedef struct {
    double **L_v2c;   /* [n][col_weight]          variable -> check messages */
    double **L_c2v;   /* [num_checks][row_weight] check -> variable messages */
    int     *hard;    /* [n]                      hard decisions             */
    double  *in;      /* [row_weight]   scratch: a check's incoming messages */
    double  *fwd;     /* [row_weight+1] scratch: forward box-plus prefixes   */
    double  *bwd;     /* [row_weight+1] scratch: backward box-plus suffixes  */
    int      n_rows_v2c;  /* row counts retained for the free routine        */
    int      n_rows_c2v;
} decoder_ws;

int  decoder_ws_alloc(decoder_ws *ws, const ldpc_code *code);
void decoder_ws_free(decoder_ws *ws);

/* Run BP for the given algorithm. Fills ws->hard and returns iterations used. */
int  ldpc_decode(const ldpc_code *code, const double *llr_ch,
                 int algorithm, int max_iter, decoder_ws *ws);

#endif /* DECODER_H */
