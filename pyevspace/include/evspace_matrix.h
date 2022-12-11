#ifndef EVSPACE_MATRIX_H
#define EVSPACE_MATRIX_H

#include <Python.h>
#include <evspace_common.h>
#include <evspace_vector.h>
#include <evspacemodule.h>

// forward declaration
static PyTypeObject EVSpace_MatrixType;

#define RC_INDEX(r, c)			EVSpace_RC_INDEX(r, c)
#define Matrix_COMP(o, r, c)	EVSpace_MATRIX_COMP(o, r, c)

#define Matrix_DATA(o)			(o->data)
#define PyMatrix_DATA(o)		(((EVSpace_Matrix*)o)->data)

#define MATRIX_SIZE				9 * sizeof(double)


// constructors 

static EVSpace_Matrix*
_matrix_from_array(double* array, PyTypeObject* type)
{
	EVSpace_Matrix* rtn = (EVSpace_Matrix*)type->tp_alloc(type, 0);
	if (!rtn)
		return NULL;

	Matrix_DATA(rtn) = calloc(9, sizeof(double));
	if (!Matrix_DATA(rtn))
		return (EVSpace_Matrix*)PyErr_NoMemory();

	if (array)
		memcpy(Matrix_DATA(rtn), array, MATRIX_SIZE);

	return rtn;
}

static EVSpace_Matrix*
_matrix_steal_array(double* array, PyTypeObject* type)
{
	assert(array != NULL);

	EVSpace_Matrix* rtn = (EVSpace_Matrix*)type->tp_alloc(type, 0);
	if (!rtn)
		return NULL;

	Matrix_DATA(rtn) = array;
	array = NULL;

	return rtn;
}

/* macros to simplify new_matrix call */
#define new_matrix(a)		_matrix_from_array(a, &EVSpace_MatrixType)
#define new_matrix_empty	new_matrix(NULL)
#define new_matrix_steal(a)	_matrix_steal_array(a, &EVSpace_MatrixType);

static PyObject*
matrix_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED(_))
{
	PyObject* parameters[3] = { Py_None, Py_None, Py_None };

	if (!PyArg_ParseTuple(args, "|OOO",
		&parameters[0],
		&parameters[1],
		&parameters[2]))
		return NULL;

	int none_count = Py_IsNone(parameters[0]) + Py_IsNone(parameters[1])
		+ Py_IsNone(parameters[2]);
	if (none_count == 3)
		return (PyObject*)new_matrix_empty;
	else if (none_count != 0) {
		PyErr_Format(PyExc_TypeError, "function takes either\
			zero or three arguments (%i given)", 3 - none_count);
		return NULL;
	}

	double* array = malloc(MATRIX_SIZE);
	if (!array)
		return PyErr_NoMemory();

	int results[3] = {
		get_sequence_state(parameters[0], array),
		get_sequence_state(parameters[1], array + 3),
		get_sequence_state(parameters[2], array + 6)
	};

	if (results[0] < 0 || results[1] < 0 || results[2] < 0) {
		free(array);
		return NULL;
	}

	PyObject* rtn = (PyObject*)new_matrix_steal(array);
	if (!rtn)
		free(array);

	return rtn;
}



// destructor 

static void
matrix_free(void* self)
{
	free(PyMatrix_DATA(self));
}



// py type methods

#define MATRIX_STRING_FORMAT	"[[%g, %g, %g]\n[%g, %g, %g]\n[%g, %g, %g]]"

static int
__matrix_string_length(const EVSpace_Matrix* self)
{
	return snprintf(NULL, 0, MATRIX_STRING_FORMAT,
		Matrix_COMP(self, 0, 0), Matrix_COMP(self, 0, 1),
		Matrix_COMP(self, 0, 2), Matrix_COMP(self, 1, 0),
		Matrix_COMP(self, 1, 1), Matrix_COMP(self, 1, 2),
		Matrix_COMP(self, 2, 0), Matrix_COMP(self, 2, 1),
		Matrix_COMP(self, 2, 2));
}

static PyObject*
matrix_str(const EVSpace_Matrix* self)
{
	const int buffer_size = __matrix_string_length(self);

	char* buffer = malloc(buffer_size + 1);
	if (!buffer)
		return PyErr_NoMemory();

	sprintf(buffer, MATRIX_STRING_FORMAT,
		Matrix_COMP(self, 0, 0), Matrix_COMP(self, 0, 1),
		Matrix_COMP(self, 0, 2), Matrix_COMP(self, 1, 0),
		Matrix_COMP(self, 1, 1), Matrix_COMP(self, 1, 2),
		Matrix_COMP(self, 2, 0), Matrix_COMP(self, 2, 1),
		Matrix_COMP(self, 2, 2));

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);
	return rtn;
}

static PyObject*
matrix_repr(const EVSpace_Matrix* self)
{
	const int buffer_size = __matrix_string_length(self);

	// 9 chars for type name, 2 chars for tabs, 1 for null char
	char* buffer = malloc(buffer_size + 12);
	if (!buffer)
		return PyErr_NoMemory();

	sprintf(buffer, "EMatrix([[%g, %g, %g]\n\t[%g, %g, %g] \
		\n\t[%g, %g, %g]])",
		Matrix_COMP(self, 0, 0), Matrix_COMP(self, 0, 1),
		Matrix_COMP(self, 0, 2), Matrix_COMP(self, 1, 0),
		Matrix_COMP(self, 1, 1), Matrix_COMP(self, 1, 2),
		Matrix_COMP(self, 2, 0), Matrix_COMP(self, 2, 1),
		Matrix_COMP(self, 2, 2));

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);
	return rtn;
}

static int
__matrix_equal(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs)
{
	return (double_almost_eq(Matrix_COMP(lhs, 0, 0), Matrix_COMP(rhs, 0, 0))
		&& double_almost_eq(Matrix_COMP(lhs, 0, 1), Matrix_COMP(rhs, 0, 1))
		&& double_almost_eq(Matrix_COMP(lhs, 0, 2), Matrix_COMP(rhs, 0, 2))
		&& double_almost_eq(Matrix_COMP(lhs, 1, 0), Matrix_COMP(rhs, 1, 0))
		&& double_almost_eq(Matrix_COMP(lhs, 1, 1), Matrix_COMP(rhs, 1, 1))
		&& double_almost_eq(Matrix_COMP(lhs, 1, 2), Matrix_COMP(rhs, 1, 2))
		&& double_almost_eq(Matrix_COMP(lhs, 2, 0), Matrix_COMP(rhs, 2, 0))
		&& double_almost_eq(Matrix_COMP(lhs, 2, 1), Matrix_COMP(rhs, 2, 1))
		&& double_almost_eq(Matrix_COMP(lhs, 2, 2), Matrix_COMP(rhs, 2, 2)));
}

static PyObject*
matrix_richcompare(EVSpace_Matrix* self, PyObject* other, int op)
{
	if (Matrix_Check(other)) {
		int result = __matrix_equal(
			(EVSpace_Matrix*)self,
			(EVSpace_Matrix*)other
		);
		if (op == Py_EQ)
			return result ? Py_NewRef(Py_True) : Py_NewRef(Py_False);
		else if (op == Py_NE)
			return result ? Py_NewRef(Py_False) : Py_NewRef(Py_True);
	}

	Py_RETURN_NOTIMPLEMENTED;
}



// capsule number methods

static EVSpace_Matrix*
add_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs)
{
	double* array = malloc(MATRIX_SIZE);
	if (!array)
		return (EVSpace_Matrix*)PyErr_NoMemory();

	array[RC_INDEX(0, 0)] = Matrix_COMP(lhs, 0, 0) + Matrix_COMP(rhs, 0, 0);
	array[RC_INDEX(0, 1)] = Matrix_COMP(lhs, 0, 1) + Matrix_COMP(rhs, 0, 1);
	array[RC_INDEX(0, 2)] = Matrix_COMP(lhs, 0, 2) + Matrix_COMP(rhs, 0, 2);
	array[RC_INDEX(1, 0)] = Matrix_COMP(lhs, 1, 0) + Matrix_COMP(rhs, 1, 0);
	array[RC_INDEX(1, 1)] = Matrix_COMP(lhs, 1, 1) + Matrix_COMP(rhs, 1, 1);
	array[RC_INDEX(1, 2)] = Matrix_COMP(lhs, 1, 2) + Matrix_COMP(rhs, 1, 2);
	array[RC_INDEX(2, 0)] = Matrix_COMP(lhs, 2, 0) + Matrix_COMP(rhs, 2, 0);
	array[RC_INDEX(2, 1)] = Matrix_COMP(lhs, 2, 1) + Matrix_COMP(rhs, 2, 1);
	array[RC_INDEX(2, 2)] = Matrix_COMP(lhs, 2, 2) + Matrix_COMP(rhs, 2, 2);

	EVSpace_Matrix* rtn = new_matrix_steal(array);
	if (!rtn)
		free(array);

	return rtn;
}

static EVSpace_Matrix*
subtract_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs)
{
	double* array = malloc(MATRIX_SIZE);
	if (!array)
		return (EVSpace_Matrix*)PyErr_NoMemory();

	array[RC_INDEX(0, 0)] = Matrix_COMP(lhs, 0, 0) - Matrix_COMP(rhs, 0, 0);
	array[RC_INDEX(0, 1)] = Matrix_COMP(lhs, 0, 1) - Matrix_COMP(rhs, 0, 1);
	array[RC_INDEX(0, 2)] = Matrix_COMP(lhs, 0, 2) - Matrix_COMP(rhs, 0, 2);
	array[RC_INDEX(1, 0)] = Matrix_COMP(lhs, 1, 0) - Matrix_COMP(rhs, 1, 0);
	array[RC_INDEX(1, 1)] = Matrix_COMP(lhs, 1, 1) - Matrix_COMP(rhs, 1, 1);
	array[RC_INDEX(1, 2)] = Matrix_COMP(lhs, 1, 2) - Matrix_COMP(rhs, 1, 2);
	array[RC_INDEX(2, 0)] = Matrix_COMP(lhs, 2, 0) - Matrix_COMP(rhs, 2, 0);
	array[RC_INDEX(2, 1)] = Matrix_COMP(lhs, 2, 1) - Matrix_COMP(rhs, 2, 1);
	array[RC_INDEX(2, 2)] = Matrix_COMP(lhs, 2, 2) - Matrix_COMP(rhs, 2, 2);

	EVSpace_Matrix* rtn = new_matrix_steal(array);
	if (!rtn)
		free(array);

	return rtn;
}

static EVSpace_Matrix*
multiply_matrix_scalar(const EVSpace_Matrix* mat, double scalar)
{
	double* ans = malloc(MATRIX_SIZE);
	if (!ans)
		return NULL;

	ans[RC_INDEX(0, 0)] = Matrix_COMP(mat, 0, 0) * scalar;
	ans[RC_INDEX(0, 1)] = Matrix_COMP(mat, 0, 1) * scalar;
	ans[RC_INDEX(0, 2)] = Matrix_COMP(mat, 0, 2) * scalar;
	ans[RC_INDEX(1, 0)] = Matrix_COMP(mat, 1, 0) * scalar;
	ans[RC_INDEX(1, 1)] = Matrix_COMP(mat, 1, 1) * scalar;
	ans[RC_INDEX(1, 2)] = Matrix_COMP(mat, 1, 2) * scalar;
	ans[RC_INDEX(2, 0)] = Matrix_COMP(mat, 2, 0) * scalar;
	ans[RC_INDEX(2, 1)] = Matrix_COMP(mat, 2, 1) * scalar;
	ans[RC_INDEX(2, 2)] = Matrix_COMP(mat, 2, 2) * scalar;

	EVSpace_Matrix* rtn = new_matrix_steal(ans);
	if (!rtn)
		free(ans);

	return rtn;
}

static void
__multiply_matrix_vector_states(const double* mat, const double* vec,
	double* ans, int stride)
{
	assert(ans != NULL);

	ans[0] = mat[RC_INDEX(0, 0)] * vec[0]
		+ mat[RC_INDEX(0, 1)] * vec[stride]
		+ mat[RC_INDEX(0, 2)] * vec[2 * stride];

	ans[stride] = mat[RC_INDEX(1, 0)] * vec[0]
		+ mat[RC_INDEX(1, 1)] * vec[stride]
		+ mat[RC_INDEX(1, 2)] * vec[2 * stride];

	ans[2 * stride] = mat[RC_INDEX(2, 0)] * vec[0]
		+ mat[RC_INDEX(2, 1)] * vec[stride]
		+ mat[RC_INDEX(2, 2)] * vec[2 * stride];
}

static EVSpace_Vector*
multiply_matrix_vector(const EVSpace_Matrix* mat, const EVSpace_Vector* vec)
{
	double* ans = malloc(3 * sizeof(double));
	if (!ans)
		return (EVSpace_Vector*)PyErr_NoMemory();

	__multiply_matrix_vector_states(Matrix_DATA(mat), Matrix_DATA(vec), ans, 1);

	EVSpace_Vector* rtn = _vector_steal_array(ans, &EVSpace_VectorType);
	if (!rtn)
		free(ans);

	return rtn;
}

static EVSpace_Matrix*
multiply_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs)
{
	double* ans = malloc(MATRIX_SIZE);
	if (!ans)
		return NULL;

	__multiply_matrix_vector_states(Matrix_DATA(lhs), Matrix_DATA(rhs),
		ans, 3);
	__multiply_matrix_vector_states(Matrix_DATA(lhs), Matrix_DATA(rhs) + 1,
		ans + 1, 3);
	__multiply_matrix_vector_states(Matrix_DATA(lhs), Matrix_DATA(rhs) + 2,
		ans + 2, 3);

	EVSpace_Matrix* rtn = new_matrix_steal(ans);
	if (!rtn)
		free(ans);

	return rtn;
}

static EVSpace_Matrix*
divide_matrix_scalar(const EVSpace_Matrix* mat, double scalar)
{
	double* array = malloc(MATRIX_SIZE);
	if (!array)
		return NULL;

	array[RC_INDEX(0, 0)] = Matrix_COMP(mat, 0, 0) / scalar;
	array[RC_INDEX(0, 1)] = Matrix_COMP(mat, 0, 1) / scalar;
	array[RC_INDEX(0, 2)] = Matrix_COMP(mat, 0, 2) / scalar;
	array[RC_INDEX(1, 0)] = Matrix_COMP(mat, 1, 0) / scalar;
	array[RC_INDEX(1, 1)] = Matrix_COMP(mat, 1, 1) / scalar;
	array[RC_INDEX(1, 2)] = Matrix_COMP(mat, 1, 2) / scalar;
	array[RC_INDEX(2, 0)] = Matrix_COMP(mat, 2, 0) / scalar;
	array[RC_INDEX(2, 1)] = Matrix_COMP(mat, 2, 1) / scalar;
	array[RC_INDEX(2, 2)] = Matrix_COMP(mat, 2, 2) / scalar;

	EVSpace_Matrix* rtn = new_matrix_steal(array);
	if (!rtn)
		free(array);

	return rtn;
}

static void
iadd_matrix_matrix(EVSpace_Matrix* self, const EVSpace_Matrix* other)
{
	Matrix_COMP(self, 0, 0) += Matrix_COMP(other, 0, 0);
	Matrix_COMP(self, 0, 0) += Matrix_COMP(other, 0, 1);
	Matrix_COMP(self, 0, 0) += Matrix_COMP(other, 0, 2);
	Matrix_COMP(self, 1, 1) += Matrix_COMP(other, 1, 0);
	Matrix_COMP(self, 1, 1) += Matrix_COMP(other, 1, 1);
	Matrix_COMP(self, 1, 1) += Matrix_COMP(other, 1, 2);
	Matrix_COMP(self, 2, 2) += Matrix_COMP(other, 2, 0);
	Matrix_COMP(self, 2, 2) += Matrix_COMP(other, 2, 1);
	Matrix_COMP(self, 2, 2) += Matrix_COMP(other, 2, 2);
}

static void
isubtract_matrix_matrix(EVSpace_Matrix* self, const EVSpace_Matrix* other)
{
	Matrix_COMP(self, 0, 0) -= Matrix_COMP(other, 0, 0);
	Matrix_COMP(self, 0, 1) -= Matrix_COMP(other, 0, 1);
	Matrix_COMP(self, 0, 2) -= Matrix_COMP(other, 0, 2);
	Matrix_COMP(self, 1, 0) -= Matrix_COMP(other, 1, 0);
	Matrix_COMP(self, 1, 1) -= Matrix_COMP(other, 1, 1);
	Matrix_COMP(self, 1, 2) -= Matrix_COMP(other, 1, 2);
	Matrix_COMP(self, 2, 0) -= Matrix_COMP(other, 2, 0);
	Matrix_COMP(self, 2, 1) -= Matrix_COMP(other, 2, 1);
	Matrix_COMP(self, 2, 2) -= Matrix_COMP(other, 2, 2);
}

static void
imultiply_matrix_scalar(EVSpace_Matrix* mat, double scalar)
{
	Matrix_COMP(mat, 0, 0) *= scalar;
	Matrix_COMP(mat, 0, 1) *= scalar;
	Matrix_COMP(mat, 0, 2) *= scalar;
	Matrix_COMP(mat, 1, 0) *= scalar;
	Matrix_COMP(mat, 1, 1) *= scalar;
	Matrix_COMP(mat, 1, 2) *= scalar;
	Matrix_COMP(mat, 2, 0) *= scalar;
	Matrix_COMP(mat, 2, 1) *= scalar;
	Matrix_COMP(mat, 2, 2) *= scalar;
}

static void
idivide_matrix_scalar(EVSpace_Matrix* mat, double scalar)
{
	Matrix_COMP(mat, 0, 0) /= scalar;
	Matrix_COMP(mat, 0, 1) /= scalar;
	Matrix_COMP(mat, 0, 2) /= scalar;
	Matrix_COMP(mat, 1, 0) /= scalar;
	Matrix_COMP(mat, 1, 1) /= scalar;
	Matrix_COMP(mat, 1, 2) /= scalar;
	Matrix_COMP(mat, 2, 0) /= scalar;
	Matrix_COMP(mat, 2, 1) /= scalar;
	Matrix_COMP(mat, 2, 2) /= scalar;
}

static EVSpace_Matrix*
_matrix_negative(const EVSpace_Matrix* self)
{
	double* state = malloc(MATRIX_SIZE);
	if (!state)
		return (EVSpace_Matrix*)PyErr_NoMemory();

	state[RC_INDEX(0, 0)] = -Matrix_COMP(self, 0, 0);
	state[RC_INDEX(0, 1)] = -Matrix_COMP(self, 0, 1);
	state[RC_INDEX(0, 2)] = -Matrix_COMP(self, 0, 2);
	state[RC_INDEX(1, 0)] = -Matrix_COMP(self, 1, 0);
	state[RC_INDEX(1, 1)] = -Matrix_COMP(self, 1, 1);
	state[RC_INDEX(1, 2)] = -Matrix_COMP(self, 1, 2);
	state[RC_INDEX(2, 0)] = -Matrix_COMP(self, 2, 0);
	state[RC_INDEX(2, 1)] = -Matrix_COMP(self, 2, 1);
	state[RC_INDEX(2, 2)] = -Matrix_COMP(self, 2, 2);

	EVSpace_Matrix* rtn = new_matrix_steal(state);
	if (!rtn)
		free(state);

	return rtn;
}



// type number methods

static PyObject*
matrix_add(EVSpace_Matrix* lhs, PyObject* rhs)
{
	if (Matrix_Check(lhs) && Matrix_Check(rhs))
		return (PyObject*)add_matrix_matrix(lhs, (EVSpace_Matrix*)rhs);

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
matrix_subtract(EVSpace_Matrix* lhs, PyObject* rhs)
{
	if (Matrix_Check(lhs) && Matrix_Check(rhs))
		return (PyObject*)subtract_matrix_matrix(lhs, (EVSpace_Matrix*)rhs);

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
matrix_multiply(EVSpace_Matrix* self, PyObject* arg)
{
	if (Matrix_Check(self)) {
		if (Vector_Check(arg))
			return (PyObject*)multiply_matrix_vector(self, (EVSpace_Vector*)arg);

		if (Matrix_Check(arg))
			return (PyObject*)multiply_matrix_matrix(self, (EVSpace_Matrix*)arg);

		if (PyNumber_Check(arg)) {
			double scalar = PyFloat_AsDouble(arg);
			if (scalar == -1.0 && PyErr_Occurred())
				return NULL;

			return (PyObject*)multiply_matrix_scalar(self, scalar);
		}
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
matrix_divide(EVSpace_Matrix* mat, PyObject* arg)
{
	if (Matrix_Check(mat)) {
		if (PyNumber_Check(arg)) {
			double scalar = PyFloat_AsDouble(arg);
			if (scalar == -1.0 && PyErr_Occurred())
				return NULL;

			return (PyObject*)divide_matrix_scalar(mat, scalar);
		}
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
matrix_iadd(EVSpace_Matrix* self, PyObject* other)
{
	if (Matrix_Check(self) && Matrix_Check(other)) {
		iadd_matrix_matrix(self, (EVSpace_Matrix*)other);
		return Py_NewRef(self);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
matrix_isubtract(EVSpace_Matrix* self, PyObject* other)
{
	if (Matrix_Check(self) && Matrix_Check(other)) {
		isubtract_matrix_matrix(self, (EVSpace_Matrix*)other);
		return Py_NewRef(self);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
matrix_imultiply(EVSpace_Matrix* mat, PyObject* arg)
{
	if (Matrix_Check(mat)) {
		if (PyNumber_Check(arg)) {
			double scalar = PyFloat_AsDouble(arg);
			if (scalar == -1.0 && PyErr_Occurred())
				return NULL;

			imultiply_matrix_scalar(mat, scalar);

			return Py_NewRef(mat);
		}
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
matrix_idivide(EVSpace_Matrix* mat, PyObject* arg)
{
	if (Matrix_Check(mat) && PyNumber_Check(arg)) {
		double scalar = PyFloat_AsDouble(arg);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;

		idivide_matrix_scalar(mat, scalar);

		return Py_NewRef(mat);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
negative_matrix(EVSpace_Matrix* self)
{
	if (Matrix_Check(self))
		return (PyObject*)_matrix_negative(self);

	Py_RETURN_NOTIMPLEMENTED;
}



/* type mapping functions */

static PyObject*
matrix_get_item(EVSpace_Matrix* self, PyObject* indices)
{
	int row = -1, col = -1;
	static Py_ssize_t three = 3;

	if (PyLong_Check(indices)) {
		row = PyLong_AsLong(indices);
		if (row == -1 && PyErr_Occurred())
			return NULL;

		if (row < 0 || row > 2) {
			PyErr_Format(PyExc_IndexError,
				"row index (%i) must be in [0-2]", row);
			return NULL;
		}

		Py_buffer* view = malloc(sizeof(Py_buffer));
		if (!view)
			return NULL;

		view->buf = PyMatrix_DATA(self) + (row * 3);
		view->obj = (PyObject*)self;
		view->len = 3 * sizeof(double);
		view->readonly = 0;
		view->itemsize = sizeof(double);
		view->format = "d";
		view->ndim = 1;
		view->shape = &three;
		view->strides = NULL;
		view->suboffsets = NULL;

		return PyMemoryView_FromBuffer(view);
	}

	if (!PyArg_ParseTuple(indices, "ii", &row, &col))
		return NULL;

	if (row < 0 || row > 2) {
		PyErr_Format(PyExc_IndexError,
			"row index (%i) must be in [0-2]", row);
		return NULL;
	}
	else if (col < 0 || col > 2) {
		PyErr_Format(PyExc_IndexError,
			"col index (%i) must be in [0-2]", col);
		return NULL;
	}

	return PyFloat_FromDouble(Matrix_COMP(self, row, col));
}

static int
matrix_set_item(EVSpace_Matrix* self, PyObject* indices, PyObject* value)
{
	int row = -1, col = -1;
	if (!PyArg_ParseTuple(indices, "ii", &row, &col))
		return -1;

	if (row < 0 || row > 2) {
		PyErr_Format(PyExc_IndexError,
			"row index (%i) must be in [0-2]", row);
		return -1;
	}
	else if (col < 0 || col > 2) {
		PyErr_Format(PyExc_IndexError,
			"col index (%i) must be in [0-2]", col);
		return -1;
	}

	double value_double = PyFloat_AsDouble(value);
	if (value_double == -1 && PyErr_Occurred())
		return -1;

	Matrix_COMP(self, row, col) = value_double;
	return 0;
}



/* type buffer functions */

static int
matrix_buffer_get(EVSpace_Matrix* obj, Py_buffer* view, int flags)
{
	if (!view) {
		PyErr_SetString(PyExc_ValueError, "NULL view in getbuffer");
		return -1;
	}

	Py_ssize_t* shape = malloc(2 * sizeof(Py_ssize_t));
	if (!shape) {
		PyErr_NoMemory();
		return -1;
	}
	shape[0] = shape[1] = 3;

	int* internal = malloc(sizeof(int));
	if (!internal) {
		PyErr_NoMemory();
		return -1;
	}
	*internal = BUFFER_RELEASE_SHAPE;

	view->obj = (PyObject*)obj;
	view->buf = PyMatrix_DATA(obj);
	view->len = MATRIX_SIZE;
	view->readonly = 0;
	view->itemsize = sizeof(double);
	view->format = "d";
	view->ndim = 2;
	view->shape = shape;
	view->strides = NULL;
	view->suboffsets = NULL;
	view->internal = (void*)internal;

	Py_INCREF(obj);
	return 0;
}

static void
matrix_buffer_release(EVSpace_Vector* obj, Py_buffer* view)
{
	if (view->internal != NULL) {
		if (*((int*)view->internal) & BUFFER_RELEASE_SHAPE)
			free(view->shape);
		free(view->internal);
	}
}



/* class methods */

static PyObject*
matrix_reduce(PyObject* self, PyObject* Py_UNUSED(_))
{
	return Py_BuildValue("(O((ddd)(ddd)(ddd)))", Py_TYPE(self),
		Matrix_COMP(self, 0, 0), Matrix_COMP(self, 0, 1), Matrix_COMP(self, 0, 2),
		Matrix_COMP(self, 1, 0), Matrix_COMP(self, 1, 1), Matrix_COMP(self, 1, 2),
		Matrix_COMP(self, 2, 0), Matrix_COMP(self, 2, 1), Matrix_COMP(self, 2, 2));
}



// module level capsule functions

static double
_determinate(const EVSpace_Matrix* self)
{
	double term_1 = Matrix_COMP(self, 0, 0)
		* (Matrix_COMP(self, 1, 1) * Matrix_COMP(self, 2, 2)
			- Matrix_COMP(self, 1, 2) * Matrix_COMP(self, 2, 1));

	double term_2 = Matrix_COMP(self, 0, 1)
		* (Matrix_COMP(self, 1, 0) * Matrix_COMP(self, 2, 2)
			- Matrix_COMP(self, 1, 2) * Matrix_COMP(self, 2, 0));

	double term_3 = Matrix_COMP(self, 0, 2)
		* (Matrix_COMP(self, 1, 0) * Matrix_COMP(self, 2, 1)
			- Matrix_COMP(self, 1, 1) * Matrix_COMP(self, 2, 0));

	return term_1 - term_2 + term_3;
}

static EVSpace_Matrix*
_transpose(const EVSpace_Matrix* self)
{
	double* array = malloc(MATRIX_SIZE);
	if (!array)
		return (EVSpace_Matrix*)PyErr_NoMemory();

	array[RC_INDEX(0, 0)] = Matrix_COMP(self, 0, 0);
	array[RC_INDEX(0, 1)] = Matrix_COMP(self, 1, 0);
	array[RC_INDEX(0, 2)] = Matrix_COMP(self, 2, 0);
	array[RC_INDEX(1, 0)] = Matrix_COMP(self, 0, 1);
	array[RC_INDEX(1, 1)] = Matrix_COMP(self, 1, 1);
	array[RC_INDEX(1, 2)] = Matrix_COMP(self, 2, 1);
	array[RC_INDEX(2, 0)] = Matrix_COMP(self, 0, 2);
	array[RC_INDEX(2, 1)] = Matrix_COMP(self, 1, 2);
	array[RC_INDEX(2, 2)] = Matrix_COMP(self, 2, 2);

	EVSpace_Matrix* rtn = new_matrix_steal(array);
	if (!rtn)
		free(array);

	return rtn;
}



// module level functions

static PyObject*
matrix_determinate(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_Format(PyExc_TypeError,
			"det() takes exactly one argument (%i given)", size);
		return NULL;
	}

	if (!Matrix_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, "argument must be EMatrix type");
		return NULL;
	}

	double det = _determinate((EVSpace_Matrix*)args[0]);

	return PyFloat_FromDouble(det);
}

static PyObject*
matrix_transpose(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_Format(PyExc_TypeError,
			"transpose() takes exactly one argument (%i given)", size);
		return NULL;
	}

	if (!Matrix_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, "argument must be EMatrix type");
		return NULL;
	}

	return (PyObject*)_transpose((EVSpace_Matrix*)args[0]);
}



#endif // EVSPACE_MATRIX_H
