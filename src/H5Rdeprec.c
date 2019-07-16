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

/*-------------------------------------------------------------------------
 *
 * Created:     H5Rdeprec.c
 *
 * Purpose:     Deprecated functions from the H5R interface.  These
 *              functions are here for compatibility purposes and may be
 *              removed in the future.  Applications should switch to the
 *              newer APIs.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#include "H5Rmodule.h"          /* This source code file is part of the H5R module */


/***********/
/* Headers */
/***********/
/* Public headers needed by this file */
#include "H5Ppublic.h"          /* Property lists                           */

/* Private headers needed by this file */
#include "H5private.h"          /* Generic Functions                        */
#include "H5ACprivate.h"        /* Metadata cache                           */
#include "H5CXprivate.h"        /* API Contexts                             */
#include "H5Eprivate.h"         /* Error handling                           */
#include "H5Gprivate.h"         /* Groups                                   */
#include "H5Iprivate.h"         /* IDs                                      */
#include "H5Oprivate.h"         /* Object headers                           */
#include "H5Rpkg.h"             /* References                               */
#include "H5Sprivate.h"         /* Dataspaces                               */


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


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
 * Function:    H5Rcreate
 *
 * Purpose: Creates a particular type of reference specified with REF_TYPE,
 * in the space pointed to by REF. The LOC_ID and NAME are used to locate the
 * object pointed to and the SPACE_ID is used to choose the region pointed to
 * (for Dataset Region references).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rcreate(void *ref, hid_t loc_id, const char *name, H5R_type_t ref_type,
    hid_t space_id)
{
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    haddr_t obj_addr;                   /* Object address */
    unsigned char *buf = (unsigned char *)ref; /* Return reference pointer */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE5("e", "*xi*sRti", ref, loc_id, name, ref_type, space_id);

    /* Check args */
    if(buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name given")
    if(ref_type != H5R_OBJECT1 && ref_type != H5R_DATASET_REGION1)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference type")

    /* Set up collective metadata if appropriate */
    if(H5CX_set_loc(loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "can't set access property list info")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(loc_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_NAME;
    loc_params.loc_data.loc_by_name.name = name;
    loc_params.obj_type = H5I_get_type(loc_id);

    /* Get the object address */
    if(H5VL_object_specific(vol_obj, &loc_params, H5VL_OBJECT_LOOKUP, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &obj_addr) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, FAIL, "unable to retrieve object address")

    /* Create reference */
    if(ref_type == H5R_OBJECT1) {
        size_t buf_size = H5R_OBJ_REF_BUF_SIZE;

        if((ret_value = H5R__encode_obj_addr_compat(obj_addr, buf, &buf_size)) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCREATE, FAIL, "unable to create object reference")
    } else {
        H5S_t  *space = NULL; /* Pointer to dataspace containing region */
        size_t buf_size = H5R_DSET_REG_REF_BUF_SIZE;

        /* Currently restrict API usage to native VOL */
        if(vol_obj->connector->cls->value != H5_VOL_NATIVE)
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "unsupported VOL class")

        if(space_id == H5I_BADID)
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "reference region dataspace id must be valid")
        if(NULL == (space = (struct H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE)))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataspace")

        if((ret_value = H5R__encode_addr_region_compat(loc_id, obj_addr, space, buf, &buf_size)) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCREATE, FAIL, "unable to create region reference")
    }

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Rcreate() */


/*-------------------------------------------------------------------------
 * Function:    H5Rget_obj_type2
 *
 * Purpose: Given a reference to some object, this function returns the type
 * of object pointed to.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Rget_obj_type2(hid_t id, H5R_type_t ref_type, const void *ref,
    H5O_type_t *obj_type)
{
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    haddr_t obj_addr;                   /* Object address */
    const unsigned char *buf = (const unsigned char *)ref; /* Reference pointer */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iRt*x*Ot", id, ref_type, ref, obj_type);

    /* Check args */
    if(buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(ref_type != H5R_OBJECT1 && ref_type != H5R_DATASET_REGION1)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference type")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier");

    if(ref_type == H5R_OBJECT1) {
        size_t buf_size = H5R_OBJ_REF_BUF_SIZE;

        /* Get object address */
        if(H5R__decode_obj_addr_compat(buf, &buf_size, &obj_addr) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "unable to get object address")
    } else {
        size_t buf_size = H5R_DSET_REG_REF_BUF_SIZE;

        /* Currently restrict API usage to native VOL */
        if(vol_obj->connector->cls->value != H5_VOL_NATIVE)
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "unsupported VOL class")

        /* Get object address */
        if(H5R__decode_addr_region_compat(id, buf, &buf_size, &obj_addr, NULL) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "unable to get object address")
    }

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5I_get_type(id);

    /* Retrieve object's type */
    if(H5VL_object_get(vol_obj, &loc_params, H5VL_OBJECT_GET_TYPE, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, obj_type) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTGET, FAIL, "can't retrieve object type")

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Rget_obj_type2() */


/*-------------------------------------------------------------------------
 * Function:    H5Rdereference2
 *
 * Purpose: Given a reference to some object, open that object and return an
 * ID for that object.
 *
 * Return:  Valid ID on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Rdereference2(hid_t obj_id, hid_t oapl_id, H5R_type_t ref_type,
    const void *ref)
{
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    haddr_t obj_addr;                   /* Object address */
    H5I_type_t opened_type;             /* Opened object type */
    void *opened_obj = NULL;            /* Opened object */
    const unsigned char *buf = (const unsigned char *)ref; /* Reference pointer */
    hid_t ret_value = H5I_INVALID_HID;  /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE4("i", "iiRt*x", obj_id, oapl_id, ref_type, ref);

    /* Check args */
    if(oapl_id < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "not a property list")
    if(buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference pointer")
    if(ref_type != H5R_OBJECT1 && ref_type != H5R_DATASET_REGION1)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference type")

    /* Verify access property list and set up collective metadata if appropriate */
    if(H5CX_set_apl(&oapl_id, H5P_CLS_DACC, obj_id, FALSE) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "can't set access property list info")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(obj_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid file identifier")

    if(ref_type == H5R_OBJECT1) {
        size_t buf_size = H5R_OBJ_REF_BUF_SIZE;

        /* Get object address */
        if(H5R__decode_obj_addr_compat(buf, &buf_size, &obj_addr) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, H5I_INVALID_HID, "unable to get object address")
    } else {
        size_t buf_size = H5R_DSET_REG_REF_BUF_SIZE;

        /* Currently restrict API usage to native VOL */
        if(vol_obj->connector->cls->value != H5_VOL_NATIVE)
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, H5I_INVALID_HID, "unsupported VOL class")

        /* Get object address */
        if(H5R__decode_addr_region_compat(obj_id, buf, &buf_size, &obj_addr, NULL) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, H5I_INVALID_HID, "unable to get object address")
    }

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5I_get_type(obj_id);

    /* Open object by address */
    if(NULL == (opened_obj = H5VL_object_open(vol_obj, &loc_params, &opened_type, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTOPENOBJ, H5I_INVALID_HID, "unable to open object by address")

    /* Register object */
    if((ret_value = H5VL_register(opened_type, opened_obj, vol_obj->connector, TRUE)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register object handle")

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Rdereference2() */


/*-------------------------------------------------------------------------
 * Function:    H5Rget_region
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
H5Rget_region(hid_t id, H5R_type_t ref_type, const void *ref)
{
    H5VL_object_t *vol_obj = NULL;  /* Object token of loc_id */
    size_t buf_size = H5R_DSET_REG_REF_BUF_SIZE;    /* Reference buffer size */
    H5S_t *space = NULL;            /* Dataspace object */
    const unsigned char *buf = (const unsigned char *)ref; /* Reference pointer */
    hid_t ret_value;                /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE3("i", "iRt*x", id, ref_type, ref);

    /* Check args */
    if(buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference pointer")
    if(ref_type != H5R_DATASET_REGION1)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference type")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid file identifier")

    /* Currently restrict API usage to native VOL */
    if(vol_obj->connector->cls->value != H5_VOL_NATIVE)
        HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, H5I_INVALID_HID, "unsupported VOL class")

    /* Get the dataspace with the correct region selected */
    if(H5R__decode_addr_region_compat(id, buf, &buf_size, NULL, &space) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, H5I_INVALID_HID, "unable to get dataspace")

    /* Atomize */
    if((ret_value = H5I_register(H5I_DATASPACE, space, TRUE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register dataspace atom")

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Rget_region1() */


/*-------------------------------------------------------------------------
 * Function:    H5Rget_name
 *
 * Purpose: Given a reference to some object, determine a path to the object
 * referenced in the file.
 *
 * Return:  Non-negative length of the path on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5Rget_name(hid_t id, H5R_type_t ref_type, const void *ref, char *name,
    size_t size)
{
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    haddr_t obj_addr;                   /* Object address */
    const unsigned char *buf = (const unsigned char *)ref; /* Reference pointer */
    ssize_t ret_value = -1;  /* Return value */

    FUNC_ENTER_API((-1))
    H5TRACE5("Zs", "iRt*x*sz", id, ref_type, ref, name, size);

    /* Check args */
    if(buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference pointer")
    if(ref_type != H5R_OBJECT1 && ref_type != H5R_DATASET_REGION1)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, (-1), "invalid reference type")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, (-1), "invalid file identifier")

    if(ref_type == H5R_OBJECT1) {
        size_t buf_size = H5R_OBJ_REF_BUF_SIZE;

        /* Get object address */
        if(H5R__decode_obj_addr_compat(buf, &buf_size, &obj_addr) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, (-1), "unable to get object address")
    } else {
        size_t buf_size = H5R_DSET_REG_REF_BUF_SIZE;

        /* Currently restrict API usage to native VOL */
        if(vol_obj->connector->cls->value != H5_VOL_NATIVE)
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, (-1), "unsupported VOL class")

        /* Get object address */
        if(H5R__decode_addr_region_compat(id, buf, &buf_size, &obj_addr, NULL) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, (-1), "unable to get object address")
    }

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5I_get_type(id);

    /* Retrieve object's name */
    if(H5VL_object_get(vol_obj, &loc_params, H5VL_OBJECT_GET_NAME, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &ret_value, name, size) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTGET, (-1), "can't retrieve object name")

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Rget_name() */

#ifndef H5_NO_DEPRECATED_SYMBOLS

/*-------------------------------------------------------------------------
 * Function:    H5Rget_obj_type1
 *
 * Purpose: Retrieves the type of the object that an object points to.
 *
 * Return: Object type (as defined in H5Gpublic.h) on success
 *         H5G_UNKNOWN on failure
 *
 *-------------------------------------------------------------------------
 */
H5G_obj_t
H5Rget_obj_type1(hid_t id, H5R_type_t ref_type, const void *ref)
{
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    haddr_t obj_addr;                   /* Object address */
    H5O_type_t obj_type;                /* Object type */
    const unsigned char *buf = (const unsigned char *)ref; /* Reference buffer */
    H5G_obj_t ret_value;                /* Return value */

    FUNC_ENTER_API(H5G_UNKNOWN)
    H5TRACE3("Go", "iRt*x", id, ref_type, ref);

    /* Check args */
    if(buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5G_UNKNOWN, "invalid reference pointer")
    if(ref_type != H5R_OBJECT1 && ref_type != H5R_DATASET_REGION1)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5G_UNKNOWN, "invalid reference type")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5G_UNKNOWN, "invalid location identifier")

    if(ref_type == H5R_OBJECT1) {
        size_t buf_size = H5R_OBJ_REF_BUF_SIZE;

        /* Get object address */
        if(H5R__decode_obj_addr_compat(buf, &buf_size, &obj_addr) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, H5G_UNKNOWN, "unable to get object address")
    } else {
        size_t buf_size = H5R_DSET_REG_REF_BUF_SIZE;

        /* Currently restrict API usage to native VOL */
        if(vol_obj->connector->cls->value != H5_VOL_NATIVE)
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, H5G_UNKNOWN, "unsupported VOL class")

        /* Get object address */
        if(H5R__decode_addr_region_compat(id, buf, &buf_size, &obj_addr, NULL) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, H5G_UNKNOWN, "unable to get object address")
    }

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5Iget_type(id);

    /* Retrieve object's type */
    if(H5VL_object_get(vol_obj, &loc_params, H5VL_OBJECT_GET_TYPE, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL, &obj_type) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTGET, H5G_UNKNOWN, "can't retrieve object type")

    /* Set return value */
    ret_value = H5G_map_obj_type(obj_type);

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Rget_obj_type1() */


/*-------------------------------------------------------------------------
 * Function:    H5Rdereference1
 *
 * Purpose: Given a reference to some object, open that object and return an
 * ID for that object.
 *
 * Return:  Valid ID on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Rdereference1(hid_t obj_id, H5R_type_t ref_type, const void *ref)
{
    H5VL_object_t *vol_obj = NULL;      /* Object token of loc_id */
    H5VL_loc_params_t loc_params;       /* Location parameters */
    haddr_t obj_addr;                   /* Object address */
    H5I_type_t opened_type;             /* Opened object type */
    void *opened_obj = NULL;            /* Opened object */
    const unsigned char *buf = (const unsigned char *)ref; /* Reference buffer */
    hid_t ret_value = H5I_INVALID_HID;  /* Return value */

    FUNC_ENTER_API(H5I_INVALID_HID)
    H5TRACE3("i", "iRt*x", obj_id, ref_type, ref);

    /* Check args */
    if(buf == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference pointer")
    if(ref_type != H5R_OBJECT1 && ref_type != H5R_DATASET_REGION1)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, H5I_INVALID_HID, "invalid reference type")

    /* Get the VOL object */
    if(NULL == (vol_obj = H5VL_vol_object(obj_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, H5I_INVALID_HID, "invalid location identifier")

    if(ref_type == H5R_OBJECT1) {
        size_t buf_size = H5R_OBJ_REF_BUF_SIZE;

        /* Get object address */
        if(H5R__decode_obj_addr_compat(buf, &buf_size, &obj_addr) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, H5I_INVALID_HID, "unable to get object address")
    } else {
        size_t buf_size = H5R_DSET_REG_REF_BUF_SIZE;

        /* Currently restrict API usage to native VOL */
        if(vol_obj->connector->cls->value != H5_VOL_NATIVE)
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, H5I_INVALID_HID, "unsupported VOL class")

        /* Get object address */
        if(H5R__decode_addr_region_compat(obj_id, buf, &buf_size, &obj_addr, NULL) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, H5I_INVALID_HID, "unable to get object address")
    }

    /* Set location parameters */
    loc_params.type = H5VL_OBJECT_BY_ADDR;
    loc_params.loc_data.loc_by_addr.addr = obj_addr;
    loc_params.obj_type = H5I_get_type(obj_id);

    /* Dereference */
    if(NULL == (opened_obj = H5VL_object_open(vol_obj, &loc_params, &opened_type, H5P_DATASET_XFER_DEFAULT, H5_REQUEST_NULL)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTOPENOBJ, H5I_INVALID_HID, "unable to open object by address")

    /* Register object */
    if((ret_value = H5VL_register(opened_type, opened_obj, vol_obj->connector, TRUE)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTREGISTER, H5I_INVALID_HID, "unable to register object handle")

done:
    FUNC_LEAVE_API(ret_value)
}   /* end H5Rdereference1() */

#endif /* H5_NO_DEPRECATED_SYMBOLS */
