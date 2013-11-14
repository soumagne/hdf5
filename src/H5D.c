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

/****************/
/* Module Setup */
/****************/

#include "H5Dmodule.h"          /* This source code file is part of the H5D module */


/***********/
/* Headers */
/***********/
#include "H5private.h"          /* Generic Functions                        */
#include "H5CXprivate.h"        /* API Contexts                             */
#include "H5Dpkg.h"             /* Datasets                                 */
#include "H5Eprivate.h"         /* Error handling                           */
#include "H5FLprivate.h"        /* Free lists                               */
#include "H5Iprivate.h"         /* IDs                                      */
#include "H5VLprivate.h"        /* Virtual Object Layer                     */

#include "H5VLnative_private.h" /* Native VOL connector                     */


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Local Prototypes */
/********************/


/*********************/
/* Package Variables */
/*********************/

/* Package initialization variable */
hbool_t H5_PKG_INIT_VAR = FALSE;

/* Declare extern the free list to manage blocks of VL data */
H5FL_BLK_EXTERN(vlen_vl_buf);

/* Declare extern the free list to manage other blocks of VL data */
H5FL_BLK_EXTERN(vlen_fl_buf);


/*****************************/
/* Library Private Variables */
/*****************************/

/* Declare extern the free list to manage blocks of type conversion data */
H5FL_BLK_EXTERN(type_conv);


/*******************/
/* Local Variables */
/*******************/



/*-------------------------------------------------------------------------
 * Function:    H5Dcreate2
 *
 * Purpose:     Creates a new dataset named NAME at LOC_ID, opens the
 *              dataset for access, and associates with that dataset constant
 *              and initial persistent properties including the type of each
 *              datapoint as stored in the file (TYPE_ID), the size of the
 *              dataset (SPACE_ID), and other initial miscellaneous
 *              properties (DCPL_ID).
 *
 *              All arguments are copied into the dataset, so the caller is
 *              allowed to derive new types, dataspaces, and creation
 *              parameters from the old ones and reuse them in calls to
 *              create other datasets.
 *
 * Return:      Success:    The object ID of the new dataset. At this
 *                          point, the dataset is ready to receive its
 *                          raw data. Attempting to read raw data from
 *                          the dataset will probably return the fill
 *                          value. The dataset should be closed when the
 *                          caller is no longer interested in it.
 *
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dcreate2(hid_t loc_id, const char *name, hid_t type_id, hid_t space_id,
    hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id)
{
    void               *dset = NULL;                    /* New dataset's info */
    H5VL_object_t      *vol_obj = NULL;                 /* object token of loc_id */
    H5VL_loc_params_t   loc_params;
    hid_t               ret_value = H5I_INVALID_HID;    /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE7("i", "i*siiiii", loc_id, name, type_id, space_id, lcpl_id, dcpl_id,
             dapl_id);

    /* Check arguments */
    if(!name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "name parameter cannot be NULL")
    if(!*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "name parameter cannot be an empty string")

    /* Get link creation property list */
    if(H5P_DEFAULT == lcpl_id)
        lcpl_id = H5P_LINK_CREATE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(lcpl_id, H5P_LINK_CREATE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "lcpl_id is not a link creation property list")

    /* Get dataset creation property list */
    if(H5P_DEFAULT == dcpl_id)
        dcpl_id = H5P_DATASET_CREATE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(dcpl_id, H5P_DATASET_CREATE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "dcpl_id is not a dataset create property list ID")

    /* Set the DCPL for the API context */
    H5CX_set_dcpl(dcpl_id);

    /* Verify access property list and set up collective metadata if appropriate */
    if(H5CX_set_apl(&dapl_id, H5P_CLS_DACC, loc_id, TRUE) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, H5I_INVALID_HID, "can't set access property list info")

    /* Get the location object */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid location identifier")

    /* Set location parameters */
    loc_params.type         = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type     = H5I_get_type(loc_id);

    /* Create the dataset */
    if(NULL == (dset = H5VL_dataset_create(vol_obj, &loc_params, name, lcpl_id, type_id, space_id, dcpl_id, dapl_id, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL)))
        HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, H5I_INVALID_HID, "unable to create dataset")

    /* Get an atom for the dataset */
    if((ret_value = H5VL_register(H5I_DATASET, dset, vol_obj->connector, TRUE)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to atomize dataset handle")

    /* Keep ID of the dataset */
    dset->shared->dset_id = ret_value;

    /* Create index if told to */
    if(H5X_can_create_data(ret_value, dcpl_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTCREATE, FAIL, "Index can't be created on this dataset")

done:
    if(H5I_INVALID_HID == ret_value)
        if(dset && H5VL_dataset_close(vol_obj, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL) < 0)
            HDONE_ERROR(H5E_DATASET, H5E_CLOSEERROR, H5I_INVALID_HID, "unable to release dataset")

    FUNC_LEAVE_API(ret_value)
} /* end H5Dcreate2() */


/*-------------------------------------------------------------------------
 * Function:    H5Dcreate_anon
 *
 * Purpose:     Creates a new dataset named NAME at LOC_ID, opens the
 *              dataset for access, and associates with that dataset constant
 *              and initial persistent properties including the type of each
 *              datapoint as stored in the file (TYPE_ID), the size of the
 *              dataset (SPACE_ID), and other initial miscellaneous
 *              properties (DCPL_ID).
 *
 *              All arguments are copied into the dataset, so the caller is
 *              allowed to derive new types, dataspaces, and creation
 *              parameters from the old ones and reuse them in calls to
 *              create other datasets.
 *
 *              The resulting ID should be linked into the file with
 *              H5Olink or it will be deleted when closed.
 *
 * Return:      Success:    The object ID of the new dataset. At this
 *                          point, the dataset is ready to receive its
 *                          raw data. Attempting to read raw data from
 *                          the dataset will probably return the fill
 *                          value. The dataset should be linked into
 *                          the group hierarchy before being closed or
 *                          it will be deleted. The dataset should be
 *                          closed when the caller is no longer interested
 *                          in it.
 *
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dcreate_anon(hid_t loc_id, hid_t type_id, hid_t space_id, hid_t dcpl_id,
    hid_t dapl_id)
{
    void                *dset       = NULL;             /* dset token from VOL connector */
    H5VL_object_t       *vol_obj    = NULL;             /* object token of loc_id */
    H5VL_loc_params_t   loc_params;
    hid_t               ret_value   = H5I_INVALID_HID;  /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE5("i", "iiiii", loc_id, type_id, space_id, dcpl_id, dapl_id);

    /* Check arguments */
    if(H5P_DEFAULT == dcpl_id)
        dcpl_id = H5P_DATASET_CREATE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(dcpl_id, H5P_DATASET_CREATE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "not dataset create property list ID")

    /* Set the DCPL for the API context */
    H5CX_set_dcpl(dcpl_id);

    /* Verify access property list and set up collective metadata if appropriate */
    if(H5CX_set_apl(&dapl_id, H5P_CLS_DACC, loc_id, TRUE) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, H5I_INVALID_HID, "can't set access property list info")

    /* get the location object */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid location identifier")

    /* Set location parameters */
    loc_params.type     = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Create the dataset */
    if(NULL == (dset = H5VL_dataset_create(vol_obj, &loc_params, NULL, H5P_LINK_CREATE_DEFAULT, type_id, space_id, dcpl_id, dapl_id, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL)))
        HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, H5I_INVALID_HID, "unable to create dataset")

    /* Get an atom for the dataset */
    if((ret_value = H5VL_register(H5I_DATASET, dset, vol_obj->connector, TRUE)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register dataset")

done:
    /* Cleanup on failure */
    if(H5I_INVALID_HID == ret_value)
        if(dset && H5VL_dataset_close(vol_obj, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL) < 0)
            HDONE_ERROR(H5E_DATASET, H5E_CLOSEERROR, H5I_INVALID_HID, "unable to release dataset")

    FUNC_LEAVE_API(ret_value)
} /* end H5Dcreate_anon() */


/*-------------------------------------------------------------------------
 * Function:    H5Dopen2
 *
 * Purpose:     Finds a dataset named NAME at LOC_ID, opens it, and returns
 *              its ID. The dataset should be close when the caller is no
 *              longer interested in it.
 *
 *              Takes a dataset access property list
 *
 * Return:      Success:    Object ID of the dataset
 *
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dopen2(hid_t loc_id, const char *name, hid_t dapl_id)
{
    void               *dset = NULL;        /* dset token from VOL connector */
    H5VL_object_t      *vol_obj = NULL;         /* object token of loc_id */
    H5VL_loc_params_t   loc_params;
    hid_t               ret_value   = H5I_INVALID_HID;  /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE3("i", "i*si", loc_id, name, dapl_id);

    /* Check args */
    if(!name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "name parameter cannot be NULL")
    if(!*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "name parameter cannot be an empty string")

    /* Verify access property list and set up collective metadata if appropriate */
    if(H5CX_set_apl(&dapl_id, H5P_CLS_DACC, loc_id, FALSE) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, H5I_INVALID_HID, "can't set access property list info")

    /* get the location object */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid location identifier")

    /* Set the location parameters */
    loc_params.type         = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type     = H5I_get_type(loc_id);

    /* Open the dataset */
    if(NULL == (dset = H5VL_dataset_open(vol_obj, &loc_params, name, dapl_id, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL)))
        HGOTO_ERROR(H5E_DATASET, H5E_CANTOPENOBJ, H5I_INVALID_HID, "unable to open dataset")

    /* Register an atom for the dataset */
    if((ret_value = H5VL_register(H5I_DATASET, dset, vol_obj->connector, TRUE)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTREGISTER, H5I_INVALID_HID, "can't register dataset atom")

    /* Keep ID of the dataset */
    dset->shared->dset_id = ret_value;

done:
    if(H5I_INVALID_HID == ret_value)
        if(dset && H5VL_dataset_close(vol_obj, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL) < 0)
            HDONE_ERROR(H5E_DATASET, H5E_CLOSEERROR, H5I_INVALID_HID, "unable to release dataset")

    FUNC_LEAVE_API(ret_value)
} /* end H5Dopen2() */


/*-------------------------------------------------------------------------
 * Function:    H5Dclose
 *
 * Purpose:     Closes access to a dataset and releases resources used by
 *              it. It is illegal to subsequently use that same dataset
 *              ID in calls to other dataset functions.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dclose(hid_t dset_id)
{
    herr_t  ret_value = SUCCEED;    /* Return value                     */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "i", dset_id);

    /* Check args */
    if(H5I_DATASET != H5I_get_type(dset_id))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset ID")

    /* Decrement the counter on the dataset.  It will be freed if the count
     * reaches zero.  
     */
    if(H5I_dec_app_ref_always_close(dset_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTDEC, FAIL, "can't decrement count on dataset ID")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dclose() */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_space
 *
 * Purpose:     Returns a copy of the file dataspace for a dataset.
 *
 * Return:      Success:    ID for a copy of the dataspace.  The data
 *                          space should be released by calling
 *                          H5Sclose().
 *
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_space(hid_t dset_id)
{
    H5VL_object_t  *vol_obj = NULL;                    /* Dataset structure    */
    hid_t           ret_value = H5I_INVALID_HID;    /* Return value         */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE1("i", "i", dset_id);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid dataset identifier")

    /* Get the dataspace */
    if(H5VL_dataset_get(vol_obj, H5VL_DATASET_GET_SPACE, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, H5I_INVALID_HID, "unable to get dataspace")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_space() */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_space_status
 *
 * Purpose:     Returns the status of dataspace allocation.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dget_space_status(hid_t dset_id, H5D_space_status_t *allocation)
{
    H5VL_object_t  *vol_obj = NULL;                    /* Dataset structure    */
    herr_t          ret_value = SUCCEED;            /* Return value         */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*Ds", dset_id, allocation);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")

    /* Get dataspace status */
    if((ret_value = H5VL_dataset_get(vol_obj, H5VL_DATASET_GET_SPACE_STATUS, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, allocation)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "unable to get space status")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Dget_space_status() */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_type
 *
 * Purpose:     Returns a copy of the file datatype for a dataset.
 *
 * Return:      Success:    ID for a copy of the datatype. The data
 *                          type should be released by calling
 *                          H5Tclose().
 *
 *              Failure:    H5I_INVALID_HID
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_type(hid_t dset_id)
{
    H5VL_object_t  *vol_obj;                           /* Dataset structure    */
    hid_t           ret_value = H5I_INVALID_HID;    /* Return value         */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE1("i", "i", dset_id);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid dataset identifier")

    /* Get the datatype */
    if(H5VL_dataset_get(vol_obj, H5VL_DATASET_GET_TYPE, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, H5I_INVALID_HID, "unable to get datatype")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_type() */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_create_plist
 *
 * Purpose:     Returns a copy of the dataset creation property list.
 *
 * Return:      Success:    ID for a copy of the dataset creation
 *                          property list.  The template should be
 *                          released by calling H5P_close().
 *
 *              Failure:    H5I_INVALID_HID
 *
 * Programmer:	Robb Matzke
 *		Tuesday, February  3, 1998
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_create_plist(hid_t dset_id)
{
    H5VL_object_t  *vol_obj;                           /* Dataset structure    */
    hid_t           ret_value = H5I_INVALID_HID;    /* Return value         */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE1("i", "i", dset_id);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid dataset identifier")

    /* Get the dataset creation property list */
    if(H5VL_dataset_get(vol_obj, H5VL_DATASET_GET_DCPL, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, H5I_INVALID_HID, "unable to get dataset creation properties")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_create_plist() */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_access_plist
 *
 * Purpose:     Returns a copy of the dataset access property list.
 *
 * Description: H5Dget_access_plist returns the dataset access property
 *              list identifier of the specified dataset.
 *
 *              The chunk cache parameters in the returned property lists will be
 *              those used by the dataset.  If the properties in the file access
 *              property list were used to determine the dataset’s chunk cache
 *              configuration, then those properties will be present in the
 *              returned dataset access property list.  If the dataset does not
 *              use a chunked layout, then the chunk cache properties will be set
 *              to the default.  The chunk cache properties in the returned list
 *              are considered to be “set”, and any use of this list will override
 *              the corresponding properties in the file’s file access property
 *              list.
 *
 *              All link access properties in the returned list will be set to the
 *              default values.
 *
 * Return:      Success:    ID for a copy of the dataset access
 *                          property list.  The template should be
 *                          released by calling H5Pclose().
 *
 *              Failure:    H5I_INVALID_HID
 *
 * Programmer:	Neil Fortner
 *		Wednesday, October 29, 2008
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_access_plist(hid_t dset_id)
{
    H5VL_object_t  *vol_obj;                           /* Dataset structure    */
    hid_t           ret_value = H5I_INVALID_HID;    /* Return value         */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE1("i", "i", dset_id);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid dataset identifier")

    /* Get the dataset access property list */
    if(H5VL_dataset_get(vol_obj, H5VL_DATASET_GET_DAPL, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, H5I_INVALID_HID, "unable to get dataset access properties")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_access_plist() */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_storage_size
 *
 * Purpose:     Returns the amount of storage that is required for the
 *              dataset. For chunked datasets this is the number of allocated
 *              chunks times the chunk size.
 *
 * Return:      Success:    The amount of storage space allocated for the
 *                          dataset, not counting meta data. The return
 *                          value may be zero if no data has been stored.
 *
 *              Failure:    Zero
 *
 *-------------------------------------------------------------------------
 */
hsize_t
H5Dget_storage_size(hid_t dset_id)
{
    H5VL_object_t  *vol_obj;                           /* Dataset for this operation   */
    hsize_t         ret_value = 0;                  /* Return value                 */

    FUNC_ENTER_API(0)
    H5TRACE1("h", "i", dset_id);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, 0, "invalid dataset identifier")

    /* Get the storage size */
    if(H5VL_dataset_get(vol_obj, H5VL_DATASET_GET_STORAGE_SIZE, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, 0, "unable to get storage size")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_storage_size() */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_offset
 *
 * Purpose:     Returns the address of dataset in file.
 *
 * Return:      Success:    The address of dataset
 *
 *              Failure:    HADDR_UNDEF (can also be a valid return value!)
 *
 *-------------------------------------------------------------------------
 */
haddr_t
H5Dget_offset(hid_t dset_id)
{
    H5VL_object_t  *vol_obj;                           /* Dataset for this operation   */
    haddr_t         ret_value = HADDR_UNDEF;        /* Return value                 */

    FUNC_ENTER_API(HADDR_UNDEF)
    H5TRACE1("a", "i", dset_id);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, HADDR_UNDEF, "invalid dataset identifier")

    /* Get the offset */
    if(H5VL_dataset_get(vol_obj, H5VL_DATASET_GET_OFFSET, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, HADDR_UNDEF, "unable to get offset")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_offset() */


/*-------------------------------------------------------------------------
 * Function:	H5Diterate
 *
 * Purpose:	This routine iterates over all the elements selected in a memory
 *      buffer.  The callback function is called once for each element selected
 *      in the dataspace.  The selection in the dataspace is modified so
 *      that any elements already iterated over are removed from the selection
 *      if the iteration is interrupted (by the H5D_operator_t function
 *      returning non-zero) in the "middle" of the iteration and may be
 *      re-started by the user where it left off.
 *
 *      NOTE: Until "subtracting" elements from a selection is implemented,
 *          the selection is not modified.
 *
 * Parameters:
 *      void *buf;          IN/OUT: Pointer to the buffer in memory containing
 *                              the elements to iterate over.
 *      hid_t type_id;      IN: Datatype ID for the elements stored in BUF.
 *      hid_t space_id;     IN: Dataspace ID for BUF, also contains the
 *                              selection to iterate over.
 *      H5D_operator_t op; IN: Function pointer to the routine to be
 *                              called for each element in BUF iterated over.
 *      void *operator_data;    IN/OUT: Pointer to any user-defined data
 *                              associated with the operation.
 *
 * Operation information:
 *      H5D_operator_t is defined as:
 *          typedef herr_t (*H5D_operator_t)(void *elem, hid_t type_id,
 *              unsigned ndim, const hsize_t *point, void *operator_data);
 *
 *      H5D_operator_t parameters:
 *          void *elem;         IN/OUT: Pointer to the element in memory containing
 *                                  the current point.
 *          hid_t type_id;      IN: Datatype ID for the elements stored in ELEM.
 *          unsigned ndim;       IN: Number of dimensions for POINT array
 *          const hsize_t *point; IN: Array containing the location of the element
 *                                  within the original dataspace.
 *          void *operator_data;    IN/OUT: Pointer to any user-defined data
 *                                  associated with the operation.
 *
 *      The return values from an operator are:
 *          Zero causes the iterator to continue, returning zero when all
 *              elements have been processed.
 *          Positive causes the iterator to immediately return that positive
 *              value, indicating short-circuit success.  The iterator can be
 *              restarted at the next element.
 *          Negative causes the iterator to immediately return that value,
 *              indicating failure. The iterator can be restarted at the next
 *              element.
 *
 * Return:	Returns the return value of the last operator if it was non-zero,
 *          or zero if all elements were processed. Otherwise returns a
 *          negative value.
 *
 * Programmer:	Quincey Koziol
 *              Friday, June 11, 1999
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Diterate(void *buf, hid_t type_id, hid_t space_id, H5D_operator_t op,
        void *operator_data)
{
    H5T_t *type;                /* Datatype */
    H5S_t *space;               /* Dataspace for iteration */
    H5S_sel_iter_op_t dset_op;  /* Operator for iteration */
    herr_t ret_value;           /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE5("e", "*xiix*x", buf, type_id, space_id, op, operator_data);

    /* Check args */
    if(NULL == op)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid operator")
    if(NULL == buf)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid buffer")
    if(H5I_DATATYPE != H5I_get_type(type_id))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid datatype")
    if(NULL == (type = (H5T_t *)H5I_object_verify(type_id, H5I_DATATYPE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not an valid base datatype")
    if(NULL == (space = (H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataspace")
    if(!(H5S_has_extent(space)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "dataspace does not have extent set")

    dset_op.op_type = H5S_SEL_ITER_OP_APP;
    dset_op.u.app_op.op = op;
    dset_op.u.app_op.type_id = type_id;

    ret_value = H5S_select_iterate(buf, type, space, &dset_op, operator_data);

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Diterate() */


/*-------------------------------------------------------------------------
 * Function:	H5Dvlen_reclaim
 *
 * Purpose:	Frees the buffers allocated for storing variable-length data
 *      in memory.  Only frees the VL data in the selection defined in the
 *      dataspace.  The dataset transfer property list is required to find the
 *      correct allocation/free methods for the VL data in the buffer.
 *
 * Return:	Non-negative on success, negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Thursday, June 10, 1999
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dvlen_reclaim(hid_t type_id, hid_t space_id, hid_t dxpl_id, void *buf)
{
    H5S_t *space;               /* Dataspace for iteration */
    herr_t ret_value;           /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iii*x", type_id, space_id, dxpl_id, buf);

    /* Check args */
    if(H5I_DATATYPE != H5I_get_type(type_id) || buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid argument")
    if(NULL == (space = (H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataspace")
    if(!(H5S_has_extent(space)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "dataspace does not have extent set")

    /* Get the default dataset transfer property list if the user didn't provide one */
    if(H5P_DEFAULT == dxpl_id)
        dxpl_id = H5P_DATASET_XFER_DEFAULT;
    else
        if(TRUE != H5P_isa_class(dxpl_id, H5P_DATASET_XFER))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not xfer parms")

    /* Set DXPL for operation */
    H5CX_set_dxpl(dxpl_id);

    /* Call internal routine */
    ret_value = H5D_vlen_reclaim(type_id, space, buf);

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Dvlen_reclaim() */


/*-------------------------------------------------------------------------
 * Function:	H5Dvlen_get_buf_size
 *
 * Purpose:	This routine checks the number of bytes required to store the VL
 *      data from the dataset, using the space_id for the selection in the
 *      dataset on disk and the type_id for the memory representation of the
 *      VL data, in memory.  The *size value is modified according to how many
 *      bytes are required to store the VL data in memory.
 *
 * Implementation: This routine actually performs the read with a custom
 *      memory manager which basically just counts the bytes requested and
 *      uses a temporary memory buffer (through the H5FL API) to make certain
 *      enough space is available to perform the read.  Then the temporary
 *      buffer is released and the number of bytes allocated is returned.
 *      Kinda kludgy, but easier than the other method of trying to figure out
 *      the sizes without actually reading the data in... - QAK
 *
 * Return:	Non-negative on success, negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Wednesday, August 11, 1999
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dvlen_get_buf_size(hid_t dataset_id, hid_t type_id, hid_t space_id,
        hsize_t *size)
{
    H5D_vlen_bufsize_t vlen_bufsize = {0, 0, 0, 0, 0, 0};
    H5VL_object_t  *vol_obj;       /* Dataset for this operation */
    H5S_t *mspace = NULL;       /* Memory dataspace */
    char bogus;                 /* bogus value to pass to H5Diterate() */
    H5S_t *space;               /* Dataspace for iteration */
    H5T_t *type;                /* Datatype */
    H5S_sel_iter_op_t dset_op;  /* Operator for iteration */
    herr_t ret_value;           /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iii*h", dataset_id, type_id, space_id, size);

    /* Check args */
    if(H5I_DATASET != H5I_get_type(dataset_id) ||
            H5I_DATATYPE != H5I_get_type(type_id) || size == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid argument")
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object(dataset_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")
    if(NULL == (type = (H5T_t *)H5I_object_verify(type_id, H5I_DATATYPE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not an valid base datatype")
    if(NULL == (space = (H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataspace")
    if(!(H5S_has_extent(space)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "dataspace does not have extent set")

    /* Save the dataset */
    vlen_bufsize.dset_vol_obj = vol_obj;
    vlen_bufsize.fspace_id = H5I_INVALID_HID;
    vlen_bufsize.mspace_id = H5I_INVALID_HID;

    /* Get a copy of the dataset's dataspace */
    if(H5VL_dataset_get(vol_obj, H5VL_DATASET_GET_SPACE, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &vlen_bufsize.fspace_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTCOPY, FAIL, "can't copy dataspace")

    /* Create a scalar for the memory dataspace */
    if(NULL == (mspace = H5S_create(H5S_SCALAR)))
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTCREATE, FAIL, "can't create dataspace")
    if((vlen_bufsize.mspace_id = H5I_register(H5I_DATASPACE, mspace, TRUE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register dataspace atom")

    /* Grab the temporary buffers required */
    if(NULL == (vlen_bufsize.fl_tbuf = H5FL_BLK_MALLOC(vlen_fl_buf, (size_t)1)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "no temporary buffers available")
    if(NULL == (vlen_bufsize.vl_tbuf = H5FL_BLK_MALLOC(vlen_vl_buf, (size_t)1)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "no temporary buffers available")

    /* Set the memory manager to the special allocation routine */
    if(H5CX_set_vlen_alloc_info(H5D__vlen_get_buf_size_alloc, &vlen_bufsize, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set VL data allocation routine")

    /* Set the initial number of bytes required */
    vlen_bufsize.size = 0;

    /* Call H5S_select_iterate with args, etc. */
    dset_op.op_type             = H5S_SEL_ITER_OP_APP;
    dset_op.u.app_op.op         = H5D__vlen_get_buf_size;
    dset_op.u.app_op.type_id    = type_id;

    ret_value = H5S_select_iterate(&bogus, type, space, &dset_op, &vlen_bufsize);

    /* Get the size if we succeeded */
    if(ret_value >= 0)
        *size = vlen_bufsize.size;

done:
    if(ret_value < 0)
        if(mspace && H5S_close(mspace) < 0)
            HDONE_ERROR(H5E_DATASPACE, H5E_CANTRELEASE, FAIL, "unable to release dataspace")

    if(vlen_bufsize.fspace_id && H5I_dec_app_ref(vlen_bufsize.fspace_id) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTDEC, FAIL, "problem freeing id")
    if(vlen_bufsize.mspace_id && H5I_dec_app_ref(vlen_bufsize.mspace_id) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTDEC, FAIL, "problem freeing id")
    if(vlen_bufsize.fl_tbuf != NULL)
        vlen_bufsize.fl_tbuf = H5FL_BLK_FREE(vlen_fl_buf, vlen_bufsize.fl_tbuf);
    if(vlen_bufsize.vl_tbuf != NULL)
        vlen_bufsize.vl_tbuf = H5FL_BLK_FREE(vlen_vl_buf, vlen_bufsize.vl_tbuf);

    FUNC_LEAVE_API(ret_value)
}   /* end H5Dvlen_get_buf_size() */


/*-------------------------------------------------------------------------
 * Function:    H5Dset_extent
 *
 * Purpose:     Modifies the dimensions of a dataset.
 *              Can change to a smaller dimension.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dset_extent(hid_t dset_id, const hsize_t size[])
{
    H5VL_object_t  *vol_obj;                   /* Dataset for this operation   */
    herr_t          ret_value = SUCCEED;    /* Return value                 */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*h", dset_id, size);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "dset_id parameter is not a valid dataset identifier")
    if(!size)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "size array cannot be NULL")

    /* Set up collective metadata if appropriate */
    if(H5CX_set_loc(dset_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set collective metadata read info")

    /* Set the extent */
    if ((ret_value = H5VL_dataset_specific(vol_obj, H5VL_DATASET_SET_EXTENT, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, size)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "unable to set dataset extent")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dset_extent() */


/*-------------------------------------------------------------------------
 * Function:    H5Dflush
 *
 * Purpose:     Flushes all buffers associated with a dataset.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dflush(hid_t dset_id)
{
    H5VL_object_t  *vol_obj;                   /* Dataset for this operation   */
    herr_t          ret_value = SUCCEED;    /* Return value                 */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "i", dset_id);
    
    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "dset_id parameter is not a valid dataset identifier")

    /* Set up collective metadata if appropriate */
    if(H5CX_set_loc(dset_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set collective metadata read info")

    /* Flush dataset information cached in memory
     * XXX: Note that we need to pass the ID to the VOL since the H5F_flush_cb_t
     *      callback needs it and that's in the public API.
     */
    if((ret_value = H5VL_dataset_specific(vol_obj, H5VL_DATASET_FLUSH, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, dset_id)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTFLUSH, FAIL, "unable to flush dataset")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Dflush */


/*-------------------------------------------------------------------------
 * Function:    H5Drefresh
 *
 * Purpose:     Refreshes all buffers associated with a dataset.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Drefresh(hid_t dset_id)
{
    H5VL_object_t  *vol_obj;                   /* Dataset for this operation   */
    herr_t          ret_value = SUCCEED;    /* Return value                 */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "i", dset_id);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "dset_id parameter is not a valid dataset identifier")

    /* Set up collective metadata if appropriate */
    if(H5CX_set_loc(dset_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set collective metadata read info")

    /* Refresh the dataset object */
    if((ret_value = H5VL_dataset_specific(vol_obj, H5VL_DATASET_REFRESH, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, dset_id)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTLOAD, FAIL, "unable to refresh dataset")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Drefresh() */


/*-------------------------------------------------------------------------
 * Function:    H5Dformat_convert (Internal)
 *
 * Purpose:     For chunked: 
 *                  Convert the chunk indexing type to version 1 B-tree if not
 *              For compact/contiguous:
 *                  Downgrade layout version to 3 if greater than 3
 *              For virtual:
 *                  No conversion
 *
 * Return:      SUCCEED/FAIL
 *
 * Programmer:  Vailin Choi
 *              Feb 2015
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dformat_convert(hid_t dset_id)
{
    H5VL_object_t  *vol_obj;                   /* Dataset for this operation   */
    herr_t          ret_value = SUCCEED;    /* Return value                 */
    
    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "i", dset_id);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "dset_id parameter is not a valid dataset identifier")

    /* Set up collective metadata if appropriate */
    if(H5CX_set_loc(dset_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set collective metadata read info")

    /* Convert the dataset */
    if(H5VL_dataset_optional(vol_obj, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, H5VL_NATIVE_DATASET_FORMAT_CONVERT) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_INTERNAL, FAIL, "can't convert dataset format")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Dformat_convert */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_chunk_index_type (Internal)
 *
 * Purpose:     Retrieve a dataset's chunk indexing type
 *
 * Return:      Non-negative on success, negative on failure
 *
 * Programmer:  Vailin Choi
 *              Feb 2015
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dget_chunk_index_type(hid_t dset_id, H5D_chunk_index_t *idx_type)
{
    H5VL_object_t  *vol_obj;                   /* Dataset for this operation   */
    herr_t          ret_value = SUCCEED;    /* Return value                 */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*Dk", dset_id, idx_type);

    /* Check args */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "dset_id parameter is not a valid dataset identifier")
    if(NULL == idx_type)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "idx_type parameter cannot be NULL")

    /* Get the chunk indexing type */
    if(H5VL_dataset_optional(vol_obj, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, H5VL_NATIVE_DATASET_GET_CHUNK_INDEX_TYPE, idx_type) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get chunk index type")

done:
    FUNC_LEAVE_API(ret_value)
} /* H5Dget_chunk_index_type() */


/*-------------------------------------------------------------------------
 * Function:    H5Dget_chunk_storage_size
 *
 * Purpose:     Returns the size of an allocated chunk.
 *
 *              Intended for use with the H5D(O)read_chunk API call so
 *              the caller can construct an appropriate buffer.
 *
 * Return:      SUCCEED/FAIL
 *
 * Programmer:  Matthew Strong (GE Healthcare)
 *              20 October 2016
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dget_chunk_storage_size(hid_t dset_id, const hsize_t *offset, hsize_t *chunk_nbytes)
{
    H5VL_object_t  *vol_obj;                   /* Dataset for this operation   */
    herr_t          ret_value = SUCCEED;    /* Return value                 */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "i*h*h", dset_id, offset, chunk_nbytes);

    /* Check arguments */
    if(NULL == (vol_obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "dset_id parameter is not a valid dataset identifier")
    if(NULL == offset)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "offset parameter cannot be NULL")
    if(NULL == chunk_nbytes)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "chunk_nbytes parameter cannot be NULL")

    /* Get the dataset creation property list */
    if(H5VL_dataset_optional(vol_obj, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, H5VL_NATIVE_DATASET_GET_CHUNK_STORAGE_SIZE, offset, chunk_nbytes) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get storage size of chunk")

done:
    FUNC_LEAVE_API(ret_value);
} /* H5Dget_chunk_storage_size() */


/*-------------------------------------------------------------------------
 * Function:    H5Dquery
 *
 * Purpose: Returns a dataspace selection of dataset elements that match
 *          the query.
 *
 * Return:  Success:    The ID of the dataspace selection
 *          Failure:    FAIL
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dquery(hid_t dset_id, hid_t file_space_id, hid_t query_id, hid_t xapl_id)
{
    H5D_t *dset = NULL;
    const H5S_t *file_space = NULL;
    H5S_t *ret_space = NULL;
    H5Q_t *query = NULL;
    hid_t xxpl_id = H5P_INDEX_XFER_DEFAULT; /* TODO for now */
    hid_t ret_value;

    FUNC_ENTER_API(FAIL)
    H5TRACE4("i", "iiii", dset_id, file_space_id, query_id, xapl_id);

    /* Check arguments */
    if (NULL == (dset = (H5D_t *) H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");
    if (H5S_ALL != file_space_id) {
        if (NULL == (file_space = (const H5S_t *) H5I_object_verify(file_space_id, H5I_DATASPACE)))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataspace");

        /* Check for valid selection */
        if (H5S_SELECT_VALID(file_space) != TRUE)
            HGOTO_ERROR(H5E_DATASPACE, H5E_BADRANGE, FAIL, "file selection+offset not within extent");
    } /* end if */
    if (NULL == (query = (H5Q_t *) H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query");

    /* Get the default index access property list if the user didn't provide one */
    if (H5P_DEFAULT == xapl_id)
        xapl_id = H5P_INDEX_ACCESS_DEFAULT;
    else
        if (TRUE != H5P_isa_class(xapl_id, H5P_INDEX_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not index access parms");

    if (NULL == (ret_space = H5D_query(dset, file_space, query, xapl_id, xxpl_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTSELECT, FAIL, "cannot query dataset");

    if ((ret_value = H5I_register(H5I_DATASPACE, ret_space, TRUE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register dataspace");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dquery() */
