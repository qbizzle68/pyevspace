#ifndef EVSPACE_MODULE_H
#define EVSPACE_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#define EVSpace_BINARY (const EVector* lhs, const EVector* rhs)

/*	nb_add	*/
#define EVSpace_Add_NUM 0
#define EVSpace_Add_RETURN EVector*
#define EVSpace_Add_PROTO (const EVector* lhs, const EVector* rhs)
/*	nb_subtract	 */
#define EVSpace_Sub_NUM 1
#define EVSpace_Sub_RETURN EVector*
#define EVSpace_Sub_PROTO EVSpace_BINARY
/*	nb_mult  */
#define EVSpace_Mult_NUM 2
#define EVSpace_Mult_RETURN EVector*
#define EVSpace_Mult_PROTO EVSpace_BINARY
/*	nb_negative  */
#define EVSpace_Neg_NUM 3
#define EVSpace_Neg_RETURN EVector*
#define EVSpace_Neg_PROTO (const EVector* vec)
/*	nb_absolute  */		// magnitude ?
#define EVSpace_Abs_NUM 4
#define EVSpace_Abs_RETURN double
#define EVSpace_Abs_PROTO (const EVector* vec)
/*	nb_inplace_add  */
#define EVSpace_Iadd_NUM 5
#define EVSpace_Iadd_RETURN EVector*
#define EVSpace_Iadd_PROTO EVSpace_BINARY
/*	nb_inplace_subtract  */
#define EVSpace_Isub_NUM 6
#define EVSpace_Isub_RETURN EVector*
#define EVSpace_Isub_PROTO EVSpace_BINARY
/*	nb_inplace_multiply  */
#define EVSpace_Imult_NUM 7
#define EVSpace_Imult_RETURN EVector*
#define EVSpace_Imult_PROTO EVSpace_BINARY
/*	nb_true_divide  */
#define EVSpace_Div_NUM 8
#define EVSpace_Div_RETURN EVector*
#define EVSpace_Div_PROTO (const EVector* lhs, double rhs)
/*	nb_inplace_true_divide  */
#define EVSpace_Idiv_NUM 9
#define EVSpace_Idiv_RETURN EVector*
#define EVSpace_Idiv_PROTO (const EVector* lhs, double rhs)

#define EVSpace_API_pointers 9

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
#else
	static void** EVector_API;

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

	static int import_evspace(void) 
	{
		EVSpace_API = (void**)PyCapsule_Import("evspace._C_API", 0);
	}

#endif // EVSPACE_MODULE

#ifdef __cplusplus
}
#endif

#endif // EVSPACE_MODULE_H
