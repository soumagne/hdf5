/* 
 * test_client_obj.c: Client side of H5O routines
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mpi.h"
#include "hdf5.h"

static herr_t
visit_cb(hid_t oid, const char *name,
         const H5O_ff_info_t *oinfo, void *udata, hid_t rcxt_id)
{
    hid_t obj_id;

    printf("----------------------------------------\n");
    printf("Visiting Object %s\n", name);
    printf("IOD ID = %"PRIx64"\n", oinfo->addr);
    printf("Object type = %d\n", oinfo->type);
    printf("Number of attributes = %d\n", (int)oinfo->num_attrs);
    printf("----------------------------------------\n");
    if(strcmp(name, ".") != 0) {
        obj_id = H5Oopen_ff(oid, name, H5P_DEFAULT, rcxt_id);
        assert(obj_id > 0);

        assert(H5Oclose_ff(obj_id, H5_EVENT_STACK_NULL) == 0);
    }
    return 0;
}

int main(int argc, char **argv) {
    char file_name[50];
    hid_t file_id;
    hid_t gid;
    hid_t did, map;
    hid_t sid, dtid;
    hid_t tid1, tid2, rid1, rid2;
    hid_t fapl_id, dxpl_id;
    hid_t e_stack;
    htri_t exists = -1;

    const unsigned int nelem=60;
    hsize_t dims[1];

    uint64_t version;
    uint64_t trans_num;

    int my_rank, my_size;
    int provided;
    MPI_Request mpi_req;

    H5ES_status_t status;
    size_t num_events = 0;
    herr_t ret;

    sprintf(file_name, "%s_%s", getenv("USER"), "eff_file_obj.h5");

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if(MPI_THREAD_MULTIPLE != provided) {
        fprintf(stderr, "MPI does not have MPI_THREAD_MULTIPLE support\n");
        exit(1);
    }

    /* Call EFF_init to initialize the EFF stack.  
       As a result of this call, the Function Shipper client is started, 
       and HDF5 VOL calls are registered with the function shipper.
       An "IOD init" call is forwarded from the FS client to the FS server 
       which should already be running. */
    EFF_init(MPI_COMM_WORLD, MPI_INFO_NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &my_size);
    fprintf(stderr, "APP processes = %d, my rank is %d\n", my_size, my_rank);

    fprintf(stderr, "Create the FAPL to set the IOD VOL plugin and create the file\n");
    /* Choose the IOD VOL plugin to use with this file. 
       First we create a file access property list. Then we call a new routine to set
       the IOD plugin to use with this fapl */
    fapl_id = H5Pcreate (H5P_FILE_ACCESS);
    H5Pset_fapl_iod(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL);

    /* create an event Queue for managing asynchronous requests.

       Event Queues will releive the use from managing and completing
       individual requests for every operation. Instead of passing a
       request for every operation, the event queue is passed and
       internally the HDF5 library creates a request and adds it to
       the event queue.

       Multiple Event queue can be created used by the application. */
    e_stack = H5EScreate();
    assert(e_stack);

    /* create the file. */
    file_id = H5Fcreate_ff(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, H5_EVENT_STACK_NULL);
    assert(file_id > 0);

    /* create 1-D dataspace with 60 elements */
    dims [0] = nelem;
    sid = H5Screate_simple(1, dims, NULL);
    dtid = H5Tcopy(H5T_STD_I32LE);

    /* start transaction 2 with default Leader/Delegate model. Leader
       which is rank 0 here starts the transaction. It can be
       asynchronous, but we make it synchronous here so that the
       Leader can tell its delegates that the transaction is
       started. */
    if(0 == my_rank) {
        hid_t rid_temp;
        hid_t anon_did;
        hid_t gid2,gid3,gid11,did11,did21,did31,map111;
        version = 1;
        rid1 = H5RCacquire(file_id, &version, H5P_DEFAULT, H5_EVENT_STACK_NULL);
        assert(1 == version);

        /* create transaction object */
        tid1 = H5TRcreate(file_id, rid1, (uint64_t)2);
        assert(tid1);
        ret = H5TRstart(tid1, H5P_DEFAULT, e_stack);
        assert(0 == ret);

        /* create objects */
        gid = H5Gcreate_ff(file_id, "G1", H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(gid >= 0);


        gid2 = H5Gcreate_ff(file_id, "G2", H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(gid2 >= 0);
        gid3 = H5Gcreate_ff(file_id, "G3", H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(gid3 >= 0);
        gid11 = H5Gcreate_ff(gid, "G1", H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(gid11 >= 0);
        did11 = H5Dcreate_ff(gid, "D2", dtid, sid, H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(did11 >= 0);
        did21 = H5Dcreate_ff(gid2, "D2", dtid, sid, H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(did21 >= 0);
        did31 = H5Dcreate_ff(gid3, "D3", dtid, sid, H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(did31 >= 0);
        map111 = H5Mcreate_ff(gid11, "MAP1", H5T_STD_I32LE, H5T_STD_I32LE, 
                           H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT, tid1, e_stack);
        assert(map111 >= 0);


        did = H5Dcreate_ff(gid, "D1", dtid, sid, H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(did >= 0);

        ret = H5Tcommit_ff(file_id, "DT1", dtid, H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(ret == 0);

        map = H5Mcreate_ff(file_id, "MAP1", H5T_STD_I32LE, H5T_STD_I32LE, 
                           H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT, tid1, e_stack);
        assert(map >= 0);

        anon_did = H5Dcreate_anon_ff(file_id, dtid, sid, H5P_DEFAULT, H5P_DEFAULT, 
                                     tid1, e_stack);
        assert(anon_did > 0);

        ret = H5Dclose_ff(anon_did, e_stack);
        assert(0 == ret);

        ret = H5TRfinish(tid1, H5P_DEFAULT, &rid_temp, e_stack);
        assert(0 == ret);

        /* wait on all requests and print completion status */
        H5ESget_count(e_stack, &num_events);
        H5ESwait_all(e_stack, &status);
        H5ESclear(e_stack);
        printf("%d events in event stack. Completion status = %d\n", num_events, status);
        assert(status == H5ES_STATUS_SUCCEED);

        /* Close transaction object. Local op */
        ret = H5TRclose(tid1);
        assert(0 == ret);

        /* create transaction object */
        tid2 = H5TRcreate(file_id, rid_temp, (uint64_t)3);
        assert(tid2);
        ret = H5TRstart(tid2, H5P_DEFAULT, e_stack);
        assert(0 == ret);

        ret = H5Oset_comment_ff(gid, "Testing Object Comment", tid2, e_stack);
        assert(ret == 0);

        ret = H5TRfinish(tid2, H5P_DEFAULT, &rid2, e_stack);
        assert(0 == ret);

        assert(H5Gclose_ff(gid, e_stack) == 0);
        assert(H5Mclose_ff(map, e_stack) == 0);
        assert(H5Tclose_ff(dtid, e_stack) == 0);
        assert(H5Dclose_ff(did, e_stack) == 0);

        assert(H5Gclose_ff(gid2, e_stack) == 0);
        assert(H5Gclose_ff(gid3, e_stack) == 0);
        assert(H5Gclose_ff(gid11, e_stack) == 0);
        assert(H5Dclose_ff(did11, e_stack) == 0);
        assert(H5Dclose_ff(did21, e_stack) == 0);
        assert(H5Dclose_ff(did31, e_stack) == 0);
        assert(H5Mclose_ff(map111, e_stack) == 0);

        /* release container version 2. This is async. */
        ret = H5RCrelease(rid_temp, e_stack);
        assert(0 == ret);
        ret = H5RCclose(rid_temp);
        assert(0 == ret);

        ret = H5RCrelease(rid1, e_stack);
        assert(0 == ret);

        /* wait on all requests and print completion status */
        H5ESget_count(e_stack, &num_events);
        H5ESwait_all(e_stack, &status);
        H5ESclear(e_stack);
        printf("%d events in event stack. Completion status = %d\n", num_events, status);
        assert(status == H5ES_STATUS_SUCCEED);

        /* Close transaction object. Local op */
        ret = H5TRclose(tid2);
        assert(0 == ret);

        version = 3;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /* Process 0 tells other procs that container version 3 is acquired */
    MPI_Bcast(&version, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    assert(3 == version);

    /* other processes just create a read context object; no need to
       acquire it */
    if(0 != my_rank) {
        rid2 = H5RCcreate(file_id, version);
        assert(rid2 > 0);
    }

    gid = H5Oopen_ff(file_id, "G1", H5P_DEFAULT, rid2);
    assert(gid);
    dtid = H5Oopen_ff(file_id, "DT1", H5P_DEFAULT, rid2);
    assert(dtid);
    did = H5Oopen_ff(gid,"D1", H5P_DEFAULT, rid2);
    assert(did);
    map = H5Oopen_ff(file_id,"MAP1", H5P_DEFAULT, rid2);
    assert(did);

    {
        ssize_t ret_size = 0;
        char *comment = NULL;
        H5O_ff_info_t oinfo;

        ret = H5Oget_comment_ff(gid, NULL, 0, &ret_size, rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);

        fprintf(stderr, "size of comment is %d\n", ret_size);

        comment = malloc((size_t)ret_size + 1);

        ret = H5Oget_comment_ff(gid, comment, (size_t)ret_size + 1, 
                                &ret_size, rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);

        fprintf(stderr, "size of comment is %d Comment is %s\n", ret_size, comment);
        free(comment);

        ret = H5Oget_info_ff(file_id, &oinfo, rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);
        assert(H5O_TYPE_GROUP == oinfo.type);
        fprintf(stderr, 
                "get_info: Group with OID: %"PRIx64", num attrs = %zu, ref count = %u\n", 
                oinfo.addr, oinfo.num_attrs, oinfo.rc);

        ret = H5Oget_info_by_name_ff(file_id, ".", &oinfo, H5P_DEFAULT, rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);
        assert(H5O_TYPE_GROUP == oinfo.type);
        fprintf(stderr, 
                "get_info: Group with OID: %"PRIx64", num attrs = %zu, ref count = %u\n", 
                oinfo.addr, oinfo.num_attrs, oinfo.rc);

        ret = H5Oget_info_ff(gid, &oinfo, rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);
        assert(H5O_TYPE_GROUP == oinfo.type);
        fprintf(stderr, 
                "get_info: Group with OID: %"PRIx64", num attrs = %zu, ref count = %u\n", 
                oinfo.addr, oinfo.num_attrs, oinfo.rc);

        ret = H5Oget_info_ff(did, &oinfo, rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);
        assert(H5O_TYPE_DATASET == oinfo.type);
        fprintf(stderr, 
                "get_info: Dataset with OID: %"PRIx64", num attrs = %zu, ref count = %u\n", 
                oinfo.addr, oinfo.num_attrs, oinfo.rc);

        ret = H5Oget_info_ff(dtid, &oinfo, rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);
        assert(H5O_TYPE_NAMED_DATATYPE == oinfo.type);
        fprintf(stderr, 
                "get_info: Named Datatype with OID: %"PRIx64", num attrs = %zu, ref count = %u\n", 
                oinfo.addr, oinfo.num_attrs, oinfo.rc);

        ret = H5Oget_info_ff(map, &oinfo, rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);
        assert(H5O_TYPE_MAP == oinfo.type);
        fprintf(stderr, 
                "get_info: MAP with OID: %"PRIx64", num attrs = %zu, ref count = %u\n", 
                oinfo.addr, oinfo.num_attrs, oinfo.rc);
    }

    /* check if an object exists. This is asynchronous, so checking
       the value should be done after the wait */
    ret = H5Oexists_by_name_ff(file_id, "G1", &exists, H5P_DEFAULT, rid2, e_stack);
    assert(ret == 0);

    printf("Ovisit on /G1: \n");
    ret = H5Ovisit_ff(gid, 0, H5_ITER_NATIVE, visit_cb, NULL, rid2, H5_EVENT_STACK_NULL);
    assert(ret == 0);

    printf("Ovisit on /DT1: \n");
    ret = H5Ovisit_ff(dtid, 0, H5_ITER_NATIVE, visit_cb, NULL, rid2, H5_EVENT_STACK_NULL);
    assert(ret == 0);

    printf("Ovisit_by_name  on /G2: \n");
    ret = H5Ovisit_by_name_ff(file_id, "G2", 0, H5_ITER_NATIVE, visit_cb, NULL, H5P_DEFAULT, 
                              rid2, H5_EVENT_STACK_NULL);
    assert(ret == 0);

    if(my_rank == 0) {
        /* release container version 3. This is async. */
        ret = H5RCrelease(rid2, e_stack);
        assert(0 == ret);

        ret = H5RCclose(rid1);
        assert(0 == ret);
    }

    assert(H5Oclose_ff(did, e_stack) == 0);
    assert(H5Oclose_ff(gid, e_stack) == 0);
    assert(H5Oclose_ff(dtid, e_stack) == 0);
    assert(H5Oclose_ff(map, e_stack) == 0);

    ret = H5RCclose(rid2);
    assert(0 == ret);

    /* wait on all requests and print completion status */
    H5ESget_count(e_stack, &num_events);
    H5ESwait_all(e_stack, &status);
    H5ESclear(e_stack);
    printf("%d events in event stack. Completion status = %d\n", num_events, status);
    assert(status == H5ES_STATUS_SUCCEED);

    /* closing the container also acts as a wait all on all pending requests 
       on the container. */
    assert(H5Fclose_ff(file_id, 1, H5_EVENT_STACK_NULL) == 0);

    assert(exists);

    H5Sclose(sid);
    H5Pclose(fapl_id);
    H5ESclose(e_stack);

    /* This finalizes the EFF stack. ships a terminate and IOD finalize to the server 
       and shutsdown the FS server (when all clients send the terminate request) 
       and client */
    MPI_Barrier(MPI_COMM_WORLD);
    ret = EFF_finalize();
    assert(ret >= 0);

    MPI_Finalize();
    return 0;
}
