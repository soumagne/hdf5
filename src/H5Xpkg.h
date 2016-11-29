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

#if !(defined H5X_FRIEND || defined H5X_MODULE)
#error "Do not include this file outside the H5X package!"
#endif

#ifndef _H5Xpkg_H
#define _H5Xpkg_H

/* Include private header file */
#include "H5Xprivate.h" /* Plugin functions                */

/*
 * Dummy plugin
 */
H5_DLLVAR const H5X_class_t H5X_DUMMY[1];

/*
 * ALACRITY plugin
 */
#ifdef H5_HAVE_ALACRITY
H5_DLLVAR const H5X_class_t H5X_ALACRITY[1];
#endif

/*
 * FastBit plugin
 */
#ifdef H5_HAVE_FASTBIT
H5_DLLVAR const H5X_class_t H5X_FASTBIT[1];
#endif

/*
 * Dummy plugin meta
 */
H5_DLLVAR const H5X_class_t H5X_META_DUMMY[1];

#ifdef H5_HAVE_EFF
/*
 * Dummy plugin
 */
H5_DLLVAR const H5X_class_t H5X_DUMMY_FF[1];

/*
 * Meta dummy plugin
 */
H5_DLLVAR const H5X_class_t H5X_META_DUMMY_FF[1];
#endif

#endif /* _H5Xpkg_H */
