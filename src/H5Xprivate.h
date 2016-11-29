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
 * This file contains private information about the H5X module
 */
#ifndef _H5Xprivate_H
#define _H5Xprivate_H

/* Include package's public header */
#include "H5Xpublic.h"

/* Private headers needed by this file */

/**************************/
/* Library Private Macros */
/**************************/

/* ========  Index creation property names ======== */
#define H5X_CRT_READ_ON_CREATE_NAME "read_on_create" /* Read existing data when creating index */

/* ========  Index access property names ======== */

/* ======== Index transfer properties ======== */


/****************************/
/* Library Private Typedefs */
/****************************/


/*****************************/
/* Library Private Variables */
/*****************************/

/******************************/
/* Library Private Prototypes */
/******************************/
H5_DLL herr_t H5X_init(void);

H5_DLL H5X_class_t *H5X_registered(unsigned plugin_id);
H5_DLL herr_t H5X_register(const H5X_class_t *index_plugin);
H5_DLL herr_t H5X_unregister(unsigned plugin_id);

H5_DLL herr_t H5X_create(hid_t dset_id, unsigned plugin_id, hid_t xcpl_id);
H5_DLL herr_t H5X_remove(hid_t dset_id, unsigned plugin_id);
H5_DLL herr_t H5X_can_create(hid_t dset_id, hid_t dcpl_id);

H5_DLL herr_t H5X_get_count(hid_t dset_id, hsize_t *idx_count);

H5_DLL herr_t H5X_get_size(hid_t dset_id, hsize_t *idx_size);

#endif /* _H5Xprivate_H */
