/*
 * ldpc.c -- Config parsing and construction of the LDPC code from the
 * sparse H description and the systematic generator matrix G.
 */
#include "ldpc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- small allocation helpers ---------- */

static int **alloc_2d_int(int rows, int cols)
{
    int **m = malloc((size_t)rows * sizeof(int *));
    if (!m) return NULL;
    for (int i = 0; i < rows; i++) {
        m[i] = calloc((size_t)cols, sizeof(int));
        if (!m[i]) return NULL;
    }
    return m;
}

static void free_2d_int(int **m, int rows)
{
    if (!m) return;
    for (int i = 0; i < rows; i++) free(m[i]);
    free(m);
}

/* ---------- config file ---------- */

static char *trim(char *s)
{
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

int config_load(const char *path, sim_config *cfg)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "config: cannot open '%s'\n", path);
        return -1;
    }

    /* defaults */
    memset(cfg, 0, sizeof(*cfg));
    cfg->algorithm = ALGO_SPA;
    strcpy(cfg->output_dir, "results");

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        char *hash = strchr(p, '#');     /* strip comments */
        if (hash) *hash = '\0';
        char *eq = strchr(p, '=');
        if (!eq) continue;               /* not a key=value line */
        *eq = '\0';
        char *key = trim(p);
        char *val = trim(eq + 1);
        if (*key == '\0') continue;

        if      (!strcmp(key, "code_n"))        cfg->n = atoi(val);
        else if (!strcmp(key, "code_m"))        cfg->m = atoi(val);
        else if (!strcmp(key, "row_weight"))    cfg->row_weight = atoi(val);
        else if (!strcmp(key, "col_weight"))    cfg->col_weight = atoi(val);
        else if (!strcmp(key, "h_file"))        strncpy(cfg->h_file, val, sizeof(cfg->h_file) - 1);
        else if (!strcmp(key, "g_file"))        strncpy(cfg->g_file, val, sizeof(cfg->g_file) - 1);
        else if (!strcmp(key, "max_iter"))      cfg->max_iter = atoi(val);
        else if (!strcmp(key, "target_errors")) cfg->target_errors = atoi(val);
        else if (!strcmp(key, "snr_start"))     cfg->snr_start = atof(val);
        else if (!strcmp(key, "snr_step"))      cfg->snr_step = atof(val);
        else if (!strcmp(key, "snr_points"))    cfg->snr_points = atoi(val);
        else if (!strcmp(key, "seed"))          cfg->seed = atol(val);
        else if (!strcmp(key, "output_dir"))    strncpy(cfg->output_dir, val, sizeof(cfg->output_dir) - 1);
        else if (!strcmp(key, "algorithm")) {
            if      (!strcasecmp(val, "SPA")) cfg->algorithm = ALGO_SPA;
            else if (!strcasecmp(val, "MSA")) cfg->algorithm = ALGO_MSA;
            else { fprintf(stderr, "config: unknown algorithm '%s'\n", val); fclose(f); return -1; }
        }
    }
    fclose(f);

    if (cfg->n <= 0 || cfg->m <= 0 || cfg->row_weight <= 0 || cfg->col_weight <= 0 ||
        cfg->h_file[0] == '\0' || cfg->g_file[0] == '\0') {
        fprintf(stderr, "config: missing or invalid required field\n");
        return -1;
    }
    if (cfg->max_iter <= 0)      cfg->max_iter = 70;
    if (cfg->target_errors <= 0) cfg->target_errors = 100;
    if (cfg->snr_points <= 0)    cfg->snr_points = 1;
    return 0;
}

/* ---------- code construction ---------- */

/* Locate the slot of `target` inside neighbour list `list` of length `len`. */
static int find_slot(const int *list, int len, int target)
{
    for (int i = 0; i < len; i++)
        if (list[i] == target) return i;
    return -1;
}

int ldpc_load(ldpc_code *code, const sim_config *cfg)
{
    memset(code, 0, sizeof(*code));
    code->n           = cfg->n;
    code->m           = cfg->m;
    code->num_checks  = cfg->n;          /* this code uses a square (redundant) H */
    code->row_weight  = cfg->row_weight;
    code->col_weight  = cfg->col_weight;

    code->check_to_var    = alloc_2d_int(code->num_checks, code->row_weight);
    code->var_to_check    = alloc_2d_int(code->n,          code->col_weight);
    code->var_edge_slot   = alloc_2d_int(code->n,          code->col_weight);
    code->check_edge_slot = alloc_2d_int(code->num_checks, code->row_weight);
    if (!code->check_to_var || !code->var_to_check ||
        !code->var_edge_slot || !code->check_edge_slot) {
        fprintf(stderr, "ldpc: out of memory building Tanner graph\n");
        ldpc_free(code);
        return -1;
    }

    /* --- parity-check matrix H (sparse adjacency, 1-based in the file) --- */
    FILE *fh = fopen(cfg->h_file, "r");
    if (!fh) { fprintf(stderr, "ldpc: cannot open H file '%s'\n", cfg->h_file); ldpc_free(code); return -1; }

    for (int c = 0; c < code->num_checks; c++)
        for (int p = 0; p < code->row_weight; p++) {
            int v;
            if (fscanf(fh, "%d", &v) != 1) {
                fprintf(stderr, "ldpc: H file truncated (check section)\n"); fclose(fh); ldpc_free(code); return -1;
            }
            code->check_to_var[c][p] = v - 1;
        }
    for (int v = 0; v < code->n; v++)
        for (int k = 0; k < code->col_weight; k++) {
            int c;
            if (fscanf(fh, "%d", &c) != 1) {
                fprintf(stderr, "ldpc: H file truncated (variable section)\n"); fclose(fh); ldpc_free(code); return -1;
            }
            code->var_to_check[v][k] = c - 1;
        }
    fclose(fh);

    /* --- precompute edge-slot maps for O(1) message exchange --- */
    for (int v = 0; v < code->n; v++)
        for (int k = 0; k < code->col_weight; k++) {
            int c = code->var_to_check[v][k];
            int p = find_slot(code->check_to_var[c], code->row_weight, v);
            if (p < 0) { fprintf(stderr, "ldpc: inconsistent H (v=%d not on c=%d)\n", v, c); ldpc_free(code); return -1; }
            code->var_edge_slot[v][k] = p;
        }
    for (int c = 0; c < code->num_checks; c++)
        for (int p = 0; p < code->row_weight; p++) {
            int v = code->check_to_var[c][p];
            int k = find_slot(code->var_to_check[v], code->col_weight, c);
            if (k < 0) { fprintf(stderr, "ldpc: inconsistent H (c=%d not on v=%d)\n", c, v); ldpc_free(code); return -1; }
            code->check_edge_slot[c][p] = k;
        }

    /* --- systematic generator matrix G ([m][n], dense 0/1 characters) --- */
    code->g = malloc((size_t)code->m * sizeof(uint8_t *));
    if (!code->g) { fprintf(stderr, "ldpc: out of memory (G)\n"); ldpc_free(code); return -1; }
    for (int i = 0; i < code->m; i++) code->g[i] = NULL;

    FILE *fg = fopen(cfg->g_file, "r");
    if (!fg) { fprintf(stderr, "ldpc: cannot open G file '%s'\n", cfg->g_file); ldpc_free(code); return -1; }
    for (int i = 0; i < code->m; i++) {
        code->g[i] = calloc((size_t)code->n, sizeof(uint8_t));
        if (!code->g[i]) { fprintf(stderr, "ldpc: out of memory (G row)\n"); fclose(fg); ldpc_free(code); return -1; }
        int j = 0, ch;
        while (j < code->n && (ch = fgetc(fg)) != EOF) {
            if (ch == '0' || ch == '1') code->g[i][j++] = (uint8_t)(ch - '0');
        }
        if (j != code->n) { fprintf(stderr, "ldpc: G file truncated at row %d\n", i); fclose(fg); ldpc_free(code); return -1; }
    }
    fclose(fg);
    return 0;
}

void ldpc_free(ldpc_code *code)
{
    if (!code) return;
    free_2d_int(code->check_to_var,    code->num_checks);
    free_2d_int(code->var_to_check,    code->n);
    free_2d_int(code->var_edge_slot,   code->n);
    free_2d_int(code->check_edge_slot, code->num_checks);
    if (code->g) {
        for (int i = 0; i < code->m; i++) free(code->g[i]);
        free(code->g);
    }
    memset(code, 0, sizeof(*code));
}
