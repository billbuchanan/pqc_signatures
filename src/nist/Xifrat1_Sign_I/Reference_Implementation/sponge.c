/* DannyNiu/NJF, 2018-02-07. Public Domain. */

#include "sponge.h"

void Sponge_Update(sponge_t *restrict s, void const *restrict data, size_t len)
{
    uint8_t const *restrict buffer = data;
    uint8_t *state = ((uint8_t *)s + s->offset);

    if( !data && len )
    {
        // 2018-02-09: Old note, may be relevant in future. 
        // Pads the block and invoke 1 permutation.
        // See cSHAKE[128|256]_Init.
        s->filled = s->rate;
        len = 0;
        goto permute;
    }

    while(len)
    {
        state[s->filled++] ^= *(buffer++);
        len--;

        if( s->filled == s->rate ) {
        permute:
            s->permute(state, state);
            s->filled = 0;
        }
    }
}

void Sponge_Final(sponge_t *restrict s)
{
    uint8_t *state = ((uint8_t *)s + s->offset);
    
    if( s->finalized ) return;

    /* Padding the Message. */

    state[s->filled] ^= s->lopad;
    state[s->rate-1] ^= s->hipad;

    s->permute(state, state);
    s->filled = 0;

    /* Finalization Guard. */

    s->finalized = 1;
}

void Sponge_Read(sponge_t *restrict s, void *restrict data, size_t len)
{
    uint8_t *ptr = data;
    uint8_t *state = ((uint8_t *)s + s->offset);
    
    while( len-- )
    {
        *(ptr++) = state[s->filled++];

        if( s->filled == s->rate ) {
            s->permute(state, state);
            s->filled = 0;
        }
    }
}
