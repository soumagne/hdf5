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
 * Purpose:     Reference routines.
 */

/****************/
/* Module Setup */
/****************/

#include "H5Rmodule.h"          /* This source code file is part of the H5R module */

/***********/
/* Headers */
/***********/
#include "H5private.h"          /* Generic Functions                        */
#include "H5CXprivate.h"        /* API Contexts                             */
#include "H5Eprivate.h"         /* Error handling                           */
#include "H5Iprivate.h"         /* IDs                                      */
#include "H5MMprivate.h"        /* Memory management                        */
#include "H5Rpkg.h"             /* References                               */
#include "H5Sprivate.h"         /* Dataspaces                               */


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


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/


/*-------------------------------------------------------------------------
 * Function:    H5Rcreate_object
 *
 * Purpose: Creates an object reference. The LOC_ID and NAME are used to locate
 * the object pointed to.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rcreate_object(hid_t loc_id, const char *name, href_t *ref_ptr)
{
    H5VL_object_t *vol_obj = NULL;  /* Object token of loc_id */
    H5VL_loc_params_t loc_params;   /* Location parameters */
    struct href *ref = (struct href *)ref_ptr; /* Internal reference */
    haddr_t obj_addr;               /* Object address */
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "i*s*Rr", loc_id, name, ref_ptr);

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name given")

    /* Get the VOL object */
    if (NULL == (vol_obj = H5VL_vol_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Set up collective metadata if appropriate */
    if(H5CX_set_loc(loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "can't set access property list info")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_NAME;
    loc_params.loc_data.loc_by_name.name = name;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Get the object address */
    if(H5VL_object_specific(vol_obj, &loc_params, H5VL_OBJECT_LOOKUP, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &obj_addr) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, FAIL, "unable to retrieve object address")

    /* Create the reference */
    HDmemset(ref_ptr, 0, H5R_REF_BUF_SIZE);
    if(H5R__create_object(obj_addr, ref) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCREATE, FAIL, "unable to create object reference")

    /* Attach loc_id to reference */
    if(H5R__set_loc_id(ref, loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "unable to attach location id to reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rcreate_object() */


/*-------------------------------------------------------------------------
 * Function:    H5Rcreate_region
 *
 * Purpose: Creates a region reference. The LOC_ID and NAME are used to locate
 * the object pointed to and the SPACE_ID is used to choose the region pointed
 * to.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rcreate_region(hid_t loc_id, const char *name, hid_t space_id,
    href_t *ref_ptr)
{
    H5VL_object_t *vol_obj = NULL;  /* Object token of loc_id */
    H5VL_loc_params_t loc_params;   /* Location parameters */
    struct href *ref = (struct href *)ref_ptr; /* Internal reference */
    haddr_t obj_addr;               /* Object address */
    struct H5S_t *space = NULL;     /* Pointer to dataspace containing region */
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "i*si*Rr", loc_id, name, space_id, ref_ptr);

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name given")
    if(space_id == H5I_BADID)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "reference region dataspace id must be valid")
    if(NULL == (space = (struct H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataspace")

    /* Get the VOL object */
    if (NULL == (vol_obj = H5VL_vol_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Set up collective metadata if appropriate */
    if(H5CX_set_loc(loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "can't set access property list info")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_NAME;
    loc_params.loc_data.loc_by_name.name = name;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Get the object address */
    if(H5VL_object_specific(vol_obj, &loc_params, H5VL_OBJECT_LOOKUP, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &obj_addr) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, FAIL, "unable to retrieve object address")

    /* Create the reference */
    HDmemset(ref_ptr, 0, H5R_REF_BUF_SIZE);
    if(H5R__create_region(obj_addr, space, ref) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCREATE, FAIL, "unable to create region reference")

    /* Attach loc_id to reference */
    if(H5R__set_loc_id(ref, loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "unable to attach location id to reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rcreate_region() */


/*-------------------------------------------------------------------------
 * Function:    H5Rcreate_attr
 *
 * Purpose: Creates an attribute reference. The LOC_ID, NAME and ATTR_NAME are
 * used to locate the attribute pointed to.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rcreate_attr(hid_t loc_id, const char *name, const char *attr_name,
    href_t *ref_ptr)
{
    H5VL_object_t *vol_obj = NULL;  /* Object token of loc_id */
    H5VL_loc_params_t loc_params;   /* Location parameters */
    struct href *ref = (struct href *)ref_ptr; /* Internal reference */
    haddr_t obj_addr;               /* Object address */
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "i*s*s*Rr", loc_id, name, attr_name, ref_ptr);

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name given")
    if(!attr_name || !*attr_name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no attribute name given")

    /* Get the VOL object */
    if (NULL == (vol_obj = H5VL_vol_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Set up collective metadata if appropriate */
    if(H5CX_set_loc(loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "can't set access property list info")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_NAME;
    loc_params.loc_data.loc_by_name.name = name;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Get the object address */
    if(H5VL_object_specific(vol_obj, &loc_params, H5VL_OBJECT_LOOKUP, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &obj_addr) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, FAIL, "unable to retrieve object address")

    /* Create the reference */
    HDmemset(ref_ptr, 0, H5R_REF_BUF_SIZE);
    if(H5R__create_attr(obj_addr, attr_name, ref) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCREATE, FAIL, "unable to create attribute reference")

    /* Attach loc_id to reference */
    if(H5R__set_loc_id(ref, loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "unable to attach location id to reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rcreate_attr() */


/*-------------------------------------------------------------------------
 * Function:    H5Rdestroy
 *
 * Purpose: Destroy reference and free resources allocated during H5Rcreate.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rdestroy(href_t *ref_ptr)
{
    struct href *ref = (struct href *)ref_ptr; /* Internal reference */
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "*Rr", ref_ptr);

    /* Check args */
    if(NULL == ref)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid reference pointer")

    /* Destroy reference */
    if(H5R__destroy(ref) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTFREE, FAIL, "unable to destroy reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rdestroy() */


/*-------------------------------------------------------------------------
 * Function:    H5Rget_type
 *
 * Purpose: Given a reference to some object, return the type of that reference.
 *
 * Return:  Reference type/H5R_BADTYPE on failure
 *
 *-------------------------------------------------------------------------
 */
H5R_type_t
H5Rget_type(const href_t *ref_ptr)
{
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    H5R_type_t ret_value;   /* Return value */

    FUNC_ENTER_API(H5R_BADTYPE)
    H5TRACE1("Rt", "*Rr", ref_ptr);

    /* Check args */
    if(NULL == ref)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5R_BADTYPE, "invalid reference pointer")

    /* Get reference type */
    ret_value = H5R__get_type(ref);
    if((ret_value <= H5R_BADTYPE) || (ret_value >= H5R_MAXTYPE))
        HGOTO_ERROR(H5E_REFERENCE, H5E_BADVALUE, H5R_BADTYPE, "invalid reference type")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_type() */


/*-------------------------------------------------------------------------
 * Function:    H5Requal
 *
 * Purpose: Compare two references
 *
 * Return:  TRUE if equal, FALSE if unequal, FAIL if error
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5Requal(const href_t *ref1_ptr, const href_t *ref2_ptr)
{
    const struct href *ref1 = (const struct href *)ref1_ptr; /* Internal reference */
    const struct href *ref2 = (const struct href *)ref2_ptr; /* Internal reference */
    htri_t ret_value;   /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("t", "*Rr*Rr", ref1_ptr, ref2_ptr);

    /* Check args */
    if(!ref1 || !ref2)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")

    /* Compare references */
    if ((ret_value = H5R__equal(ref1, ref2)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOMPARE, FAIL, "cannot compare references")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Requal() */


/*-------------------------------------------------------------------------
 * Function:    H5Rcopy
 *
 * Purpose: Copy a reference
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rcopy(const href_t *src_ref_ptr, href_t *dest_ref_ptr)
{
    const struct href *src_ref = (const struct href *)src_ref_ptr; /* Internal reference */
    struct href *dest_ref = (struct href *)dest_ref_ptr; /* Internal reference */
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "*Rr*Rr", src_ref_ptr, dest_ref_ptr);

    /* Check args */
    if(NULL == src_ref || NULL == dest_ref)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid reference pointer")

    /* Copy reference */
    if (H5R__copy(src_ref, dest_ref) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, FAIL, "cannot copy reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rcopy() */


/*-------------------------------------------------------------------------
 * Function:    H5Ropen_object
 *
 * Purpose: Given a reference to some object, open that object and return an
 * ID for that object.
 *
 * Return:  Valid ID on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Ropen_object(const href_t *ref_ptr, hid_t oapl_id)
{
    hid_t loc_id;                       /* Reference location ID */
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    haddr_t obj_addr;                   /* Object address */
    H5I_type_t opened_type;             /* Opened object type */
    void *opened_obj = NULL;            /* Opened object */
    hid_t ret_value = H5I_INVALID_HID;  /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE2("i", "*Rri", ref_ptr, oapl_id);

    /* Check args */
    if(oapl_id < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "not a property list")
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference pointer")
    if(H5R__get_type(ref) <= H5R_BADTYPE || H5R__get_type(ref) >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference type")

    /* Retrieve loc_id from reference */
    if(H5I_INVALID_HID == (loc_id = H5R__get_loc_id(ref)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference location ID")

    /* Get object address */
    if(H5R__get_obj_addr(ref, &obj_addr) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, H5I_INVALID_HID, "unable to get object address")

    /* Verify access property list and set up collective metadata if appropriate */
    if(H5CX_set_apl(&oapl_id, H5P_CLS_DACC, loc_id, FALSE) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, H5I_INVALID_HID, "can't set access property list info")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid location identifier")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Open object by address */
    if(NULL == (opened_obj = H5VL_object_open(vol_obj, &loc_params, &opened_type, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTOPENOBJ, H5I_INVALID_HID, "unable to open object by address")

    /* Register object */
    if((ret_value = H5VL_register(opened_type, opened_obj, vol_obj->connector, TRUE)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register object handle")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Ropen_object() */


/*-------------------------------------------------------------------------
 * Function:    H5Ropen_region
 *
 * Purpose: Given a reference to some object, creates a copy of the dataset
 * pointed to's dataspace and defines a selection in the copy which is the
 * region pointed to.
 *
 * Return:  Valid ID on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Ropen_region(const href_t *ref_ptr)
{
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    H5S_t *space = NULL;        /* Pointer to dataspace containing region */
    hid_t ret_value;            /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE1("i", "*Rr", ref_ptr);

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference pointer")
    if((H5R__get_type(ref) != H5R_DATASET_REGION1) && (H5R__get_type(ref) != H5R_DATASET_REGION2))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference type")

    /* Get the dataspace with the correct region selected */
    if(NULL == (space = H5R__get_region(ref)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, H5I_INVALID_HID, "unable to get dataspace")

    /* Atomize */
    if((ret_value = H5I_register(H5I_DATASPACE, space, TRUE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register dataspace atom")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Ropen_region() */


/*-------------------------------------------------------------------------
 * Function:    H5Ropen_attr
 *
 * Purpose: Given a reference to some attribute, open that attribute and
 * return an ID for that attribute.
 *
 * Return:  Valid ID on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Ropen_attr(const href_t *ref_ptr, hid_t aapl_id)
{
    hid_t loc_id;                           /* Reference location ID */
    H5VL_object_t *vol_obj = NULL;          /* Object token of loc_id */
    H5VL_loc_params_t loc_params;           /* Location parameters */
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    haddr_t obj_addr;                       /* Object address */
    H5I_type_t opened_type;                 /* Opened object type */
    void *opened_obj = NULL;                /* Opened object */
    hid_t opened_obj_id = H5I_INVALID_HID;  /* Opened object ID */
    char *attr_name = NULL;                 /* Attribute name */
    ssize_t attr_name_size = 0;             /* Attribute name size */
    void *opened_attr = NULL;               /* Opened attribute */
    hid_t ret_value = H5I_INVALID_HID;      /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE2("i", "*Rri", ref_ptr, aapl_id);

    /* Check args */
    if(aapl_id < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "not a property list")
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference pointer")
    if(H5R__get_type(ref) != H5R_ATTR)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference type")

    /* Retrieve loc_id from reference */
    if(H5I_INVALID_HID == (loc_id = H5R__get_loc_id(ref)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference location ID")

    /* Get object address */
    if(H5R__get_obj_addr(ref, &obj_addr) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, H5I_INVALID_HID, "unable to get object address")

    /* TODO Verify access property list and set up collective metadata if appropriate */
    /* Note there is currently no way to pass that to H5VL_object_open by addr */
//    if(H5CX_set_apl(&oapl_id, H5P_CLS_DACC, loc_id, FALSE) < 0)
//        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, H5I_INVALID_HID, "can't set access property list info")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid location identifier")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Open object by address */
    if(NULL == (opened_obj = H5VL_object_open(vol_obj, &loc_params, &opened_type, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTOPENOBJ, H5I_INVALID_HID, "unable to open object by address")

    /* Register object */
    if((opened_obj_id = H5VL_register(opened_type, opened_obj, vol_obj->connector, FALSE)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register object handle")

    /* Retrieve attribute name from reference */
    if((attr_name_size = H5R__get_attr_name(ref, NULL, 0)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_BADVALUE, H5I_INVALID_HID, "invalid attribute name length")
    if(NULL == (attr_name = (char *)H5MM_malloc((size_t)attr_name_size)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, H5I_INVALID_HID, "cannot allocate memory for attribute name")
    if(H5R__get_attr_name(ref, attr_name, (size_t)attr_name_size) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, H5I_INVALID_HID, "can't retrieve attribute name")

    /* Verify access property list and set up collective metadata if appropriate */
    if(H5CX_set_apl(&aapl_id, H5P_CLS_AACC, loc_id, FALSE) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTSET, H5I_INVALID_HID, "can't set access property list info")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type = opened_type;

    /* Get VOL object object */
    if(NULL == (opened_obj = H5VL_vol_object(opened_obj_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid location identifier")

    /* Open the attribute */
    if(NULL == (opened_attr = H5VL_attr_open(opened_obj, &loc_params, attr_name, aapl_id, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL)))
        HGOTO_ERROR(H5E_ATTR, H5E_CANTOPENOBJ, H5I_INVALID_HID, "unable to open attribute: '%s'", attr_name)

    /* Register the attribute and get an ID for it */
    if((ret_value = H5VL_register(H5I_ATTR, opened_attr, vol_obj->connector, TRUE)) < 0)
        HGOTO_ERROR(H5E_ATTR, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to atomize attribute handle")

done:
    H5MM_free(attr_name);
    if((opened_obj_id != H5I_INVALID_HID) && (H5I_dec_ref(opened_obj_id) < 0))
        HDONE_ERROR(H5E_REFERENCE, H5E_CLOSEERROR, H5I_INVALID_HID, "can't close object")
    if(H5I_INVALID_HID == ret_value) /* Cleanup on failure */
        if(opened_attr && H5VL_attr_close(vol_obj, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL) < 0)
            HDONE_ERROR(H5E_ATTR, H5E_CLOSEERROR, H5I_INVALID_HID, "can't close attribute")

    FUNC_LEAVE_API(ret_value)
} /* end H5Ropen_attr() */


/*-------------------------------------------------------------------------
 * Function:    H5Rget_obj_type3
 *
 * Purpose: Given a reference to some object, this function returns the type
 * of object pointed to.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rget_obj_type3(const href_t *ref_ptr, H5O_type_t *obj_type)
{
    hid_t loc_id;                       /* Reference location ID */
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    haddr_t obj_addr;                   /* Object address */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "*Rr*Ot", ref_ptr, obj_type);

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(H5R__get_type(ref) <= H5R_BADTYPE || H5R__get_type(ref) >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference type")

    /* Retrieve loc_id from reference */
    if(H5I_INVALID_HID == (loc_id = H5R__get_loc_id(ref)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference location ID")

    /* Get object address */
    if(H5R__get_obj_addr(ref, &obj_addr) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, FAIL, "unable to get object address")

    /* Get the VOL object */
    if (NULL == (vol_obj = H5VL_vol_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Retrieve object's type */
    if(H5VL_object_get(vol_obj, &loc_params, H5VL_OBJECT_GET_TYPE, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, obj_type) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTGET, FAIL, "can't retrieve object type")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_obj_type3() */


/*-------------------------------------------------------------------------
 * Function:    H5Rget_file_name
 *
 * Purpose: Given a reference to some object, determine a file name of the
 * object located into.
 *
 * Return:  Non-negative length of the path on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5Rget_file_name(const href_t *ref_ptr, char *name, size_t size)
{
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    ssize_t ret_value;  /* Return value */

    FUNC_ENTER_API((-1))
    H5TRACE3("Zs", "*Rr*sz", ref_ptr, name, size);

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference pointer")
    if(H5R__get_type(ref) <= H5R_BADTYPE || H5R__get_type(ref) >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference type")

    /* Get name */
    if((ret_value = H5R__get_file_name(ref, name, size)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, (-1), "unable to retrieve file name")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_file_name() */


/*-------------------------------------------------------------------------
 * Function:    H5Rget_obj_name
 *
 * Purpose: Given a reference to some object, determine a path to the object
 * referenced in the file.
 *
 * Return:  Non-negative length of the path on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5Rget_obj_name(const href_t *ref_ptr, char *name, size_t size)
{
    hid_t loc_id;                       /* Reference location ID */
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    haddr_t obj_addr;                   /* Object address */
    ssize_t ret_value;                  /* Return value */

    FUNC_ENTER_API((-1))
    H5TRACE3("Zs", "*Rr*sz", ref_ptr, name, size);

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference pointer")
    if(H5R__get_type(ref) <= H5R_BADTYPE || H5R__get_type(ref) >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference type")

    /* Retrieve loc_id from reference */
    if(H5I_INVALID_HID == (loc_id = H5R__get_loc_id(ref)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference location ID")

    /* Get object address */
    if((ret_value = H5R__get_obj_addr(ref, &obj_addr)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, (-1), "unable to get object address")

    /* Get the VOL object */
    if (NULL == (vol_obj = H5VL_vol_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, (-1), "invalid location identifier")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Retrieve object's name */
    if(H5VL_object_get(vol_obj, &loc_params, H5VL_OBJECT_GET_NAME, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &ret_value, name, size) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTGET, (-1), "can't retrieve object name")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_obj_name() */


/*-------------------------------------------------------------------------
 * Function:    H5Rget_attr_name
 *
 * Purpose: Given a reference to some attribute, determine its name.
 *
 * Return:  Non-negative length of the path on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5Rget_attr_name(const href_t *ref_ptr, char *name, size_t size)
{
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    ssize_t ret_value;  /* Return value */

    FUNC_ENTER_API((-1))
    H5TRACE3("Zs", "*Rr*sz", ref_ptr, name, size);

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference pointer")
    if(H5R__get_type(ref) != H5R_ATTR)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference type")

    /* Get attribute name */
    if((ret_value = H5R__get_attr_name(ref, name, size)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, (-1), "unable to determine attribute name")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_attr_name() */


/*-------------------------------------------------------------------------
 * Function:    H5Rencode
 *
 * Purpose: Given a reference, serialize it into a buffer.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rencode(const href_t *ref_ptr, void *buf, size_t *nalloc)
{
    const struct href *ref = (const struct href *)ref_ptr; /* Internal reference */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "*Rr*x*z", ref_ptr, buf, nalloc);

    /* Check arguments */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(nalloc == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer for buffer size")

    /* Go encode the reference */
    if(H5R__encode(ref, (unsigned char *)buf, nalloc) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "can't encode reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rencode() */


/*-------------------------------------------------------------------------
 * Function:    H5Rdecode
 *
 * Purpose: Deserialize a reference.
 *
 * Return:  Success:    Reference
 *          Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rdecode(hid_t loc_id, const void *buf, href_t *ref_ptr)
{
    struct href *ref = (struct href *)ref_ptr; /* Internal reference */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "i*x*Rr", loc_id, buf, ref_ptr);

    /* Check args */
    if(buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "empty buffer")
    if(ref_ptr == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")

    /* Create reference by decoding buffer */
    if(H5R__decode((const unsigned char *)buf, NULL, ref) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "can't decode reference")

    /* Set location ID */
    if(H5R__set_loc_id(ref, loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "unable to attach location id to reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rdecode() */
