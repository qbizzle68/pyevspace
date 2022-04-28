/* declarations of ematrix metods in te pyevspace c api */
#ifndef EVSPACE_MATRIX_DECL_H
#define EVSPACE_MATRIX_DECL_H

/* te macro definitions are found in evspace_matrix_defn.h
	include that file before this one */

static EVSpace_Madd_RETURN EVSpace_Madd EVSpace_Madd_PROTO;
static EVSpace_Msub_RETURN EVSpace_Msub EVSpace_Msub_PROTO;
static EVSpace_Mmultm_RETURN EVSpace_Mmultm EVSpace_Mmultm_PROTO;
static EVSpace_Mmultv_RETURN EVSpace_Mmultv EVSpace_Mmultv_PROTO;
static EVSpace_Mmultd_RETURN EVSpace_Mmultd EVSpace_Mmultd_PROTO;
static EVSpace_Mneg_RETURN EVSpace_Mneg EVSpace_Mneg_PROTO;
static EVSpace_Miadd_RETURN EVSpace_Miadd EVSpace_Miadd_PROTO;
static EVSpace_Misub_RETURN EVSpace_Misub EVSpace_Misub_PROTO;
static EVSpace_Mimultm_RETURN EVSpace_Mimultm EVSpace_Mimultm_PROTO;
static EVSpace_Mimultd_RETURN EVSpace_Mimultd EVSpace_Mimultd_PROTO;
static EVSpace_Mdiv_RETURN EVSpace_Mdiv EVSpace_Mdiv_PROTO;
static EVSpace_Midiv_RETURN EVSpace_Midiv EVSpace_Midiv_PROTO;
static EVSpace_Meq_RETURN EVSpace_Meq EVSpace_Meq_PROTO;
static EVSpace_Mne_RETURN EVSpace_Mne EVSpace_Mne_PROTO;
static EVSpace_Det_RETURN EVSpace_Det EVSpace_Det_PROTO;
static EVSpace_Trans_RETURN EVSpace_Trans EVSpace_Trans_PROTO;
static EVSpace_Mset_RETURN EVSpace_Mset EVSpace_Mset_PROTO;

#endif // EVSPACE_MATRIX_DECL_H