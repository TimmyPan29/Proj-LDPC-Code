/*
 * ldpc.h -- Core data structures for the (1023, 781) LDPC code.
 *
 * The parity-check matrix H is stored in sparse adjacency form (the dense
 * 1023x1023 matrix would be wasteful). Two views of the Tanner graph are kept:
 *
 *   check_to_var[c][p]  -- the p-th variable node attached to check node c
 *   var_to_check[v][k]  -- the k-th check node attached to variable node v
 *
 * To pass messages along an edge in O(1) we also precompute, for every edge,
 * the slot index it occupies in the *other* node's neighbour list:
 *
 *   var_edge_slot[v][k]   -- slot of v inside check_to_var[var_to_check[v][k]]
 *   check_edge_slot[c][p] -- slot of c inside var_to_check[check_to_var[c][p]]
 *
 * All node indices are 0-based internally (the text files are 1-based).
 */
#ifndef LDPC_H
#define LDPC_H

#include <stdint.h>

/* Decoding algorithms. */
enum { ALGO_MSA = 0, ALGO_SPA = 1 };

typedef struct {
    int n;            /* codeword length (number of variable nodes)      */
    int m;            /* message length (number of information bits)      */
    int num_checks;   /* number of check nodes (== n for this code)       */
    int row_weight;   /* check-node degree  (1's per row of H)            */
    int col_weight;   /* variable-node degree (1's per column of H)       */

    int **check_to_var;    /* [num_checks][row_weight]                    */
    int **var_to_check;    /* [n][col_weight]                             */
    int **var_edge_slot;   /* [n][col_weight]                             */
    int **check_edge_slot; /* [num_checks][row_weight]                    */

    uint8_t **g;           /* systematic generator matrix [m][n]          */
} ldpc_code;

typedef struct {
    int    n, m, row_weight, col_weight;
    char   h_file[256];
    char   g_file[256];
    int    algorithm;       /* ALGO_SPA or ALGO_MSA                       */
    int    max_iter;
    int    target_errors;   /* Monte-Carlo stop: blocks per SNR point     */
    double snr_start;       /* Eb/N0 in dB                                */
    double snr_step;
    int    snr_points;
    long   seed;
    char   output_dir[256];
} sim_config;

/* Parse a `key = value` config file. Returns 0 on success, -1 on error. */
int config_load(const char *path, sim_config *cfg);

/* Build the LDPC code (loads H and G). Returns 0 on success, -1 on error. */
int ldpc_load(ldpc_code *code, const sim_config *cfg);

/* Release everything allocated by ldpc_load. */
void ldpc_free(ldpc_code *code);

#endif /* LDPC_H */
