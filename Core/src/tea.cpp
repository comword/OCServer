#include "tea.h"

#include <stdlib.h>
#include <string.h>

#include <string.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
#else
#if defined(__FreeBSD__) && __FreeBSD__ < 5
/* FreeBSD 4 doesn't have stdint.h file */
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#endif

#include <sys/types.h> /* This will likely define BYTE_ORDER */

#ifndef BYTE_ORDER
#if (BSD >= 199103)
# include <machine/endian.h>
#else
#if defined(linux) || defined(__linux__)
# include <endian.h>
#else
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax, pc) */
#define BIG_ENDIAN  4321    /* most-significant byte first (IBM, net) */
#define PDP_ENDIAN  3412    /* LSB first in word, MSW first in long (pdp)*/

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
   defined(vax) || defined(ns32000) || defined(sun386) || \
   defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
   defined(__alpha__) || defined(__alpha)
#define BYTE_ORDER    LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined (BIT_ZERO_ON_LEFT) || defined(m68k) || defined(__sparc)
#define BYTE_ORDER  BIG_ENDIAN
#endif
#endif /* linux */
#endif /* BSD */
#endif /* BYTE_ORDER */

#ifndef BYTE_ORDER
#ifdef __BYTE_ORDER
#if defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#endif
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif
#endif
#endif

#define DELTA 0x9e3779b9

size_t tea_fillBuffer( uint8_t *dat, size_t len, uint8_t **odat )
{
    signed char filln = ( 8 - ( len + 2 ) ) % 8;
    filln += ( filln < 0 ) ? 10 : 2;
    uint8_t *out = ( uint8_t * )malloc( filln + len + 8 );
    out[0] = ( 0 & 0xf8 ) | (filln-2);
    for( int i = 1; i <= filln; i++ ) {
        //out[i] = std::rand() % 256;
    	out[i] = 0;
    }
    memcpy( out + filln + 1, dat, len );
    memset( out + filln + 1 + len, 0, 7 );
    *odat = out;
    return filln + len + 8;
}

inline void push_bigendian( uint32_t t[2], uint8_t *s )
{
#if defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN)
	t[0] = *( s + 3 ) & 0x000000ff;
	t[0] |= ( *( s + 2 ) << 8 ) & 0x0000ff00;
	t[0] |= ( *( s + 1 ) << 16 ) & 0x00ff0000;
	t[0] |= ( *( s ) << 24 ) & 0xff000000;
	t[1] = *( s + 7 ) & 0x000000ff;
	t[1] |= ( *( s + 6 ) << 8 ) & 0x0000ff00;
	t[1] |= ( *( s + 5 ) << 16 ) & 0x00ff0000;
	t[1] |= ( *( s + 4 ) << 24 ) & 0xff000000;
#else
	memcpy(t,s,2*sizeof(uint32_t));
#endif
}

inline void pop_bigendian( uint8_t *s, uint32_t t[2] )
{
#if defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN)
	*( s ) = (t[0] & 0xff000000) >> 24;
	*( s + 1 ) = (t[0] & 0x00ff0000) >> 16;
	*( s + 2 ) = (t[0] & 0x0000ff00) >> 8;
	*( s + 3 ) = t[0] & 0x000000ff;
	*( s + 4 ) = (t[1] & 0xff000000) >> 24;
	*( s + 5 ) = (t[1] & 0x00ff0000) >> 16;
	*( s + 6 ) = (t[1] & 0x0000ff00) >> 8;
	*( s + 7 ) = t[1] & 0x000000ff;
#else
	memcpy(s,t,2*sizeof(uint32_t));
#endif
}

void encipher( uint32_t dat[2], uint32_t *key, uint32_t *odat )
{
    char n = 32;
    uint32_t y = dat[0], z = dat[1], sum = DELTA;
    uint32_t a = key[0], b = key[1], c = key[2], d = key[3];
    while( n-- > 0 ) {
        y += ( ( ( z << 4 ) & 0xFFFFFFF0 ) + a ) ^ ( z + sum ) ^ ( ( ( z >> 5 ) & 0x07ffffff ) + b );
        z += ( ( ( y << 4 ) & 0xFFFFFFF0 ) + c ) ^ ( y + sum ) ^ ( ( ( y >> 5 ) & 0x07ffffff ) + d );
        sum += DELTA;
    }
    odat[0] = y;
    odat[1] = z;
}

void decipher( uint32_t dat[2], uint32_t *key, uint32_t *odat )
{
    char n = 32;
    uint32_t y = dat[0], z = dat[1], sum = DELTA << 5 & 0xffffffff;
    uint32_t a = key[0], b = key[1], c = key[2], d = key[3];
    while( n-- > 0 ) {
        z -= ( ( ( y << 4 & 0xFFFFFFF0 ) + c ) ^ ( y + sum ) ^ ( ( y >> 5 & 0x07ffffff ) + d ) );
        z &= 0xffffffff;
        y -= ( ( ( z << 4 & 0xFFFFFFF0 ) + a ) ^ ( z + sum ) ^ ( ( z >> 5 & 0x07ffffff ) + b ) );
        y &= 0xffffffff;
        sum -= DELTA;
    }
    odat[0] = y;
    odat[1] = z;
}

inline void m_xor( uint32_t *o, uint32_t *a, uint32_t *b )
{
    o[0] = a[0] ^ b[0];
    o[1] = a[1] ^ b[1];
}

void *tea_encrypt( const void *data, size_t len, uint32_t *key, size_t *out_len )
{
    uint8_t *buf;
    size_t buf_len = tea_fillBuffer( ( uint8_t * )data, len, &buf );
    *out_len = buf_len;
    uint32_t ibyte32[2] = {0};
    uint32_t obyte32[2] = {0};
    uint32_t tr[2] = {0};
    uint32_t to[2] = {0};
    uint32_t o[2] = {0};
    for( size_t i = 0; i < buf_len; i += 8 ) {
        push_bigendian( ibyte32, buf + i );
        m_xor( o, ibyte32, tr );
        memcpy( ibyte32, o, 2 * sizeof( uint32_t ) );
        encipher( ibyte32, key, obyte32 );
        m_xor( tr, obyte32, to );
        memcpy( to, o, 2 * sizeof( uint32_t ) );
        pop_bigendian( buf + i, tr );
        //push_vec( res, tr );
    }
    return buf;
}

void *tea_decrypt( const void *data, size_t len, uint32_t *key, size_t *out_len )
{
	if( len <= 0 || ( len % 8 ) != 0 )
		return nullptr;
	uint32_t ibyte32[2] = {0};
	uint32_t obyte32[2] = {0};
	push_bigendian( ibyte32, (uint8_t*)data );
	decipher( ibyte32, key, obyte32 );
	int pos = ( ( obyte32[0] >> 24 ) & 0x7 ) + 3;
	uint32_t x[2] = {0};
	uint8_t *buf = (uint8_t *)malloc(len);
	pop_bigendian(buf,obyte32);
	for( size_t i = 8; i < len; i += 8 ) {
		push_bigendian( ibyte32, (uint8_t *)data + i );
		m_xor( x, ibyte32, obyte32 );
		ibyte32[0] = x[0];
		ibyte32[1] = x[1];
		decipher(ibyte32, key, obyte32);
		push_bigendian( ibyte32, (uint8_t *)data + i - 8 );
		m_xor( x, obyte32, ibyte32 );
		m_xor( obyte32, x, ibyte32 );
		push_bigendian( ibyte32, (uint8_t *)data + i );
		pop_bigendian(buf+i,x);
	}
	for( size_t i = len - 1; i >= len - 7; i-- )
		if( buf[i] != 0 )
			return nullptr;
	*out_len = len - pos - 7;
	uint8_t *res = (uint8_t *)malloc(*out_len);
	memcpy(res,buf+pos,*out_len);
	free(buf);
	return res;
}
