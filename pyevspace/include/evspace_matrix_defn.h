/* defines for the ematrix methods in the pyevspace c api */
#ifndef EVSPACE_MATRIX_DEFN_H
#define EVSPACE_MATRIX_DEGN_H

#define API_NUM_OFFSET 20

/* nb_add */
#define EVSpace_Madd_NUM 0 + API_NUM_OFFSET
#define EVSpace_Madd_RETURN void 
#define EVSpace_Madd_PROTO (EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
/* nb_sub */
#define EVSpace_Msub_NUM 1 + API_NUM_OFFSET
#define EVSpace_Msub_RETURN void 
#define EVSpace_Msub_PROTO (EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
/* nb_mult (matrix) */
#define EVSpace_Mmultm_NUM 2 + API_NUM_OFFSET
#define EVSpace_Mmultm_RETURN void 
#define EVSpace_Mmultm_PROTO (EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
/* nb_mult (vector) */
#define EVSpace_Mmultv_NUM 3 + API_NUM_OFFSET
#define EVSpace_Mmultv_RETURN void 
#define EVSpace_Mmultv_PROTO (EVector* ans, const EMatrix* lhs, const EVector* rhs)
/* nb_mult (double) */
#define EVSpace_Mmultd_NUM 4 + API_NUM_OFFSET
#define EVSpace_Mmultd_RETURN void
#define EVSpace_Mmultd_PROTO (EMatrix* ans, const EMatrix* lhs, double rhs)
/* nb_negative */
#define EVSpace_Mneg_NUM 5 + API_NUM_OFFSET
#define EVSpace_Mneg_RETURN void 
#define EVSpace_Mneg_PROTO (EMatrix* ans, const EMatrix* vec)
/* nb_inplace_add */
#define EVSpace_Miadd_NUM 6 + API_NUM_OFFSET
#define EVSpace_Miadd_RETURN void 
#define EVSpace_Miadd_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_subtract */
#define EVSpace_Misub_NUM 7 + API_NUM_OFFSET
#define EVSpace_Misub_RETURN void 
#define EVSpace_Misub_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_multiply (matrix) */
#define EVSpace_Mimultm_NUM 8 + API_NUM_OFFSET
#define EVSpace_Mimultm_RETURN void 
#define EVSpace_Mimultm_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_multiply (double) */
#define EVSpace_Mimultd_NUM 9 + API_NUM_OFFSET
#define EVSpace_Mimultd_RETURN void 
#define EVSpace_Mimultd_PROTO (EMatrix* lhs, double rhs)
/* nb_true_divide */
#define EVSpace_Mdiv_NUM 10 + API_NUM_OFFSET
#define EVSpace_Mdiv_RETURN void 
#define EVSpace_Mdiv_PROTO (EMatrix* ans, const EMatrix* lhs, double rhs)
/* nb_inplace_true_divide */
#define EVSpace_Midiv_NUM 11 + API_NUM_OFFSET
#define EVSpace_Midiv_RETURN void 
#define EVSpace_Midiv_PROTO (EMatrix* lhs, double rhs)
/* equal to */
#define EVSpace_Mqe_NUM 12 + API_NUM_OFFSET
#define EVSpace_Meq_RETURN int 
#define EVSpace_Meq_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* not equal to */
#define EVSpace_Mne_NUM 13 + API_NUM_OFFSET
#define EVSpace_Mne_RETURN int 
#define EVSpace_Mne_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* det */
#define EVSpace_Det_NUM 14 + API_NUM_OFFSET
#define EVSpace_Det_RETURN double 
#define EVSpace_Det_PROTO (const EMatrix* lhs)
/* transpose */
#define EVSpace_Trans_NUM 15 + API_NUM_OFFSET
#define EVSpace_Trans_RETURN void 
#define EVSpace_Trans_PROTO (EMatrix* ans, const EMatrix* mat)
/* set */
#define EVSpace_Mset_NUM 16 + API_NUM_OFFSET
#define EVSpace_Mset_RETURN void
#define EVSpace_Mset_PROTO (EMatrix* self, int i, int j, double val)

#define EVSpace_EMatrix_API_COUNT 17

#endif // EVSPACE_MATRIX_DEGN_H