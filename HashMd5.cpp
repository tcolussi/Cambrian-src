#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

#if 0
typedef struct
{
	UINT total[2];
	UINT state[4];
	BYTE buffer[64];
} md5_context;

void md5_starts( md5_context *ctx );
void md5_update( md5_context *ctx, const BYTE *input, UINT length );
void md5_finish( md5_context *ctx, BYTE digest[16] );

/*
 *  RFC 1321 compliant MD5 implementation
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define GET_UINT32(n,b,i)                       \
{                                               \
	(n) = ( (UINT) (b)[(i)    ]       )       \
		| ( (UINT) (b)[(i) + 1] <<  8 )       \
		| ( (UINT) (b)[(i) + 2] << 16 )       \
		| ( (UINT) (b)[(i) + 3] << 24 );      \
}

#define PUT_UINT32(n,b,i)                       \
{                                               \
	(b)[(i)    ] = (BYTE) ( (n)       );       \
	(b)[(i) + 1] = (BYTE) ( (n) >>  8 );       \
	(b)[(i) + 2] = (BYTE) ( (n) >> 16 );       \
	(b)[(i) + 3] = (BYTE) ( (n) >> 24 );       \
}

void md5_starts( md5_context *ctx )
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
}

void md5_process( md5_context *ctx, const BYTE data[64] )
{
	UINT X[16], A, B, C, D;
	memcpy(X, data, sizeof(X));

#define S(x,n) ((x << n) | ((x) >> (32 - n)))

#define P(a,b,c,d,k,s,t)                                \
{                                                       \
	a += F(b,c,d) + X[k] + t; a = S(a,s) + b;           \
}

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];

#define F(x,y,z) (z ^ (x & (y ^ z)))

	P( A, B, C, D,  0,  7, 0xD76AA478 );
	P( D, A, B, C,  1, 12, 0xE8C7B756 );
	P( C, D, A, B,  2, 17, 0x242070DB );
	P( B, C, D, A,  3, 22, 0xC1BDCEEE );
	P( A, B, C, D,  4,  7, 0xF57C0FAF );
	P( D, A, B, C,  5, 12, 0x4787C62A );
	P( C, D, A, B,  6, 17, 0xA8304613 );
	P( B, C, D, A,  7, 22, 0xFD469501 );
	P( A, B, C, D,  8,  7, 0x698098D8 );
	P( D, A, B, C,  9, 12, 0x8B44F7AF );
	P( C, D, A, B, 10, 17, 0xFFFF5BB1 );
	P( B, C, D, A, 11, 22, 0x895CD7BE );
	P( A, B, C, D, 12,  7, 0x6B901122 );
	P( D, A, B, C, 13, 12, 0xFD987193 );
	P( C, D, A, B, 14, 17, 0xA679438E );
	P( B, C, D, A, 15, 22, 0x49B40821 );

#undef F

#define F(x,y,z) (y ^ (z & (x ^ y)))

	P( A, B, C, D,  1,  5, 0xF61E2562 );
	P( D, A, B, C,  6,  9, 0xC040B340 );
	P( C, D, A, B, 11, 14, 0x265E5A51 );
	P( B, C, D, A,  0, 20, 0xE9B6C7AA );
	P( A, B, C, D,  5,  5, 0xD62F105D );
	P( D, A, B, C, 10,  9, 0x02441453 );
	P( C, D, A, B, 15, 14, 0xD8A1E681 );
	P( B, C, D, A,  4, 20, 0xE7D3FBC8 );
	P( A, B, C, D,  9,  5, 0x21E1CDE6 );
	P( D, A, B, C, 14,  9, 0xC33707D6 );
	P( C, D, A, B,  3, 14, 0xF4D50D87 );
	P( B, C, D, A,  8, 20, 0x455A14ED );
	P( A, B, C, D, 13,  5, 0xA9E3E905 );
	P( D, A, B, C,  2,  9, 0xFCEFA3F8 );
	P( C, D, A, B,  7, 14, 0x676F02D9 );
	P( B, C, D, A, 12, 20, 0x8D2A4C8A );

#undef F

#define F(x,y,z) (x ^ y ^ z)

	P( A, B, C, D,  5,  4, 0xFFFA3942 );
	P( D, A, B, C,  8, 11, 0x8771F681 );
	P( C, D, A, B, 11, 16, 0x6D9D6122 );
	P( B, C, D, A, 14, 23, 0xFDE5380C );
	P( A, B, C, D,  1,  4, 0xA4BEEA44 );
	P( D, A, B, C,  4, 11, 0x4BDECFA9 );
	P( C, D, A, B,  7, 16, 0xF6BB4B60 );
	P( B, C, D, A, 10, 23, 0xBEBFBC70 );
	P( A, B, C, D, 13,  4, 0x289B7EC6 );
	P( D, A, B, C,  0, 11, 0xEAA127FA );
	P( C, D, A, B,  3, 16, 0xD4EF3085 );
	P( B, C, D, A,  6, 23, 0x04881D05 );
	P( A, B, C, D,  9,  4, 0xD9D4D039 );
	P( D, A, B, C, 12, 11, 0xE6DB99E5 );
	P( C, D, A, B, 15, 16, 0x1FA27CF8 );
	P( B, C, D, A,  2, 23, 0xC4AC5665 );

#undef F

#define F(x,y,z) (y ^ (x | ~z))

	P( A, B, C, D,  0,  6, 0xF4292244 );
	P( D, A, B, C,  7, 10, 0x432AFF97 );
	P( C, D, A, B, 14, 15, 0xAB9423A7 );
	P( B, C, D, A,  5, 21, 0xFC93A039 );
	P( A, B, C, D, 12,  6, 0x655B59C3 );
	P( D, A, B, C,  3, 10, 0x8F0CCC92 );
	P( C, D, A, B, 10, 15, 0xFFEFF47D );
	P( B, C, D, A,  1, 21, 0x85845DD1 );
	P( A, B, C, D,  8,  6, 0x6FA87E4F );
	P( D, A, B, C, 15, 10, 0xFE2CE6E0 );
	P( C, D, A, B,  6, 15, 0xA3014314 );
	P( B, C, D, A, 13, 21, 0x4E0811A1 );
	P( A, B, C, D,  4,  6, 0xF7537E82 );
	P( D, A, B, C, 11, 10, 0xBD3AF235 );
	P( C, D, A, B,  2, 15, 0x2AD7D2BB );
	P( B, C, D, A,  9, 21, 0xEB86D391 );

#undef F

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;
}

void md5_update( md5_context *ctx, const BYTE *input, UINT length )
{
	UINT left, fill;

	if( ! length ) return;

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += length;
	ctx->total[0] &= 0xFFFFFFFF;

	if( ctx->total[0] < length )
		ctx->total[1]++;

	if( left && length >= fill )
	{
		memcpy( (void *) (ctx->buffer + left),
				(void *) input, fill );
		md5_process( ctx, ctx->buffer );
		length -= fill;
		input  += fill;
		left = 0;
	}

	while( length >= 64 )
	{
		md5_process( ctx, input );
		length -= 64;
		input  += 64;
	}

	if( length )
		memcpy( (void *) (ctx->buffer + left),	(void *) input, length );
}

static const BYTE md5_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void md5_finish( md5_context *ctx, BYTE digest[16] )
	{
	UINT last, padn;
	UINT high, low;
	BYTE msglen[8];

	high = ( ctx->total[0] >> 29 ) | ( ctx->total[1] <<  3 );
	low  = ( ctx->total[0] <<  3 );
	PUT_UINT32( low,  msglen, 0 );
	PUT_UINT32( high, msglen, 4 );

	last = ctx->total[0] & 0x3F;
	padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

	md5_update( ctx, md5_padding, padn );
	md5_update( ctx, msglen, 8 );

	PUT_UINT32( ctx->state[0], digest,  0 );
	PUT_UINT32( ctx->state[1], digest,  4 );
	PUT_UINT32( ctx->state[2], digest,  8 );
	PUT_UINT32( ctx->state[3], digest, 12 );
	}
#endif

#if 0
/* md5.c - Functions to compute MD5 message digest of files or memory blocks
   according to the definition of MD5 in RFC 1321 from April 1992.
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.

   NOTE: This source is derived from an old version taken from the GNU C
   Library (glibc).

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

#ifdef WORDS_BIGENDIAN
# define SWAP(n)							\
	(((n) << 24) | (((n) & 0xff00) << 8) | (((n) >> 8) & 0xff00) | ((n) >> 24))
#else
# define SWAP(n) (n)
#endif

typedef unsigned long md5_uint32;

/* This array contains the bytes used to pad the buffer to the next
   64-byte boundary.  (RFC 1321, 3.1: Step 1)  */
static const unsigned char fillbuf[64] = { 0x80, 0 /* , 0, 0, ...  */ };

/* Structure to save state of computation between the single steps.  */
struct md5_ctx
{
  md5_uint32 A;
  md5_uint32 B;
  md5_uint32 C;
  md5_uint32 D;

  md5_uint32 total[2];
  md5_uint32 buflen;
  char buffer[128];
};


/* Initialize structure containing state of computation.
   (RFC 1321, 3.3: Step 3)  */
void
md5_init_ctx (struct md5_ctx *ctx)
{
  ctx->A = (md5_uint32) 0x67452301;
  ctx->B = (md5_uint32) 0xefcdab89;
  ctx->C = (md5_uint32) 0x98badcfe;
  ctx->D = (md5_uint32) 0x10325476;

  ctx->total[0] = ctx->total[1] = 0;
  ctx->buflen = 0;
}

/* Put result from CTX in first 16 bytes following RESBUF.  The result
   must be in little endian byte order.

   IMPORTANT: On some systems it is required that RESBUF is correctly
   aligned for a 32 bits value.  */
void *
md5_read_ctx (const struct md5_ctx *ctx, void *resbuf)
{
  ((md5_uint32 *) resbuf)[0] = SWAP (ctx->A);
  ((md5_uint32 *) resbuf)[1] = SWAP (ctx->B);
  ((md5_uint32 *) resbuf)[2] = SWAP (ctx->C);
  ((md5_uint32 *) resbuf)[3] = SWAP (ctx->D);

  return resbuf;
}

void md5_process_bytes (const void *buffer, size_t len, struct md5_ctx *ctx);
void md5_process_block (const void *buffer, size_t len, struct md5_ctx *ctx);

/* Process the remaining bytes in the internal buffer and the usual
   prolog according to the standard and write the result to RESBUF.

   IMPORTANT: On some systems it is required that RESBUF is correctly
   aligned for a 32 bits value.  */
void *
md5_finish_ctx (struct md5_ctx *ctx, void *resbuf)
{
  /* Take yet unprocessed bytes into account.  */
  md5_uint32 bytes = ctx->buflen;
  size_t pad;

  /* Now count remaining bytes.  */
  ctx->total[0] += bytes;
  if (ctx->total[0] < bytes)
	++ctx->total[1];

  pad = bytes >= 56 ? 64 + 56 - bytes : 56 - bytes;
  memcpy (&ctx->buffer[bytes], fillbuf, pad);

  /* Put the 64-bit file length in *bits* at the end of the buffer.  */
  *(md5_uint32 *) &ctx->buffer[bytes + pad] = SWAP (ctx->total[0] << 3);
  *(md5_uint32 *) &ctx->buffer[bytes + pad + 4] = SWAP ((ctx->total[1] << 3) |
							(ctx->total[0] >> 29));

  /* Process last bytes.  */
  md5_process_block (ctx->buffer, bytes + pad + 8, ctx);

  return md5_read_ctx (ctx, resbuf);
}



/* Compute MD5 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
void *
md5_buffer (const char *buffer, size_t len, void *resblock)
{
  struct md5_ctx ctx;

  /* Initialize the computation context.  */
  md5_init_ctx (&ctx);

  /* Process whole buffer but last len % 64 bytes.  */
  md5_process_bytes (buffer, len, &ctx);

  /* Put result in desired memory area.  */
  return md5_finish_ctx (&ctx, resblock);
}


void
md5_process_bytes (const void *buffer, size_t len, struct md5_ctx *ctx)
{
  /* When we already have some bits in our internal buffer concatenate
	 both inputs first.  */
  if (ctx->buflen != 0)
	{
	  size_t left_over = ctx->buflen;
	  size_t add = 128 - left_over > len ? len : 128 - left_over;

	  memcpy (&ctx->buffer[left_over], buffer, add);
	  ctx->buflen += add;

	  if (left_over + add > 64)
	{
	  md5_process_block (ctx->buffer, (left_over + add) & ~63, ctx);
	  /* The regions in the following copy operation cannot overlap.  */
	  memcpy (ctx->buffer, &ctx->buffer[(left_over + add) & ~63],
		  (left_over + add) & 63);
	  ctx->buflen = (left_over + add) & 63;
	}

	  buffer = (const void *) ((const char *) buffer + add);
	  len -= add;
	}

  /* Process available complete blocks.  */
  if (len > 64)
	{
	  md5_process_block (buffer, len & ~63, ctx);
	  buffer = (const void *) ((const char *) buffer + (len & ~63));
	  len &= 63;
	}

  /* Move remaining bytes in internal buffer.  */
  if (len > 0)
	{
	  memcpy (ctx->buffer, buffer, len);
	  ctx->buflen = len;
	}
}


/* These are the four functions used in the four steps of the MD5 algorithm
   and defined in the RFC 1321.  The first function is a little bit optimized
   (as found in Colin Plumbs public domain implementation).  */
/* #define FF(b, c, d) ((b & c) | (~b & d)) */
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

/* Process LEN bytes of BUFFER, accumulating context into CTX.
   It is assumed that LEN % 64 == 0.  */

void
md5_process_block (const void *buffer, size_t len, struct md5_ctx *ctx)
{
  md5_uint32 correct_words[16];
  const md5_uint32 *words = (const md5_uint32 *) buffer;
  size_t nwords = len / sizeof (md5_uint32);
  const md5_uint32 *endp = words + nwords;
  md5_uint32 A = ctx->A;
  md5_uint32 B = ctx->B;
  md5_uint32 C = ctx->C;
  md5_uint32 D = ctx->D;

  /* First increment the byte count.  RFC 1321 specifies the possible
	 length of the file up to 2^64 bits.  Here we only compute the
	 number of bytes.  Do a double word increment.  */
  ctx->total[0] += len;
  if (ctx->total[0] < len)
	++ctx->total[1];

  /* Process all bytes in the buffer with 64 bytes in each round of
	 the loop.  */
  while (words < endp)
	{
	  md5_uint32 *cwp = correct_words;
	  md5_uint32 A_save = A;
	  md5_uint32 B_save = B;
	  md5_uint32 C_save = C;
	  md5_uint32 D_save = D;

	  /* First round: using the given function, the context and a constant
	 the next context is computed.  Because the algorithms processing
	 unit is a 32-bit word and it is determined to work on words in
	 little endian byte order we perhaps have to change the byte order
	 before the computation.  To reduce the work for the next steps
	 we store the swapped words in the array CORRECT_WORDS.  */

#define OP(a, b, c, d, s, T)						\
	  do								\
		{								\
	  a += FF (b, c, d) + (*cwp++ = SWAP (*words)) + T;		\
	  ++words;							\
	  CYCLIC (a, s);						\
	  a += b;							\
		}								\
	  while (0)

	  /* It is unfortunate that C does not provide an operator for
	 cyclic rotation.  Hope the C compiler is smart enough.  */
#define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

	  /* Before we start, one word to the strange constants.
	 They are defined in RFC 1321 as

	 T[i] = (int) (4294967296.0 * fabs (sin (i))), i=1..64
	   */

	  /* Round 1.  */
	  OP (A, B, C, D,  7, (md5_uint32) 0xd76aa478);
	  OP (D, A, B, C, 12, (md5_uint32) 0xe8c7b756);
	  OP (C, D, A, B, 17, (md5_uint32) 0x242070db);
	  OP (B, C, D, A, 22, (md5_uint32) 0xc1bdceee);
	  OP (A, B, C, D,  7, (md5_uint32) 0xf57c0faf);
	  OP (D, A, B, C, 12, (md5_uint32) 0x4787c62a);
	  OP (C, D, A, B, 17, (md5_uint32) 0xa8304613);
	  OP (B, C, D, A, 22, (md5_uint32) 0xfd469501);
	  OP (A, B, C, D,  7, (md5_uint32) 0x698098d8);
	  OP (D, A, B, C, 12, (md5_uint32) 0x8b44f7af);
	  OP (C, D, A, B, 17, (md5_uint32) 0xffff5bb1);
	  OP (B, C, D, A, 22, (md5_uint32) 0x895cd7be);
	  OP (A, B, C, D,  7, (md5_uint32) 0x6b901122);
	  OP (D, A, B, C, 12, (md5_uint32) 0xfd987193);
	  OP (C, D, A, B, 17, (md5_uint32) 0xa679438e);
	  OP (B, C, D, A, 22, (md5_uint32) 0x49b40821);

	  /* For the second to fourth round we have the possibly swapped words
	 in CORRECT_WORDS.  Redefine the macro to take an additional first
	 argument specifying the function to use.  */
#undef OP
#define OP(a, b, c, d, k, s, T)						\
	  do 								\
	{								\
	  a += FX (b, c, d) + correct_words[k] + T;			\
	  CYCLIC (a, s);						\
	  a += b;							\
	}								\
	  while (0)

#define FX(b, c, d) FG (b, c, d)

	  /* Round 2.  */
	  OP (A, B, C, D,  1,  5, (md5_uint32) 0xf61e2562);
	  OP (D, A, B, C,  6,  9, (md5_uint32) 0xc040b340);
	  OP (C, D, A, B, 11, 14, (md5_uint32) 0x265e5a51);
	  OP (B, C, D, A,  0, 20, (md5_uint32) 0xe9b6c7aa);
	  OP (A, B, C, D,  5,  5, (md5_uint32) 0xd62f105d);
	  OP (D, A, B, C, 10,  9, (md5_uint32) 0x02441453);
	  OP (C, D, A, B, 15, 14, (md5_uint32) 0xd8a1e681);
	  OP (B, C, D, A,  4, 20, (md5_uint32) 0xe7d3fbc8);
	  OP (A, B, C, D,  9,  5, (md5_uint32) 0x21e1cde6);
	  OP (D, A, B, C, 14,  9, (md5_uint32) 0xc33707d6);
	  OP (C, D, A, B,  3, 14, (md5_uint32) 0xf4d50d87);
	  OP (B, C, D, A,  8, 20, (md5_uint32) 0x455a14ed);
	  OP (A, B, C, D, 13,  5, (md5_uint32) 0xa9e3e905);
	  OP (D, A, B, C,  2,  9, (md5_uint32) 0xfcefa3f8);
	  OP (C, D, A, B,  7, 14, (md5_uint32) 0x676f02d9);
	  OP (B, C, D, A, 12, 20, (md5_uint32) 0x8d2a4c8a);

#undef FX
#define FX(b, c, d) FH (b, c, d)

	  /* Round 3.  */
	  OP (A, B, C, D,  5,  4, (md5_uint32) 0xfffa3942);
	  OP (D, A, B, C,  8, 11, (md5_uint32) 0x8771f681);
	  OP (C, D, A, B, 11, 16, (md5_uint32) 0x6d9d6122);
	  OP (B, C, D, A, 14, 23, (md5_uint32) 0xfde5380c);
	  OP (A, B, C, D,  1,  4, (md5_uint32) 0xa4beea44);
	  OP (D, A, B, C,  4, 11, (md5_uint32) 0x4bdecfa9);
	  OP (C, D, A, B,  7, 16, (md5_uint32) 0xf6bb4b60);
	  OP (B, C, D, A, 10, 23, (md5_uint32) 0xbebfbc70);
	  OP (A, B, C, D, 13,  4, (md5_uint32) 0x289b7ec6);
	  OP (D, A, B, C,  0, 11, (md5_uint32) 0xeaa127fa);
	  OP (C, D, A, B,  3, 16, (md5_uint32) 0xd4ef3085);
	  OP (B, C, D, A,  6, 23, (md5_uint32) 0x04881d05);
	  OP (A, B, C, D,  9,  4, (md5_uint32) 0xd9d4d039);
	  OP (D, A, B, C, 12, 11, (md5_uint32) 0xe6db99e5);
	  OP (C, D, A, B, 15, 16, (md5_uint32) 0x1fa27cf8);
	  OP (B, C, D, A,  2, 23, (md5_uint32) 0xc4ac5665);

#undef FX
#define FX(b, c, d) FI (b, c, d)

	  /* Round 4.  */
	  OP (A, B, C, D,  0,  6, (md5_uint32) 0xf4292244);
	  OP (D, A, B, C,  7, 10, (md5_uint32) 0x432aff97);
	  OP (C, D, A, B, 14, 15, (md5_uint32) 0xab9423a7);
	  OP (B, C, D, A,  5, 21, (md5_uint32) 0xfc93a039);
	  OP (A, B, C, D, 12,  6, (md5_uint32) 0x655b59c3);
	  OP (D, A, B, C,  3, 10, (md5_uint32) 0x8f0ccc92);
	  OP (C, D, A, B, 10, 15, (md5_uint32) 0xffeff47d);
	  OP (B, C, D, A,  1, 21, (md5_uint32) 0x85845dd1);
	  OP (A, B, C, D,  8,  6, (md5_uint32) 0x6fa87e4f);
	  OP (D, A, B, C, 15, 10, (md5_uint32) 0xfe2ce6e0);
	  OP (C, D, A, B,  6, 15, (md5_uint32) 0xa3014314);
	  OP (B, C, D, A, 13, 21, (md5_uint32) 0x4e0811a1);
	  OP (A, B, C, D,  4,  6, (md5_uint32) 0xf7537e82);
	  OP (D, A, B, C, 11, 10, (md5_uint32) 0xbd3af235);
	  OP (C, D, A, B,  2, 15, (md5_uint32) 0x2ad7d2bb);
	  OP (B, C, D, A,  9, 21, (md5_uint32) 0xeb86d391);

	  /* Add the starting values of the context.  */
	  A += A_save;
	  B += B_save;
	  C += C_save;
	  D += D_save;
	}

  /* Put checksum in context given as argument.  */
  ctx->A = A;
  ctx->B = B;
  ctx->C = C;
  ctx->D = D;
}

#endif

typedef unsigned char md5byte;
typedef quint32 UWORD32;

struct MD5Context {
	UWORD32 buf[4];
	UWORD32 bytes[2];
	UWORD32 in[16];
};

static void MD5Init(struct MD5Context *context);
static void MD5Update(struct MD5Context *context, md5byte const *buf, unsigned len);
static void MD5Final(struct MD5Context *context, unsigned char digest[16]);
static void MD5Transform(UWORD32 buf[4], UWORD32 const in[16]);

static void
byteSwap(UWORD32 *buf, unsigned words)
{
		const quint32 byteOrderTest = 0x1;
		if (((char *)&byteOrderTest)[0] == 0) {
			md5byte *p = (md5byte *)buf;

			do {
				*buf++ = (UWORD32)((unsigned)p[3] << 8 | p[2]) << 16 |
					((unsigned)p[1] << 8 | p[0]);
				p += 4;
			} while (--words);
		}
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void
MD5Init(struct MD5Context *ctx)
{
	ctx->buf[0] = 0x67452301;
	ctx->buf[1] = 0xefcdab89;
	ctx->buf[2] = 0x98badcfe;
	ctx->buf[3] = 0x10325476;

	ctx->bytes[0] = 0;
	ctx->bytes[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void
MD5Update(struct MD5Context *ctx, md5byte const *buf, unsigned len)
{
	UWORD32 t;

	/* Update byte count */

	t = ctx->bytes[0];
	if ((ctx->bytes[0] = t + len) < t)
		ctx->bytes[1]++;	/* Carry from low to high */

	t = 64 - (t & 0x3f);	/* Space available in ctx->in (at least 1) */
	if (t > len) {
		memcpy((md5byte *)ctx->in + 64 - t, buf, len);
		return;
	}
	/* First chunk is an odd size */
	memcpy((md5byte *)ctx->in + 64 - t, buf, t);
	byteSwap(ctx->in, 16);
	MD5Transform(ctx->buf, ctx->in);
	buf += t;
	len -= t;

	/* Process data in 64-byte chunks */
	while (len >= 64) {
		memcpy(ctx->in, buf, 64);
		byteSwap(ctx->in, 16);
		MD5Transform(ctx->buf, ctx->in);
		buf += 64;
		len -= 64;
	}

	/* Handle any remaining bytes of data. */
	memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void
MD5Final(struct MD5Context *ctx, md5byte digest[16])
{
	int count = ctx->bytes[0] & 0x3f;	/* Number of bytes in ctx->in */
	md5byte *p = (md5byte *)ctx->in + count;

	/* Set the first char of padding to 0x80.  There is always room. */
	*p++ = 0x80;

	/* Bytes of padding needed to make 56 bytes (-8..55) */
	count = 56 - 1 - count;

	if (count < 0) {	/* Padding forces an extra block */
		memset(p, 0, count + 8);
		byteSwap(ctx->in, 16);
		MD5Transform(ctx->buf, ctx->in);
		p = (md5byte *)ctx->in;
		count = 56;
	}
	memset(p, 0, count);
	byteSwap(ctx->in, 14);

	/* Append length in bits and transform */
	ctx->in[14] = ctx->bytes[0] << 3;
	ctx->in[15] = ctx->bytes[1] << 3 | ctx->bytes[0] >> 29;
	MD5Transform(ctx->buf, ctx->in);

	byteSwap(ctx->buf, 4);
	memcpy(digest, ctx->buf, 16);
	memset(ctx, 0, sizeof(*ctx));	/* In case it's sensitive */
}

#ifndef ASM_MD5

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f,w,x,y,z,in,s) \
	 (w += f(x,y,z) + in, w = (w<<s | w>>(32-s)) + x)

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void
MD5Transform(UWORD32 buf[4], UWORD32 const in[16])
{
	register UWORD32 a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}
#endif


VOID
HashMd5_CalculateFromBinary(OUT SHashMd5 * pHashMd5, IN PCVOID pvData, int cbData)
	{
	Assert(pHashMd5 != NULL);
	Assert(pvData != NULL);
	Assert(cbData >= 0);
	Endorse(cbData == 0);	// This will return the hash code "D41D8CD98F00B204E9800998ECF8427E"

	/*
	md5_context ctx;
	md5_starts(OUT &ctx);
	md5_update(INOUT &ctx, (const BYTE *)pvData, cbData);
	md5_finish(INOUT &ctx, OUT (BYTE *)pHashMd5);
	*/

	//md5_buffer(IN (const char *)pvData, cbData, OUT pHashMd5);

	#if 1
	MD5Context context;
	MD5Init(&context);
	MD5Update(&context, IN (const md5byte *)pvData, cbData);
	MD5Final(&context, OUT (md5byte *)pHashMd5);

	#else
	// The [slow] Qt hash functions which should produce the same results
	QByteArray arraybHash = QCryptographicHash::hash(QByteArray::fromRawData((const char *)pvData, cbData), QCryptographicHash::Md5);
	Assert(arraybHash.size() == sizeof(SHashMd5));
	memcpy(OUT pHashMd5, arraybHash.constData(), sizeof(SHashMd5));
	#endif
	}

void
HashMd5_CalculateFromCBin(OUT SHashMd5 * pHashMd5, IN const CBin & bin)
	{
	HashMd5_CalculateFromBinary(OUT pHashMd5, bin.PszuGetDataNZ(), bin.CbGetData());
	}

void
HashMd5_CalculateFromCStr(OUT SHashMd5 * pHashMd5, IN const CStr & str)
	{
	HashMd5_CalculateFromBinary(OUT pHashMd5, str.PszuGetDataNZ(), str.CchGetLength());
	}

void HashMd5_CalculateFromStringU(OUT SHashMd5 * pHashMd5, IN PSZUC pszuString)
	{
	HashMd5_CalculateFromBinary(OUT pHashMd5, pszuString, strlenU(pszuString));
	}

void
HashMd5_CalculateFromStringFormatted_Gsb(OUT SHashMd5 * pHashMd5, PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	// For performance, use g_strScratchBufferStatusBar as a temporary buffer
	g_strScratchBufferStatusBar.CalculateHashMd5FromFormattedString_VL(OUT pHashMd5, pszFmtTemplate, vlArgs) CONST_TEMPORARY_MODIFIED;
	}

/*
BOOL
HashMd5_FAssignHashValue(INOUT_F_UNCH SFileSizeAndMd5 * pHashMd5Destination, IN const SHashMd5 * pHashMd5Source)
	{

	}
*/
