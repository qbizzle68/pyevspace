/* definitions for the api for matrix methods in pyevspace module */
#ifndef EVSPACE_MATRIX_API_DEFN_H
#define EVSPACE_MATRIX_API_DEFN_H

/* these definitions include macro definitions from evspace_matrix_defn.h
	include that file before this one */
/* the api array must be declared before include of this file */

#define EVSpace_Matrix_add \
	(*(EVSpace_Matrix_add_RETURN) (*)EVSpace_Matrix_add_PROTO) EVSpace_API[EVSpace_Matrix_add_NUM]
#define EVSpace_Matrix_sub \
	(*(EVSpace_Matrix_sub_RETURN) (*)EVSpace_Matrix_sub_PROTO) EVSpace_API[EVSpace_Matrix_sub_NUM]
#define EVSpace_Mmult \
	(*(EVSpace_Mmult_RETURN) (*)EVSpace_Mmult_PROTO) EVSpace_API[EVSpace_Mmult_NUM]
#define EVSpace_Matrix_neg \
	(*(EVSpace_Matrix_neg_RETURN) (*)EVSpace_Matrix_neg_PROTO) EVSpace_API[EVSpace_Matrix_neg_NUM]
#define EVSpace_Mabs \
	(*(EVSpace_Mabs_RETURN) (*)EVSpace_Mabs_PROTO) EVSpace_API[EVSpace_Mabs_NUM]
#define EVSpace_Matrix_iadd \
	(*(EVSpace_Matrix_iadd_RETURN) (*)EVSpace_Matrix_iadd_PROTO) EVSpace_API[EVSpace_Matrix_iadd_NUM]
#define EVSpace_Matrix_isub \
	(*(EVSpace_Matrix_isub_RETURN) (*)EVSpace_Matrix_isub_PROTO) EVSpace_API[EVSpace_Matrix_isub_NUM]
#define EVSpace_Mimult \
	(*(EVSpace_Mimult_RETURN) (*)EVSpace_Mimult_PROTO) EVSpace_API[EVSpace_Mimult_NUM]
#define EVSpace_Matrix_div \
	(*(EVSpace_Matrix_div_RETURN) (*)EVSpace_Matrix_div_PROTO) EVSpace_API[EVSpace_Matrix_div_NUM]
#define EVSpace_Matrix_idiv \
	(*(EVSpace_Matrix_idiv_RETURN) (*)EVSpace_Matrix_idiv_PROTO) EVSpace_API[EVSpace_Matrix_idiv_NUM]
#define EVSpace_Matrix_eq \
	(*(EVSpace_Matrix_eq_RETURN) (*)EVSpace_Matrix_eq_PROTO) EVSpace_API[EVSpace_Matrix_eq_NUM]
#define EVSpace_Matrix_ne \
	(*(EVSpace_Matrix_ne_RETURN) (*)EVSpace_Matrix_ne_PROTO) EVSpace_API[EVSpace_Matrix_ne_NUM]
#define EVSpace_Matrix_det \
	(*(EVSpace_Matrix_det_RETURN) (*)EVSpace_Matrix_det_PROTO) EVSpace_API[EVSpace_Matrix_det_NUM]
#define EVSpace_Matrix_trans \
	(*(EVSpace_Matrix_trans_RETURN) (*)EVSpace_Matrix_trans_PROTO) EVSpace_API[EVSpace_Matrix_trans_NUM]

#endif // EVSPACE_MATRIX_API_DEFN_H