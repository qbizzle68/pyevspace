/* definitions for the api for vector methods in pyevspace module */
#ifndef EVSPACE_VECTOR_API_DEFN_H
#define EVSPACE_VECTOR_API_DEFN_H

/* these definitions include macro definitions from evspace_vector_defn.h
	include that file before this one */
/* the api array must be declared before inclusion of this file */

#define EVSpace_Vector_add \
	(*(EVSpace_Vadd_RETURN) (*)EVSpace_Vadd_PROTO) EVSpace_API[EVSpace_Vadd_NUM]
#define EVSpace_Vector_sub \
	(*(EVSpace_Vsub_RETURN) (*)EVSpace_Vsub_PROTO) EVSpace_API[EVSpace_Vsub_NUM]
#define EVSpace_Vector_mult \
	(*(EVSpace_Vmult_RETURN) (*)EVSpace_Vmult_PROTO) EVSpace_API[EVSpace_Vmult_NUM]
#define EVSpace_Vector_neg \
	(*(EVSpace_Vneg_RETURN) (*)EVSpace_Vneg_PROTO) EVSpace_API[EVSpace_Vneg_NUM]
#define EVSpace_Vector_abs \
	(*(EVSpace_Vabs_RETURN) (*)EVSpace_Vabs_PROTO) EVSpace_API[EVSpace_Vabs_NUM]
#define EVSpace_Vector_iadd \
	(*(EVSpace_Viadd_RETURN) (*)EVSpace_Viadd_PROTO) EVSpace_API[EVSpace_Viadd_NUM]
#define EVSpace_Vector_isub \
	(*(EVSpace_Visub_RETURN) (*)EVSpace_Visub_PROTO) EVSpace_API[EVSpace_Visub_NUM]
#define EVSpace_Vector_imult \
	(*(EVSpace_Vimult_RETURN) (*)EVSpace_Vimult_PROTO) EVSpace_API[EVSpace_Vimult_NUM]
#define EVSpace_Vector_div \
	(*(EVSpace_Vdiv_RETURN) (*)EVSpace_Vdiv_PROTO) EVSpace_API[EVSpace_Vdiv_NUM]
#define EVSpace_Vector_idiv \
	(*(EVSpace_Vidiv_RETURN) (*)EVSpace_Vidiv_PROTO) EVSpace_API[EVSpace_Vidiv_NUM]
#define EVSpace_Vector_eq \
	(*(EVSpace_Veq_RETURN) (*)EVSpace_Veq_PROTO) EVSpace_API[EVSpace_Veq_NUM]
#define EVSpace_Vector_ne \
	(*(EVSpace_Vne_RETURN) (*)EVSpace_Vne_PROTO) EVSpace_API[EVSpace_Vne_NUM]
#define EVSpace_Vector_dot \
	(*(EVSpace_Dot_RETURN) (*)EVSpace_Dot_PROTO) EVSpace_API[EVSpace_Dot_NUM]
#define EVSpace_Vector_cross \
	(*(EVSpace_Cross_RETURN) (*)EVSpace_Cross_PROTO) EVSpace_API[EVSpace_Cross_NUM]
#define EVSpace_vector_mag \
	(*(EVSpace_Mag_RETURN) (*)EVSpace_Mag_PROTO) EVSpace_API[EVSpace_Mag_NUM]
#define EVSpace_Vector_mag2 \
	(*(EVSpace_Mag2_RETURN) (*)EVSpace_Mag2_PROTO) EVSpace_API[EVSpace_Mag2_NUM]
#define EVSpace_Vector_norm \
	(*(EVSpace_Norm_RETURN) (*)EVSpace_Norm_PROTO) EVSpace_API[EVSpace_Norm_NUM]
#define EVSpace_Vector_inorm \
	(*(EVSpace_Inorm_RETURN) (*)EVSpace_Inorm_PROTO) EVSpace_API[EVSpace_Inorm_NUM]
#define EVSpace_Vector_vang \
	(*(EVSpace_Vang_RETURN) (*)EVSpace_Vang_PROTO) EVSpace_API[EVSpace_Vang_NUM]
#define EVSpace_Vector_vxcl \
	(*(EVSpace_Vxcl_RETURN) (*)EVSpace_Vxcl_PROTO) EVSpace_API[EVSpace_Vxcl_NUM]

#endif // EVSPACE_VECTOR_API_DEFN_H