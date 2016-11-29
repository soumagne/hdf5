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
 * Purpose:	Query routines.
 */

/****************/
/* Module Setup */
/****************/

#include "H5Qmodule.h"          /* This source code file is part of the H5Q module */

/***********/
/* Headers */
/***********/
#include "H5private.h"      /* Generic Functions */
#include "H5Qprivate.h"     /* Query */
#include "H5Eprivate.h"     /* Error handling */
#include "H5Iprivate.h"     /* IDs */
#include "H5MMprivate.h"    /* Memory management */
#include "H5Pprivate.h"
#include "H5FLprivate.h"    /* Free lists                           */
#include "H5Dprivate.h"     /* Datasets */

/* TODO for now */
#define H5G_PACKAGE
#include "H5Gpkg.h"

/****************/
/* Local Macros */
/****************/
#define H5Q_TYPE_SEQ \
    ((H5T_NATIVE_CHAR)(char))\
    ((H5T_NATIVE_SHORT)(short)) \
    ((H5T_NATIVE_INT)(int)) \
    ((H5T_NATIVE_LONG)(long)) \
    ((H5T_NATIVE_LLONG)(long long)) \
    ((H5T_NATIVE_FLOAT)(float)) \
    ((H5T_NATIVE_DOUBLE)(double))

#define H5Q_APPLY_MATCH_OP(result, op, x, y) \
    switch (op) { \
        case H5Q_MATCH_EQUAL: \
            result = (x == y); \
            break; \
        case H5Q_MATCH_NOT_EQUAL: \
            result = (x != y); \
            break; \
        case H5Q_MATCH_LESS_THAN: \
            result = (x < y); \
            break; \
        case H5Q_MATCH_GREATER_THAN: \
            result = (x > y); \
            break; \
        default: \
            break; \
    }

#define H5Q_CMP_DATA_ELEM(result, op, type, x, y) \
{ \
    type x_value; \
    type y_value; \
    x_value = *((type *) x); \
    y_value = *((type *) y); \
    \
    H5Q_APPLY_MATCH_OP(result, op, x_value, y_value) \
}

#define H5Q_CMP_TYPE(type1, type2, native_type) \
    (0 == H5T_cmp(type1, native_type, FALSE)) || \
    (0 == H5T_cmp(type2, native_type, FALSE))

/******************/
/* Local Typedefs */
/******************/
typedef enum H5Q_match_type_t { /* The different kinds of native types we can match */
    H5Q_NATIVE_INT_MATCH_CHAR,
    H5Q_NATIVE_INT_MATCH_SHORT,
    H5Q_NATIVE_INT_MATCH_INT,
    H5Q_NATIVE_INT_MATCH_LONG,
    H5Q_NATIVE_INT_MATCH_LLONG,
    H5Q_NATIVE_FLOAT_MATCH_FLOAT,
    H5Q_NATIVE_FLOAT_MATCH_DOUBLE,
    H5Q_INVALID_MATCH_TYPE
} H5Q_match_type_t;

typedef struct {
    H5Q_t *query;
    unsigned *result;
} H5Q_apply_arg_t;

/********************/
/* Local Prototypes */
/********************/
static herr_t H5Q_apply_data_elem(H5Q_t *query, hbool_t *result, H5T_t *type,
        const void *elem);
static herr_t H5Q_apply_attr_name(H5Q_t *query, hbool_t *result,
        const char *name);
static herr_t H5Q_apply_link_name(H5Q_t *query, hbool_t *result,
        const char *name);

static void
H5Q_encode_memcpy(unsigned char **buf_ptr, size_t *nalloc, const void *data,
        size_t data_size)
{
    if (*buf_ptr != NULL) {
        HDmemcpy(*buf_ptr, data, data_size);
        *buf_ptr += data_size;
    }
    *nalloc += data_size;
}

static void
H5Q_decode_memcpy(void *data, size_t data_size, const unsigned char **buf_ptr)
{
    if (*buf_ptr != NULL) {
        HDmemcpy(data, *buf_ptr, data_size);
        *buf_ptr += data_size;
    }
}

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

/* Declare a free list to manage the H5Q_t struct */
H5FL_DEFINE(H5Q_t);

/* Query ID class */
static const H5I_class_t H5I_QUERY_CLS[1] = {{
    H5I_QUERY,             /* Class ID for the type */
    0,                     /* Class behavior flags */
    0,                     /* Number of reserved IDs for this type */
    (H5I_free_t)H5Q_close  /* Free function for object's of this type */
}};

/*-------------------------------------------------------------------------
 * Function:    H5Q_init
 *
 * Purpose:	Initialize the interface from some other package.
 *
 * Return:  Success:    non-negative
 *          Failure:    negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_init(void)
{
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI(FAIL)
    /* FUNC_ENTER() does all the work */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_init() */

/*--------------------------------------------------------------------------
NAME
   H5Q__init_package -- Initialize interface-specific information
USAGE
    herr_t H5Q__init_package()

RETURNS
    Non-negative on success/Negative on failure
DESCRIPTION
    Initializes any interface-specific data or routines.

--------------------------------------------------------------------------*/
herr_t
H5Q__init_package(void)
{
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* Initialize the atom group for the QUERY IDs */
    if (H5I_register_type(H5I_QUERY_CLS) < 0)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTINIT, FAIL, "unable to initialize interface");

    H5_PKG_INIT_VAR = TRUE;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q__init_package() */

/*--------------------------------------------------------------------------
 NAME
    H5Q_term_package
 PURPOSE
    Terminate various H5Q objects
 USAGE
    void H5Q_term_package()
 RETURNS
    Non-negative on success/Negative on failure
 DESCRIPTION
    Release the atom group and any other resources allocated.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
     Can't report errors...
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int
H5Q_term_package(void)
{
    int n = 0;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if (H5_interface_initialize_g) {
        if ((n = (int) H5I_nmembers(H5I_QUERY))) {
            H5I_clear_type(H5I_QUERY, FALSE, FALSE);
        } /* end if */
        else {
            /* Free data types */
            H5I_dec_type_ref(H5I_QUERY);

            /* Shut down interface */
            H5_interface_initialize_g = 0;
            n = 1; /* H5I */
        } /* end else */
    } /* end if */

    FUNC_LEAVE_NOAPI(n)
} /* end H5Q_term_package() */

/*-------------------------------------------------------------------------
 * Function:    H5Qcreate
 *
 * Purpose: Create a new query object of query_type type, with match_op
 * determining the query's match condition and additional parameters
 * determined by the type of the query.
 *
 * Return:  Success:    The ID for a new query.
 *          Failure:    FAIL
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Qcreate(H5Q_type_t query_type, H5Q_match_op_t match_op, ...)
{
    H5Q_t *query = NULL;
    va_list ap;
    hid_t ret_value;

    FUNC_ENTER_API(FAIL)
    H5TRACE2("i", "QtQm", query_type, match_op);

    va_start(ap, match_op);

    switch (query_type) {
        case H5Q_TYPE_DATA_ELEM:
        case H5Q_TYPE_ATTR_VALUE:
        {
            H5T_t *datatype = NULL;
            hid_t datatype_id;
            const void *value;

            /* Get arguments */
            datatype_id = va_arg(ap, hid_t);
            value = va_arg(ap, const void *);

            /* Get type */
            if (NULL == (datatype = (H5T_t *) H5I_object_verify(datatype_id, H5I_DATATYPE)))
                HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data type");

            /* Create a new query object */
            if (NULL == (query = H5Q_create(query_type, match_op, datatype, value)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCREATE, FAIL, "unable to create query");
        }
        break;
        case H5Q_TYPE_ATTR_NAME:
        {
            const char *attr_name;

            /* Get arguments */
            attr_name = va_arg(ap, const char *);

            /* Create a new query object */
            if (NULL == (query = H5Q_create(query_type, match_op, attr_name)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCREATE, FAIL, "unable to create query");
        }
        break;
        case H5Q_TYPE_LINK_NAME:
        {
            const char *link_name;

            /* Get arguments */
            link_name = va_arg(ap, const char *);

            /* Create a new query object */
            if (NULL == (query = H5Q_create(query_type, match_op, link_name)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCREATE, FAIL, "unable to create query");
        }
        break;
        case H5Q_TYPE_MISC:
        default:
        {
            HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, FAIL, "unsupported/unrecognized query type");
        }
        break;
    }

    va_end(ap);

    /* Register the new query object to get an ID for it */
    if ((query->query_id = H5I_register(H5I_QUERY, query, TRUE)) < 0)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTREGISTER, FAIL, "can't register query handle");
    ret_value = query->query_id;

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qcreate() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_create
 *
 * Purpose: Private function for H5Qcreate
 *
 * Return:  Success:    Pointer to the new query
 *          Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
H5Q_t *
H5Q_create(H5Q_type_t query_type, H5Q_match_op_t match_op, ...)
{
    H5Q_t *query = NULL;
    va_list ap;
    H5Q_t *ret_value = NULL; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    va_start(ap, match_op);

    /* Allocate query struct */
    if (NULL == (query = H5FL_CALLOC(H5Q_t)))
        HGOTO_ERROR(H5E_QUERY, H5E_NOSPACE, NULL, "can't allocate query structure");

    query->is_combined = FALSE;
    query->query_id = H5I_UNINIT;
    query->ref_count = 1;
    query->query.select.type = query_type;
    query->query.select.match_op = match_op;
    switch (query_type) {
        case H5Q_TYPE_DATA_ELEM:
        case H5Q_TYPE_ATTR_VALUE:
            {
                H5T_t *datatype = NULL;
                H5T_t *native_datatype = NULL;
                size_t datatype_size;
                const void *value;
                void *value_buf = NULL;

                datatype = va_arg(ap, H5T_t*);
                value = va_arg(ap, const void *);

                /* Only use native type */
                if (NULL == (native_datatype = H5T_get_native_type(datatype, H5T_DIR_DEFAULT, NULL, NULL, NULL)))
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve native type");
                query->query.select.elem.data_elem.type = native_datatype;
                if (0 == (datatype_size = H5T_get_size(native_datatype)))
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid size");
                query->query.select.elem.data_elem.type_size = datatype_size;
                if (NULL == (value_buf = H5MM_malloc(datatype_size)))
                    HGOTO_ERROR(H5E_QUERY, H5E_CANTALLOC, NULL,
                            "can't allocate value buffer");
                HDmemcpy(value_buf, value, datatype_size);
                query->query.select.elem.data_elem.value = value_buf;
            }
            break;
        case H5Q_TYPE_ATTR_NAME:
            {
                const char *attr_name;

                attr_name = va_arg(ap, const char *);
                query->query.select.elem.attr_name.name = H5MM_strdup(attr_name);
            }
            break;
        case H5Q_TYPE_LINK_NAME:
            {
                const char *link_name;

                link_name = va_arg(ap, const char *);
                query->query.select.elem.link_name.name = H5MM_strdup(link_name);
            }
            break;
        case H5Q_TYPE_MISC:
        default:
            HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, NULL, "unsupported/unrecognized query type");
            break;
    } /* end switch */

    va_end(ap);

    /* set return value */
    ret_value = query;

done:
    if (!ret_value && query)
        query = H5FL_FREE(H5Q_t, query);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_create() */

/*-------------------------------------------------------------------------
 * Function:    H5Qclose
 *
 * Purpose: The H5Qclose routine terminates access to a query object,
 * given by query_id.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Qclose(hid_t query_id)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "i", query_id);

    /* Check args */
    if (NULL == H5I_object_verify(query_id, H5I_QUERY))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");

    if (H5I_dec_app_ref(query_id) < 0)
    	HGOTO_ERROR(H5E_QUERY, H5E_CANTDEC, FAIL, "unable to decrement ref count on query");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qclose() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_close
 *
 * Purpose: Private function for H5Qclose.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_close(H5Q_t *query)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert(query);

    if (--query->ref_count)
        HGOTO_DONE(SUCCEED)

    if (query->is_combined) {
        if (FAIL == H5Q_close(query->query.combine.l_query))
            HGOTO_ERROR(H5E_QUERY, H5E_CANTFREE, FAIL, "unable to free query");
            query->query.combine.l_query = NULL;
        if (FAIL == H5Q_close(query->query.combine.r_query))
            HGOTO_ERROR(H5E_QUERY, H5E_CANTFREE, FAIL, "unable to free query");
            query->query.combine.r_query = NULL;
    } else {
        switch (query->query.select.type) {
            case H5Q_TYPE_DATA_ELEM:
            case H5Q_TYPE_ATTR_VALUE:
                if (FAIL == H5T_close(query->query.select.elem.data_elem.type))
                    HGOTO_ERROR(H5E_QUERY, H5E_CANTFREE, FAIL, "unable to free datatype");
                query->query.select.elem.data_elem.type = NULL;
                H5MM_free(query->query.select.elem.data_elem.value);
                query->query.select.elem.data_elem.value = NULL;
                break;
            case H5Q_TYPE_ATTR_NAME:
                H5MM_free(query->query.select.elem.attr_name.name);
                query->query.select.elem.attr_name.name = NULL;
                break;
            case H5Q_TYPE_LINK_NAME:
                H5MM_free(query->query.select.elem.link_name.name);
                query->query.select.elem.link_name.name = NULL;
                break;
            case H5Q_TYPE_MISC:
            default:
                HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, FAIL, "unsupported/unrecognized query type");
                break;
        }
    }

    /* Free the query struct */
    query = H5FL_FREE(H5Q_t, query);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_close() */

/*-------------------------------------------------------------------------
 * Function:    H5Qcombine
 *
 * Purpose: The H5Qcombine routine creates a new compound query object by
 * combining two query objects (given by query1 and query2), using the
 * combination operator combine_op. Valid combination operators are:
 *      - H5Q_COMBINE_AND
 *      - H5Q_COMBINE_OR
 *
 * Return:  Success:    The ID for a new combined query.
 *          Failure:    FAIL
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Qcombine(hid_t query1_id, H5Q_combine_op_t combine_op, hid_t query2_id)
{
    H5Q_t *query = NULL, *query1 = NULL, *query2 = NULL;
    hid_t ret_value;

    FUNC_ENTER_API(FAIL)
    H5TRACE3("i", "iQci", query1_id, combine_op, query2_id);

    /* Check args and get the query objects */
    if (NULL == (query1 = (H5Q_t *) H5I_object_verify(query1_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");
    if (NULL == (query2 = (H5Q_t *) H5I_object_verify(query2_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");

    /* Combine query objects */
    if (NULL == (query = H5Q_combine(query1, combine_op, query2)))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTCREATE, FAIL, "unable to combine query objects");

    /* Register the new query object to get an ID for it */
    if ((query->query_id = H5I_register(H5I_QUERY, query, TRUE)) < 0)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTREGISTER, FAIL, "can't register query handle");
    ret_value = query->query_id;

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qcombine() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_combine
 *
 * Purpose: Private function for H5Qcombine.
 *
 * Return:  Success:    Pointer to the new query
 *          Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
H5Q_t *
H5Q_combine(H5Q_t *query1, H5Q_combine_op_t combine_op, H5Q_t *query2)
{
    H5Q_t *query = NULL;
    H5Q_type_t type1, type2;
    H5Q_t *ret_value = NULL; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* Allocate query struct */
    if (NULL == (query = H5FL_CALLOC(H5Q_t)))
        HGOTO_ERROR(H5E_QUERY, H5E_NOSPACE, NULL, "can't allocate query structure");

    switch (combine_op) {
        case H5Q_COMBINE_AND:
        case H5Q_COMBINE_OR:
            query->query.combine.op = combine_op;
            break;
        case H5Q_SINGLETON:
        default:
            HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, NULL, "unsupported/unrecognized combine op");
            break;
    }
    query->is_combined = TRUE;
    query->query_id = H5I_UNINIT;
    query->ref_count = 1;
    query->query.combine.l_query = query1;
    query1->ref_count++;
    query->query.combine.r_query = query2;
    query2->ref_count++;
    /* Work out query type of the combined query */
    type1 = (query1->is_combined) ? query1->query.combine.type : query1->query.select.type;
    type2 = (query2->is_combined) ? query2->query.combine.type : query2->query.select.type;
    query->query.combine.type = (type1 == type2) ? type1 : H5Q_TYPE_MISC;

    /* set return value */
    ret_value = query;

done:
    if (!ret_value && query)
        query = H5FL_FREE(H5Q_t, query);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_combine() */

/*-------------------------------------------------------------------------
 * Function:    H5Qget_type
 *
 * Purpose: The H5Qget_type routine queries a query object,
 * given by query_id, for its type information, originally provided to
 * H5Qcreate or created after a call to H5Qcombine. Information is returned
 * through the query_type parameter. See H5Qcreate for a table listing the
 * complete set of values that may be returned for query_type.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Qget_type(hid_t query_id, H5Q_type_t *query_type)
{
    H5Q_t *query = NULL;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*Qt", query_id, query_type);

    /* Check args and get the query objects */
    if (NULL == (query = (H5Q_t *) H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");
    if (!query_type)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer to query type");

    /* Get match info */
    if (FAIL == H5Q_get_type(query, query_type))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTGET, FAIL, "unable to get query type");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qget_type() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_get_type
 *
 * Purpose: Private function for H5Qget_type.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_get_type(H5Q_t *query, H5Q_type_t *query_type)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(query);
    HDassert(query_type);

    *query_type = (query->is_combined) ? query->query.combine.type :
            query->query.select.type;

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_get_type() */


/*-------------------------------------------------------------------------
 * Function:    H5Qget_match_op
 *
 * Purpose: The H5Qget_match_op routine queries a singleton query object,
 * given by query_id, for its op information, originally provided to
 * H5Qcreate. Match information is returned through the
 * match_op parameter. See H5Qcreate for a table listing the complete set of
 * values that may be returned for match_op.
 * It is an error to perform this call on a compound query object (one which
 * was created with H5Qcombine).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Qget_match_op(hid_t query_id, H5Q_match_op_t *match_op)
{
    H5Q_t *query = NULL;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*Qm", query_id, match_op);

    /* Check args and get the query objects */
    if (NULL == (query = (H5Q_t *) H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");
    if (!match_op)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer to match op");

    /* Get match info */
    if (FAIL == H5Q_get_match_op(query, match_op))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTGET, FAIL, "unable to get match op");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qget_match_op() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_get_match_op
 *
 * Purpose: Private function for H5Qget_match_op.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_get_match_op(H5Q_t *query, H5Q_match_op_t *match_op)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(query);
    HDassert(match_op);

    if (query->is_combined)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTGET, FAIL, "cannot retrieve op from combined query");

    *match_op = query->query.select.match_op;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_get_match_op() */

/*-------------------------------------------------------------------------
 * Function:    H5Qget_components
 *
 * Purpose: The H5Qget_components routine queries a compound query object,
 * given by query_id, for its component queries.  The component queries are
 * returned in sub_query1_id and sub_query2_id, both of which must be closed
 * with H5Qclose.
 * It is an error to apply H5Qget_components to a singleton query object (one
 * which was created with H5Qcreate).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Qget_components(hid_t query_id, hid_t *sub_query1_id, hid_t *sub_query2_id)
{
    H5Q_t *query = NULL, *sub_query1 = NULL, *sub_query2 = NULL;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "i*i*i", query_id, sub_query1_id, sub_query2_id);

    /* Check args and get the query objects */
    if (NULL == (query = (H5Q_t *) H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");
    if (!sub_query1_id || !sub_query2_id)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer to query_id");

    /* Get components */
    if (FAIL == H5Q_get_components(query, &sub_query1, &sub_query2))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTGET, FAIL, "unable to get components");

    /* Register the type and return the ID */
    if ((*sub_query1_id = H5I_register(H5I_QUERY, sub_query1, TRUE)) < 0)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTREGISTER, FAIL, "unable to register query");
    if ((*sub_query2_id = H5I_register(H5I_QUERY, sub_query2, TRUE)) < 0)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTREGISTER, FAIL, "unable to register query")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qget_components() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_get_components
 *
 * Purpose: Private function for H5Qget_components.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_get_components(H5Q_t *query, H5Q_t **sub_query1, H5Q_t **sub_query2)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(query);
    HDassert(sub_query1);
    HDassert(sub_query2);

    if (!query->is_combined)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTGET, FAIL, "not a combined query");

    *sub_query1 = query->query.combine.l_query;
    query->query.combine.l_query->ref_count++;
    *sub_query2 = query->query.combine.r_query;
    query->query.combine.r_query->ref_count++;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_get_components() */

/*-------------------------------------------------------------------------
 * Function:    H5Qget_combine_op
 *
 * Purpose: The H5Qget_combine_op routine queries a query object, given by
 * query_id, for its operator type. The possible operator types returned are:
 *      - H5Q_SINGLETON
 *      - H5Q_COMBINE_AND
 *      - H5Q_COMBINE_OR
 * H5Q_COMBINE_AND and H5Q_COMBINE_OR are only returned for query objects
 * produced with H5Qcombine and H5Q_SINGLETON is returned for query objects
 * produced with H5Qcreate.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Qget_combine_op(hid_t query_id, H5Q_combine_op_t *op_type)
{
    H5Q_t *query = NULL;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*Qc", query_id, op_type);

    /* Check args and get the query objects */
    if (NULL == (query = (H5Q_t *) H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");
    if (!op_type)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer to op type");

    /* Get combine op */
    if (FAIL == H5Q_get_combine_op(query, op_type))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTGET, FAIL, "unable to get combine op");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qget_combine_op() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_get_combine_op
 *
 * Purpose: Private function for H5Qget_combine_op.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_get_combine_op(H5Q_t *query, H5Q_combine_op_t *op_type)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(query);
    HDassert(op_type);

    if (!query->is_combined)
        *op_type = H5Q_SINGLETON;
    else
        *op_type = query->query.combine.op;

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_get_combine_op() */

/*-------------------------------------------------------------------------
 * Function:    H5Qencode
 *
 * Purpose: Given a query ID, serialize the query into buf.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Qencode(hid_t query_id, void *buf, size_t *nalloc)
{
    H5Q_t *query = NULL;
    herr_t ret_value;

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "i*x*z", query_id, buf, nalloc);

    /* Check argument and retrieve object */
    if (NULL == (query = (H5Q_t *)H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");
    if (nalloc == NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer for buffer size");

    /* Encode the query */
    if ((ret_value = H5Q_encode(query, (unsigned char *)buf, nalloc)) < 0)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTENCODE, FAIL, "can't encode query");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qencode() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_encode
 *
 * Purpose: Private function for H5Qencode.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_encode(H5Q_t *query, unsigned char *buf, size_t *nalloc)
{
    size_t buf_size = 0;
    herr_t ret_value = SUCCEED;
    unsigned char *buf_ptr = buf;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(query);
    HDassert(nalloc);

    H5Q_encode_memcpy(&buf_ptr, &buf_size, &query->is_combined, sizeof(hbool_t));
    if (query->is_combined) {
        size_t l_buf_size = 0, r_buf_size = 0;

        H5Q_encode_memcpy(&buf_ptr, &buf_size, &query->query.combine.type, sizeof(H5Q_type_t));
        H5Q_encode_memcpy(&buf_ptr, &buf_size, &query->query.combine.op, sizeof(H5Q_combine_op_t));
        H5Q_encode(query->query.combine.l_query, buf_ptr, &l_buf_size);
        buf_size += l_buf_size;
        if (buf_ptr) buf_ptr += l_buf_size;
        H5Q_encode(query->query.combine.r_query, buf_ptr, &r_buf_size);
        buf_size += r_buf_size;
        if (buf_ptr) buf_ptr += r_buf_size;
    } else {
        H5Q_encode_memcpy(&buf_ptr, &buf_size, &query->query.select.type, sizeof(H5Q_type_t));
        H5Q_encode_memcpy(&buf_ptr, &buf_size, &query->query.select.match_op, sizeof(H5Q_match_op_t));
        switch (query->query.select.type) {
            case H5Q_TYPE_DATA_ELEM:
            case H5Q_TYPE_ATTR_VALUE:
            {
                size_t type_id_nalloc = 0;
                H5T_t *type = query->query.select.elem.data_elem.type;
                size_t type_size = query->query.select.elem.data_elem.type_size;
                void *value_buf = query->query.select.elem.data_elem.value;

                if (FAIL == H5T_encode(type, NULL, &type_id_nalloc))
                    HGOTO_ERROR(H5E_QUERY, H5E_CANTENCODE, FAIL, "can't get encoding size for datatype");
                H5Q_encode_memcpy(&buf_ptr, &buf_size, &type_id_nalloc, sizeof(size_t));
                if (FAIL == H5T_encode(type, buf_ptr, &type_id_nalloc))
                    HGOTO_ERROR(H5E_QUERY, H5E_CANTENCODE, FAIL, "can't encode datatype");
                buf_size += type_id_nalloc;
                if (buf_ptr) buf_ptr += type_id_nalloc;
                H5Q_encode_memcpy(&buf_ptr, &buf_size, &type_size, sizeof(size_t));
                H5Q_encode_memcpy(&buf_ptr, &buf_size, value_buf, type_size);
            }
                break;
            case H5Q_TYPE_ATTR_NAME:
            {
                size_t name_len = HDstrlen(query->query.select.elem.attr_name.name) + 1;
                char *name = query->query.select.elem.attr_name.name;

                H5Q_encode_memcpy(&buf_ptr, &buf_size, &name_len, sizeof(size_t));
                H5Q_encode_memcpy(&buf_ptr, &buf_size, name, name_len);
            }
                break;
            case H5Q_TYPE_LINK_NAME:
            {
                size_t name_len = HDstrlen(query->query.select.elem.link_name.name) + 1;
                char *name = query->query.select.elem.attr_name.name;

                H5Q_encode_memcpy(&buf_ptr, &buf_size, &name_len, sizeof(size_t));
                H5Q_encode_memcpy(&buf_ptr, &buf_size, name, name_len);
            }
                break;
            case H5Q_TYPE_MISC:
            default:
                HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, FAIL, "unsupported/unrecognized query type");
                break;
        }
    }

    *nalloc = buf_size;

done:

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_encode() */

/*-------------------------------------------------------------------------
 * Function:    H5Qdecode
 *
 * Purpose: Deserialize the buffer and return a new query handle. The handle
 * must be closed using H5Qclose.
 *
 * Return:  Success:    query ID (non-negative)
 *
 *          Failure:    negative
 *-------------------------------------------------------------------------
 */
hid_t
H5Qdecode(const void *buf)
{
    H5Q_t *query;
    hid_t ret_value;      /* Return value */
    const unsigned char *buf_ptr = (const unsigned char *) buf;

    FUNC_ENTER_API(FAIL)
    H5TRACE1("i", "*x", buf);

    /* Check args */
    if (buf == NULL) HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "empty buffer");

    /* Create datatype by decoding buffer */
    if (NULL == (query = H5Q_decode(&buf_ptr)))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTDECODE, FAIL, "can't decode object");

    /* Register the type and return the ID */
    if ((query->query_id = H5I_register(H5I_QUERY, query, TRUE)) < 0)
        HGOTO_ERROR(H5E_QUERY, H5E_CANTREGISTER, FAIL, "unable to register query");
    ret_value = query->query_id;

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qdecode() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_decode
 *
 * Purpose: Private function for H5Qdecode.
 *
 * Return:  Success:    Pointer to the decoded query
 *          Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
H5Q_t *
H5Q_decode(const unsigned char **buf_ptr)
{
    H5Q_t *ret_value = NULL;
    H5Q_t *query = NULL;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(*buf_ptr);

    /* Allocate query struct */
    if (NULL == (query = H5FL_CALLOC(H5Q_t)))
        HGOTO_ERROR(H5E_QUERY, H5E_NOSPACE, NULL, "can't allocate query structure");

    /* Set ref count */
    query->query_id = H5I_UNINIT;
    query->ref_count = 1;

    H5Q_decode_memcpy(&query->is_combined, sizeof(hbool_t), buf_ptr);
    if (query->is_combined) {
        H5Q_decode_memcpy(&query->query.combine.type, sizeof(H5Q_type_t), buf_ptr);
        H5Q_decode_memcpy(&query->query.combine.op, sizeof(H5Q_combine_op_t), buf_ptr);
        query->query.combine.l_query = H5Q_decode(buf_ptr);
        query->query.combine.r_query = H5Q_decode(buf_ptr);
    } else {
        H5Q_decode_memcpy(&query->query.select.type, sizeof(H5Q_type_t), buf_ptr);
        H5Q_decode_memcpy(&query->query.select.match_op, sizeof(H5Q_match_op_t), buf_ptr);
        switch (query->query.select.type) {
            case H5Q_TYPE_DATA_ELEM:
            case H5Q_TYPE_ATTR_VALUE:
            {
                size_t type_id_nalloc = 0;
                H5T_t *type;
                size_t type_size = 0;
                void *value_buf;

                H5Q_decode_memcpy(&type_id_nalloc, sizeof(size_t), buf_ptr);
                if (NULL == (type = H5T_decode(*buf_ptr)))
                    HGOTO_ERROR(H5E_DATATYPE, H5E_CANTDECODE, NULL, "can't decode datatype");
                query->query.select.elem.data_elem.type = type;
                *buf_ptr += type_id_nalloc;

                H5Q_decode_memcpy(&type_size, sizeof(size_t), buf_ptr);
                if (!type_size)
                    HGOTO_ERROR(H5E_QUERY, H5E_BADVALUE, NULL, "NULL type size");
                query->query.select.elem.data_elem.type_size = type_size;
                if (NULL == (value_buf = H5MM_malloc(type_size)))
                    HGOTO_ERROR(H5E_QUERY, H5E_CANTALLOC, NULL, "can't allocate value buffer");
                H5Q_decode_memcpy(value_buf, type_size, buf_ptr);
                query->query.select.elem.data_elem.value = value_buf;
            }
            break;
            case H5Q_TYPE_ATTR_NAME:
            {
                size_t name_len = 0;
                char *name;
                
                H5Q_decode_memcpy(&name_len, sizeof(size_t), buf_ptr);
                if (!name_len)
                    HGOTO_ERROR(H5E_QUERY, H5E_BADVALUE, NULL, "NULL name len");
                if (NULL == (name = (char *)H5MM_malloc(name_len)))
                    HGOTO_ERROR(H5E_QUERY, H5E_CANTALLOC, NULL,
                            "can't allocate value buffer");
                H5Q_decode_memcpy(name, name_len, buf_ptr);
                query->query.select.elem.attr_name.name = name;
            }
            break;
            case H5Q_TYPE_LINK_NAME:
            {
                size_t name_len = 0;
                char *name;

                H5Q_decode_memcpy(&name_len, sizeof(size_t), buf_ptr);
                if (!name_len)
                    HGOTO_ERROR(H5E_QUERY, H5E_BADVALUE, NULL, "NULL name len");
                if (NULL == (name = (char *)H5MM_malloc(name_len)))
                    HGOTO_ERROR(H5E_QUERY, H5E_CANTALLOC, NULL,
                            "can't allocate value buffer");
                H5Q_decode_memcpy(name, name_len, buf_ptr);
                query->query.select.elem.link_name.name = name;
            }
            break;
            case H5Q_TYPE_MISC:
            default:
                HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, NULL, "unsupported/unrecognized query type");
                break;
        }
    }

    ret_value = query;
done:

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_decode() */

/*-------------------------------------------------------------------------
 * Function:    H5Qapply_singleton
 *
 * Purpose: Apply a query and return the result. Parameters, which the
 * query applies to, are determined by the type of the query.
 * It is an error to apply H5Qapply_singleton to a combined query object (one
 * which was created with H5Qcombine).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Qapply_singleton(hid_t query_id, hbool_t *result, ...)
{
    H5Q_t *query = NULL;
    H5T_t *native_type = NULL;
    herr_t ret_value;
    va_list ap;

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*b", query_id, result);

    /* Check args and get the query objects */
    if (!result)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer for result");
    if (NULL == (query = (H5Q_t *) H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");

    if (query->is_combined)
        HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, FAIL, "cannot apply to combined query");

    va_start(ap, result);

    switch (query->query.select.type) {
        case H5Q_TYPE_DATA_ELEM:
        case H5Q_TYPE_ATTR_VALUE:
        {
            H5T_t *type = NULL;
            hid_t type_id;
            const void *value;

            /* Get arguments */
            type_id = va_arg(ap, hid_t);
            value = va_arg(ap, const void *);

            /* Get type */
            if (NULL == (type = (H5T_t *) H5I_object_verify(type_id, H5I_DATATYPE)))
                HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data type");

            /* Only use native type */
            if (NULL == (native_type = H5T_get_native_type(type, H5T_DIR_DEFAULT, NULL, NULL, NULL)))
                HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "cannot retrieve native type");

            /* Apply query */
            if (FAIL == (ret_value = H5Q_apply_singleton(query, result, native_type, value)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query");
        }
        break;
        case H5Q_TYPE_ATTR_NAME:
        {
            const char *attr_name;

            /* Get arguments */
            attr_name = va_arg(ap, const char *);

            /* Apply query */
            if (FAIL == (ret_value = H5Q_apply_singleton(query, result, attr_name)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query");
        }
        break;
        case H5Q_TYPE_LINK_NAME:
        {
            const char *link_name;

            /* Get arguments */
            link_name = va_arg(ap, const char *);

            /* Apply query */
            if (FAIL == (ret_value = H5Q_apply_singleton(query, result, link_name)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query");
        }
        break;
        case H5Q_TYPE_MISC:
        default:
        {
            HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, FAIL, "unsupported/unrecognized query type");
        }
        break;
    }

    va_end(ap);

done:
    if (native_type)
        H5T_close(native_type);
    FUNC_LEAVE_API(ret_value)
} /* end H5Qapply() */

/*-------------------------------------------------------------------------
 * Function:    H5Qapply_combine
 *
 * Purpose: Apply a query and return the result. Parameters, which the
 * query applies to, are determined by the type of the query.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Qapply_combine(hid_t query_id, hbool_t *result, hid_t type_id, const void *value)
{
    H5Q_t *query = NULL;
    H5T_t *type = NULL, *native_type = NULL;
    herr_t ret_value;

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "i*bi*x", query_id, result, type_id, value);

    /* Check args and get the query objects */
    if (!result)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer for result");
    if (NULL == (query = (H5Q_t *) H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");

    /* Get type */
    if (NULL == (type = (H5T_t *) H5I_object_verify(type_id, H5I_DATATYPE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data type");

    /* Only use native type */
    if (NULL == (native_type = H5T_get_native_type(type, H5T_DIR_DEFAULT, NULL, NULL, NULL)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "cannot retrieve native type");

    /* Apply query */
    if (FAIL == (ret_value = H5Q_apply_combine(query, result, native_type, value)))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query");

done:
    if (native_type)
        H5T_close(native_type);
    FUNC_LEAVE_API(ret_value)
} /* end H5Qapply_combine() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_apply_combine
 *
 * Purpose: Private function for H5Qapply_combine.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_apply_combine(H5Q_t *query, hbool_t *result, H5T_t *type, const void *elem)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(query);
    HDassert(result);

    if (query->is_combined) {
        hbool_t result1, result2;

        if (FAIL == H5Q_apply_combine(query->query.combine.l_query, &result1, type, elem))
            HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query")
        if (FAIL == H5Q_apply_combine(query->query.combine.r_query, &result2, type, elem))
            HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query")

         switch (query->query.combine.op) {
             case H5Q_COMBINE_AND:
                 *result = result1 && result2;
                 break;
             case H5Q_COMBINE_OR:
                 *result = result1 || result2;
                 break;
             case H5Q_SINGLETON:
             default:
                 HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, FAIL, "unsupported/unrecognized combine op")
                 break;
         }
    } else {
        if (FAIL == (ret_value = H5Q_apply_singleton(query, result, type, elem)))
            HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query")
    }

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_apply_combine() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_apply_singleton
 *
 * Purpose: Private function for H5Qapply_singleton.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Q_apply_singleton(H5Q_t *query, hbool_t *result, ...)
{
    herr_t ret_value = SUCCEED; /* Return value */
    va_list ap;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(query);
    HDassert(result);
    HDassert(query->is_combined == FALSE);

    va_start(ap, result);

    switch (query->query.select.type) {
        case H5Q_TYPE_DATA_ELEM:
        case H5Q_TYPE_ATTR_VALUE:
        {
            H5T_t *type = va_arg(ap, H5T_t*);
            const void *elem = va_arg(ap, const void *);

            if (FAIL == (ret_value = H5Q_apply_data_elem(query, result, type, elem)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply data element query");
            break;
        }
        case H5Q_TYPE_ATTR_NAME:
        {
            const char *attr_name = va_arg(ap, const char *);

            if (FAIL == (ret_value = H5Q_apply_attr_name(query, result, attr_name)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply attribute name query");
            break;
        }
        case H5Q_TYPE_LINK_NAME:
        {
            const char *link_name = va_arg(ap, const char *);

            if (FAIL == (ret_value = H5Q_apply_link_name(query, result, link_name)))
                HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply link name query");
            break;
        }
        case H5Q_TYPE_MISC:
        default:
            HGOTO_ERROR(H5E_QUERY, H5E_BADTYPE, FAIL, "unsupported/unrecognized query type");
            break;
    }

    va_end(ap);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_apply() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_promote_type
 *
 * Purpose: Private function for H5Qapply (promote data element type
 *          so that data element and query element can be compared).
 *
 * Return:  Success:    Pointer to promoted native type
 *          Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
static H5T_t *
H5Q_promote_type(H5T_t *type1, H5T_t *type2, H5Q_match_type_t *match_type)
{
    H5T_t *ret_value; /* Return value */
    H5T_class_t type1_class, type2_class, promoted_type_class;
    H5T_t *promoted_type;

    FUNC_ENTER_NOAPI_NOINIT

    /* Get class of types */
    if (H5T_NO_CLASS == (type1_class = H5T_get_class(type1, FALSE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid class");
    if (H5T_NO_CLASS == (type2_class = H5T_get_class(type2, FALSE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid class");

     if ((type1_class == H5T_FLOAT) || (type2_class == H5T_FLOAT)) {
         promoted_type_class = H5T_FLOAT;
     } else {
         promoted_type_class = H5T_INTEGER;
     }

    switch (promoted_type_class) {
         case H5T_INTEGER:
             if (H5Q_CMP_TYPE(type1, type2, (H5T_t *)H5I_object(H5T_NATIVE_LLONG_g))) {
                 *match_type = H5Q_NATIVE_INT_MATCH_LLONG;
                 promoted_type = (H5T_t *)H5I_object(H5T_NATIVE_LLONG_g);
             } else if (H5Q_CMP_TYPE(type1, type2, (H5T_t *)H5I_object(H5T_NATIVE_LONG_g))) {
                 *match_type = H5Q_NATIVE_INT_MATCH_LONG;
                 promoted_type = (H5T_t *)H5I_object(H5T_NATIVE_LONG_g);
             } else if (H5Q_CMP_TYPE(type1, type2, (H5T_t *)H5I_object(H5T_NATIVE_INT_g))) {
                 *match_type = H5Q_NATIVE_INT_MATCH_INT;
                 promoted_type = (H5T_t *)H5I_object(H5T_NATIVE_INT_g);
             } else if (H5Q_CMP_TYPE(type1, type2, (H5T_t *)H5I_object(H5T_NATIVE_SHORT_g))) {
                 *match_type = H5Q_NATIVE_INT_MATCH_SHORT;
                 promoted_type = (H5T_t *)H5I_object(H5T_NATIVE_SHORT_g);
             } else if (H5Q_CMP_TYPE(type1, type2, (H5T_t *)H5I_object(H5T_NATIVE_SCHAR_g))) {
                 *match_type = H5Q_NATIVE_INT_MATCH_CHAR;
                 promoted_type = (H5T_t *)H5I_object(H5T_NATIVE_SCHAR_g);
             } else {
                 HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid type");
             }
             break;
         case H5T_FLOAT:
             if (H5Q_CMP_TYPE(type1, type2, (H5T_t *)H5I_object(H5T_NATIVE_DOUBLE_g))) {
                 *match_type = H5Q_NATIVE_FLOAT_MATCH_DOUBLE;
                 promoted_type = (H5T_t *)H5I_object(H5T_NATIVE_DOUBLE_g);
             } else if (H5Q_CMP_TYPE(type1, type2, (H5T_t *)H5I_object(H5T_NATIVE_FLOAT_g))) {
                 *match_type = H5Q_NATIVE_FLOAT_MATCH_FLOAT;
                 promoted_type = (H5T_t *)H5I_object(H5T_NATIVE_FLOAT_g);
             } else {
                 HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid type");
             }
             break;
         default:
             HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid class");
             break;
     }

    ret_value = promoted_type;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_promote_type() */


/*-------------------------------------------------------------------------
 * Function:    H5Q_apply_data_elem
 *
 * Purpose: Private function for H5Qapply_singleton (data element).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5Q_apply_data_elem(H5Q_t *query, hbool_t *result, H5T_t *type, const void *value)
{
    herr_t ret_value = SUCCEED; /* Return value */
    void *value_buf = NULL, *query_value_buf = NULL;
    H5T_t *query_type, *promoted_type;
    H5Q_match_type_t match_type = H5Q_INVALID_MATCH_TYPE;
    size_t type_size, query_type_size, promoted_type_size;
    hid_t type_id = FAIL, query_type_id = FAIL, promoted_type_id = FAIL;
    H5T_path_t *tpath;
    H5Q_match_op_t query_op;
    hbool_t query_result = FALSE;

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(query);
    HDassert((query->query.select.type == H5Q_TYPE_DATA_ELEM) ||
            (query->query.select.type == H5Q_TYPE_ATTR_VALUE));

    /* Keep a copy of elem to work on */
    if (0 == (type_size = H5T_get_size(type)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a valid size");

    /* Keep a copy of the query value */
    query_type = query->query.select.elem.data_elem.type;
    query_type_size = query->query.select.elem.data_elem.type_size;
    query_op = query->query.select.match_op;

    /* Promote type to compare elements with query */
    promoted_type = H5Q_promote_type(type, query_type, &match_type);
    if (0 == (promoted_type_size = H5T_get_size(promoted_type)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a valid size");

    /* Resize value and query value buf for convert
     * (promoted_type_size is always bigger) */
    if (NULL == (value_buf = H5MM_malloc(promoted_type_size)))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTALLOC, FAIL, "can't allocate value buffer");
    HDmemcpy(value_buf, value, type_size);

    if (NULL == (query_value_buf = H5MM_malloc(promoted_type_size)))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTALLOC, FAIL, "can't allocate value buffer");
    HDmemcpy(query_value_buf, query->query.select.elem.data_elem.value, query_type_size);

    /* Create temporary IDs for H5T_convert */
    if ((type_id = H5I_register(H5I_DATATYPE, type, FALSE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register datatype");
    if ((query_type_id = H5I_register(H5I_DATATYPE, query_type, FALSE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register datatype");
    if ((promoted_type_id = H5I_register(H5I_DATATYPE, promoted_type, FALSE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register datatype");

    /* Find the conversion function */
    if (NULL == (tpath = H5T_path_find(type, promoted_type, NULL, NULL, H5P_LST_DATASET_XFER_ID_g, FALSE)))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTINIT, FAIL, "unable to find type info");
    if (FAIL == (H5T_convert(tpath, type_id, promoted_type_id, 1, (size_t)0, (size_t)0, value_buf, NULL, H5P_LST_DATASET_XFER_ID_g)))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTCONVERT, FAIL, "can't convert value");

    if (NULL == (tpath = H5T_path_find(query_type, promoted_type, NULL, NULL, H5P_LST_DATASET_XFER_ID_g, FALSE)))
          HGOTO_ERROR(H5E_QUERY, H5E_CANTINIT, FAIL, "unable to find type info");
    if (FAIL == (H5T_convert(tpath, query_type_id, promoted_type_id, 1, (size_t)0, (size_t)0, query_value_buf, NULL, H5P_LST_DATASET_XFER_ID_g)))
         HGOTO_ERROR(H5E_QUERY, H5E_CANTCONVERT, FAIL, "can't convert query value");

    /* Could also use BOOST preprocessor for that but not really nice */
    switch (match_type) {
        case H5Q_NATIVE_INT_MATCH_CHAR:
            H5Q_CMP_DATA_ELEM(query_result, query_op, char, value_buf, query_value_buf);
            break;
        case H5Q_NATIVE_INT_MATCH_SHORT:
            H5Q_CMP_DATA_ELEM(query_result, query_op, short, value_buf, query_value_buf);
            break;
        case H5Q_NATIVE_INT_MATCH_INT:
            H5Q_CMP_DATA_ELEM(query_result, query_op, int, value_buf, query_value_buf);
            break;
        case H5Q_NATIVE_INT_MATCH_LONG:
            H5Q_CMP_DATA_ELEM(query_result, query_op, long, value_buf, query_value_buf);
            break;
        case H5Q_NATIVE_INT_MATCH_LLONG:
            H5Q_CMP_DATA_ELEM(query_result, query_op, long long, value_buf, query_value_buf);
            break;
        case H5Q_NATIVE_FLOAT_MATCH_FLOAT:
            H5Q_CMP_DATA_ELEM(query_result, query_op, float, value_buf, query_value_buf);
            break;
        case H5Q_NATIVE_FLOAT_MATCH_DOUBLE:
            H5Q_CMP_DATA_ELEM(query_result, query_op, double, value_buf, query_value_buf);
            break;
        case H5Q_INVALID_MATCH_TYPE:
        default:
            HGOTO_ERROR(H5E_DATATYPE, H5E_BADTYPE, FAIL, "unsupported/unrecognized datatype");
            break;
    }

    *result = query_result;

done:
    H5MM_free(value_buf);
    H5MM_free(query_value_buf);

    /* Free temporary IDs */
    if ((promoted_type_id != FAIL) && !H5I_remove(promoted_type_id))
        HGOTO_ERROR(H5E_ATOM, H5E_CANTFREE, FAIL, "problem freeing id");
    if ((query_type_id != FAIL) && !H5I_remove(query_type_id))
        HGOTO_ERROR(H5E_ATOM, H5E_CANTFREE, FAIL, "problem freeing id");
    if ((type_id != FAIL) && !H5I_remove(type_id))
        HGOTO_ERROR(H5E_ATOM, H5E_CANTFREE, FAIL, "problem freeing id");

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_apply_data_elem() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_apply_attr_name
 *
 * Purpose: Private function for H5Qapply_singleton (attribute name).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5Q_apply_attr_name(H5Q_t *query, hbool_t *result, const char *name)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(query);
    HDassert(query->query.select.type == H5Q_TYPE_ATTR_NAME);

    if(query->query.select.match_op == H5Q_MATCH_EQUAL) {
        if(name)
            *result = (0 == HDstrcmp(name, query->query.select.elem.attr_name.name));
        else
            *result = FALSE;
    }
    else {
        if(name)
            *result = (0 != HDstrcmp(name, query->query.select.elem.attr_name.name));
        else
            *result = TRUE;
    }

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_apply_attr_name() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_apply_link_name
 *
 * Purpose: Private function for H5Qapply_singleton (link name).
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5Q_apply_link_name(H5Q_t *query, hbool_t *result, const char *name)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(query);
    HDassert(query->query.select.type == H5Q_TYPE_LINK_NAME);

    if(query->query.select.match_op == H5Q_MATCH_EQUAL) {
        if(name)
            *result = (0 == HDstrcmp(name, query->query.select.elem.link_name.name));
        else
            *result = FALSE;
    }
    else {
        if(name)
            *result = (0 != HDstrcmp(name, query->query.select.elem.link_name.name));
        else
            *result = TRUE;
    }

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_apply_link_name() */

/*-------------------------------------------------------------------------
 * Function:    H5Qapply
 *
 * Purpose: Apply a query and return the result. Parameters, which the
 * query applies to, are determined by the type of the query.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Qapply(hid_t loc_id, hid_t query_id, unsigned *result, hid_t vcpl_id)
{
    H5Q_t *query = NULL;
    H5G_loc_t loc;
    H5G_t *ret_grp;
    hid_t ret_value;

    FUNC_ENTER_API(FAIL)

    /* Check args and get the query objects */
    if(H5G_loc(loc_id, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    if (NULL == (query = (H5Q_t *) H5I_object_verify(query_id, H5I_QUERY)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a query ID");
    if (!result)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "NULL pointer for result");

    /* Get the default view creation property list if the user didn't provide one */
    /* TODO fix that */
    if (H5P_DEFAULT == vcpl_id)
        vcpl_id = H5P_INDEX_ACCESS_DEFAULT;
    else
        if (TRUE != H5P_isa_class(vcpl_id, H5P_INDEX_ACCESS))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not index access parms");

    /* Apply query */
    if (NULL == (ret_grp = H5Q_apply(loc.oloc, loc_id, query, result, vcpl_id)))
        HGOTO_ERROR(H5E_QUERY, H5E_CANTCOMPARE, FAIL, "unable to apply query");

    if ((ret_value = H5I_register(H5I_GROUP, ret_grp, TRUE)) < 0)
        HGOTO_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL, "unable to register group");
    printf("\nID: %d\n", ret_value);

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Qapply() */

/*-------------------------------------------------------------------------
 * Function:    H5Q_apply_iterate
 *
 * Purpose: Private function for H5Qapply.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5Q_apply_iterate(hid_t oid, const char *name, const H5O_info_t *oinfo,
        void *udata)
{
    H5G_loc_t loc;
    H5Q_apply_arg_t *args = (H5Q_apply_arg_t *) udata;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    if(H5G_loc(oid, &loc) < 0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");

    if (oinfo->type == H5O_TYPE_GROUP) {
        H5G_t *grp;

        printf("\ngroup: %s\n", name);
        printf("num attributes: %d\n", oinfo->num_attrs);
//        if(NULL == (grp = (H5G_t *) H5I_object_verify(oid, H5I_GROUP)))
//            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a group")
    } else if (oinfo->type == H5O_TYPE_DATASET) {
        H5D_t *dataset;
        H5S_t *dataspace;
        hid_t dset_id;

        printf("\ndataset: %s\n", name);
        printf("num attributes: %d\n", oinfo->num_attrs);
        dset_id = H5O_open_name(&loc, name, H5P_LINK_ACCESS_DEFAULT, FALSE);
        if (NULL == (dataset = (H5D_t *) H5I_object_verify(dset_id, H5I_DATASET)))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset");

        if (NULL == (dataspace = H5D_query(dataset, NULL, args->query, H5P_INDEX_ACCESS_DEFAULT, H5P_INDEX_XFER_DEFAULT)))
            HGOTO_ERROR(H5E_DATASET, H5E_CANTSELECT, FAIL, "can't query dataset");
        H5D_close(dataset);
        H5S_close(dataspace);
        H5I_remove(dset_id);
    }
    else
        HGOTO_ERROR(H5E_SYM, H5E_BADITER, FAIL, "unsupported object type");

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_apply_iterate */

/*-------------------------------------------------------------------------
 * Function:    H5Q_apply
 *
 * Purpose: Private function for H5Qapply.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
H5G_t *
H5Q_apply(H5O_loc_t *oloc, hid_t loc_id, H5Q_t *query, unsigned *result, hid_t vcpl_id)
{
    H5G_obj_create_t gcrt_info; /* Information for group creation */
    H5Q_apply_arg_t args = { query, result };
    H5G_t *ret_grp = NULL; /* New group created */
    H5G_t *ret_value = NULL; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(oloc);
    HDassert(query);
    HDassert(result);

    /* TODO create H5G_create_anon routine */
    /* Set up group creation info */
    gcrt_info.gcpl_id = H5P_GROUP_CREATE_DEFAULT;
    gcrt_info.cache_type = H5G_NOTHING_CACHED;
    HDmemset(&gcrt_info.cache, 0, sizeof(gcrt_info.cache));

    /* Create the new group & get its ID */
    if (NULL == (ret_grp = H5G__create(oloc->file, &gcrt_info, H5AC_dxpl_id)))
        HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, NULL, "unable to create group")

    if ((H5O_visit(loc_id, ".", H5_INDEX_NAME, H5_ITER_NATIVE, H5Q_apply_iterate, &args, H5P_LINK_ACCESS_DEFAULT, H5AC_ind_dxpl_id)) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_BADITER, NULL, "object visitation failed");

    ret_value = ret_grp;
done:
    /* Release the group's object header, if it was created */
    if (ret_grp) {
        H5O_loc_t *grp_oloc;         /* Object location for group */

        /* Get the new group's object location */
        if(NULL == (grp_oloc = H5G_oloc(ret_grp)))
            HDONE_ERROR(H5E_SYM, H5E_CANTGET, NULL, "unable to get object location of group");

        /* Decrement refcount on group's object header in memory */
        if(H5O_dec_rc_by_loc(grp_oloc, H5AC_dxpl_id) < 0)
            HDONE_ERROR(H5E_SYM, H5E_CANTDEC, NULL, "unable to decrement refcount on newly created object");
    } /* end if */

    /* Cleanup on failure */
    if (NULL == ret_value)
        if (ret_grp && H5G_close(ret_grp) < 0)
            HDONE_ERROR(H5E_SYM, H5E_CLOSEERROR, NULL, "unable to release group")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5Q_apply() */
