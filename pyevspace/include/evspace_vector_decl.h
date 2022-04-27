/* declarations of evector methods in the pyevspace c api */
#ifndef EVSPACE_VECTOR_DECL_H
#define EVSPACE_VECTOR_DECL_H

/* the macro definitions are found in evspace_vector_defn 
	include that file before this one */

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

#endif // EVSPACE_VECTOR_DECL_H