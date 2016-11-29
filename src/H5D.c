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

/****************/
/* Module Setup */
/****************/

#include "H5Dmodule.h"          /* This source code file is part of the H5D module */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Dpkg.h"		/* Datasets 				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5ESprivate.h"        /* Event Stacks                         */
#include "H5FLprivate.h"	/* Free lists                           */
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5VLprivate.h"	/* VOL plugins				*/

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

/*******************/
/* Local Variables */
/*******************/



/*-------------------------------------------------------------------------
 * Function:	H5Dcreate2
 *
 * Purpose:	Creates a new dataset named NAME at LOC_ID, opens the
 *		dataset for access, and associates with that dataset constant
 *		and initial persistent properties including the type of each
 *		datapoint as stored in the file (TYPE_ID), the size of the
 *		dataset (SPACE_ID), and other initial miscellaneous
 *		properties (DCPL_ID).
 *
 *		All arguments are copied into the dataset, so the caller is
 *		allowed to derive new types, dataspaces, and creation
 *		parameters from the old ones and reuse them in calls to
 *		create other datasets.
 *
 * Return:	Success:	The object ID of the new dataset.  At this
 *				point, the dataset is ready to receive its
 *				raw data.  Attempting to read raw data from
 *				the dataset will probably return the fill
 *				value.	The dataset should be closed when the
 *				caller is no longer interested in it.
 *
 *		Failure:	FAIL
 *
 * Programmer:	Quincey Koziol
 *		Thursday, April 5, 2007
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dcreate2(hid_t loc_id, const char *name, hid_t type_id, hid_t space_id,
    hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id)
{
    void *dset = NULL;                /* dset token from VOL plugin */
    H5VL_object_t *obj = NULL;        /* object token of loc_id */
    H5VL_loc_params_t loc_params;
    H5P_genplist_t *plist = NULL;     /* Property list pointer */
    hid_t ret_value = FAIL;           /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE7("i", "i*siiiii", loc_id, name, type_id, space_id, lcpl_id, dcpl_id,
             dapl_id);

    if(!name || !*name)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name")

    /* Get correct property list */
    if(H5P_DEFAULT == lcpl_id)
        lcpl_id = H5P_LINK_CREATE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(lcpl_id, H5P_LINK_CREATE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not link creation property list")

    /* Get correct property list */
    if(H5P_DEFAULT == dcpl_id)
        dcpl_id = H5P_DATASET_CREATE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(dcpl_id, H5P_DATASET_CREATE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not dataset create property list ID")

    /* Get correct property list */
    if(H5P_DEFAULT == dapl_id)
        dapl_id = H5P_DATASET_ACCESS_DEFAULT;
    else
        if(TRUE != H5P_isa_class(dapl_id, H5P_DATASET_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not dataset access property list")

    /* Get the plist structure */
    if(NULL == (plist = (H5P_genplist_t *)H5I_object(dcpl_id)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* get the location object */
    if(NULL == (obj = (H5VL_object_t *)H5I_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* set creation properties */
    if(H5P_set(plist, H5VL_PROP_DSET_TYPE_ID, &type_id) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set property value for datatype id")
    if(H5P_set(plist, H5VL_PROP_DSET_SPACE_ID, &space_id) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set property value for space id")
    if(H5P_set(plist, H5VL_PROP_DSET_LCPL_ID, &lcpl_id) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set property value for lcpl id")

    loc_params.type = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Create the dataset through the VOL */
    if(NULL == (dset = H5VL_dataset_create(obj->vol_obj, loc_params, obj->vol_info->vol_cls, 
                                           name, dcpl_id, dapl_id, H5AC_dxpl_id, H5_REQUEST_NULL)))
	HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to create dataset")

    /* Get an atom for the dataset */
    if((ret_value = H5VL_register_id(H5I_DATASET, dset, obj->vol_info, TRUE)) < 0)
	HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to atomize dataset handle")

    /* Keep ID of the dataset */
    dset->shared->dset_id = ret_value;

    /* Create index if told to */
    if(H5X_can_create(ret_value, dcpl_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTCREATE, FAIL, "Index can't be created on this dataset")

done:
    if (ret_value < 0 && dset)
        if(H5VL_dataset_close (dset, obj->vol_info->vol_cls, H5AC_dxpl_id, H5_REQUEST_NULL) < 0)
            HDONE_ERROR(H5E_DATASET, H5E_CLOSEERROR, FAIL, "unable to release dataset")
    FUNC_LEAVE_API(ret_value)
} /* end H5Dcreate2() */


/*-------------------------------------------------------------------------
 * Function:	H5Dcreate_anon
 *
 * Purpose:	Creates a new dataset named NAME at LOC_ID, opens the
 *		dataset for access, and associates with that dataset constant
 *		and initial persistent properties including the type of each
 *		datapoint as stored in the file (TYPE_ID), the size of the
 *		dataset (SPACE_ID), and other initial miscellaneous
 *		properties (DCPL_ID).
 *
 *		All arguments are copied into the dataset, so the caller is
 *		allowed to derive new types, dataspaces, and creation
 *		parameters from the old ones and reuse them in calls to
 *		create other datasets.
 *
 *              The resulting ID should be linked into the file with
 *              H5Olink or it will be deleted when closed.
 *
 * Return:	Success:	The object ID of the new dataset.  At this
 *				point, the dataset is ready to receive its
 *				raw data.  Attempting to read raw data from
 *				the dataset will probably return the fill
 *				value.	The dataset should be linked into
 *                              the group hierarchy before being closed or
 *                              it will be deleted.  The dataset should be
 *                              closed when the caller is no longer interested
 *                              in it.
 *
 *		Failure:	FAIL
 *
 * Programmer:	James Laird
 *		Tuesday, January 24, 2006
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dcreate_anon(hid_t loc_id, hid_t type_id, hid_t space_id, hid_t dcpl_id,
    hid_t dapl_id)
{
    void *dset = NULL;                /* dset token from VOL plugin */
    H5VL_object_t *obj = NULL;        /* object token of loc_id */
    H5VL_loc_params_t loc_params;
    H5P_genplist_t *plist;            /* Property list pointer */
    hid_t ret_value;                  /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE5("i", "iiiii", loc_id, type_id, space_id, dcpl_id, dapl_id);

    /* Check arguments */
    if(H5P_DEFAULT == dcpl_id)
        dcpl_id = H5P_DATASET_CREATE_DEFAULT;
    else
        if(TRUE != H5P_isa_class(dcpl_id, H5P_DATASET_CREATE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not dataset create property list ID")

    /* Get correct property list */
    if(H5P_DEFAULT == dapl_id)
        dapl_id = H5P_DATASET_ACCESS_DEFAULT;
    else
        if(TRUE != H5P_isa_class(dapl_id, H5P_DATASET_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not dataset access property list")

    /* get the location object */
    if(NULL == (obj = (H5VL_object_t *)H5I_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Get the plist structure */
    if(NULL == (plist = (H5P_genplist_t *)H5I_object(dcpl_id)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* set creation properties */
    if(H5P_set(plist, H5VL_PROP_DSET_TYPE_ID, &type_id) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set property value for datatype id")
    if(H5P_set(plist, H5VL_PROP_DSET_SPACE_ID, &space_id) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't set property value for space id")

    loc_params.type = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Create the dataset through the VOL */
    if(NULL == (dset = H5VL_dataset_create(obj->vol_obj, loc_params, obj->vol_info->vol_cls, 
                                           NULL, dcpl_id, dapl_id, 
                                           H5AC_dxpl_id, H5_REQUEST_NULL)))
	HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to create dataset")

    /* Get an atom for the dataset */
    if((ret_value = H5VL_register_id(H5I_DATASET, dset, obj->vol_info, TRUE)) < 0)
	HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to atomize dataset handle")

done:
    if (ret_value < 0 && dset)
        if(H5VL_dataset_close (dset, obj->vol_info->vol_cls, H5AC_dxpl_id, H5_REQUEST_NULL) < 0)
            HDONE_ERROR(H5E_DATASET, H5E_CLOSEERROR, FAIL, "unable to release dataset")
    FUNC_LEAVE_API(ret_value)
} /* end H5Dcreate_anon() */


/*-------------------------------------------------------------------------
 * Function:	H5Dopen2
 *
 * Purpose:	Finds a dataset named NAME at LOC_ID, opens it, and returns
 *		its ID.	 The dataset should be close when the caller is no
 *		longer interested in it.
 *
 *              Takes a dataset access property list
 *
 * Return:	Success:	A new dataset ID
 *		Failure:	FAIL
 *
 * Programmer:	James Laird
 *		Thursday, July 27, 2006
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dopen2(hid_t loc_id, const char *name, hid_t dapl_id)
{
    void *dset = NULL;                /* dset token from VOL plugin */
    H5VL_object_t *obj = NULL;        /* object token of loc_id */
    H5VL_loc_params_t loc_params;
    hid_t dxpl_id = H5AC_ind_dxpl_id;    /* dxpl to use to open datset */
    hid_t ret_value;

    FUNC_ENTER_API(FAIL)
    H5TRACE3("i", "i*si", loc_id, name, dapl_id);

    /* Check args */
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name")

    /* Get correct property list */
    if(H5P_DEFAULT == dapl_id)
        dapl_id = H5P_DATASET_ACCESS_DEFAULT;
    else
        if(TRUE != H5P_isa_class(dapl_id, H5P_DATASET_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not dataset access property list")

    loc_params.type = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* get the location object */
    if(NULL == (obj = (H5VL_object_t *)H5I_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Create the dataset through the VOL */
    if(NULL == (dset = H5VL_dataset_open(obj->vol_obj, loc_params, obj->vol_info->vol_cls, name, 
                                         dapl_id, dxpl_id, H5_REQUEST_NULL)))
	HGOTO_ERROR(H5E_DATASET, H5E_CANTOPENOBJ, FAIL, "unable to open dataset")

    /* Get an atom for the dataset */
    if((ret_value = H5VL_register_id(H5I_DATASET, dset, obj->vol_info, TRUE)) < 0)
	HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to atomize dataset handle")

    /* Keep ID of the dataset */
    dset->shared->dset_id = ret_value;

done:
    if (ret_value < 0 && dset)
        if(H5VL_dataset_close (dset, obj->vol_info->vol_cls, dxpl_id, H5_REQUEST_NULL) < 0)
            HDONE_ERROR(H5E_DATASET, H5E_CLOSEERROR, FAIL, "unable to release dataset")
    FUNC_LEAVE_API(ret_value)
} /* end H5Dopen2() */


/*-------------------------------------------------------------------------
 * Function:	H5Dclose
 *
 * Purpose:	Closes access to a dataset (DATASET_ID) and releases
 *		resources used by it. It is illegal to subsequently use that
 *		same dataset ID in calls to other dataset functions.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Thursday, December  4, 1997
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dclose(hid_t dset_id)
{
    H5VL_object_t *obj;
    herr_t       ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "i", dset_id);

    /* Check args */
    if(NULL == (obj = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset ID")

    /* set the async request and dxpl IDs to be passed on to the VOL layer */
    obj->close_estack_id = H5_EVENT_STACK_NULL;
    obj->close_dxpl_id = H5AC_dxpl_id;

    /*
     * Decrement the counter on the dataset.  It will be freed if the count
     * reaches zero.  
     *
     * Pass in TRUE for the 3rd parameter to tell the function to remove
     * dataset's ID even though the freeing function might fail.  Please
     * see the comments in H5I_dec_ref for details. (SLU - 2010/9/7)
     */
    if(H5I_dec_app_ref_always_close(dset_id) < 0)
	HGOTO_ERROR(H5E_DATASET, H5E_CANTDEC, FAIL, "can't decrement count on dataset ID")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dclose() */


/*-------------------------------------------------------------------------
 * Function:	H5Dget_space
 *
 * Purpose:	Returns a copy of the file dataspace for a dataset.
 *
 * Return:	Success:	ID for a copy of the dataspace.  The data
 *				space should be released by calling
 *				H5Sclose().
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, January 28, 1998
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_space(hid_t dset_id)
{
    H5VL_object_t       *dset;
    hid_t	ret_value = FAIL;

    FUNC_ENTER_API(FAIL)
    H5TRACE1("i", "i", dset_id);

    /* Check args */
    if(NULL == (dset = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")

    /* get the dataspace through the VOL */
    if(H5VL_dataset_get(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_GET_SPACE, H5AC_dxpl_id, 
			H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, FAIL, "unable to get data space")

done:
    FUNC_LEAVE_API(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5Dget_space_status
 *
 * Purpose:	Returns the status of dataspace allocation.
 *
 * Return:
 *		Success:	Non-negative
 *
 *		Failture:	Negative
 *
 * Programmer:	Raymond Lu
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dget_space_status(hid_t dset_id, H5D_space_status_t *allocation)
{
    H5VL_object_t       *dset;
    herr_t 	ret_value = SUCCEED;

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*Ds", dset_id, allocation);

    /* Check args */
    if(NULL == (dset = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")

    /* Read data space address through the VOL and return */
    if((ret_value = H5VL_dataset_get(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_GET_SPACE_STATUS, 
                                     H5AC_ind_dxpl_id, H5_REQUEST_NULL, allocation)) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, FAIL, "unable to get space status")

done:
    FUNC_LEAVE_API(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5Dget_type
 *
 * Purpose:	Returns a copy of the file datatype for a dataset.
 *
 * Return:	Success:	ID for a copy of the datatype.	 The data
 *				type should be released by calling
 *				H5Tclose().
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Tuesday, February  3, 1998
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_type(hid_t dset_id)
{
    H5VL_object_t       *dset;
    hid_t	ret_value = FAIL;              /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("i", "i", dset_id);

    /* Check args */
    if(NULL == (dset = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")

    /* get the datatype through the VOL */
    if(H5VL_dataset_get(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_GET_TYPE, 
                        H5AC_dxpl_id, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, FAIL, "unable to get datatype")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_type() */


/*-------------------------------------------------------------------------
 * Function:	H5Dget_create_plist
 *
 * Purpose:	Returns a copy of the dataset creation property list.
 *
 * Return:	Success:	ID for a copy of the dataset creation
 *				property list.  The template should be
 *				released by calling H5P_close().
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Tuesday, February  3, 1998
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_create_plist(hid_t dset_id)
{
    H5VL_object_t       *dset;
    hid_t       ret_value = FAIL;              /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("i", "i", dset_id);

    /* Check args */
    if(NULL == (dset = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")

    if(H5VL_dataset_get(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_GET_DCPL, H5AC_dxpl_id, 
                        H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, FAIL, "unable to get dataset creation properties")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_create_plist() */


/*-------------------------------------------------------------------------
 * Function:	H5Dget_access_plist
 *
 * Purpose:	Returns a copy of the dataset creation property list.
 *
 * Description: H5Dget_access_plist returns the dataset access property
 *      list identifier of the specified dataset.
 *
 *      The chunk cache parameters in the returned property lists will be
 *      those used by the dataset.  If the properties in the file access
 *      property list were used to determine the dataset’s chunk cache
 *      configuration, then those properties will be present in the
 *      returned dataset access property list.  If the dataset does not
 *      use a chunked layout, then the chunk cache properties will be set
 *      to the default.  The chunk cache properties in the returned list
 *      are considered to be “set”, and any use of this list will override
 *      the corresponding properties in the file’s file access property
 *      list.
 *
 *      All link access properties in the returned list will be set to the
 *      default values.
 *
 * Return:	Success:	ID for a copy of the dataset access
 *				property list.  The template should be
 *				released by calling H5Pclose().
 *
 *		Failure:	FAIL
 *
 * Programmer:	Neil Fortner
 *		Wednesday, October 29, 2008
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Dget_access_plist(hid_t dset_id)
{
    H5VL_object_t       *dset;
    hid_t       ret_value = FAIL;      /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("i", "i", dset_id);

    /* Check args */
    if(NULL == (dset = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")

    if(H5VL_dataset_get(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_GET_DAPL, 
                        H5AC_dxpl_id, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, FAIL, "unable to get dataset access properties")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_access_plist() */


/*-------------------------------------------------------------------------
 * Function:	H5Dget_storage_size
 *
 * Purpose:	Returns the amount of storage that is required for the
 *		dataset. For chunked datasets this is the number of allocated
 *		chunks times the chunk size.
 *
 * Return:	Success:	The amount of storage space allocated for the
 *				dataset, not counting meta data. The return
 *				value may be zero if no data has been stored.
 *
 *		Failure:	Zero
 *
 * Programmer:	Robb Matzke
 *              Wednesday, April 21, 1999
 *
 *-------------------------------------------------------------------------
 */
hsize_t
H5Dget_storage_size(hid_t dset_id)
{
    H5VL_object_t       *dset;
    hsize_t	ret_value;      /* Return value */

    FUNC_ENTER_API(0)
    H5TRACE1("h", "i", dset_id);

    /* Check args */
    if(NULL == (dset = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, 0, "invalid dataset identifier")

    /* get storage size through the VOL */
    if(H5VL_dataset_get(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_GET_STORAGE_SIZE, 
                        H5AC_ind_dxpl_id, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, 0, "unable to get storage size")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Dget_storage_size() */


/*-------------------------------------------------------------------------
 * Function:	H5Dget_offset
 *
 * Purpose:	Returns the address of dataset in file.
 *
 * Return:	Success:        the address of dataset
 *
 *		Failure:	HADDR_UNDEF
 *
 * Programmer:  Raymond Lu
 *              November 6, 2002
 *
 *-------------------------------------------------------------------------
 */
haddr_t
H5Dget_offset(hid_t dset_id)
{
    H5VL_object_t       *dset;
    haddr_t	ret_value;      /* Return value */

    FUNC_ENTER_API(HADDR_UNDEF)
    H5TRACE1("a", "i", dset_id);

    /* Check args */
    if(NULL == (dset = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, HADDR_UNDEF, "invalid dataset identifier")

    /* get offset through the VOL */
    if(H5VL_dataset_get(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_GET_OFFSET, 
                        H5AC_dxpl_id, H5_REQUEST_NULL, &ret_value) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, HADDR_UNDEF, "unable to get offset")

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
 * Function:    H5Dref_reclaim
 *
 * Purpose: Frees the buffers allocated for storing variable-length reference
 *      in memory.  Only frees the VL data in the selection defined in the
 *      dataspace.  The dataset transfer property list is required to find the
 *      correct allocation/free methods for the VL data in the buffer.
 *
 * Return:  Non-negative on success, negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dref_reclaim(hid_t type_id, hid_t space_id, hid_t plist_id, void *buf)
{
    H5S_t *space;               /* Dataspace for iteration */
    herr_t ret_value;           /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iii*x", type_id, space_id, plist_id, buf);

    /* Check args */
    if(H5I_DATATYPE != H5I_get_type(type_id) || buf == NULL)
    HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid argument")
    if(NULL == (space = (H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataspace")
    if(!(H5S_has_extent(space)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "dataspace does not have extent set")

    /* Get the default dataset transfer property list if the user didn't provide one */
    if(H5P_DEFAULT == plist_id)
        plist_id = H5P_DATASET_XFER_DEFAULT;
    else
        if(TRUE != H5P_isa_class(plist_id, H5P_DATASET_XFER))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not xfer parms")

    /* Call internal routine */
    ret_value = H5D_ref_reclaim(type_id, space, plist_id, buf);

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Dref_reclaim() */


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
H5Dvlen_reclaim(hid_t type_id, hid_t space_id, hid_t plist_id, void *buf)
{
    H5S_t *space;               /* Dataspace for iteration */
    herr_t ret_value;           /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iii*x", type_id, space_id, plist_id, buf);

    /* Check args */
    if((H5I_DATATYPE != H5I_get_type(type_id)) || 
       buf == NULL)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid argument")
    if(NULL == (space = (H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataspace")
    if(!(H5S_has_extent(space)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "dataspace does not have extent set")

    /* Get the default dataset transfer property list if the user didn't provide one */
    if(H5P_DEFAULT == plist_id)
        plist_id = H5P_DATASET_XFER_DEFAULT;
    else
        if(TRUE != H5P_isa_class(plist_id, H5P_DATASET_XFER))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not xfer parms")

    /* Call internal routine */
    ret_value = H5D_vlen_reclaim(type_id, space, plist_id, buf);

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
    H5D_vlen_bufsize_t vlen_bufsize = {0, 0, 0, 0, 0, 0, 0};
    H5VL_object_t  *dset;                /* Dataset for operation */
    H5S_t *mspace = NULL;       /* Memory dataspace */
    char bogus;                 /* bogus value to pass to H5Diterate() */
    H5S_t *space;               /* Dataspace for iteration */
    H5P_genplist_t  *plist;     /* Property list */
    H5T_t *type;                /* Datatype */
    H5S_sel_iter_op_t dset_op;  /* Operator for iteration */
    herr_t ret_value;           /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iii*h", dataset_id, type_id, space_id, size);

    /* Check args */
    if(H5I_DATASET != H5I_get_type(dataset_id) ||
       (H5I_DATATYPE != H5I_get_type(type_id)) || size == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid argument")
    /* get the dataset object */
    if(NULL == (dset = (H5VL_object_t *)H5I_object(dataset_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")
    if(NULL == (type = (H5T_t *)H5I_object_verify(type_id, H5I_DATATYPE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not an valid base datatype")
    if(NULL == (space = (H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataspace")
    if(!(H5S_has_extent(space)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "dataspace does not have extent set")

    /* Save the dataset */
    vlen_bufsize.dset = dset;
    vlen_bufsize.fspace_id = FAIL;
    vlen_bufsize.mspace_id = FAIL;

    /* Get a copy of the dataspace ID */
    if(H5VL_dataset_get(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_GET_SPACE, H5AC_dxpl_id, 
		        H5_REQUEST_NULL, &vlen_bufsize.fspace_id) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTCOPY, FAIL, "can't copy dataspace")

    /* Create a scalar for the memory dataspace */
    if(NULL == (mspace = H5S_create(H5S_SCALAR)))
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTCREATE, FAIL, "can't create dataspace")
    /* Atomize */
    if((vlen_bufsize.mspace_id = H5I_register (H5I_DATASPACE, mspace, TRUE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register dataspace atom")

    /* Grab the temporary buffers required */
    if(NULL == (vlen_bufsize.fl_tbuf = H5FL_BLK_MALLOC(vlen_fl_buf, (size_t)1)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "no temporary buffers available")
    if(NULL == (vlen_bufsize.vl_tbuf = H5FL_BLK_MALLOC(vlen_vl_buf, (size_t)1)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "no temporary buffers available")

    /* Change to the custom memory allocation routines for reading VL data */
    if((vlen_bufsize.xfer_pid = H5P_create_id(H5P_CLS_DATASET_XFER_g, FALSE)) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTCREATE, FAIL, "no dataset xfer plists available")

    /* Get the property list struct */
    if(NULL == (plist = (H5P_genplist_t *)H5I_object(vlen_bufsize.xfer_pid)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset transfer property list")

    /* Set the memory manager to the special allocation routine */
    if(H5P_set_vlen_mem_manager(plist, H5D__vlen_get_buf_size_alloc, &vlen_bufsize, NULL, NULL) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINIT, FAIL, "can't set VL data allocation routine")

    /* Set the initial number of bytes required */
    vlen_bufsize.size = 0;

    /* Call H5S_select_iterate with args, etc. */
    dset_op.op_type = H5S_SEL_ITER_OP_APP;
    dset_op.u.app_op.op = H5D__vlen_get_buf_size;
    dset_op.u.app_op.type_id = type_id;

    ret_value = H5S_select_iterate(&bogus, type, space, &dset_op, &vlen_bufsize);

    /* Get the size if we succeeded */
    if(ret_value >= 0)
        *size = vlen_bufsize.size;

done:
    if(ret_value < 0) {
        if(mspace && H5S_close(mspace) < 0)
            HDONE_ERROR(H5E_DATASPACE, H5E_CANTRELEASE, FAIL, "unable to release dataspace")
    } /* end if */

    if(vlen_bufsize.fspace_id && H5I_dec_app_ref(vlen_bufsize.fspace_id) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTDEC, FAIL, "problem freeing id")
    if(vlen_bufsize.mspace_id && H5I_dec_app_ref(vlen_bufsize.mspace_id) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTDEC, FAIL, "problem freeing id")
    if(vlen_bufsize.fl_tbuf != NULL)
        vlen_bufsize.fl_tbuf = H5FL_BLK_FREE(vlen_fl_buf, vlen_bufsize.fl_tbuf);
    if(vlen_bufsize.vl_tbuf != NULL)
        vlen_bufsize.vl_tbuf = H5FL_BLK_FREE(vlen_vl_buf, vlen_bufsize.vl_tbuf);
    if(vlen_bufsize.xfer_pid > 0 && H5I_dec_ref(vlen_bufsize.xfer_pid) < 0)
        HDONE_ERROR(H5E_DATASET, H5E_CANTDEC, FAIL, "unable to decrement ref count on property list")

    FUNC_LEAVE_API(ret_value)
}   /* end H5Dvlen_get_buf_size() */


/*-------------------------------------------------------------------------
 * Function:	H5Dset_extent
 *
 * Purpose:	Modifies the dimensions of a dataset.
 *		Can change to a smaller dimension.
 *
 * Return:	Non-negative on success, negative on failure
 *
 * Programmer:  Pedro Vicente, pvn@ncsa.uiuc.edu
 *              April 9, 2002
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Dset_extent(hid_t dset_id, const hsize_t size[])
{
    H5VL_object_t       *dset;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*h", dset_id, size);

    /* Check args */
    if(NULL == (dset = (H5VL_object_t *)H5I_object_verify(dset_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid dataset identifier")

    if(!size)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no size specified")

    /* set the extent through the VOL */
    if((ret_value = H5VL_dataset_specific(dset->vol_obj, dset->vol_info->vol_cls, H5VL_DATASET_SET_EXTENT, 
                                          H5AC_dxpl_id, H5_REQUEST_NULL, size)) < 0)
	HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to set extent of dataset")

done:
        FUNC_LEAVE_API(ret_value)
} /* end H5Dset_extent() */


/*-------------------------------------------------------------------------
 * Function:	H5D_close_dataset
 *
 * Purpose:	Called when the ref count reaches zero on the dataset_id
 *
 * Return:	Success:	Non-negative
 *
 *		Failure:	Negative
 *
 * Programmer:	Mohamad Chaarawi
 *              June 2012
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D_close_dataset(void *_dset)
{
    H5_priv_request_t  *request = NULL;        /* private request struct inserted in event queue */
    void **req = NULL;            /* pointer to plugin generate requests (Stays NULL if plugin does not support async */
    H5VL_object_t *dset = (H5VL_object_t *)_dset;
    herr_t ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    if(dset->close_estack_id != H5_EVENT_STACK_NULL) {
        /* create the private request */
        if(NULL == (request = (H5_priv_request_t *)H5MM_calloc(sizeof(H5_priv_request_t))))
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed");
        req = &request->req;
        request->vol_cls = dset->vol_info->vol_cls;
    }

    /* Close the dataset through the VOL */
    if((ret_value = H5VL_dataset_close(dset->vol_obj, dset->vol_info->vol_cls, 
                                       dset->close_dxpl_id, req)) < 0)
	HGOTO_ERROR(H5E_DATASET, H5E_CLOSEERROR, FAIL, "unable to close dataset")

    if(request && *req)
        if(H5ES_insert(dset->close_estack_id, request) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "failed to insert request in event stack")

done:
    /* MSC - Weird thing for datasets and filters:
       Always decrement the ref count on the vol for datasets, since
       the ID is removed even if the close fails */

    /* free dset */
    if(H5VL_free_object(dset) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTDEC, FAIL, "unable to free VOL object")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D_close_dataset() */


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
