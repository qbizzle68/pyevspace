#ifndef EVSPACE_MODULE_H
#define EVSPACE_MODULE_H

#ifdef __cplusplus
extern "C" {
#else

#endif

// EVector method definitions
#include "evspace_vector_defn.h"
#include "evspace_matrix_defn.h"

/**************************** EMatrix methods ****************************/

#define EVSpace_API_pointers \
			EVSpace_EVector_API_COUNT + EVSpace_EMatrix_API_COUNT

#ifdef EVSPACE_MODULE

// EVector method declarations
#include "evspace_vector_decl.h"
#include "evspace_matrix_decl.h"

#else
	static void** EVSpace_API;
	
// EVector method api definitions
#include "evspace_vector_api_defn.h"
#include "evspace_matrix_api_defn.h"

	static int import_evspace(void) 
	{
		EVSpace_API = (void**)PyCapsule_Import("evspace._C_API", 0);
	}

#endif // EVSPACE_MODULE

#ifdef __cplusplus
}
#endif

#endif // EVSPACE_MODULE_H
