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

#include "h5test.h"

const char *FILENAME[] = {
    "index",
    NULL
};

#define DATASETNAME1 "dataset1"
#define DATASETNAME2 "dataset2"

#define NTUPLES 256
#define NLOOP 5

static herr_t
write_dataset(hid_t file, const char *dataset_name, hsize_t ntuples,
        hsize_t ncomponents, hid_t datatype, void *buf, hid_t dcpl)
{
    hid_t dataset = H5I_BADID;
    hid_t file_space = H5I_BADID;
    hsize_t dims[2] = {ntuples, ncomponents};
    int rank = (ncomponents == 1) ? 1 : 2;

    /* Create the data space for the first dataset. */
    file_space = H5Screate_simple(rank, dims, NULL);
    if (file_space < 0) FAIL_STACK_ERROR;

    /* Create and write dataset */
    dataset = H5Dcreate2(file, dataset_name, datatype, file_space, H5P_DEFAULT,
            dcpl, H5P_DEFAULT);
    if (dataset < 0) FAIL_STACK_ERROR;

    if (H5Dwrite(dataset, datatype, H5S_ALL, file_space, H5P_DEFAULT, buf) < 0)
        FAIL_STACK_ERROR;

    if (H5Dclose(dataset) < 0) FAIL_STACK_ERROR;
    if (H5Sclose(file_space) < 0) FAIL_STACK_ERROR;

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Dclose(dataset);
        H5Sclose(file_space);
    } H5E_END_TRY;
    return -1;
}

static herr_t
test_create_index(hid_t file, const char *dataset_name, unsigned plugin)
{
    hid_t dataset = H5I_BADID;

    TESTING("index create from existing dataset");

    dataset = H5Dopen(file, dataset_name, H5P_DEFAULT);
    if (dataset < 0) FAIL_STACK_ERROR;

    /* Add indexing information */
    if (H5Xcreate(dataset, plugin, H5P_DEFAULT) < 0) FAIL_STACK_ERROR;

    if (H5Dclose(dataset) < 0) FAIL_STACK_ERROR;

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Dclose(dataset);
    } H5E_END_TRY;
    return -1;
}

static herr_t
test_create_dataset_index(hid_t file, const char *dataset_name,
        hsize_t ntuples, hsize_t ncomponents, hid_t datatype, void *buf,
        unsigned plugin)
{
    hid_t dcpl;

    TESTING("index create from new dataset");

    dcpl = H5Pcreate(H5P_DATASET_CREATE);
    if (dcpl < 0) FAIL_STACK_ERROR;
    if (H5Pset_index_plugin(dcpl, plugin) < 0) FAIL_STACK_ERROR;

    if (write_dataset(file, dataset_name, ntuples, ncomponents, datatype, buf,
            dcpl) < 0)
        FAIL_STACK_ERROR;

    if (H5Pclose(dcpl) < 0) FAIL_STACK_ERROR;

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(dcpl);
    } H5E_END_TRY;
    return -1;
}

static herr_t
test_remove_index(hid_t file, const char *dataset_name, unsigned plugin)
{
    hid_t dataset = H5I_BADID;
    hsize_t idx_count = 0;

    TESTING("index remove");

    dataset = H5Dopen(file, dataset_name, H5P_DEFAULT);
    if (dataset < 0) FAIL_STACK_ERROR;

    /* Add indexing information */
    if (H5Xremove(dataset, plugin) < 0) FAIL_STACK_ERROR;

    /* Check that there is no more index */
    if (H5Xget_count(dataset, &idx_count) < 0) FAIL_STACK_ERROR;
    if (idx_count) FAIL_STACK_ERROR;

    if (H5Dclose(dataset) < 0) FAIL_STACK_ERROR;

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Dclose(dataset);
    } H5E_END_TRY;
    return -1;
}

//static herr_t
//write_incr(hid_t file_id, const char *dataset_name,
//        hsize_t total, hsize_t ncomponents,
//        hsize_t ntuples, hsize_t start, void *buf)
//{
//    hid_t       dataset_id;
//    hid_t       file_space_id;
////    hsize_t     dims[2] = {total, ncomponents};
//    hsize_t     offset[2] = {start, 0};
//    hsize_t     count[2] = {ntuples, ncomponents};
//    int         rank = (ncomponents == 1) ? 1 : 2;
//    herr_t      ret;
//    int n;
//
//    (void) total;
//
//    /* do incremental updates */
//    for (n = 0; n < my_size; n++) {
//        if (my_rank == n) {
//            hid_t mem_space_id;
//
//            dataset_id = H5Dopen(file_id, dataset_name, H5P_DEFAULT);
//            assert(dataset_id);
//
//            file_space_id = H5Dget_space(dataset_id);
//            assert(file_space_id);
//
//            mem_space_id = H5Screate_simple(rank, count, NULL);
//            assert(mem_space_id);
//            /* write data to datasets */
//            ret = H5Sselect_hyperslab(file_space_id, H5S_SELECT_SET, offset,
//            NULL, count, NULL);
//            assert(0 == ret);
//            ret = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, mem_space_id,
//                    file_space_id, H5P_DEFAULT, buf);
//            assert(0 == ret);
//            ret = H5Sclose(mem_space_id);
//            assert(0 == ret);
//
//            /* Close the first dataset. */
//            H5Sclose(file_space_id);
//            ret = H5Dclose(dataset_id);
//            assert(0 == ret);
//        }
//        MPI_Barrier(MPI_COMM_WORLD);
//    }
//
//    return ret;
//}

static herr_t
test_query(hid_t file_id, const char *dataset_name)
{
    hsize_t start_coord[H5S_MAX_RANK + 1], end_coord[H5S_MAX_RANK + 1];
    hid_t dataset = H5I_BADID;
    hid_t space = H5I_BADID;
    float query_lb = 39.1f, query_ub = 42.6f;
    hid_t query = H5I_BADID, query1 = H5I_BADID, query2 = H5I_BADID;
    struct timeval t1, t2;
    struct timeval t_total = {0, 0};
    int i;

    TESTING("index query");

    /* Create a simple query */
    /* query = 39.1 < x < 42.1 */
    query1 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_GREATER_THAN,
            H5T_NATIVE_FLOAT, &query_lb);
    if (query1 < 0) FAIL_STACK_ERROR;

    query2 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_LESS_THAN,
            H5T_NATIVE_FLOAT, &query_ub);
    if (query2 < 0) FAIL_STACK_ERROR;

    query = H5Qcombine(query1, H5Q_COMBINE_AND, query2);
    if (query < 0) FAIL_STACK_ERROR;

    dataset = H5Dopen(file_id, dataset_name, H5P_DEFAULT);
    if (dataset < 0) FAIL_STACK_ERROR;

    for (i = 0; i < NLOOP; i++) {
        HDgettimeofday(&t1, NULL);
        if ((space = H5Dquery(dataset, H5S_ALL, query, H5P_DEFAULT)) < 0) FAIL_STACK_ERROR;
        HDgettimeofday(&t2, NULL);

        /* Bounds should be 40 and 42 */
        H5Sget_select_bounds(space, start_coord, end_coord);

        /*
        {
            hsize_t nelmts;
            nelmts = (hsize_t) H5Sget_select_npoints(space);
            printf("\n Created dataspace with %llu elements,"
                    " bounds = [(%llu, %llu):(%llu, %llu)]\n",
                    nelmts, start_coord[0], start_coord[1], end_coord[0], end_coord[1]);
        }
        */

        if (start_coord[0] != 40) FAIL_STACK_ERROR;
    //    if (start_coord[1] != 0) FAIL_STACK_ERROR;
        if (end_coord[0] != 42) FAIL_STACK_ERROR;
    //    if (end_coord[1] != 2) FAIL_STACK_ERROR;

        t_total.tv_sec += (t2.tv_sec - t1.tv_sec);
        t_total.tv_usec += (t2.tv_usec - t1.tv_usec);

        if (H5Sclose(space) < 0) FAIL_STACK_ERROR;
    }

    printf("Index query time: %lf ms\n",
            ((float) t_total.tv_sec) * 1000.0f / NLOOP
            + ((float) t_total.tv_usec) / (NLOOP * 1000.0f));

    if (H5Dclose(dataset) < 0) FAIL_STACK_ERROR;
    if (H5Qclose(query) < 0) FAIL_STACK_ERROR;
    if (H5Qclose(query2) < 0) FAIL_STACK_ERROR;
    if (H5Qclose(query1) < 0) FAIL_STACK_ERROR;

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space);
        H5Dclose(dataset);
        H5Qclose(query);
        H5Qclose(query2);
        H5Qclose(query1);
    } H5E_END_TRY;
    return -1;
}

static herr_t
test_size(hid_t file_id, const char *dataset_name)
{
    hid_t dataset = H5I_BADID;
    hsize_t index_size;

    TESTING("index get size");

    dataset = H5Dopen(file_id, dataset_name, H5P_DEFAULT);
    if (dataset < 0) FAIL_STACK_ERROR;

    if ((index_size = H5Xget_size(dataset)) == 0) FAIL_STACK_ERROR;

    if (H5Dclose(dataset) < 0) FAIL_STACK_ERROR;

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Dclose(dataset);
    } H5E_END_TRY;
    return -1;
}


int
main(int argc, char **argv)
{
    unsigned plugin = H5X_PLUGIN_DUMMY;
    char filename[1024]; /* file name */
    hsize_t ntuples = NTUPLES;
    hsize_t ncomponents = 1;
    float *data;
    hid_t file = H5I_BADID, fapl = H5I_BADID;
    hsize_t i, j;
//    int incr_update;

    if (argc > 2) {
        ntuples = (hsize_t) atoi(argv[1]);
        plugin = (unsigned) atoi(argv[2]);
        //    incr_update = atoi(argv[3]);
    }

    /* Initialize the data. */
    data = (float *) HDmalloc(sizeof(float) * ncomponents * ntuples);
    for (i = 0; i < ntuples; i++) {
        for (j = 0; j < ncomponents; j++) {
            data[ncomponents * i + j] = (float) i;
        }
    }

    /* Reset library */
    h5_reset();
    fapl = h5_fileaccess();

    h5_fixname(FILENAME[0], fapl, filename, sizeof(filename));
    /* Check that no object is left open */
    H5Pset_fclose_degree(fapl, H5F_CLOSE_SEMI);

    if ((file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl)) < 0)
        goto error;
    if (H5Pclose(fapl) < 0) goto error;

    if (write_dataset(file, DATASETNAME1, ntuples, ncomponents, H5T_NATIVE_FLOAT,
            data, H5P_DEFAULT) < 0)
        FAIL_STACK_ERROR;

    /*
     * Test creating index...
     */
    if (test_create_index(file, DATASETNAME1, plugin) < 0)
        FAIL_STACK_ERROR;

    /*
     * TODO Test creating index from dataset create...
     */
    if (test_create_dataset_index(file, DATASETNAME2, ntuples, ncomponents,
            H5T_NATIVE_FLOAT, data, plugin) < 0)
        FAIL_STACK_ERROR;

    /* Close and reopen the file */
    if (H5Fclose(file) < 0) goto error;

    fapl = h5_fileaccess();
    /* Check that no object is left open */
    H5Pset_fclose_degree(fapl, H5F_CLOSE_SEMI);
    if ((file = H5Fopen(filename, H5F_ACC_RDWR, fapl)) < 0)
        goto error;

    if (test_query(file, DATASETNAME1) < 0)
        FAIL_STACK_ERROR;
    if (test_size(file, DATASETNAME1) < 0)
        FAIL_STACK_ERROR;
    if (test_query(file, DATASETNAME2) < 0)
        FAIL_STACK_ERROR;

//    if (incr_update) {
//        start = ntuples * (hsize_t) my_rank;
//        total = ntuples * (hsize_t) my_size;
//
//        write_incr(file_id, "D0", total, ncomponents, ntuples, start, data);
//
//        MPI_Barrier(MPI_COMM_WORLD);
//
//        query_and_view(file_id, "D0");
//
//        MPI_Barrier(MPI_COMM_WORLD);
//    }

    if (test_remove_index(file, DATASETNAME1, plugin) < 0)
        FAIL_STACK_ERROR;

    /* Close the file. */
    if (H5Fclose(file) < 0) goto error;

    HDfree(data);

    /* Verify symbol table messages are cached */
    if(h5_verify_cached_stabs(FILENAME, fapl) < 0) TEST_ERROR

    puts("All index tests passed.");
    h5_cleanup(FILENAME, fapl);

    return 0;

error:
    puts("*** TESTS FAILED ***");
    H5E_BEGIN_TRY {
        H5Fclose(file);
        HDfree(data);
    } H5E_END_TRY;

    return 1;
}
