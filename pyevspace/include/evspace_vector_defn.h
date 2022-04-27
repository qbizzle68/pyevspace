/* defines for the evector methods in the pyevspace c api */
#ifndef EVSPACE_VECTOR_DEFN_H
#define EVSPACE_VECTOR_DEFN_H

/*	nb_add	*/
#define EVSpace_Vadd_NUM 0
#define EVSpace_Vadd_RETURN void
#define EVSpace_Vadd_PROTO (EVector* ans, const EVector* lhs, const EVector* rhs)
/*	nb_subtract	 */
#define EVSpace_Vsub_NUM 1
#define EVSpace_Vsub_RETURN void
#define EVSpace_Vsub_PROTO (EVector* ans, const EVector* lhs, const EVector* rhs)
/*	nb_mult  */
#define EVSpace_Vmult_NUM 2
#define EVSpace_Vmult_RETURN void
#define EVSpace_Vmult_PROTO (EVector* ans, const EVector* lhs, double rhs)
/*	nb_negative  */
#define EVSpace_Vneg_NUM 3
#define EVSpace_Vneg_RETURN void
#define EVSpace_Vneg_PROTO (EVector* ans, const EVector* vec)
/*	nb_absolute  */		// magnitude ?
#define EVSpace_Vabs_NUM 4
#define EVSpace_Vabs_RETURN double
#define EVSpace_Vabs_PROTO (const EVector* vec)
/*	nb_inplace_add  */
#define EVSpace_Viadd_NUM 5
#define EVSpace_Viadd_RETURN void
#define EVSpace_Viadd_PROTO (EVector* lhs, const EVector* rhs)
/*	nb_inplace_subtract  */
#define EVSpace_Visub_NUM 6
#define EVSpace_Visub_RETURN void
#define EVSpace_Visub_PROTO (EVector* lhs, const EVector* rhs)
/*	nb_inplace_multiply  */
#define EVSpace_Vimult_NUM 7
#define EVSpace_Vimult_RETURN void
#define EVSpace_Vimult_PROTO (EVector* lhs, double rhs)
/*	nb_true_divide  */
#define EVSpace_Vdiv_NUM 8
#define EVSpace_Vdiv_RETURN void
#define EVSpace_Vdiv_PROTO (EVector* ans, const EVector* lhs, double rhs)
/*	nb_inplace_true_divide  */
#define EVSpace_Vidiv_NUM 9
#define EVSpace_Vidiv_RETURN void
#define EVSpace_Vidiv_PROTO (EVector* lhs, double rhs)
/*  equal to  */
#define EVSpace_Veq_NUM 10
#define EVSpace_Veq_RETURN int
#define EVSpace_Veq_PROTO (const EVector* lhs, const EVector* rhs)
/*  not equal to  */
#define EVSpace_Vne_NUM 11
#define EVSpace_Vne_RETURN int
#define EVSpace_Vne_PROTO (const EVector* lhs, const EVector* rhs)
/*  dot  */
#define EVSpace_Dot_NUM 12
#define EVSpace_Dot_RETURN double
#define EVSpace_Dot_PROTO (const EVector* lhs, const EVector* rhs)
/*  cross  */
#define EVSpace_Cross_NUM 13
#define EVSpace_Cross_RETURN void
#define EVSpace_Cross_PROTO (EVector* and, const EVector* lhs, const EVector* rhs)
/*  magnitude  */
#define EVSpace_Mag_NUM 14
#define EVSpace_Mag_RETURN double
#define EVSpace_Mag_PROTO (const EVector* vec)
/*  magnitude squared  */
#define EVSpace_Mag2_NUM 15
#define EVSpace_Mag2_RETURN double
#define EVSpace_Mag2_PROTO (const EVector* vec)
/*  norm  */
#define EVSpace_Norm_NUM 16
#define EVSpace_Norm_RETURN void
#define EVSpace_Norm_PROTO (EVector* ans, const EVector* vec)
/*  inplace norm  */
#define EVSpace_Inorm_NUM 17
#define EVSpace_Inorm_RETURN void
#define EVSpace_Inorm_PROTO (EVector* ans)
/*  vector angle  */
#define EVSpace_Vang_NUM 18
#define EVSpace_Vang_RETURN double
#define EVSpace_Vang_PROTO (const EVector* lhs, const EVector* rhs)
/*  vector exclude  */
#define EVSpace_Vxcl_NUM 19
#define EVSpace_Vxcl_RETURN void
#define EVSpace_Vxcl_PROTO (EVector* ans, const EVector* vec, const EVector* xcl)

#define EVSpace_EVector_API_COUNT 20

#endif // EVSPACE_VECTOR_DEFN_H