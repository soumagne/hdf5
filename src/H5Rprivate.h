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
 * This file contains private information about the H5R module
 */
#ifndef _H5Rprivate_H
#define _H5Rprivate_H

/* Include package's public header */
#include "H5Rpublic.h"

/* Private headers needed by this file */
#include "H5Gprivate.h"

/* Internal data structures */
struct href_t {
    H5R_type_t ref_type;
    union {
        struct {
            size_t buf_size;/* Size of serialized reference */
            void *buf;      /* Pointer to serialized reference */
        } obj_enc;
        haddr_t obj_addr;
    };
};

#define H5R_INITIALIZER { H5R_BADTYPE, {0, NULL} }

/* To prevent including H5Sprivate.h that includes H5Rprivate.h */
struct H5S_t;

/* Private functions */
H5_DLL href_t *H5R_create_object(H5G_loc_t *loc, const char *name, hid_t dxpl_id);
H5_DLL href_t *H5R_create_region(H5G_loc_t *loc, const char *name, hid_t dxpl_id, struct H5S_t *space);
H5_DLL href_t *H5R_create_attr(H5G_loc_t *loc, const char *name, hid_t dxpl_id, const char *attr_name);
H5_DLL href_t *H5R_create_ext_object(H5G_loc_t *loc, const char *name, hid_t dxpl_id);
H5_DLL href_t *H5R_create_ext_region(H5G_loc_t *loc, const char *name, hid_t dxpl_id, struct H5S_t *space);
H5_DLL href_t *H5R_create_ext_attr(H5G_loc_t *loc, const char *name, hid_t dxpl_id, const char *attr_name);
H5_DLL herr_t  H5R_destroy(href_t *ref);

#endif  /* _H5Rprivate_H */
