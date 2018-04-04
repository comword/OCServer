#ifndef XXTEA_INCLUDED
#define XXTEA_INCLUDED
#include <stdint.h>
#include <stdlib.h>
#include "export.h"

size_t xxtea_fillBuffer( uint8_t *dat, size_t len, uint8_t *odat );
inline void push_bigendian( uint32_t t[2], char *s );
inline void pop_bigendian( uint8_t *s, uint32_t t[2] );
void encipher( uint32_t dat[2], uint32_t *key, uint32_t *odat );
void decipher( uint32_t dat[2], uint32_t *key, uint32_t *odat );

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function: xxtea_encrypt
 * @data:    Data to be encrypted
 * @len:     Length of the data to be encrypted
 * @key:     Symmetric key
 * @out_len: Pointer to output length variable
 * Returns:  Encrypted data or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
DLL_PUBLIC void *tea_encrypt( const void *data, size_t len, uint32_t *key, size_t *out_len );

/**
 * Function: xxtea_decrypt
 * @data:    Data to be decrypted
 * @len:     Length of the data to be decrypted
 * @key:     Symmetric key
 * @out_len: Pointer to output length variable
 * Returns:  Decrypted data or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
DLL_PUBLIC void *tea_decrypt( const void *data, size_t len, uint32_t *key, size_t *out_len );

#ifdef __cplusplus
}
#endif

#endif
