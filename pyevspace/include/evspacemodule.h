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

/********************* EVector methods ***********************/
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
#define EVSpace_Veq_RETURN bool
#define EVSpace_Veq_PROTO (const EVector* lhs, const EVector* rhs)
/*  not equal to  */
#define EVSpace_Vne_NUM 11
#define EVSpace_Vne_RETURN bool
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

/**************************** EMatrix methods ****************************/

#define EVSpace_API_pointers 20

#ifdef EVSPACE_MODULE
	static EVSpace_Vadd_RETURN EVSpace_Vadd EVSpace_Vadd_PROTO;
	static EVSpace_Vsub_RETURN EVSpace_Vsub EVSpace_Vsub_PROTO;
	static EVSpace_Vmult_RETURN EVSpace_Vmult EVSpace_Vmult_PROTO;
	static EVSpace_Vneg_RETURN EVSpace_Vneg EVSpace_Vneg_PROTO;
	static EVSpace_Vabs_RETURN EVSpace_Vabs EVSpace_Vabs_PROTO;
	static EVSpace_Viadd_RETURN EVSpace_Viadd EVSpace_Viadd_PROTO;
	static EVSpace_Visub_RETURN EVSpace_Visub EVSpace_Visub_PROTO;
	static EVSpace_Vimult_RETURN EVSpace_Vimult EVSpace_Vimult_PROTO;
	static EVSpace_Vdiv_RETURN EVSpace_Vdiv EVSpace_Vdiv_PROTO;
	static EVSpace_Vidiv_RETURN EVSpace_Vidiv EVSpace_Vidiv_PROTO;
	static EVSpace_Veq_RETURN EVSpace_Veq EVSpace_Veq_PROTO;
	static EVSpace_Vne_RETURN EVSpace_Vne EVSpace_Vne_PROTO;
	static EVSpace_Dot_RETURN EVSpace_Dot EVSpace_Dot_PROTO;
	static EVSpace_Cross_RETURN EVSpace_Cross EVSpace_Cross_PROTO;
	static EVSpace_Mag_RETURN EVSpace_Mag EVSpace_Mag_PROTO;
	static EVSpace_Mag2_RETURN EVSpace_Mag2 EVSpace_Mag2_PROTO;
	static EVSpace_Norm_RETURN EVSpace_Norm EVSpace_Norm_PROTO;
	static EVSpace_Inorm_RETURN EVSpace_Inorm EVSpace_Inorm_PROTO;
	static EVSpace_Vang_RETURN EVSpace_Vang EVSpace_Vang_PROTO;
	static EVSpace_Vxcl_RETURN EVSpace_Vxcl EVSpace_Vxcl_PROTO;
#else
	static void** EVSpace_API;

#define EVSpace_Vadd \
	(*(EVSpace_Vadd_RETURN) (*)EVSpace_Vadd_PROTO) EVSpace_API[EVSpace_Vadd_NUM]
#define EVSpace_Vsub \
	(*(EVSpace_Vsub_RETURN) (*)EVSpace_Vsub_PROTO) EVSpace_API[EVSpace_Vsub_NUM]
#define EVSpace_Vmult \
	(*(EVSpace_Vmult_RETURN) (*)EVSpace_Vmult_PROTO) EVSpace_API[EVSpace_Vmult_NUM]
#define EVSpace_Vneg \
	(*(EVSpace_Vneg_RETURN) (*)EVSpace_Vneg_PROTO) EVSpace_API[EVSpace_Vneg_NUM]
#define EVSpace_Vabs \
	(*(EVSpace_Vabs_RETURN) (*)EVSpace_Vabs_PROTO) EVSpace_API[EVSpace_Vabs_NUM]
#define EVSpace_Viadd \
	(*(EVSpace_Viadd_RETURN) (*)EVSpace_Viadd_PROTO) EVSpace_API[EVSpace_Viadd_NUM]
#define EVSpace_Visub \
	(*(EVSpace_Visub_RETURN) (*)EVSpace_Visub_PROTO) EVSpace_API[EVSpace_Visub_NUM]
#define EVSpace_Vimult \
	(*(EVSpace_Vimult_RETURN) (*)EVSpace_Vimult_PROTO) EVSpace_API[EVSpace_Vimult_NUM]
#define EVSpace_Vdiv \
	(*(EVSpace_Vdiv_RETURN) (*)EVSpace_Vdiv_PROTO) EVSpace_API[EVSpace_Vdiv_NUM]
#define EVSpace_Vidiv \
	(*(EVSpace_Vidiv_RETURN) (*)EVSpace_Vidiv_PROTO) EVSpace_API[EVSpace_Vidiv_NUM]
#define EVSpace_Veq \
	(*(EVSpace_Veq_RETURN) (*)EVSpace_Veq_PROTO) EVSpace_API[EVSpace_Veq_NUM]
#define EVSpace_Vne \
	(*(EVSpace_Vne_RETURN) (*)EVSpace_Vne_PROTO) EVSpace_API[EVSpace_Vne_NUM]
#define EVSpace_Vne \
	(*(EVSpace_Vne_RETURN) (*)EVSpace_Vne_PROTO) EVSpace_API[EVSpace_Vne_NUM]
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
