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
 * This file contains function prototypes for each exported function in the
 * H5FF module.
 */
#ifndef _H5FFpublic_H
#define _H5FFpublic_H

/* System headers needed by this file */

/* Public headers needed by this file */
#include "H5VLpublic.h" 	/* Public VOL header file		*/

/*****************/
/* Public Macros */
/*****************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef H5_HAVE_EFF

typedef uint64_t haddr_ff_t;

/*******************/
/* Public Typedefs */
/*******************/

typedef struct H5L_ff_info_t{
    H5L_type_t     type;
    H5T_cset_t     cset;
    union {
        haddr_ff_t  address;
        size_t      val_size;
    } u;
} H5L_ff_info_t;

typedef struct H5O_ff_info_t {
    haddr_ff_t          addr;       /* Object address in file               */
    H5O_type_t          type;       /* Basic object type                    */
    unsigned            rc;         /* Reference count of object            */
    hsize_t             num_attrs;  /* # of attributes attached to object   */
} H5O_ff_info_t;

/* Bitflag values to enable/disable checksuming in different layers */
typedef enum H5FF_checksum_bitflag_t {
    H5_CHECKSUM_NONE      = 0x00,
    H5_CHECKSUM_TRANSFER  = 0x01,
    H5_CHECKSUM_IOD       = 0x02,
    H5_CHECKSUM_MEMORY    = 0x04,
    H5_CHECKSUM_ALL       = 0x07
} H5FF_checksum_bitflag_t;

#define H5R_OBJ_FF_REF_BUF_SIZE    sizeof(haddr_t)


/********************/
/* Public Variables */
/********************/

/*********************/
/* Public Prototypes */
/*********************/

/* Typedef for H5Aiterate_ff() callbacks */
typedef herr_t (*H5A_operator_ff_t)(hid_t location_id, const char *attr_name, const H5A_info_t *ainfo, 
                                    void *op_data, hid_t rcxt_id);

/* Prototype for H5Ovisit_ff/H5Ovisit_by_name_ff() operator */
typedef herr_t (*H5O_iterate_ff_t)(hid_t obj, const char *name, const H5O_ff_info_t *info,
                                   void *op_data, hid_t rcxt_id);

/* Prototype for H5Literate_ff/H5Literate_by_name_ff() operator */
typedef herr_t (*H5L_iterate_ff_t)(hid_t obj, const char *name, const H5L_ff_info_t *info,
                                   void *op_data, hid_t rcxt_id);

/* API wrappers */
H5_DLL hid_t H5Fcreate_ff(const char *filename, unsigned flags, hid_t fcpl,
                          hid_t fapl, hid_t estack_id);
H5_DLL hid_t H5Fopen_ff(const char *filename, unsigned flags, hid_t fapl_id,
                        hid_t *rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Fclose_ff(hid_t file_id, hbool_t persist_flag, hid_t estack_id);

H5_DLL hid_t H5Gcreate_ff(hid_t loc_id, const char *name, hid_t lcpl_id,
                          hid_t gcpl_id, hid_t gapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL hid_t H5Gopen_ff(hid_t loc_id, const char *name, hid_t gapl_id,
                        hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Gclose_ff(hid_t group_id, hid_t estack_id);

H5_DLL hid_t H5Dcreate_ff(hid_t loc_id, const char *name, hid_t type_id,
                          hid_t space_id, hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id,
                          hid_t trans_id, hid_t estack_id);
H5_DLL hid_t H5Dcreate_anon_ff(hid_t file_id, hid_t type_id, hid_t space_id,
                               hid_t plist_id, hid_t dapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL hid_t H5Dopen_ff(hid_t loc_id, const char *name, hid_t dapl_id,
                        hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Dwrite_ff(hid_t dset_id, hid_t mem_type_id, hid_t mem_space_id,
                          hid_t file_space_id, hid_t dxpl_id, const void *buf,
                          hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Dread_ff(hid_t dset_id, hid_t mem_type_id, hid_t mem_space_id,
                         hid_t file_space_id, hid_t dxpl_id, void *buf/*out*/,
                         hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Dset_extent_ff(hid_t dset_id, const hsize_t size[], hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Dclose_ff(hid_t dset_id, hid_t estack_id);

H5_DLL herr_t H5Tcommit_ff(hid_t loc_id, const char *name, hid_t type_id, hid_t lcpl_id,
                           hid_t tcpl_id, hid_t tapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL hid_t H5Topen_ff(hid_t loc_id, const char *name, hid_t tapl_id, 
                        hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Tclose_ff(hid_t type_id, hid_t estack_id);

H5_DLL hid_t H5Acreate_ff(hid_t loc_id, const char *attr_name, hid_t type_id, hid_t space_id,
                          hid_t acpl_id, hid_t aapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL hid_t H5Acreate_by_name_ff(hid_t loc_id, const char *obj_name, const char *attr_name,
                                  hid_t type_id, hid_t space_id, hid_t acpl_id, hid_t aapl_id,
                                  hid_t lapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL hid_t H5Aopen_ff(hid_t loc_id, const char *attr_name, hid_t aapl_id, 
                        hid_t rcxt_id, hid_t estack_id);
H5_DLL hid_t H5Aopen_by_name_ff(hid_t loc_id, const char *obj_name, const char *attr_name,
                                hid_t aapl_id, hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Awrite_ff(hid_t attr_id, hid_t dtype_id, const void *buf, 
                          hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Aread_ff(hid_t attr_id, hid_t dtype_id, void *buf, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Arename_ff(hid_t loc_id, const char *old_name, const char *new_name, 
                           hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Arename_by_name_ff(hid_t loc_id, const char *obj_name, const char *old_attr_name,
                                   const char *new_attr_name, hid_t lapl_id, 
                                   hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Adelete_ff(hid_t loc_id, const char *name, hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Adelete_by_name_ff(hid_t loc_id, const char *obj_name, const char *attr_name,
                                   hid_t lapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Aexists_by_name_ff(hid_t loc_id, const char *obj_name, const char *attr_name,
                                   hid_t lapl_id, htri_t *ret, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Aexists_ff(hid_t obj_id, const char *attr_name, htri_t *ret, 
                           hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Aiterate_ff(hid_t loc_id, H5_index_t idx_type, H5_iter_order_t order,
                            hsize_t *idx, H5A_operator_ff_t op, void *op_data,
                            hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Aiterate_by_name_ff(hid_t loc_id, const char *obj_name, H5_index_t idx_type,
                                    H5_iter_order_t order, hsize_t *idx, H5A_operator_ff_t op, void *op_data,
                                    hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Aclose_ff(hid_t attr_id, hid_t estack_id);

H5_DLL herr_t H5Lmove_ff(hid_t src_loc_id, const char *src_name, hid_t dst_loc_id, 
                         const char *dst_name, hid_t lcpl_id, hid_t lapl_id, 
                         hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Lcopy_ff(hid_t src_loc_id, const char *src_name, hid_t dst_loc_id,
                         const char *dst_name, hid_t lcpl_id, hid_t lapl_id, 
                         hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Lcreate_soft_ff(const char *link_target, hid_t link_loc_id, const char *link_name, 
                                hid_t lcpl_id, hid_t lapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Lcreate_hard_ff(hid_t cur_loc_id, const char *cur_name, hid_t new_loc_id, 
                                const char *new_name, hid_t lcpl_id, hid_t lapl_id, 
                                hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Ldelete_ff(hid_t loc_id, const char *name, hid_t lapl_id, 
                           hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Lexists_ff(hid_t loc_id, const char *name, hid_t lapl_id, htri_t *ret, 
                           hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Lget_info_ff(hid_t link_loc_id, const char *link_name, H5L_ff_info_t *link_buff,
                             hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Lget_val_ff(hid_t link_loc_id, const char *link_name, void *linkval_buff, 
                            size_t size, hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Literate_ff(hid_t loc_id, H5_index_t idx_type, H5_iter_order_t order, hsize_t *idx,
                            H5L_iterate_ff_t op, void *op_data, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Literate_by_name_ff(hid_t loc_id, const char *obj_name, H5_index_t idx_type,
                                    H5_iter_order_t order, hsize_t *idx, H5L_iterate_ff_t op, void *op_data, 
                                    hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Lvisit_ff(hid_t loc_id, H5_index_t idx_type, H5_iter_order_t order,
                          H5L_iterate_ff_t op, void *op_data, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Lvisit_by_name_ff(hid_t loc_id, const char *obj_name, H5_index_t idx_type,
                                  H5_iter_order_t order, H5L_iterate_ff_t op, void *op_data, 
                                  hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);

H5_DLL hid_t H5Oopen_ff(hid_t loc_id, const char *name, hid_t lapl_id, hid_t rcxt_id);
H5_DLL hid_t H5Oopen_by_addr_ff(hid_t loc_id, haddr_ff_t addr, hid_t rcxt_id);
H5_DLL herr_t H5Oget_token(hid_t obj_id, void *token, size_t *token_size);
H5_DLL hid_t H5Oopen_by_token(const void *token, hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Olink_ff(hid_t obj_id, hid_t new_loc_id, const char *new_name, hid_t lcpl_id,
                         hid_t lapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Oexists_by_name_ff(hid_t loc_id, const char *name, htri_t *ret, 
                                   hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Oset_comment_ff(hid_t obj_id, const char *comment, hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Oset_comment_by_name_ff(hid_t loc_id, const char *name, const char *comment,
                                        hid_t lapl_id, hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Oget_comment_ff(hid_t loc_id, char *comment, size_t bufsize, ssize_t *ret,
                                hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Oget_comment_by_name_ff(hid_t loc_id, const char *name, char *comment, size_t bufsize,
                                        ssize_t *ret, hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
//H5_DLL herr_t H5Ocopy_ff(hid_t src_loc_id, const char *src_name, hid_t dst_loc_id,
//const char *dst_name, hid_t ocpypl_id, hid_t lcpl_id, 
//hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Oget_info_ff(hid_t object_id, H5O_ff_info_t *object_info, 
                             hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Oget_info_by_name_ff(hid_t loc_id, const char *object_name, 
                                     H5O_ff_info_t *object_info, hid_t lapl_id, 
                                     hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Oget_addr_ff(hid_t object_id, haddr_ff_t *addr);
H5_DLL herr_t H5Ovisit_ff(hid_t obj_id, H5_index_t idx_type, H5_iter_order_t order,
                          H5O_iterate_ff_t op, void *op_data, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Ovisit_by_name_ff(hid_t loc_id, const char *obj_name,
                                  H5_index_t idx_type, H5_iter_order_t order, H5O_iterate_ff_t op,
                                  void *op_data, hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5Oclose_ff(hid_t object_id, hid_t estack_id);

H5_DLL hid_t H5Qapply_ff(hid_t loc_id, hid_t query_id, unsigned *result, hid_t vcpl_id,
    hid_t rcxt_id, hid_t estack_id);
H5_DLL hid_t H5Qapply_multi_ff(size_t loc_count, hid_t *loc_ids, hid_t query_id,
    unsigned *result, hid_t vcpl_id, hid_t *rcxt_ids, hid_t estack_id);

/* Functions in H5R.c */
//H5_DLL herr_t H5Rcreate_object_ff(href_ff_t *ref, hid_t loc_id, const char *name, hid_t lapl_id,
//                                  hid_t rcxt_id, hid_t estack_id);
//H5_DLL herr_t H5Rcreate_region_ff(href_ff_t *ref, hid_t loc_id, const char *name, hid_t space_id,
//                                  hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
//H5_DLL herr_t H5Rcreate_attr_ff(href_ff_t *ref, hid_t loc_id, const char *name, const char *attr_name,
//                                hid_t lapl_id, hid_t rcxt_id, hid_t estack_id);
H5_DLL hid_t  H5Rget_object_ff(hid_t loc_id, hid_t oapl_id, href_t ref,
    hid_t rcxt_id, hid_t estack_id);

H5_DLL herr_t H5Aprefetch_ff(hid_t attr_id, hid_t rcxt_id, hrpl_t *replica_id,
                             hid_t dxpl_id, hid_t estack_id);
H5_DLL herr_t H5Gprefetch_ff(hid_t grp_id, hid_t rcxt_id, hrpl_t *replica_id,
                             hid_t dxpl_id, hid_t estack_id);
H5_DLL herr_t H5Tprefetch_ff(hid_t type_id, hid_t rcxt_id, hrpl_t *replica_id,
                             hid_t dxpl_id, hid_t estack_id);
H5_DLL herr_t H5Dprefetch_ff(hid_t dset_id, hid_t rcxt_id, hrpl_t *replica_id,
                             hid_t dxpl_id, hid_t estack_id);
H5_DLL herr_t H5Mprefetch_ff(hid_t map_id, hid_t rcxt_id, hrpl_t *replica_id,
                             hid_t dxpl_id, hid_t estack_id);

H5_DLL herr_t H5Aevict_ff(hid_t attr_id, uint64_t c_version, hid_t dxpl_id, hid_t estack_id);
H5_DLL herr_t H5Devict_ff(hid_t dset_id, uint64_t c_version, hid_t dxpl_id, hid_t estack_id);
H5_DLL herr_t H5Mevict_ff(hid_t map_id, uint64_t c_version, hid_t dxpl_id, hid_t estack_id);
H5_DLL herr_t H5Gevict_ff(hid_t grp_id, uint64_t c_version, hid_t dxpl_id, hid_t estack_id);
H5_DLL herr_t H5Tevict_ff(hid_t type_id, uint64_t c_version, hid_t dxpl_id, hid_t estack_id);

/* New Routines for Dynamic Data Structures Use Case (ACG) */
H5_DLL herr_t H5DOappend(hid_t dataset_id, hid_t dxpl_id, unsigned axis, size_t extension, 
                         hid_t memtype, const void *buffer);
H5_DLL herr_t H5DOappend_ff(hid_t dataset_id, hid_t dxpl_id, unsigned axis, size_t extension, 
                            hid_t memtype, const void *buffer, hid_t trans_id, 
                            hid_t estack_id);
H5_DLL herr_t H5DOsequence(hid_t dataset_id, hid_t dxpl_id, unsigned axis, hsize_t start, 
                           size_t sequence, hid_t memtype, void *buffer);
H5_DLL herr_t H5DOsequence_ff(hid_t dataset_id, hid_t dxpl_id, unsigned axis, hsize_t start, 
                              size_t sequence, hid_t memtype, void *buffer, 
                              hid_t rcxt_id, hid_t estack_id);
H5_DLL herr_t H5DOset(hid_t dataset_id, hid_t dxpl_id, const hsize_t coord[],
                      hid_t memtype, const void *buffer);
H5_DLL herr_t H5DOset_ff(hid_t dataset_id, hid_t dxpl_id, const hsize_t coord[],hid_t memtype, 
                         const void *buffer, hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5DOget(hid_t dataset_id, hid_t dxpl_id, const hsize_t coord[],hid_t memtype, void *buffer);
H5_DLL herr_t H5DOget_ff(hid_t dataset_id, hid_t dxpl_id, const hsize_t coord[],hid_t memtype, 
                          void *buffer, hid_t rcxt_id, hid_t estack_id);

H5_DLL herr_t H5Xcreate_ff(hid_t loc_id, unsigned plugin_id, hid_t xcpl_id,
    hid_t trans_id, hid_t estack_id);
H5_DLL herr_t H5Xremove_ff(hid_t loc_id, unsigned idx, hid_t trans_id,
    hid_t estack_id);
H5_DLL herr_t H5Xget_count_ff(hid_t loc_id, hsize_t *idx_count, hid_t rcxt_id,
    hid_t estack_id);
H5_DLL herr_t H5Pget_xapl_transaction(hid_t xapl_id, hid_t *trans_id);
H5_DLL herr_t H5Pget_xapl_read_context(hid_t xapl_id, hid_t *rcxt_id);
H5_DLL herr_t H5Pget_xxpl_transaction(hid_t xxpl_id, hid_t *trans_id);
H5_DLL herr_t H5Pget_xxpl_read_context(hid_t xxpl_id, hid_t *rcxt_id);

#endif /* H5_HAVE_EFF */

#ifdef __cplusplus
}
#endif

#endif /* _H5FFpublic_H */

