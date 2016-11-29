/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
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
 * This file contains function prototypes for each exported function in the
 * H5Q module.
 */
#ifndef _H5Qpublic_H
#define _H5Qpublic_H

/* Public headers needed by this file */
#include "H5public.h"
#include "H5Ipublic.h"

/*****************/
/* Public Macros */
/*****************/
#define H5Q_REF_REG   (H5CHECK 0x100u)   /* region references are present */
#define H5Q_REF_OBJ   (H5CHECK 0x010u)   /* object references are present */
#define H5Q_REF_ATTR  (H5CHECK 0x001u)   /* attribute references are present */

#define H5Q_VIEW_REF_REG_NAME   "Reg_refs"  /* region references */
#define H5Q_VIEW_REF_OBJ_NAME   "Obj_refs"  /* object references */
#define H5Q_VIEW_REF_ATTR_NAME  "Attr_refs" /* attribute references */

/*******************/
/* Public Typedefs */
/*******************/

/* Query type */
typedef enum H5Q_type_t {
    H5Q_TYPE_DATA_ELEM,  /* selects data elements */
    H5Q_TYPE_ATTR_VALUE, /* selects attribute values */
    H5Q_TYPE_ATTR_NAME,  /* selects attributes */
    H5Q_TYPE_LINK_NAME,  /* selects objects */
    H5Q_TYPE_MISC        /* (for combine queries) selects misc objects */
} H5Q_type_t;

/* Query match conditions */
typedef enum H5Q_match_op_t {
    H5Q_MATCH_EQUAL,        /* equal */
    H5Q_MATCH_NOT_EQUAL,    /* not equal */
    H5Q_MATCH_LESS_THAN,    /* less than */
    H5Q_MATCH_GREATER_THAN  /* greater than */
} H5Q_match_op_t;

/* Query combine operators */
typedef enum H5Q_combine_op_t {
    H5Q_COMBINE_AND,
    H5Q_COMBINE_OR,
    H5Q_SINGLETON
} H5Q_combine_op_t;

/********************/
/* Public Variables */
/********************/

/*********************/
/* Public Prototypes */
/*********************/
#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
H5_DLL hid_t H5Qcreate(H5Q_type_t query_type, H5Q_match_op_t match_op, ...);
H5_DLL herr_t H5Qclose(hid_t query_id);
H5_DLL hid_t H5Qcombine(hid_t query1_id, H5Q_combine_op_t combine_op, hid_t query2_id);
H5_DLL herr_t H5Qget_type(hid_t query_id, H5Q_type_t *query_type);
H5_DLL herr_t H5Qget_match_op(hid_t query_id, H5Q_match_op_t *match_op);
H5_DLL herr_t H5Qget_components(hid_t query_id, hid_t *sub_query1_id, hid_t *sub_query2_id);
H5_DLL herr_t H5Qget_combine_op(hid_t query_id, H5Q_combine_op_t *op_type);

/* Encode / decode */
H5_DLL herr_t H5Qencode(hid_t query_id, void *buf, size_t *nalloc);
H5_DLL hid_t H5Qdecode(const void *buf);

/* Apply query */
H5_DLL herr_t H5Qapply_atom(hid_t query_id, hbool_t *result, ...);
H5_DLL hid_t H5Qapply(hid_t loc_id, hid_t query_id, unsigned *result, hid_t vcpl_id);
H5_DLL hid_t H5Qapply_multi(size_t loc_count, hid_t *loc_ids, hid_t query_id,
    unsigned *result, hid_t vcpl_id);

#ifdef __cplusplus
}
#endif
#endif /* _H5Qpublic_H */
