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
 * This file contains private information about the H5VL module
 */
#ifndef _H5VLiod_common_H
#define _H5VLiod_common_H

#include "H5public.h"
#include "H5FFpublic.h"
#include "H5VLpublic.h"
#include "H5VLiod.h"            /* Iod VOL plugin			*/

#ifdef H5_HAVE_EFF

#define NA_UNDEFINED NULL
#define IOD_OH_UNDEFINED ((uint64_t)0)//(pow(2.0,64.0) - 1)
#define IOD_ID_UNDEFINED ((uint64_t)(-1))//(pow(2.0,64.0) - 1)
#define IOD_COUNT_UNDEFINED ((uint64_t)(-1))//(pow(2.0,64.0) - 1)
#define H5_EFF_DEBUG 0

/* function shipper IDs for different routines */
extern hg_id_t H5VL_EFF_INIT_ID;
extern hg_id_t H5VL_EFF_FINALIZE_ID;
extern hg_id_t H5VL_ANALYSIS_INVOKE_ID;
extern hg_id_t H5VL_FILE_CREATE_ID;
extern hg_id_t H5VL_FILE_OPEN_ID;
extern hg_id_t H5VL_FILE_CLOSE_ID;
extern hg_id_t H5VL_ATTR_CREATE_ID;
extern hg_id_t H5VL_ATTR_OPEN_ID;
extern hg_id_t H5VL_ATTR_READ_ID;
extern hg_id_t H5VL_ATTR_WRITE_ID;
extern hg_id_t H5VL_ATTR_EXISTS_ID;
extern hg_id_t H5VL_ATTR_ITERATE_ID;
extern hg_id_t H5VL_ATTR_RENAME_ID;
extern hg_id_t H5VL_ATTR_REMOVE_ID;
extern hg_id_t H5VL_ATTR_CLOSE_ID;
extern hg_id_t H5VL_GROUP_CREATE_ID;
extern hg_id_t H5VL_GROUP_OPEN_ID;
extern hg_id_t H5VL_GROUP_CLOSE_ID;
extern hg_id_t H5VL_MAP_CREATE_ID;
extern hg_id_t H5VL_MAP_OPEN_ID;
extern hg_id_t H5VL_MAP_SET_ID;
extern hg_id_t H5VL_MAP_GET_ID;
extern hg_id_t H5VL_MAP_GET_COUNT_ID;
extern hg_id_t H5VL_MAP_EXISTS_ID;
//extern hg_id_t H5VL_MAP_ITERATE_ID;
extern hg_id_t H5VL_MAP_DELETE_ID;
extern hg_id_t H5VL_MAP_CLOSE_ID;
extern hg_id_t H5VL_DSET_CREATE_ID;
extern hg_id_t H5VL_DSET_OPEN_ID;
extern hg_id_t H5VL_DSET_READ_ID;
extern hg_id_t H5VL_DSET_MULTI_READ_ID;
extern hg_id_t H5VL_DSET_GET_VL_SIZE_ID;
extern hg_id_t H5VL_DSET_WRITE_ID;
extern hg_id_t H5VL_DSET_MULTI_WRITE_ID;
extern hg_id_t H5VL_DSET_SET_EXTENT_ID;
extern hg_id_t H5VL_DSET_CLOSE_ID;
extern hg_id_t H5VL_DTYPE_COMMIT_ID;
extern hg_id_t H5VL_DTYPE_OPEN_ID;
extern hg_id_t H5VL_DTYPE_CLOSE_ID;
extern hg_id_t H5VL_LINK_CREATE_ID;
extern hg_id_t H5VL_LINK_MOVE_ID;
extern hg_id_t H5VL_LINK_EXISTS_ID;
extern hg_id_t H5VL_LINK_GET_INFO_ID;
extern hg_id_t H5VL_LINK_GET_VAL_ID;
extern hg_id_t H5VL_LINK_REMOVE_ID;
extern hg_id_t H5VL_LINK_ITERATE_ID;
extern hg_id_t H5VL_OBJECT_OPEN_BY_TOKEN_ID;
extern hg_id_t H5VL_OBJECT_OPEN_ID;
extern hg_id_t H5VL_OBJECT_OPEN_BY_ADDR_ID;
//extern hg_id_t H5VL_OBJECT_COPY_ID;
extern hg_id_t H5VL_OBJECT_VISIT_ID;
extern hg_id_t H5VL_OBJECT_EXISTS_ID;
extern hg_id_t H5VL_OBJECT_SET_COMMENT_ID;
extern hg_id_t H5VL_OBJECT_GET_COMMENT_ID;
extern hg_id_t H5VL_OBJECT_GET_INFO_ID;
extern hg_id_t H5VL_RC_ACQUIRE_ID;
extern hg_id_t H5VL_RC_RELEASE_ID;
extern hg_id_t H5VL_RC_PERSIST_ID;
extern hg_id_t H5VL_RC_SNAPSHOT_ID;
extern hg_id_t H5VL_TR_START_ID;
extern hg_id_t H5VL_TR_FINISH_ID;
extern hg_id_t H5VL_TR_SET_DEPEND_ID;
extern hg_id_t H5VL_TR_SKIP_ID;
extern hg_id_t H5VL_TR_ABORT_ID;
extern hg_id_t H5VL_PREFETCH_ID;
extern hg_id_t H5VL_EVICT_ID;
extern hg_id_t H5VL_CANCEL_OP_ID;
extern hg_id_t H5VL_VIEW_CREATE_ID;
extern hg_id_t H5VL_IDX_SET_INFO_ID;
extern hg_id_t H5VL_IDX_GET_INFO_ID;
extern hg_id_t H5VL_IDX_RM_INFO_ID;

/* Structure for a span of fixed-length data in a type */
 typedef struct H5VL_iod_fl_span_t {
    size_t offset;          /* Offset of start of span in type */
    size_t size;            /* Size of span */
} H5VL_iod_fl_span_t;

/* Structure for a variable-length type in a type */
typedef struct H5VL_iod_vl_info_t {
    size_t offset;          /* Offset of vlen in type */
    struct H5VL_iod_type_info_t *base_type; /* Type info for vlen base type.  Set to NULL if this vlen is a vl string. */
} H5VL_iod_vl_info_t;

/* Structure containing information about a type to use during I/O */
typedef struct H5VL_iod_type_info_t {
    size_t size;            /* Size of one element of this type */
    size_t num_fl_spans;    /* Size of fl_spans array */
    H5VL_iod_fl_span_t *fl_spans; /* Array of spans of fixed-length data in type */
    size_t num_vls;         /* Size of vls array */
    H5VL_iod_vl_info_t *vls; /* Array of variable-length types in type */
    size_t rc;              /* Number of references to this struct */
    H5T_class_t type_class; /* Class of dataype */
    H5R_type_t ref_type;    /* Reference type if this a is a reference datatype */
} H5VL_iod_type_info_t;

typedef enum H5VL_iod_state_t {
    H5VL_IOD_PENDING,
    H5VL_IOD_COMPLETED,
    H5VL_IOD_CANCELLED
} H5VL_iod_state_t;

typedef struct dims_t {
    int rank;
    hsize_t *size;
} dims_t;

typedef struct coords_t {
    int rank;
    uint64_t *start_cell;
    uint64_t *end_cell;
} coords_t;

typedef struct name_t {
    size_t size;
    ssize_t *value_size;
    char *value;
} name_t;

typedef struct binary_buf_t {
    size_t buf_size;
    void *buf;
} binary_buf_t;

typedef struct value_t {
    size_t val_size;
    void *val;
} value_t;

typedef struct path_t {
    size_t path_size;
    char *path;
} path_t;

typedef struct axe_t {
    AXE_task_t axe_id;
    AXE_task_t start_range;
    size_t count;
    size_t num_parents;
    AXE_task_t *parent_axe_ids;
} axe_t;

typedef struct iod_handles_t {
    iod_handle_t rd_oh;
    iod_handle_t wr_oh;
} iod_handles_t;

typedef struct region_info_t {
    size_t count;
    binary_buf_t *tokens;
    hid_t *regions;
} region_info_t;

typedef struct attr_info_t {
    size_t count;
    binary_buf_t *tokens;
} attr_info_t;

typedef struct obj_info_t {
    size_t count;
    binary_buf_t *tokens;
} obj_info_t;

typedef struct obj_iterate_t {
    int32_t ret;
    uint32_t num_objs;
    const char **paths;
    H5O_ff_info_t *oinfos;
} obj_iterate_t;

typedef struct link_iterate_t {
    int32_t ret;
    uint32_t num_objs;
    const char **paths;
    H5L_ff_info_t *linfos;
} link_iterate_t;

typedef struct attr_iterate_t {
    int32_t ret;
    uint32_t num_attrs;
    const char **attr_names;
} attr_iterate_t;

typedef binary_buf_t loc_info_t;


H5_DLL int H5VL_iod_get_type_info(hid_t type_id, H5VL_iod_type_info_t *type_info);
H5_DLL void H5VL_iod_type_info_reset(H5VL_iod_type_info_t *type_info);
H5_DLL int H5VL_iod_create_segments_send(char *buf, H5VL_iod_type_info_t *type_info,
           size_t nelem, void ***addrs, size_t **sizes, size_t *num_segments,
           char **vl_lengths, size_t *vl_lengths_nused, void ***free_list,
           size_t *free_list_len);
H5_DLL int H5VL_iod_create_segments_recv(char *buf, H5VL_iod_type_info_t *type_info,
           size_t nelem, void ***addrs, size_t **sizes, size_t *num_segments,
           char *vl_lengths, size_t vl_lengths_nused, void ***free_list,
           size_t *free_list_len);
H5_DLL void H5VL_iod_free_list_free(void **free_list, size_t free_list_len);


H5_DLL herr_t EFF_setup_coresident(MPI_Comm comm, MPI_Info info);
H5_DLL herr_t EFF_terminate_coresident(void);
H5_DLL void EFF__mercury_register_callbacks(void);

H5_DLL int H5VL_iod_server_index_set_info(hg_handle_t handle);
H5_DLL int H5VL_iod_server_index_get_info(hg_handle_t handle);
H5_DLL int H5VL_iod_server_index_remove_info(hg_handle_t handle);

H5_DLL int H5VL_iod_server_eff_init(hg_handle_t handle);
H5_DLL int H5VL_iod_server_eff_finalize(hg_handle_t handle);
H5_DLL int H5VL_iod_server_analysis_invoke(hg_handle_t handle);
H5_DLL int H5VL_iod_server_file_create(hg_handle_t handle);
H5_DLL int H5VL_iod_server_file_open(hg_handle_t handle);
H5_DLL int H5VL_iod_server_file_close(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_create(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_open(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_read(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_write(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_exists(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_iterate(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_rename(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_remove(hg_handle_t handle);
H5_DLL int H5VL_iod_server_attr_close(hg_handle_t handle);
H5_DLL int H5VL_iod_server_group_create(hg_handle_t handle);
H5_DLL int H5VL_iod_server_group_open(hg_handle_t handle);
H5_DLL int H5VL_iod_server_group_close(hg_handle_t handle);
H5_DLL int H5VL_iod_server_map_create(hg_handle_t handle);
H5_DLL int H5VL_iod_server_map_open(hg_handle_t handle);
H5_DLL int H5VL_iod_server_map_set(hg_handle_t handle);
H5_DLL int H5VL_iod_server_map_get(hg_handle_t handle);
H5_DLL int H5VL_iod_server_map_get_count(hg_handle_t handle);
H5_DLL int H5VL_iod_server_map_exists(hg_handle_t handle);
H5_DLL int H5VL_iod_server_map_delete(hg_handle_t handle);
H5_DLL int H5VL_iod_server_map_close(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_create(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_open(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_read(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_multi_read(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_get_vl_size(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_write(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_multi_write(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_set_extent(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dset_close(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dtype_commit(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dtype_open(hg_handle_t handle);
H5_DLL int H5VL_iod_server_dtype_close(hg_handle_t handle);
H5_DLL int H5VL_iod_server_link_create(hg_handle_t handle);
H5_DLL int H5VL_iod_server_link_move(hg_handle_t handle);
H5_DLL int H5VL_iod_server_link_exists(hg_handle_t handle);
H5_DLL int H5VL_iod_server_link_get_info(hg_handle_t handle);
H5_DLL int H5VL_iod_server_link_get_val(hg_handle_t handle);
H5_DLL int H5VL_iod_server_link_remove(hg_handle_t handle);
H5_DLL int H5VL_iod_server_link_iterate(hg_handle_t handle);
H5_DLL int H5VL_iod_server_object_open_by_token(hg_handle_t handle);
H5_DLL int H5VL_iod_server_object_open(hg_handle_t handle);
H5_DLL int H5VL_iod_server_object_open_by_addr(hg_handle_t handle);
//H5_DLL int H5VL_iod_server_object_copy(hg_handle_t handle);
H5_DLL int H5VL_iod_server_object_visit(hg_handle_t handle);
H5_DLL int H5VL_iod_server_object_exists(hg_handle_t handle);
H5_DLL int H5VL_iod_server_object_set_comment(hg_handle_t handle);
H5_DLL int H5VL_iod_server_object_get_comment(hg_handle_t handle);
H5_DLL int H5VL_iod_server_object_get_info(hg_handle_t handle);
H5_DLL int H5VL_iod_server_rcxt_acquire(hg_handle_t handle);
H5_DLL int H5VL_iod_server_rcxt_release(hg_handle_t handle);
H5_DLL int H5VL_iod_server_rcxt_persist(hg_handle_t handle);
H5_DLL int H5VL_iod_server_rcxt_snapshot(hg_handle_t handle);
H5_DLL int H5VL_iod_server_trans_start(hg_handle_t handle);
H5_DLL int H5VL_iod_server_trans_finish(hg_handle_t handle);
H5_DLL int H5VL_iod_server_trans_set_dependency(hg_handle_t handle);
H5_DLL int H5VL_iod_server_trans_skip(hg_handle_t handle);
H5_DLL int H5VL_iod_server_trans_abort(hg_handle_t handle);
H5_DLL int H5VL_iod_server_prefetch(hg_handle_t handle);
H5_DLL int H5VL_iod_server_evict(hg_handle_t handle);
H5_DLL int H5VL_iod_server_view_create(hg_handle_t handle);
H5_DLL int H5VL_iod_server_cancel_op(hg_handle_t handle);

H5_DLL uint64_t H5_checksum_crc64(const void *buf, size_t buf_size);
H5_DLL uint64_t H5_checksum_crc64_segments(void **addrs, size_t *sizes, size_t count);
H5_DLL uint64_t H5_checksum_crc64_fragments(void **buf, size_t *buf_size, size_t count);

H5_DLL int hg_proc_ret_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_axe_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_size_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_hid_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_htri_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_hbool_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_iod_obj_id_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_iod_handle_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_iod_handles_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_dims_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_axe_ids_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_name_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_value_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_binary_buf_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_linfo_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_oinfo_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_coords_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_hrpl_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_region_info_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_obj_info_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_attr_info_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_obj_iterate_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_link_iterate_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_attr_iterate_t(hg_proc_t proc, void *data);
H5_DLL int hg_proc_dset_multi_io_list_t(hg_proc_t proc, void *data);

MERCURY_GEN_PROC(analysis_invoke_in_t, 
                 ((axe_t)(axe_info))
                 ((hid_t)(query_id)) 
                 ((hg_const_string_t)(file_name))
                 ((hg_const_string_t)(split_script))
                 ((hg_const_string_t)(combine_script))
                 ((hg_const_string_t)(integrate_script)))
MERCURY_GEN_PROC(analysis_invoke_out_t, ((int32_t)(ret)))
MERCURY_GEN_PROC(analysis_farm_in_t, 
                 ((iod_handle_t)(coh)) 
                 ((hid_t)(space_id))
                 ((hid_t)(type_id)) 
                 ((uint64_t)(rtid)) 
                 ((iod_obj_id_t)(obj_id))
                 ((size_t)(num_cells))
                 ((hg_const_string_t)(split_script)))
MERCURY_GEN_PROC(analysis_farm_out_t, 
                 ((int32_t)(ret)) 
                 ((uint64_t)(axe_id))
                 ((hid_t)(type_id))
                 ((size_t)(num_elmts)))
MERCURY_GEN_PROC(analysis_transfer_in_t,
                 ((hg_bulk_t)(bulk_handle))
                 ((uint64_t)(axe_id)))
MERCURY_GEN_PROC(analysis_transfer_out_t,
                 ((int32_t)(ret)))

MERCURY_GEN_PROC(eff_init_in_t, 
                 ((uint32_t)(proc_num)))

MERCURY_GEN_PROC(file_create_in_t, 
                 ((axe_t)(axe_info)) 
                 ((hg_const_string_t)(name)) 
                 ((uint32_t)(flags)) 
                 ((iod_obj_id_t)(root_id))
                 ((iod_obj_id_t)(mdkv_id)) 
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((iod_obj_id_t)(oidkv_id)) 
                 ((uint32_t)(num_peers))
                 ((hid_t)(fapl_id)) 
                 ((hid_t)(fcpl_id)))
MERCURY_GEN_PROC(file_create_out_t, 
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(root_oh)))
MERCURY_GEN_PROC(file_open_in_t, 
                 ((axe_t)(axe_info)) 
                 ((hg_const_string_t)(name)) 
                 ((uint32_t)(flags)) 
                 ((hbool_t)(acquire)) 
                 ((uint32_t)(num_peers))
                 ((hid_t)(fapl_id)))
MERCURY_GEN_PROC(file_open_out_t, 
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(root_oh))
                 ((uint64_t)(kv_oid_index)) 
                 ((uint64_t)(array_oid_index)) 
                 ((uint64_t)(blob_oid_index)) 
                 ((iod_obj_id_t)(root_id)) 
                 ((iod_obj_id_t)(mdkv_id)) 
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((iod_obj_id_t)(oidkv_id)) 
                 ((uint64_t)(c_version)) 
                 ((hid_t)(fcpl_id)))
MERCURY_GEN_PROC(file_close_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope)) 
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(root_oh))
                 ((iod_obj_id_t)(root_id)) 
                 ((hbool_t)(persist_on_close))
                 ((iod_obj_id_t)(max_kv_index)) 
                 ((iod_obj_id_t)(max_array_index)) 
                 ((iod_obj_id_t)(max_blob_index)))

MERCURY_GEN_PROC(attr_create_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num)) 
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((iod_obj_id_t)(loc_attrkv_id))
                 ((iod_obj_id_t)(attr_id))
                 ((iod_obj_id_t)(mdkv_id))
                 ((hg_const_string_t)(path))
                 ((hg_const_string_t)(attr_name))
                 ((hid_t)(type_id)) 
                 ((hid_t)(space_id)))
MERCURY_GEN_PROC(attr_create_out_t, 
                 ((iod_handles_t)(iod_oh)))
MERCURY_GEN_PROC(attr_open_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((iod_obj_id_t)(loc_attrkv_id))
                 ((hg_const_string_t)(path)) 
                 ((hg_const_string_t)(attr_name)))
MERCURY_GEN_PROC(attr_open_out_t, 
                 ((iod_handles_t)(iod_oh)) 
                 ((iod_obj_id_t)(iod_id)) 
                 ((iod_obj_id_t)(mdkv_id))
                 ((hid_t)(type_id)) 
                 ((hid_t)(space_id)))
MERCURY_GEN_PROC(attr_op_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num)) 
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((iod_obj_id_t)(loc_attrkv_id))
                 ((hg_const_string_t)(path)) 
                 ((hg_const_string_t)(attr_name)))
MERCURY_GEN_PROC(attr_rename_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num)) 
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((iod_obj_id_t)(loc_attrkv_id))
                 ((hg_const_string_t)(path)) 
                 ((hg_const_string_t)(old_attr_name)) 
                 ((hg_const_string_t)(new_attr_name)))
MERCURY_GEN_PROC(attr_io_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num)) 
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(iod_oh)) 
                 ((iod_obj_id_t)(iod_id)) 
                 ((iod_obj_id_t)(mdkv_id))
                 ((uint64_t)(checksum)) 
                 ((hid_t)(space_id))
                 ((hid_t)(type_id)) 
                 ((hg_bulk_t)(bulk_handle)))
MERCURY_GEN_PROC(attr_close_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handles_t)(iod_oh)) 
                 ((iod_obj_id_t)(iod_id)))

MERCURY_GEN_PROC(group_create_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num)) 
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((iod_obj_id_t)(grp_id))
                 ((iod_obj_id_t)(mdkv_id)) 
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hg_const_string_t)(name)) 
                 ((hid_t)(gapl_id)) 
                 ((hid_t)(gcpl_id)) 
                 ((hid_t)(lcpl_id)))
MERCURY_GEN_PROC(group_create_out_t, 
                 ((iod_handles_t)(iod_oh)))
MERCURY_GEN_PROC(group_open_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((hg_const_string_t)(name))
                 ((hid_t)(gapl_id)))
MERCURY_GEN_PROC(group_open_out_t, 
                 ((iod_handles_t)(iod_oh)) 
                 ((iod_obj_id_t)(iod_id)) 
                 ((iod_obj_id_t)(mdkv_id)) 
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hid_t)(gcpl_id)))
MERCURY_GEN_PROC(group_close_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handles_t)(iod_oh)) 
                 ((iod_obj_id_t)(iod_id)))

MERCURY_GEN_PROC(map_create_in_t, 
                 ((axe_t)(axe_info)) 
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num)) 
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id))
                 ((iod_obj_id_t)(map_id))
                 ((iod_obj_id_t)(mdkv_id))
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hg_const_string_t)(name)) 
                 ((hid_t)(keytype_id))
                 ((hid_t)(valtype_id)) 
                 ((hid_t)(mapl_id))
                 ((hid_t)(mcpl_id)) 
                 ((hid_t)(lcpl_id)))
MERCURY_GEN_PROC(map_create_out_t, 
                 ((iod_handles_t)(iod_oh)))
MERCURY_GEN_PROC(map_open_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((hg_const_string_t)(name))
                 ((hid_t)(mapl_id)))
MERCURY_GEN_PROC(map_open_out_t, 
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)) 
                 ((iod_obj_id_t)(mdkv_id))
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hid_t)(keytype_id))
                 ((hid_t)(valtype_id))
                 ((hid_t)(mcpl_id)))
MERCURY_GEN_PROC(map_set_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)) 
                 ((hid_t)(key_maptype_id))
                 ((hid_t)(key_memtype_id))
                 ((binary_buf_t)(key))
                 ((hid_t)(val_maptype_id))
                 ((hid_t)(val_memtype_id))
                 ((uint64_t)(val_checksum))
                 ((hg_bulk_t)(val_handle))
                 ((hid_t)(dxpl_id)))
MERCURY_GEN_PROC(map_get_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)) 
                 ((hid_t)(key_maptype_id))
                 ((hid_t)(key_memtype_id))
                 ((binary_buf_t)(key))
                 ((hid_t)(val_maptype_id))
                 ((hid_t)(val_memtype_id))
                 ((hbool_t)(val_is_vl))
                 ((hg_bulk_t)(val_handle))
                 ((size_t)(val_size))
                 ((hid_t)(dxpl_id)))
MERCURY_GEN_PROC(map_get_out_t, 
                 ((int32_t)(ret))
                 ((size_t)(val_size))
                 ((uint64_t)(val_cs)))
MERCURY_GEN_PROC(map_get_count_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)))
MERCURY_GEN_PROC(map_op_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)) 
                 ((hid_t)(key_maptype_id))
                 ((hid_t)(key_memtype_id))
                 ((binary_buf_t)(key)))
MERCURY_GEN_PROC(map_close_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)))

MERCURY_GEN_PROC(dset_create_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id))
                 ((iod_obj_id_t)(dset_id)) 
                 ((iod_obj_id_t)(mdkv_id))
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hg_const_string_t)(name))
                 ((hid_t)(dapl_id))
                 ((hid_t)(dcpl_id))
                 ((hid_t)(lcpl_id))
                 ((hid_t)(type_id))
                 ((hid_t)(space_id)))
MERCURY_GEN_PROC(dset_create_out_t, 
                 ((iod_handles_t)(iod_oh)))
MERCURY_GEN_PROC(dset_open_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((hg_const_string_t)(name))
                 ((hid_t)(dapl_id)))
MERCURY_GEN_PROC(dset_open_out_t, 
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)) 
                 ((iod_obj_id_t)(mdkv_id))
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hid_t)(dcpl_id))
                 ((hid_t)(type_id))
                 ((hid_t)(space_id)))
MERCURY_GEN_PROC(dset_set_extent_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(iod_oh)) 
                 ((iod_obj_id_t)(iod_id))
                 ((iod_obj_id_t)(mdkv_id)) 
                 ((hid_t)(space_id))
                 ((dims_t)(dims)))
MERCURY_GEN_PROC(dset_io_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(iod_oh)) 
                 ((iod_obj_id_t)(iod_id))
                 ((iod_obj_id_t)(mdkv_id))
                 ((hid_t)(dset_type_id))
                 ((hid_t)(mem_type_id))
                 ((hid_t)(space_id))
                 ((hid_t)(dxpl_id))
                 ((uint64_t)(checksum))
                 ((hg_bulk_t)(bulk_handle))
                 ((hg_bulk_t)(vl_len_bulk_handle))
                 ((uint64_t)(axe_id)))
MERCURY_GEN_PROC(dset_multi_io_list_ent_t,
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id))
                 ((iod_obj_id_t)(mdkv_id))
                 ((hid_t)(dset_type_id))
                 ((hid_t)(mem_type_id))
                 ((hid_t)(space_id))
                 ((uint64_t)(checksum))
                 ((hg_bulk_t)(bulk_handle))
                 ((hg_bulk_t)(vl_len_bulk_handle)))
typedef struct dset_multi_io_list_t {
    size_t count;
    dset_multi_io_list_ent_t *list;
} dset_multi_io_list_t;
MERCURY_GEN_PROC(dset_multi_io_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((dset_multi_io_list_t)(list))
                 ((hid_t)(dxpl_id))
                 ((uint64_t)(axe_id)))
MERCURY_GEN_PROC(dset_read_out_t, 
                 ((int32_t)(ret))
                 ((uint64_t)(cs))
                 ((size_t)(buf_size)))
MERCURY_GEN_PROC(dset_close_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)))

MERCURY_GEN_PROC(dtype_commit_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id))
                 ((iod_obj_id_t)(dtype_id)) 
                 ((iod_obj_id_t)(mdkv_id))
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hg_const_string_t)(name))
                 ((hid_t)(tapl_id))
                 ((hid_t)(tcpl_id))
                 ((hid_t)(lcpl_id))
                 ((hid_t)(type_id)))
MERCURY_GEN_PROC(dtype_commit_out_t, 
                 ((iod_handles_t)(iod_oh)))
MERCURY_GEN_PROC(dtype_open_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id)) 
                 ((hg_const_string_t)(name))
                 ((hid_t)(tapl_id)))
MERCURY_GEN_PROC(dtype_open_out_t, 
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)) 
                 ((iod_obj_id_t)(mdkv_id))
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hid_t)(tcpl_id))
                 ((hid_t)(type_id)))
MERCURY_GEN_PROC(dtype_close_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)))

MERCURY_GEN_PROC(link_create_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((int8_t)(create_type))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id))
                 ((hg_const_string_t)(loc_name))
                 ((iod_handles_t)(target_loc_oh)) 
                 ((iod_obj_id_t)(target_loc_id))
                 ((iod_obj_id_t)(target_mdkv_id))
                 ((hg_const_string_t)(target_name))
                 ((hg_const_string_t)(link_value))
                 ((hid_t)(lapl_id))
                 ((hid_t)(lcpl_id)))
MERCURY_GEN_PROC(link_move_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((hbool_t)(copy_flag))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(src_loc_oh))
                 ((iod_obj_id_t)(src_loc_id))
                 ((hg_const_string_t)(src_loc_name))
                 ((iod_handles_t)(dst_loc_oh))
                 ((iod_obj_id_t)(dst_loc_id))
                 ((hg_const_string_t)(dst_loc_name))
                 ((hid_t)(lapl_id))
                 ((hid_t)(lcpl_id)))
MERCURY_GEN_PROC(link_op_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh)) 
                 ((iod_obj_id_t)(loc_id))
                 ((iod_obj_id_t)(loc_mdkv_id))
                 ((iod_obj_id_t)(loc_attrkv_id))
                 ((hbool_t)(recursive))
                 ((hg_const_string_t)(path)))
MERCURY_GEN_PROC(link_get_val_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh)) 
                 ((iod_obj_id_t)(loc_id)) 
                 ((hg_const_string_t)(path))
                 ((uint64_t)(length)))
MERCURY_GEN_PROC(link_get_val_out_t, 
                 ((int32_t)(ret))
                 ((value_t)(value)))

MERCURY_GEN_PROC(object_token_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((iod_obj_id_t)(iod_id))
                 ((uint64_t)(trans_num)))
MERCURY_GEN_PROC(object_op_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh)) 
                 ((iod_obj_id_t)(loc_id))
                 ((iod_obj_id_t)(loc_mdkv_id))
                 ((iod_obj_id_t)(loc_attrkv_id))
                 ((hg_const_string_t)(loc_name)))
MERCURY_GEN_PROC(object_open_out_t, 
                 ((int32_t)(obj_type))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)) 
                 ((iod_obj_id_t)(mdkv_id))
                 ((iod_obj_id_t)(attrkv_id)) 
                 ((hid_t)(cpl_id))
                 ((hid_t)(id1))
                 ((hid_t)(id2)))
MERCURY_GEN_PROC(object_copy_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh)) 
                 ((iod_handles_t)(src_loc_oh))
                 ((iod_obj_id_t)(src_loc_id))
                 ((hg_const_string_t)(src_loc_name))
                 ((iod_handles_t)(dst_loc_oh))
                 ((iod_obj_id_t)(dst_loc_id))
                 ((hg_const_string_t)(dst_loc_name))
                 ((hid_t)(ocpypl_id))
                 ((hid_t)(lcpl_id)))
MERCURY_GEN_PROC(object_set_comment_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh)) 
                 ((iod_obj_id_t)(loc_id))
                 ((iod_obj_id_t)(loc_mdkv_id))
                 ((hg_const_string_t)(path))
                 ((hg_const_string_t)(comment)))
MERCURY_GEN_PROC(object_get_comment_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope)) 
                 ((uint64_t)(rcxt_num))
                 ((iod_handle_t)(coh))
                 ((iod_handles_t)(loc_oh)) 
                 ((iod_obj_id_t)(loc_id))
                 ((iod_obj_id_t)(loc_mdkv_id))
                 ((hg_const_string_t)(path))
                 ((uint64_t)(length)))
MERCURY_GEN_PROC(object_get_comment_out_t, 
                 ((int32_t)(ret))
                 ((name_t)(name)))

MERCURY_GEN_PROC(rc_acquire_in_t, 
                 ((axe_t)(axe_info)) 
                 ((iod_handle_t)(coh))
                 ((uint64_t)(c_version))
                 ((hid_t)(rcapl_id)))
MERCURY_GEN_PROC(rc_acquire_out_t, 
                 ((int32_t)(ret))
                 ((uint64_t)(c_version)))
MERCURY_GEN_PROC(rc_release_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint64_t)(c_version)))
MERCURY_GEN_PROC(rc_persist_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint64_t)(c_version)))
MERCURY_GEN_PROC(rc_snapshot_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((hg_const_string_t)(snapshot_name)) 
                 ((uint64_t)(c_version)))

MERCURY_GEN_PROC(tr_start_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint64_t)(trans_num))
                 ((hid_t)(trspl_id)))
MERCURY_GEN_PROC(tr_finish_in_t, 
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((iod_handle_t)(coh))
                 ((uint64_t)(trans_num))
                 ((hbool_t)(acquire))
                 ((uint32_t)(client_rank))
                 ((iod_obj_id_t)(oidkv_id))
                 ((iod_obj_id_t)(kv_oid_index)) 
                 ((iod_obj_id_t)(array_oid_index)) 
                 ((iod_obj_id_t)(blob_oid_index))
                 ((hid_t)(trfpl_id)))
MERCURY_GEN_PROC(tr_set_depend_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint64_t)(child_trans_num))
                 ((uint64_t)(parent_trans_num)))
MERCURY_GEN_PROC(tr_skip_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh)) 
                 ((uint64_t)(start_trans_num))
                 ((uint64_t)(count)))
MERCURY_GEN_PROC(tr_abort_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint64_t)(trans_num)))

MERCURY_GEN_PROC(prefetch_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint32_t)(cs_scope))  
                 ((uint64_t)(rcxt_num))
                 ((int32_t)(layout_type))
                 ((hid_t)(selection))
                 ((hid_t)(key_type))
                 ((binary_buf_t)(low_key))
                 ((binary_buf_t)(high_key))
                 ((int32_t)(obj_type))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id)))
MERCURY_GEN_PROC(evict_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint32_t)(cs_scope))  
                 ((uint64_t)(rcxt_num))
                 ((hrpl_t)(replica_id))
                 ((int32_t)(obj_type))
                 ((iod_handles_t)(iod_oh))
                 ((iod_obj_id_t)(iod_id))
                 ((iod_obj_id_t)(mdkv_id))
                 ((iod_obj_id_t)(attrkv_id)))

MERCURY_GEN_PROC(view_create_in_t, 
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint32_t)(cs_scope))  
                 ((iod_handles_t)(loc_oh))
                 ((iod_obj_id_t)(loc_id))
                 ((uint64_t)(rcxt_num))
                 ((int32_t)(obj_type))
                 ((hid_t)(vcpl_id))
                 ((hid_t)(query_id)))

MERCURY_GEN_PROC(view_create_out_t, 
                 ((hbool_t)(valid_view))
                 ((obj_info_t)(obj_info))
                 ((attr_info_t)(attr_info))
                 ((region_info_t)(region_info)))

MERCURY_GEN_PROC(idx_set_info_in_t,
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(trans_num))
                 ((iod_obj_id_t)(mdkv_id))
                 ((uint32_t)(idx_plugin_id))
                 ((binary_buf_t)(idx_metadata)))
MERCURY_GEN_PROC(idx_get_info_in_t,
                 ((axe_t)(axe_info))
                 ((iod_handle_t)(coh))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(rcxt_num))
                 ((iod_obj_id_t)(mdkv_id)))
MERCURY_GEN_PROC(idx_get_info_out_t,
                 ((int32_t)(ret))
                 ((uint64_t)(idx_count))
                 ((uint32_t)(idx_plugin_id))
                 ((binary_buf_t)(idx_metadata)))
MERCURY_GEN_PROC(idx_rm_info_in_t,
                 ((axe_t)(axe_info))
                 ((uint32_t)(cs_scope))
                 ((uint64_t)(trans_num))
                 ((iod_handle_t)(coh))
                 ((iod_obj_id_t)(mdkv_id)))

#endif /* H5_HAVE_EFF */
#endif /* _H5VLiod_common_H */
