/*
 * main.c -- Monte-Carlo BER/BLER simulation of the (1023, 781) LDPC code.
 *
 * For each Eb/N0 point the simulator transmits encoded PRBS blocks over an
 * AWGN channel and decodes them, counting block and bit errors until
 * `target_errors` block errors have accumulated. Results are printed to the
 * console and appended to a CSV file under the configured output directory.
 *
 *   usage: ldpc_sim [config_file]      (default: config/sim.txt)
 */
#include "ldpc.h"
#include "encoder.h"
#include "channel.h"
#include "decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *algo_name(int a) { return a == ALGO_SPA ? "SPA" : "MSA"; }

int main(int argc, char **argv)
{
    const char *config_path = (argc > 1) ? argv[1] : "config/sim.txt";

    sim_config cfg;
    if (config_load(config_path, &cfg) != 0) return EXIT_FAILURE;

    ldpc_code code;
    if (ldpc_load(&code, &cfg) != 0) return EXIT_FAILURE;

    decoder_ws ws;
    if (decoder_ws_alloc(&ws, &code) != 0) {
        fprintf(stderr, "main: cannot allocate decoder workspace\n");
        ldpc_free(&code);
        return EXIT_FAILURE;
    }

    int    *msg      = malloc((size_t)code.m * sizeof(int));
    int    *codeword = malloc((size_t)code.n * sizeof(int));
    double *llr      = malloc((size_t)code.n * sizeof(double));
    if (!msg || !codeword || !llr) {
        fprintf(stderr, "main: out of memory\n");
        free(msg); free(codeword); free(llr);
        decoder_ws_free(&ws); ldpc_free(&code);
        return EXIT_FAILURE;
    }

    const double rate = (double)code.m / (double)code.n;

    /* open the CSV results file */
    mkdir(cfg.output_dir, 0755);   /* ignore "already exists" */
    char csv_path[512];
    snprintf(csv_path, sizeof(csv_path), "%s/results_%s.csv", cfg.output_dir, algo_name(cfg.algorithm));
    FILE *csv = fopen(csv_path, "w");
    if (!csv) { fprintf(stderr, "main: cannot open '%s' for writing\n", csv_path); }
    else fprintf(csv, "snr_db,blocks,error_blocks,error_bits,ber,bler\n");

    printf("LDPC (%d, %d)  rate=%.4f  %s decoder  max_iter=%d  target_errors=%d\n",
           code.n, code.m, rate, algo_name(cfg.algorithm), cfg.max_iter, cfg.target_errors);
    printf("seed=%ld  H=%s  G=%s\n\n", cfg.seed, cfg.h_file, cfg.g_file);
    printf("  Eb/N0(dB)      blocks   errBlk        BER        BLER\n");
    printf("  ---------   ---------   ------   --------   --------\n");

    rng_state rng;
    rng_init(&rng, cfg.seed);
    prbs_source prbs;
    prbs_init(&prbs);

    const int show_progress = isatty(fileno(stderr));
    clock_t t0 = clock();

    for (int s = 0; s < cfg.snr_points; s++) {
        double snr = cfg.snr_start + s * cfg.snr_step;

        long blocks = 0, error_blocks = 0;
        long long error_bits = 0;

        while (error_blocks < cfg.target_errors) {
            prbs_generate(&prbs, msg, code.m);
            ldpc_encode(&code, msg, codeword);
            channel_transmit(codeword, code.n, rate, snr, &rng, llr);
            ldpc_decode(&code, llr, cfg.algorithm, cfg.max_iter, &ws);

            int block_errs = 0;
            for (int i = 0; i < code.m; i++)
                if (ws.hard[i] != msg[i]) block_errs++;

            blocks++;
            if (block_errs) { error_blocks++; error_bits += block_errs; }

            if (show_progress && (blocks & 0x3F) == 0)   /* live progress line */
                fprintf(stderr, "\r  %.1f dB: blocks=%ld errBlk=%ld/%d ",
                        snr, blocks, error_blocks, cfg.target_errors);
        }
        if (show_progress) fprintf(stderr, "\r%*s\r", 60, "");   /* clear it */

        double ber  = (double)error_bits / ((double)code.m * (double)blocks);
        double bler = (double)error_blocks / (double)blocks;

        printf("  %8.2f   %9ld   %6ld   %.2e   %.2e\n", snr, blocks, error_blocks, ber, bler);
        fflush(stdout);
        if (csv) {
            fprintf(csv, "%.2f,%ld,%ld,%lld,%.6e,%.6e\n",
                    snr, blocks, error_blocks, error_bits, ber, bler);
            fflush(csv);
        }
    }

    double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("\nelapsed: %.2f s\n", secs);
    if (csv) { printf("results written to %s\n", csv_path); fclose(csv); }

    free(msg); free(codeword); free(llr);
    decoder_ws_free(&ws);
    ldpc_free(&code);
    return EXIT_SUCCESS;
}
