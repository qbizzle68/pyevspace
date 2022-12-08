#ifndef EVSPACE_VECTOR_H
#define EVSPACE_VECTOR_H

#include <Python.h>
#include <evspace_common.h>

/* forward declaration */
static PyTypeObject EVSpace_VectorType;

typedef struct {
	PyObject_HEAD
	double* data;			/* x, y, z component of vectors */
} EVSpace_Vector;

#define Vector_Check(o)		PyObject_TypeCheck(o, &EVSpace_VectorType)

#define EVSpace_VECTOR_X(o)	(((EVSpace_Vector*)o)->data[0])
#define EVSpace_VECTOR_Y(o)	(((EVSpace_Vector*)o)->data[1])
#define EVSpace_VECTOR_Z(o)	(((EVSpace_Vector*)o)->data[2])

#define Vector_EQ(l, r)		(double_almost_eq(Vector_X(l), Vector_X(r)) \
							&& double_almost_eq(Vector_Y(l), Vector_Y(r)) \
							&& double_almost_eq(Vector_Z(l), Vector_Z(r)))

/**
 *	\\ constructors \\
 *
 * Two constructors for the vector exist, one which copies the contents of
 * an array into the newly allocated vector, and another that trades pointers
 * to a data array. Functions that call the constructor are responsible for
 * allocating and freeing memory of the state arrays with the exception of
 * course being when the array is 'stolen'. If the state array is NULL the
 * vector values will be initialized to 0. Macros also exist to simplify
 * the calls to these constructors.
 */

static PyObject* 
vector_from_array(const double* arr, PyTypeObject* type);

static PyObject* 
vector_steal_array(double* arr, PyTypeObject* type);

static PyObject*
vector_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED);



/* destructor */

static void
vector_free(void* self);



/* py type functions */

static PyObject*
vector_str(const EVSpace_Vector* self);

static PyObject*
vector_repr(const EVSpace_Vector* self);

static PyObject*
vector_iter(EVSpace_Vector* self);

static PyObject*
vector_richcompare(EVSpace_Vector* self, PyObject* other, int op);



/* capsule number functions */

static PyObject*
add_vector_vector(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs);

static PyObject*
subtract_vector_vector(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs);

static PyObject*
multiply_vector_scalar(const EVSpace_Vector* vector, double scalar);

static PyObject*
divide_vector_scalar(const EVSpace_Vector* vector, double scalar);

static void
iadd_vector_vector(EVSpace_Vector* self, const EVSpace_Vector* other);

static void
isubtract_vector_vector(EVSpace_Vector* self, const EVSpace_Vector* other);

static void
imultiply_vector_scalar(EVSpace_Vector* self, double scalar);

static void
idivide_vector_scalar(EVSpace_Vector* self, double scalar);

static PyObject*
negative_vector(const EVSpace_Vector* self);



/* type as number functions */

static PyObject*
vector_add(EVSpace_Vector* lhs, PyObject* rhs);

static PyObject*
vector_subtract(EVSpace_Vector* lhs, PyObject* rhs);

static PyObject*
vector_multiply(EVSpace_Vector* lhs, PyObject* rhs);

static PyObject*
vector_divide(EVSpace_Vector* lhs, PyObject* rhs);

static PyObject*
vector_iadd(EVSpace_Vector* self, PyObject* other);

static PyObject*
vector_isubtract(EVSpace_Vector* self, PyObject* other);

static PyObject*
vector_imultiply(EVSpace_Vector* self, PyObject* scalar);

static PyObject*
vector_idivide(EVSpace_Vector* self, PyObject* scalar);

static PyObject*
vector_negative(EVSpace_Vector* self);



/* type sequence functions */

static Py_ssize_t
vector_length(EVSpace_Vector* self);

static PyObject*
vector_get_item(EVSpace_Vector* self, Py_ssize_t index);

static int
vector_set_item(EVSpace_Vector* self, Py_ssize_t index, PyObject* arg);



/* type buffer functions */

static int
vector_buffer_get(EVSpace_Vector* obj, Py_buffer* view, int flags);

static void
buffer_release(EVSpace_Vector* obj, Py_buffer* view);



/* class methods capsule */

static double
capsule_vector_magnitude(const EVSpace_Vector* self);

static double
capsule_vector_magnitude2(const EVSpace_Vector* self);

static void
capsule_vector_normalize(EVSpace_Vector* self);



/* class methods */

static PyObject*
vector_magnitude(EVSpace_Vector* self, PyObject* Py_UNUSED);

static PyObject*
vector_magnitude_square(EVSpace_Vector* self, PyObject* Py_UNUSED);

static PyObject*
vector_normalize(EVSpace_Vector* self, PyObject* Py_UNUSED);

static PyObject*
vector_reduce(EVSpace_Vector* self, PyObject* Py_UNUSED);


// module level capsule functions

static double
capsule_vector_dot(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs);

static PyObject*
capsule_vector_cross(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs);

static PyObject*
capsule_vector_norm(const EVSpace_Vector* self);

static double
capsule_vang(const EVSpace_Vector* from, const EVSpace_Vector* to);

static PyObject*
capsule_vxcl(const EVSpace_Vector* vector, const EVSpace_Vector* exclude);

static PyObject*
capsule_proj(const EVSpace_Vector* proj, const EVSpace_Vector* onto);



// module level functions

static PyObject*
vector_dot(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size);

static PyObject*
vector_cross(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size);

static PyObject*
vector_norm(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size);

static PyObject*
vector_vang(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size);

static PyObject*
vector_vxcl(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size);

static PyObject*
vector_proj(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size);

#endif // EVSPACE_VECTOR_H