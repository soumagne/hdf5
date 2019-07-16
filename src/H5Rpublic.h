/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * This file contains public declarations for the H5R module.
 */
#ifndef _H5Rpublic_H
#define _H5Rpublic_H

/* Public headers needed by this file */
#include "H5public.h"
#include "H5Gpublic.h"
#include "H5Ipublic.h"

/*****************/
/* Public Macros */
/*****************/

/* Note! Be careful with the sizes of the references because they should really
 * depend on the run-time values in the file.
 */
#define H5R_REF_BUF_SIZE        64

/*******************/
/* Public Typedefs */
/*******************/

/*
 * Reference types allowed.
 * DO NOT CHANGE THE ORDER or VALUES as reference type values are encoded into
 * the datatype message header.
 */
typedef enum {
    H5R_BADTYPE     =   (-1),   /* Invalid reference type           */
    H5R_OBJECT1     =     0,    /* Backward compatibility (object)  */
    H5R_DATASET_REGION1 = 1,    /* Backward compatibility (region)  */
    H5R_OBJECT2         = 2,    /* Object reference                 */
    H5R_DATASET_REGION2 = 3,    /* Region reference                 */
    H5R_ATTR            = 4,    /* Attribute Reference              */
    H5R_MAXTYPE         = 5     /* Highest type (invalid)           */
} H5R_type_t;

/**
 * Opaque reference type. The same reference type is used for object,
 * dataset region and attribute references.
 */
typedef unsigned char href_t[H5R_REF_BUF_SIZE];

/********************/
/* Public Variables */
/********************/


/*********************/
/* Public Prototypes */
/*********************/

#ifdef __cplusplus
extern "C" {
#endif

/* Constructors */
H5_DLL herr_t   H5Rcreate_object(hid_t loc_id, const char *name, href_t *ref_ptr);
H5_DLL herr_t   H5Rcreate_region(hid_t loc_id, const char *name, hid_t space_id, href_t *ref_ptr);
H5_DLL herr_t   H5Rcreate_attr(hid_t loc_id, const char *name, const char *attr_name, href_t *ref_ptr);
H5_DLL herr_t   H5Rdestroy(href_t *ref_ptr);

/* Info */
H5_DLL H5R_type_t   H5Rget_type(const href_t *ref_ptr);
H5_DLL htri_t   H5Requal(const href_t *ref1_ptr, const href_t *ref2_ptr);
H5_DLL herr_t   H5Rcopy(const href_t *src_ref_ptr, href_t *dest_ref_ptr);

/* Dereference */
H5_DLL hid_t    H5Ropen_object(const href_t *ref_ptr, hid_t oapl_id);
H5_DLL hid_t    H5Ropen_region(const href_t *ref_ptr);
H5_DLL hid_t    H5Ropen_attr(const href_t *ref_ptr, hid_t aapl_id);

/* Get type */
H5_DLL herr_t   H5Rget_obj_type3(const href_t *ref_ptr, H5O_type_t *obj_type);

/* Get name */
H5_DLL ssize_t  H5Rget_file_name(const href_t *ref_ptr, char *name, size_t size);
H5_DLL ssize_t  H5Rget_obj_name(const href_t *ref_ptr, char *name, size_t size);
H5_DLL ssize_t  H5Rget_attr_name(const href_t *ref_ptr, char *name, size_t size);

/* Serialization */
H5_DLL herr_t   H5Rencode(const href_t *ref_ptr, void *buf, size_t *nalloc);
H5_DLL herr_t   H5Rdecode(hid_t loc_id, const void *buf, href_t *ref_ptr);

/* Symbols defined for compatibility with previous versions of the HDF5 API.
 *
 * Use of these symbols will be deprecated in the future.
 */

/* Macros */

/* Versions for compatibility */
#define H5R_OBJECT          H5R_OBJECT1
#define H5R_DATASET_REGION  H5R_DATASET_REGION1

/* Note! Be careful with the sizes of the references because they should really
 * depend on the run-time values in the file.  Unfortunately, the arrays need
 * to be defined at compile-time, so we have to go with the worst case sizes
 * for them.  -QAK
 */
#define H5R_OBJ_REF_BUF_SIZE        sizeof(haddr_t)

/* 4 is used instead of sizeof(int) to permit portability between the Crays
 * and other machines (the heap ID is always encoded as an int32 anyway).
 */
#define H5R_DSET_REG_REF_BUF_SIZE   (sizeof(haddr_t) + 4)

/* Typedefs */

/* Object reference structure for user's code
 * This needs to be large enough to store largest haddr_t on a worst case
 * machine (8 bytes currently).
 */
typedef haddr_t hobj_ref_t;

/* Dataset Region reference structure for user's code
 * (Buffer to store heap ID and index)
 * This needs to be large enough to store largest haddr_t in a worst case
 * machine (8 bytes currently) plus an int.
 * Note! This type can only be used with the "native" HDF5 VOL connector.
 */
typedef unsigned char hdset_reg_ref_t[H5R_DSET_REG_REF_BUF_SIZE];

/* Function prototypes */
H5_DLL herr_t H5Rcreate(void *ref, hid_t loc_id, const char *name,
                        H5R_type_t ref_type, hid_t space_id);
H5_DLL herr_t H5Rget_obj_type2(hid_t id, H5R_type_t ref_type, const void *ref,
    H5O_type_t *obj_type);
H5_DLL hid_t H5Rdereference2(hid_t obj_id, hid_t oapl_id, H5R_type_t ref_type,
    const void *ref);
H5_DLL hid_t H5Rget_region(hid_t dataset, H5R_type_t ref_type, const void *ref);
H5_DLL ssize_t H5Rget_name(hid_t loc_id, H5R_type_t ref_type, const void *ref,
    char *name, size_t size);

/* Symbols defined for compatibility with previous versions of the HDF5 API.
 *
 * Use of these symbols is deprecated.
 */
#ifndef H5_NO_DEPRECATED_SYMBOLS

/* Function prototypes */
H5_DLL H5G_obj_t H5Rget_obj_type1(hid_t id, H5R_type_t ref_type,
    const void *ref);
H5_DLL hid_t H5Rdereference1(hid_t obj_id, H5R_type_t ref_type,
    const void *ref);

#endif /* H5_NO_DEPRECATED_SYMBOLS */

#ifdef __cplusplus
}
#endif

#endif  /* _H5Rpublic_H */

