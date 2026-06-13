/*
 * selftest.c -- Correctness checks for the LDPC simulator.
 *
 * These exercise the code invariants rather than just "does it run":
 *   A. every encoded word satisfies H * c = 0  (G/H consistency + encoder)
 *   B. confident clean LLRs decode to the codeword in one iteration
 *   C. a handful of weak-wrong LLRs are corrected back to the codeword
 *   D. the full encode -> AWGN -> decode pipeline is error-free at high Eb/N0
 *
 * Run from the repo root (paths come from config/sim.txt):  make test
 */
#include "ldpc.h"
#include "encoder.h"
#include "channel.h"
#include "decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int failures = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  [ ok ] %s\n", msg); }                \
    else      { printf("  [FAIL] %s\n", msg); failures++; }    \
} while (0)

/* number of unsatisfied parity checks for a hard codeword */
static int syndrome_weight(const ldpc_code *code, const int *word)
{
    int bad = 0;
    for (int c = 0; c < code->num_checks; c++) {
        int parity = 0;
        for (int p = 0; p < code->row_weight; p++)
            parity ^= word[code->check_to_var[c][p]];
        if (parity) bad++;
    }
    return bad;
}

int main(void)
{
    sim_config cfg;
    if (config_load("config/sim.txt", &cfg) != 0) return 2;
    ldpc_code code;
    if (ldpc_load(&code, &cfg) != 0) return 2;

    decoder_ws ws;
    if (decoder_ws_alloc(&ws, &code) != 0) { ldpc_free(&code); return 2; }

    int    *msg = malloc((size_t)code.m * sizeof(int));
    int    *cw  = malloc((size_t)code.n * sizeof(int));
    double *llr = malloc((size_t)code.n * sizeof(double));

    prbs_source prbs; prbs_init(&prbs);

    printf("LDPC self-test on the (%d, %d) code\n", code.n, code.m);

    /* --- A. encoder produces valid codewords (zero syndrome) --- */
    int a_ok = 1;
    for (int b = 0; b < 20; b++) {
        prbs_generate(&prbs, msg, code.m);
        ldpc_encode(&code, msg, cw);
        if (syndrome_weight(&code, cw) != 0) a_ok = 0;
    }
    CHECK(a_ok, "A. encoded words satisfy all parity checks (H*c = 0)");

    /* use the last encoded word for the LLR-level tests */
    const double L = 4.0;

    /* --- B. confident clean LLRs decode to the codeword in 1 iteration --- */
    for (int i = 0; i < code.n; i++) llr[i] = cw[i] ? -L : L;
    int iters = ldpc_decode(&code, llr, ALGO_SPA, cfg.max_iter, &ws);
    int b_match = 1;
    for (int i = 0; i < code.n; i++) if (ws.hard[i] != cw[i]) b_match = 0;
    CHECK(b_match && iters == 1, "B. clean LLRs decode to the codeword in 1 iteration");

    /* --- C. weak-wrong LLRs on spread-out bits are corrected --- */
    for (int i = 0; i < code.n; i++) llr[i] = cw[i] ? -L : L;
    const int K = 20;                       /* corrupted bits (< d_min = 33) */
    for (int t = 0; t < K; t++) {
        int idx = t * (code.n / K);
        llr[idx] = cw[idx] ? 0.8 : -0.8;     /* weak LLR pointing the wrong way */
    }
    ldpc_decode(&code, llr, ALGO_SPA, cfg.max_iter, &ws);
    int c_match = 1;
    for (int i = 0; i < code.n; i++) if (ws.hard[i] != cw[i]) c_match = 0;
    CHECK(c_match, "C. SPA corrects 20 weak-wrong LLRs back to the codeword");

    /* --- D. full pipeline is error-free at high Eb/N0 --- */
    rng_state rng; rng_init(&rng, cfg.seed);
    const double rate = (double)code.m / (double)code.n;
    long bit_errors = 0;
    const int BLOCKS = 200;
    for (int b = 0; b < BLOCKS; b++) {
        prbs_generate(&prbs, msg, code.m);
        ldpc_encode(&code, msg, cw);
        channel_transmit(cw, code.n, rate, 7.0, &rng, llr);  /* 7 dB: ~error-free */
        ldpc_decode(&code, llr, cfg.algorithm, cfg.max_iter, &ws);
        for (int i = 0; i < code.m; i++) if (ws.hard[i] != msg[i]) bit_errors++;
    }
    printf("       (D) %d blocks at 7 dB -> %ld info-bit errors\n", BLOCKS, bit_errors);
    CHECK(bit_errors == 0, "D. encode -> AWGN -> decode is error-free at 7 dB");

    free(msg); free(cw); free(llr);
    decoder_ws_free(&ws);
    ldpc_free(&code);

    printf("\n%s (%d failure%s)\n", failures ? "SELF-TEST FAILED" : "ALL CHECKS PASSED",
           failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
