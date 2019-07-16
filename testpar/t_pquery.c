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

/* Programmer:  Jerome Soumagne <jsoumagne@hdfgroup.org>
 *              Wednesday, Sep 24, 2014
 */

/***********/
/* Headers */
/***********/
#include "h5test.h"

#if defined(H5_HAVE_MDHIM) || defined(H5_HAVE_PARALLEL)
#include <mpi.h>
#endif

extern void H5Q_enable_visualize_query(void);


/****************/
/* Local Macros */
/****************/
#define NTYPES 4
#define NTUPLES 1024*4
#define NCOMPONENTS 1

#define MAX_NAME 64
#define MULTI_NFILES 3
#define NLOOP 100

/* File_Access_type bits */
#define FACC_DEFAULT    0x0     /* default */
#define FACC_MPIO       0x1     /* MPIO */
#define FACC_SPLIT      0x2     /* Split File */

#define DXFER_COLLECTIVE_IO 0x1  /* Collective IO*/
#define DXFER_INDEPENDENT_IO 0x2 /* Independent IO collectively */

/******************/
/* Local Typedefs */
/******************/
struct test_case {
    const char *filename;
    unsigned meta_idx_plugin;
    unsigned data_idx_plugin;
};

/********************/
/* Local Prototypes */
/********************/
static hid_t test_query_create(void);
static hid_t test_query_create_type(H5R_type_t type, hbool_t compound);
static herr_t test_query_close(hid_t query);
static herr_t test_query_apply_elem(hid_t query, hbool_t *result, hid_t type_id,
    const void *value);
static herr_t test_query_apply(hid_t query);
static herr_t test_query_encode(hid_t query);

static char *gen_file_name(unsigned meta_idx_plugin, unsigned data_idx_plugin);
static herr_t test_query_create_simple_file(const char *filename, hid_t fapl,
    unsigned n_objs, unsigned meta_idx_plugin, unsigned data_idx_plugin);
static herr_t test_query_read_selection(size_t file_count,
    const char *filenames[], hid_t *files, hid_t view, H5R_type_t rtype);
static herr_t test_query_apply_view(const char *filename, hid_t fapl,
    unsigned n_objs, unsigned meta_idx_plugin, unsigned data_idx_plugin);
static herr_t test_query_apply_view_multi(const char *filenames[], hid_t fapl,
    unsigned n_objs, unsigned meta_idx_plugin, unsigned data_idx_plugin);

/*******************/
/* Local Variables */
/*******************/
/* Must match H5Xpublic.h */
static const char *plugin_names[] = { "none", "dummy" ,"fastbit", "alacrity",
    "dummy", "db", "mdhim" };


#ifdef H5_HAVE_PARALLEL
int mpi_size;
int mpi_rank;

MPI_Comm default_comm = MPI_COMM_WORLD;
MPI_Info default_info = MPI_INFO_NULL;
#endif

/*
 * Create the appropriate File access property list
 */
hid_t
create_faccess_plist(MPI_Comm comm, MPI_Info info, int l_facc_type)
{
    hid_t ret_pl = -1;
    herr_t ret;                 /* generic return value */

    /* need the rank for error checking macros */
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    ret_pl = H5Pcreate (H5P_FILE_ACCESS);

    if (l_facc_type == FACC_DEFAULT)
	return (ret_pl);

    if (l_facc_type == FACC_MPIO){
	/* set Parallel access with communicator */
	ret = H5Pset_fapl_mpio(ret_pl, comm, info);
	return(ret_pl);
    }

    if (l_facc_type == (FACC_MPIO | FACC_SPLIT)){
	hid_t mpio_pl;

	mpio_pl = H5Pcreate (H5P_FILE_ACCESS);
	/* set Parallel access with communicator */
	ret = H5Pset_fapl_mpio(mpio_pl, comm, info);

	/* setup file access template */
	ret_pl = H5Pcreate (H5P_FILE_ACCESS);
	/* set Parallel access with communicator */
	ret = H5Pset_fapl_split(ret_pl, ".meta", mpio_pl, ".raw", mpio_pl);
	H5Pclose(mpio_pl);
	return(ret_pl);
    }

    /* unknown file access types */
    return (ret_pl);
}


/* Create query */
static hid_t
test_query_create(void)
{
    int min = 17;
    int max = 22;
    double value1 = 21.2f;
    int value2 = 25;
    hid_t q1 = H5I_BADID, q2 = H5I_BADID, q3 = H5I_BADID, q4 = H5I_BADID;
    hid_t q5 = H5I_BADID, q6 = H5I_BADID, q7 = H5I_BADID;

    /* Create and combine a bunch of queries
     * Query is: ((17 < x < 22) AND (x != 21.2)) OR (x == 25)
     */
    if ((q1 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_GREATER_THAN,
            H5T_NATIVE_INT, &min)) < 0) FAIL_STACK_ERROR;
    if ((q2 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_LESS_THAN, H5T_NATIVE_INT,
            &max)) < 0) FAIL_STACK_ERROR;
    if ((q3 = H5Qcombine(q1, H5Q_COMBINE_AND, q2)) < 0) FAIL_STACK_ERROR;
    if ((q4 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_NOT_EQUAL,
            H5T_NATIVE_DOUBLE, &value1)) < 0) FAIL_STACK_ERROR;
    if ((q5 = H5Qcombine(q3, H5Q_COMBINE_AND, q4)) < 0) FAIL_STACK_ERROR;
    if ((q6 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_EQUAL, H5T_NATIVE_INT,
            &value2)) < 0) FAIL_STACK_ERROR;
    if ((q7 = H5Qcombine(q5, H5Q_COMBINE_OR, q6)) < 0) FAIL_STACK_ERROR;

    // H5Q_enable_visualize_query();

    return q7;

error:
    H5E_BEGIN_TRY {
        H5Qclose(q1);
        H5Qclose(q2);
        H5Qclose(q3);
        H5Qclose(q4);
        H5Qclose(q5);
        H5Qclose(q6);
        H5Qclose(q7);
    } H5E_END_TRY;
    return -1;
}

/* Create query for type of reference */
static hid_t
test_query_create_type(H5R_type_t type, hbool_t compound)
{
    int min = 17;
    int max = 22;
    const char *link_name = "Pressure";
    const char *attr_name = "SensorID";
    int attr_value = 2;
    hid_t q1 = H5I_BADID, q2 = H5I_BADID, q3 = H5I_BADID, q4 = H5I_BADID;
    hid_t q5 = H5I_BADID, q6 = H5I_BADID, q7 = H5I_BADID, q8 = H5I_BADID;
    hid_t q9 = H5I_BADID;

    /* Select region */
    if ((q1 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_GREATER_THAN,
            H5T_NATIVE_INT, &min)) < 0) FAIL_STACK_ERROR;
    if ((q2 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_LESS_THAN, H5T_NATIVE_INT,
            &max)) < 0) FAIL_STACK_ERROR;
    if ((q3 = H5Qcombine(q1, H5Q_COMBINE_AND, q2)) < 0) FAIL_STACK_ERROR;

    /* Select attribute */
    if ((q4 = H5Qcreate(H5Q_TYPE_ATTR_NAME, H5Q_MATCH_EQUAL, attr_name)) < 0) FAIL_STACK_ERROR;
    if ((q5 = H5Qcreate(H5Q_TYPE_ATTR_VALUE, H5Q_MATCH_EQUAL, H5T_NATIVE_INT, &attr_value)) < 0) FAIL_STACK_ERROR;
    if ((q6 = H5Qcombine(q4, H5Q_COMBINE_AND, q5)) < 0) FAIL_STACK_ERROR;

    /* Select object */
    if ((q7 = H5Qcreate(H5Q_TYPE_LINK_NAME, H5Q_MATCH_EQUAL, link_name)) < 0) FAIL_STACK_ERROR;

    /* Combine queries */
    if (type == H5R_REGION) {
        if (mpi_rank == 0)
            printf("Query-> (17 < x < 22) AND ((link='Pressure') AND ((attr='SensorID') AND (attr=2)))\n");
        if ((q8 = H5Qcombine(q6, H5Q_COMBINE_AND, q7)) < 0) FAIL_STACK_ERROR;
        if ((q9 = H5Qcombine(q3, H5Q_COMBINE_AND, q8)) < 0) FAIL_STACK_ERROR;
    }
    else if (type == H5R_OBJECT) {
        if (mpi_rank == 0)
            printf("Query-> (link='Pressure') AND (attr='SensorID') AND (attr=2)\n");
        if ((q8 = H5Qcombine(q6, H5Q_COMBINE_AND, q7)) < 0) FAIL_STACK_ERROR;
        q9 = q8;
    }
    else if (type == H5R_ATTR) {
        if (compound) {
            if (mpi_rank == 0)
                printf("Query-> (attr='SensorID') AND (attr=2)\n");
            q9 = q6;
        } else {
            if (mpi_rank == 0)
                printf("Query-> (attr=2)\n");
            q9 = q5;
        }
    }

    // H5Q_enable_visualize_query();

    return q9;

error:
    H5E_BEGIN_TRY {
        H5Qclose(q1);
        H5Qclose(q2);
        H5Qclose(q3);
        H5Qclose(q4);
        H5Qclose(q5);
        H5Qclose(q6);
        H5Qclose(q7);
        H5Qclose(q8);
        H5Qclose(q9);
    } H5E_END_TRY;
    return -1;
}

/* Close query */
static herr_t
test_query_close(hid_t query)
{
    H5Q_combine_op_t op_type;

    H5Qget_combine_op(query, &op_type);
    if (op_type == H5Q_SINGLETON) {
        if (H5Qclose(query) < 0) FAIL_STACK_ERROR;
    } else {
        hid_t sub_query1, sub_query2;

        if (H5Qget_components(query, &sub_query1, &sub_query2) < 0)
            FAIL_STACK_ERROR;

        if (test_query_close(sub_query1) < 0) FAIL_STACK_ERROR;
        if (test_query_close(sub_query2) < 0) FAIL_STACK_ERROR;
    }

    return 0;

error:
    H5E_BEGIN_TRY {
        /* Nothing */
    } H5E_END_TRY;
    return -1;
}

/* Apply query on data element */
static herr_t
test_query_apply_elem(hid_t query, hbool_t *result, hid_t type_id, const void *value)
{
    H5Q_combine_op_t op_type;

    H5Qget_combine_op(query, &op_type);
    if (op_type == H5Q_SINGLETON) {
        H5Q_type_t query_type;

        if (H5Qget_type(query, &query_type) < 0) FAIL_STACK_ERROR;

        /* Query type should be H5Q_TYPE_DATA_ELEM */
        if (query_type != H5Q_TYPE_DATA_ELEM) FAIL_STACK_ERROR;
        if (H5Qapply_atom(query, result, type_id, value) < 0) FAIL_STACK_ERROR;
    } else {
        hbool_t sub_result1, sub_result2;
        hid_t sub_query1_id, sub_query2_id;

        if (H5Qget_components(query, &sub_query1_id, &sub_query2_id) < 0)
            FAIL_STACK_ERROR;
        if (test_query_apply_elem(sub_query1_id, &sub_result1, type_id, value) < 0)
            FAIL_STACK_ERROR;
        if (test_query_apply_elem(sub_query2_id, &sub_result2, type_id, value) < 0)
            FAIL_STACK_ERROR;

        *result = (op_type == H5Q_COMBINE_AND) ? sub_result1 && sub_result2 :
                sub_result1 || sub_result2;

        if (H5Qclose(sub_query1_id) < 0) FAIL_STACK_ERROR;
        if (H5Qclose(sub_query2_id) < 0) FAIL_STACK_ERROR;
    }

    return 0;

error:
    H5E_BEGIN_TRY {
        /* Nothing */
    } H5E_END_TRY;
    return -1;
}

/* Apply query */
static herr_t
test_query_apply(hid_t query)
{
    int data_elem1 = 15, data_elem2 = 20, data_elem3 = 25;
    double data_elem4 = 21.2f;
    float data_elem5 = 17.2f;
    double data_elem6 = 18.0f;
    double data_elem7 = 2.4f;
    double data_elem8 = 25.0f;
    hbool_t result = 0;

    if (test_query_apply_elem(query, &result, H5T_NATIVE_INT, &data_elem1) < 0)
        FAIL_STACK_ERROR;
    if (result) FAIL_STACK_ERROR;

    if (test_query_apply_elem(query, &result, H5T_NATIVE_INT, &data_elem2) < 0)
        FAIL_STACK_ERROR;
    if (!result) FAIL_STACK_ERROR;

    if (test_query_apply_elem(query, &result, H5T_NATIVE_INT, &data_elem3) < 0)
        FAIL_STACK_ERROR;
    if (!result) FAIL_STACK_ERROR;

    if (test_query_apply_elem(query, &result, H5T_NATIVE_DOUBLE, &data_elem4) < 0)
        FAIL_STACK_ERROR;
    if (result) FAIL_STACK_ERROR;

    if (test_query_apply_elem(query, &result, H5T_NATIVE_FLOAT, &data_elem5) < 0)
        FAIL_STACK_ERROR;
    if (!result) FAIL_STACK_ERROR;

    if (test_query_apply_elem(query, &result, H5T_NATIVE_DOUBLE, &data_elem6) < 0)
        FAIL_STACK_ERROR;
    if (!result) FAIL_STACK_ERROR;

    if (test_query_apply_elem(query, &result, H5T_NATIVE_DOUBLE, &data_elem7) < 0)
        FAIL_STACK_ERROR;
    if (result) FAIL_STACK_ERROR;

    if (test_query_apply_elem(query, &result, H5T_NATIVE_DOUBLE, &data_elem8) < 0)
        FAIL_STACK_ERROR;
    if (!result) FAIL_STACK_ERROR;

    return 0;

error:
    H5E_BEGIN_TRY {
        /* Nothing */
    } H5E_END_TRY;
    return -1;
}

/* Encode query */
static herr_t
test_query_encode(hid_t query)
{
    hid_t dec_query = H5I_BADID;
    char *buf = NULL;
    size_t nalloc = 0;

    if (H5Qencode(query, NULL, &nalloc) < 0) FAIL_STACK_ERROR;
    buf = (char *) malloc(nalloc);
    if (H5Qencode(query, buf, &nalloc) < 0) FAIL_STACK_ERROR;
    if ((dec_query = H5Qdecode(buf)) < 0) FAIL_STACK_ERROR;

    /* Check/apply decoded query */
    if (test_query_apply(dec_query) < 0) FAIL_STACK_ERROR;

    if (H5Qclose(dec_query) < 0) FAIL_STACK_ERROR;
    HDfree(buf);

    return 0;

error:
    H5E_BEGIN_TRY {
        if (dec_query != H5I_BADID) H5Qclose(dec_query);
        HDfree(buf);
    } H5E_END_TRY;
    return -1;
}

static char *
gen_file_name(unsigned meta_idx_plugin, unsigned data_idx_plugin)
{
    char *filename = NULL;

    if (NULL == (filename = (char *)HDmalloc(MAX_NAME)))
        return NULL;
    sprintf(filename, "query_index_%s_%s", plugin_names[meta_idx_plugin],
        plugin_names[data_idx_plugin]);

    return filename;
}

static herr_t
test_query_create_simple_file(const char *filename, hid_t fapl, unsigned n_objs,
     unsigned meta_idx_plugin, unsigned data_idx_plugin)
{
    hid_t acc_tpl;		/* File access templates */
    hid_t file_id = H5I_BADID;
    hid_t aspace = H5I_BADID;
    hid_t memspace = H5I_BADID;
    hid_t filespace = H5I_BADID;
    hid_t dcpl = H5P_DEFAULT;
    hid_t dxpl_id = H5P_DEFAULT;
    hid_t fctmpl = H5P_DEFAULT;
    hid_t fapl_id = H5P_DEFAULT;
    hsize_t adim[1] = {1};
    hsize_t dims[1] = {NTUPLES};
    hsize_t ntuples = NTUPLES;
    hsize_t i, offset = 0;
    float nextValue, *data_slice = NULL;
    unsigned n;

    if ( mpi_rank == 0 ) {
        HDfprintf(stdout, "Constructing test files...");
    }

    ntuples = (hsize_t)(NTUPLES/mpi_size);
    nextValue = (float)(mpi_rank * ntuples);

    /* setup data to write */
    if ((data_slice = (float *)HDmalloc(ntuples * sizeof(float))) == NULL )
        FAIL_STACK_ERROR;

    for(i=0; i<ntuples; i++) {
        data_slice[i] = nextValue;
        nextValue += 1;
    }
    if ((fctmpl = H5Pcreate(H5P_FILE_CREATE)) < 0) 
        FAIL_STACK_ERROR;

    /* setup FAPL */
    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0 )
        FAIL_STACK_ERROR;

    if ((H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL)) < 0 )
        FAIL_STACK_ERROR;

    if ( (file_id = H5Fcreate(filename, H5F_ACC_TRUNC,fctmpl, fapl_id)) < 0 )
        FAIL_STACK_ERROR;

    if (data_idx_plugin && ((dcpl = H5Pcreate(H5P_DATASET_CREATE)) < 0)) FAIL_STACK_ERROR;
    if (data_idx_plugin && (H5Pset_index_plugin(dcpl, data_idx_plugin)) < 0) FAIL_STACK_ERROR;

    /* create and write the dataset */
    if ( (dxpl_id = H5Pcreate(H5P_DATASET_XFER)) < 0 )
        FAIL_STACK_ERROR;

    if ( (H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE)) < 0 )
        FAIL_STACK_ERROR;

    dims[0] = ntuples;
    if ( (memspace = H5Screate_simple(1, dims, NULL)) < 0 )
        FAIL_STACK_ERROR;

    dims[0] = (hsize_t)NTUPLES;
    if ( (filespace = H5Screate_simple(1, dims, NULL)) < 0 )
        FAIL_STACK_ERROR;

    if ((aspace = H5Screate_simple(1, adim, NULL)) < 0)
        FAIL_STACK_ERROR;

    offset = (hsize_t)mpi_rank * (hsize_t)ntuples;
    if ( (H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset, NULL, &ntuples, NULL)) < 0 )
        FAIL_STACK_ERROR;

    for (n = 1; n <= n_objs; n++) {
        hid_t obj = H5I_BADID;
        hid_t pres = H5I_BADID;
        hid_t temp = H5I_BADID;
        hid_t id_pres = H5I_BADID;
        hid_t id_temp = H5I_BADID;
        char obj_name[MAX_NAME];

        if (mpi_rank == 0)
            printf("Writing object %d...\r", n);

        /* Create simple group and dataset */
        HDmemset(obj_name, '\0', MAX_NAME);
        sprintf(obj_name, "Object%d", n);
        if ((obj = H5Gcreate(file_id, obj_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;

        if ( (pres = H5Dcreate(obj, "Pressure", H5T_NATIVE_FLOAT, filespace, H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0 )
            FAIL_STACK_ERROR;
        if ( (temp = H5Dcreate(obj, "Temperature", H5T_NATIVE_FLOAT, filespace, H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0 )
            FAIL_STACK_ERROR;

        /* Add attributes to datasets */
        if ((id_pres = H5Acreate(pres, "SensorID", H5T_NATIVE_INT, aspace, H5P_DEFAULT, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;
        if ((id_temp = H5Acreate(temp, "SensorID", H5T_NATIVE_INT, aspace, H5P_DEFAULT, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;

        if (H5Awrite(id_pres, H5T_NATIVE_INT, &n) < 0) FAIL_STACK_ERROR;
        if (H5Awrite(id_temp, H5T_NATIVE_INT, &n) < 0) FAIL_STACK_ERROR;

        if (H5Aclose(id_pres) < 0) FAIL_STACK_ERROR;
        if (H5Aclose(id_temp) < 0) FAIL_STACK_ERROR;

        /* Write data */
        if ( (H5Dwrite(pres, H5T_NATIVE_FLOAT, memspace, filespace, dxpl_id, data_slice)) < 0 ) 
            FAIL_STACK_ERROR;
        if ( (H5Dwrite(temp, H5T_NATIVE_FLOAT, memspace, filespace, dxpl_id, data_slice)) < 0 ) 
            FAIL_STACK_ERROR;

        if (H5Dclose(pres) < 0) FAIL_STACK_ERROR;
        if (H5Dclose(temp) < 0) FAIL_STACK_ERROR;
        if (H5Gclose(obj) < 0) FAIL_STACK_ERROR;

    }

    if (data_idx_plugin && (dcpl != H5P_DEFAULT) && (H5Pclose(dcpl) < 0))
        FAIL_STACK_ERROR;
    dcpl = H5I_BADID;

    if (H5Sclose(aspace) < 0)
        FAIL_STACK_ERROR;
    aspace = H5I_BADID;

    if (H5Sclose(memspace) < 0 ) 
        FAIL_STACK_ERROR;
    memspace = H5I_BADID;

    if (H5Sclose(filespace) < 0 )
        FAIL_STACK_ERROR;
    filespace = H5I_BADID;

    if (H5Pclose(dxpl_id) < 0 )
        FAIL_STACK_ERROR;
    dxpl_id = H5I_BADID;

    if (H5Pclose(fapl_id) < 0 )
        FAIL_STACK_ERROR;
    fapl_id = H5I_BADID;

    if (H5Pclose(fctmpl) < 0)
        FAIL_STACK_ERROR;
    fctmpl = H5I_BADID;

    if (H5Fclose(file_id) < 0 )
        FAIL_STACK_ERROR;
    file_id = H5I_BADID;

    /* free data_slice if it has been allocated */
    if ( data_slice != NULL ) {
        HDfree(data_slice);
        data_slice = NULL;
    }
    return 0;

error:


    H5E_BEGIN_TRY {
      if (memspace != H5I_BADID)
        H5Pclose(memspace);
      if (filespace != H5I_BADID)
        H5Sclose(filespace);
      if (aspace != H5I_BADID)
        H5Sclose(aspace);
      if (file_id != H5I_BADID)
        H5Fclose(file_id);
      if (data_slice != NULL)
        HDfree(data_slice);
    } H5E_END_TRY;

    return -1;

    
}

/* Read region */
static herr_t
test_query_read_selection(size_t file_count, const char *filenames[],
    hid_t *files, hid_t view, H5R_type_t rtype)
{
    hid_t refs = H5I_BADID, ref_type = H5I_BADID, ref_space = H5I_BADID;
    size_t n_refs = 0, ref_size, ref_buf_size;
    void *ref_buf = NULL;
    href_t *ref_ptr = NULL;
    const char *ref_path = NULL;
    hid_t obj = H5I_BADID, type = H5I_BADID, space = H5I_BADID, mem_space = H5I_BADID;
    size_t n_elem, elem_size, buf_size;
    float *buf = NULL;
    unsigned int i;

    if (rtype == H5R_REGION)
        ref_path = H5Q_VIEW_REF_REG_NAME;
    else if (rtype == H5R_OBJECT)
        ref_path = H5Q_VIEW_REF_OBJ_NAME;
    else if (rtype == H5R_ATTR)
        ref_path = H5Q_VIEW_REF_ATTR_NAME;

    /* Get region references from view */
    if (H5Lexists(view, ref_path, H5P_DEFAULT) == TRUE) {
       if ((refs = H5Dopen(view, ref_path, H5P_DEFAULT)) < 0) FAIL_STACK_ERROR;
       if ((ref_type = H5Dget_type(refs)) < 0) FAIL_STACK_ERROR;
       if ((ref_space = H5Dget_space(refs)) < 0) FAIL_STACK_ERROR;
       if (0 == (n_refs = (size_t) H5Sget_select_npoints(ref_space))) FAIL_STACK_ERROR;
    }
    if (mpi_rank == 0)
           printf("Found %zu reference(s)\n", n_refs);
    if (n_refs) {
        if (0 == (ref_size = H5Tget_size(ref_type))) FAIL_STACK_ERROR;
        //    printf("Reference type size: %zu\n", ref_size);

        /* Allocate buffer to hold data */
        ref_buf_size = n_refs * ref_size;
        if (NULL == (ref_buf = HDmalloc(ref_buf_size))) FAIL_STACK_ERROR;

        if ((H5Dread(refs, ref_type, H5S_ALL, ref_space, H5P_DEFAULT, ref_buf)) < 0) FAIL_STACK_ERROR;

        /* Get dataset / space / type ID for the referenced dataset region */
        ref_ptr = (href_t *) ref_buf;
        for (i = 0; i < n_refs; i++) {
            char obj_path[MAX_NAME];
            char filename[MAX_NAME];
            hid_t loc = H5I_BADID;

            if (H5Rget_file_name(ref_ptr[i], filename, MAX_NAME) < 0) FAIL_STACK_ERROR;
            printf("Found reference from file: %s\n", filename);
            if (file_count > 1) {
                unsigned int j;
                for (j = 0; j < file_count; j++) {
                    if (0 == HDstrcmp(filename, filenames[j])) {
                        loc = files[j];
                        break;
                    }
                }
            } else {
                if (0 != HDstrcmp(filename, filenames[0])) FAIL_STACK_ERROR;
                loc = files[0];
            }
            if (H5Rget_obj_name(loc, ref_ptr[i], obj_path, MAX_NAME) < 0) FAIL_STACK_ERROR;
                printf("Found reference from object: %s\n", obj_path);
                if ((obj = H5Rget_object(loc, H5P_DEFAULT, ref_ptr[i])) < 0) FAIL_STACK_ERROR;

                if (rtype == H5R_REGION) {
                    unsigned int j;

                if ((space = H5Rget_region2(loc, ref_ptr[i])) < 0) FAIL_STACK_ERROR;
                if ((type = H5Dget_type(obj)) < 0) FAIL_STACK_ERROR;
                if (0 == (n_elem = (size_t) H5Sget_select_npoints(space))) FAIL_STACK_ERROR;
                if (0 == (elem_size = H5Tget_size(type))) FAIL_STACK_ERROR;

                /* Get name of dataset */
                printf("Region has %zu elements of size %zu\n", n_elem, elem_size);

                /* Allocate buffer to hold data */
                buf_size = n_elem * elem_size;
                if (NULL == (buf = (float *) HDmalloc(buf_size))) FAIL_STACK_ERROR;

                if ((mem_space = H5Screate_simple(1, (hsize_t *) &n_elem, NULL)) < 0) FAIL_STACK_ERROR;

                if ((H5Dread(obj, type, mem_space, space, H5P_DEFAULT, buf)) < 0) FAIL_STACK_ERROR;

                printf("Elements found are:\n");
                for (j = 0; j < n_elem; j++)
                    printf("%f ", buf[j]);
                printf("\n");

                if (H5Sclose(mem_space) < 0) FAIL_STACK_ERROR;
                if (H5Sclose(space) < 0) FAIL_STACK_ERROR;
                if (H5Tclose(type) < 0) FAIL_STACK_ERROR;
                HDfree(buf);
                buf = NULL;
            }
            if (rtype == H5R_ATTR) {
                char attr_name[MAX_NAME];

                if (H5Rget_attr_name(obj, ref_ptr[i], attr_name, MAX_NAME) < 0) FAIL_STACK_ERROR;
                printf("Attribute name: %s\n", attr_name);
            }
            if (H5Dclose(obj) < 0) FAIL_STACK_ERROR;
        }

        if ((H5Dref_reclaim(ref_type, ref_space, H5P_DEFAULT, ref_buf)) < 0) FAIL_STACK_ERROR;
        if (H5Sclose(ref_space) < 0) FAIL_STACK_ERROR;
        if (H5Tclose(ref_type) < 0) FAIL_STACK_ERROR;
        if (H5Dclose(refs) < 0) FAIL_STACK_ERROR;
        HDfree(ref_buf);
        ref_buf = NULL;
    }
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(mem_space);
        H5Dclose(obj);
        H5Sclose(space);
        H5Tclose(type);
        HDfree(buf);
        H5Dclose(refs);
        H5Sclose(ref_space);
        H5Tclose(ref_type);
        HDfree(ref_buf);
    } H5E_END_TRY;

    return -1;
}

static herr_t
test_query_apply_view(const char *filename, hid_t fapl, unsigned n_objs,
    unsigned meta_idx_plugin, unsigned data_idx_plugin)
{
    H5R_type_t ref_types[NTYPES] = { H5R_REGION, H5R_OBJECT, H5R_ATTR, H5R_ATTR };
    unsigned ref_masks[NTYPES] = { H5Q_REF_REG, H5Q_REF_OBJ, H5Q_REF_ATTR, H5Q_REF_ATTR };
    const char *ref_names[NTYPES] = { "regions (compound)",
        "objects (compound)", "attributes (compound)", "attributes (simple)" };
    hbool_t compound[NTYPES] = { TRUE, TRUE, TRUE, FALSE };
    hid_t file = H5I_BADID;
    hid_t view = H5I_BADID;
    hid_t query = H5I_BADID;
    hid_t fapl_id = H5P_DEFAULT;

    int i;

    if (mpi_rank == 0) {
       printf(" ...\n---\n");
       /* Create a simple file for testing queries */
       printf("Creating test file \"%s\"\n", filename);
    }
    if ((test_query_create_simple_file(filename, fapl, n_objs, meta_idx_plugin,
            data_idx_plugin)) < 0) FAIL_STACK_ERROR;

    /* setup FAPL */
    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0 )
        FAIL_STACK_ERROR;

    if ((H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL)) < 0 )
        FAIL_STACK_ERROR;

    /* Open the file in read-only */
    if (meta_idx_plugin) {
        if ((file = H5Fopen(filename, H5F_ACC_RDWR, fapl_id)) < 0) FAIL_STACK_ERROR;
        /* Create metadata index */
        if (H5Xcreate(file, meta_idx_plugin, H5P_DEFAULT) < 0) FAIL_STACK_ERROR;
    } else {
        if ((file = H5Fopen(filename, H5F_ACC_RDWR, fapl_id)) < 0) FAIL_STACK_ERROR;
    }

    for (i = 0; i < NTYPES; i++) {
        struct timeval t1, t2;
        struct timeval t_total = {0, 0};
        int loop;
        unsigned result = 0;

        if (mpi_rank == 0) {
            printf("\nQuery on %s\n", ref_names[i]);
            printf("%s\n", "--------------------------------------------------------------------------------");
	}
        /* Test query */
        if ((query = test_query_create_type(ref_types[i], compound[i])) < 0) FAIL_STACK_ERROR;

        for (loop = 0; loop < NLOOP; loop++) {
	    result = 0;
            HDgettimeofday(&t1, NULL);
            if ((view = H5Qapply(file, query, &result, H5P_DEFAULT)) < 0) FAIL_STACK_ERROR;
            HDgettimeofday(&t2, NULL);

            t_total.tv_sec += (t2.tv_sec - t1.tv_sec);
            t_total.tv_usec += (t2.tv_usec - t1.tv_usec);

            if (loop < (NLOOP - 1))
                if (H5Gclose(view) < 0) FAIL_STACK_ERROR;
        }

        if (mpi_rank == 0)
            printf("View creation time on %s: %lf ms\n", ref_names[i],
            ((float) t_total.tv_sec) * 1000.0f / NLOOP
            + ((float) t_total.tv_usec) / (NLOOP * 1000.0f));

        if (!(result & ref_masks[i])) FAIL_STACK_ERROR;
        if (test_query_read_selection(1, &filename, &file, view, ref_types[i]) < 0) FAIL_STACK_ERROR;

        if (H5Gclose(view) < 0) FAIL_STACK_ERROR;
        if (test_query_close(query)) FAIL_STACK_ERROR;
    }

    /* Remove index */
    if (meta_idx_plugin && H5Xremove(file, meta_idx_plugin) < 0) FAIL_STACK_ERROR;

    /* Close file */
    if (H5Fclose(file) < 0) FAIL_STACK_ERROR;

    if (mpi_rank == 0)
        printf("---\n...");
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(view);
        H5Fclose(file);
        test_query_close(query);
    } H5E_END_TRY;
    return -1;
}

static herr_t
test_query_apply_view_multi(const char *filenames[], hid_t fapl, unsigned n_objs,
    unsigned meta_idx_plugin, unsigned data_idx_plugin)
{
    hid_t files[MULTI_NFILES] = {H5I_BADID, H5I_BADID, H5I_BADID};
    hid_t view = H5I_BADID;
    hid_t query = H5I_BADID;
    struct timeval t1, t2;
    unsigned result = 0;
    unsigned int i;

    printf(" ...\n---\n");

    /* Create simple files for testing queries */
    for (i = 0; i < MULTI_NFILES; i++) {
        printf("Creating test file \"%s\"\n", filenames[i]);
        if ((test_query_create_simple_file(filenames[i], fapl, n_objs,
            meta_idx_plugin, data_idx_plugin)) < 0) FAIL_STACK_ERROR;
        /* Open the file in read-only */
        if ((files[i] = H5Fopen(filenames[i], H5F_ACC_RDONLY, fapl)) < 0) FAIL_STACK_ERROR;
    }

    printf("\nRegion query\n");
    printf(  "------------\n");

    /* Test region query */
    if ((query = test_query_create_type(H5R_REGION, TRUE)) < 0) FAIL_STACK_ERROR;

    HDgettimeofday(&t1, NULL);

    if ((view = H5Qapply_multi(MULTI_NFILES, files, query, &result, H5P_DEFAULT)) < 0) FAIL_STACK_ERROR;

    HDgettimeofday(&t2, NULL);

    printf("View creation time on region: %lf ms\n",
            ((float) (t2.tv_sec - t1.tv_sec)) * 1000.0f
            + ((float) (t2.tv_usec - t1.tv_usec)) / 1000.0f);

    if (!(result & H5Q_REF_REG)) FAIL_STACK_ERROR;
    if (test_query_read_selection(MULTI_NFILES, filenames, files, view, H5R_REGION) < 0) FAIL_STACK_ERROR;

    if (H5Gclose(view) < 0) FAIL_STACK_ERROR;
    if (test_query_close(query)) FAIL_STACK_ERROR;

    printf("\nObject query\n");
    printf(  "------------\n");

    /* Test object query */
    if ((query = test_query_create_type(H5R_OBJECT, TRUE)) < 0) FAIL_STACK_ERROR;
    if ((view = H5Qapply_multi(MULTI_NFILES, files, query, &result, H5P_DEFAULT)) < 0) FAIL_STACK_ERROR;

    if (!(result & H5Q_REF_OBJ)) FAIL_STACK_ERROR;
    if (test_query_read_selection(MULTI_NFILES, filenames, files, view, H5R_OBJECT) < 0) FAIL_STACK_ERROR;

    if (H5Gclose(view) < 0) FAIL_STACK_ERROR;
    if (test_query_close(query)) FAIL_STACK_ERROR;

    printf("\nAttribute query\n");
    printf(  "---------------\n");

    /* Test attribute query */
    if ((query = test_query_create_type(H5R_ATTR, TRUE)) < 0) FAIL_STACK_ERROR;
    if ((view = H5Qapply_multi(MULTI_NFILES, files, query, &result, H5P_DEFAULT)) < 0) FAIL_STACK_ERROR;

    if (!(result & H5Q_REF_ATTR)) FAIL_STACK_ERROR;
    if (test_query_read_selection(MULTI_NFILES, filenames, files, view, H5R_ATTR) < 0) FAIL_STACK_ERROR;

    if (H5Gclose(view) < 0) FAIL_STACK_ERROR;
    if (test_query_close(query)) FAIL_STACK_ERROR;

    for (i = 0; i < MULTI_NFILES; i++)
        if (H5Fclose(files[i]) < 0) FAIL_STACK_ERROR;
    if (test_query_close(query)) FAIL_STACK_ERROR;

    printf("---\n...");

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(view);
        for (i = 0; i < MULTI_NFILES; i++)
            H5Fclose(files[i]);
        test_query_close(query);
    } H5E_END_TRY;
    return -1;
}

int
main(int argc, char *argv[])
{
    char filename[MAX_NAME]; /* file name */
    char **FILENAME;
    hid_t query = H5I_BADID, fapl = H5I_BADID;
    unsigned n_objs = 3;
    unsigned meta_idx_plugin = H5X_PLUGIN_NONE;
    unsigned data_idx_plugin = H5X_PLUGIN_NONE;
    unsigned n_tests = 1; /* TODO for now */
#ifdef H5_HAVE_MDHIM
    int provided;
#endif


#ifdef H5_HAVE_PARALLEL
    MPI_Init(&argc, &argv);
#endif

    /* For manual tests and benchmarks */
    /* TODO enable verbose mode */
    if (argc > 1)
        n_objs = (unsigned) atoi(argv[1]);
    if (argc > 2)
        meta_idx_plugin = (unsigned) atoi(argv[2]);
    if (argc > 3)
        data_idx_plugin = (unsigned) atoi(argv[3]);
#ifdef H5_HAVE_MDHIM
    if (meta_idx_plugin == H5X_PLUGIN_META_MDHIM) {
        if (MPI_SUCCESS != MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE,
            &provided)) {
            fprintf(stderr, "Error initializing MPI with threads\n");
            goto error;
        }
        if (provided != MPI_THREAD_MULTIPLE) {
            fprintf(stderr, "Not able to enable MPI_THREAD_MULTIPLE mode\n");
            goto error;
        }
    }
#endif

    /* Reset library */
    h5_reset();

    fapl = h5_fileaccess();
#ifdef H5_HAVE_PARALLEL
    {
      herr_t ret;
      if ((MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank) == MPI_SUCCESS) &&
	  (MPI_Comm_size(MPI_COMM_WORLD, &mpi_size) == MPI_SUCCESS)) {
	if (mpi_size > 1) {
	  hid_t ret_pl = H5Pcreate (H5P_FILE_ACCESS);
	  if (H5Pset_fapl_mpio(ret_pl, MPI_COMM_WORLD, MPI_INFO_NULL) < 0)
	      puts("file access property list FAILED to set MPIO");
	  else fapl = ret_pl;
	} /* mpi_size */
      }
    }
#endif

    /* Generate file names */
    FILENAME = (char **)HDmalloc(sizeof(char *) * (n_tests + 1));
    FILENAME[n_tests] = NULL;
    FILENAME[0] = gen_file_name(meta_idx_plugin, data_idx_plugin);

    /* Check that no object is left open */
    H5Pset_fclose_degree(fapl, H5F_CLOSE_SEMI);

    /* Use latest format */
    if (H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0)
        FAIL_STACK_ERROR;

    TESTING("query create and combine");

    if ((query = test_query_create()) < 0) FAIL_STACK_ERROR;

    PASSED();

    TESTING("query apply");

    if (test_query_apply(query) < 0) FAIL_STACK_ERROR;

    PASSED();

    TESTING("query encode/decode");

    if (test_query_encode(query) < 0) FAIL_STACK_ERROR;

    PASSED();

    TESTING("query close");

    if (test_query_close(query) < 0) FAIL_STACK_ERROR;

    PASSED();

    TESTING("query apply view");

    h5_fixname(FILENAME[0], fapl, filename, sizeof(filename));

    if (test_query_apply_view(filename, fapl, n_objs, meta_idx_plugin,
        data_idx_plugin) < 0) FAIL_STACK_ERROR;

    PASSED();

//    TESTING("query apply view multiple (no index)");
//
//    if (test_query_apply_view_multi(filename_multi, fapl, n_objs,
//        H5X_PLUGIN_NONE, H5X_PLUGIN_NONE) < 0) FAIL_STACK_ERROR;
//
//    PASSED();

    /* Verify symbol table messages are cached */
    if(h5_verify_cached_stabs(FILENAME, fapl) < 0) TEST_ERROR

    puts("All query tests passed.");
#if 0
    h5_cleanup(FILENAME, fapl);
#endif
//    for (i = 0; i < MULTI_NFILES; i++)
//        HDfree(filename_multi[i]);
//    HDfree(filename_multi);
//    HDremove(filename);
    HDfree(FILENAME[0]);
    HDfree(FILENAME);


#ifdef H5_HAVE_MDHIM
    if ((meta_idx_plugin == H5X_PLUGIN_META_MDHIM) &&
        (MPI_SUCCESS != MPI_Finalize())) {
        fprintf(stderr, "Error finalizing MPI\n");
        goto error;
    }
#endif
#ifdef H5_HAVE_PARALLEL
    MPI_Finalize();
#endif
    return 0;

error:
    puts("*** TESTS FAILED ***");
    H5E_BEGIN_TRY {
        test_query_close(query);
    } H5E_END_TRY;

#ifdef H5_HAVE_PARALLEL
    MPI_Finalize();
#endif
    return 1;
}
