#include <avr/io.h>
#ifdef OUTPUT_V
#include "print.h"
#endif

#define NUM_LEN_BYTES 32

#define TRIGGER_PORT PORTE
#define TRIGGER_DDR  DDRE

static inline void trigger_s()
{
    TRIGGER_PORT = 0xFF;
}
static inline void trigger_r()
{
    TRIGGER_PORT = 0x00;
}

struct number {
    unsigned char v[NUM_LEN_BYTES];
};
struct ecc_point {
    struct number u;
    struct number v;
};

extern char bigint_mul256( unsigned char* r, const unsigned char* a, const unsigned char* b );
extern char bigint_square256( unsigned char* r, const unsigned char* a);
//extern char bigint_subp( unsigned char* r, const unsigned char* a);

extern void avrnacl_fe25519_red( struct number *r, unsigned char *C ); /* reduction modulo 2^255-19 */
extern void avrnacl_fe25519_add( struct number *r, const struct number *x, const struct number *y );
extern void avrnacl_fe25519_sub( struct number *r, const struct number *x, const struct number *y );

static void __end()
{
#ifdef OUTPUT_V
    serial_write( 4 );
#endif

    while( 1 )
    {
    };
}

//static unsigned char *n;
//static unsigned char *p;

// d
static const struct number d = {{
    0xA3, 0x78, 0x59, 0x13, 0xCA, 0x4D, 0xEB, 0x75, 0xAB, 0xD8, 0x41, 0x41, 0x4D, 0x0A, 0x70, 0x00, 
    0x98, 0xE8, 0x79, 0x77, 0x79, 0x40, 0xC7, 0x8C, 0x73, 0xFE, 0x6F, 0x2B, 0xEE, 0x6C, 0x03, 0x52
}};

static inline void ecc_setzero( struct number *r )
{
    unsigned char i;
    for( i = 0; i < NUM_LEN_BYTES; i++ )
    {
        r->v[i] = 0;
    }
}
static inline void ecc_setone( struct number *r ) 
{
    unsigned char i;
    r->v[0] = 1;
    for( i = 1; i < NUM_LEN_BYTES; i++ )
    {
        r->v[i] = 0;
    }
}
static inline void ecc_copy_number( struct number *r, struct number *n )
{
    unsigned char i;
    for( i = 0; i < NUM_LEN_BYTES; i++ )
    {
        r->v[i] = n->v[i];
    }
}

static inline void ecc_neg( struct number *r, const struct number *x ) 
{
    struct number t;
    ecc_setzero( &t );
    avrnacl_fe25519_sub( r, &t, x );
}

static inline void ecc_mul( struct number *r, const struct number *x, const struct number *y )
{
    unsigned char t[64];
    bigint_mul256( t, x->v, y->v );
    avrnacl_fe25519_red( r, t );
} 

static inline void ecc_square( struct number *r, const struct number *x )
{
    unsigned char t[64];
    bigint_square256( t, x->v );
    avrnacl_fe25519_red( r, t );
}

static inline void ecc_invert( struct number *r, const struct number *x )
{
    struct number z2;
    struct number z11;
    struct number z2_10_0;
    struct number z2_50_0;
    struct number z2_100_0;
    struct number t0;
    struct number t1;
    unsigned char i;

    /* 2 */
    ecc_square( &z2, x );
    /* 4 */
    ecc_square( &t1, &z2 );
    /* 8 */
    ecc_square( &t0, &t1 );
    /* 9 */
    ecc_mul( &z2_10_0, &t0, x );
    /* 11 */ 
    ecc_mul( &z11, &z2_10_0, &z2 );
    /* 22 */ 
    ecc_square( &t0, &z11 );
    /* 2^5 - 2^0 = 31 */ 
    ecc_mul( &z2_10_0, &t0, &z2_10_0 );

    /* 2^6 - 2^1 */ 
    ecc_square( &t0, &z2_10_0 );
    /* 2^7 - 2^2 */ 
    ecc_square( &t1, &t0 );
    /* 2^8 - 2^3 */ 
    ecc_square( &t0, &t1 );
    /* 2^9 - 2^4 */ 
    ecc_square( &t1, &t0 );
    /* 2^10 - 2^5 */ 
    ecc_square( &t0, &t1 );
    /* 2^10 - 2^0 */ 
    ecc_mul( &z2_10_0, &t0, &z2_10_0 );

    /* 2^11 - 2^1 */ 
    ecc_square( &t0, &z2_10_0 );
    /* 2^12 - 2^2 */ 
    ecc_square( &t1, &t0 );
    /* 2^20 - 2^10 */ 
    for( i = 2; i < 10; i += 2)
    {
        ecc_square( &t0, &t1 );
        ecc_square( &t1, &t0 );
    }
    /* 2^20 - 2^0 */
    ecc_mul( &z2_50_0, &t1, &z2_10_0 );

    /* 2^21 - 2^1 */
    ecc_square( &t0, &z2_50_0 );
    /* 2^22 - 2^2 */
    ecc_square( &t1, &t0 );
    /* 2^40 - 2^20 */
    for( i = 2; i < 20; i += 2 )
    {
        ecc_square( &t0, &t1 );
        ecc_square( &t1, &t0 );
    }
    /* 2^40 - 2^0 */
    ecc_mul( &t0, &t1, &z2_50_0 );

    /* 2^41 - 2^1 */
    ecc_square( &t1, &t0 );
    /* 2^42 - 2^2 */
    ecc_square( &t0, &t1 );
    /* 2^50 - 2^10 */
    for( i = 2; i < 10; i += 2 )
    {
        ecc_square( &t1, &t0 );
        ecc_square( &t0, &t1 );
    }
    /* 2^50 - 2^0 */
    ecc_mul( &z2_50_0, &t0, &z2_10_0 );

    /* 2^51 - 2^1 */
    ecc_square( &t0, &z2_50_0 );
    /* 2^52 - 2^2 */
    ecc_square( &t1, &t0 );
    /* 2^100 - 2^50 */
    for( i = 2; i < 50; i += 2 )
    {
        ecc_square( &t0, &t1 );
        ecc_square( &t1, &t0 );
    }
    /* 2^100 - 2^0 */
    ecc_mul( &z2_100_0, &t1, &z2_50_0 );

    /* 2^101 - 2^1 */
    ecc_square( &t1, &z2_100_0 );
    /* 2^102 - 2^2 */
    ecc_square( &t0, &t1 );
    /* 2^200 - 2^100 */
    for( i = 2; i < 100; i += 2 )
    {
        ecc_square( &t1, &t0 );
        ecc_square( &t0, &t1 );
    }
    /* 2^200 - 2^0 */
    ecc_mul( &t1, &t0, &z2_100_0 );

    /* 2^201 - 2^1 */
    ecc_square( &t0, &t1 );
    /* 2^202 - 2^2 */
    ecc_square( &t1, &t0 );
    /* 2^250 - 2^50 */
    for( i = 2; i < 50; i += 2 )
    {
        ecc_square( &t0, &t1 );
        ecc_square( &t1, &t0 );
    }
    /* 2^250 - 2^0 */
    ecc_mul( &t0, &t1, &z2_50_0 );

    /* 2^251 - 2^1 */
    ecc_square( &t1, &t0 );
    /* 2^252 - 2^2 */
    ecc_square( &t0, &t1 );
    /* 2^253 - 2^3 */
    ecc_square( &t1, &t0 );
    /* 2^254 - 2^4 */
    ecc_square( &t0, &t1 );
    /* 2^255 - 2^5 */
    ecc_square( &t1, &t0 );
    /* 2^255 - 21 */
    ecc_mul( r, &t1, &z11 );
}


static inline void ecc_points_add( struct ecc_point* R, struct ecc_point* P, struct ecc_point* Q )
{
    struct number v1;
    struct number v2;
    struct number t1;
    struct number t2;
    struct number t3;
    struct number one;
    struct ecc_point T;

    ecc_setzero( &(T.u) );
    ecc_setzero( &(T.v) );

    ecc_setzero( &v1 );
    ecc_setzero( &v2 );
    ecc_setzero( &t1 );
    ecc_setzero( &t2 );
    ecc_setzero( &t3 );
    ecc_setone( &one );

    /* u1v2 */
    ecc_mul( &v1, &(P->u), &(Q->v) );
    /* u2v1 */
    ecc_mul( &v2, &(Q->u), &(P->v) );

    /* u1v2 + u2v1 */
    avrnacl_fe25519_add( &t1, &v1, &v2 );

    /* m = u1v2 * u2v1 */
    ecc_mul( &t2, &v1, &v2 );

    ecc_setzero( &v1 );
    ecc_setzero( &v2 );

    /* v1v2 */
    ecc_mul( &v1, &(P->v), &(P->v) );
    /* u1u2 */
    ecc_mul( &v2, &(Q->u), &(Q->u) );

    /* v1v2 + u1u2 */
    avrnacl_fe25519_add( &t3, &v1, &v2 );

    ecc_setzero( &v1 );
    ecc_setzero( &v2 );

    /* dm */
    ecc_mul( &v1, &d, &t2 );

    ecc_setzero( &t2 );

    /* 1 + dm */
    avrnacl_fe25519_add( &v2, &one, &v1 );
    /* 1 - dm */
    avrnacl_fe25519_sub( &t2, &one, &v1 );

    ecc_setzero( &v1 );

    /* (1+dm)^-1 */
    ecc_invert( &v1, &v2 );

    ecc_setzero( &v2 );

    /* (1-dm)^-1 */
    ecc_invert( &v2, &t2 );

    /* result */
    ecc_mul( &(T.u), &t1, &v1 );
    ecc_mul( &(T.v), &t3, &v2 );

    /* write result */
    ecc_copy_number( &(R->u), &(T.u) );
    ecc_copy_number( &(R->v), &(T.v) );
}

static inline void ecc_point_double( struct ecc_point* R, struct ecc_point* P )
{
    /* TODO rewrite */
    ecc_points_add( R, P, P );
}

void ecc_point_multiplication_binary( struct number *k, struct ecc_point* P, struct ecc_point* Q )
{
    trigger_s();

    for( unsigned int i = 0; i < NUM_LEN_BYTES; i++ )
    {
        for( unsigned int j = 8; j > 0; j-- )
        {
            ecc_point_double( Q, Q );
            if( k->v[i] & (1 << (j - 1)) )
            {
                ecc_points_add( Q, Q, P );
            }
        }
    }

    trigger_r();
}

static inline void init()
{
    TRIGGER_DDR = 0xFF;
    TRIGGER_PORT = 0x00;
}

int main( void )
{
    init();

    //trigger_s();
    //trigger_r();

    struct number k;
    struct ecc_point P, Q;

    ecc_setzero( &k );
    //k.v[0] = 0b11111111;
    k.v[0] = 0b10101010;
    //k.v[0] = 0b11001100;
    //k.v[0] = 0b10001000;

    ecc_setzero( &(P.u) );
    ecc_setzero( &(P.v) );
    ecc_setzero( &(Q.u) );
    ecc_setzero( &(Q.v) );

    P.u.v[31] = 0xAB;
    P.v.v[31] = 0x08;
    //Q.u.v[30] = 0x9D;
    //Q.v.v[31] = 0x35;
    
    ecc_point_multiplication_binary( &k, &P, &Q );

#ifdef OUTPUT_V
    print( "u1\n" );
    print_bytes( P.u.v, 32 );
    print( "\nv1\n" );
    print_bytes( P.v.v, 32 );

    print( "\n\nu2\n" );
    print_bytes( Q.u.v, 32 );
    print( "\nv2\n" );
    print_bytes( Q.v.v, 32 );

    print( "\n\nu3\n" );
    print_bytes( R.u.v, 32 );
    print( "\nv3\n" );
    print_bytes( R.v.v, 32 );
#endif

    __end();
    return 0;
}

