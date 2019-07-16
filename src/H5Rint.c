/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
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

#include "H5Rmodule.h"          /* This source code file is part of the H5R module */


/***********/
/* Headers */
/***********/
#include "H5private.h"          /* Generic Functions                        */
#include "H5ACprivate.h"        /* Metadata cache                           */
#include "H5CXprivate.h"        /* API Contexts                             */
#include "H5Dprivate.h"         /* Datasets                                 */
#include "H5Eprivate.h"         /* Error handling                           */
#include "H5Gprivate.h"         /* Groups                                   */
#include "H5HGprivate.h"        /* Global Heaps                             */
#include "H5Iprivate.h"         /* IDs                                      */
#include "H5MMprivate.h"        /* Memory management                        */
#include "H5Oprivate.h"         /* Object headers                           */
#include "H5Rpkg.h"             /* References                               */
#include "H5Sprivate.h"         /* Dataspaces                               */
#include "H5Tprivate.h"         /* Datatypes                                */


/****************/
/* Local Macros */
/****************/

#define H5R_MAX_STRING_LEN  (2 << 15)   /* Max encoded string length    */

/* Size of an address in this file, plus 4 bytes for the size of a heap ID */
#define H5R_ENCODE_HEAP_SIZE(f) (H5_SIZEOF_UINT32_T + (size_t)H5F_SIZEOF_ADDR(f))

/* Debug */
//#define H5R_DEBUG
#ifdef H5R_DEBUG
#define H5R_LOG_DEBUG(...) do {                                 \
      fprintf(stdout, " # %s(): ", __func__);                   \
      fprintf(stdout, __VA_ARGS__);                             \
      fprintf(stdout, "\n");                                    \
      fflush(stdout);                                           \
  } while (0)
#else
#define H5R_LOG_DEBUG(...) do { } while (0)
#endif

/******************/
/* Local Typedefs */
/******************/

#define H5R_FIXED_REF_SIZE 0x01
#define H5R_HAS_ASCII      0x02


struct href_encode_header {
    uint32_t version;
    uint32_t flags;

};


/********************/
/* Local Prototypes */
/********************/

static herr_t H5R__encode_string(const char *string, unsigned char *buf,
    size_t *nalloc);
static herr_t H5R__decode_string(const unsigned char *buf, size_t *nbytes,
    char **string_ptr);

/*********************/
/* Package Variables */
/*********************/

/* Package initialization variable */
hbool_t H5_PKG_INIT_VAR = FALSE;

/*****************************/
/* Library Private Variables */
/*****************************/

/*******************/
/* Local Variables */
/*******************/

/* Flag indicating "top" of interface has been initialized */
static hbool_t H5R_top_package_initialize_s = FALSE;


/*--------------------------------------------------------------------------
NAME
   H5R__init_package -- Initialize interface-specific information
USAGE
    herr_t H5R__init_package()

RETURNS
    Non-negative on success/Negative on failure
DESCRIPTION
    Initializes any interface-specific data or routines.

--------------------------------------------------------------------------*/
herr_t
H5R__init_package(void)
{
    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* Mark "top" of interface as initialized */
    H5R_top_package_initialize_s = TRUE;

    /* Sanity check */
    HDcompile_assert(sizeof(struct href) <= H5R_REF_BUF_SIZE);

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5R__init_package() */


/*--------------------------------------------------------------------------
 NAME
    H5R_top_term_package
 PURPOSE
    Terminate various H5R objects
 USAGE
    void H5R_top_term_package()
 RETURNS
    void
 DESCRIPTION
    Release IDs for the atom group, deferring full interface shutdown
    until later (in H5R_term_package).
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
     Can't report errors...
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int
H5R_top_term_package(void)
{
    int	n = 0;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* Mark closed if initialized */
    if(H5R_top_package_initialize_s)
        if(0 == n)
            H5R_top_package_initialize_s = FALSE;

    FUNC_LEAVE_NOAPI(n)
} /* end H5R_top_term_package() */


/*--------------------------------------------------------------------------
 NAME
    H5R_term_package
 PURPOSE
    Terminate various H5R objects
 USAGE
    void H5R_term_package()
 RETURNS
    void
 DESCRIPTION
    Release the atom group and any other resources allocated.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
     Can't report errors...

     Finishes shutting down the interface, after H5R_top_term_package()
     is called
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int
H5R_term_package(void)
{
    int	n = 0;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if(H5_PKG_INIT_VAR) {
        /* Sanity checks */
        HDassert(FALSE == H5R_top_package_initialize_s);

        /* Mark closed */
        if(0 == n)
            H5_PKG_INIT_VAR = FALSE;
    }

    FUNC_LEAVE_NOAPI(n)
} /* end H5R_term_package() */


/*-------------------------------------------------------------------------
 * Function:    H5R__create_object
 *
 * Purpose: Creates an object reference.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__create_object(haddr_t obj_addr, struct href *ref)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_PACKAGE

    HDassert(ref);

    /* Create new reference */
    ref->loc_id = H5I_INVALID_HID;
    ref->type = H5R_OBJECT2;
    ref->ref.obj.filename = NULL;
    ref->ref.obj.addr = obj_addr;

    /* Cache encoding size */
    if(H5R__encode(ref, NULL, &ref->encode_size) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "unable to determine encoding size")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__create_object() */


/*-------------------------------------------------------------------------
 * Function:    H5R__create_region
 *
 * Purpose: Creates a region reference.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__create_region(haddr_t obj_addr, H5S_t *space, struct href *ref)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_PACKAGE

    HDassert(space);
    HDassert(ref);

    /* Create new reference */
    ref->loc_id = H5I_INVALID_HID;
    ref->type = H5R_DATASET_REGION2;
    ref->ref.reg.obj.filename = NULL;
    ref->ref.reg.obj.addr = obj_addr;
    if(NULL == (ref->ref.reg.space = H5S_copy(space, FALSE, TRUE)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, FAIL, "unable to copy dataspace")

    /* Cache encoding size */
    if(H5R__encode(ref, NULL, &ref->encode_size) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "unable to determine encoding size")

done:
    if(ret_value < 0) {
        if(ref->ref.reg.space) {
            H5S_close(ref->ref.reg.space);
            ref->ref.reg.space = NULL;
        }
    }
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5R__create_region */


/*-------------------------------------------------------------------------
 * Function:    H5R__create_attr
 *
 * Purpose: Creates an attribute reference.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__create_attr(haddr_t obj_addr, const char *attr_name, struct href *ref)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_PACKAGE

    HDassert(attr_name);
    HDassert(ref);

    /* Make sure that attribute name is not longer than supported encode size */
    if(HDstrlen(attr_name) > H5R_MAX_STRING_LEN)
        HGOTO_ERROR(H5E_REFERENCE, H5E_ARGS, FAIL, "attribute name too long (%d > %d)", (int)HDstrlen(attr_name), H5R_MAX_STRING_LEN)

    /* Create new reference */
    ref->loc_id = H5I_INVALID_HID;
    ref->type = H5R_ATTR;
    ref->ref.attr.obj.filename = NULL;
    ref->ref.attr.obj.addr = obj_addr;
    if(NULL == (ref->ref.attr.name = HDstrdup(attr_name)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, FAIL, "Cannot copy attribute name")

    /* Cache encoding size */
    if(H5R__encode(ref, NULL, &ref->encode_size) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "unable to determine encoding size")

    H5R_LOG_DEBUG("Created attribute reference, %d", sizeof(struct href));

done:
    if(ret_value < 0) {
        ref->ref.attr.name = H5MM_xfree(ref->ref.attr.name);
    }
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5R__create_attr */


/*-------------------------------------------------------------------------
 * Function:    H5R__destroy
 *
 * Purpose: Destroy reference.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__destroy(struct href *ref)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_PACKAGE

    HDassert(ref != NULL);

    switch(ref->type) {
        case H5R_OBJECT2:
            ref->ref.obj.filename = H5MM_xfree(ref->ref.obj.filename);
            break;
        case H5R_DATASET_REGION2:
            if(H5S_close(ref->ref.reg.space) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTFREE, FAIL, "Cannot close dataspace")
            ref->ref.reg.space = NULL;
            ref->ref.reg.obj.filename = H5MM_xfree(ref->ref.reg.obj.filename);
            break;
        case H5R_ATTR:
            ref->ref.attr.obj.filename = H5MM_xfree(ref->ref.attr.obj.filename);
            ref->ref.attr.name = H5MM_xfree(ref->ref.attr.name);
            break;
        case H5R_OBJECT1:
        case H5R_DATASET_REGION1:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    /* Decrement refcount of attached loc_id */
    if((ref->loc_id != H5I_INVALID_HID) && (H5I_dec_ref(ref->loc_id) < 0))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDEC, FAIL, "decrementing location ID failed")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__destroy() */


/*-------------------------------------------------------------------------
 * Function:    H5R__set_loc_id
 *
 * Purpose: Attach location ID to reference and increment location refcount.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__set_loc_id(struct href *ref, hid_t id)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_PACKAGE

    HDassert(ref != NULL);
    HDassert(id != H5I_INVALID_HID);

    /* If a location ID was previously assigned, decrement refcount and assign new one */
    if((ref->loc_id != H5I_INVALID_HID) && (H5I_dec_ref(ref->loc_id) < 0))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDEC, FAIL, "decrementing location ID failed")
    ref->loc_id = id;
    /* Prevent location ID from being freed until reference is destroyed */
    if(H5I_inc_ref(ref->loc_id, FALSE) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINC, FAIL, "incrementing location ID failed")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__set_loc_id() */


/*-------------------------------------------------------------------------
 * Function:    H5R__get_loc_id
 *
 * Purpose: Retrieve location ID attached to existing reference.
 *
 * Return:  Valid ID on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5R__get_loc_id(const struct href *ref)
{
    hid_t ret_value = H5I_INVALID_HID;  /* Return value */

    FUNC_ENTER_PACKAGE_NOERR

    HDassert(ref != NULL);

    ret_value = ref->loc_id;

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_loc_id() */


/*-------------------------------------------------------------------------
 * Function:    H5R__get_type
 *
 * Purpose: Given a reference to some object, return the type of that reference.
 *
 * Return:  Type of the reference
 *
 *-------------------------------------------------------------------------
 */
H5R_type_t
H5R__get_type(const struct href *ref)
{
    H5R_type_t ret_value = H5R_BADTYPE;

    FUNC_ENTER_PACKAGE_NOERR

    HDassert(ref != NULL);
    ret_value = ref->type;

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_type() */


/*-------------------------------------------------------------------------
 * Function:    H5R__equal
 *
 * Purpose: Compare two references
 *
 * Return:  TRUE if equal, FALSE if unequal, FAIL if error
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5R__equal(const struct href *ref1, const struct href *ref2)
{
    htri_t ret_value = TRUE;

    FUNC_ENTER_PACKAGE

    HDassert(ref1 != NULL);
    HDassert(ref2 != NULL);

    if(ref1->type != ref2->type)
        HGOTO_DONE(FALSE);

    switch(ref1->type) {
        case H5R_OBJECT2:
            if(ref1->ref.obj.addr != ref2->ref.obj.addr)
                HGOTO_DONE(FALSE);
            break;
        case H5R_DATASET_REGION2:
            if(ref1->ref.reg.obj.addr != ref2->ref.reg.obj.addr)
                HGOTO_DONE(FALSE);
            if((ret_value = H5S_extent_equal(ref1->ref.reg.space, ref2->ref.reg.space)) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOMPARE, FAIL, "cannot compare dataspace extents")
            break;
        case H5R_ATTR:
            if(ref1->ref.attr.obj.addr != ref2->ref.attr.obj.addr)
                HGOTO_DONE(FALSE);
            if(0 != HDstrcmp(ref1->ref.attr.name, ref2->ref.attr.name))
                HGOTO_DONE(FALSE);
            break;
        case H5R_OBJECT1:
        case H5R_DATASET_REGION1:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__equal() */


/*-------------------------------------------------------------------------
 * Function:    H5R__copy
 *
 * Purpose: Copy a reference
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__copy(const struct href *src_ref, struct href *dest_ref)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert((src_ref != NULL) && (dest_ref != NULL));

    /* Allocate the space to store the serialized information */
    dest_ref->type = src_ref->type;

    switch(src_ref->type) {
        case H5R_OBJECT2:
            dest_ref->ref.obj.addr = src_ref->ref.obj.addr;
            break;
        case H5R_DATASET_REGION2:
            dest_ref->ref.reg.obj.addr = src_ref->ref.reg.obj.addr;
            if(NULL == (dest_ref->ref.reg.space = H5S_copy(src_ref->ref.reg.space, FALSE, TRUE)))
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, FAIL, "unable to copy dataspace")
            break;
        case H5R_ATTR:
            dest_ref->ref.attr.obj.addr = src_ref->ref.attr.obj.addr;
            if(NULL == (dest_ref->ref.attr.name = HDstrdup(src_ref->ref.attr.name)))
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, FAIL, "Cannot copy attribute name")
            break;
        case H5R_OBJECT1:
        case H5R_DATASET_REGION1:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    /* Set location ID */
    if(H5R__set_loc_id(dest_ref, src_ref->loc_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "cannot set reference location ID")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__copy() */


/*-------------------------------------------------------------------------
 * Function:    H5R__get_obj_addr
 *
 * Purpose: Given a reference to some object, get the encoded object addr.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__get_obj_addr(const struct href *ref, haddr_t *obj_addr_ptr)
{
    haddr_t obj_addr;           /* Object address */
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_PACKAGE

    HDassert(ref != NULL);
    HDassert(obj_addr_ptr);

    switch (ref->type) {
        case H5R_OBJECT2:
            obj_addr = ref->ref.obj.addr;
            break;
        case H5R_DATASET_REGION2:
            obj_addr = ref->ref.reg.obj.addr;
            break;
        case H5R_ATTR:
            obj_addr = ref->ref.attr.obj.addr;
            break;
        case H5R_OBJECT1:
        case H5R_DATASET_REGION1:
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    *obj_addr_ptr = obj_addr;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_object() */


/*-------------------------------------------------------------------------
 * Function:    H5R__get_region
 *
 * Purpose: Given a reference to some object, creates a copy of the dataset
 * pointed to's dataspace and defines a selection in the copy which is the
 * region pointed to.
 *
 * Return:  Pointer to the dataspace on success/NULL on failure
 *
 *-------------------------------------------------------------------------
 */
H5S_t *
H5R__get_region(const struct href *ref)
{
    H5S_t *ret_value = NULL;    /* Return value */

    FUNC_ENTER_PACKAGE

    HDassert(ref != NULL);

    switch (ref->type) {
        case H5R_DATASET_REGION2:
            if(NULL == (ret_value = H5S_copy(ref->ref.reg.space, FALSE, TRUE)))
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, NULL, "unable to copy dataspace")
            break;

        case H5R_OBJECT1:
        case H5R_OBJECT2:
        case H5R_DATASET_REGION1:
        case H5R_ATTR:
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, NULL, "internal error (unknown reference type)")
    } /* end switch */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_region() */


/*-------------------------------------------------------------------------
 * Function:    H5R__get_file_name
 *
 * Purpose: Given a reference to some object, determine a file name of the
 * object located into.
 *
 * Return:  Non-negative length of the path on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5R__get_file_name(const struct href *ref, char *name, size_t size)
{
    char *filename;
    size_t copy_len;
    ssize_t ret_value = -1;     /* Return value */

    FUNC_ENTER_PACKAGE

    /* Check args */
    HDassert(ref != NULL);

    switch (ref->type) {
        case H5R_OBJECT2:
            filename = ref->ref.obj.filename;
            break;
        case H5R_DATASET_REGION2:
            filename = ref->ref.reg.obj.filename;
            break;
        case H5R_ATTR:
            filename = ref->ref.attr.obj.filename;
            break;
        case H5R_OBJECT1:
        case H5R_DATASET_REGION1:
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    /* Get the file name length */
    copy_len = HDstrlen(filename);

    /* Copy the file name */
    if (name) {
        copy_len = MIN(copy_len, size - 1);
        HDmemcpy(name, filename, copy_len);
        name[copy_len] = '\0';
    }
    ret_value = (ssize_t)(copy_len + 1);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_file_name() */


/*-------------------------------------------------------------------------
 * Function:    H5R__get_attr_name
 *
 * Purpose: Given a reference to some attribute, determine its name.
 *
 * Return:  Non-negative length of the path on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5R__get_attr_name(const struct href *ref, char *name, size_t size)
{
    ssize_t ret_value = -1;     /* Return value */
    size_t attr_name_len;       /* Length of the attribute name */

    FUNC_ENTER_PACKAGE_NOERR

    /* Check args */
    HDassert(ref != NULL);
    HDassert(ref->type == H5R_ATTR);

    /* Get the attribute name length */
    attr_name_len = HDstrlen(ref->ref.attr.name);
    HDassert(attr_name_len <= H5R_MAX_STRING_LEN);

    /* Get the attribute name */
    if (name) {
        size_t copy_len = MIN(attr_name_len, size - 1);
        HDmemcpy(name, ref->ref.attr.name, copy_len);
        name[copy_len] = '\0';
    }

    ret_value = (ssize_t)(attr_name_len + 1);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_attr_name() */


/*-------------------------------------------------------------------------
 * Function:    H5R__encode
 *
 * Purpose: Private function for H5Rencode.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__encode(const struct href *ref, unsigned char *buf, size_t *nalloc)
{
    uint8_t *p = (uint8_t *)buf;
    size_t buf_size, buf_size_left = 0, enc_size = 0;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(ref);
    HDassert(nalloc);

    /* Protocol version | Additional header? | Ref type | Obj address | Etc */
    buf_size = 2 * sizeof(uint8_t);

    /* Don't encode if buffer size isn't big enough or buffer is empty */
    if(buf && *nalloc >= buf_size) {
        /* Encode the protocol version */
        *p++ = (uint8_t)H5R_ENCODE_VERSION;
        /* Encode the type of the information */
        *p++ = (uint8_t)ref->type;

        buf_size_left = *nalloc - buf_size;
    }

    switch(ref->type) {
        case H5R_OBJECT2:
            enc_size = buf_size_left;
            if(H5R__encode_obj_addr(ref->ref.obj.addr, p, &enc_size) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "Cannot encode object address")
            if(p && buf_size_left >= enc_size) {
                p += enc_size;
                buf_size_left -= enc_size;
            }
            buf_size += enc_size;
            break;
        case H5R_DATASET_REGION2:
            enc_size = buf_size_left;
            if(H5R__encode_obj_addr(ref->ref.reg.obj.addr, p, &enc_size) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "Cannot encode object address")
            if(p && buf_size_left >= enc_size) {
                p += enc_size;
                buf_size_left -= enc_size;
            }
            buf_size += enc_size;

            enc_size = buf_size_left;
            if(H5R__encode_region(ref->ref.reg.space, p, &enc_size) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "Cannot encode region")
            if(p && buf_size_left >= enc_size) {
                p += enc_size;
                buf_size_left -= enc_size;
            }
            buf_size += enc_size;
            break;
        case H5R_ATTR:
            enc_size = buf_size_left;
            if(H5R__encode_obj_addr(ref->ref.attr.obj.addr, p, &enc_size) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "Cannot encode object address")
            if(p && buf_size_left >= enc_size) {
                p += enc_size;
                buf_size_left -= enc_size;
            }
            buf_size += enc_size;

            enc_size = buf_size_left;
            if(H5R__encode_string(ref->ref.attr.name, p, &enc_size) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "Cannot encode attribute name")
            if(p && buf_size_left >= enc_size) {
                p += enc_size;
                buf_size_left -= enc_size;
            }
            buf_size += enc_size;
            break;
        case H5R_OBJECT1:
            /* Already encoded, nothing to do */
        case H5R_DATASET_REGION1:
            /* Already encoded, nothing to do */
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    *nalloc = buf_size;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__encode() */


/*-------------------------------------------------------------------------
 * Function:    H5R__decode
 *
 * Purpose: Private function for H5Rdecode.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__decode(const unsigned char *buf, size_t *nbytes, struct href *ref)
{
    const uint8_t *p = (const uint8_t *)buf;
    size_t buf_size, buf_size_left, dec_size = 0;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(buf);
    HDassert(nbytes);
    HDassert(ref);

    /* Don't decode if buffer size isn't big enough */
    if(*nbytes < 2 * sizeof(uint8_t))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Buffer size is too small")

    /* Verify protocol version */
    if(*p++ != (uint8_t)H5R_ENCODE_VERSION)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Protocol version does not match")

    /* Set new reference */
    ref->type = (H5R_type_t)*p++;
    if(ref->type <= H5R_BADTYPE || ref->type >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference type")

    buf_size_left = *nbytes - 2 * sizeof(uint8_t);
    dec_size = buf_size_left;

    switch(ref->type) {
        case H5R_OBJECT2:
            if(H5R__decode_obj_addr(p, &dec_size, &ref->ref.obj.addr) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Cannot decode object address")
            buf_size_left -= dec_size;
            ref->ref.obj.filename = NULL;
            break;
        case H5R_DATASET_REGION2:
            if(H5R__decode_obj_addr(p, &dec_size, &ref->ref.reg.obj.addr) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Cannot decode object address")
            buf_size_left -= dec_size;
            p += dec_size;
            dec_size = buf_size_left;
            ref->ref.reg.obj.filename = NULL;
            if(NULL == (ref->ref.reg.space = H5R__decode_region(p, &dec_size)))
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Cannot decode region")
            break;
        case H5R_ATTR:
            if(H5R__decode_obj_addr(p, &dec_size, &ref->ref.attr.obj.addr) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Cannot decode object address")
            buf_size_left -= dec_size;
            p += dec_size;
            dec_size = buf_size_left;
            ref->ref.attr.obj.filename = NULL;
            if(H5R__decode_string(p, &dec_size, &ref->ref.attr.name) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Cannot decode attribute name")
            break;
        case H5R_OBJECT1:
        case H5R_DATASET_REGION1:
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    /* TODO should check if already set ? */
    ref->loc_id = H5I_INVALID_HID;

    /* TODO */
    ref->encode_size = 0;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__decode() */


/*-------------------------------------------------------------------------
 * Function:    H5R__encode_obj_addr
 *
 * Purpose: Encode an object address.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__encode_obj_addr(haddr_t addr, unsigned char *buf, size_t *nalloc)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE_NOERR

    HDassert(nalloc);
    /* haddr_t are always 64-bit */
    HDassert(sizeof(addr) == sizeof(uint64_t));

    /* Don't encode if buffer size isn't big enough or buffer is empty */
    if(buf && *nalloc >= sizeof(addr)) {
        uint8_t *p = (uint8_t *)buf;

        UINT64ENCODE(p, addr);
    }
    *nalloc = sizeof(addr);

    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__decode_obj_addr
 *
 * Purpose: Decode an object address.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__decode_obj_addr(const unsigned char *buf, size_t *nbytes, haddr_t *addr_ptr)
{
    const uint8_t *p = (const uint8_t *)buf;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(buf);
    HDassert(nbytes);
    HDassert(addr_ptr);
    /* haddr_t are always 64-bit */
    HDassert(sizeof(*addr_ptr) == sizeof(uint64_t));

    /* Don't decode if buffer size isn't big enough */
    if(*nbytes < sizeof(*addr_ptr))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Buffer size is too small")

    UINT64DECODE(p, *addr_ptr);

    *nbytes = sizeof(*addr_ptr);

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__encode_region
 *
 * Purpose: Encode a selection.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__encode_region(H5S_t *space, unsigned char *buf, size_t *nalloc)
{
    uint8_t *p = NULL; /* Pointer to data to store */
    size_t buf_size = 0;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(space);
    HDassert(nalloc);

    /* Get the amount of space required to serialize the selection */
    if(H5S_encode(space, (unsigned char **)&p, &buf_size) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "Cannot determine amount of space needed for serializing selection")

    /* Don't encode if buffer size isn't big enough or buffer is empty */
    if(buf && *nalloc >= (buf_size + sizeof(uint32_t))) {
        p = (uint8_t *)buf;

        /* Encode the size for safety check */
        UINT32ENCODE(p, (uint32_t)buf_size);

        /* Serialize the selection */
        if (H5S_encode(space, (unsigned char **)&p, &buf_size) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "can't serialize selection")
    }
    *nalloc = buf_size + sizeof(uint32_t);

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__decode_region
 *
 * Purpose: Decode a selection.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
H5S_t *
H5R__decode_region(const unsigned char *buf, size_t *nbytes)
{
    const uint8_t *p = (const uint8_t *)buf;
    size_t buf_size = 0;
    H5S_t *ret_value = NULL;

    FUNC_ENTER_PACKAGE

    HDassert(buf);
    HDassert(nbytes);

    /* Don't decode if buffer size isn't big enough */
    if(*nbytes < sizeof(uint32_t))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, NULL, "Buffer size is too small")

    /* Decode the selection size */
    UINT32DECODE(p, buf_size);
    buf_size += sizeof(uint32_t);

    /* Don't decode if buffer size isn't big enough */
    if(*nbytes < buf_size)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, NULL, "Buffer size is too small")

    /* Deserialize the selection */
    if (NULL == (ret_value = H5S_decode(&p)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, NULL, "can't deserialize selection")

    *nbytes = buf_size;

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__encode_string
 *
 * Purpose: Encode a string.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5R__encode_string(const char *string, unsigned char *buf, size_t *nalloc)
{
    size_t string_len, buf_size;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(string);
    HDassert(nalloc);

    /* Get the amount of space required to serialize the string */
    string_len = HDstrlen(string);
    if(string_len > H5R_MAX_STRING_LEN)
        HGOTO_ERROR(H5E_REFERENCE, H5E_ARGS, FAIL, "string too long")

    /* Compute buffer size, allow for the attribute name length and object address */
    buf_size = string_len + sizeof(uint16_t);

    if(buf && *nalloc >= buf_size) {
        uint8_t *p = (uint8_t *)buf;
        /* Serialize information for string length into the buffer */
        UINT16ENCODE(p, string_len);
        /* Copy the string into the buffer */
        HDmemcpy(p, string, string_len);
    }
    *nalloc = buf_size;

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__decode_string
 *
 * Purpose: Decode a string.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5R__decode_string(const unsigned char *buf, size_t *nbytes, char **string_ptr)
{
    const uint8_t *p = (const uint8_t *)buf;
    size_t string_len;
    char *string = NULL;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(buf);
    HDassert(nbytes);
    HDassert(string_ptr);

    /* Don't decode if buffer size isn't big enough */
    if(*nbytes < sizeof(uint16_t))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Buffer size is too small")

    /* Get the string length */
    UINT16DECODE(p, string_len);
    HDassert(string_len <= H5R_MAX_STRING_LEN);

    /* Allocate the string */
    if (NULL == (string = H5MM_malloc(string_len + 1)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, FAIL, "Cannot allocate string")

     /* Copy the string */
     HDmemcpy(string, p, string_len);
     string[string_len] = '\0';

     *string_ptr = string;
     *nbytes = sizeof(uint16_t) + string_len;

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__encode_heap
 *
 * Purpose: Encode data and insert into heap (native only).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__encode_heap(H5F_t *f, unsigned char *buf, size_t *nalloc,
    const unsigned char *data, size_t data_size)
{
    size_t buf_size;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(f);
    HDassert(nalloc);

    buf_size = H5R_ENCODE_HEAP_SIZE(f);
    if(buf && *nalloc >= buf_size) {
        H5HG_t hobjid;
        uint8_t *p = (uint8_t *)buf;

        /* Write the reference information to disk (allocates space also) */
        if(H5HG_insert(f, data_size, (void *)data, &hobjid) < 0)
            HGOTO_ERROR(H5E_DATATYPE, H5E_WRITEERROR, FAIL, "Unable to write reference information")

        /* Encode the heap information */
        H5F_addr_encode(f, &p, hobjid.addr);
        UINT32ENCODE(p, hobjid.idx);
    }
    *nalloc = buf_size;

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__decode_heap
 *
 * Purpose: Decode data inserted into heap (native only).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__decode_heap(H5F_t *f, const unsigned char *buf, size_t *nbytes,
    unsigned char **data_ptr, size_t *data_size)
{
    const uint8_t *p = (const uint8_t *)buf;
    H5HG_t hobjid;
    size_t buf_size;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(f);
    HDassert(buf);
    HDassert(nbytes);
    HDassert(data_ptr);

    buf_size = H5R_ENCODE_HEAP_SIZE(f);
    /* Don't decode if buffer size isn't big enough */
    if(*nbytes < buf_size)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Buffer size is too small")

    /* Get the heap information */
    H5F_addr_decode(f, &p, &(hobjid.addr));
    if(!H5F_addr_defined(hobjid.addr) || hobjid.addr == 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "Undefined reference pointer")
    UINT32DECODE(p, hobjid.idx);

    /* Read the information from disk */
    if(NULL == (*data_ptr = (unsigned char *)H5HG_read(f, &hobjid, (void *)*data_ptr, data_size)))
        HGOTO_ERROR(H5E_DATATYPE, H5E_READERROR, FAIL, "Unable to read reference data")

    *nbytes = buf_size;

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__free_heap
 *
 * Purpose: Remove data previously inserted into heap (native only).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__free_heap(H5F_t *f, const unsigned char *buf, size_t nbytes)
{
    H5HG_t hobjid;
    const uint8_t *p = (const uint8_t *)buf;
    size_t buf_size;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(f);
    HDassert(buf);

    buf_size = H5R_ENCODE_HEAP_SIZE(f);
    /* Don't decode if buffer size isn't big enough */
    if(nbytes < buf_size)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Buffer size is too small")

    /* Get the heap information */
    H5F_addr_decode(f, &p, &(hobjid.addr));
    if(!H5F_addr_defined(hobjid.addr) || hobjid.addr == 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "Undefined reference pointer")
    UINT32DECODE(p, hobjid.idx);

    /* Free heap object */
    if(hobjid.addr > 0) {
        /* Free heap object */
        if(H5HG_remove(f, &hobjid) < 0)
            HGOTO_ERROR(H5E_DATATYPE, H5E_WRITEERROR, FAIL, "Unable to remove heap object")
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__encode_obj_addr_compat
 *
 * Purpose: Encode an object address (encoding done by type conversion).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__encode_obj_addr_compat(haddr_t addr, unsigned char *buf, size_t *nalloc)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE_NOERR

    HDassert(nalloc);

    /* Don't encode if buffer size isn't big enough or buffer is empty */
    if(buf && *nalloc >= sizeof(addr))
        HDmemcpy(buf, &addr, sizeof(addr));
    *nalloc = sizeof(addr);

    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__decode_obj_addr_compat
 *
 * Purpose: Decode an object address.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__decode_obj_addr_compat(const unsigned char *buf, size_t *nbytes,
    haddr_t *addr_ptr)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(buf);
    HDassert(nbytes);
    HDassert(addr_ptr);

    /* Don't decode if buffer size isn't big enough */
    if(*nbytes < sizeof(*addr_ptr))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Buffer size is too small")

    HDmemcpy(addr_ptr, buf, sizeof(*addr_ptr));

    *nbytes = sizeof(*addr_ptr);

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__encode_addr_region_compat
 *
 * Purpose: Encode dataset selection and insert data into heap (native only).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__encode_addr_region_compat(hid_t loc_id, haddr_t obj_addr, H5S_t *space,
    unsigned char *buf, size_t *nalloc)
{
    size_t buf_size;
    H5I_type_t vol_obj_type = H5I_BADID;/* Object type of loc_id */
    hid_t file_id = H5I_INVALID_HID;
    void *vol_obj_file = NULL;
    H5F_t *vol_file = NULL;
    unsigned char *data = NULL;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(space);
    HDassert(nalloc);

    /* Get object type */
    if((vol_obj_type = H5I_get_type(loc_id)) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Get the file for the object */
    if((file_id = H5F_get_file_id(loc_id, vol_obj_type)) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a file or file object")

    /* Retrieve VOL object */
    if(NULL == (vol_obj_file = H5VL_vol_object(file_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Retrieve file from VOL object */
    if(NULL == (vol_file = (H5F_t *)H5VL_object_data(vol_obj_file)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid VOL object")

    /* Get required buffer size */
    if(H5R__encode_heap(vol_file, NULL, &buf_size, NULL, (size_t)0) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    if(buf && *nalloc >= buf_size) {
        ssize_t data_size;
        uint8_t *p;

        /* Pass the correct encoding version for the selection depending on the
         * file libver bounds, this is later retrieved in H5S hyper encode */
        H5CX_set_libver_bounds(vol_file);

        /* Zero the heap ID out, may leak heap space if user is re-using
         * reference and doesn't have garbage collection turned on
         */
        HDmemset(buf, 0, buf_size);

        /* Get the amount of space required to serialize the selection */
        if ((data_size = H5S_SELECT_SERIAL_SIZE(space)) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, FAIL, "Invalid amount of space for serializing selection")

        /* Increase buffer size to allow for the dataset OID */
        data_size += (hssize_t)sizeof(haddr_t);

        /* Allocate the space to store the serialized information */
        H5_CHECK_OVERFLOW(data_size, hssize_t, size_t);
        if (NULL == (data = (uint8_t *)H5MM_malloc((size_t)data_size)))
            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed")

        /* Serialize information for dataset OID into heap buffer */
        p = (uint8_t *)data;
        H5F_addr_encode(vol_file, &p, obj_addr);

        /* Serialize the selection into heap buffer */
        if (H5S_SELECT_SERIALIZE(space, &p) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, FAIL, "Unable to serialize selection")

        /* Write to heap */
        if(H5R__encode_heap(vol_file, buf, nalloc, data, (size_t)data_size) < 0)
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")
    }
    *nalloc = buf_size;

done:
    H5MM_free(data);
    if(file_id != H5I_INVALID_HID && H5I_dec_ref(file_id) < 0)
        HDONE_ERROR(H5E_REFERENCE, H5E_CANTDEC, FAIL, "unable to decrement refcount on file")
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5R__decode_obj_addr_compat
 *
 * Purpose: Decode dataset selection from data inserted into heap (native only).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5R__decode_addr_region_compat(hid_t loc_id, const unsigned char *buf,
    size_t *nbytes, haddr_t *obj_addr_ptr, H5S_t **space_ptr)
{
    H5I_type_t vol_obj_type = H5I_BADID;/* Object type of loc_id */
    hid_t file_id = H5I_INVALID_HID;
    void *vol_obj_file = NULL;
    H5F_t *vol_file = NULL;
    unsigned char *data = NULL;
    size_t data_size;
    haddr_t obj_addr;
    const uint8_t *p;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

    HDassert(buf);
    HDassert(nbytes);

    /* Get object type */
    if((vol_obj_type = H5I_get_type(loc_id)) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Get the file for the object */
    if((file_id = H5F_get_file_id(loc_id, vol_obj_type)) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a file or file object")

    /* Retrieve VOL object */
    if(NULL == (vol_obj_file = H5VL_vol_object(file_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Retrieve file from VOL object */
    if(NULL == (vol_file = (H5F_t *)H5VL_object_data(vol_obj_file)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid VOL object")

    /* Read from heap */
    if(H5R__decode_heap(vol_file, buf, nbytes, &data, &data_size) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid location identifier")

    /* Get object address */
    p = (const uint8_t *)data;
    H5F_addr_decode(vol_file, &p, &obj_addr);
    if(!H5F_addr_defined(obj_addr) || obj_addr == 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "Undefined reference pointer")

    if(space_ptr) {
        H5O_loc_t oloc; /* Object location */
        H5S_t *space = NULL;

        /* Initialize the object location */
        H5O_loc_reset(&oloc);
        oloc.file = vol_file;
        oloc.addr = obj_addr;

        /* Open and copy the dataset's dataspace */
        if(NULL == (space = H5S_read(&oloc)))
            HGOTO_ERROR(H5E_DATASPACE, H5E_NOTFOUND, FAIL, "not found")

        /* Unserialize the selection */
        if(H5S_SELECT_DESERIALIZE(&space, &p) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "can't deserialize selection")

        *space_ptr = space;
    }
    if(obj_addr_ptr)
        *obj_addr_ptr = obj_addr;

done:
    H5MM_free(data);
    if(file_id != H5I_INVALID_HID && H5I_dec_ref(file_id) < 0)
        HDONE_ERROR(H5E_REFERENCE, H5E_CANTDEC, FAIL, "unable to decrement refcount on file")
    FUNC_LEAVE_NOAPI(ret_value)
}
