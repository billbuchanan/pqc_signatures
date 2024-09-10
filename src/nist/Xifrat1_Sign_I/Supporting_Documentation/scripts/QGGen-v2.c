/* DannyNiu/NJF, 2022-04-01. Public Domain. */

#include "../../Reference_Implementation/endian.h"
#include "../../Reference_Implementation/shake.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

shake128_t prng_ctx;
#define prng(buf,len) SHAKE_Read(&prng_ctx, buf, len)

#define P_MAX 64
#define P2_MAX (P_MAX * P_MAX)

// row-major.
typedef int16_t sbox_t[P2_MAX];

typedef struct {
    // Set to the count of 0s in vec.
    int16_t left;

    // The set of occupied and available cell values.
    int16_t vec[P_MAX];
} cellopts_t;

void cellopts_init(cellopts_t *co, int16_t p)
{
    co->left = p;
    for(int16_t i=0; i<P_MAX; i++)
        co->vec[i] = 0;
}

int16_t cellopts_samp(cellopts_t *co)
{
    if( co->left <= 0 ) return -1;
    uint32_t w;
    int16_t v;

    do
    {
        prng(&w, sizeof w);
        w = le32toh(w);
    }
    while( (UINT32_MAX / co->left) * co->left <= w );

    v = w % co->left--;
    for(int16_t i=0; i<=v; i++)
        v += co->vec[i];
    
    co->vec[v] = 1;
    return v;
}

void cellopts_set(cellopts_t *co, int16_t v)
{
    if( co->vec[v] ) return;
    co->vec[v] = 1;
    co->left--;
}

typedef struct {
    int16_t     p, p2;
    int16_t     sind; // current position in sbox.
    cellopts_t  co, rows[P_MAX];
    sbox_t      sbox;
} state_t;

#if true

int state_test(state_t *state)
{
    int16_t a, b, c, d, u, v, x, y;
    int16_t *t1, *t2;
    int16_t p = state->p;

    for(a=0; a<p; a++)
    {
        for(d=0; d<p; d++)
        {
            if( a == d && state->sbox[a*p+d] == a ) return false;
            
            for(b=0; b<p; b++)
                for(c=b; c<p; c++)
                {
                    u = state->sbox[a*p+b];
                    v = state->sbox[c*p+d];
                    if( u<0 || v<0 ) continue;

                    x = state->sbox[a*p+c];
                    y = state->sbox[b*p+d];
                    if( x<0 || y<0 ) continue;

                    if( (u == x) != (v == y) ) return false;

                    t1 = state->sbox + (u*p+v);
                    t2 = state->sbox + (x*p+y);

                    if( *t1<0 && *t2<0 ) continue;
                    else if( *t1>=0 && *t2>=0 && *t1!=*t2 ) return false;
                    else if( *t1>=0 ) *t2 = *t1;
                    else if( *t2>=0 ) *t1 = *t2;
                }
        }
    }

    return true;
}

#else

int state_test(const state_t *state)
{
    int16_t a, b, c, d, u, v, x;
    int16_t p = state->p;

    for(a=0; a<p; a++)
    {
        for(d=0; d<p; d++)
        {
            if( a == d && state->sbox[a*p+d] == a ) return false;
            
            for(b=0; b<p; b++)
                for(c=b; c<p; c++)
                {
                    u = state->sbox[a*p+b];
                    v = state->sbox[c*p+d];
                    if( u<0 || v<0 ) continue;
                    
                    x = state->sbox[u*p+v];
                    if( x<0 ) continue;
                    
                    u = state->sbox[a*p+c];
                    v = state->sbox[b*p+d];
                    if( u<0 || v<0 ) continue;

                    u = u * p + v;
                    if( state->sbox[u] < 0 ) continue;
                    if( state->sbox[u] != x ) return false;
                }
        }
    }

    return true;
}

#endif

static state_t statestack[P2_MAX + P_MAX];
static state_t *sp = statestack;
static int isp = 0;

void state_try1step(state_t *state, int16_t v)
{
    int16_t p = state->p;
    int16_t p2 = state->p2;
    int16_t i, j;

    i = state->sind;

    // set current sbox.
    state->sbox[i] = v;
    cellopts_set(state->rows+i/p, v);
    
    for(j=i; j-=p, j>=0 && j<p2; )
    {
        if( state->rows[i/p].vec[state->sbox[j]] )
            continue;
        
        // trace the current column and set current row partially.
        for(int16_t m=i%p, ni=i-m, nj=j-m; m<p; m++)
        {
            if( state->sbox[m+nj] != v )
                continue;

            state->sbox[m+ni] = state->sbox[j];
            cellopts_set(state->rows+i/p, state->sbox[j]);
            break;
        }
    }
}

void sp_push()
{
    memcpy(sp+1, sp, sizeof(state_t));
    sp++;
    isp++;
}

void sp_pop()
{
    sp--;
    isp--;
}

static uint64_t scnt;

int state_iter(int16_t p)
{
    int16_t i, j;
    
    int16_t p2 = p * p;
    int16_t v;

    sp->p = p;
    sp->p2 = p2;
    sp->sind = 0;

    cellopts_init(&sp->co, p);
    for(i=0; i<p; i++) cellopts_init(sp->rows+i, p);
    for(i=0; i<p2; i++) sp->sbox[i] = -1;

loop_iter:
    scnt++;
    v = cellopts_samp(&sp->co);
    if( v >= 0 && v < p )
    {
        sp_push();
        state_try1step(sp, v);
        
        if( state_test(sp) )
        {
            cellopts_init(&sp->co, p);
            
            while( sp->sbox[sp->sind] != -1 )
            {
                sp->sind++;
                
                if( sp->sind >= sp->p2 )
                    return true;
            }
            
            i = sp->sind / p;
            j = sp->sind % p;
            for(int16_t t=0; t<p; t++)
            {
                v = sp->sbox[i*p+t];
                if( v >= 0 && v < p ) cellopts_set(&sp->co, v);
                
                v = sp->sbox[t*p+j];
                if( v >= 0 && v < p ) cellopts_set(&sp->co, v);
            }
            
            goto loop_iter;
        }
        else
        {
            sp_pop();
            goto loop_iter;
        }
    }
    else
    {
        if( sp == statestack )
            return false;
        
        sp_pop();
        goto loop_iter;
    }

    printf("Fell Through.\n");
    return false;
}

int16_t QGValue(int16_t a, int16_t b)
{
    int16_t p = sp->p;

    return sp->sbox[a*p+b];
}

int QGTest()
{
    int16_t p = sp->p;
    int16_t a, b, c, d, u, v, x;

    for(a=0; a<p; a++)
    {
        for(b=0; b<p; b++)
            for(c=0; c<p; c++)
                for(d=0; d<p; d++)
                {
                    u = QGValue(a, b);
                    v = QGValue(c, d);
                    x = QGValue(u, v);

                    u = QGValue(a, c);
                    v = QGValue(b, d);
                    if( QGValue(u, v) != x )
                    {
                        printf("inconsistent!\n");
                        return false;
                    }
                }
    }

    return true;
}

int main(int argc, char *argv[])
{
    const static char seed[] = "xifrat - public-key cryptosystem";

    int16_t p = 8;
    int subret;

    setvbuf(stdout, NULL, _IONBF, 512);
    SHAKE128_Init(&prng_ctx);
    SHAKE_Write(&prng_ctx, seed, strlen(seed));
    SHAKE_Final(&prng_ctx);

    if( argc > 1 ) p = atoi(argv[1]);

    subret = state_iter(p);

    if( !QGTest() ) subret = false;
    
    for(int16_t i=0; i<p; i++)
    {
        for(int16_t j=0; j<p; j++)
        {
            printf("% 3d ", sp->sbox[i*p+j]);
        }
        printf("\n");
    }

    if( subret )
    {
        printf("Success! scnt:%llu\n", (unsigned long long)scnt);
        return EXIT_SUCCESS;
    }
    else
    {
        printf("Failed! scnt:%llu\n", (unsigned long long)scnt);
        return EXIT_FAILURE;
    }
}
