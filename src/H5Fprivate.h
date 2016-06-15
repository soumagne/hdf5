/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * This file contains macros & information for file access
 */

#ifndef _H5Fprivate_H
#define _H5Fprivate_H

/* Include package's public header */
#include "H5Fpublic.h"

/* Public headers needed by this file */
#include "H5FDpublic.h"		/* File drivers				*/

/* Private headers needed by this file */
#ifdef H5_HAVE_PARALLEL
#include "H5Pprivate.h"		/* Property lists			*/
#endif /* H5_HAVE_PARALLEL */
#include "H5VMprivate.h"		/* Vectors and arrays */


/**************************/
/* Library Private Macros */
/**************************/

/*
 * Encode and decode macros for file meta-data.
 * Currently, all file meta-data is little-endian.
 */

#  define INT16ENCODE(p, i) {						      \
   *(p) = (uint8_t)( (unsigned)(i)	 & 0xff); (p)++;		      \
   *(p) = (uint8_t)(((unsigned)(i) >> 8) & 0xff); (p)++;		      \
}

#  define UINT16ENCODE(p, i) {						      \
   *(p) = (uint8_t)( (unsigned)(i)	 & 0xff); (p)++;		      \
   *(p) = (uint8_t)(((unsigned)(i) >> 8) & 0xff); (p)++;		      \
}

#  define INT32ENCODE(p, i) {						      \
   *(p) = (uint8_t)( (uint32_t)(i)	  & 0xff); (p)++;		      \
   *(p) = (uint8_t)(((uint32_t)(i) >>  8) & 0xff); (p)++;		      \
   *(p) = (uint8_t)(((uint32_t)(i) >> 16) & 0xff); (p)++;		      \
   *(p) = (uint8_t)(((uint32_t)(i) >> 24) & 0xff); (p)++;		      \
}

#  define UINT32ENCODE(p, i) {						      \
   *(p) = (uint8_t)( (i)        & 0xff); (p)++;				      \
   *(p) = (uint8_t)(((i) >>  8) & 0xff); (p)++;				      \
   *(p) = (uint8_t)(((i) >> 16) & 0xff); (p)++;				      \
   *(p) = (uint8_t)(((i) >> 24) & 0xff); (p)++;				      \
}

/* Encode an unsigned integer into a variable-sized buffer */
/* (Assumes that the high bits of the integer are zero) */
#  define ENCODE_VAR(p, typ, n, l) {					      \
   typ _n = (n);							      \
   size_t _i;								      \
   uint8_t *_p = (uint8_t*)(p);						      \
									      \
   for(_i = 0; _i < l; _i++, _n >>= 8)					      \
      *_p++ = (uint8_t)(_n & 0xff);					      \
   (p) = (uint8_t*)(p) + l;						      \
}

/* Encode a 32-bit unsigned integer into a variable-sized buffer */
/* (Assumes that the high bits of the integer are zero) */
#  define UINT32ENCODE_VAR(p, n, l)     ENCODE_VAR(p, uint32_t, n, l)

#  define INT64ENCODE(p, n) {						      \
   int64_t _n = (n);							      \
   size_t _i;								      \
   uint8_t *_p = (uint8_t*)(p);						      \
									      \
   for (_i = 0; _i < sizeof(int64_t); _i++, _n >>= 8)			      \
      *_p++ = (uint8_t)(_n & 0xff);					      \
   for (/*void*/; _i < 8; _i++)						      \
      *_p++ = (n) < 0 ? 0xff : 0;					      \
   (p) = (uint8_t*)(p)+8;						      \
}

#  define UINT64ENCODE(p, n) {						      \
   uint64_t _n = (n);							      \
   size_t _i;								      \
   uint8_t *_p = (uint8_t*)(p);						      \
									      \
   for (_i = 0; _i < sizeof(uint64_t); _i++, _n >>= 8)			      \
      *_p++ = (uint8_t)(_n & 0xff);					      \
   for (/*void*/; _i < 8; _i++)						      \
      *_p++ = 0;							      \
   (p) = (uint8_t*)(p) + 8;						      \
}

/* Encode a 64-bit unsigned integer into a variable-sized buffer */
/* (Assumes that the high bits of the integer are zero) */
#  define UINT64ENCODE_VAR(p, n, l)     ENCODE_VAR(p, uint64_t, n, l)

/* Encode a 64-bit unsigned integer and its length into a variable-sized buffer */
/* (Assumes that the high bits of the integer are zero) */
#  define UINT64ENCODE_VARLEN(p, n) {					      \
   uint64_t __n = (uint64_t)(n);							      \
   unsigned _s = H5VM_limit_enc_size(__n);				      \
									      \
   *(p)++ = (uint8_t)_s;						      \
   UINT64ENCODE_VAR(p, __n, _s);						      \
}

#  define H5_ENCODE_UNSIGNED(p, n) {					      \
    HDcompile_assert(sizeof(unsigned) == sizeof(uint32_t));		      \
    UINT32ENCODE(p, n)							      \
}

/* Assumes the endianness of uint64_t is the same as double */
#  define H5_ENCODE_DOUBLE(p, n) {					      \
    uint64_t _n;							      \
    size_t _u;								      \
    uint8_t *_p = (uint8_t*)(p);					      \
									      \
    HDcompile_assert(sizeof(double) == 8);				      \
    HDcompile_assert(sizeof(double) == sizeof(uint64_t));		      \
    HDmemcpy(&_n, &n, sizeof(double));					      \
    for(_u = 0; _u < sizeof(uint64_t); _u++, _n >>= 8)			      \
        *_p++ = (uint8_t)(_n & 0xff);					      \
    (p) = (uint8_t *)(p) + 8;						      \
}

/* DECODE converts little endian bytes pointed by p to integer values and store
 * it in i.  For signed values, need to do sign-extension when converting
 * the last byte which carries the sign bit.
 * The macros does not require i be of a certain byte sizes.  It just requires
 * i be big enough to hold the intended value range.  E.g. INT16DECODE works
 * correctly even if i is actually a 64bit int like in a Cray.
 */

#  define INT16DECODE(p, i) {						      \
   (i)	= (int16_t)((*(p) & 0xff));      (p)++;				      \
   (i) |= (int16_t)(((*(p) & 0xff) << 8) |                                    \
                   ((*(p) & 0x80) ? ~0xffff : 0x0)); (p)++;		      \
}

#  define UINT16DECODE(p, i) {						      \
   (i)	= (uint16_t) (*(p) & 0xff);	  (p)++;			      \
   (i) |= (uint16_t)((*(p) & 0xff) << 8); (p)++;			      \
}

#  define INT32DECODE(p, i) {						      \
   (i)	= ((int32_t)(*(p) & (unsigned)0xff));	  (p)++;		      \
   (i) |= ((int32_t)(*(p) & (unsigned)0xff) <<  8); (p)++;		      \
   (i) |= ((int32_t)(*(p) & (unsigned)0xff) << 16); (p)++;		      \
   (i) |= ((int32_t)(((*(p) & (unsigned)0xff) << 24) |                        \
                   ((*(p) & (unsigned)0x80) ? (unsigned)(~0xffffffff) : (unsigned)0x0))); (p)++; \
}

#  define UINT32DECODE(p, i) {						      \
   (i)	=  (uint32_t)(*(p) & 0xff);	   (p)++;			      \
   (i) |= ((uint32_t)(*(p) & 0xff) <<  8); (p)++;			      \
   (i) |= ((uint32_t)(*(p) & 0xff) << 16); (p)++;			      \
   (i) |= ((uint32_t)(*(p) & 0xff) << 24); (p)++;			      \
}

/* Decode a variable-sized buffer */
/* (Assumes that the high bits of the integer will be zero) */
#  define DECODE_VAR(p, n, l) {						      \
   size_t _i;								      \
									      \
   n = 0;								      \
   (p) += l;								      \
   for (_i = 0; _i < l; _i++)						      \
      n = (n << 8) | *(--p);						      \
   (p) += l;								      \
}

/* Decode a variable-sized buffer into a 32-bit unsigned integer */
/* (Assumes that the high bits of the integer will be zero) */
#  define UINT32DECODE_VAR(p, n, l)     DECODE_VAR(p, n, l)

#  define INT64DECODE(p, n) {						      \
   /* WE DON'T CHECK FOR OVERFLOW! */					      \
   size_t _i;								      \
									      \
   n = 0;								      \
   (p) += 8;								      \
   for (_i = 0; _i < sizeof(int64_t); _i++)					      \
      n = (n << 8) | *(--p);						      \
   (p) += 8;								      \
}

#  define UINT64DECODE(p, n) {						      \
   /* WE DON'T CHECK FOR OVERFLOW! */					      \
   size_t _i;								      \
									      \
   n = 0;								      \
   (p) += 8;								      \
   for (_i = 0; _i < sizeof(uint64_t); _i++)				      \
      n = (n << 8) | *(--p);						      \
   (p) += 8;								      \
}

/* Decode a variable-sized buffer into a 64-bit unsigned integer */
/* (Assumes that the high bits of the integer will be zero) */
#  define UINT64DECODE_VAR(p, n, l)     DECODE_VAR(p, n, l)

/* Decode a 64-bit unsigned integer and its length from a variable-sized buffer */
/* (Assumes that the high bits of the integer will be zero) */
#  define UINT64DECODE_VARLEN(p, n) {					      \
   unsigned _s = *(p)++;						      \
									      \
   UINT64DECODE_VAR(p, n, _s);						      \
}

#  define H5_DECODE_UNSIGNED(p, n) {					      \
    HDcompile_assert(sizeof(unsigned) == sizeof(uint32_t));		      \
    UINT32DECODE(p, n)							      \
}

/* Assumes the endianness of uint64_t is the same as double */
#  define H5_DECODE_DOUBLE(p, n) {					      \
    uint64_t _n;							      \
    size_t _u;								      \
									      \
    HDcompile_assert(sizeof(double) == 8);				      \
    HDcompile_assert(sizeof(double) == sizeof(uint64_t));		      \
    _n = 0;								      \
    (p) += 8;								      \
    for(_u = 0; _u < sizeof(uint64_t); _u++)				      \
        _n = (_n << 8) | *(--p);					      \
    HDmemcpy(&(n), &_n, sizeof(double));					      \
    (p) += 8;								      \
}

/* Address-related macros */
#define H5F_addr_overflow(X,Z)	(HADDR_UNDEF==(X) ||			      \
				 HADDR_UNDEF==(X)+(haddr_t)(Z) ||	      \
				 (X)+(haddr_t)(Z)<(X))
#define H5F_addr_defined(X)	((X)!=HADDR_UNDEF)
/* The H5F_addr_eq() macro guarantees that Y is not HADDR_UNDEF by making
 * certain that X is not HADDR_UNDEF and then checking that X equals Y
 */
#define H5F_addr_eq(X,Y)	((X)!=HADDR_UNDEF &&			      \
				 (X)==(Y))
#define H5F_addr_ne(X,Y)	(!H5F_addr_eq((X),(Y)))
#define H5F_addr_lt(X,Y) 	((X)!=HADDR_UNDEF &&			      \
				 (Y)!=HADDR_UNDEF &&			      \
				 (X)<(Y))
#define H5F_addr_le(X,Y)	((X)!=HADDR_UNDEF &&			      \
				 (Y)!=HADDR_UNDEF &&			      \
				 (X)<=(Y))
#define H5F_addr_gt(X,Y)	((X)!=HADDR_UNDEF &&			      \
				 (Y)!=HADDR_UNDEF &&			      \
				 (X)>(Y))
#define H5F_addr_ge(X,Y)	((X)!=HADDR_UNDEF &&			      \
				 (Y)!=HADDR_UNDEF &&			      \
				 (X)>=(Y))
#define H5F_addr_cmp(X,Y)	(H5F_addr_eq((X), (Y)) ? 0 :		      \
				 (H5F_addr_lt((X), (Y)) ? -1 : 1))
#define H5F_addr_pow2(N)	((haddr_t)1<<(N))
#define H5F_addr_overlap(O1,L1,O2,L2) (((O1) < (O2) && ((O1) + (L1)) > (O2)) || \
                                 ((O1) >= (O2) && (O1) < ((O2) + (L2))))

/* If the module using this macro is allowed access to the private variables, access them directly */
#ifdef H5F_MODULE
#define H5F_INTENT(F)           ((F)->shared->flags)
#define H5F_OPEN_NAME(F)        ((F)->open_name)
#define H5F_ACTUAL_NAME(F)      ((F)->actual_name)
#define H5F_EXTPATH(F)          ((F)->extpath)
#define H5F_SHARED(F)           ((F)->shared)
#define H5F_SAME_SHARED(F1, F2) ((F1)->shared == (F2)->shared)
#define H5F_NOPEN_OBJS(F)       ((F)->nopen_objs)
#define H5F_INCR_NOPEN_OBJS(F)  ((F)->nopen_objs++)
#define H5F_DECR_NOPEN_OBJS(F)  ((F)->nopen_objs--)
#define H5F_FILE_ID(F)          ((F)->file_id)
#define H5F_PARENT(F)           ((F)->parent)
#define H5F_NMOUNTS(F)          ((F)->nmounts)
#define H5F_DRIVER_ID(F)        ((F)->shared->lf->driver_id)
#define H5F_GET_FILENO(F,FILENUM) ((FILENUM) = (F)->shared->lf->fileno)
#define H5F_HAS_FEATURE(F,FL)   ((F)->shared->lf->feature_flags & (FL))
#define H5F_BASE_ADDR(F)        ((F)->shared->sblock->base_addr)
#define H5F_SYM_LEAF_K(F)       ((F)->shared->sblock->sym_leaf_k)
#define H5F_KVALUE(F,T)         ((F)->shared->sblock->btree_k[(T)->id])
#define H5F_NREFS(F)		((F)->shared->nrefs)
#define H5F_SIZEOF_ADDR(F)      ((F)->shared->sizeof_addr)
#define H5F_SIZEOF_SIZE(F)      ((F)->shared->sizeof_size)
#define H5F_SOHM_ADDR(F)        ((F)->shared->sohm_addr)
#define H5F_SET_SOHM_ADDR(F, A) ((F)->shared->sohm_addr = (A))
#define H5F_SOHM_VERS(F)        ((F)->shared->sohm_vers)
#define H5F_SET_SOHM_VERS(F, V) ((F)->shared->sohm_vers = (V))
#define H5F_SOHM_NINDEXES(F)    ((F)->shared->sohm_nindexes)
#define H5F_SET_SOHM_NINDEXES(F, N) ((F)->shared->sohm_nindexes = (N))
#define H5F_FCPL(F)             ((F)->shared->fcpl_id)
#define H5F_GET_FC_DEGREE(F)    ((F)->shared->fc_degree)
#define H5F_EVICT_ON_CLOSE(F)   ((F)->shared->evict_on_close)
#define H5F_RDCC_NSLOTS(F)      ((F)->shared->rdcc_nslots)
#define H5F_RDCC_NBYTES(F)      ((F)->shared->rdcc_nbytes)
#define H5F_RDCC_W0(F)          ((F)->shared->rdcc_w0)
#define H5F_SIEVE_BUF_SIZE(F)   ((F)->shared->sieve_buf_size)
#define H5F_GC_REF(F)           ((F)->shared->gc_ref)
#define H5F_USE_LATEST_FLAGS(F,FL)  ((F)->shared->latest_flags & (FL))
#define H5F_STORE_MSG_CRT_IDX(F)    ((F)->shared->store_msg_crt_idx)
#define H5F_SET_STORE_MSG_CRT_IDX(F, FL)    ((F)->shared->store_msg_crt_idx = (FL))
#define H5F_GRP_BTREE_SHARED(F) ((F)->shared->grp_btree_shared)
#define H5F_SET_GRP_BTREE_SHARED(F, RC) (((F)->shared->grp_btree_shared = (RC)) ? SUCCEED : FAIL)
#define H5F_USE_TMP_SPACE(F)    ((F)->shared->use_tmp_space)
#define H5F_IS_TMP_ADDR(F, ADDR) (H5F_addr_le((F)->shared->tmp_addr, (ADDR)))
#ifdef H5_HAVE_PARALLEL
#define H5F_COLL_MD_READ(F)     ((F)->coll_md_read)
#endif /* H5_HAVE_PARALLEL */
#else /* H5F_MODULE */
#define H5F_INTENT(F)           (H5F_get_intent(F))
#define H5F_OPEN_NAME(F)        (H5F_get_open_name(F))
#define H5F_ACTUAL_NAME(F)      (H5F_get_actual_name(F))
#define H5F_EXTPATH(F)          (H5F_get_extpath(F))
#define H5F_SHARED(F)           (H5F_get_shared(F))
#define H5F_SAME_SHARED(F1, F2) (H5F_same_shared((F1), (F2)))
#define H5F_NOPEN_OBJS(F)       (H5F_get_nopen_objs(F))
#define H5F_INCR_NOPEN_OBJS(F)  (H5F_incr_nopen_objs(F))
#define H5F_DECR_NOPEN_OBJS(F)  (H5F_decr_nopen_objs(F))
#define H5F_FILE_ID(F)          (H5F_get_file_id(F))
#define H5F_PARENT(F)           (H5F_get_parent(F))
#define H5F_NMOUNTS(F)          (H5F_get_nmounts(F))
#define H5F_DRIVER_ID(F)        (H5F_get_driver_id(F))
#define H5F_GET_FILENO(F,FILENUM) (H5F_get_fileno((F), &(FILENUM)))
#define H5F_HAS_FEATURE(F,FL)   (H5F_has_feature(F,FL))
#define H5F_BASE_ADDR(F)        (H5F_get_base_addr(F))
#define H5F_SYM_LEAF_K(F)       (H5F_sym_leaf_k(F))
#define H5F_KVALUE(F,T)         (H5F_Kvalue(F,T))
#define H5F_NREFS(F)		(H5F_get_nrefs(F))
#define H5F_SIZEOF_ADDR(F)      (H5F_sizeof_addr(F))
#define H5F_SIZEOF_SIZE(F)      (H5F_sizeof_size(F))
#define H5F_SOHM_ADDR(F)        (H5F_get_sohm_addr(F))
#define H5F_SET_SOHM_ADDR(F, A) (H5F_set_sohm_addr((F), (A)))
#define H5F_SOHM_VERS(F)        (H5F_get_sohm_vers(F))
#define H5F_SET_SOHM_VERS(F, V) (H5F_set_sohm_vers((F), (V)))
#define H5F_SOHM_NINDEXES(F)    (H5F_get_sohm_nindexes(F))
#define H5F_SET_SOHM_NINDEXES(F, N) (H5F_set_sohm_nindexes((F), (N)))
#define H5F_FCPL(F)             (H5F_get_fcpl(F))
#define H5F_GET_FC_DEGREE(F)    (H5F_get_fc_degree(F))
#define H5F_EVICT_ON_CLOSE(F)   (H5F_get_evict_on_close(F))
#define H5F_RDCC_NSLOTS(F)      (H5F_rdcc_nslots(F))
#define H5F_RDCC_NBYTES(F)      (H5F_rdcc_nbytes(F))
#define H5F_RDCC_W0(F)          (H5F_rdcc_w0(F))
#define H5F_SIEVE_BUF_SIZE(F)   (H5F_sieve_buf_size(F))
#define H5F_GC_REF(F)           (H5F_gc_ref(F))
#define H5F_USE_LATEST_FLAGS(F,FL) (H5F_use_latest_flags(F,FL))
#define H5F_STORE_MSG_CRT_IDX(F) (H5F_store_msg_crt_idx(F))
#define H5F_SET_STORE_MSG_CRT_IDX(F, FL)    (H5F_set_store_msg_crt_idx((F), (FL)))
#define H5F_GRP_BTREE_SHARED(F) (H5F_grp_btree_shared(F))
#define H5F_SET_GRP_BTREE_SHARED(F, RC) (H5F_set_grp_btree_shared((F), (RC)))
#define H5F_USE_TMP_SPACE(F)    (H5F_use_tmp_space(F))
#define H5F_IS_TMP_ADDR(F, ADDR) (H5F_is_tmp_addr((F), (ADDR)))
#ifdef H5_HAVE_PARALLEL
#define H5F_COLL_MD_READ(F)     (H5F_coll_md_read(F))
#endif /* H5_HAVE_PARALLEL */
#endif /* H5F_MODULE */


/* Macros to encode/decode offset/length's for storing in the file */
#define H5F_ENCODE_OFFSET(f,p,o) switch(H5F_SIZEOF_ADDR(f)) {		      \
    case 4: UINT32ENCODE(p,o); break;					      \
    case 8: UINT64ENCODE(p,o); break;					      \
    case 2: UINT16ENCODE(p,o); break;					      \
}

#define H5F_DECODE_OFFSET(f,p,o) switch (H5F_SIZEOF_ADDR (f)) {		      \
    case 4: UINT32DECODE(p, o); break;					      \
    case 8: UINT64DECODE(p, o); break;					      \
    case 2: UINT16DECODE(p, o); break;					      \
}

#define H5F_ENCODE_LENGTH_LEN(p,l,s) switch(s) {		      	      \
    case 4: UINT32ENCODE(p,l); break;					      \
    case 8: UINT64ENCODE(p,l); break;					      \
    case 2: UINT16ENCODE(p,l); break;					      \
    default: HDassert("bad sizeof size" && 0);				      \
}

#define H5F_ENCODE_LENGTH(f,p,l) H5F_ENCODE_LENGTH_LEN(p,l,H5F_SIZEOF_SIZE(f))

#define H5F_DECODE_LENGTH_LEN(p,l,s) switch(s) {		              \
    case 4: UINT32DECODE(p,l); break;					      \
    case 8: UINT64DECODE(p,l); break;					      \
    case 2: UINT16DECODE(p,l); break;					      \
    default: HDassert("bad sizeof size" && 0);				      \
}

#define H5F_DECODE_LENGTH(f,p,l) H5F_DECODE_LENGTH_LEN(p,l,H5F_SIZEOF_SIZE(f))

/*
 * Macros that check for overflows.  These are somewhat dangerous to fiddle
 * with.
 */
#if (H5_SIZEOF_SIZE_T >= H5_SIZEOF_OFF_T)
#   define H5F_OVERFLOW_SIZET2OFFT(X)					      \
    ((size_t)(X)>=(size_t)((size_t)1<<(8*sizeof(HDoff_t)-1)))
#else
#   define H5F_OVERFLOW_SIZET2OFFT(X) 0
#endif
#if (H5_SIZEOF_HSIZE_T >= H5_SIZEOF_OFF_T)
#   define H5F_OVERFLOW_HSIZET2OFFT(X)					      \
    ((hsize_t)(X)>=(hsize_t)((hsize_t)1<<(8*sizeof(HDoff_t)-1)))
#else
#   define H5F_OVERFLOW_HSIZET2OFFT(X) 0
#endif

/* Sizes of object addresses & sizes in the file (in bytes) */
#define H5F_OBJ_ADDR_SIZE   sizeof(haddr_t)
#define H5F_OBJ_SIZE_SIZE   sizeof(hsize_t)

/* File-wide default character encoding can not yet be set via the file
 * creation property list and is always ASCII. */
#define H5F_DEFAULT_CSET H5T_CSET_ASCII

/* ========= File Creation properties ============ */
#define H5F_CRT_USER_BLOCK_NAME      "block_size"       /* Size of the file user block in bytes */
#define H5F_CRT_SYM_LEAF_NAME        "symbol_leaf"      /* 1/2 rank for symbol table leaf nodes */
#define H5F_CRT_SYM_LEAF_DEF         4
#define H5F_CRT_BTREE_RANK_NAME      "btree_rank"       /* 1/2 rank for btree internal nodes    */
#define H5F_CRT_ADDR_BYTE_NUM_NAME   "addr_byte_num"    /* Byte number in an address            */
#define H5F_CRT_OBJ_BYTE_NUM_NAME    "obj_byte_num"     /* Byte number for object size          */
#define H5F_CRT_SUPER_VERS_NAME      "super_version"    /* Version number of the superblock     */
#define H5F_CRT_SHMSG_NINDEXES_NAME  "num_shmsg_indexes" /* Number of shared object header message indexes */
#define H5F_CRT_SHMSG_INDEX_TYPES_NAME "shmsg_message_types" /* Types of message in each index */
#define H5F_CRT_SHMSG_INDEX_MINSIZE_NAME "shmsg_message_minsize" /* Minimum size of messages in each index */
#define H5F_CRT_SHMSG_LIST_MAX_NAME  "shmsg_list_max"   /* Shared message list maximum size */
#define H5F_CRT_SHMSG_BTREE_MIN_NAME "shmsg_btree_min"  /* Shared message B-tree minimum size */
#define H5F_CRT_FILE_SPACE_STRATEGY_NAME "file_space_strategy"  /* File space handling strategy */
#define H5F_CRT_FREE_SPACE_THRESHOLD_NAME "free_space_threshold"  /* Free space section threshold */



/* ========= File Access properties ============ */
#define H5F_ACS_META_CACHE_INIT_CONFIG_NAME	"mdc_initCacheCfg" /* Initial metadata cache resize configuration */
#define H5F_ACS_DATA_CACHE_NUM_SLOTS_NAME       "rdcc_nslots"   /* Size of raw data chunk cache(slots) */
#define H5F_ACS_DATA_CACHE_BYTE_SIZE_NAME       "rdcc_nbytes"   /* Size of raw data chunk cache(bytes) */
#define H5F_ACS_PREEMPT_READ_CHUNKS_NAME        "rdcc_w0"       /* Preemption read chunks first */
#define H5F_ACS_ALIGN_THRHD_NAME                "threshold"     /* Threshold for alignment */
#define H5F_ACS_ALIGN_NAME                      "align"         /* Alignment */
#define H5F_ACS_META_BLOCK_SIZE_NAME            "meta_block_size" /* Minimum metadata allocation block size (when aggregating metadata allocations) */
#define H5F_ACS_SIEVE_BUF_SIZE_NAME             "sieve_buf_size" /* Maximum sieve buffer size (when data sieving is allowed by file driver) */
#define H5F_ACS_SDATA_BLOCK_SIZE_NAME           "sdata_block_size" /* Minimum "small data" allocation block size (when aggregating "small" raw data allocations) */
#define H5F_ACS_GARBG_COLCT_REF_NAME            "gc_ref"        /* Garbage-collect references */
#define H5F_ACS_FILE_DRV_NAME                   "driver-id/info" /* File driver ID & info */
#define H5F_ACS_CLOSE_DEGREE_NAME		"close_degree"  /* File close degree */
#define H5F_ACS_FAMILY_OFFSET_NAME              "family_offset" /* Offset position in file for family file driver */
#define H5F_ACS_FAMILY_NEWSIZE_NAME             "family_newsize" /* New member size of family driver.  (private property only used by h5repart) */
#define H5F_ACS_FAMILY_TO_SEC2_NAME             "family_to_sec2" /* Whether to convert family to sec2 driver.  (private property only used by h5repart) */
#define H5F_ACS_MULTI_TYPE_NAME                 "multi_type"    /* Data type in multi file driver */
#define H5F_ACS_LATEST_FORMAT_NAME              "latest_format" /* 'Use latest format version' flag */
#define H5F_ACS_WANT_POSIX_FD_NAME              "want_posix_fd" /* Internal: query the file descriptor from the core VFD, instead of the memory address */
#define H5F_ACS_OBJECT_FLUSH_CB_NAME     	"object_flush_cb" 	 /* Object flush callback */
#define H5F_ACS_EFC_SIZE_NAME                   "efc_size"      /* Size of external file cache */
#define H5F_ACS_FILE_IMAGE_INFO_NAME            "file_image_info" /* struct containing initial file image and callback info */
#define H5F_ACS_CORE_WRITE_TRACKING_FLAG_NAME       "core_write_tracking_flag" /* Whether or not core VFD backing store write tracking is enabled */
#define H5F_ACS_EVICT_ON_CLOSE_FLAG_NAME        "evict_on_close_flag" /* Whether or not the metadata cache will evict objects on close */
#define H5F_ACS_CORE_WRITE_TRACKING_PAGE_SIZE_NAME  "core_write_tracking_page_size" /* The page size in kiB when core VFD write tracking is enabled */
#define H5F_ACS_COLL_MD_WRITE_FLAG_NAME         "collective_metadata_write" /* property indicating whether metadata writes are done collectively or not */

/* ======================== File Mount properties ====================*/
#define H5F_MNT_SYM_LOCAL_NAME 		"local"                 /* Whether absolute symlinks local to file. */


#ifdef H5_HAVE_PARALLEL
/* Which process writes metadata */
#define H5_PAR_META_WRITE 0
#endif /* H5_HAVE_PARALLEL */

/* Define the HDF5 file signature */
#define H5F_SIGNATURE	  "\211HDF\r\n\032\n"
#define H5F_SIGNATURE_LEN 8

/* Version #'s of the major components of the file format */
#define HDF5_SUPERBLOCK_VERSION_DEF	0	/* The default super block format	  */
#define HDF5_SUPERBLOCK_VERSION_1	1	/* Version with non-default B-tree 'K' value */
#define HDF5_SUPERBLOCK_VERSION_2	2	/* Revised version with superblock extension and checksum */
#define HDF5_SUPERBLOCK_VERSION_LATEST	HDF5_SUPERBLOCK_VERSION_2	/* The maximum super block format    */
#define HDF5_SUPERBLOCK_VERSION_V18_LATEST HDF5_SUPERBLOCK_VERSION_2    /* The latest superblock version for v18 */
#define HDF5_FREESPACE_VERSION	        0	/* of the Free-Space Info	  */
#define HDF5_OBJECTDIR_VERSION	        0	/* of the Object Directory format */
#define HDF5_SHAREDHEADER_VERSION       0	/* of the Shared-Header Info	  */
#define HDF5_DRIVERINFO_VERSION_0       0	/* of the Driver Information Block*/

/* B-tree internal 'K' values */
#define HDF5_BTREE_SNODE_IK_DEF         16
#define HDF5_BTREE_CHUNK_IK_DEF         32      /* Note! this value is assumed
                                                    to be 32 for version 0
                                                    of the superblock and
                                                    if it is changed, the code
                                                    must compensate. -QAK
                                                 */
#define HDF5_BTREE_IK_MAX_ENTRIES       65536 	/* 2^16 - 2 bytes for storing entries (children) */
						/* See format specification on version 1 B-trees */

/* Default file space handling strategy */
#define H5F_FILE_SPACE_STRATEGY_DEF	        H5F_FILE_SPACE_ALL
/* Default free space section threshold used by free-space managers */
#define H5F_FREE_SPACE_THRESHOLD_DEF	        1

/* Macros to define signatures of all objects in the file */

/* Size of signature information (on disk) */
/* (all on-disk signatures should be this length) */
#define H5_SIZEOF_MAGIC                 4

/* v1 B-tree node signature */
#define H5B_MAGIC	                "TREE"

/* v2 B-tree signatures */
#define H5B2_HDR_MAGIC                  "BTHD"          /* Header */
#define H5B2_INT_MAGIC                  "BTIN"          /* Internal node */
#define H5B2_LEAF_MAGIC                 "BTLF"          /* Leaf node */

/* Extensible array signatures */
#define H5EA_HDR_MAGIC                  "EAHD"          /* Header */
#define H5EA_IBLOCK_MAGIC               "EAIB"          /* Index block */
#define H5EA_SBLOCK_MAGIC               "EASB"          /* Super block */
#define H5EA_DBLOCK_MAGIC               "EADB"          /* Data block */

/* Fixed array signatures */
#define H5FA_HDR_MAGIC                  "FAHD"          /* Header */
#define H5FA_DBLOCK_MAGIC               "FADB"          /* Data block */

/* Free space signatures */
#define H5FS_HDR_MAGIC                  "FSHD"          /* Header */
#define H5FS_SINFO_MAGIC                "FSSE"          /* Serialized sections */

/* Symbol table node signature */
#define H5G_NODE_MAGIC                  "SNOD"

/* Fractal heap signatures */
#define H5HF_HDR_MAGIC                  "FRHP"          /* Header */
#define H5HF_IBLOCK_MAGIC               "FHIB"          /* Indirect block */
#define H5HF_DBLOCK_MAGIC               "FHDB"          /* Direct block */

/* Global heap signature */
#define H5HG_MAGIC	                "GCOL"

/* Local heap signature */
#define H5HL_MAGIC                      "HEAP"

/* Object header signatures */
#define H5O_HDR_MAGIC                   "OHDR"          /* Header */
#define H5O_CHK_MAGIC                   "OCHK"          /* Continuation chunk */

/* Shared Message signatures */
#define H5SM_TABLE_MAGIC                "SMTB"          /* Shared Message Table */
#define H5SM_LIST_MAGIC                 "SMLI"          /* Shared Message List */


/* Latest format will activate the following latest version support */
/* "latest_flags" in H5F_file_t */
#define H5F_LATEST_DATATYPE             0x0001
#define H5F_LATEST_DATASPACE            0x0002
#define H5F_LATEST_ATTRIBUTE            0x0004
#define H5F_LATEST_FILL_MSG             0x0008
#define H5F_LATEST_PLINE_MSG            0x0010
#define H5F_LATEST_LAYOUT_MSG           0x0020
#define H5F_LATEST_NO_MOD_TIME_MSG      0x0040
#define H5F_LATEST_STYLE_GROUP          0x0080
#define H5F_LATEST_OBJ_HEADER           0x0100
#define H5F_LATEST_SUPERBLOCK           0x0200
#define H5F_LATEST_ALL_FLAGS            (H5F_LATEST_DATATYPE | H5F_LATEST_DATASPACE | H5F_LATEST_ATTRIBUTE | H5F_LATEST_FILL_MSG | H5F_LATEST_PLINE_MSG | H5F_LATEST_LAYOUT_MSG | H5F_LATEST_NO_MOD_TIME_MSG | H5F_LATEST_STYLE_GROUP | H5F_LATEST_OBJ_HEADER | H5F_LATEST_SUPERBLOCK)

#define H5F_LATEST_DSET_MSG_FLAGS    (H5F_LATEST_FILL_MSG | H5F_LATEST_PLINE_MSG | H5F_LATEST_LAYOUT_MSG)

/****************************/
/* Library Private Typedefs */
/****************************/

/* Forward declarations (for prototypes & type definitions) */
struct H5B_class_t;
struct H5UC_t;
struct H5O_loc_t;
struct H5HG_heap_t;
struct H5P_genplist_t;

/* Forward declarations for anonymous H5F objects */

/* Main file structures */
typedef struct H5F_t H5F_t;
typedef struct H5F_file_t H5F_file_t;

/* Block aggregation structure */
typedef struct H5F_blk_aggr_t H5F_blk_aggr_t;

/* Structure for object flush callback property (H5Pset_object_flush_cb)*/
typedef struct H5F_object_flush_t {
    H5F_flush_cb_t func;        /* The callback function */
    void *udata;                /* User data */
} H5F_object_flush_t;

/* I/O Info for an operation */
typedef struct H5F_io_info_t {
    const H5F_t *f;                     /* File object */
    const struct H5P_genplist_t *dxpl;         /* DXPL object */
} H5F_io_info_t;

/* Concise info about a block of bytes in a file */
typedef struct H5F_block_t {
    haddr_t offset;             /* Offset of the block in the file */
    hsize_t length;             /* Length of the block in the file */
} H5F_block_t;


/*****************************/
/* Library-private Variables */
/*****************************/


/***************************************/
/* Library-private Function Prototypes */
/***************************************/


/* Private functions */
H5_DLL H5F_t *H5F_open(const char *name, unsigned flags, hid_t fcpl_id,
    hid_t fapl_id, hid_t dxpl_id);
H5_DLL herr_t H5F_try_close(H5F_t *f);

/* Functions than retrieve values from the file struct */
H5_DLL unsigned H5F_get_intent(const H5F_t *f);
H5_DLL char *H5F_get_open_name(const H5F_t *f);
H5_DLL char *H5F_get_actual_name(const H5F_t *f);
H5_DLL char *H5F_get_extpath(const H5F_t *f);
H5_DLL H5F_file_t *H5F_get_shared(const H5F_t *f);
H5_DLL hbool_t H5F_same_shared(const H5F_t *f1, const H5F_t *f2);
H5_DLL unsigned H5F_get_nopen_objs(const H5F_t *f);
H5_DLL unsigned H5F_incr_nopen_objs(H5F_t *f);
H5_DLL unsigned H5F_decr_nopen_objs(H5F_t *f);
H5_DLL hid_t H5F_get_file_id(const H5F_t *f);
H5_DLL H5F_t *H5F_get_parent(const H5F_t *f);
H5_DLL unsigned H5F_get_nmounts(const H5F_t *f);
H5_DLL hid_t H5F_get_access_plist(H5F_t *f, hbool_t app_ref);
H5_DLL hid_t H5F_get_id(H5F_t *file, hbool_t app_ref);
H5_DLL herr_t H5F_get_obj_count(const H5F_t *f, unsigned types, hbool_t app_ref, size_t *obj_id_count_ptr);
H5_DLL herr_t H5F_get_obj_ids(const H5F_t *f, unsigned types, size_t max_objs, hid_t *oid_list, hbool_t app_ref, size_t *obj_id_count_ptr);

/* Functions than retrieve values set/cached from the superblock/FCPL */
H5_DLL haddr_t H5F_get_base_addr(const H5F_t *f);
H5_DLL unsigned H5F_sym_leaf_k(const H5F_t *f);
H5_DLL unsigned H5F_Kvalue(const H5F_t *f, const struct H5B_class_t *type);
H5_DLL unsigned H5F_get_nrefs(const H5F_t *f);
H5_DLL uint8_t H5F_sizeof_addr(const H5F_t *f);
H5_DLL uint8_t H5F_sizeof_size(const H5F_t *f);
H5_DLL haddr_t H5F_get_sohm_addr(const H5F_t *f);
H5_DLL herr_t H5F_set_sohm_addr(H5F_t *f, haddr_t addr);
H5_DLL unsigned H5F_get_sohm_vers(const H5F_t *f);
H5_DLL herr_t H5F_set_sohm_vers(H5F_t *f, unsigned vers);
H5_DLL unsigned H5F_get_sohm_nindexes(const H5F_t *f);
H5_DLL herr_t H5F_set_sohm_nindexes(H5F_t *f, unsigned nindexes);
H5_DLL hid_t H5F_get_fcpl(const H5F_t *f);
H5_DLL H5F_close_degree_t H5F_get_fc_degree(const H5F_t *f);
H5_DLL hbool_t H5F_get_evict_on_close(const H5F_t *f);
H5_DLL size_t H5F_rdcc_nbytes(const H5F_t *f);
H5_DLL size_t H5F_rdcc_nslots(const H5F_t *f);
H5_DLL double H5F_rdcc_w0(const H5F_t *f);
H5_DLL size_t H5F_sieve_buf_size(const H5F_t *f);
H5_DLL unsigned H5F_gc_ref(const H5F_t *f);
H5_DLL unsigned H5F_use_latest_flags(const H5F_t *f, unsigned fl);
H5_DLL hbool_t H5F_store_msg_crt_idx(const H5F_t *f);
H5_DLL herr_t H5F_set_store_msg_crt_idx(H5F_t *f, hbool_t flag);
H5_DLL struct H5UC_t *H5F_grp_btree_shared(const H5F_t *f);
H5_DLL herr_t H5F_set_grp_btree_shared(H5F_t *f, struct H5UC_t *rc);
H5_DLL hbool_t H5F_use_tmp_space(const H5F_t *f);
H5_DLL hbool_t H5F_is_tmp_addr(const H5F_t *f, haddr_t addr);
#ifdef H5_HAVE_PARALLEL
H5_DLL H5P_coll_md_read_flag_t H5F_coll_md_read(const H5F_t *f);
H5_DLL void H5F_set_coll_md_read(H5F_t *f, H5P_coll_md_read_flag_t flag);
#endif /* H5_HAVE_PARALLEL */

/* Functions that retrieve values from VFD layer */
H5_DLL hid_t H5F_get_driver_id(const H5F_t *f);
H5_DLL herr_t H5F_get_fileno(const H5F_t *f, unsigned long *filenum);
H5_DLL hbool_t H5F_has_feature(const H5F_t *f, unsigned feature);
H5_DLL haddr_t H5F_get_eoa(const H5F_t *f, H5FD_mem_t type);
H5_DLL herr_t H5F_get_vfd_handle(const H5F_t *file, hid_t fapl, void **file_handle);

/* Functions that check file mounting information */
H5_DLL hbool_t H5F_is_mount(const H5F_t *file);
H5_DLL hbool_t H5F_has_mount(const H5F_t *file);
H5_DLL herr_t H5F_traverse_mount(struct H5O_loc_t *oloc/*in,out*/);
H5_DLL herr_t H5F_flush_mounts(H5F_t *f, hid_t dxpl_id);

/* Functions that operate on blocks of bytes wrt super block */
H5_DLL herr_t H5F_block_read(const H5F_t *f, H5FD_mem_t type, haddr_t addr,
                size_t size, hid_t dxpl_id, void *buf/*out*/);
H5_DLL herr_t H5F_block_write(const H5F_t *f, H5FD_mem_t type, haddr_t addr,
                size_t size, hid_t dxpl_id, const void *buf);

/* Functions that flush or evict */
H5_DLL herr_t H5F_flush_tagged_metadata(H5F_t * f, haddr_t tag, hid_t dxpl_id);
H5_DLL herr_t H5F_evict_tagged_metadata(H5F_t * f, haddr_t tag, hid_t dxpl_id);

/* Routine to invoke callback function upon object flush */
H5_DLL herr_t H5F_object_flush_cb(H5F_t *f, hid_t obj_id);

/* Address-related functions */
H5_DLL void H5F_addr_encode(const H5F_t *f, uint8_t **pp, haddr_t addr);
H5_DLL void H5F_addr_encode_len(size_t addr_len, uint8_t **pp, haddr_t addr);
H5_DLL void H5F_addr_decode(const H5F_t *f, const uint8_t **pp, haddr_t *addr_p);
H5_DLL void H5F_addr_decode_len(size_t addr_len, const uint8_t **pp, haddr_t *addr_p);

/* Shared file list related routines */
H5_DLL void H5F_sfile_assert_num(unsigned n);

/* Routines for creating & destroying "fake" file structures */
H5_DLL H5F_t *H5F_fake_alloc(uint8_t sizeof_size);
H5_DLL herr_t H5F_fake_free(H5F_t *f);

/* Superblock related routines */
H5_DLL herr_t H5F_super_dirty(H5F_t *f);

/* Parallel I/O (i.e. MPI) related routines */
#ifdef H5_HAVE_PARALLEL
H5_DLL herr_t H5F_get_mpi_handle(const H5F_t *f, MPI_File **f_handle);
H5_DLL int H5F_mpi_get_rank(const H5F_t *f);
H5_DLL MPI_Comm H5F_mpi_get_comm(const H5F_t *f);
H5_DLL int H5F_mpi_get_size(const H5F_t *f);
H5_DLL herr_t H5F_mpi_retrieve_comm(hid_t loc_id, hid_t acspl_id, MPI_Comm *mpi_comm);
#endif /* H5_HAVE_PARALLEL */

/* External file cache routines */
H5_DLL H5F_t *H5F_efc_open(H5F_t *parent, const char *name, unsigned flags,
    hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id);
H5_DLL herr_t H5F_efc_close(H5F_t *parent, H5F_t *file);

/* Global heap CWFS routines */
H5_DLL herr_t H5F_cwfs_add(H5F_t *f, struct H5HG_heap_t *heap);
H5_DLL herr_t H5F_cwfs_find_free_heap(H5F_t *f, hid_t dxpl_id, size_t need, haddr_t *addr);
H5_DLL herr_t H5F_cwfs_advance_heap(H5F_t *f, struct H5HG_heap_t *heap,
    hbool_t add_heap);
H5_DLL herr_t H5F_cwfs_remove_heap(H5F_file_t *shared, struct H5HG_heap_t *heap);

/* Debugging functions */
H5_DLL herr_t H5F_debug(H5F_t *f, FILE * stream, int indent, int fwidth);

#endif /* _H5Fprivate_H */

