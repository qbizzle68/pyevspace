#ifndef EVSPACE_VECTOR_H
#define EVSPACE_VECTOR_H

#include <Python.h>
#include <evspace_common.h>
#include <evspacemodule.h>

/* forward declaration */
static PyTypeObject EVSpace_VectorType;


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

static EVSpace_Vector*
_vector_from_array(const double* arr, PyTypeObject* type)
{
	EVSpace_Vector* self = (EVSpace_Vector*)(type->tp_alloc(type, 0));
	if (!self)
		return NULL;

	if (arr) {
		Vector_DATA(self) = (double*)malloc(Vector_SIZE);
		if (!Vector_DATA(self))
			return (EVSpace_Vector*)PyErr_NoMemory();

		memcpy(Vector_DATA(self), arr, Vector_SIZE);
	}
	else {
		Vector_DATA(self) = (double*)calloc(3, sizeof(double));
		if (!Vector_DATA(self))
			return (EVSpace_Vector*)PyErr_NoMemory();
	}

	return self;
}

static EVSpace_Vector*
_vector_steal_array(double* arr, PyTypeObject* type)
{
	assert(arr != NULL);

	EVSpace_Vector* self = (EVSpace_Vector*)(type->tp_alloc(type, 0));
	if (!self)
		return NULL;

	Vector_DATA(self) = arr;
	arr = NULL;

	return self;
}

/* macros to simplify the constructor calls */
#define new_vector(a)		_vector_from_array(a, &EVSpace_VectorType)
#define new_vector_empty	_vector_from_array(NULL, &EVSpace_VectorType)
#define new_vector_steal(a)	_vector_steal_array(a, &EVSpace_VectorType)

static PyObject*
vector_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED)
{
	PyObject* parameter = NULL;
	double x = 0, y = 0, z = 0;

	Py_ssize_t tuple_size = PyTuple_GET_SIZE(args);
	if (tuple_size == 0) {
		return (PyObject*)new_vector_empty;
	}

	/* constructor arg is optional (allows init to zero)
	if (!PyArg_ParseTuple(args, "|O", &parameter))
		return NULL;

	if (Py_IsNone(parameter))
		return (PyObject*)new_vector_empty;*/

	double* arr = (double*)malloc(Vector_SIZE);
	if (!arr)
		return PyErr_NoMemory();

	if (tuple_size == 1) {
		if (!PyArg_ParseTuple(args, "O", &parameter)) {
			return NULL;
		}

		if (__get_sequence_state(parameter, arr) < 0) {
			free(arr);
			return NULL;
		}

	//	return (PyObject*)_vector_steal_array(arr, type);
	}
	else if (!PyArg_ParseTuple(args, "ddd", arr, arr+1, arr+2)) {
		return NULL;
	}

	/*if (__get_sequence_state(parameter, arr) < 0) {
		free(arr);
		return NULL;
	}*/

	return (PyObject*)_vector_steal_array(arr, type);
}



/* destructor */

static void
vector_free(void* self)
{
	free(PyVector_DATA(self));
}



/* py type functions */

#define VECTOR_STR_FORMAT	"[%g, %g, %g]"

/* helper for str and repr functions */
static int
__vector_string_length(const EVSpace_Vector* self)
{
	// see how many bytes snprintf would write
	return snprintf(NULL, 0, VECTOR_STR_FORMAT,
		Vector_X(self), Vector_Y(self), Vector_Z(self));
}

static PyObject*
vector_str(const EVSpace_Vector* self)
{
	// find how many bytes we need to allocate
	const size_t buffer_size = __vector_string_length(self);

	char* buffer = malloc(buffer_size + 1);
	if (!buffer)
		return PyErr_NoMemory();

	// don't need snprintf since we know how many bytes to write
	sprintf(buffer, VECTOR_STR_FORMAT,
		Vector_X(self), Vector_Y(self), Vector_Z(self));

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);

	return rtn;
}

static PyObject*
vector_repr(const EVSpace_Vector* self)
{
	// find how many bytes we need to allocate
	const size_t buffer_size = __vector_string_length(self);

	// 9 extra chars for type name and 1 for null char
	char* buffer = malloc(buffer_size + 10);
	if (!buffer)
		return PyErr_NoMemory();

	// don't need snprintf since we know how many bytes to write
	sprintf(buffer, "Vector([%g, %g, %g])",
		Vector_X(self), Vector_Y(self), Vector_Z(self));

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);

	return rtn;
}

static PyObject*
vector_iter(EVSpace_Vector* self)
{
	return PySeqIter_New((PyObject*)self);
}

static inline int
__vector_eq(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)
{
	return (__double_almost_eq(Vector_X(lhs), Vector_X(rhs)) && 
			__double_almost_eq(Vector_Y(lhs), Vector_Y(rhs)) && 
			__double_almost_eq(Vector_Z(lhs), Vector_Z(rhs)));
}

static PyObject*
vector_richcompare(EVSpace_Vector* self, PyObject* other, int op)
{
	if (Vector_Check(other)) {
		if (op == Py_EQ)
		{
			return __vector_eq(self, (EVSpace_Vector*)other)
				? Py_NewRef(Py_True)
				: Py_NewRef(Py_False);
		}
		else if (op == Py_NE)
		{
			return (!__vector_eq(self, (EVSpace_Vector*)other))
				? Py_NewRef(Py_True)
				: Py_NewRef(Py_False);
		}
	}

	Py_RETURN_NOTIMPLEMENTED;
}



/* capsule number functions */

static EVSpace_Vector*
_vector_add(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)
{
	double* arr = malloc(Vector_SIZE);
	if (!arr)
		return (EVSpace_Vector*)PyErr_NoMemory();

	arr[0] = Vector_X(lhs) + Vector_X(rhs);
	arr[1] = Vector_Y(lhs) + Vector_Y(rhs);
	arr[2] = Vector_Z(lhs) + Vector_Z(rhs);

	EVSpace_Vector* rtn = new_vector_steal(arr);
	if (!rtn)
		free(arr);

	return rtn;
}

static EVSpace_Vector*
_vector_subtract(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)
{
	double* arr = malloc(Vector_SIZE);
	if (!arr)
		return (EVSpace_Vector *)PyErr_NoMemory();

	arr[0] = Vector_X(lhs) - Vector_X(rhs);
	arr[1] = Vector_Y(lhs) - Vector_Y(rhs);
	arr[2] = Vector_Z(lhs) - Vector_Z(rhs);

	EVSpace_Vector* rtn = new_vector_steal(arr);
	if (!rtn)
		free(arr);

	return rtn;
}

static EVSpace_Vector*
_vector_multiply(const EVSpace_Vector* vector, double scalar)
{
	double* arr = malloc(Vector_SIZE);
	if (!arr)
		return (EVSpace_Vector*)PyErr_NoMemory();

	arr[0] = Vector_X(vector) * scalar;
	arr[1] = Vector_Y(vector) * scalar;
	arr[2] = Vector_Z(vector) * scalar;

	EVSpace_Vector* rtn = new_vector_steal(arr);
	if (!rtn)
		free(arr);

	return rtn;
}

static EVSpace_Vector*
_vector_divide(const EVSpace_Vector* vector, double scalar)
{
	double* arr = malloc(Vector_SIZE);
	if (!arr)
		return NULL;

	arr[0] = Vector_X(vector) / scalar;
	arr[1] = Vector_Y(vector) / scalar;
	arr[2] = Vector_Z(vector) / scalar;

	EVSpace_Vector* rtn = new_vector_steal(arr);
	if (!rtn)
		free(arr);

	return (EVSpace_Vector*)rtn;
}

static void
_vector_iadd(EVSpace_Vector* self, const EVSpace_Vector* other)
{
	Vector_X(self) += Vector_X(other);
	Vector_Y(self) += Vector_Y(other);
	Vector_Z(self) += Vector_Z(other);
}

static void
_vector_isubtract(EVSpace_Vector* self, const EVSpace_Vector* other)
{
	Vector_X(self) -= Vector_X(other);
	Vector_Y(self) -= Vector_Y(other);
	Vector_Z(self) -= Vector_Z(other);
}

static void
_vector_imultiply(EVSpace_Vector* self, double scalar)
{
	Vector_X(self) *= scalar;
	Vector_Y(self) *= scalar;
	Vector_Z(self) *= scalar;
}

static void
_vector_idivide(EVSpace_Vector* self, double scalar)
{
	Vector_X(self) /= scalar;
	Vector_Y(self) /= scalar;
	Vector_Z(self) /= scalar;
}

static EVSpace_Vector*
_vector_negative(const EVSpace_Vector* self)
{
	double* arr = malloc(Vector_SIZE);
	if (!arr)
		return NULL;

	arr[0] = -Vector_X(self);
	arr[1] = -Vector_Y(self);
	arr[2] = -Vector_Z(self);

	EVSpace_Vector* rtn = new_vector_steal(arr);
	if (!rtn)
		free(arr);

	return rtn;
}



/* type as number functions */

static PyObject*
vector_add(EVSpace_Vector* lhs, PyObject* rhs)
{
	if (Vector_Check((PyObject*)lhs) && Vector_Check(rhs))
	{
		return (PyObject*)_vector_add(lhs, (EVSpace_Vector*)rhs);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
vector_subtract(EVSpace_Vector* lhs, PyObject* rhs)
{
	if (Vector_Check(lhs) && Vector_Check(rhs))
	{
		return (PyObject*)_vector_subtract(lhs, (EVSpace_Vector*)rhs);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
vector_multiply(EVSpace_Vector* lhs, PyObject* rhs)
{
	if (Vector_Check(lhs)) {
		double scalar = PyFloat_AsDouble(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;

		return (PyObject*)_vector_multiply(lhs, scalar);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
vector_divide(EVSpace_Vector* lhs, PyObject* rhs)
{
	if (Vector_Check(lhs)) {
		double scalar = PyFloat_AsDouble(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;

		return (PyObject*)_vector_divide(lhs, scalar);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
vector_iadd(EVSpace_Vector* self, PyObject* other)
{
	if (Vector_Check(self) && Vector_Check(other)) {
		_vector_iadd(self, (EVSpace_Vector*)other);
		return Py_NewRef(self);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
vector_isubtract(EVSpace_Vector* self, PyObject* other)
{
	if (Vector_Check(self) && Vector_Check(other)) {
		_vector_isubtract(self, (EVSpace_Vector*)other);
		return Py_NewRef(self);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
vector_imultiply(EVSpace_Vector* self, PyObject* other)
{
	if (Vector_Check(self)) {
		double scalar = PyFloat_AsDouble(other);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;

		_vector_imultiply(self, scalar);

		return Py_NewRef(self);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
vector_idivide(EVSpace_Vector* self, PyObject* other)
{
	if (Vector_Check(self)) {
		double scalar = PyFloat_AsDouble(other);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;

		_vector_idivide(self, scalar);

		return Py_NewRef(self);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
vector_negative(EVSpace_Vector* self)
{
	if (!Vector_Check(self))
		return NULL;

	return (PyObject*)_vector_negative(self);
}



/* type sequence functions */

static Py_ssize_t
vector_length(EVSpace_Vector* self)
{
	return 3;
}

static PyObject*
vector_get_item(EVSpace_Vector* self, Py_ssize_t index)
{
	if (index < 0 || index > 2) {
		PyErr_Format(
			PyExc_IndexError,
			"index (%i) must be in [0-2]",
			index);
		return NULL;
	}

	return PyFloat_FromDouble(Vector_INDEX(self, index));
}

static int
vector_set_item(EVSpace_Vector* self, Py_ssize_t index, PyObject* arg)
{
	if (index < 0 || index > 2) {
		PyErr_Format(
			PyExc_IndexError,
			"index (%i) must be in [0-2]",
			index);
		return -1;
	}

	double value = PyFloat_AsDouble(arg);
	if (value == -1.0 && PyErr_Occurred())
		return -1;

	Vector_INDEX(self, index) = value;

	return 0;
}



/* type buffer functions */

static int
vector_get_buffer(EVSpace_Vector* obj, Py_buffer* view, int flags)
{
	if (!view) {
		PyErr_SetString(
			PyExc_ValueError,
			"NULL view in getbuffer");
		return -1;
	}

	Py_ssize_t* shape = malloc(sizeof(Py_ssize_t));
	if (!shape) {
		PyErr_NoMemory();
		return -1;
	}
	*shape = 3;

	int* internal = malloc(sizeof(int));
	if (!internal) {
		PyErr_NoMemory();
		return -1;
	}
	*internal = BUFFER_RELEASE_SHAPE;

	view->obj = (PyObject*)obj;
	view->buf = PyVector_DATA(obj);
	view->len = Vector_SIZE;
	view->readonly = 0;
	view->itemsize = sizeof(double);
	view->format = "d";
	view->ndim = 1;
	view->shape = shape;
	view->strides = NULL;
	view->suboffsets = NULL;
	view->internal = (void*)internal;

	Py_INCREF(obj);
	return 0;
}

static inline double
__vector_dot(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)
{
	return (Vector_X(lhs) * Vector_X(rhs)
		  + Vector_Y(lhs) * Vector_Y(rhs)
		  + Vector_Z(lhs) * Vector_Z(rhs));
}

#define VECTOR_MAG2(o)		__vector_dot(o, o)
#define VECTOR_MAG(o)		sqrt(VECTOR_MAG2(o))

/* class methods capsule */

static double
_vector_magnitude(const EVSpace_Vector* self)
{
	return VECTOR_MAG(self);
}

static double
_vector_magnitude2(const EVSpace_Vector* self)
{
	return VECTOR_MAG2(self);
}

static void
_vector_normalize(EVSpace_Vector* self)
{
	_vector_idivide(self, VECTOR_MAG(self));
}



/* class methods */

static PyObject*
vector_magnitude(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
	if (!Vector_Check(self)) {
		PyErr_SetString(
			PyExc_TypeError,
			"calling object must be Vector type");
		return NULL;
	}

	return PyFloat_FromDouble(VECTOR_MAG(self));
}

static PyObject*
vector_magnitude_square(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
	if (!Vector_Check(self)) {
		PyErr_SetString(
			PyExc_TypeError,
			"calling object must be Vector type");
		return NULL;
	}

	return PyFloat_FromDouble(VECTOR_MAG2(self));
}

static PyObject*
vector_normalize(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
	if (!Vector_Check(self)) {
		PyErr_SetString(
			PyExc_TypeError,
			"calling object must be Vector type");
		return NULL;
	}

	_vector_idivide(self, VECTOR_MAG(self));

	return Py_NewRef(self);
}

static PyObject*
vector_reduce(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
	// need the extra tuple here to please the EVector constructor
	return Py_BuildValue(
		"(O((ddd)))",
		Py_TYPE(self),
		Vector_X(self),
		Vector_Y(self),
		Vector_Z(self));
}


// module level capsule functions

static double
_vector_dot(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)
{
	return __vector_dot(lhs, rhs);
}

static EVSpace_Vector*
_vector_cross(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs)
{
	double* arr = malloc(Vector_SIZE);
	if (!arr)
		return (EVSpace_Vector*)PyErr_NoMemory();

	arr[0] = Vector_Y(lhs) * Vector_Z(rhs) - Vector_Z(lhs) * Vector_Y(rhs);
	arr[1] = Vector_Z(lhs) * Vector_X(rhs) - Vector_X(lhs) * Vector_Z(rhs);
	arr[2] = Vector_X(lhs) * Vector_Y(rhs) - Vector_Y(lhs) * Vector_X(rhs);

	EVSpace_Vector* rtn = new_vector_steal(arr);
	if (!rtn)
		free(arr);

	return rtn;
}

static EVSpace_Vector*
_vector_norm(const EVSpace_Vector* self)
{
	return _vector_divide(self, VECTOR_MAG(self));
}

static double
_vector_angle(const EVSpace_Vector* from, const EVSpace_Vector* to)
{
	double dot = __vector_dot(from, to);
	double lhs_mag = VECTOR_MAG(from);
	double rhs_mag = VECTOR_MAG(to);

	// this can't be out of range since lhs or rhs don't change between dot 
	// and mag calls and they're both in R^3
	return acos(dot / (lhs_mag * rhs_mag));
}

static EVSpace_Vector*
_vector_exclude(const EVSpace_Vector* vector, const EVSpace_Vector* exclude)
{
	double scale = __vector_dot(vector, exclude) / VECTOR_MAG2(exclude);

	double* arr = malloc(Vector_SIZE);
	if (!arr)
		return (EVSpace_Vector*)PyErr_NoMemory();

	arr[0] = Vector_X(vector) - Vector_X(exclude) * scale;
	arr[1] = Vector_Y(vector) - Vector_Y(exclude) * scale;
	arr[2] = Vector_Z(vector) - Vector_Z(exclude) * scale;

	EVSpace_Vector* rtn = new_vector_steal(arr);
	if (!rtn)
		free(arr);

	return rtn;
}

static EVSpace_Vector*
_vector_projection(const EVSpace_Vector* proj, const EVSpace_Vector* onto)
{
	double dot = __vector_dot(proj, onto);
	double mag2 = VECTOR_MAG2(onto);

	double* arr = malloc(Vector_SIZE);
	if (!arr)
		return (EVSpace_Vector*)PyErr_NoMemory();

	arr[0] = Vector_X(onto) * dot / mag2;
	arr[1] = Vector_Y(onto) * dot / mag2;
	arr[2] = Vector_Z(onto) * dot / mag2;

	EVSpace_Vector* rtn = new_vector_steal(arr);
	if (!rtn)
		free(arr);

	return rtn;
}



// module level functions

static PyObject*
vector_dot(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_Format(PyExc_TypeError,
			"dot() expected exactly 2 arguments (%i given)", size);
		return NULL;
	}

	if (!Vector_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be Vector type");
		return NULL;
	}
	else if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError, 
			"second argument must be Vector type");
		return NULL;
	}

	double dot = __vector_dot((EVSpace_Vector*)args[0], (EVSpace_Vector*)args[1]);

	return PyFloat_FromDouble(dot);
}

static PyObject*
vector_cross(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_Format(PyExc_TypeError,
			"cross() expected exactly 2 arguments (%i given)", size);
		return NULL;
	}

	if (!Vector_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, 
			"first argument must be Vector type");
		return NULL;
	}
	else if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError, 
			"second argument must be Vector type");
		return NULL;
	}

	return (PyObject*)_vector_cross((EVSpace_Vector*)args[0], 
									(EVSpace_Vector*)args[1]);
}

static PyObject*
vector_norm(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_Format(PyExc_TypeError, 
			"norm() expected exactly 1 argument (%i given)", size);
		return NULL;
	}

	if (!Vector_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, 
			"argument must be Vector type");
		return NULL;
	}

	return (PyObject*)_vector_norm((EVSpace_Vector*)args[0]);
}

static PyObject*
vector_angle(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_Format(PyExc_TypeError, 
			"vang() expected exactly 1 argument (%i given)", size);
		return NULL;
	}

	if (!Vector_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, 
			"first argument must be Vector type");
		return NULL;
	}
	else if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError, 
			"second argument must be Vector type");
		return NULL;
	}

	double angle = _vector_angle((EVSpace_Vector*)args[0], (EVSpace_Vector*)args[1]);

	return PyFloat_FromDouble(angle);
}

static PyObject*
vector_exclude(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_Format(PyExc_TypeError, 
			"vxcl() expected exactly 1 argument (%i given)", size);
		return NULL;
	}

	if (!Vector_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, 
			"first argument must be Vector type");
		return NULL;
	}
	else if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError, 
			"second argument must be Vector type");
		return NULL;
	}

	return (PyObject*)_vector_exclude((EVSpace_Vector*)args[0], 
									  (EVSpace_Vector*)args[1]);
}

static PyObject*
vector_proj(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_Format(PyExc_TypeError, 
			"proj() expected exactly 2 arguments (%i given)", size);
		return NULL;
	}

	if (!Vector_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, 
			"first argument must be Vector type");
		return NULL;
	}
	if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError, 
			"second argument must be Vector type");
		return NULL;
	}

	return (PyObject*)_vector_projection((EVSpace_Vector*)args[0], 
										 (EVSpace_Vector*)args[1]);
}

#endif // EVSPACE_VECTOR_H