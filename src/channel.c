/*
 * channel.c -- ran1 PRNG, plus BPSK + AWGN transmission.
 */
#include "channel.h"

#include <math.h>

/* ran1 constants (Numerical Recipes, "minimal standard" with shuffle). */
#define IA   16807
#define IM   2147483647
#define AM   (1.0 / IM)
#define IQ   127773
#define IR   2836
#define NDIV (1 + (IM - 1) / RNG_NTAB)
#define EPS  1.2e-7
#define RNMX (1.0 - EPS)

void rng_init(rng_state *r, long seed)
{
    r->idum = (seed < 0) ? seed : -seed;   /* a negative seed forces (re)init */
    if (r->idum == 0) r->idum = -1;
    r->iy = 0;
}

double rng_uniform(rng_state *r)
{
    int j;
    long k;

    if (r->idum <= 0 || !r->iy) {
        if (-r->idum < 1) r->idum = 1; else r->idum = -r->idum;
        for (j = RNG_NTAB + 7; j >= 0; j--) {
            k = r->idum / IQ;
            r->idum = IA * (r->idum - k * IQ) - IR * k;
            if (r->idum < 0) r->idum += IM;
            if (j < RNG_NTAB) r->iv[j] = r->idum;
        }
        r->iy = r->iv[0];
    }
    k = r->idum / IQ;
    r->idum = IA * (r->idum - k * IQ) - IR * k;
    if (r->idum < 0) r->idum += IM;
    j = (int)(r->iy / NDIV);
    r->iy = r->iv[j];
    r->iv[j] = r->idum;

    double temp = AM * r->iy;
    return (temp > RNMX) ? RNMX : temp;
}

/* One standard normal sample via the Marsaglia polar method (two at a time). */
static void gaussian_pair(rng_state *rng, double *z0, double *z1)
{
    double u1, u2, s;
    do {
        u1 = 2.0 * rng_uniform(rng) - 1.0;
        u2 = 2.0 * rng_uniform(rng) - 1.0;
        s  = u1 * u1 + u2 * u2;
    } while (s >= 1.0 || s == 0.0);
    double factor = sqrt(-2.0 * log(s) / s);
    *z0 = u1 * factor;
    *z1 = u2 * factor;
}

void channel_transmit(const int *codeword, int n, double rate, double snr_db,
                      rng_state *rng, double *llr_out)
{
    double snr_lin = pow(10.0, snr_db / 10.0);     /* Eb/N0 (linear)       */
    double sigma   = sqrt(1.0 / (2.0 * rate * snr_lin));
    double Lc      = 4.0 * rate * snr_lin;         /* = 2 / sigma^2        */

    for (int i = 0; i < n; i += 2) {
        double z0, z1;
        gaussian_pair(rng, &z0, &z1);

        double s0 = (codeword[i] ? -1.0 : 1.0) + sigma * z0;
        llr_out[i] = Lc * s0;
        if (i + 1 < n) {
            double s1 = (codeword[i + 1] ? -1.0 : 1.0) + sigma * z1;
            llr_out[i + 1] = Lc * s1;
        }
    }
}
