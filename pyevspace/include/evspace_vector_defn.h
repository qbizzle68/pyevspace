/* defines for the evector methods in the pyevspace c api */
#ifndef EVSPACE_VECTOR_DEFN_H
#define EVSPACE_VECTOR_DEFN_H

/*	nb_add	*/
#define EVSpace_Vector_add_NUM 0
#define EVSpace_Vector_add_RETURN EVector*
#define EVSpace_Vector_add_PROTO (const EVector* lhs, const EVector* rhs)
/*	nb_subtract	 */
#define EVSpace_Vector_sub_NUM 1
#define EVSpace_Vector_sub_RETURN EVector*
#define EVSpace_Vector_sub_PROTO (const EVector* lhs, const EVector* rhs)
/*	nb_mult  */
#define EVSpace_Vector_mult_NUM 2
#define EVSpace_Vector_mult_RETURN EVector*
#define EVSpace_Vector_mult_PROTO (const EVector* lhs, double rhs)
/*	nb_negative  */
#define EVSpace_Vector_neg_NUM 3
#define EVSpace_Vector_neg_RETURN EVector*
#define EVSpace_Vector_neg_PROTO (const EVector* vec)
/*	nb_absolute  */		// magnitude ?
#define EVSpace_Vector_abs_NUM 4
#define EVSpace_Vector_abs_RETURN double
#define EVSpace_Vector_abs_PROTO (const EVector* vec)
/*	nb_inplace_add  */
#define EVSpace_Vector_iadd_NUM 5
#define EVSpace_Vector_iadd_RETURN void
#define EVSpace_Vector_iadd_PROTO (EVector* lhs, const EVector* rhs)
/*	nb_inplace_subtract  */
#define EVSpace_Vector_isub_NUM 6
#define EVSpace_Vector_isub_RETURN void
#define EVSpace_Vector_isub_PROTO (EVector* lhs, const EVector* rhs)
/*	nb_inplace_multiply  */
#define EVSpace_Vector_imult_NUM 7
#define EVSpace_Vector_imult_RETURN void
#define EVSpace_Vector_imult_PROTO (EVector* lhs, double rhs)
/*	nb_true_divide  */
#define EVSpace_Vector_div_NUM 8
#define EVSpace_Vector_div_RETURN EVector*
#define EVSpace_Vector_div_PROTO (const EVector* lhs, double rhs)
/*	nb_inplace_true_divide  */
#define EVSpace_Vector_idiv_NUM 9
#define EVSpace_Vector_idiv_RETURN void
#define EVSpace_Vector_idiv_PROTO (EVector* lhs, double rhs)
/*  equal to  */
#define EVSpace_Vector_eq_NUM 10
#define EVSpace_Vector_eq_RETURN int
#define EVSpace_Vector_eq_PROTO (const EVector* lhs, const EVector* rhs)
/*  not equal to  */
#define EVSpace_Vector_ne_NUM 11
#define EVSpace_Vector_ne_RETURN int
#define EVSpace_Vector_ne_PROTO (const EVector* lhs, const EVector* rhs)
/*  dot  */
#define EVSpace_Vector_dot_NUM 12
#define EVSpace_Vector_dot_RETURN double
#define EVSpace_Vector_dot_PROTO (const EVector* lhs, const EVector* rhs)
/*  cross  */
#define EVSpace_Vector_cross_NUM 13
#define EVSpace_Vector_cross_RETURN EVector*
#define EVSpace_Vector_cross_PROTO (const EVector* lhs, const EVector* rhs)
/*  magnitude  */
#define EVSpace_Vector_mag_NUM 14
#define EVSpace_Vector_mag_RETURN double
#define EVSpace_Vector_mag_PROTO (const EVector* vec)
/*  magnitude squared  */
#define EVSpace_Vector_mag2_NUM 15
#define EVSpace_Vector_mag2_RETURN double
#define EVSpace_Vector_mag2_PROTO (const EVector* vec)
/*  norm  */
#define EVSpace_Vector_norm_NUM 16
#define EVSpace_Vector_norm_RETURN EVector*
#define EVSpace_Vector_norm_PROTO (const EVector* vec)
/*  inplace norm  */
#define EVSpace_Vector_inorm_NUM 17
#define EVSpace_Vector_inorm_RETURN void
#define EVSpace_Vector_inorm_PROTO (EVector* ans)
/*  vector angle  */
#define EVSpace_Vector_vang_NUM 18
#define EVSpace_Vector_vang_RETURN double
#define EVSpace_Vector_vang_PROTO (const EVector* lhs, const EVector* rhs)
/*  vector exclude  */
#define EVSpace_Vector_vxcl_NUM 19
#define EVSpace_Vector_vxcl_RETURN EVector*
#define EVSpace_Vector_vxcl_PROTO (const EVector* vec, const EVector* xcl)

#define EVSpace_EVector_API_COUNT 20

#endif // EVSPACE_VECTOR_DEFN_H