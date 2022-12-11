#ifndef EVSPACE_MATRIX_H
#define EVSPACE_MATRIX_H

#include <Python.h>
#include <evspace_common.h>
#include <evspace_vector.h>

// forward declaration
static PyTypeObject EVSpace_MatrixType;

typedef struct {
	PyObject_HEAD
	double* data;			/* row by column ordering */
} EVSpace_Matrix;

#define Matrix_Check(o)			PyObject_TypeCheck(o, &EVSpace_MatrixType)

#define EVSpace_RC_INDEX(r, c)		(3 * r + c)
#define EVSpace_MATRIX_COMP(o, r, c) \
		(((EVSpace_Matrix*)o)->data[EVSpace_RC_INDEX(r, c)])

#define Matrix_DATA(o)			(o->data)
#define PyMatrix_DATA(o)		(((EVSpace_Matrix*)o)->data)

#define MATRIX_SIZE				9 * sizeof(double)


// constructors 

static PyObject*
matrix_from_array(double* array, PyTypeObject* type);

static PyObject*
matrix_steal_array(double* array, PyTypeObject* type);

static PyObject*
matrix_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED(_));



// destructor 

static void
matrix_free(void* self);



// py type methods

static PyObject*
matrix_str(const EVSpace_Matrix* self);

static PyObject*
matrix_repr(const EVSpace_Matrix* self);

static PyObject*
matrix_richcompare(EVSpace_Matrix* self, PyObject* other, int op);



// capsule number methods

static PyObject*
add_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs);

static PyObject*
subtract_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs);

static PyObject*
multiply_matrix_scalar(const EVSpace_Matrix* mat, double scalar);

static PyObject*
multiply_matrix_vector(const EVSpace_Matrix* mat, const EVSpace_Vector* vec);

static PyObject*
multiply_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs);

static PyObject*
divide_matrix_scalar(const EVSpace_Matrix* mat, double scalar);

static void
iadd_matrix_matrix(EVSpace_Matrix* self, const EVSpace_Matrix* other);

static void
isubtract_matrix_matrix(EVSpace_Matrix* self, const EVSpace_Matrix* other);

static void
imultiply_matrix_scalar(EVSpace_Matrix* mat, double scalar);

static void
idivide_matrix_scalar(EVSpace_Matrix* mat, double scalar);

static PyObject*
capsule_matrix_negative(const EVSpace_Matrix* self);



// type number methods

static PyObject*
matrix_add(EVSpace_Matrix* lhs, PyObject* rhs);

static PyObject*
matrix_subtract(EVSpace_Matrix* lhs, PyObject* rhs);

static PyObject*
matrix_multiply(EVSpace_Matrix* self, PyObject* arg);

static PyObject*
matrix_divide(EVSpace_Matrix* mat, PyObject* scalar);

static PyObject*
matrix_iadd(EVSpace_Matrix* self, PyObject* other);

static PyObject*
matrix_isubtract(EVSpace_Matrix* self, PyObject* other);

static PyObject*
matrix_imultiply(EVSpace_Matrix* mat, PyObject* arg);

static PyObject*
matrix_idivide(EVSpace_Matrix* mat, PyObject* scalar);

static PyObject*
negative_matrix(EVSpace_Matrix* self);



/* type mapping functions */

static PyObject*
matrix_get_item(EVSpace_Matrix* self, PyObject* indices);

static int
matrix_set_item(EVSpace_Matrix* self, PyObject* indices, PyObject* value);



/* type buffer functions */

static int
matrix_buffer_get(EVSpace_Matrix* obj, Py_buffer* view, int flags);

static void
matrix_buffer_release(EVSpace_Vector* obj, Py_buffer* view);



/* class methods */

static PyObject*
matrix_reduce(PyObject* self, PyObject* Py_UNUSED(_));



// module level capsule functions

static double
capsule_determinate(const EVSpace_Matrix* self);

static PyObject*
capsule_transpose(const EVSpace_Matrix* self);



// module level functions

static PyObject*
matrix_determinate(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size);

static PyObject*
matrix_transpose(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size);



#endif // EVSPACE_MATRIX_H
