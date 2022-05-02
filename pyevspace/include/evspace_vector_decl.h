/* declarations of evector methods in the pyevspace c api */
#ifndef EVSPACE_VECTOR_DECL_H
#define EVSPACE_VECTOR_DECL_H

/* the macro definitions are found in evspace_vector_defn.h
	include that file before this one */

static EVSpace_Vadd_RETURN EVSpace_Vector_add EVSpace_Vadd_PROTO;
static EVSpace_Vsub_RETURN EVSpace_Vector_sub EVSpace_Vsub_PROTO;
static EVSpace_Vmult_RETURN EVSpace_Vector_mult EVSpace_Vmult_PROTO;
static EVSpace_Vneg_RETURN EVSpace_Vector_neg EVSpace_Vneg_PROTO;
static EVSpace_Vabs_RETURN EVSpace_Vector_abs EVSpace_Vabs_PROTO;
static EVSpace_Viadd_RETURN EVSpace_Vector_iadd EVSpace_Viadd_PROTO;
static EVSpace_Visub_RETURN EVSpace_Vector_isub EVSpace_Visub_PROTO;
static EVSpace_Vimult_RETURN EVSpace_Vector_imult EVSpace_Vimult_PROTO;
static EVSpace_Vdiv_RETURN EVSpace_Vector_div EVSpace_Vdiv_PROTO;
static EVSpace_Vidiv_RETURN EVSpace_Vector_idiv EVSpace_Vidiv_PROTO;
static EVSpace_Veq_RETURN EVSpace_Vector_eq EVSpace_Veq_PROTO;
static EVSpace_Vne_RETURN EVSpace_Vector_ne EVSpace_Vne_PROTO;
static EVSpace_Dot_RETURN EVSpace_Vector_dot EVSpace_Dot_PROTO;
static EVSpace_Cross_RETURN EVSpace_Vector_cross EVSpace_Cross_PROTO;
static EVSpace_Mag_RETURN EVSpace_vector_mag EVSpace_Mag_PROTO;
static EVSpace_Mag2_RETURN EVSpace_Vector_mag2 EVSpace_Mag2_PROTO;
static EVSpace_Norm_RETURN EVSpace_Vector_norm EVSpace_Norm_PROTO;
static EVSpace_Inorm_RETURN EVSpace_Vector_inorm EVSpace_Inorm_PROTO;
static EVSpace_Vang_RETURN EVSpace_Vector_vang EVSpace_Vang_PROTO;
static EVSpace_Vxcl_RETURN EVSpace_Vector_vxcl EVSpace_Vxcl_PROTO;

#endif // EVSPACE_VECTOR_DECL_H