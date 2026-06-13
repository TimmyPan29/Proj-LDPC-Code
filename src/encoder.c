/*
 * encoder.c -- PRBS information source and systematic encoder.
 */
#include "encoder.h"

void prbs_init(prbs_source *s)
{
    s->reg[0] = 1;
    s->reg[1] = 0;
    s->reg[2] = 0;
    s->reg[3] = 0;
    s->reg[4] = 0;
    s->reg[5] = 0;
}

void prbs_generate(prbs_source *s, int *msg, int len)
{
    for (int i = 0; i < len; i++) {
        int bit = s->reg[0] ^ s->reg[1];   /* b[i-6] XOR b[i-5] */
        for (int j = 0; j < 5; j++) s->reg[j] = s->reg[j + 1];
        s->reg[5] = bit;
        msg[i] = bit;
    }
}

void ldpc_encode(const ldpc_code *code, const int *msg, int *codeword)
{
    for (int j = 0; j < code->n; j++) codeword[j] = 0;

    /* Each set information bit XORs in its generator row. For the systematic
     * part this simply copies the message into the first m positions. */
    for (int i = 0; i < code->m; i++) {
        if (!msg[i]) continue;
        const uint8_t *row = code->g[i];
        for (int j = 0; j < code->n; j++)
            codeword[j] ^= row[j];
    }
}
