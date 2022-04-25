#ifndef EVSPACE_MODULE_H
#define EVSPACE_MODULE_H

#ifdef __cplusplus
extern "C" {
#else

#ifndef _BOOL
typedef unsigned char bool;
static const bool False = 0;
static const bool True = 1;
#endif

#endif

/*	nb_add	*/
#define EVSpace_Add_NUM 0
#define EVSpace_Add_RETURN void
#define EVSpace_Add_PROTO (EVector* ans, const EVector* lhs, const EVector* rhs)
/*	nb_subtract	 */
#define EVSpace_Sub_NUM 1
#define EVSpace_Sub_RETURN void
#define EVSpace_Sub_PROTO (EVector* ans, const EVector* lhs, const EVector* rhs)
/*	nb_mult  */
#define EVSpace_Mult_NUM 2
#define EVSpace_Mult_RETURN void
#define EVSpace_Mult_PROTO (EVector* ans, const EVector* lhs, const EVector* rhs)
/*	nb_negative  */
#define EVSpace_Neg_NUM 3
#define EVSpace_Neg_RETURN void
#define EVSpace_Neg_PROTO (EVector* ans, const EVector* vec)
/*	nb_absolute  */		// magnitude ?
#define EVSpace_Abs_NUM 4
#define EVSpace_Abs_RETURN double
#define EVSpace_Abs_PROTO (const EVector* vec)
/*	nb_inplace_add  */
#define EVSpace_Iadd_NUM 5
#define EVSpace_Iadd_RETURN void
#define EVSpace_Iadd_PROTO (EVector* lhs, const EVector* rhs)
/*	nb_inplace_subtract  */
#define EVSpace_Isub_NUM 6
#define EVSpace_Isub_RETURN void
#define EVSpace_Isub_PROTO (EVector* lhs, const EVector* rhs)
/*	nb_inplace_multiply  */
#define EVSpace_Imult_NUM 7
#define EVSpace_Imult_RETURN void
#define EVSpace_Imult_PROTO (EVector* lhs, const EVector* rhs)
/*	nb_true_divide  */
#define EVSpace_Div_NUM 8
#define EVSpace_Div_RETURN void
#define EVSpace_Div_PROTO (EVector* ans, const EVector* lhs, double rhs)
/*	nb_inplace_true_divide  */
#define EVSpace_Idiv_NUM 9
#define EVSpace_Idiv_RETURN void
#define EVSpace_Idiv_PROTO (EVector* lhs, double rhs)
/*  equal to  */
#define EVSpace_ET_NUM 10
#define EVSpace_ET_RETURN bool
#define EVSpace_ET_PROTO (const EVector* lhs, const EVector* rhs)
/*  not equal to  */
#define EVSpace_NE_NUM 11
#define EVSpace_NE_RETURN bool
#define EVSpace_NE_PROTO (const EVector* lhs, const EVector* rhs)
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

#define EVSpace_API_pointers 20

#ifdef EVSPACE_MODULE
	static EVSpace_Add_RETURN EVector_Add EVSpace_Add_PROTO;
	static EVSpace_Sub_RETURN EVector_Sub EVSpace_Sub_PROTO;
	static EVSpace_Mult_RETURN EVector_Mult EVSpace_Mult_PROTO;
	static EVSpace_Neg_RETURN EVector_Neg EVSpace_Neg_PROTO;
	static EVSpace_Abs_RETURN EVector_Abs EVSpace_Abs_PROTO;
	static EVSpace_Iadd_RETURN EVector_Iadd EVSpace_Iadd_PROTO;
	static EVSpace_Isub_RETURN EVector_Isub EVSpace_Isub_PROTO;
	static EVSpace_Imult_RETURN EVector_Imult EVSpace_Imult_PROTO;
	static EVSpace_Div_RETURN EVector_Div EVSpace_Div_PROTO;
	static EVSpace_Idiv_RETURN EVector_Idiv EVSpace_Idiv_PROTO;
	static EVSpace_ET_RETURN EVector_ET EVSpace_ET_PROTO;
	static EVSpace_NE_RETURN EVector_NE EVSpace_Neg_PROTO;
	static EVSpace_Dot_RETURN EVector_Dot EVSpace_Dot_PROTO;
	static EVSpace_Cross_RETURN EVector_Cross EVSpace_Cross_PROTO;
	static EVSpace_Mag_RETURN EVector_Mag EVSpace_Mag_PROTO;
	static EVSpace_Mag2_RETURN EVector_Mag2 EVSpace_Mag2_PROTO;
	static EVSpace_Norm_RETURN EVector_Norm EVSpace_Norm_PROTO;
	static EVSpace_Inorm_RETURN EVector_Inorm EVSpace_Inorm_PROTO;
	static EVSpace_Vang_RETURN EVector_Vang EVSpace_Vang_PROTO;
	static EVSpace_Vxcl_RETURN EVector_Vxcl EVSpace_Vxcl_PROTO;
#else
	static void** EVSpace_API;

#define EVSpace_Add \
	(*(EVSpace_Add_RETURN) (*)EVSpace_Add_PROTO) EVSpace_API[EVSpace_Add_NUM]
#define EVSpace_Sub \
	(*(EVSpace_Sub_RETURN) (*)EVSpace_Sub_PROTO) EVSpace_API[EVSpace_Sub_NUM]
#define EVSpace_Mult \
	(*(EVSpace_Mult_RETURN) (*)EVSpace_Mult_PROTO) EVSpace_API[EVSpace_Mult_NUM]
#define EVSpace_Neg \
	(*(EVSpace_Neg_RETURN) (*)EVSpace_Neg_PROTO) EVSpace_API[EVSpace_Neg_NUM]
#define EVSpace_Abs \
	(*(EVSpace_Abs_RETURN) (*)EVSpace_Abs_PROTO) EVSpace_API[EVSpace_Abs_NUM]
#define EVSpace_Iadd \
	(*(EVSpace_Iadd_RETURN) (*)EVSpace_Iadd_PROTO) EVSpace_API[EVSpace_Iadd_NUM]
#define EVSpace_Isub \
	(*(EVSpace_Isub_RETURN) (*)EVSpace_Isub_PROTO) EVSpace_API[EVSpace_Isub_NUM]
#define EVSpace_Imult \
	(*(EVSpace_Imult_RETURN) (*)EVSpace_Imult_PROTO) EVSpace_API[EVSpace_Imult_NUM]
#define EVSpace_Div \
	(*(EVSpace_Div_RETURN) (*)EVSpace_Div_PROTO) EVSpace_API[EVSpace_Div_NUM]
#define EVSpace_Idiv \
	(*(EVSpace_Idiv_RETURN) (*)EVSpace_Idiv_PROTO) EVSpace_API[EVSpace_Idiv_NUM]
#define EVSpace_ET \
	(*(EVSpace_ET_RETURN) (*)EVSpace_ET_PROTO) EVSpace_API[EVSpace_ET_NUM]
#define EVSpace_NE \
	(*(EVSpace_NE_RETURN) (*)EVSpace_NE_PROTO) EVSpace_API[EVSpace_NE_NUM]
#define EVSpace_Dot \
	(*(EVSpace_Dot_RETURN) (*)EVSpace_Dot_PROTO) EVSpace_API[EVSpace_Dot_NUM]
#define EVSpace_Cross \
	(*(EVSpace_Cross_RETURN) (*)EVSpace_Cross_PROTO) EVSpace_API[EVSpace_Cross_NUM]
#define EVSpace_Mag \
	(*(EVSpace_Mag_RETURN) (*)EVSpace_Mag_PROTO) EVSpace_API[EVSpace_Mag_NUM]
#define EVSpace_Mag2 \
	(*(EVSpace_Mag2_RETURN) (*)EVSpace_Mag2_PROTO) EVSpace_API[EVSpace_Mag2_NUM]
#define EVSpace_Norm \
	(*(EVSpace_Norm_RETURN) (*)EVSpace_Norm_PROTO) EVSpace_API[EVSpace_Norm_NUM]
#define EVSpace_Inorm \
	(*(EVSpace_Inorm_RETURN) (*)EVSpace_Inorm_PROTO) EVSpace_API[EVSpace_Inorm_NUM]
#define EVSpace_Vang \
	(*(EVSpace_Vang_RETURN) (*)EVSpace_Vang_PROTO) EVSpace_API[EVSpace_Vang_NUM]
#define EVSpace_Vxcl \
	(*(EVSpace_Vxcl_RETURN) (*)EVSpace_Vxcl_PROTO) EVSpace_API[EVSpace_Vxcl_NUM]

	static int import_evspace(void) 
	{
		EVSpace_API = (void**)PyCapsule_Import("evspace._C_API", 0);
	}

#endif // EVSPACE_MODULE

#ifdef __cplusplus
}
#endif

#endif // EVSPACE_MODULE_H
