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

#include "H5VLiod_server.h"

#ifdef H5_HAVE_EFF

/*
 * Programmer:  Mohamad Chaarawi <chaarawi@hdfgroup.gov>
 *              March, 2014
 *
 * Purpose:	The IOD plugin server side indexing routines.
 */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_index_set_info_cb
 *
 * Purpose:	Stored index information of the dataset.
 *
 * Return:	Success:	SUCCEED 
 *		Failure:	Negative
 *
 * Programmer:  Mohamad Chaarawi
 *              March, 2014
 *
 *-------------------------------------------------------------------------
 */
void
H5VL_iod_server_index_set_info_cb(AXE_engine_t H5_ATTR_UNUSED axe_engine,
                               size_t H5_ATTR_UNUSED num_n_parents, AXE_task_t H5_ATTR_UNUSED n_parents[], 
                               size_t H5_ATTR_UNUSED num_s_parents, AXE_task_t H5_ATTR_UNUSED s_parents[], 
                               void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    idx_set_info_in_t *input = (idx_set_info_in_t *)op_data->input;
    iod_handle_t coh = input->coh; /* container handle */
    iod_obj_id_t mdkv_id = input->mdkv_id; /* The ID of the metadata KV to be created */
    iod_trans_id_t wtid = input->trans_num;
    uint32_t cs_scope = input->cs_scope;
    iod_handle_t mdkv_oh;
    iod_kv_t kv;
    iod_ret_t ret;
    herr_t ret_value = SUCCEED;

#if H5_EFF_DEBUG 
    fprintf(stderr, "Start dataset set_index_info\n");
#endif

    /* Open Metadata KV object for write */
    ret = iod_obj_open_write(coh, mdkv_id, wtid, NULL, &mdkv_oh, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't open MDKV object");

    kv.key = H5VL_IOD_IDX_PLUGIN_ID;
    kv.key_len = (iod_size_t)strlen(H5VL_IOD_IDX_PLUGIN_ID) + 1;
    kv.value = &input->idx_plugin_id;
    kv.value_len = (iod_size_t)sizeof(uint32_t);

    if(cs_scope & H5_CHECKSUM_IOD) {
        iod_checksum_t cs[2];

        cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
        cs[1] = H5_checksum_crc64(kv.value, kv.value_len);
        ret = iod_kv_set(mdkv_oh, wtid, NULL, &kv, cs, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
    }
    else {
        ret = iod_kv_set(mdkv_oh, wtid, NULL, &kv, NULL, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
    }

    kv.key = H5VL_IOD_IDX_PLUGIN_MD;
    kv.key_len = (iod_size_t)strlen(H5VL_IOD_IDX_PLUGIN_MD) + 1;
    kv.value = input->idx_metadata.buf;
    kv.value_len = (iod_size_t)input->idx_metadata.buf_size;

    if(cs_scope & H5_CHECKSUM_IOD) {
        iod_checksum_t cs[2];

        cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
        cs[1] = H5_checksum_crc64(kv.value, kv.value_len);
        ret = iod_kv_set(mdkv_oh, wtid, NULL, &kv, cs, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
    }
    else {
        ret = iod_kv_set(mdkv_oh, wtid, NULL, &kv, NULL, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
    }

#if H5_EFF_DEBUG
    fprintf(stderr, "Done with dataset set_index_info, sending response to client\n");
#endif

done:
    if(HG_SUCCESS != HG_Handler_start_output(op_data->hg_handle, &ret_value))
        HDONE_ERROR_FF(FAIL, "can't send result of write to client");

    /* close the Metadata KV object */
    ret = iod_obj_close(mdkv_oh, NULL, NULL);
    if(ret < 0)
        HDONE_ERROR_FF(ret, "can't close object");

    HG_Handler_free_input(op_data->hg_handle, input);
    HG_Handler_free(op_data->hg_handle);
    input = (idx_set_info_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

} /* end H5VL_iod_server_index_set_info_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_index_get_info_cb
 *
 * Purpose:	Stored index information of the dataset.
 *
 * Return:	Success:	SUCCEED 
 *		Failure:	Negative
 *
 * Programmer:  Mohamad Chaarawi
 *              March, 2014
 *
 *-------------------------------------------------------------------------
 */
void
H5VL_iod_server_index_get_info_cb(AXE_engine_t H5_ATTR_UNUSED axe_engine,
                               size_t H5_ATTR_UNUSED num_n_parents, AXE_task_t H5_ATTR_UNUSED n_parents[], 
                               size_t H5_ATTR_UNUSED num_s_parents, AXE_task_t H5_ATTR_UNUSED s_parents[], 
                               void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    idx_get_info_in_t *input = (idx_get_info_in_t *)op_data->input;
    idx_get_info_out_t output;
    iod_handle_t coh = input->coh; /* container handle */
    iod_obj_id_t mdkv_id = input->mdkv_id; /* The ID of the metadata KV to be created */
    iod_trans_id_t rtid = input->rcxt_num;
    uint32_t cs_scope = input->cs_scope;
    iod_handle_t mdkv_oh;
    iod_size_t key_size = 0;
    iod_size_t val_size = 0;
    iod_checksum_t *iod_cs = NULL;
    char *key = NULL;
    iod_ret_t ret;
    herr_t ret_value = SUCCEED;

#if H5_EFF_DEBUG 
    fprintf(stderr, "Start dataset get_index_info\n");
#endif

    /* Open Metadata KV object for write */
    ret = iod_obj_open_read(coh, mdkv_id, rtid, NULL, &mdkv_oh, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't open MDKV object");

    if(cs_scope & H5_CHECKSUM_IOD) {
        iod_cs = (iod_checksum_t *)malloc(sizeof(iod_checksum_t) * 2);
    }

    key = H5VL_IOD_IDX_PLUGIN_ID;
    key_size = strlen(key) + 1;
    val_size = sizeof(uint32_t);

    if((ret = iod_kv_get_value(mdkv_oh, rtid, key, key_size, &output.idx_plugin_id, 
                               &val_size, iod_cs, NULL)) < 0) {
        if(ret == -ENOKEY) {
            fprintf(stderr, "no index to retrieve\n");

            output.ret = ret_value;
            output.idx_count = 0;
            output.idx_plugin_id = 0;
            output.idx_metadata.buf_size = 0;
            output.idx_metadata.buf = NULL;
            HG_Handler_start_output(op_data->hg_handle, &output);
            HGOTO_DONE(SUCCEED);
        }
        HGOTO_ERROR_FF(ret, "lookup failed");
    }
    if(cs_scope & H5_CHECKSUM_IOD) {
        iod_checksum_t cs[2];

        cs[0] = H5_checksum_crc64(key, key_size);
        cs[1] = H5_checksum_crc64(&output.idx_plugin_id, val_size);

        if(iod_cs[0] != cs[0] && iod_cs[1] != cs[1])
            HGOTO_ERROR_FF(FAIL, "Corruption detected when reading metadata from IOD");
    }


    key = H5VL_IOD_IDX_PLUGIN_MD;
    key_size = strlen(key) + 1;
    val_size = 0;

    ret = iod_kv_get_value(mdkv_oh, rtid, key, key_size, NULL, 
                           &val_size, iod_cs, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "lookup failed");

    output.idx_metadata.buf_size = val_size;
    output.idx_metadata.buf = malloc(val_size);

    ret = iod_kv_get_value(mdkv_oh, rtid, key, key_size, (char *)output.idx_metadata.buf, 
                           &val_size, iod_cs, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "lookup failed");

    if(cs_scope & H5_CHECKSUM_IOD) {
        iod_checksum_t cs[2];

        cs[0] = H5_checksum_crc64(key, key_size);
        cs[1] = H5_checksum_crc64(output.idx_metadata.buf, val_size);

        if(iod_cs[0] != cs[0] && iod_cs[1] != cs[1])
            HGOTO_ERROR_FF(FAIL, "Corruption detected when reading metadata from IOD");
    }

    output.ret = ret_value;
    /* MSC for now, idx_count is always 1 */
    output.idx_count = 1;
    fprintf(stderr, "Found index, index count is %d!\n", output.idx_count);
    printf("Get index info ret is: %d\n", output.ret);
    printf("Index count is: %d\n", output.idx_count);
    printf("Plugin ID is: %d\n", output.idx_plugin_id);

#if H5_EFF_DEBUG
    fprintf(stderr, "Done with dataset get_index_info, sending response to client\n");
#endif

    HG_Handler_start_output(op_data->hg_handle, &output);

done:
    if(ret_value < 0) {
        fprintf(stderr, "INDEX get info FAILED\n");
        output.ret = ret_value;
        output.idx_plugin_id = 0;
        output.idx_metadata.buf_size = 0;
        if(output.idx_metadata.buf)
            free(output.idx_metadata.buf);
        output.idx_metadata.buf = NULL;
        HG_Handler_start_output(op_data->hg_handle, &output);
    }

    /* close the Metadata KV object */
    ret = iod_obj_close(mdkv_oh, NULL, NULL);
    if(ret < 0)
        HDONE_ERROR_FF(ret, "can't close object");

    HG_Handler_free_input(op_data->hg_handle, input);
    HG_Handler_free(op_data->hg_handle);
    input = (idx_get_info_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

} /* end H5VL_iod_server_index_get_info_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_index_remove_info_cb
 *
 * Purpose:	Stored index information of the dataset.
 *
 * Return:	Success:	SUCCEED 
 *		Failure:	Negative
 *
 * Programmer:  Mohamad Chaarawi
 *              March, 2014
 *
 *-------------------------------------------------------------------------
 */
void
H5VL_iod_server_index_remove_info_cb(AXE_engine_t H5_ATTR_UNUSED axe_engine,
                               size_t H5_ATTR_UNUSED num_n_parents, AXE_task_t H5_ATTR_UNUSED n_parents[], 
                               size_t H5_ATTR_UNUSED num_s_parents, AXE_task_t H5_ATTR_UNUSED s_parents[], 
                               void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    idx_rm_info_in_t *input = (idx_rm_info_in_t *)op_data->input;
    iod_handle_t coh = input->coh; /* container handle */
    iod_obj_id_t mdkv_id = input->mdkv_id; /* The ID of the metadata KV to be created */
    iod_trans_id_t wtid = input->trans_num;
    //uint32_t cs_scope = input->cs_scope;
    iod_handle_t mdkv_oh;
    iod_kv_params_t kvs;
    iod_kv_t kv;
    iod_ret_t ret;
    herr_t ret_value = SUCCEED;

#if H5_EFF_DEBUG 
    fprintf(stderr, "Start dataset rm_index_info\n");
#endif

    /* Open Metadata KV object for write */
    ret = iod_obj_open_write(coh, mdkv_id, wtid, NULL, &mdkv_oh, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't open MDKV object");

    kv.key = H5VL_IOD_IDX_PLUGIN_ID;
    kv.key_len = (iod_size_t)strlen(H5VL_IOD_IDX_PLUGIN_ID) + 1;
    kvs.kv = &kv;
    kvs.cs = NULL;
    kvs.ret = &ret;
    ret = iod_kv_unlink_keys(mdkv_oh, wtid, NULL, 1, &kvs, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "Unable to unlink KV pair");

    kv.key = H5VL_IOD_IDX_PLUGIN_MD;
    kv.key_len = (iod_size_t)strlen(H5VL_IOD_IDX_PLUGIN_MD) + 1;
    kvs.kv = &kv;
    kvs.cs = NULL;
    kvs.ret = &ret;
    ret = iod_kv_unlink_keys(mdkv_oh, wtid, NULL, 1, &kvs, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "Unable to unlink KV pair");

#if H5_EFF_DEBUG
    fprintf(stderr, "Done with dataset rm_index_info, sending response to client\n");
#endif

done:
    if(HG_SUCCESS != HG_Handler_start_output(op_data->hg_handle, &ret_value))
        HDONE_ERROR_FF(FAIL, "can't send result of write to client");

    /* close the Metadata KV object */
    ret = iod_obj_close(mdkv_oh, NULL, NULL);
    if(ret < 0)
        HDONE_ERROR_FF(ret, "can't close object");

    HG_Handler_free_input(op_data->hg_handle, input);
    HG_Handler_free(op_data->hg_handle);
    input = (idx_rm_info_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

} /* end H5VL_iod_server_index_remove_info_cb() */

#endif /* H5_HAVE_EFF */
