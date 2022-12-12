#ifndef EVSPACEMODULE_H
#define EVSPACEMODULE_H

#include <Python.h>
#include <evspace_api.h>


#define PI		3.14159265358979323846
#define PI_2	1.57079632679489661923


#define Vector_X(o)			EVSpace_VECTOR_X(o)
#define Vector_Y(o)			EVSpace_VECTOR_Y(o)
#define Vector_Z(o)			EVSpace_VECTOR_Z(o)

#define Vector_INDEX(v, i)	(v->data[i])

#define Vector_DATA(o)		(o->data)
#define PyVector_DATA(o)	(((EVSpace_Vector*)o)->data)

#define Vector_SIZE			(3 * sizeof(double))


#define RC_INDEX(r, c)			EVSpace_RC_INDEX(r, c)
#define Matrix_COMP(o, r, c)	EVSpace_MATRIX_COMP(o, r, c)

#define Matrix_DATA(o)			(o->data)
#define PyMatrix_DATA(o)		(((EVSpace_Matrix*)o)->data)

#define Matrix_SIZE				9 * sizeof(double)

#endif // EVSPACEMODULE_H
