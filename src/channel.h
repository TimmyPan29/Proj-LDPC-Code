/*
 * channel.h -- BPSK modulation over an AWGN channel.
 *
 * Random numbers come from `ran1` (Press et al., Numerical Recipes): a minimal
 * portable generator with a Bays-Durham shuffle, so results are reproducible
 * from the seed regardless of platform. State is held per-stream rather than in
 * file-scope statics, which keeps the channel reentrant.
 */
#ifndef CHANNEL_H
#define CHANNEL_H

#define RNG_NTAB 32

typedef struct {
    long idum;            /* current state (seed must be negative to (re)init) */
    long iy;
    long iv[RNG_NTAB];
} rng_state;

/* Seed the generator. `seed` should be a negative integer. */
void rng_init(rng_state *r, long seed);

/* Uniform deviate in (0,1). */
double rng_uniform(rng_state *r);

/*
 * Map a binary codeword to BPSK symbols (bit 0 -> +1, bit 1 -> -1), pass it
 * through an AWGN channel at the given Eb/N0, and write the per-bit channel
 * LLRs to `llr_out` (length n). `rate` is the code rate R = m/n; a positive
 * LLR favours bit 0, matching the modulation convention above.
 */
void channel_transmit(const int *codeword, int n, double rate, double snr_db,
                      rng_state *rng, double *llr_out);

#endif /* CHANNEL_H */
