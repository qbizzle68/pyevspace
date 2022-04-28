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
/* nb_mult */
#define EVSpace_Mmult_NUM 2 + API_NUM_OFFSET
#define EVSpace_Mmult_RETURN void 
#define EVSpace_Mmult_PROTO (EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
/* nb_negative */
#define EVSpace_Mneg_NUM 3 + API_NUM_OFFSET
#define EVSpace_Mneg_RETURN void 
#define EVSpace_Mneg_PROTO (EMatrix* ans, const EMatrix* vec)
/* nb_absolute */
#define EVSpace_Mabs_NUM 4 + API_NUM_OFFSET
#define EVSpace_Mabs_RETURN double 
#define EVSpace_Mabs_PROTO (const EMatrix* ans)
/* nb_inplace_add */
#define EVSpace_Miadd_NUM 5 + API_NUM_OFFSET
#define EVSpace_Miadd_RETURN void 
#define EVSpace_Miadd_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_subtract */
#define EVSpace_Misub_NUM 6 + API_NUM_OFFSET
#define EVSpace_Misub_RETURN void 
#define EVSpace_Misub_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_multiply */
#define EVSpace_Mimult_NUM 7 + API_NUM_OFFSET
#define EVSpace_Mimult_RETURN void 
#define EVSpace_Mimult_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* nb_true_divide */
#define EVSpace_Mdiv_NUM 8 + API_NUM_OFFSET
#define EVSpace_Mdiv_RETURN void 
#define EVSpace_Mdiv_PROTO (EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
/* nb_inplace_true_divide */
#define EVSpace_Midiv_NUM 9 + API_NUM_OFFSET
#define EVSpace_Midiv_RETURN void 
#define EVSpace_Midiv_PROTO (EMatrix* lhs, const EMatrix* rhs)
/* equal to */
#define EVSpace_Mqe_NUM 10 + API_NUM_OFFSET
#define EVSpace_Meq_RETURN int 
#define EVSpace_Meq_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* not equal to */
#define EVSpace_Mne_NUM 11 + API_NUM_OFFSET
#define EVSpace_Mne_RETURN int 
#define EVSpace_Mne_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* det */
#define EVSpace_Det_NUM 12 + API_NUM_OFFSET
#define EVSpace_Det_RETURN double 
#define EVSpace_Det_PROTO (const EMatrix* lhs, const EMatrix* rhs)
/* transpose */
#define EVSpace_Trans_NUM 13 + API_NUM_OFFSET
#define EVSpace_Trans_RETURN void 
#define EVSpace_Trans_PROTO (EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
/* set */
#define EVSpace_Mset_NUM 14 + API_NUM_OFFSET
#define EVSpace_Mset_RETURN void
#define EVSpace_Mset_PROTO (EMatrix* self, const EVector* c0, const EVector* c1, const EVector* c2)

#define EVSpace_EMatrix_API_COUNT 15

#endif // EVSPACE_MATRIX_DEGN_H