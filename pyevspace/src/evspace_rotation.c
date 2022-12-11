#ifdef __EVSPACE_SOURCE_INCLUDE__

#ifndef EVSPACE_ROTATION_C
#define EVSPACE_ROTATION_C

#include <evspace_rotation.h>


// these may not be defined by the time this file is included into main source
#ifndef new_matrix
#define new_matrix(a)		matrix_from_array(a, &EVSpace_MatrixType)
#endif
#ifndef new_matrix_empty
#define new_matrix_empty	new_matrix(NULL)
#endif
#ifndef new_matrix_steal
#define new_matrix_steal(a)	matrix_steal_array(a, &EVSpace_MatrixType);
#endif



//static PyObject*
//new_rotation_ex(const EVSpace_Order* order, const EVSpace_Angles* angles,
//	PyTypeObject* type)








//static EVSpace_Matrix*
//get_rotation_matrix(EVSpace_Axis axis, double angle)


//static EVSpace_Matrix*
//get_euler_matrix(const EVSpace_Order* order, const EVSpace_Angles* angles)


static EVSpace_Matrix*
_get_euler_transpose(const EVSpace_Order* order, const EVSpace_Angles* angles)
{
	EVSpace_Matrix* matrix = get_euler_matrix(order, angles);
	if (!matrix)
		return NULL;

	double temp[3] = { EVSpace_MATRIX_COMP(matrix, 0, 1),
		EVSpace_MATRIX_COMP(matrix, 0, 2),
		EVSpace_MATRIX_COMP(matrix, 1, 2) };

	EVSpace_MATRIX_COMP(matrix, 0, 1) = EVSpace_MATRIX_COMP(matrix, 1, 0);
	EVSpace_MATRIX_COMP(matrix, 0, 2) = EVSpace_MATRIX_COMP(matrix, 2, 0);
	EVSpace_MATRIX_COMP(matrix, 1, 2) = EVSpace_MATRIX_COMP(matrix, 2, 1);
	EVSpace_MATRIX_COMP(matrix, 1, 0) = temp[0];
	EVSpace_MATRIX_COMP(matrix, 2, 0) = temp[1];
	EVSpace_MATRIX_COMP(matrix, 2, 1) = temp[2];

	return matrix;
}

static EVSpace_Matrix*
get_matrix_from_to(const EVSpace_Order* order_from, const EVSpace_Angles* angles_from,
	const EVSpace_Order* order_to, const EVSpace_Angles* angles_to)


#endif // EVSPACE_ROTATION_C
#endif // __EVSPACE_SOURCE_INCLUDE__