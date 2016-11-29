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

#include "H5Rmodule.h"      /* This source code file is part of the H5R module */


/***********/
/* Headers */
/***********/
#include "H5private.h"      /* Generic Functions    */
#include "H5ACprivate.h"    /* Metadata cache       */
#include "H5Aprivate.h"     /* Attributes           */
#include "H5Dprivate.h"     /* Datasets             */
#include "H5Eprivate.h"     /* Error handling       */
#include "H5Gprivate.h"     /* Groups               */
#include "H5HGprivate.h"    /* Global Heaps         */
#include "H5Iprivate.h"     /* IDs                  */
#include "H5MMprivate.h"    /* Memory management    */
#include "H5Rpkg.h"         /* References           */
#include "H5Sprivate.h"     /* Dataspaces           */
#include "H5VLprivate.h"    /* VOL plugins          */


/****************/
/* Local Macros */
/****************/
#define H5R_MAX_ATTR_REF_NAME_LEN (64 * 1024)

/******************/
/* Local Typedefs */
/******************/


/********************/
/* Local Prototypes */
/********************/

static H5S_t * H5R__get_region(H5F_t *file, hid_t dxpl_id, const href_t *_ref);
static ssize_t H5R__get_name(H5F_t *file, hid_t lapl_id, hid_t dxpl_id, hid_t id,
    const href_t *_ref, char *name, size_t size);
static ssize_t H5R__get_filename(const href_t *ref, char *name, size_t size);


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

/* Reference ID class */
static const H5I_class_t H5I_REFERENCE_CLS[1] = {{
    H5I_REFERENCE,		/* ID class value */
    0,				/* Class flags */
    0,				/* # of reserved IDs for class */
    NULL			/* Callback routine for closing objects of this class */
}};

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
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* Initialize the atom group for the file IDs */
    if(H5I_register_type(H5I_REFERENCE_CLS) < 0)
	HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, FAIL, "unable to initialize interface")

    /* Mark "top" of interface as initialized, too */
    H5R_top_package_initialize_s = TRUE;

done:
    FUNC_LEAVE_NOAPI(ret_value)
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

    if(H5R_top_package_initialize_s) {
	if(H5I_nmembers(H5I_REFERENCE) > 0) {
	    (void)H5I_clear_type(H5I_REFERENCE, FALSE, FALSE);
            n++; /*H5I*/
	} /* end if */

        /* Mark closed */
        if(0 == n)
            H5R_top_package_initialize_s = FALSE;
    } /* end if */

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
        HDassert(0 == H5I_nmembers(H5I_REFERENCE));
        HDassert(FALSE == H5R_top_package_initialize_s);

        /* Destroy the reference id group */
        n += (H5I_dec_type_ref(H5I_REFERENCE) > 0);

        /* Mark closed */
        if(0 == n)
            H5_PKG_INIT_VAR = FALSE;
    } /* end if */

    FUNC_LEAVE_NOAPI(n)
} /* end H5R_term_package() */


/*--------------------------------------------------------------------------
 NAME
    H5R_create_object
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5R_create_object(loc, name, dxpl_id)
        H5G_loc_t *loc;     IN: File location used to locate object pointed to
        const char *name;   IN: Name of object at location of object pointed to
        hid_t dxpl_id;      IN: Property list ID

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF.  The LOC and NAME are used to locate the object
    pointed to.
--------------------------------------------------------------------------*/
href_t *
H5R_create_object(H5G_loc_t *loc, const char *name, hid_t dxpl_id)
{
    H5G_loc_t obj_loc;          /* Group hier. location of object */
    H5G_name_t path;            /* Object group hier. path */
    H5O_loc_t oloc;             /* Object object location */
    hbool_t obj_found = FALSE;  /* Object location found */
    href_t *ret_value = NULL;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(loc);
    HDassert(name);

    /* Set up object location to fill in */
    obj_loc.oloc = &oloc;
    obj_loc.path = &path;
    H5G_loc_reset(&obj_loc);

    /* Find the object */
    if(H5G_loc_find(loc, name, &obj_loc, H5P_DEFAULT, dxpl_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_NOTFOUND, NULL, "object not found")
    obj_found = TRUE;

    if(NULL == (ret_value = (href_t *)H5MM_malloc(sizeof(href_t))))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Cannot allocate memory for reference")

    ret_value->ref_type = H5R_OBJECT;
    ret_value->obj_addr = obj_loc.oloc->addr;

done:
    if(obj_found)
        H5G_loc_free(&obj_loc);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R_create_object() */

/*--------------------------------------------------------------------------
 NAME
    H5R_create_region
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5R_create_region(loc, name, dxpl_id, space)
        H5G_loc_t *loc;     IN: File location used to locate object pointed to
        const char *name;   IN: Name of object at location of object pointed to
        hid_t dxpl_id;      IN: Property list ID
        H5S_t *space;       IN: Dataspace with selection, used for Dataset
                                Region references.

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF. The LOC and NAME are used to locate the object
    pointed to and the SPACE is used to choose the region pointed to.
--------------------------------------------------------------------------*/
href_t *
H5R_create_region(H5G_loc_t *loc, const char *name, hid_t dxpl_id, H5S_t *space)
{
    H5G_loc_t obj_loc;          /* Group hier. location of object */
    H5G_name_t path;            /* Object group hier. path */
    H5O_loc_t oloc;             /* Object object location */
    hbool_t obj_found = FALSE;  /* Object location found */
    hssize_t buf_size;          /* Size of buffer needed to serialize selection */
    uint8_t *p;                 /* Pointer to OID to store */
    href_t *ref = NULL;         /* Reference to be returned */
    href_t *ret_value = NULL;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(loc);
    HDassert(name);
    HDassert(space);

    /* Set up object location to fill in */
    obj_loc.oloc = &oloc;
    obj_loc.path = &path;
    H5G_loc_reset(&obj_loc);

    /* Find the object */
    if(H5G_loc_find(loc, name, &obj_loc, H5P_DEFAULT, dxpl_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_NOTFOUND, NULL, "object not found")
    obj_found = TRUE;

    /* Get the amount of space required to serialize the selection */
    if((buf_size = H5S_SELECT_SERIAL_SIZE(space)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, NULL, "Invalid amount of space for serializing selection")

    /* Increase buffer size to allow for the dataset OID */
    buf_size += (hssize_t)sizeof(haddr_t);

    /* Allocate the space to store the serialized information */
    H5_CHECK_OVERFLOW(buf_size, hssize_t, size_t);
    if(NULL == (ref = (href_t *)H5MM_malloc(sizeof(href_t))))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Cannot allocate memory for reference")
    if (NULL == (ref->obj_enc.buf = H5MM_malloc((size_t) buf_size)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, NULL, "Cannot allocate buffer to serialize selection")
    ref->obj_enc.buf_size = (size_t) buf_size;
    ref->ref_type = H5R_REGION;

    /* Serialize information for dataset OID into buffer */
    p = (uint8_t *)ref->obj_enc.buf;
    H5F_addr_encode(loc->oloc->file, &p, obj_loc.oloc->addr);

    /* Serialize the selection into buffer */
    if(H5S_SELECT_SERIALIZE(space, &p) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, NULL, "Unable to serialize selection")

    ret_value = ref;

done:
    if(obj_found)
        H5G_loc_free(&obj_loc);
    if(NULL == ret_value) {
        H5MM_free(ref->obj_enc.buf);
        H5MM_free(ref);
    }

    FUNC_LEAVE_NOAPI(ret_value)
} /* H5R_create_region */

/*--------------------------------------------------------------------------
 NAME
    H5R_create_attr
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5R_create_attr(loc, name, dxpl_id, attr_name)
        H5G_loc_t *loc;         IN: File location used to locate object pointed to
        const char *name;       IN: Name of object at location of object pointed to
        hid_t dxpl_id;          IN: Property list ID
        const char *attr_name;  IN: Name of attribute pointed to, used for
                                    Attribute references.

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF.  The LOC, NAME and ATTR_NAME are used to locate
    the attribute pointed to.
--------------------------------------------------------------------------*/
href_t *
H5R_create_attr(H5G_loc_t *loc, const char *name, hid_t dxpl_id, const char *attr_name)
{
    H5G_loc_t obj_loc;          /* Group hier. location of object */
    H5G_name_t path;            /* Object group hier. path */
    H5O_loc_t oloc;             /* Object object location */
    hbool_t obj_found = FALSE;  /* Object location found */
    size_t buf_size;            /* Size of buffer needed to serialize attribute */
    size_t attr_name_len;       /* Length of the attribute name */
    uint8_t *p;                 /* Pointer to OID to store */
    href_t *ref = NULL;         /* Reference to be returned */
    href_t *ret_value = NULL;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(loc);
    HDassert(name);
    HDassert(attr_name);

    /* Set up object location to fill in */
    obj_loc.oloc = &oloc;
    obj_loc.path = &path;
    H5G_loc_reset(&obj_loc);

    /* Find the object */
    if(H5G_loc_find(loc, name, &obj_loc, H5P_DEFAULT, dxpl_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_NOTFOUND, NULL, "object not found")
    obj_found = TRUE;

    /* Get the amount of space required to serialize the attribute name */
    attr_name_len = HDstrlen(attr_name);
    if(attr_name_len >= H5R_MAX_ATTR_REF_NAME_LEN)
        HGOTO_ERROR(H5E_REFERENCE, H5E_ARGS, NULL, "attribute name too long")

    /* Compute buffer size, allow for the attribute name length and object's OID */
    buf_size = attr_name_len + 2 + sizeof(haddr_t);

    /* Allocate the space to store the serialized information */
    if(NULL == (ref = (href_t *)H5MM_malloc(sizeof(href_t))))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Cannot allocate memory for reference")
    if (NULL == (ref->obj_enc.buf = H5MM_malloc((size_t) buf_size)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, NULL, "Cannot allocate buffer to serialize selection")
    ref->obj_enc.buf_size = (size_t) buf_size;
    ref->ref_type = H5R_ATTR;

    /* Serialize information for object's OID into buffer */
    p = (uint8_t *)ref->obj_enc.buf;
    H5F_addr_encode(loc->oloc->file, &p, obj_loc.oloc->addr);

    /* Serialize information for attribute name length into the buffer */
    UINT16ENCODE(p, attr_name_len);

    /* Copy the attribute name into the buffer */
    HDmemcpy(p, attr_name, attr_name_len);

    /* Sanity check */
    HDassert((size_t)((p + attr_name_len) - (uint8_t *)ref->obj_enc.buf) == buf_size);

    ret_value = ref;

done:
    if(obj_found)
        H5G_loc_free(&obj_loc);
    if(NULL == ret_value) {
        H5MM_free(ref->obj_enc.buf);
        H5MM_free(ref);
    }

    FUNC_LEAVE_NOAPI(ret_value)
} /* H5R_create_attr */

/*--------------------------------------------------------------------------
 NAME
    H5R_create_ext_object
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5R_create_ext_object(loc, name, dxpl_id)
        H5G_loc_t *loc;     IN: File location used to locate object pointed to
        const char *name;   IN: Name of object at location of object pointed to
        hid_t dxpl_id;      IN: Property list ID

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF.  The LOC and NAME are used to locate the object
    pointed to.
--------------------------------------------------------------------------*/
href_t *
H5R_create_ext_object(H5G_loc_t *loc, const char *name, hid_t dxpl_id)
{
    H5G_loc_t obj_loc;          /* Group hier. location of object */
    H5G_name_t path;            /* Object group hier. path */
    H5O_loc_t oloc;             /* Object object location */
    hbool_t obj_found = FALSE;  /* Object location found */
    size_t filename_len;        /* Length of the file name */
    size_t buf_size;            /* Size of buffer needed to serialize reference */
    uint8_t *p;                 /* Pointer to OID to store */
    href_t *ref = NULL;         /* Reference to be returned */
    href_t *ret_value = NULL;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(loc);
    HDassert(name);

    /* Set up object location to fill in */
    obj_loc.oloc = &oloc;
    obj_loc.path = &path;
    H5G_loc_reset(&obj_loc);

    /* Find the object */
    if(H5G_loc_find(loc, name, &obj_loc, H5P_DEFAULT, dxpl_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_NOTFOUND, NULL, "object not found")
    obj_found = TRUE;

    /* Need to add name of the file */
    filename_len = HDstrlen(H5F_OPEN_NAME(loc->oloc->file));

    /* Compute buffer size, allow for the attribute name length and object's OID */
    buf_size = filename_len + 2 + sizeof(haddr_t);

    if(NULL == (ref = (href_t *)H5MM_malloc(sizeof(href_t))))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Cannot allocate memory for reference")
    if (NULL == (ref->obj_enc.buf = H5MM_malloc((size_t) buf_size)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, NULL, "Cannot allocate buffer to serialize selection")
    ref->obj_enc.buf_size = (size_t) buf_size;
    ref->ref_type = H5R_EXT_OBJECT;

    /* Serialize information for file name length into the buffer */
    p = (uint8_t *)ref->obj_enc.buf;
    UINT16ENCODE(p, filename_len);

    /* Copy the file name into the buffer */
    HDmemcpy(p, H5F_OPEN_NAME(loc->oloc->file), filename_len);
    p += filename_len;

    /* Serialize information for object's OID into buffer */
    H5F_addr_encode(loc->oloc->file, &p, obj_loc.oloc->addr);

    ret_value = ref;

done:
    if(obj_found)
        H5G_loc_free(&obj_loc);
    if(NULL == ret_value) {
        H5MM_free(ref->obj_enc.buf);
        H5MM_free(ref);
    }

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R_create_ext_object() */

/*--------------------------------------------------------------------------
 NAME
    H5R_create_ext_region
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5R_create_ext_region(loc, name, dxpl_id, space)
        H5G_loc_t *loc;     IN: File location used to locate object pointed to
        const char *name;   IN: Name of object at location of object pointed to
        hid_t dxpl_id;      IN: Property list ID
        H5S_t *space;       IN: Dataspace with selection, used for Dataset
                                Region references.

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF. The LOC and NAME are used to locate the object
    pointed to and the SPACE is used to choose the region pointed to.
--------------------------------------------------------------------------*/
href_t *
H5R_create_ext_region(H5G_loc_t *loc, const char *name, hid_t dxpl_id, H5S_t *space)
{
    H5G_loc_t obj_loc;          /* Group hier. location of object */
    H5G_name_t path;            /* Object group hier. path */
    H5O_loc_t oloc;             /* Object object location */
    hbool_t obj_found = FALSE;  /* Object location found */
    size_t filename_len;        /* Length of the file name */
    hssize_t buf_size;          /* Size of buffer needed to serialize selection */
    uint8_t *p;                 /* Pointer to OID to store */
    href_t *ref = NULL;         /* Reference to be returned */
    href_t *ret_value = NULL;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(loc);
    HDassert(name);
    HDassert(space);

    /* Set up object location to fill in */
    obj_loc.oloc = &oloc;
    obj_loc.path = &path;
    H5G_loc_reset(&obj_loc);

    /* Find the object */
    if(H5G_loc_find(loc, name, &obj_loc, H5P_DEFAULT, dxpl_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_NOTFOUND, NULL, "object not found")
    obj_found = TRUE;

    /* Need to add name of the file */
    filename_len = HDstrlen(H5F_OPEN_NAME(loc->oloc->file));

    /* Get the amount of space required to serialize the selection */
    if((buf_size = H5S_SELECT_SERIAL_SIZE(space)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, NULL, "Invalid amount of space for serializing selection")

    /* Increase buffer size to allow for the dataset OID */
    buf_size += (hssize_t)(sizeof(haddr_t) + 2 + filename_len);

    /* Allocate the space to store the serialized information */
    H5_CHECK_OVERFLOW(buf_size, hssize_t, size_t);
    if(NULL == (ref = (href_t *)H5MM_malloc(sizeof(href_t))))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Cannot allocate memory for reference")
    if (NULL == (ref->obj_enc.buf = H5MM_malloc((size_t) buf_size)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, NULL, "Cannot allocate buffer to serialize selection")
    ref->obj_enc.buf_size = (size_t) buf_size;
    ref->ref_type = H5R_EXT_REGION;

    /* Serialize information for file name length into the buffer */
    p = (uint8_t *)ref->obj_enc.buf;
    UINT16ENCODE(p, filename_len);

    /* Copy the file name into the buffer */
    HDmemcpy(p, H5F_OPEN_NAME(loc->oloc->file), filename_len);
    p += filename_len;

    /* Serialize information for dataset OID into buffer */
    H5F_addr_encode(loc->oloc->file, &p, obj_loc.oloc->addr);

    /* Serialize the selection into buffer */
    if(H5S_SELECT_SERIALIZE(space, &p) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTCOPY, NULL, "Unable to serialize selection")

    ret_value = ref;

done:
    if(obj_found)
        H5G_loc_free(&obj_loc);
    if(NULL == ret_value) {
        H5MM_free(ref->obj_enc.buf);
        H5MM_free(ref);
    }

    FUNC_LEAVE_NOAPI(ret_value)
} /* H5R_create_ext_region */

/*--------------------------------------------------------------------------
 NAME
    H5R_create_ext_attr
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5R_create_ext_attr(loc, name, dxpl_id, attr_name)
        H5G_loc_t *loc;         IN: File location used to locate object pointed to
        const char *name;       IN: Name of object at location of object pointed to
        hid_t dxpl_id;          IN: Property list ID
        const char *attr_name;  IN: Name of attribute pointed to, used for
                                    Attribute references.

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF.  The LOC, NAME and ATTR_NAME are used to locate
    the attribute pointed to.
--------------------------------------------------------------------------*/
href_t *
H5R_create_ext_attr(H5G_loc_t *loc, const char *name, hid_t dxpl_id, const char *attr_name)
{
    H5G_loc_t obj_loc;          /* Group hier. location of object */
    H5G_name_t path;            /* Object group hier. path */
    H5O_loc_t oloc;             /* Object object location */
    hbool_t obj_found = FALSE;  /* Object location found */
    size_t filename_len;        /* Length of the file name */
    size_t buf_size;            /* Size of buffer needed to serialize attribute */
    size_t attr_name_len;       /* Length of the attribute name */
    uint8_t *p;                 /* Pointer to OID to store */
    href_t *ref = NULL;         /* Reference to be returned */
    href_t *ret_value = NULL;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(loc);
    HDassert(name);
    HDassert(attr_name);

    /* Set up object location to fill in */
    obj_loc.oloc = &oloc;
    obj_loc.path = &path;
    H5G_loc_reset(&obj_loc);

    /* Find the object */
    if(H5G_loc_find(loc, name, &obj_loc, H5P_DEFAULT, dxpl_id) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_NOTFOUND, NULL, "object not found")
    obj_found = TRUE;

    /* Need to add name of the file */
    filename_len = HDstrlen(H5F_OPEN_NAME(loc->oloc->file));

    /* Get the amount of space required to serialize the attribute name */
    attr_name_len = HDstrlen(attr_name);
    if(attr_name_len >= H5R_MAX_ATTR_REF_NAME_LEN)
        HGOTO_ERROR(H5E_REFERENCE, H5E_ARGS, NULL, "attribute name too long")

    /* Compute buffer size, allow for the attribute name length and object's OID */
    buf_size = filename_len + 2 + attr_name_len + 2 + sizeof(haddr_t);

    /* Allocate the space to store the serialized information */
    if(NULL == (ref = (href_t *)H5MM_malloc(sizeof(href_t))))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "Cannot allocate memory for reference")
    if (NULL == (ref->obj_enc.buf = H5MM_malloc((size_t) buf_size)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, NULL, "Cannot allocate buffer to serialize selection")
    ref->obj_enc.buf_size = (size_t) buf_size;
    ref->ref_type = H5R_EXT_ATTR;

    /* Serialize information for file name length into the buffer */
    p = (uint8_t *)ref->obj_enc.buf;
    UINT16ENCODE(p, filename_len);

    /* Copy the file name into the buffer */
    HDmemcpy(p, H5F_OPEN_NAME(loc->oloc->file), filename_len);
    p += filename_len;

    /* Serialize information for object's OID into buffer */
    H5F_addr_encode(loc->oloc->file, &p, obj_loc.oloc->addr);

    /* Serialize information for attribute name length into the buffer */
    UINT16ENCODE(p, attr_name_len);

    /* Copy the attribute name into the buffer */
    HDmemcpy(p, attr_name, attr_name_len);

    /* Sanity check */
    HDassert((size_t)((p + attr_name_len) - (uint8_t *)ref->obj_enc.buf) == buf_size);

    ret_value = ref;

done:
    if(obj_found)
        H5G_loc_free(&obj_loc);
    if(NULL == ret_value) {
        H5MM_free(ref->obj_enc.buf);
        H5MM_free(ref);
    }

    FUNC_LEAVE_NOAPI(ret_value)
} /* H5R_create_ext_attr */


/*--------------------------------------------------------------------------
 NAME
    H5Rcreate_object
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5Rcreate_object(loc_id, name)
        hid_t loc_id;       IN: Location ID used to locate object pointed to
        const char *name;   IN: Name of object at location LOC_ID of object
                                pointed to

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF.  The LOC_ID and NAME are used to locate the object
    pointed to.
--------------------------------------------------------------------------*/
href_t *
H5Rcreate_object(hid_t loc_id, const char *name)
{
    H5G_loc_t loc; /* File location */
    href_t     *ret_value = NULL; /* Return value */

    FUNC_ENTER_API(NULL)

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a location")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "no name given")

    if(NULL == (ret_value = H5R_create_object(&loc, name, H5AC_dxpl_id)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, NULL, "unable to create object reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rcreate_object() */

/*--------------------------------------------------------------------------
 NAME
    H5Rcreate_region
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5Rcreate_region(loc_id, name, space_id)
        hid_t loc_id;       IN: Location ID used to locate object pointed to
        const char *name;   IN: Name of object at location LOC_ID of object
                                pointed to
        hid_t space_id;     IN: Dataspace ID with selection, used for Dataset
                                Region references.

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF.  The LOC_ID and NAME are used to locate the object
    pointed to and the SPACE_ID is used to choose the region pointed to.
--------------------------------------------------------------------------*/
href_t *
H5Rcreate_region(hid_t loc_id, const char *name, hid_t space_id)
{
    H5G_loc_t loc; /* File location */
    H5S_t      *space = NULL;   /* Pointer to dataspace containing region */
    href_t     *ret_value = NULL; /* Return value */

    FUNC_ENTER_API(NULL)

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a location")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "no name given")
    if(space_id == H5I_BADID)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "reference region dataspace id must be valid")
    if(space_id != H5I_BADID && (NULL == (space = (H5S_t *)H5I_object_verify(space_id, H5I_DATASPACE))))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a dataspace")

    if(NULL == (ret_value = H5R_create_region(&loc, name, H5AC_dxpl_id, space)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, NULL, "unable to create region reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rcreate_region() */

/*--------------------------------------------------------------------------
 NAME
    H5Rcreate_attr
 PURPOSE
    Creates a particular kind of reference for the user
 USAGE
    href_t *H5Rcreate_attr(loc_id, name, attr_name)
        hid_t loc_id;       IN: Location ID used to locate object pointed to
        const char *name;   IN: Name of object at location LOC_ID of object
                                pointed to
        const char *attr_name;   IN: Name of attribute pointed to, used for
                                  Attribute references.

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Creates a particular type of reference specified with REF_TYPE, in the
    space pointed to by REF.  The LOC_ID, NAME and ATTR_NAME are used to locate
    the attribute pointed to.
--------------------------------------------------------------------------*/
href_t *
H5Rcreate_attr(hid_t loc_id, const char *name, const char *attr_name)
{
    H5G_loc_t loc; /* File location */
    href_t    *ret_value = NULL; /* Return value */

    FUNC_ENTER_API(NULL)

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a location")
    if(!name || !*name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "no name given")
    if(!attr_name || !*attr_name)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "no attribute name given")

    if(NULL == (ret_value = H5R_create_attr(&loc, name, H5AC_dxpl_id, attr_name)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, NULL, "unable to create atribute reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rcreate_attr() */

/*--------------------------------------------------------------------------
 NAME
    H5R_destroy
 PURPOSE
    Destroy a reference
 USAGE
    herr_t H5R_destroy(ref)
        href_t *ref;       IN: Reference

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Destroy reference and free resources allocated during H5Rcreate.
--------------------------------------------------------------------------*/
herr_t
H5R_destroy(href_t *ref)
{
    herr_t    ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(ref);

    if(H5R_OBJECT != ref->ref_type)
        H5MM_free(ref->obj_enc.buf);
    H5MM_free(ref);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Rdestroy() */

/*--------------------------------------------------------------------------
 NAME
    H5Rdestroy
 PURPOSE
    Destroy a reference
 USAGE
    herr_t H5Rdestroy(ref)
        href_t *ref;       IN: Reference

 RETURNS
    Reference created on success/NULL on failure
 DESCRIPTION
    Destroy reference and free resources allocated during H5Rcreate.
--------------------------------------------------------------------------*/
herr_t
H5Rdestroy(href_t *ref)
{
    herr_t    ret_value = FAIL; /* Return value */

    FUNC_ENTER_API(FAIL)

    /* Check args */
    if(NULL == ref)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid reference pointer")

    if(FAIL == (ret_value = H5R_destroy(ref)))
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, FAIL, "unable to destroy reference")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rdestroy() */


/*--------------------------------------------------------------------------
 NAME
    H5R__dereference
 PURPOSE
    Opens the HDF5 object referenced.
 USAGE
    hid_t H5R__dereference(file, oapl_id, dxpl_id, ref, app_ref)
        H5F_t *file;        IN: File the object being dereferenced is within
        hid_t oapl_id;      IN: Property list of the object being referenced.
        hid_t dxpl_id;      IN: Property list ID
        ref_t *ref;         IN: Reference to open
        hbool_t app_ref     IN: Referenced by application.

 RETURNS
    Valid ID on success, Negative on failure
 DESCRIPTION
    Given a reference to some object, open that object and return an ID for
    that object.
--------------------------------------------------------------------------*/
hid_t
H5R__dereference(H5F_t *file, hid_t oapl_id, hid_t dxpl_id, const href_t *ref, hbool_t app_ref)
{
    H5O_loc_t oloc;                 /* Object location */
    H5G_name_t path;                /* Path of object */
    H5G_loc_t loc;                  /* Group location */
    char *filename = NULL;          /* Name of the file (for external references) */
    char *attr_name = NULL;         /* Name of the attribute (for attribute references) */
    unsigned rc;		            /* Reference count of object */
    H5O_type_t obj_type;            /* Type of object */
    const uint8_t *p = NULL;        /* Pointer to OID to store */
    hid_t ret_value = H5I_BADID;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(file);
    HDassert(ref);
    HDassert(ref->ref_type > H5R_BADTYPE && ref->ref_type < H5R_MAXTYPE);

    /* Initialize the object location */
    H5O_loc_reset(&oloc);
    oloc.file = file;

    switch (ref->ref_type) {
        case H5R_EXT_OBJECT:
        case H5R_EXT_REGION:
        case H5R_EXT_ATTR:
        {
            size_t filename_len; /* Length of the file name */

            /* Get the file name length */
            p = (const uint8_t *)ref->obj_enc.buf;
            UINT16DECODE(p, filename_len);
            HDassert(filename_len < H5R_MAX_ATTR_REF_NAME_LEN);

            /* Allocate space for the file name */
            if(NULL == (filename = (char *)H5MM_malloc(filename_len + 1)))
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, FAIL, "memory allocation failed")

            /* Get the file name */
            HDmemcpy(filename, p, filename_len);
            filename[filename_len] = '\0';
            p += filename_len;
        }
            break;
        case H5R_OBJECT:
        case H5R_REGION:
        case H5R_ATTR:
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            break;
    }

    switch(ref->ref_type) {
        case H5R_OBJECT:
            oloc.addr = ref->obj_addr;
            if(!H5F_addr_defined(oloc.addr) || oloc.addr == 0)
                HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "Undefined reference pointer")
            break;

        case H5R_EXT_OBJECT:
        case H5R_EXT_REGION:
        case H5R_REGION:
        {
            /* Get the object oid for the dataset */
            if (!p) p = (const uint8_t *)ref->obj_enc.buf;
            H5F_addr_decode(oloc.file, &p, &(oloc.addr));
        } /* end case */
        break;

        case H5R_EXT_ATTR:
        case H5R_ATTR:
        {
            size_t attr_name_len; /* Length of the attribute name */

            /* Get the object oid for the dataset */
            if (!p) p = (const uint8_t *)ref->obj_enc.buf;
            H5F_addr_decode(oloc.file, &p, &(oloc.addr));

            /* Get the attribute name length */
            UINT16DECODE(p, attr_name_len);
            HDassert(attr_name_len < H5R_MAX_ATTR_REF_NAME_LEN);

            /* Allocate space for the attribute name */
            if(NULL == (attr_name = (char *)H5MM_malloc(attr_name_len + 1)))
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, FAIL, "memory allocation failed")

            /* Get the attribute name */
            HDmemcpy(attr_name, p, attr_name_len);
            attr_name[attr_name_len] = '\0';
        } /* end case */
        break;

        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    /* Get the # of links for object, and its type */
    /* (To check to make certain that this object hasn't been deleted since the reference was created) */
    if(H5O_get_rc_and_type(&oloc, dxpl_id, &rc, &obj_type) < 0 || 0 == rc)
        HGOTO_ERROR(H5E_REFERENCE, H5E_LINKCOUNT, FAIL, "dereferencing deleted object")

    /* Construct a group location for opening the object */
    H5G_name_reset(&path);
    loc.oloc = &oloc;
    loc.path = &path;

    /* Open an attribute */
    if(H5R_ATTR == ref->ref_type || H5R_EXT_ATTR == ref->ref_type) {
        H5A_t *attr;            /* Attribute opened */

        /* Open the attribute */
        if(NULL == (attr = H5A_open(&loc, attr_name, dxpl_id)))
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTOPENOBJ, FAIL, "can't open attribute")

        /* Create an atom for the attribute */
        if((ret_value = H5I_register(H5I_ATTR, attr, app_ref)) < 0) {
            H5A_close(attr);
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTREGISTER, FAIL, "can't register attribute")
        } /* end if */
    } /* end if */
    else {
        /* Open the object */
        switch(obj_type) {
            case H5O_TYPE_GROUP:
                {
                    H5G_t *group;               /* Pointer to group to open */

                    if(NULL == (group = H5G_open(&loc, dxpl_id)))
                        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "not found")

                    /* Create an atom for the group */
                    if((ret_value = H5I_register(H5I_GROUP, group, app_ref)) < 0) {
                        H5G_close(group);
                        HGOTO_ERROR(H5E_SYM, H5E_CANTREGISTER, FAIL, "can't register group")
                    } /* end if */
                } /* end case */
                break;

            case H5O_TYPE_NAMED_DATATYPE:
                {
                    H5T_t *type;                /* Pointer to datatype to open */

                    if(NULL == (type = H5T_open(&loc, dxpl_id)))
                        HGOTO_ERROR(H5E_DATATYPE, H5E_NOTFOUND, FAIL, "not found")

                    /* Create an atom for the datatype */
                    if((ret_value = H5I_register(H5I_DATATYPE, type, app_ref)) < 0) {
                        H5T_close(type);
                        HGOTO_ERROR(H5E_DATATYPE, H5E_CANTREGISTER, FAIL, "can't register datatype")
                    } /* end if */
                } /* end case */
                break;

            case H5O_TYPE_DATASET:
                {
                    H5D_t *dset;                /* Pointer to dataset to open */

                    /* Get correct property list */
                    if(H5P_DEFAULT == oapl_id)
                        oapl_id = H5P_DATASET_ACCESS_DEFAULT;
                    else if(TRUE != H5P_isa_class(oapl_id, H5P_DATASET_ACCESS))
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not dataset access property list")

                    /* Open the dataset */
                    if(NULL == (dset = H5D_open(&loc, oapl_id, dxpl_id)))
                        HGOTO_ERROR(H5E_DATASET, H5E_NOTFOUND, FAIL, "not found")

                    /* Create an atom for the dataset */
                    if((ret_value = H5I_register(H5I_DATASET, dset, app_ref)) < 0) {
                        H5D_close(dset);
                        HGOTO_ERROR(H5E_DATASET, H5E_CANTREGISTER, FAIL, "can't register dataset")
                    } /* end if */
                } /* end case */
                break;

            case H5O_TYPE_UNKNOWN:
            case H5O_TYPE_NTYPES:
            default:
                HGOTO_ERROR(H5E_REFERENCE, H5E_BADTYPE, FAIL, "can't identify type of object referenced")
         } /* end switch */
    } /* end else */

done:
    H5MM_xfree(filename);
    H5MM_xfree(attr_name);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__dereference() */


/*--------------------------------------------------------------------------
 NAME
    H5Rdereference2
 PURPOSE
    Opens the HDF5 object referenced.
 USAGE
    hid_t H5Rdereference2(obj_id, oapl_id, ref)
        hid_t obj_id;   IN: Dataset reference object is in or location ID of
                            object that the dataset is located within.
        hid_t oapl_id;  IN: Property list of the object being referenced.
        href_t *ref;    IN: Reference to open.

 RETURNS
    Valid ID on success, Negative on failure
 DESCRIPTION
    Given a reference to some object, open that object and return an ID for
    that object.
--------------------------------------------------------------------------*/
hid_t
H5Rdereference2(hid_t obj_id, hid_t oapl_id, const href_t *ref)
{
    H5VL_object_t *obj = NULL;        /* object token of loc_id */
    H5I_type_t opened_type;
    void *opened_obj = NULL;
    H5VL_loc_params_t loc_params;
    hid_t ret_value = FAIL;

    FUNC_ENTER_API(FAIL)

    /* Check args */
     if(oapl_id < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a property list")
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(ref->ref_type <= H5R_BADTYPE || ref->ref_type >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference type")

    /* get the vol object */
    if(NULL == (obj = H5VL_get_object(obj_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid file identifier")

    loc_params.type = H5VL_OBJECT_BY_REF;
    loc_params.loc_data.loc_by_ref.ref_type = ref_type;
    loc_params.loc_data.loc_by_ref._ref = _ref;
    loc_params.loc_data.loc_by_ref.lapl_id = oapl_id;
    loc_params.obj_type = H5I_get_type(obj_id);

    /* Open the object through the VOL */
    if(NULL == (opened_obj = H5VL_object_open(obj->vol_obj, loc_params, obj->vol_info->vol_cls, &opened_type, 
                                              H5AC_ind_dxpl_id, H5_REQUEST_NULL)))
	HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, FAIL, "unable to dereference object")

    if((ret_value = H5VL_register_id(opened_type, opened_obj, obj->vol_info, TRUE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to atomize object handle")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rdereference2() */


/*--------------------------------------------------------------------------
 NAME
    H5R__get_region
 PURPOSE
    Retrieves a dataspace with the region pointed to selected.
 USAGE
    H5S_t *H5R__get_region(file, dxpl_id, ref_type, ref)
        H5F_t *file;            IN: File the object being dereferenced is within
        hid_t dxpl_id;          IN: Property list ID
        href_t *ref;            IN: Reference to open.

 RETURNS
    Pointer to the dataspace on success, NULL on failure
 DESCRIPTION
    Given a reference to some object, creates a copy of the dataset pointed
    to's dataspace and defines a selection in the copy which is the region
    pointed to.
--------------------------------------------------------------------------*/
static H5S_t *
H5R__get_region(H5F_t *file, hid_t dxpl_id, const href_t *ref)
{
    H5O_loc_t oloc;             /* Object location */
    const uint8_t *p;           /* Pointer to OID to store */
    char *filename = NULL;      /* Name of the file (for external references) */
    H5S_t *ret_value;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(file);
    HDassert(ref);
    HDassert((ref->ref_type == H5R_REGION) || (ref->ref_type == H5R_EXT_REGION));

    /* Initialize the object location */
    H5O_loc_reset(&oloc);
    oloc.file = file;

    /* Point to reference buffer now */
    p = (const uint8_t *)ref->obj_enc.buf;

    if (ref->ref_type == H5R_EXT_REGION) {
        size_t filename_len; /* Length of the file name */

        /* Get the file name length */
        UINT16DECODE(p, filename_len);
        HDassert(filename_len < H5R_MAX_ATTR_REF_NAME_LEN);

        /* Allocate space for the file name */
        if(NULL == (filename = (char *)H5MM_malloc(filename_len + 1)))
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, NULL, "memory allocation failed")

        /* Get the file name */
        HDmemcpy(filename, p, filename_len);
        filename[filename_len] = '\0';
        p += filename_len;
    }

    /* Get the object oid for the dataset */
    H5F_addr_decode(oloc.file, &p, &(oloc.addr));

    /* Open and copy the dataset's dataspace */
    if((ret_value = H5S_read(&oloc, dxpl_id)) == NULL)
        HGOTO_ERROR(H5E_DATASPACE, H5E_NOTFOUND, NULL, "not found")

    /* Unserialize the selection */
    if(H5S_SELECT_DESERIALIZE(&ret_value, &p) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, NULL, "can't deserialize selection")

done:
    H5MM_xfree(filename);
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_region() */


/*--------------------------------------------------------------------------
 NAME
    H5Rget_region
 PURPOSE
    Retrieves a dataspace with the region pointed to selected.
 USAGE
    hid_t H5Rget_region(loc_id, ref)
        hid_t loc_id;   IN: Dataset reference object is in or location ID of
                            object that the dataset is located within.
        href_t *ref;    IN: Reference to open.

 RETURNS
    Valid ID on success, Negative on failure
 DESCRIPTION
    Given a reference to some object, creates a copy of the dataset pointed
    to's dataspace and defines a selection in the copy which is the region
    pointed to.
--------------------------------------------------------------------------*/
hid_t
H5Rget_region(hid_t loc_id, const href_t *ref)
{
    H5VL_object_t    *obj = NULL;        /* object token of loc_id */
    H5VL_loc_params_t loc_params;
    hid_t ret_value;

    FUNC_ENTER_API(FAIL)

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(ref->ref_type != H5R_REGION && ref->ref_type != H5R_EXT_REGION)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference type")

    loc_params.type = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type = H5I_get_type(id);

    /* get the file object */
    if(NULL == (obj = (H5VL_object_t *)H5I_object(id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid file identifier")

    /* Get the space id through the VOL */
    if(H5VL_object_get(obj->vol_obj, loc_params, obj->vol_info->vol_cls, H5VL_REF_GET_REGION, 
                       H5AC_ind_dxpl_id, H5_REQUEST_NULL, &ret_value, ref_type, ref) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, FAIL, "unable to get group info")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_region() */


/*--------------------------------------------------------------------------
 NAME
    H5R__get_obj_type
 PURPOSE
    Retrieves the type of object that an object reference points to
 USAGE
    herr_t H5R_get_obj_type(file, dxpl_id, ref, obj_type)
        H5F_t *file;            IN: File the object being dereferenced is within
        hid_t dxpl_id;          IN: Property list ID
        href_t *ref;            IN: Reference to open
        H5O_type_t *obj_type    OUT: Object type

 RETURNS
    Non-negative on success/Negative on failure
 DESCRIPTION
    Given a reference to some object, this function returns the type of object
    pointed to.
--------------------------------------------------------------------------*/
herr_t
H5R__get_obj_type(H5F_t *file, hid_t dxpl_id, const href_t *ref, H5O_type_t *obj_type)
{
    H5O_loc_t oloc;             /* Object location */
    unsigned rc;                /* Reference count of object    */
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(ref);
    HDassert(file);

    /* Initialize the symbol table entry */
    H5O_loc_reset(&oloc);
    oloc.file = file;

    switch(ref->ref_type) {
        case H5R_OBJECT:
            /* Get the object oid */
            oloc.addr = ref->obj_addr;
            break;

        case H5R_REGION:
        {
            const uint8_t *p;   /* Pointer to reference to decode */

            /* Get the object oid for the dataset */
            p = (const uint8_t *)ref->obj_enc.buf;
            H5F_addr_decode(oloc.file, &p, &(oloc.addr));

            break;
        } /* end case */

        case H5R_ATTR:
        {
            const uint8_t *p;   /* Pointer to reference to decode */

            /* Get the object oid for the dataset */
            p = (const uint8_t *)ref->obj_enc.buf;
            H5F_addr_decode(oloc.file, &p, &(oloc.addr));

            break;
        } /* end case */

        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    /* Get the # of links for object, and its type */
    /* (To check to make certain that this object hasn't been deleted since the reference was created) */
    if(H5O_get_rc_and_type(&oloc, dxpl_id, &rc, obj_type) < 0 || 0 == rc)
        HGOTO_ERROR(H5E_REFERENCE, H5E_LINKCOUNT, FAIL, "dereferencing deleted object")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_obj_type() */


/*--------------------------------------------------------------------------
 NAME
    H5Rget_obj_type2
 PURPOSE
    Retrieves the type of object that an object reference points to
 USAGE
    herr_t H5Rget_obj_type2(loc_id, ref, obj_type)
        hid_t loc_id;       IN: Dataset reference object is in or location ID of
                            object that the dataset is located within.
        href_t *ref;        IN: Reference to query.
        H5O_type_t *obj_type;   OUT: Type of object reference points to

 RETURNS
    Non-negative on success/Negative on failure
 DESCRIPTION
    Given a reference to some object, this function retrieves the type of
    object pointed to.
--------------------------------------------------------------------------*/
herr_t
H5Rget_obj_type2(hid_t loc_id, const href_t *ref, H5O_type_t *obj_type)
{
    H5VL_object_t    *obj = NULL;        /* object token of loc_id */
    H5VL_loc_params_t loc_params;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(ref->ref_type <= H5R_BADTYPE || ref->ref_type >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference type")

    loc_params.type = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type = H5I_get_type(id);

    /* get the file object */
    if(NULL == (obj = (H5VL_object_t *)H5I_object(id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid file identifier")

    /* get the object type through the VOL */
    if((ret_value = H5VL_object_get(obj->vol_obj, loc_params, obj->vol_info->vol_cls, 
                                    H5VL_REF_GET_TYPE, H5AC_ind_dxpl_id, 
                                    H5_REQUEST_NULL, obj_type, ref_type, ref)) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, FAIL, "unable to get group info")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_obj_type2() */


/*--------------------------------------------------------------------------
 NAME
    H5R__get_name
 PURPOSE
    Internal routine to determine a name for the object referenced
 USAGE
    ssize_t H5R__get_name(f, lapl_id, dxpl_id, loc_id, ref, name, size)
        H5F_t *f;       IN: Pointer to the file that the reference is pointing
                            into
        hid_t lapl_id;  IN: LAPL to use for operation
        hid_t dxpl_id;  IN: DXPL to use for operation
        hid_t loc_id;   IN: Location ID given for reference
        href_t *ref;    IN: Reference to query.
        char *name;     OUT: Buffer to place name of object referenced
        size_t size;    IN: Size of name buffer

 RETURNS
    Non-negative length of the path on success, Negative on failure
 DESCRIPTION
    Given a reference to some object, determine a path to the object
    referenced in the file.
--------------------------------------------------------------------------*/
static ssize_t
H5R__get_name(H5F_t *f, hid_t lapl_id, hid_t dxpl_id, hid_t loc_id, const href_t *ref,
    char *name, size_t size)
{
    hid_t file_id = H5I_BADID;  /* ID for file that the reference is in */
    H5O_loc_t oloc;             /* Object location describing object for reference */
    ssize_t ret_value = -1;     /* Return value */
    char *filename = NULL;      /* Name of the file (for external references) */
    const uint8_t *p;           /* Pointer to reference to decode */

    FUNC_ENTER_NOAPI_NOINIT

    /* Check args */
    HDassert(f);
    HDassert(ref);

    /* Get the file pointer from the entry */
    f = loc->oloc->file;
    HDassert(f);

    /* Initialize the object location */
    H5O_loc_reset(&oloc);
    oloc.file = f;

    switch (ref->ref_type) {
        case H5R_EXT_OBJECT:
        case H5R_EXT_REGION:
        case H5R_EXT_ATTR:
        {
            size_t filename_len; /* Length of the file name */

            /* Get the file name length */
            p = (const uint8_t *)ref->obj_enc.buf;
            UINT16DECODE(p, filename_len);
            HDassert(filename_len < H5R_MAX_ATTR_REF_NAME_LEN);

            /* Allocate space for the file name */
            if(NULL == (filename = (char *)H5MM_malloc(filename_len + 1)))
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTALLOC, FAIL, "memory allocation failed")

            /* Get the file name */
            HDmemcpy(filename, p, filename_len);
            filename[filename_len] = '\0';
            p += filename_len;
        }
            break;
        case H5R_OBJECT:
        case H5R_REGION:
        case H5R_ATTR:
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            break;
    }

    /* Get address for reference */
    switch(ref->ref_type) {
        case H5R_OBJECT:
            oloc.addr = ref->obj_addr;
            break;

        case H5R_EXT_OBJECT:
        case H5R_EXT_REGION:
        case H5R_REGION:
        {
            /* Get the object oid for the dataset */
            if (!p) p = (const uint8_t *)ref->obj_enc.buf;
            H5F_addr_decode(oloc.file, &p, &(oloc.addr));

        } /* end case */
        break;

        case H5R_EXT_ATTR:
        case H5R_ATTR:
        {
            size_t attr_name_len; /* Length of the attribute name */
            size_t copy_len;

            /* Get the object oid for the dataset */
            if (!p) p = (const uint8_t *)ref->obj_enc.buf;
            H5F_addr_decode(oloc.file, &p, &(oloc.addr));

            /* Get the attribute name length */
            UINT16DECODE(p, attr_name_len);
            HDassert(attr_name_len < H5R_MAX_ATTR_REF_NAME_LEN);
            copy_len = MIN(attr_name_len, size - 1);

            /* Get the attribute name */
            if (name) {
                HDmemcpy(name, p, copy_len);
                name[copy_len] = '\0';
            }
            ret_value = (ssize_t)copy_len;
        } /* end case */
        break;
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
    } /* end switch */

    if (ref->ref_type != H5R_ATTR && ref->ref_type != H5R_EXT_ATTR) {
        /* Retrieve file ID for name search */
        if((file_id = H5I_get_file_id(loc_id, FALSE)) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, FAIL, "can't retrieve file ID")

            /* Get name, length, etc. */
            if((ret_value = H5G_get_name_by_addr(file_id, lapl_id, dxpl_id, &oloc, name, size)) < 0)
                HGOTO_ERROR(H5E_REFERENCE, H5E_CANTGET, FAIL, "can't determine name")
    }

done:
    /* Close file ID used for search */
    if(file_id > 0 && H5I_dec_ref(file_id) < 0)
        HDONE_ERROR(H5E_REFERENCE, H5E_CANTDEC, FAIL, "can't decrement ref count of temp ID")
    H5MM_xfree(filename);
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_name() */


/*--------------------------------------------------------------------------
 NAME
    H5Rget_name
 PURPOSE
    Determines a name for the object referenced
 USAGE
    ssize_t H5Rget_name(loc_id, ref, name, size)
        hid_t loc_id;   IN: Dataset reference object is in or location ID of
                            object that the dataset is located within.
        href_t *ref;    IN: Reference to query.
        char *name;     OUT: Buffer to place name of object referenced. If NULL
	                     then this call will return the size in bytes of name.
        size_t size;    IN: Size of name buffer (user needs to include NULL terminator
                            when passing in the size)

 RETURNS
    Non-negative length of the path on success, Negative on failure
 DESCRIPTION
    Given a reference to some object, determine a path to the object
    referenced in the file.
--------------------------------------------------------------------------*/
ssize_t
H5Rget_name(hid_t loc_id, const href_t *ref, char *name, size_t size)
{
    H5VL_object_t    *obj = NULL;        /* object token of loc_id */
    H5VL_loc_params_t loc_params;
    ssize_t ret_value;  /* Return value */

    FUNC_ENTER_API(FAIL)

    /* Check args */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location")
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(ref->ref_type <= H5R_BADTYPE || ref->ref_type >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference type")

    loc_params.type = H5VL_OBJECT_BY_SELF;
    loc_params.obj_type = H5I_get_type(id);

    /* get the file object */
    if(NULL == (obj = (H5VL_object_t *)H5I_object(id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "invalid file identifier")

    /* get the object type through the VOL */
    if(H5VL_object_get(obj->vol_obj, loc_params, obj->vol_info->vol_cls, H5VL_REF_GET_NAME, 
                       H5AC_ind_dxpl_id, H5_REQUEST_NULL, 
                       &ret_value, name, size, ref_type, _ref) < 0)
        HGOTO_ERROR(H5E_INTERNAL, H5E_CANTGET, FAIL, "unable to get group info")
done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_name() */

/*--------------------------------------------------------------------------
 NAME
    H5R__get_filename
 PURPOSE
    Determines a file name for the object referenced
 USAGE
    ssize_t H5R__get_filename(ref, name, size)
        href_t *ref;    IN: Reference to query.
        char *name;     OUT: Buffer to place file name of object referenced. If NULL
                         then this call will return the size in bytes of name.
        size_t size;    IN: Size of name buffer (user needs to include NULL terminator
                            when passing in the size)

 RETURNS
    Non-negative length of the path on success, Negative on failure
 DESCRIPTION
    Given a reference to some object, determine a file name to the object
    referenced.
--------------------------------------------------------------------------*/
static ssize_t
H5R__get_filename(const href_t *ref, char *name, size_t size)
{
    ssize_t ret_value = -1;     /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* Check args */
    HDassert(ref);

    switch (ref->ref_type) {
        case H5R_EXT_OBJECT:
        case H5R_EXT_REGION:
        case H5R_EXT_ATTR:
        {
            const uint8_t *p;    /* Pointer to reference to decode */
            size_t filename_len; /* Length of the file name */
            size_t copy_len;

            /* Get the file name length */
            p = (const uint8_t *)ref->obj_enc.buf;
            UINT16DECODE(p, filename_len);
            copy_len = MIN(filename_len, size - 1);

            /* Get the attribute name */
            if (name) {
                HDmemcpy(name, p, copy_len);
                name[copy_len] = '\0';
            }
            ret_value = (ssize_t)copy_len;
        }
            break;
        case H5R_OBJECT:
        case H5R_REGION:
        case H5R_ATTR:
        case H5R_BADTYPE:
        case H5R_MAXTYPE:
        default:
            HDassert("unknown reference type" && 0);
            HGOTO_ERROR(H5E_REFERENCE, H5E_UNSUPPORTED, FAIL, "internal error (unknown reference type)")
            break;
    } /* end switch */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5R__get_filename() */

/*--------------------------------------------------------------------------
 NAME
    H5Rget_filename
 PURPOSE
    Determines a file name for the object referenced
 USAGE
    ssize_t H5Rget_filename(ref, name, size)
        href_t *ref;    IN: Reference to query.
        char *name;     OUT: Buffer to place file name of object referenced. If NULL
                         then this call will return the size in bytes of name.
        size_t size;    IN: Size of name buffer (user needs to include NULL terminator
                            when passing in the size)

 RETURNS
    Non-negative length of the path on success, Negative on failure
 DESCRIPTION
    Given a reference to some object, determine a file name to the object
    referenced.
--------------------------------------------------------------------------*/
ssize_t
H5Rget_filename(const href_t *ref, char *name, size_t size)
{
    ssize_t ret_value;  /* Return value */

    FUNC_ENTER_API(FAIL)

    /* Check args */
    if(ref == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference pointer")
    if(ref->ref_type <= H5R_BADTYPE || ref->ref_type >= H5R_MAXTYPE)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid reference type")

    /* Get name */
    if((ret_value = H5R__get_filename(ref, name, size)) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTINIT, FAIL, "unable to retrieve file name")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Rget_filename() */
