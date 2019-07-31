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
 * Programmer:  Raymond Lu <songyulu@hdfgroup.org>
 *              April, 2019
 *
 * Purpose:     Private function for external.c and external_env.c
 */
#ifndef _EXTERNAL_COMMON_H
#define _EXTERNAL_COMMON_H

/* Include test header files */
#include "h5test.h"

static const char *EXT_FNAME[] = {
    "extern_1",
    "extern_2",
    "extern_3",
    "extern_4",
    "extern_dir/file_1",
    "extern_5",
    "extern_env_dir/file_2",
    NULL
};

/* A similar collection of files is used for the tests that
 * perform file I/O.
 */
#define N_EXT_FILES         4
#define PART_SIZE           25
#define TOTAL_SIZE          100
#define GARBAGE_PER_FILE    10

herr_t reset_raw_data_files(hbool_t is_env);
#endif /* _EXTERNAL_COMMON_H */
