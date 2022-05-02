/* defines for the ematrix methods in the pyevspace c api */
#ifndef EVSPACE_MATRIX_DEFN_H
#define EVSPACE_MATRIX_DEGN_H

#define API_NUM_OFFSET 20

/* nb_add */
#define EVSpace_Matrix_add_NUM 0 + API_NUM_OFFSET
#define EVSpace_Matrix_add_RETURN EMatrix* 
#define EVSpace_Matrix_add_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* nb_sub */
#define EVSpace_Matrix_sub_NUM 1 + API_NUM_OFFSET
#define EVSpace_Matrix_sub_RETURN EMatrix* 
#define EVSpace_Matrix_sub_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* nb_mult (matrix) */
#define EVSpace_Matrix_multm_NUM 2 + API_NUM_OFFSET
#define EVSpace_Matrix_multm_RETURN EMatrix* 
#define EVSpace_Matrix_multm_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* nb_mult (vector) */
#define EVSpace_Matrix_multv_NUM 3 + API_NUM_OFFSET
#define EVSpace_Matrix_multv_RETURN EVector* 
#define EVSpace_Matrix_multv_PROTO (const EMatrix* lhs, const EVector* rhs)
/* nb_mult (double) */
#define EVSpace_Matrix_multd_NUM 4 + API_NUM_OFFSET
#define EVSpace_Matrix_multd_RETURN EMatrix*
#define EVSpace_Matrix_multd_PROTO (const EMatrix* lhs, double rhs)
/* nb_negative */
#define EVSpace_Matrix_neg_NUM 5 + API_NUM_OFFSET
#define EVSpace_Matrix_neg_RETURN EMatrix* 
#define EVSpace_Matrix_neg_PROTO (const EMatrix* vec)
/* nb_inplace_add */
#define EVSpace_Matrix_iadd_NUM 6 + API_NUM_OFFSET
#define EVSpace_Matrix_iadd_RETURN void 
#define EVSpace_Matrix_iadd_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_subtract */
#define EVSpace_Matrix_isub_NUM 7 + API_NUM_OFFSET
#define EVSpace_Matrix_isub_RETURN void 
#define EVSpace_Matrix_isub_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_multiply (matrix) */
#define EVSpace_Matrix_imultm_NUM 8 + API_NUM_OFFSET
#define EVSpace_Matrix_imultm_RETURN void 
#define EVSpace_Matrix_imultm_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_multiply (double) */
#define EVSpace_Matrix_imultd_NUM 9 + API_NUM_OFFSET
#define EVSpace_Matrix_imultd_RETURN void 
#define EVSpace_Matrix_imultd_PROTO (EMatrix* lhs, double rhs)
/* nb_true_divide */
#define EVSpace_Matrix_div_NUM 10 + API_NUM_OFFSET
#define EVSpace_Matrix_div_RETURN EMatrix* 
#define EVSpace_Matrix_div_PROTO (const EMatrix* lhs, double rhs)
/* nb_inplace_true_divide */
#define EVSpace_Matrix_idiv_NUM 11 + API_NUM_OFFSET
#define EVSpace_Matrix_idiv_RETURN void 
#define EVSpace_Matrix_idiv_PROTO (EMatrix* lhs, double rhs)
/* equal to */
#define EVSpace_Matrix_eq_NUM 12 + API_NUM_OFFSET
#define EVSpace_Matrix_eq_RETURN int 
#define EVSpace_Matrix_eq_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* not equal to */
#define EVSpace_Matrix_ne_NUM 13 + API_NUM_OFFSET
#define EVSpace_Matrix_ne_RETURN int 
#define EVSpace_Matrix_ne_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* det */
#define EVSpace_Matrix_det_NUM 14 + API_NUM_OFFSET
#define EVSpace_Matrix_det_RETURN double 
#define EVSpace_Matrix_det_PROTO (const EMatrix* lhs)
/* transpose */
#define EVSpace_Matrix_trans_NUM 15 + API_NUM_OFFSET
#define EVSpace_Matrix_trans_RETURN EMatrix*
#define EVSpace_Matrix_trans_PROTO (const EMatrix* mat)

#define EVSpace_EMatrix_API_COUNT 16

#endif // EVSPACE_MATRIX_DEGN_H