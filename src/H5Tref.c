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

/*
 * Module Info: This module contains the functionality for reference
 *      datatypes in the H5T interface.
 */

#include "H5Tmodule.h"          /* This source code file is part of the H5T module */
#define H5F_FRIEND              /*suppress error about including H5Fpkg   */
#define H5R_FRIEND              /*suppress error about including H5Rpkg   */

#include "H5private.h"          /* Generic Functions    */
#include "H5Eprivate.h"         /* Error handling       */
#include "H5Iprivate.h"         /* IDs                  */
#include "H5MMprivate.h"        /* Memory management    */
#include "H5HGprivate.h"        /* Global Heaps         */
#include "H5Fpkg.h"             /* File                 */
#include "H5Rpkg.h"             /* References           */
#include "H5Tpkg.h"             /* Datatypes            */

/****************/
/* Local Macros */
/****************/

#define H5T_REF_MEM_SIZE        (H5R_REF_BUF_SIZE)

/* Size of element on disk is 4 bytes for the size, plus the size
 * of an address in this file, plus 4 bytes for the size of a heap ID
 */
#define H5T_REF_DISK_SIZE(f)    ((2 * H5_SIZEOF_UINT32_T) + (size_t)H5F_SIZEOF_ADDR(f))

/******************/
/* Local Typedefs */
/******************/

struct H5T_ref_disk {
    hid_t loc_id;
    void *buf; /* Keep it last */
};

/********************/
/* Local Prototypes */
/********************/

static size_t H5T__ref_mem_getsize(H5F_t *f, const void *buf, size_t buf_size);
static herr_t H5T__ref_mem_read(H5F_t *f, const void *src_buf, size_t src_size, void *dest_buf, size_t dest_size);
static herr_t H5T__ref_mem_write(H5F_t *f, const void *src_buf, size_t src_size, void *dest_buf, size_t dest_size, void *bg_buf);

static size_t H5T__ref_disk_getsize(H5F_t *f, const void *buf, size_t buf_size);
static herr_t H5T__ref_disk_read(H5F_t *f, const void *src_buf, size_t src_size, void *dest_buf, size_t dest_size);
static herr_t H5T__ref_disk_write(H5F_t *f, const void *src_buf, size_t src_size, void *dest_buf, size_t dest_size, void *bg_buf);

/*******************/
/* Local Variables */
/*******************/

/*-------------------------------------------------------------------------
 * Function: H5T__ref_set_loc
 *
 * Purpose:	Sets the location of a reference datatype to be either on disk
 *          or in memory
 *
 * Return:
 *  One of two values on success:
 *      TRUE - If the location of any reference types changed
 *      FALSE - If the location of any reference types is the same
 *  Negative value is returned on failure
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5T__ref_set_loc(const H5T_t *dt, H5F_t *f, H5T_loc_t loc)
{
    htri_t ret_value = FALSE; /* Indicate success, but no location change */

    FUNC_ENTER_PACKAGE

    HDassert(dt);
    /* f is NULL when loc == H5T_LOC_MEMORY */
    HDassert(loc >= H5T_LOC_BADLOC && loc < H5T_LOC_MAXLOC);
    /* We currently do not let H5R_OBJECT1 and H5R_DATASET_REGION1 go through type conversion */
    HDassert(dt->shared->u.atomic.u.r.rtype != H5R_OBJECT1
        && dt->shared->u.atomic.u.r.rtype != H5R_DATASET_REGION1);

    /* Only change the location if it's different */
    if(loc != dt->shared->u.atomic.u.r.loc || f != dt->shared->u.atomic.u.r.f) {
        switch(loc) {
            case H5T_LOC_MEMORY: /* Memory based reference datatype */
                HDassert(NULL == f);

                /* Mark this type as being stored in memory */
                dt->shared->u.atomic.u.r.loc = H5T_LOC_MEMORY;

                /* Size in memory, disk size is different */
                dt->shared->size = H5T_REF_MEM_SIZE;

                /* Set up the function pointers to access the reference in memory */
                dt->shared->u.atomic.u.r.getsize = H5T__ref_mem_getsize;
                dt->shared->u.atomic.u.r.read = H5T__ref_mem_read;
                dt->shared->u.atomic.u.r.write = H5T__ref_mem_write;

                /* Reset file ID (since this reference is in memory) */
                dt->shared->u.atomic.u.r.f = f;     /* f is NULL */
                break;

            case H5T_LOC_DISK: /* Disk based reference datatype */
                HDassert(f);

                /* Mark this type as being stored on disk */
                dt->shared->u.atomic.u.r.loc = H5T_LOC_DISK;

                /* Size on disk, memory size is different */
                dt->shared->size = H5T_REF_DISK_SIZE(f);

                /* Set up the function pointers to access the information on
                 * disk. Region and attribute references are stored identically
                 * on disk, so use the same functions
                 */
                dt->shared->u.atomic.u.r.getsize = H5T__ref_disk_getsize;
                dt->shared->u.atomic.u.r.read = H5T__ref_disk_read;
                dt->shared->u.atomic.u.r.write = H5T__ref_disk_write;

                /* Set file pointer (since this reference is on disk) */
                dt->shared->u.atomic.u.r.f = f;
                break;

            case H5T_LOC_BADLOC:
            case H5T_LOC_MAXLOC:
            default:
                HGOTO_ERROR(H5E_DATATYPE, H5E_BADRANGE, FAIL, "invalid reference datatype location")
        } /* end switch */

        /* Indicate that the location changed */
        ret_value = TRUE;
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5T__ref_set_loc() */


/*-------------------------------------------------------------------------
 * Function:	H5T__ref_mem_getsize
 *
 * Purpose:	Retrieves the size of a memory based reference.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5T__ref_mem_getsize(H5F_t H5_ATTR_UNUSED *f, const void *buf, size_t buf_size)
{
    size_t ret_value = 0;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(buf);
    HDassert(buf_size == H5T_REF_MEM_SIZE);

    /* Get encoding size */
    ret_value = ((const struct href *)buf)->encode_size;

    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5T__ref_mem_getsize() */


/*-------------------------------------------------------------------------
 * Function:	H5T__ref_mem_read
 *
 * Purpose:	"Reads" the memory based reference into a buffer
 *
 * Return:	Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5T__ref_mem_read(H5F_t H5_ATTR_UNUSED *f, const void *src_buf, size_t src_size,
    void *dest_buf, size_t dest_size)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(src_buf);
    HDassert(src_size == H5T_REF_MEM_SIZE);
    HDassert(dest_buf);
    HDassert(dest_size);

    /* Encode reference */
    if(H5R__encode((const struct href *)src_buf, dest_buf, &dest_size) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTENCODE, FAIL, "Cannot encode reference")

done:
    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5T__ref_mem_read() */


/*-------------------------------------------------------------------------
 * Function:	H5T__ref_mem_write
 *
 * Purpose:	"Writes" the memory reference from a buffer
 *
 * Return:	Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5T__ref_mem_write(H5F_t H5_ATTR_UNUSED *f, const void *src_buf, size_t src_size,
    void *dest_buf, size_t dest_size, void H5_ATTR_UNUSED *bg_buf)
{
    const uint8_t *p = (const uint8_t *)src_buf;
    struct href *dest_ref = (struct href *)dest_buf;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(src_buf);
    HDassert(src_size);
    HDassert(dest_buf);
    HDassert(dest_size == H5T_REF_MEM_SIZE);

    /* Decode reference */
    src_size -= H5_SIZEOF_HID_T;
    if(H5R__decode((const unsigned char *)p, &src_size, dest_ref) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Cannot decode reference")
    p += src_size;

    /* Set location ID */
    if(H5R__set_loc_id(dest_ref, *(hid_t *)p) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTSET, FAIL, "unable to attach location id to reference")

done:
    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5T__ref_mem_write() */


/*-------------------------------------------------------------------------
 * Function:	H5T__ref_disk_getsize
 *
 * Purpose:	Retrieves the length of a disk based reference.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5T__ref_disk_getsize(H5F_t *f, const void *buf, size_t buf_size)
{
    const uint8_t *p = (const uint8_t *)buf;
    size_t ret_value = 0;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(buf);
    HDassert(buf_size == H5T_REF_DISK_SIZE(f));

    /* Retrieve encoded data size */
    UINT32DECODE(p, ret_value);

    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5T__ref_disk_getsize() */


/*-------------------------------------------------------------------------
 * Function:	H5T__ref_disk_read
 *
 * Purpose:	Reads the disk based reference into a buffer
 *
 * Return:	Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5T__ref_disk_read(H5F_t *f, const void *src_buf, size_t src_size,
    void *dest_buf, size_t dest_size)
{
    const uint8_t *p = (const uint8_t *)src_buf;
    const uint8_t *q = (const uint8_t *)dest_buf;
    size_t buf_size_left = src_size;
    size_t expected_size = dest_size;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(f);
    HDassert(src_buf);
    HDassert(src_size == H5T_REF_DISK_SIZE(f));
    HDassert(dest_buf);
    HDassert(dest_size);

    /* Skip the length of the sequence */
    p += H5_SIZEOF_UINT32_T;
    HDassert(buf_size_left > H5_SIZEOF_UINT32_T);
    buf_size_left -= H5_SIZEOF_UINT32_T;

    /* Decode from heap */
    if(H5R__decode_heap(f, p, &buf_size_left, (unsigned char **)&q, &dest_size) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Cannot decode reference from heap")
    q += dest_size;

    /* Retrieve file and pass around file ID */
    dest_size += H5_SIZEOF_HID_T;
    if((*(hid_t *)q = H5F__get_file_id(f)) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a file or file object")

    if(dest_size != expected_size)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Expected data size does not match")
done:
    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5T__ref_disk_read() */


/*-------------------------------------------------------------------------
 * Function:	H5T__ref_disk_write
 *
 * Purpose:	Writes the disk based reference from a buffer
 *
 * Return:	Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5T__ref_disk_write(H5F_t *f, const void *src_buf, size_t src_size,
    void *dest_buf, size_t dest_size, void *bg_buf)
{
    uint8_t *p = (uint8_t *)dest_buf;
    size_t buf_size_left = dest_size;
    uint8_t *p_bg = (uint8_t *)bg_buf;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(f);
    HDassert(src_buf);
    HDassert(src_size);
    HDassert(dest_buf);
    HDassert(dest_size == H5T_REF_DISK_SIZE(f));

    /* TODO Should get rid of bg stuff */
    if(p_bg) {
        size_t p_buf_size_left = dest_size;

        /* Skip the length of the reference */
        p_bg += H5_SIZEOF_UINT32_T;
        HDassert(p_buf_size_left > H5_SIZEOF_UINT32_T);
        p_buf_size_left -= H5_SIZEOF_UINT32_T;

        /* Free heap object for old data */
        if(H5R__free_heap(f, p_bg, p_buf_size_left) < 0)
            HGOTO_ERROR(H5E_REFERENCE, H5E_CANTFREE, FAIL, "Cannot free reference from heap")
    } /* end if */

    /* Set the size (include size for passing around file hid_t) */
    UINT32ENCODE(p, src_size + H5_SIZEOF_HID_T);
    HDassert(buf_size_left > H5_SIZEOF_UINT32_T);
    buf_size_left -= H5_SIZEOF_UINT32_T;

    /* Encode to heap */
    if(H5R__encode_heap(f, p, &buf_size_left, src_buf, src_size) < 0)
        HGOTO_ERROR(H5E_REFERENCE, H5E_CANTDECODE, FAIL, "Cannot decode reference from heap")

done:
    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5T__ref_disk_write() */
