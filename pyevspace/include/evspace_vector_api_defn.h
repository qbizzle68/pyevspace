/* definitions for the api for vector methods in pyevspace module */
#ifndef EVSPACE_VECTOR_API_DEFN_H
#define EVSPACE_VECTOR_API_DEFN_H

/* these definitions include macro definitions from evspace_vector_defn.h
	include that file before this one */
/* the api array must be declared before inclusion of this file */

#define EVSpace_Vector_add \
	(*(EVSpace_Vector_add_RETURN) (*)EVSpace_Vector_add_PROTO) EVSpace_API[EVSpace_Vector_add_NUM]
#define EVSpace_Vector_sub \
	(*(EVSpace_Vector_sub_RETURN) (*)EVSpace_Vector_sub_PROTO) EVSpace_API[EVSpace_Vector_sub_NUM]
#define EVSpace_Vector_mult \
	(*(EVSpace_Vector_mult_RETURN) (*)EVSpace_Vector_mult_PROTO) EVSpace_API[EVSpace_Vector_mult_NUM]
#define EVSpace_Vector_neg \
	(*(EVSpace_Vector_neg_RETURN) (*)EVSpace_Vector_neg_PROTO) EVSpace_API[EVSpace_Vector_neg_NUM]
#define EVSpace_Vector_abs \
	(*(EVSpace_Vector_abs_RETURN) (*)EVSpace_Vector_abs_PROTO) EVSpace_API[EVSpace_Vector_abs_NUM]
#define EVSpace_Vector_iadd \
	(*(EVSpace_Vector_iadd_RETURN) (*)EVSpace_Vector_iadd_PROTO) EVSpace_API[EVSpace_Vector_iadd_NUM]
#define EVSpace_Vector_isub \
	(*(EVSpace_Vector_isub_RETURN) (*)EVSpace_Vector_isub_PROTO) EVSpace_API[EVSpace_Vector_isub_NUM]
#define EVSpace_Vector_imult \
	(*(EVSpace_Vector_imult_RETURN) (*)EVSpace_Vector_imult_PROTO) EVSpace_API[EVSpace_Vector_imult_NUM]
#define EVSpace_Vector_div \
	(*(EVSpace_Vector_div_RETURN) (*)EVSpace_Vector_div_PROTO) EVSpace_API[EVSpace_Vector_div_NUM]
#define EVSpace_Vector_idiv \
	(*(EVSpace_Vector_idiv_RETURN) (*)EVSpace_Vector_idiv_PROTO) EVSpace_API[EVSpace_Vector_idiv_NUM]
#define EVSpace_Vector_eq \
	(*(EVSpace_Vector_eq_RETURN) (*)EVSpace_Vector_eq_PROTO) EVSpace_API[EVSpace_Vector_eq_NUM]
#define EVSpace_Vector_ne \
	(*(EVSpace_Vector_ne_RETURN) (*)EVSpace_Vector_ne_PROTO) EVSpace_API[EVSpace_Vector_ne_NUM]
#define EVSpace_Vector_dot \
	(*(EVSpace_Vector_dot_RETURN) (*)EVSpace_Vector_dot_PROTO) EVSpace_API[EVSpace_Vector_dot_NUM]
#define EVSpace_Vector_cross \
	(*(EVSpace_Vector_cross_RETURN) (*)EVSpace_Vector_cross_PROTO) EVSpace_API[EVSpace_Vector_cross_NUM]
#define EVSpace_vector_mag \
	(*(EVSpace_Vector_mag_RETURN) (*)EVSpace_Vector_mag_PROTO) EVSpace_API[EVSpace_Vector_mag_NUM]
#define EVSpace_Vector_mag2 \
	(*(EVSpace_Vector_mag2_RETURN) (*)EVSpace_Vector_mag2_PROTO) EVSpace_API[EVSpace_Vector_mag2_NUM]
#define EVSpace_Vector_norm \
	(*(EVSpace_Vector_norm_RETURN) (*)EVSpace_Vector_norm_PROTO) EVSpace_API[EVSpace_Vector_norm_NUM]
#define EVSpace_Vector_inorm \
	(*(EVSpace_Vector_inorm_RETURN) (*)EVSpace_Vector_inorm_PROTO) EVSpace_API[EVSpace_Vector_inorm_NUM]
#define EVSpace_Vector_vang \
	(*(EVSpace_Vector_vang_RETURN) (*)EVSpace_Vector_vang_PROTO) EVSpace_API[EVSpace_Vector_vang_NUM]
#define EVSpace_Vector_vxcl \
	(*(EVSpace_Vector_vxcl_RETURN) (*)EVSpace_Vector_vxcl_PROTO) EVSpace_API[EVSpace_Vector_vxcl_NUM]

#endif // EVSPACE_VECTOR_API_DEFN_H