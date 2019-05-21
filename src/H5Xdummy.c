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

/*
 * Purpose:	Dummy index routines.
 */

/****************/
/* Module Setup */
/****************/

/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions */
#include "H5Xprivate.h"     /* Index */
#include "H5Eprivate.h"		/* Error handling */
#include "H5Iprivate.h"		/* IDs */
#include "H5MMprivate.h"	/* Memory management */
#include "H5Pprivate.h"
#include "H5Qprivate.h"
#include "H5Sprivate.h"
/* TODO using private headers but could use public ones */

/****************/
/* Local Macros */
/****************/
//#define H5X_DUMMY_DEBUG

#ifdef H5X_DUMMY_DEBUG
#define H5X_DUMMY_LOG_DEBUG(...) do {                           \
      fprintf(stdout, " # %s(): ", __func__);                   \
      fprintf(stdout, __VA_ARGS__);                             \
      fprintf(stdout, "\n");                                    \
      fflush(stdout);                                           \
  } while (0)
#else
#define H5X_DUMMY_LOG_DEBUG(...) do { \
  } while (0)
#endif

/******************/
/* Local Typedefs */
/******************/
typedef struct H5X_dummy_t {
    hid_t dataset_id;
    hid_t idx_anon_id;
    void *idx_token;
    size_t idx_token_size;
} H5X_dummy_t;

typedef struct H5X__dummy_query_data_t {
    size_t num_elmts;
    hid_t query_id;
    hid_t space_query;
} H5X__dummy_query_data_t;

/********************/
/* Local Prototypes */
/********************/

static void *
H5X_dummy_create(hid_t dataset_id, hid_t xcpl_id, hid_t xapl_id,
        size_t *metadata_size, void **metadata);

static herr_t
H5X_dummy_remove(hid_t file_id, size_t metadata_size, void *metadata);

static void *
H5X_dummy_open(hid_t dataset_id, hid_t xapl_id, size_t metadata_size,
        void *metadata);

static herr_t
H5X_dummy_close(void *idx_handle);

static herr_t
H5X_dummy_pre_update(void *idx_handle, hid_t dataspace_id, hid_t xxpl_id);

static herr_t
H5X_dummy_post_update(void *idx_handle, const void *buf, hid_t dataspace_id,
        hid_t xxpl_id);

static hid_t
H5X_dummy_query(void *idx_handle, hid_t dataspace_id, hid_t query_id,
        hid_t xxpl_id);

static herr_t
H5X__dummy_get_query_data_cb(void *elem, hid_t type_id, unsigned ndim,
        const hsize_t *point, void *_udata);

static herr_t
H5X_dummy_get_size(void *idx_handle, hsize_t *idx_size);

/*********************/
/* Package Variables */
/*********************/

/*****************************/
/* Library Private Variables */
/*****************************/

/*******************/
/* Local Variables */
/*******************/

/* Dummy index class */
static H5X_idx_class_t idx_class = {.data_class = {
    H5X_dummy_create,       /* create */
    H5X_dummy_remove,       /* remove */
    H5X_dummy_open,         /* open */
    H5X_dummy_close,        /* close */
    H5X_dummy_pre_update,   /* pre_update */
    H5X_dummy_post_update,  /* post_update */
    H5X_dummy_query,        /* query */
    NULL,                   /* refresh */
    NULL,                   /* copy */
    H5X_dummy_get_size      /* get_size */
}};

const H5X_class_t H5X_DUMMY[1] = {{
    H5X_CLASS_T_VERS,       /* (From the H5Xpublic.h header file) */
    H5X_PLUGIN_DUMMY,       /* (Or whatever number is assigned) */
    "dummy index plugin",   /* Whatever name desired */
    H5X_TYPE_DATA,          /* This plugin operates on dataset elements */
    H5X_SIMPLE_QUERY|H5X_COMPOUND_QUERY,   /* plugin does support multiple condition (compound) queries */
    &idx_class              /* Index class */
}};

/*-------------------------------------------------------------------------
 * Function:    H5X__dummy_read_data
 *
 * Purpose: Read data from dataset.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5X__dummy_read_data(hid_t dataset_id, void **buf, size_t *buf_size)
{
    herr_t ret_value = SUCCEED; /* Return value */
    hid_t type_id = FAIL, space_id = FAIL;
    size_t nelmts, elmt_size;
    void *data = NULL;
    size_t data_size;

    FUNC_ENTER_NOAPI_NOINIT

    /* Get space info */
    if (FAIL == (type_id = H5Dget_type(dataset_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, FAIL, "can't get type from dataset");
    if (FAIL == (space_id = H5Dget_space(dataset_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, FAIL, "can't get dataspace from dataset");
    if (0 == (nelmts = (size_t) H5Sget_select_npoints(space_id)))
        HGOTO_ERROR(H5E_DATASPACE, H5E_BADVALUE, FAIL, "invalid number of elements");
    if (0 == (elmt_size = H5Tget_size(type_id)))
        HGOTO_ERROR(H5E_DATATYPE, H5E_BADTYPE, FAIL, "invalid size of element");

    /* Allocate buffer to hold data */
    data_size = nelmts * elmt_size;
    if (NULL == (data = H5MM_malloc(data_size)))
        HGOTO_ERROR(H5E_INDEX, H5E_NOSPACE, FAIL, "can't allocate read buffer");

    /* Read data from dataset */
    if (FAIL == H5Dread(dataset_id, type_id, H5S_ALL, space_id, H5P_DEFAULT, data))
        HGOTO_ERROR(H5E_INDEX, H5E_READERROR, FAIL, "can't read data");

    *buf = data;
    *buf_size = data_size;

done:
    if (type_id != FAIL)
        H5Tclose(type_id);
    if (space_id != FAIL)
        H5Sclose(space_id);
    if (ret_value == FAIL) {
        *buf = NULL;
	*buf_size = 0;
        H5MM_free(data);
    }
    FUNC_LEAVE_NOAPI(ret_value)
}

/*-------------------------------------------------------------------------
 * Function:    H5X_dummy_create
 *
 * Purpose: This function creates a new instance of a dummy plugin index.
 *
 * Return:  Success:    Pointer to the new index
 *          Failure:    NULL
 *
 *------------------------------------------------------------------------
 */
static void *
H5X_dummy_create(hid_t dataset_id, hid_t H5_ATTR_UNUSED xcpl_id, hid_t H5_ATTR_UNUSED xapl_id,
        size_t *metadata_size, void **metadata)
{
    static int entered_count=0;
    H5X_dummy_t *dummy = NULL;
    hid_t file_id = FAIL, type_id, space_id;
    void *buf = NULL;
    size_t buf_size;
    H5O_info_t dset_info;
    void *ret_value = NULL; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    H5X_DUMMY_LOG_DEBUG("Calling H5X_dummy_create");

    if (NULL == (dummy = (H5X_dummy_t *) H5MM_malloc(sizeof(H5X_dummy_t))))
        HGOTO_ERROR(H5E_INDEX, H5E_NOSPACE, NULL, "can't allocate dummy struct");

    dummy->dataset_id = dataset_id;

    if (FAIL == (type_id = H5Dget_type(dataset_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, NULL, "can't get type from dataset");
    if (FAIL == (space_id = H5Dget_space(dataset_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, NULL, "can't get dataspace from dataset");

    entered_count++;
    // printf("H5X_dummy_create: entered=%d\tdataset=%lld\n", entered_count, dataset_id);    
    /* Get data from dataset */
    if (FAIL == H5X__dummy_read_data(dataset_id, &buf, &buf_size))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, NULL, "can't get data from dataset");

    /* Get file ID */
    if (FAIL == (file_id = H5Iget_file_id(dataset_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, NULL, "can't get file ID from dataset");

    /* Create anonymous datasets */
    if (FAIL == (dummy->idx_anon_id = H5Dcreate_anon(file_id, type_id, space_id,
            H5P_DEFAULT, H5P_DEFAULT)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTCREATE, NULL, "can't create anonymous dataset");

    /* Update index elements (simply write data for now) */
    if (FAIL == H5Dwrite(dummy->idx_anon_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTUPDATE, NULL, "can't update index elements");

    /* Increment refcount so that anonymous dataset is persistent */
    if (FAIL == H5Oincr_refcount(dummy->idx_anon_id))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTINC, NULL, "can't increment dataset refcount");

    /* Get addr info for dataset */
    if (FAIL == H5Oget_info(dummy->idx_anon_id, &dset_info))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, NULL, "can't get dataset info");

    dummy->idx_token_size = sizeof(haddr_t);

    if (NULL == (dummy->idx_token = H5MM_malloc(dummy->idx_token_size)))
        HGOTO_ERROR(H5E_INDEX, H5E_NOSPACE, NULL, "can't allocate token  for anonymous dataset");

    HDmemcpy(dummy->idx_token, &dset_info.addr, sizeof(haddr_t));

    /* Metadata is token for anonymous dataset */
    *metadata = dummy->idx_token;
    *metadata_size = dummy->idx_token_size;

    ret_value = dummy;

done:
    if (FAIL != file_id)
        H5Fclose(file_id);
    if (buf) H5MM_free(buf);
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_dummy_create() */

/*-------------------------------------------------------------------------
 * Function:    H5X_dummy_remove
 *
 * Purpose: This function removes the dummy plugin index from the file.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5X_dummy_remove(hid_t file_id, size_t metadata_size, void *metadata)
{
    hid_t idx_anon_id = FAIL;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    H5X_DUMMY_LOG_DEBUG("Calling H5X_dummy_remove");

    if (metadata_size < sizeof(haddr_t))
        HGOTO_ERROR(H5E_INDEX, H5E_BADVALUE, FAIL, "metadata size is not valid");

    if (FAIL == (idx_anon_id = H5Oopen_by_addr(file_id, *((haddr_t *) metadata))))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTOPENOBJ, FAIL, "can't open anonymous dataset");

    /* Decrement refcount so that anonymous dataset gets deleted */
    if (FAIL == H5Odecr_refcount(idx_anon_id))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTDEC, FAIL, "can't decrement dataset refcount");

done:
    if (FAIL != idx_anon_id)
        H5Dclose(idx_anon_id);
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_dummy_remove() */

/*-------------------------------------------------------------------------
 * Function:    H5X_dummy_open
 *
 * Purpose: This function open an already existing dummy index from a file.
 *
 * Return:  Success:    Pointer to the index
 *          Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
static void *
H5X_dummy_open(hid_t dataset_id, hid_t H5_ATTR_UNUSED xapl_id, size_t metadata_size,
        void *metadata)
{
    static int entered_count=0;
    hid_t file_id;
    H5X_dummy_t *dummy = NULL;
    void *ret_value = NULL; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    H5X_DUMMY_LOG_DEBUG("Calling H5X_dummy_open");

    if (!metadata_size)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "NULL metadata size");
    if (!metadata)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "NULL metadata");

    if (NULL == (dummy = (H5X_dummy_t *) H5MM_malloc(sizeof(H5X_dummy_t))))
        HGOTO_ERROR(H5E_INDEX, H5E_NOSPACE, NULL, "can't allocate dummy struct");

    entered_count++;
    dummy->dataset_id = dataset_id;
    dummy->idx_token_size = metadata_size;
    if (NULL == (dummy->idx_token = H5MM_malloc(dummy->idx_token_size)))
        HGOTO_ERROR(H5E_INDEX, H5E_NOSPACE, NULL, "can't allocate token");
    HDmemcpy(dummy->idx_token, metadata, dummy->idx_token_size);

    if (FAIL == (file_id = H5Iget_file_id(dataset_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, NULL, "can't get file ID from dataset");

    if (FAIL == (dummy->idx_anon_id = H5Oopen_by_addr(file_id, *((haddr_t *) dummy->idx_token))))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTOPENOBJ, NULL, "can't open anonymous dataset");

    ret_value = dummy;
    // printf("H5X_dummy_open: entered=%d\tdataset=%lld\n", entered_count, dataset_id);
done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_dummy_open() */

/*-------------------------------------------------------------------------
 * Function:    H5X_dummy_close
 *
 * Purpose: This function unregisters an index class.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5X_dummy_close(void *idx_handle)
{
    H5X_dummy_t *dummy = (H5X_dummy_t *) idx_handle;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    H5X_DUMMY_LOG_DEBUG("Calling H5X_dummy_close");

    if (NULL == dummy)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL index handle");

    /* Close anonymous dataset */
    if (FAIL == H5Dclose(dummy->idx_anon_id))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTCLOSEOBJ, FAIL, "can't close anonymous dataset for index");

    if (dummy->idx_token != NULL)
    H5MM_free(dummy->idx_token);
    H5MM_free(dummy);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_dummy_close() */

/*-------------------------------------------------------------------------
 * Function:    H5X_dummy_pre_update
 *
 * Purpose: This function unregisters an index class.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5X_dummy_pre_update(void *idx_handle, hid_t H5_ATTR_UNUSED dataspace_id, hid_t H5_ATTR_UNUSED xxpl_id)
{
    H5X_dummy_t *dummy = (H5X_dummy_t *) idx_handle;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    H5X_DUMMY_LOG_DEBUG("Calling H5X_dummy_pre_update");

    if (NULL == dummy)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL index handle");

    /* Not needed here */
done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_dummy_pre_update() */

/*-------------------------------------------------------------------------
 * Function:    H5X_dummy_post_update
 *
 * Purpose: This function unregisters an index class.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5X_dummy_post_update(void *idx_handle, const void *buf, hid_t dataspace_id,
        hid_t H5_ATTR_UNUSED xxpl_id)
{
    H5X_dummy_t *dummy = (H5X_dummy_t *) idx_handle;
    hid_t mem_type_id, file_space_id;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    H5X_DUMMY_LOG_DEBUG("Calling H5X_dummy_post_update");

    if (NULL == dummy)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL index handle");

    if (FAIL == (mem_type_id = H5Dget_type(dummy->idx_anon_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, FAIL, "can't get type from dataset");
    if (FAIL == (file_space_id = H5Dget_space(dummy->idx_anon_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, FAIL, "can't get dataspace from dataset");

    /* Update index elements (simply write data for now) */
    if (FAIL == H5Dwrite(dummy->idx_anon_id, mem_type_id, dataspace_id,
            file_space_id, H5P_DEFAULT, buf))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTUPDATE, FAIL, "can't update index elements");
done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_dummy_post_update() */

/*-------------------------------------------------------------------------
 * Function:    H5X__dummy_get_query_data_cb
 *
 * Purpose: This function unregisters an index class.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5X__dummy_get_query_data_cb(void *elem, hid_t type_id, unsigned H5_ATTR_UNUSED ndim,
        const hsize_t *point, void *_udata)
{
    H5X__dummy_query_data_t *udata = (H5X__dummy_query_data_t *)_udata;
    hbool_t result;
    herr_t ret_value = SUCCEED;
    hsize_t count[H5S_MAX_RANK + 1];
    int i;

    FUNC_ENTER_NOAPI_NOINIT

    /* Apply the query */
    if (H5Qapply_atom(udata->query_id, &result, type_id, elem) < 0)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query to data element");

    /* Initialize count */
    for (i = 0; i < H5S_MAX_RANK; i++)
        count[i] = 1;

    /* If element satisfies query, add it to the selection */
    if (result) {
        /* TODO remove that after demo */
        /* printf("Element |%d| matches query\n", *((int *) elem)); */
        udata->num_elmts++;

        /* Add converted coordinate to selection */
        if (H5Sselect_hyperslab(udata->space_query, H5S_SELECT_OR, point, NULL, count, NULL))
            HGOTO_ERROR(H5E_DATASPACE, H5E_CANTSET, FAIL, "unable to add point to selection");

    }

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X__dummy_get_query_data_cb */

/*-------------------------------------------------------------------------
 * Function:    H5X_dummy_query
 *
 * Purpose: This function unregisters an index class.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static hid_t
H5X_dummy_query(void *idx_handle, hid_t H5_ATTR_UNUSED dataspace_id, hid_t query_id,
        hid_t H5_ATTR_UNUSED xxpl_id)
{
    H5X_dummy_t *dummy = (H5X_dummy_t *) idx_handle;
    H5X__dummy_query_data_t udata;
    hid_t space_id, type_id;
    size_t nelmts;
    size_t elmt_size = 0, buf_size = 0;
    void *buf = NULL;
    hid_t ret_value = FAIL; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    H5X_DUMMY_LOG_DEBUG("Calling H5X_dummy_query");

    if (NULL == dummy)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL index handle");
    if (FAIL == (type_id = H5Dget_type(dummy->idx_anon_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, FAIL, "can't get type from index");
    if (FAIL == (space_id = H5Dget_space(dummy->idx_anon_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTGET, FAIL, "can't get dataspace from index");
    if (0 == (nelmts = (size_t) H5Sget_select_npoints(space_id)))
        HGOTO_ERROR(H5E_DATASPACE, H5E_BADVALUE, FAIL, "invalid number of elements");
    if (0 == (elmt_size = H5Tget_size(type_id)))
        HGOTO_ERROR(H5E_DATATYPE, H5E_BADTYPE, FAIL, "invalid size of element");

    /* allocate buffer to hold data */
    buf_size = nelmts * elmt_size;
    if(NULL == (buf = H5MM_malloc(buf_size)))
        HGOTO_ERROR(H5E_INDEX, H5E_NOSPACE, FAIL, "can't allocate read buffer");

    /* read data from index */
    if (FAIL == H5Dread(dummy->idx_anon_id, type_id, H5S_ALL, space_id,
            H5P_DEFAULT, buf))
        HGOTO_ERROR(H5E_INDEX, H5E_READERROR, FAIL, "can't read data");

    if(FAIL == (udata.space_query = H5Scopy(space_id)))
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTINIT, FAIL, "unable to copy dataspace");
    if(H5Sselect_none(udata.space_query) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTINIT, FAIL, "unable to reset selection");

    udata.num_elmts = 0;
    udata.query_id = query_id;

    /* iterate over every element and apply the query on it. If the
       query is not satisfied, then remove it from the query selection */
    /* Fix-ME! don't iterate */
#if 0
    if (H5Xgather_local_indices(buf, type_id, space_id, H5X__dummy_get_query_data_cb, &udata) >= 0) {
        printf("H5Xgather_local_indices returned success\n");
    }
    else
#endif
    if (H5Diterate(buf, type_id, space_id, H5X__dummy_get_query_data_cb, &udata) < 0)
        HGOTO_ERROR(H5E_INDEX, H5E_CANTCOMPUTE, FAIL, "failed to compute buffer size");

    ret_value = udata.space_query;
    H5X_DUMMY_LOG_DEBUG("Created dataspace from index with %d elements",
            (int) H5Sget_select_npoints(ret_value));

done:
    if (buf) H5MM_free(buf);
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_dummy_query() */

/*-------------------------------------------------------------------------
 * Function:    H5X_dummy_get_size
 *
 * Purpose: This function gets the storage size of the index.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5X_dummy_get_size(void *idx_handle, hsize_t *idx_size)
{
    H5X_dummy_t *dummy = (H5X_dummy_t *) idx_handle;
    hsize_t size;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT
    H5X_DUMMY_LOG_DEBUG("Enter");

    if (NULL == dummy)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL index handle");
    if (!idx_size)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer");

    size = H5Dget_storage_size(dummy->idx_anon_id);

    *idx_size = size;

done:
    H5X_DUMMY_LOG_DEBUG("Leave");
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_dummy_get_size */
