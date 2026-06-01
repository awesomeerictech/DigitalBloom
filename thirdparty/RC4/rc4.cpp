#include "rc4.h"

/* ************************************************************************** */

void rc4_crypt(const unsigned char *key, const unsigned key_length,
               unsigned char *data, const unsigned data_length)
{
    // rc4_state
    unsigned x, y, m[256];

    // rc4_setup
    x = 0;
    y = 0;

    for (int i = 0; i < 256; i++)
    {
        m[i] = i;
    }

    unsigned k = 0;
    unsigned j = 0;

    for (unsigned i = 0; i < 256; i++)
    {
        unsigned a = m[i];
        j = (unsigned char)(j + a + key[k]);
        m[i] = m[j];
        m[j] = a;
        if (++k >= key_length) k = 0;
    }

    // rc4_crypt
    for (unsigned i = 0; i < data_length; i++)
    {
        x = (unsigned char)(x + 1);
        unsigned a = m[x];
        y = (unsigned char)(y + a);
        unsigned b = m[y];
        m[x] = m[y];
        m[y] = a;
        data[i] ^= m[(unsigned char)(a + b)];
    }
}

/* ************************************************************************** */
