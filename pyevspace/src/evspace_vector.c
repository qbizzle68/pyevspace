/**
* This source file is meant to be included in a larger compilation unit.
* As a precaution it has extra protection before it can be included.
* This file will only be included if the __EVSPACE_SOURCE_INCLUDE__ 
* symbol is defined.
* 
*/

#ifdef __EVSPACE_SOURCE_INCLUDE__

#ifndef EVSPACE_VECTOR_C
#define EVSPACE_VECTOR_C

#include <evspace_vector.h>
#include <evspace_common.h>



//static PyObject*
//vector_from_array(const double* arr, PyTypeObject* type)


//static PyObject*
//vector_steal_array(double* arr, PyTypeObject* type)



//static PyObject*
//vector_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED)


//static void
//vector_free(void* self) 





//static PyObject*
//vector_str(const EVSpace_Vector* self)


//static PyObject*
//vector_repr(const EVSpace_Vector* self)


//static PyObject*
//vector_iter(EVSpace_Vector* self)


//static PyObject*
//vector_richcompare(EVSpace_Vector* self, PyObject* other, int op)




//	capsule number functions

//static PyObject*
//add_vector_vector(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)


//static PyObject*
//subtract_vector_vector(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)


//static PyObject*
//multiply_vector_scalar(const EVSpace_Vector* vector, double scalar)


//static PyObject*
//divide_vector_scalar(const EVSpace_Vector* vector, double scalar)


//static void
//iadd_vector_vector(EVSpace_Vector* self, const EVSpace_Vector* other)


//static void
//isubtract_vector_vector(EVSpace_Vector* self, const EVSpace_Vector* other)


//static void
//imultiply_vector_scalar(EVSpace_Vector* self, double scalar)


//static void
//idivide_vector_scalar(EVSpace_Vector* self, double scalar)


//static PyObject*
//negative_vector(const EVSpace_Vector* self)




// vector type as number functions

//static PyObject*
//vector_add(EVSpace_Vector* lhs, PyObject* rhs)


//static PyObject*
//vector_subtract(EVSpace_Vector* lhs, PyObject* rhs)


//static PyObject*
//vector_multiply(EVSpace_Vector* lhs, PyObject* rhs)


//static PyObject*
//vector_divide(EVSpace_Vector* lhs, PyObject* rhs)


//static PyObject*
//vector_iadd(EVSpace_Vector* self, PyObject* other)


//static PyObject*
//vector_isubtract(EVSpace_Vector* self, PyObject* other)


//static PyObject*
//vector_imultiply(EVSpace_Vector* self, PyObject* other)


//static PyObject*
//vector_idivide(EVSpace_Vector* self, PyObject* other)


//static PyObject*
//vector_negative(EVSpace_Vector* self)




// type sequence functions

//static Py_ssize_t
//vector_length(EVSpace_Vector* self)


//static PyObject*
//vector_get_item(EVSpace_Vector* self, Py_ssize_t index)


//static int
//vector_set_item(EVSpace_Vector* self, Py_ssize_t index, PyObject* arg)




// type buffer functions

//static int
//vector_buffer_get(EVSpace_Vector* obj, Py_buffer* view, int flags)


//static void
//buffer_release(EVSpace_Vector* obj, Py_buffer* view)






// class capsule methods

//static double
//capsule_vector_magnitude(const EVSpace_Vector* self)


//static double
//capsule_vector_magnitude2(const EVSpace_Vector* self)


//static void
//capsule_vector_normalize(EVSpace_Vector* self)




// type class methods

//static PyObject*
//vector_magnitude(EVSpace_Vector* self, PyObject* Py_UNUSED)


//static PyObject*
//vector_magnitude_square(EVSpace_Vector* self, PyObject* Py_UNUSED)


//static PyObject*
//vector_normalize(EVSpace_Vector* self, PyObject* Py_UNUSED)


//static PyObject*
//vector_reduce(EVSpace_Vector* self, PyObject* Py_UNUSED)




// module level capsule functions

//static double
//capsule_vector_dot(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)


//static PyObject*
//capsule_vector_cross(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)


//static PyObject*
//capsule_vector_norm(const EVSpace_Vector* self)


//static double
//capsule_vang(const EVSpace_Vector* from, const EVSpace_Vector* to)


//static PyObject*
//capsule_vxcl(const EVSpace_Vector* vector, const EVSpace_Vector* exclude)


//static PyObject*
//capsule_proj(const EVSpace_Vector* proj, const EVSpace_Vector* onto)




// module level functions

//static PyObject*
//vector_dot(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)


//static PyObject*
//vector_cross(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)


//static PyObject*
//vector_norm(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)


//static PyObject*
//vector_vang(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)


//static PyObject*
//vector_vxcl(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)


//static PyObject*
//vector_proj(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)


#endif // EVSPACE_VECTOR_C
#endif // __EVSPACE_SOURCE_INCLUDE__
