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

/*
 * This file contains private information about the H5VL iod server module
 */
#ifndef _H5VLiod_server_H
#define _H5VLiod_server_H

#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5Ppublic.h"
#include "H5Spublic.h"
#include "H5VLiod_common.h"

#ifdef H5_HAVE_EFF

/* Key names for Metadata stored in KV objects */
#define H5VL_IOD_KEY_SOFT_LINK       "soft_link_value"
#define H5VL_IOD_KEY_DTYPE_SIZE      "serialized_size"
#define H5VL_IOD_KEY_KV_IDS_INDEX    "kv_ids_index"
#define H5VL_IOD_KEY_ARRAY_IDS_INDEX "array_ids_index"
#define H5VL_IOD_KEY_BLOB_IDS_INDEX  "blob_ids_index"
#define H5VL_IOD_KEY_OBJ_COMMENT     "object_comment"
#define H5VL_IOD_KEY_OBJ_CPL         "object_create_plist"
#define H5VL_IOD_KEY_OBJ_LINK_COUNT  "object_link_count"
#define H5VL_IOD_KEY_OBJ_TYPE        "object_type"
#define H5VL_IOD_KEY_OBJ_DATATYPE    "object_datatype"
#define H5VL_IOD_KEY_OBJ_DATASPACE   "object_dataspace"
#define H5VL_IOD_KEY_MAP_KEY_TYPE    "map_keytype"
#define H5VL_IOD_KEY_MAP_VALUE_TYPE  "map_valtype"
#define H5VL_IOD_IDX_PLUGIN_ID       "index_plugin_id"
#define H5VL_IOD_IDX_PLUGIN_MD       "index_plugin_metadata"

#define ERFILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define HGOTO_ERROR_FF(ret_val, string) {			               \
    fprintf(stderr, "%s:%d\t%d (%s). --- %s\n", ERFILE, __LINE__, ret_val, strerror(-ret_val), string); \
    HGOTO_DONE_FF(ret_val)						       \
}

#define HDONE_ERROR_FF(ret_val, string) {			               \
    fprintf(stderr, "%s:%d\t%d (%s). --- %s\n", ERFILE, __LINE__, ret_val, strerror(-ret_val), string); \
    ret_value = ret_val;                                                       \
}

#define HGOTO_DONE_FF(ret_val) {ret_value = ret_val; goto done;}

/* the IOD scratch pad type */
typedef iod_obj_id_t scratch_pad[4];

/* Enum for metadata types stored in MD KV for HDF5->IOD objects */
typedef enum H5VL_iod_metadata_t {
    H5VL_IOD_PLIST,             /*type ID for property lists     	    */
    H5VL_IOD_LINK_COUNT,        /*type ID for link count                    */
    H5VL_IOD_OBJECT_TYPE,       /*type ID for object type                   */
    H5VL_IOD_DATATYPE,          /*type ID for datatypes                     */
    H5VL_IOD_DATASPACE,         /*type ID for dataspaces                    */
    H5VL_IOD_LINK               /*type ID for links                         */
} H5VL_iod_metadata_t;

/* the AXE op data strucutre stored with every operation */
typedef struct op_data_t {
    void *input;
    void *output;
    AXE_task_t axe_id;
    hg_handle_t hg_handle;
} op_data_t;

/* the link value stored in KV stores */
typedef struct H5VL_iod_link_t {
    H5L_type_t link_type;    /* The type of the link (Hard & Soft only suppoted for now) */
    union {
        iod_obj_id_t iod_id;
        char *symbolic_name;
    } u;
    iod_obj_id_t iod_id;     /* The ID of the object the link points to */
} H5VL_iod_link_t;

/* User data for dataspace iteration to query elements. */
typedef struct {
    size_t num_elmts;
    hid_t query_id;
    hid_t space_query;
} H5VL__iod_get_query_data_t;

extern iod_obj_id_t ROOT_ID;
extern int num_ions_g;
extern int my_rank_g;
extern na_addr_t *server_addr_g;
extern char **server_loc_g;
extern hg_id_t H5VL_EFF_OPEN_CONTAINER;
extern hg_id_t H5VL_EFF_CLOSE_CONTAINER;
extern hg_id_t H5VL_EFF_ANALYSIS_FARM;
extern hg_id_t H5VL_EFF_ANALYSIS_FARM_TRANSFER;
extern na_addr_t PEER;

/* Define the operator function pointer for H5Diterate() */
typedef herr_t (*H5VL_operator_t)(iod_handle_t coh, iod_obj_id_t obj_id, iod_trans_id_t rtid,
                                  H5I_type_t obj_type, uint32_t cs_scope, void *operator_data);

/* Define the operator function pointer for server_iterate() */
typedef herr_t (*H5VL_iterate_op_t)(iod_handle_t coh, iod_obj_id_t obj_id, iod_trans_id_t rtid,
                                    H5I_type_t obj_type,  const char *link_name, const char *attr_name,
                                    uint32_t cs_scope, void *operator_data);

/* Define the operator function pointer for server_visit() */
typedef herr_t (*H5VL_visit_op_t)(iod_handle_t coh, iod_obj_id_t obj_id, iod_handle_t obj_oh,
                                  const char *path, uint32_t cs_scope, iod_trans_id_t rtid, void *op_data);

H5_DLL int H5VL_iod_server_analysis_farm(hg_handle_t handle);
H5_DLL int H5VL_iod_server_analysis_transfer(hg_handle_t handle);
H5_DLL int H5VL_iod_server_container_open(hg_handle_t handle);
H5_DLL int H5VL_iod_server_container_close(hg_handle_t handle);

H5_DLL void H5VL_iod_server_analysis_invoke_cb(AXE_engine_t axe_engine, 
                                               size_t num_n_parents, AXE_task_t n_parents[], 
                                               size_t num_s_parents, AXE_task_t s_parents[], 
                                               void *_op_data);
H5_DLL void H5VL_iod_server_analysis_farm_cb(AXE_engine_t axe_engine, 
                                             size_t num_n_parents, AXE_task_t n_parents[], 
                                             size_t num_s_parents, AXE_task_t s_parents[], 
                                             void *_op_data);
H5_DLL void H5VL_iod_server_analysis_transfer_cb(AXE_engine_t axe_engine,
                                                 size_t num_n_parents, AXE_task_t n_parents[],
                                                 size_t num_s_parents, AXE_task_t s_parents[],
                                                 void *_op_data);

H5_DLL void H5VL_iod_server_index_set_info_cb(AXE_engine_t axe_engine,
    size_t num_n_parents, AXE_task_t n_parents[], size_t num_s_parents,
    AXE_task_t s_parents[], void *_op_data);
H5_DLL void H5VL_iod_server_index_get_info_cb(AXE_engine_t axe_engine,
    size_t num_n_parents, AXE_task_t n_parents[], size_t num_s_parents,
    AXE_task_t s_parents[], void *_op_data);
H5_DLL void H5VL_iod_server_index_remove_info_cb(AXE_engine_t axe_engine,
    size_t num_n_parents, AXE_task_t n_parents[], size_t num_s_parents,
    AXE_task_t s_parents[], void *_op_data);

H5_DLL void H5VL_iod_server_file_create_cb(AXE_engine_t axe_engine, 
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_file_open_cb(AXE_engine_t axe_engine,  
                                         size_t num_n_parents, AXE_task_t n_parents[], 
                                         size_t num_s_parents, AXE_task_t s_parents[], 
                                         void *op_data);
H5_DLL void H5VL_iod_server_file_close_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_attr_create_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_attr_open_cb(AXE_engine_t axe_engine,  
                                         size_t num_n_parents, AXE_task_t n_parents[], 
                                         size_t num_s_parents, AXE_task_t s_parents[], 
                                         void *op_data);
H5_DLL void H5VL_iod_server_attr_read_cb(AXE_engine_t axe_engine,  
                                         size_t num_n_parents, AXE_task_t n_parents[], 
                                         size_t num_s_parents, AXE_task_t s_parents[], 
                                         void *op_data);
H5_DLL void H5VL_iod_server_attr_write_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_attr_exists_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_attr_iterate_cb(AXE_engine_t axe_engine, 
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_attr_rename_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_attr_remove_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_attr_close_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_group_create_cb(AXE_engine_t axe_engine,  
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_group_open_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_group_close_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_map_create_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_map_open_cb(AXE_engine_t axe_engine,  
                                        size_t num_n_parents, AXE_task_t n_parents[], 
                                        size_t num_s_parents, AXE_task_t s_parents[], 
                                        void *op_data);
H5_DLL void H5VL_iod_server_map_set_cb(AXE_engine_t axe_engine,  
                                       size_t num_n_parents, AXE_task_t n_parents[], 
                                       size_t num_s_parents, AXE_task_t s_parents[], 
                                       void *op_data);
H5_DLL void H5VL_iod_server_map_get_cb(AXE_engine_t axe_engine,  
                                       size_t num_n_parents, AXE_task_t n_parents[], 
                                       size_t num_s_parents, AXE_task_t s_parents[], 
                                       void *op_data);
H5_DLL void H5VL_iod_server_map_get_count_cb(AXE_engine_t axe_engine,  
                                             size_t num_n_parents, AXE_task_t n_parents[], 
                                             size_t num_s_parents, AXE_task_t s_parents[], 
                                             void *op_data);
H5_DLL void H5VL_iod_server_map_exists_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_map_delete_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_map_close_cb(AXE_engine_t axe_engine,  
                                         size_t num_n_parents, AXE_task_t n_parents[], 
                                         size_t num_s_parents, AXE_task_t s_parents[], 
                                         void *op_data);
H5_DLL void H5VL_iod_server_dset_create_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_dset_open_cb(AXE_engine_t axe_engine,  
                                         size_t num_n_parents, AXE_task_t n_parents[], 
                                         size_t num_s_parents, AXE_task_t s_parents[], 
                                         void *op_data);
H5_DLL void H5VL_iod_server_dset_read_cb(AXE_engine_t axe_engine,  
                                         size_t num_n_parents, AXE_task_t n_parents[], 
                                         size_t num_s_parents, AXE_task_t s_parents[], 
                                         void *op_data);
H5_DLL void H5VL_iod_server_dset_multi_read_cb(AXE_engine_t axe_engine,  
                                               size_t num_n_parents, AXE_task_t n_parents[], 
                                               size_t num_s_parents, AXE_task_t s_parents[], 
                                               void *op_data);
H5_DLL void H5VL_iod_server_dset_get_vl_size_cb(AXE_engine_t axe_engine,  
                                                size_t num_n_parents, AXE_task_t n_parents[], 
                                                size_t num_s_parents, AXE_task_t s_parents[], 
                                                void *op_data);
H5_DLL void H5VL_iod_server_dset_write_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_dset_multi_write_cb(AXE_engine_t axe_engine,  
                                                size_t num_n_parents, AXE_task_t n_parents[], 
                                                size_t num_s_parents, AXE_task_t s_parents[], 
                                                void *op_data);
H5_DLL void H5VL_iod_server_dset_set_extent_cb(AXE_engine_t axe_engine,  
                                               size_t num_n_parents, AXE_task_t n_parents[], 
                                               size_t num_s_parents, AXE_task_t s_parents[], 
                                               void *op_data);
H5_DLL void H5VL_iod_server_dset_close_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_dtype_commit_cb(AXE_engine_t axe_engine,  
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_dtype_open_cb(AXE_engine_t axe_engine,  
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_dtype_close_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_link_create_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_link_move_cb(AXE_engine_t axe_engine, 
                                         size_t num_n_parents, AXE_task_t n_parents[], 
                                         size_t num_s_parents, AXE_task_t s_parents[], 
                                         void *op_data);
H5_DLL void H5VL_iod_server_link_exists_cb(AXE_engine_t axe_engine, 
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_link_get_info_cb(AXE_engine_t axe_engine, 
                                             size_t num_n_parents, AXE_task_t n_parents[], 
                                             size_t num_s_parents, AXE_task_t s_parents[], 
                                             void *op_data);
H5_DLL void H5VL_iod_server_link_get_val_cb(AXE_engine_t axe_engine, 
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_link_remove_cb(AXE_engine_t axe_engine, 
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_link_iterate_cb(AXE_engine_t axe_engine, 
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);

H5_DLL void H5VL_iod_server_object_open_by_token_cb(AXE_engine_t axe_engine, 
                                                    size_t num_n_parents, AXE_task_t n_parents[], 
                                                    size_t num_s_parents, AXE_task_t s_parents[], 
                                                    void *_op_data);
H5_DLL void H5VL_iod_server_object_open_cb(AXE_engine_t axe_engine,  
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_object_open_by_addr_cb(AXE_engine_t axe_engine,  
                                                   size_t num_n_parents, AXE_task_t n_parents[], 
                                                   size_t num_s_parents, AXE_task_t s_parents[], 
                                                   void *op_data);
H5_DLL void H5VL_iod_server_object_copy_cb(AXE_engine_t axe_engine, 
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_object_exists_cb(AXE_engine_t axe_engine, 
                                             size_t num_n_parents, AXE_task_t n_parents[], 
                                             size_t num_s_parents, AXE_task_t s_parents[], 
                                             void *op_data);
H5_DLL void H5VL_iod_server_object_visit_cb(AXE_engine_t axe_engine, 
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_object_set_comment_cb(AXE_engine_t axe_engine, 
                                                  size_t num_n_parents, AXE_task_t n_parents[], 
                                                  size_t num_s_parents, AXE_task_t s_parents[], 
                                                  void *op_data);
H5_DLL void H5VL_iod_server_object_get_comment_cb(AXE_engine_t axe_engine, 
                                                  size_t num_n_parents, AXE_task_t n_parents[], 
                                                  size_t num_s_parents, AXE_task_t s_parents[], 
                                                  void *op_data);
H5_DLL void H5VL_iod_server_object_get_info_cb(AXE_engine_t axe_engine, 
                                               size_t num_n_parents, AXE_task_t n_parents[], 
                                               size_t num_s_parents, AXE_task_t s_parents[], 
                                               void *op_data);

H5_DLL void H5VL_iod_server_rcxt_acquire_cb(AXE_engine_t axe_engine, 
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_rcxt_release_cb(AXE_engine_t axe_engine, 
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_rcxt_persist_cb(AXE_engine_t axe_engine, 
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_rcxt_snapshot_cb(AXE_engine_t axe_engine, 
                                             size_t num_n_parents, AXE_task_t n_parents[], 
                                             size_t num_s_parents, AXE_task_t s_parents[], 
                                             void *op_data);
H5_DLL void H5VL_iod_server_trans_start_cb(AXE_engine_t axe_engine, 
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_trans_finish_cb(AXE_engine_t axe_engine, 
                                            size_t num_n_parents, AXE_task_t n_parents[], 
                                            size_t num_s_parents, AXE_task_t s_parents[], 
                                            void *op_data);
H5_DLL void H5VL_iod_server_trans_set_dependency_cb(AXE_engine_t axe_engine, 
                                                    size_t num_n_parents, AXE_task_t n_parents[], 
                                                    size_t num_s_parents, AXE_task_t s_parents[], 
                                                    void *op_data);
H5_DLL void H5VL_iod_server_trans_skip_cb(AXE_engine_t axe_engine, 
                                          size_t num_n_parents, AXE_task_t n_parents[], 
                                          size_t num_s_parents, AXE_task_t s_parents[], 
                                          void *op_data);
H5_DLL void H5VL_iod_server_trans_abort_cb(AXE_engine_t axe_engine, 
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);
H5_DLL void H5VL_iod_server_prefetch_cb(AXE_engine_t axe_engine, 
                                        size_t num_n_parents, AXE_task_t n_parents[], 
                                        size_t num_s_parents, AXE_task_t s_parents[], 
                                        void *op_data);
H5_DLL void H5VL_iod_server_evict_cb(AXE_engine_t axe_engine, 
                                     size_t num_n_parents, AXE_task_t n_parents[], 
                                     size_t num_s_parents, AXE_task_t s_parents[], 
                                     void *op_data);
H5_DLL void H5VL_iod_server_view_create_cb(AXE_engine_t axe_engine, 
                                           size_t num_n_parents, AXE_task_t n_parents[], 
                                           size_t num_s_parents, AXE_task_t s_parents[], 
                                           void *op_data);

/* Helper routines used several times in different places */
H5_DLL herr_t H5VL_iod_server_traverse(iod_handle_t coh, iod_obj_id_t loc_id, iod_handles_t loc_handle, 
                                       const char *path, iod_trans_id_t wtid, iod_trans_id_t rtid, 
                                       hbool_t create_interm_grps, uint32_t cs_scope,
                                       char **last_comp, iod_obj_id_t *iod_id, iod_handles_t *iod_oh);
H5_DLL herr_t H5VL_iod_server_open_path(iod_handle_t coh, iod_obj_id_t loc_id, 
                                        iod_handles_t loc_handle, const char *path,
                                        iod_trans_id_t tid, uint32_t cs_scope, 
                                        /*out*/iod_obj_id_t *iod_id, /*out*/iod_handles_t *iod_oh);
H5_DLL herr_t H5VL_iod_get_file_desc(hid_t space_id, hssize_t *count, iod_hyperslab_t *hslabs);
H5_DLL herr_t H5VL_iod_insert_plist(iod_handle_t oh, iod_trans_id_t tid, hid_t plist_id,
                                    uint32_t cs_scope, iod_hint_list_t *hints, iod_event_t *event);
H5_DLL herr_t H5VL_iod_insert_link_count(iod_handle_t oh, iod_trans_id_t tid, uint64_t count,
                                         uint32_t cs_scope, iod_hint_list_t *hints, iod_event_t *event);
H5_DLL herr_t H5VL_iod_insert_object_type(iod_handle_t oh, iod_trans_id_t tid, H5I_type_t obj_type,
                                          uint32_t cs_scope, iod_hint_list_t *hints, iod_event_t *event);
H5_DLL herr_t H5VL_iod_insert_datatype(iod_handle_t oh, iod_trans_id_t tid, hid_t type_id,
                                       uint32_t cs_scope, iod_hint_list_t *hints, iod_event_t *event);
H5_DLL herr_t  H5VL_iod_insert_datatype_with_key(iod_handle_t oh, iod_trans_id_t tid, hid_t type_id, 
                                                 const char *key, uint32_t cs_scope,
                                                 iod_hint_list_t *hints, iod_event_t *event);
H5_DLL herr_t H5VL_iod_insert_dataspace(iod_handle_t oh, iod_trans_id_t tid, hid_t space_id,
                                        uint32_t cs_scope, iod_hint_list_t *hints, iod_event_t *event);
H5_DLL herr_t H5VL_iod_insert_new_link(iod_handle_t oh, iod_trans_id_t tid, const char *link_name,
                                       H5L_type_t link_type, const void *link_val, 
                                       uint32_t cs_scope, iod_hint_list_t *hints, 
                                       iod_event_t *event);
H5_DLL herr_t H5VL_iod_get_metadata(iod_handle_t oh, iod_trans_id_t tid, H5VL_iod_metadata_t md_type,
                                    const char *key, uint32_t cs_scope, iod_event_t *event, void *ret);
H5_DLL herr_t H5VL__iod_server_adjust_buffer(hid_t from_type_id, hid_t to_type_id, size_t nelmts, 
                                             hid_t dxpl_id, na_bool_t is_coresident,
                                             size_t size, void **buf, 
                                             hbool_t *is_vl_data, size_t *_buf_size);
H5_DLL herr_t H5VL__iod_server_type_is_vl(hid_t type_id, hbool_t *is_vl_data);
H5_DLL herr_t H5VL_iod_verify_scratch_pad(scratch_pad *sp, iod_checksum_t iod_cs);
H5_DLL herr_t H5VL_iod_verify_kv_pair(void *key, iod_size_t key_size, void *value, iod_size_t val_size, 
                                      iod_checksum_t *iod_cs);
H5_DLL herr_t H5VL__iod_server_final_io(iod_handle_t coh, iod_handle_t iod_oh, hid_t space_id, size_t elmt_size,
                                        hbool_t write_op, void *buf, size_t buf_size, 
                                        iod_checksum_t cs, uint32_t cs_scope, iod_trans_id_t tid);

H5_DLL herr_t H5VL_iod_server_iterate(iod_handle_t coh, iod_obj_id_t obj_id, iod_trans_id_t rtid,
                                       H5I_type_t obj_type, 
                                       const char *link_name, const char *attr_name, 
                                       uint32_t cs_scope, H5VL_iterate_op_t op, void *op_data);

H5_DLL herr_t H5VL_iod_server_visit(iod_handle_t coh, iod_obj_id_t obj_id, iod_handle_t obj_oh, 
                                    const char *path, uint32_t cs_scope, iod_trans_id_t rtid, 
                                    H5VL_visit_op_t op, void *op_data);
H5_DLL H5I_type_t H5VL__iod_get_h5_obj_type(iod_obj_id_t oid, iod_handle_t coh, 
                                            iod_trans_id_t rtid, uint32_t cs_scope);
H5_DLL herr_t H5VL__iod_get_query_data_cb(void *elem, hid_t type_id, unsigned ndim, 
                                          const hsize_t *point, void *_udata);

H5_DLL void print_iod_obj_map(iod_obj_map_t *obj_map);

#endif /* H5_HAVE_EFF */
#endif /* _H5VLiod_server_H */
