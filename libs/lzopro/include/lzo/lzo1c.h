/* lzo1c.h -- public interface of the LZO1C compression algorithm

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


#ifndef __LZO1C_H_INCLUDED
#define __LZO1C_H_INCLUDED

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

#define LZO1C_MEM_COMPRESS      ((lzo_uint32) (16384L * lzo_sizeof_dict_t))
#define LZO1C_MEM_DECOMPRESS    (0)


/* compression levels */
#define LZO1C_BEST_SPEED             1
#define LZO1C_BEST_COMPRESSION       9
#define LZO1C_DEFAULT_COMPRESSION  (-1)     /* fastest by default */


LZO_EXTERN(int)
lzo1c_compress          ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem,
                                int compression_level );

/* decompression */
LZO_EXTERN(int)
lzo1c_decompress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );

/* safe decompression with overrun testing */
LZO_EXTERN(int)
lzo1c_decompress_safe   ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );


/***********************************************************************
//
************************************************************************/

LZO_EXTERN(int)
lzo1c_1_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_2_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_3_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_4_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_5_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_6_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_7_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_8_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_9_compress        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


/***********************************************************************
// better compression ratio at the cost of more memory and time
************************************************************************/

#define LZO1C_99_MEM_COMPRESS   ((lzo_uint32) (65536L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1c_99_compress       ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


#define LZO1C_999_MEM_COMPRESS  ((lzo_uint32) (5 * 16384L * sizeof(short)))

LZO_EXTERN(int)
lzo1c_999_compress      ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */


/* vim:set ts=4 et: */
