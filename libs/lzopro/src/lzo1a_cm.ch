/* lzo1a_cm.ch -- implementation of the LZO1A compression algorithm

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


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */



/***********************************************************************
// code the match in LZO1 compatible format
************************************************************************/

#define THRESHOLD   (M2_MIN_LEN - 1)
#define MSIZE       LZO_SIZE(M2L_BITS)


/***********************************************************************
//
************************************************************************/

#if (DD_BITS == 0)

        /* we already matched M2_MIN_LEN bytes,
         * m_pos also already advanced M2_MIN_LEN bytes */
        ip += M2_MIN_LEN;
        assert(m_pos < ip);

        /* try to match another M2_MAX_LEN + 1 - M2_MIN_LEN bytes
         * to see if we get more than a M2 match */
#define M2_OR_M3    (MATCH_M2)

#else /* (DD_BITS == 0) */

        /* we already matched m_len bytes */
        assert(m_len >= M2_MIN_LEN);
        ip += m_len;
        assert(ip <= in_end);

#define M2_OR_M3    (m_len <= M2_MAX_LEN)

#endif /* (DD_BITS == 0) */


        if (M2_OR_M3)
        {
        /* we've found a short match */
            assert(ip <= in_end);

        /* 2a) compute match parameters */
#if (DD_BITS == 0)
                assert(pd(ip,m_pos) == m_off);
            --ip;   /* ran one too far, point back to non-match */
            m_len = ip - ii;
#endif
                assert(m_len >= M2_MIN_LEN);
                assert(m_len <= M2_MAX_LEN);

                assert(m_off >= M2_MIN_OFFSET);
                assert(m_off <= M2_MAX_OFFSET);
                assert(ii-m_off == m_pos_sav);
                assert(lzo_memcmp(m_pos_sav,ii,m_len) == 0);

        /* 2b) code the match */
            m_off -= M2_MIN_OFFSET;
            /* code short match len + low offset bits */
            *op++ = LZO_BYTE(((m_len - THRESHOLD) << M2O_BITS) |
                             (m_off & M2O_MASK));
            /* code high offset bits */
            *op++ = LZO_BYTE(m_off >> M2O_BITS);


            if (ip >= ip_end)
            {
                ii = ip;
                break;
            }


        /* 2c) Insert phrases (beginning with ii+1) into the dictionary. */

#if (CLEVEL == 9) || (CLEVEL >= 7 && M2L_BITS <= 4) || (CLEVEL >= 5 && M2L_BITS <= 3)
        /* Insert the whole match (ii+1)..(ip-1) into dictionary.  */
            ++ii;
            do {
                DVAL_NEXT(dv,ii);
#if 0
                UPDATE_D(dict,drun,dv,ii,in);
#else
                dict[ DINDEX(dv,ii) ] = DENTRY(ii,in);
#endif
                MI
            } while (++ii < ip);
            DVAL_NEXT(dv,ii);
            assert(ii == ip);
            DVAL_ASSERT(dv,ip);
#elif (CLEVEL >= 3)
            SI   DI DI   XI
#elif (CLEVEL >= 2)
            SI   DI      XI
#else
                         XI
#endif
        }

        else

        {
        /* we've found a long match - see how far we can still go */
            const lzo_bytep end;

            assert(ip <= in_end);
            assert(ii == ip - (M2_MAX_LEN + 1));
            assert(lzo_memcmp(m_pos_sav,ii,(lzo_uint)(ip-ii)) == 0);

#if (DD_BITS > 0)
            assert(m_len == (lzo_uint)(ip-ii));
            m_pos = ip - m_off;
            assert(m_pos == m_pos_sav + m_len);
#endif

            if (pd(in_end,ip) <= (M3_MAX_LEN - M3_MIN_LEN))
                end = in_end;
            else
            {
                end = ip + (M3_MAX_LEN - M3_MIN_LEN);
                assert(end < in_end);
            }

            while (ip < end  &&  *m_pos == *ip)
                m_pos++, ip++;
            assert(ip <= in_end);

            /* 2a) compute match parameters */
            m_len = pd(ip, ii);
                assert(m_len >= M3_MIN_LEN);
                assert(m_len <= M3_MAX_LEN);

                assert(m_off >= M3_MIN_OFFSET);
                assert(m_off <= M3_MAX_OFFSET);
                assert(ii-m_off == m_pos_sav);
                assert(lzo_memcmp(m_pos_sav,ii,m_len) == 0);
                assert(pd(ip,m_pos) == m_off);

        /* 2b) code the match */
            m_off -= M3_MIN_OFFSET - M3_EOF_OFFSET;
            /* code long match flag + low offset bits */
            *op++ = LZO_BYTE(((MSIZE - 1) << M3O_BITS) | (m_off & M3O_MASK));
            /* code high offset bits */
            *op++ = LZO_BYTE(m_off >> M3O_BITS);
            /* code match len */
            *op++ = LZO_BYTE(m_len - M3_MIN_LEN);


            if (ip >= ip_end)
            {
                ii = ip;
                break;
            }


        /* 2c) Insert phrases (beginning with ii+1) into the dictionary. */
#if (CLEVEL == 9)
        /* Insert the whole match (ii+1)..(ip-1) into dictionary.  */
        /* This is not recommended because it can be slow. */
            ++ii;
            do {
                DVAL_NEXT(dv,ii);
#if 0
                UPDATE_D(dict,drun,dv,ii,in);
#else
                dict[ DINDEX(dv,ii) ] = DENTRY(ii,in);
#endif
                MI
            } while (++ii < ip);
            DVAL_NEXT(dv,ii);
            assert(ii == ip);
            DVAL_ASSERT(dv,ip);
#elif (CLEVEL >= 8)
            SI   DI DI DI DI DI DI DI DI   XI
#elif (CLEVEL >= 7)
            SI   DI DI DI DI DI DI DI      XI
#elif (CLEVEL >= 6)
            SI   DI DI DI DI DI DI         XI
#elif (CLEVEL >= 5)
            SI   DI DI DI DI               XI
#elif (CLEVEL >= 4)
            SI   DI DI DI                  XI
#elif (CLEVEL >= 3)
            SI   DI DI                     XI
#elif (CLEVEL >= 2)
            SI   DI                        XI
#else
                                           XI
#endif
        }

        /* ii now points to the start of the next literal run */
        assert(ii == ip);


/*
vi:ts=4:et
*/
