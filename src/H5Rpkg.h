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

/* Purpose:     This file contains declarations which are visible
 *              only within the H5R package. Source files outside the
 *              H5R package should include H5Rprivate.h instead.
 */
#if !(defined H5R_FRIEND || defined H5R_MODULE)
#error "Do not include this file outside the H5R package!"
#endif

#ifndef _H5Rpkg_H
#define _H5Rpkg_H

/* Get package's private header */
#include "H5Rprivate.h"

/* Other private headers needed by this file */
#include "H5Fprivate.h"         /* Files                                    */
#include "H5Gprivate.h"         /* Groups                                   */
#include "H5Oprivate.h"         /* Object headers                           */
#include "H5Sprivate.h"         /* Dataspaces                               */


/**************************/
/* Package Private Macros */
/**************************/

#define H5R_ENCODE_VERSION    0x1     /* Version for encoding references */

/****************************/
/* Package Private Typedefs */
/****************************/

/* Object reference */
struct href_obj {
    char *filename;             /* File name */
    haddr_t addr;               /* Object address */
};

/* Region reference */
struct href_reg {
    struct href_obj obj;        /* Object reference */
    H5S_t *space;               /* Selection */
};

/* Attribute reference */
struct href_attr {
    struct href_obj obj;        /* Object reference */
    char *name;                 /* Attribute name */
};

/* Generic reference type */
struct href {
    union {
        struct href_obj  obj;   /* Object reference                 */
        struct href_reg  reg;   /* Region reference                 */
        struct href_attr attr;  /* Attribute Reference              */
    } ref;
    hid_t loc_id;               /* Cached location identifier */
    size_t encode_size;         /* Cached encoding size */
    H5R_type_t type;
    char unused[16];
};

/*****************************/
/* Package Private Variables */
/*****************************/


/******************************/
/* Package Private Prototypes */
/******************************/
H5_DLL herr_t   H5R__create_object(haddr_t obj_addr, struct href *ref);
H5_DLL herr_t   H5R__create_region(haddr_t obj_addr, H5S_t *space, struct href *ref);
H5_DLL herr_t   H5R__create_attr(haddr_t obj_addr, const char *attr_name, struct href *ref);
H5_DLL herr_t   H5R__destroy(struct href *ref);

H5_DLL herr_t   H5R__set_loc_id(struct href *ref, hid_t id);
H5_DLL hid_t    H5R__get_loc_id(const struct href *ref);

H5_DLL H5R_type_t   H5R__get_type(const struct href *ref);
H5_DLL htri_t   H5R__equal(const struct href *ref1, const struct href *ref2);
H5_DLL herr_t   H5R__copy(const struct href *src_ref, struct href *dest_ref);

H5_DLL herr_t   H5R__get_obj_addr(const struct href *ref, haddr_t *obj_addr_ptr);
H5_DLL H5S_t *  H5R__get_region(const struct href *ref);

H5_DLL ssize_t  H5R__get_file_name(const struct href *ref, char *name, size_t size);
H5_DLL ssize_t  H5R__get_attr_name(const struct href *ref, char *name, size_t size);

H5_DLL herr_t   H5R__encode(const struct href *ref, unsigned char *buf, size_t *nalloc);
H5_DLL herr_t   H5R__decode(const unsigned char *buf, size_t *nbytes, struct href *ref);

H5_DLL herr_t   H5R__encode_obj_addr(haddr_t obj_addr, unsigned char *buf, size_t *nalloc);
H5_DLL herr_t   H5R__decode_obj_addr(const unsigned char *buf, size_t *nbytes, haddr_t *obj_addr_ptr);

H5_DLL herr_t   H5R__encode_region(H5S_t *space, unsigned char *buf, size_t *nalloc);
H5_DLL H5S_t *  H5R__decode_region(const unsigned char *buf, size_t *nbytes);

H5_DLL herr_t   H5R__encode_heap(H5F_t *f, unsigned char *buf, size_t *nalloc, const unsigned char *data, size_t data_size);
H5_DLL herr_t   H5R__decode_heap(H5F_t *f, const unsigned char *buf, size_t *nbytes, unsigned char **data_ptr, size_t *data_size);
H5_DLL herr_t   H5R__free_heap(H5F_t *f, const unsigned char *buf, size_t nbytes);

H5_DLL herr_t   H5R__encode_obj_addr_compat(haddr_t obj_addr, unsigned char *buf, size_t *nalloc);
H5_DLL herr_t   H5R__decode_obj_addr_compat(const unsigned char *buf, size_t *nbytes, haddr_t *obj_addr_ptr);

H5_DLL herr_t   H5R__encode_addr_region_compat(hid_t loc_id, haddr_t obj_addr, H5S_t *space, unsigned char *buf, size_t *nalloc);
H5_DLL herr_t   H5R__decode_addr_region_compat(hid_t loc_id, const unsigned char *buf, size_t *nbytes, haddr_t *obj_addr_ptr, H5S_t **space_ptr);

#endif /* _H5Rpkg_H */

