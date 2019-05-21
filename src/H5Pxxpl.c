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

/*-------------------------------------------------------------------------
 *
 * Created:		H5Pxxpl.c
 *			February 2014
 *
 * Purpose:		Index transfer property list class routines
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#include "H5Pmodule.h"      /* This source code file is part of the H5P module */


/***********/
/* Headers */
/***********/
#include "H5private.h"      /* Generic Functions    */
#include "H5Eprivate.h"     /* Error handling       */
#include "H5Iprivate.h"     /* IDs                  */
#include "H5Sprivate.h"     /* Dataspaces           */
#include "H5Xprivate.h"     /* Index                */
#include "H5Ppkg.h"         /* Property lists       */


/****************/
/* Local Macros */
/****************/

/* ========  Index transfer properties ======== */

/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/

/* Property class callbacks */
static herr_t H5P__xxfr_reg_prop(H5P_genclass_t *pclass);

/*********************/
/* Package Variables */
/*********************/

/* Index access property list class library initialization object */
const H5P_libclass_t H5P_CLS_XXFR[1] = {{
    "index transfer",           /* Class name for debugging     */
    H5P_TYPE_INDEX_XFER,        /* Class type                   */
    &H5P_CLS_ROOT_g,            /* Parent class                 */
    &H5P_CLS_INDEX_XFER_g,      /* Pointer to class             */
    &H5P_CLS_INDEX_XFER_ID_g,   /* Pointer to class ID          */
    &H5P_LST_INDEX_XFER_ID_g,   /* Pointer to default property list ID */
    H5P__xxfr_reg_prop,         /* Default property registration routine */
    NULL,                       /* Class creation callback      */
    NULL,                       /* Class creation callback info */
    NULL,                       /* Class copy callback          */
    NULL,                       /* Class copy callback info     */
    NULL,                       /* Class close callback         */
    NULL                        /* Class close callback info    */
}};

/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/

/*-------------------------------------------------------------------------
 * Function:    H5P__xxfr_reg_prop
 *
 * Purpose:     Register the index transfer property list class's
 *              properties
 *
 * Return:      Non-negative on success/Negative on failure
 *-------------------------------------------------------------------------
 */
static herr_t
H5P__xxfr_reg_prop(H5P_genclass_t H5_ATTR_UNUSED *pclass)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P__xxfr_reg_prop() */
