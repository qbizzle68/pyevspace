/* definitions for the api for matrix methods in pyevspace module */
#ifndef EVSPACE_MATRIX_API_DEFN_H
#define EVSPACE_MATRIX_API_DEFN_H

/* these definitions include macro definitions from evspace_matrix_defn.h
	include that file before this one */
/* the api array must be declared before include of this file */

#define EVSpace_Madd \
	(*(EVSpace_Madd_RETURN) (*)EVSpace_Madd_PROTO) EVSpace_API[EVSpace_Madd_NUM]
#define EVSpace_Msub \
	(*(EVSpace_Msub_RETURN) (*)EVSpace_Msub_PROTO) EVSpace_API[EVSpace_Msub_NUM]
#define EVSpace_Mmult \
	(*(EVSpace_Mmult_RETURN) (*)EVSpace_Mmult_PROTO) EVSpace_API[EVSpace_Mmult_NUM]
#define EVSpace_Mneg \
	(*(EVSpace_Mneg_RETURN) (*)EVSpace_Mneg_PROTO) EVSpace_API[EVSpace_Mneg_NUM]
#define EVSpace_Mabs \
	(*(EVSpace_Mabs_RETURN) (*)EVSpace_Mabs_PROTO) EVSpace_API[EVSpace_Mabs_NUM]
#define EVSpace_Miadd \
	(*(EVSpace_Miadd_RETURN) (*)EVSpace_Miadd_PROTO) EVSpace_API[EVSpace_Miadd_NUM]
#define EVSpace_Misub \
	(*(EVSpace_Misub_RETURN) (*)EVSpace_Misub_PROTO) EVSpace_API[EVSpace_Misub_NUM]
#define EVSpace_Mimult \
	(*(EVSpace_Mimult_RETURN) (*)EVSpace_Mimult_PROTO) EVSpace_API[EVSpace_Mimult_NUM]
#define EVSpace_Mdiv \
	(*(EVSpace_Mdiv_RETURN) (*)EVSpace_Mdiv_PROTO) EVSpace_API[EVSpace_Mdiv_NUM]
#define EVSpace_Midiv \
	(*(EVSpace_Midiv_RETURN) (*)EVSpace_Midiv_PROTO) EVSpace_API[EVSpace_Midiv_NUM]
#define EVSpace_Meq \
	(*(EVSpace_Meq_RETURN) (*)EVSpace_Meq_PROTO) EVSpace_API[EVSpace_Meq_NUM]
#define EVSpace_Mne \
	(*(EVSpace_Mne_RETURN) (*)EVSpace_Mne_PROTO) EVSpace_API[EVSpace_Mne_NUM]
#define EVSpace_Det \
	(*(EVSpace_Det_RETURN) (*)EVSpace_Det_PROTO) EVSpace_API[EVSpace_Det_NUM]
#define EVSpace_Trans \
	(*(EVSpace_Trans_RETURN) (*)EVSpace_Trans_PROTO) EVSpace_API[EVSpace_Trans_NUM]
#define EVSpace_Mset \
	(*(EVSpace_Mset_RETURN) (*)EVSpace_Mset_PROTO) EVSpace_API[EVSpace_Mset_NUM]

#endif // EVSPACE_MATRIX_API_DEFN_H