/* lzo1x.h -- public interface of the LZO1X compression algorithm

   This file is part of the LZO-Professional data compression library.

   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   CONFIDENTIAL & PROPRIETARY SOURCE CODE.

   ANY USAGE OF THIS FILE IS SUBJECT TO YOUR LICENSE AGREEMENT.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/products/lzo-professional/
 */


#ifndef __LZO1X_H_INCLUDED
#define __LZO1X_H_INCLUDED

#ifndef __LZOCONF_H_INCLUDED
#include "lzoconf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
//
************************************************************************/

/* Memory required for the wrkmem parameter.
 * When the required size is 0, you can also pass a NULL pointer.
 */

#define LZO1X_MEM_COMPRESS      LZO1X_1_MEM_COMPRESS
#define LZO1X_MEM_DECOMPRESS    (0)
#define LZO1X_MEM_OPTIMIZE      (0)


/* decompression */
LZO_EXTERN(int)
lzo1x_decompress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );

/* safe decompression with overrun testing */
LZO_EXTERN(int)
lzo1x_decompress_safe   ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );


/***********************************************************************
//
************************************************************************/

#define LZO1X_1_MEM_COMPRESS    ((lzo_uint32) (16384L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


/***********************************************************************
// special compressor versions
************************************************************************/

/* this version needs only 8 kB work memory */
#define LZO1X_1_11_MEM_COMPRESS ((lzo_uint32) (2048L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_11_compress     ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


/* this version needs 16 kB work memory */
#define LZO1X_1_12_MEM_COMPRESS ((lzo_uint32) (4096L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_12_compress     ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


/* use this version if you need a little more compression speed */
#define LZO1X_1_15_MEM_COMPRESS ((lzo_uint32) (32768L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_15_compress     ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


/***********************************************************************
// better compression ratio at the cost of more memory and time
************************************************************************/

#define LZO1X_999_MEM_COMPRESS  ((lzo_uint32) (14 * 16384L * sizeof(short)))

LZO_EXTERN(int)
lzo1x_999_compress      ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


/***********************************************************************
//
************************************************************************/

LZO_EXTERN(int)
lzo1x_999_compress_dict     ( const lzo_bytep in , lzo_uint  in_len,
                                    lzo_bytep out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_bytep dict, lzo_uint dict_len );

LZO_EXTERN(int)
lzo1x_999_compress_level    ( const lzo_bytep in , lzo_uint  in_len,
                                    lzo_bytep out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_bytep dict, lzo_uint dict_len,
                                    lzo_callback_p cb,
                                    int compression_level );

LZO_EXTERN(int)
lzo1x_decompress_dict_safe ( const lzo_bytep in,  lzo_uint  in_len,
                                   lzo_bytep out, lzo_uintp out_len,
                                   lzo_voidp wrkmem /* NOT USED */,
                             const lzo_bytep dict, lzo_uint dict_len );


/***********************************************************************
// optimize a compressed data block
************************************************************************/

LZO_EXTERN(int)
lzo1x_optimize          (       lzo_bytep in , lzo_uint  in_len,
                                lzo_bytep out, lzo_uintp out_len,
                                lzo_voidp wrkmem /* NOT USED */ );



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */


/* vim:set ts=4 et: */
