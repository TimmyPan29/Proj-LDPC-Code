/*
 * encoder.h -- Information source and systematic LDPC encoding.
 *
 * The information bits come from a maximal-length LFSR (PRBS) with the tap
 * recurrence b[i] = b[i-5] XOR b[i-6]. State is carried across blocks so the
 * source behaves as one continuous pseudo-random stream.
 */
#ifndef ENCODER_H
#define ENCODER_H

#include "ldpc.h"

typedef struct {
    int reg[6];   /* the six most recent bits; reg[0] is the oldest */
} prbs_source;

/* Initialise the LFSR to a fixed non-zero state. */
void prbs_init(prbs_source *s);

/* Generate `len` information bits into `msg`. */
void prbs_generate(prbs_source *s, int *msg, int len);

/* codeword = msg * G over GF(2). `codeword` has length code->n. */
void ldpc_encode(const ldpc_code *code, const int *msg, int *codeword);

#endif /* ENCODER_H */
