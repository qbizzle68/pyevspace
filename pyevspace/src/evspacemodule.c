#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <float.h> // DBL_EPSILON
#include <stdint.h> // int32_t, int64_t

/* don't need the extra's for the C API */
#define _EVSPACE_IMPL
#include <evspacemodule.h>

#define PI 3.14159265358979323846

/* define macros for type checking EVSpace types */
#define Vector_Check(o)			PyObject_TypeCheck(o, &EVSpace_VectorType)
#define Vector_CheckExact(o)	Py_IS_TYPE(o, &EVSpace_VectorType)
#define Matrix_Check(o)			PyObject_TypeCheck(o, &EVSpace_MatrixType)
#define Matrix_CheckExact(o)	Py_IS_TYPE(o, &EVSpace_MatrixType)

/* shorten macros for readability */
#define Vector_GETX(o)			EVSpace_VECTOR_GETX(o)
#define Vector_GETY(o)			EVSpace_VECTOR_GETY(o)
#define Vector_GETZ(o)			EVSpace_VECTOR_GETZ(o)
#define Vector_SETX(o, v)		EVSpace_VECTOR_SETX(o, v)
#define Vector_SETY(o, v)		EVSpace_VECTOR_SETY(o, v)
#define Vector_SETZ(o, v)		EVSpace_VECTOR_SETZ(o, v)
#define Matrix_GET(o, r, c)		EVSpace_MATRIX_GET(o, r, c)
#define Matrix_SET(o, r, c, v)	EVSpace_MATRIX_SET(o, r, c, v)

/* forward declarations */
static PyTypeObject EVSpace_VectorType;
static PyTypeObject EVSpace_MatrixType;


/* vector constructor for C API */
static PyObject* 
vector_from_array(const double* arr, PyTypeObject* type)
{
	EVSpace_Vector* self = (EVSpace_Vector*)(type->tp_alloc(type, 0));
	if (!self)
		return NULL;

	self->data = malloc(3 * sizeof(double));
	if (!self->data)
		return PyErr_NoMemory();

	if (arr)
		memcpy(self->data, arr, 3 * sizeof(double));

	return (PyObject*)self;
}

static PyObject*
vector_steal_array(double* arr, PyTypeObject* type)
{
	assert(arr != NULL);

	EVSpace_Vector* self = (EVSpace_Vector*)(type->tp_alloc(type, 0));
	if (!self)
		return NULL;
	self->data = arr;
	arr = NULL;

	return (PyObject*)self;
}

static void 
vector_free(void* self) 
{
	free(((EVSpace_Vector*)self)->data);
}

/* macros to simplify the constructor calls */
#define new_vector(a)	vector_from_array(a, &EVSpace_VectorType)
#define new_vector_empty vector_from_array(NULL, &EVSpace_VectorType)
#define new_vector_steal(a) vector_steal_array(a, &EVSpace_VectorType)

/* get double from PyObject */
static double 
get_double(PyObject* arg) 
{
	double value = PyFloat_AsDouble(arg);

	if (value == -1.0 && PyErr_Occurred()) 
		PyErr_SetString(PyExc_TypeError, "a numeric value is required");

	return value;
}

/* extract floats from sequence */
static PyObject* 
get_state_sequence(PyObject* arg, double* arr)
{
	// todo: get data from buffer first if able

	char* err = "";
	// what do we do with err?
	PyObject* fast_sequence = PySequence_Fast(arg, err);
	if (!fast_sequence) {
		if (PyErr_Occurred() == PyExc_TypeError)
			PyErr_SetString(PyExc_TypeError, "parameter must be a sequence");
		return NULL;
	}
	
	if (PySequence_Fast_GET_SIZE(fast_sequence) == 3) {
		PyObject** items = PySequence_Fast_ITEMS(fast_sequence);
		arr[0] = get_double(items[0]);
		if (arr[0] == -1.0 && PyErr_Occurred())
			goto error;
		arr[1] = get_double(items[1]);
		if (arr[1] == -1.0 && PyErr_Occurred())
			goto error;
		arr[2] = get_double(items[2]);
		if (arr[2] == -1.0 && PyErr_Occurred())
			goto error;

		return arg;
	}
	else {
		PyErr_SetString(PyExc_ValueError, "a sequence or iterable must have exactly 3 elements");
		return NULL;
	}

error:
	Py_DECREF(fast_sequence);
	return NULL;
}

static PyObject* 
vector_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED) 
{
	PyObject* parameter = Py_None;

	if (!PyArg_ParseTuple(args, "|O", &parameter))
		return NULL;
	
	if (Py_IsNone(parameter))
		return new_vector_empty;
	
	double* arr = malloc(3 * sizeof(double));
	if (!arr)
		return PyErr_NoMemory();
	PyObject* result = get_state_sequence(parameter, arr);
	if (!result)
		return NULL;

	return vector_steal_array(arr, type);
}

/* EVector type methods */
static PyObject* 
vector_str(const EVSpace_Vector* self) 
{
	int buffer_size = snprintf(NULL, 0, "[%g, %g, %g]", Vector_GETX(self), Vector_GETY(self), Vector_GETZ(self));
	char* buffer = malloc(buffer_size + 1);
	if (!buffer)
		return PyErr_NoMemory();
	sprintf(buffer, "[%g, %g, %g]", Vector_GETX(self), Vector_GETY(self), Vector_GETZ(self));

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);
	return rtn;
}

static PyObject* 
vector_iter(PyObject* self) 
{
	Py_INCREF(self);
	((EVSpace_Vector*)self)->itr_number = 0;
	return self;
}

static PyObject* 
vector_next(PyObject* self) 
{
	EVSpace_Vector* itr = (EVSpace_Vector*)self;
	if (itr->itr_number < 3) {
		PyObject* value = PyFloat_FromDouble(itr->data[itr->itr_number]);
		if (value)
			(itr->itr_number)++;
		return value;
	}
	else {
		PyErr_SetNone(PyExc_StopIteration);
		return NULL;
	}
}

#define ULP_MAXIMUM	10 // this is a guess, 1 seems too stringent 

static int 
double_almost_eq(double a, double b ) 
{
	// check for really close values near zero
	if (fabs(a - b) < DBL_EPSILON)
		return 1;

	// signs dont match 
	if ((a < 0) != (b < 0))
		return 0;

	int64_t aInt = *(int64_t*)&a;	// bit magic
	int64_t bInt = *(int64_t*)&b;
	if (llabs(aInt - bInt) <= ULP_MAXIMUM)
		return 1;
	return 0;
}

/* macro for simplifying equality expression */
#define Vector_EQ(l, r) (double_almost_eq(Vector_GETX(l), Vector_GETX(r)) \
						&& double_almost_eq(Vector_GETY(l), Vector_GETY(r)) \
						&& double_almost_eq(Vector_GETZ(l), Vector_GETZ(r)))

static PyObject* 
vector_richcompare(PyObject* self, PyObject* other, int op) 
{
	if (Vector_Check(other)) {
		if (op == Py_EQ)
			return Vector_EQ(self, other) ? Py_NewRef(Py_True) : Py_NewRef(Py_False);
		else if (op == Py_NE)
			return (!Vector_EQ(self, other)) ? Py_NewRef(Py_True) : Py_NewRef(Py_False);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

/* number methods */
// factor should be 1 for addition, -1 for subtration
static PyObject* 
add_vector_vector(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs, int factor) 
{
	assert(factor == 1 || factor == -1);

	double* arr = malloc(3 * sizeof(double));
	if (!arr)
		return PyErr_NoMemory();

	arr[0] = Vector_GETX(lhs) + Vector_GETX(rhs) * factor;
	arr[1] = Vector_GETY(lhs) + Vector_GETY(rhs) * factor;
	arr[2] = Vector_GETZ(lhs) + Vector_GETZ(rhs) * factor;

	return new_vector_steal(arr);
}

static PyObject* 
vector_add(PyObject* lhs, PyObject* rhs) 
{
	if (Vector_Check(lhs) && Vector_Check(rhs)) {
		return add_vector_vector((EVSpace_Vector*)lhs, (EVSpace_Vector*)rhs, 1);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
vector_subtract(PyObject* lhs, PyObject* rhs) 
{
	if (Vector_Check(lhs) && Vector_Check(rhs)) {
		return add_vector_vector((EVSpace_Vector*)lhs, (EVSpace_Vector*)rhs, -1);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
mult_vector_scalar(const EVSpace_Vector* lhs, double rhs) 
{
	double* arr = malloc(3 * sizeof(double));
	if (!arr)
		return PyErr_NoMemory();

	arr[0] = Vector_GETX(lhs) * rhs;
	arr[1] = Vector_GETY(lhs) * rhs;
	arr[2] = Vector_GETZ(lhs) * rhs;
	return new_vector_steal(arr);
}

static PyObject* 
vector_multiply(PyObject* lhs, PyObject* rhs) 
{
	if (Vector_Check(lhs)) {
		double scalar = PyFloat_AsDouble(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		return mult_vector_scalar((EVSpace_Vector*)lhs, scalar);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
div_vector_scalar(const EVSpace_Vector* lhs, double rhs) 
{
	double* arr = malloc(3 * sizeof(double));
	if (!arr)
		return NULL;

	arr[0] = Vector_GETX(lhs) / rhs;
	arr[1] = Vector_GETY(lhs) / rhs;
	arr[2] = Vector_GETZ(lhs) / rhs;

	return new_vector_steal(arr);
}

static PyObject* 
vector_divide(PyObject* lhs, PyObject* rhs) 
{
	if (Vector_Check(lhs)) {
		double scalar = PyFloat_AsDouble(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		return div_vector_scalar((EVSpace_Vector*)lhs, scalar);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

/* factor is 1 for addition and -1 for subtraction */
static void 
iadd_vector_vector(EVSpace_Vector* lhs, const EVSpace_Vector* rhs, int factor) 
{
	assert(factor == 1 || factor == -1);
	Vector_SETX(lhs, Vector_GETX(lhs) + Vector_GETX(rhs) * factor);
	Vector_SETY(lhs, Vector_GETY(lhs) + Vector_GETY(rhs) * factor);
	Vector_SETZ(lhs, Vector_GETZ(lhs) + Vector_GETZ(rhs) * factor);
}

static PyObject* 
vector_iadd(PyObject* lhs, PyObject* rhs) 
{
	if (Vector_Check(lhs) && Vector_Check(rhs)) {
		iadd_vector_vector((EVSpace_Vector*)lhs, (EVSpace_Vector*)rhs, 1);
		return Py_NewRef(lhs);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
vector_isubtract(PyObject* lhs, PyObject* rhs) 
{
	if (Vector_Check(lhs) && Vector_Check(rhs)) {
		iadd_vector_vector((EVSpace_Vector*)lhs, (EVSpace_Vector*)rhs, -1);
		return Py_NewRef(lhs);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static void 
imult_vector_scalar(EVSpace_Vector* lhs, double rhs) 
{
	Vector_SETX(lhs, Vector_GETX(lhs) * rhs);
	Vector_SETY(lhs, Vector_GETY(lhs) * rhs);
	Vector_SETZ(lhs, Vector_GETZ(lhs) * rhs);
}

static PyObject* 
vector_imultiply(PyObject* lhs, PyObject* rhs) 
{
	if (Vector_Check(lhs)) {
		double scalar = PyFloat_AsDouble(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		imult_vector_scalar((EVSpace_Vector*)lhs, scalar);
		return Py_NewRef(lhs);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static void 
idiv_vector_scalar(EVSpace_Vector* lhs, double rhs) 
{
	Vector_SETX(lhs, Vector_GETX(lhs) / rhs);
	Vector_SETY(lhs, Vector_GETY(lhs) / rhs);
	Vector_SETZ(lhs, Vector_GETZ(lhs) / rhs);
}

static PyObject* 
vector_idivide(PyObject* lhs, PyObject* rhs) 
{
	if (Vector_Check(lhs)) {
		double scalar = PyFloat_AsDouble(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		idiv_vector_scalar((EVSpace_Vector*)lhs, scalar);
		return Py_NewRef(lhs);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
neg_vector(const EVSpace_Vector* self) 
{
	double* arr = malloc(3 * sizeof(double));
	if (!arr)
		return NULL;

	arr[0] = -Vector_GETX(self);
	arr[1] = -Vector_GETY(self);
	arr[2] = -Vector_GETZ(self);

	return new_vector_steal(arr);
}

static PyObject* 
vector_negative(PyObject* self) 
{
	if (!Vector_Check(self))
		return NULL;
	return neg_vector((EVSpace_Vector*)self);
}

static PyNumberMethods vector_as_number = {
	.nb_add = (binaryfunc)vector_add,
	.nb_subtract = (binaryfunc)vector_subtract,
	.nb_multiply = (binaryfunc)vector_multiply,
	.nb_true_divide = (binaryfunc)vector_divide,
	.nb_negative = (unaryfunc)vector_negative,
	.nb_inplace_add = (binaryfunc)vector_iadd,
	.nb_inplace_subtract = (binaryfunc)vector_isubtract,
	.nb_inplace_multiply = (binaryfunc)vector_imultiply,
	.nb_inplace_true_divide = (binaryfunc)vector_idivide,
};


/* vector sequence methods */
static Py_ssize_t 
vector_length(EVSpace_Vector* self) 
{
	return 3;
}

static PyObject* 
vector_get(EVSpace_Vector* self, Py_ssize_t index) 
{
	if (index < 0 || index > 2) {
		PyErr_SetString(PyExc_IndexError, "index must be in [0-2]");
		return NULL;
	}
	return PyFloat_FromDouble(self->data[index]);
}

static int 
vector_set(EVSpace_Vector* self, Py_ssize_t index, PyObject* arg) 
{
	if (index < 0 || index > 2) {
		PyErr_SetString(PyExc_IndexError, "index must be in [0-2]");
		return -1;
	}

	double value = PyFloat_AsDouble(arg);
	if (value == -1.0 && PyErr_Occurred())
		return -1;

	self->data[index] = value;
	return 0;
}

static PySequenceMethods vector_as_sequence = {
	.sq_length = (lenfunc)vector_length,
	.sq_item = (ssizeargfunc)vector_get,
	.sq_ass_item = (ssizeobjargproc)vector_set,
};

#define BUFFER_RELEASE_SHAPE	1

static int 
vector_buffer_get(PyObject* obj, Py_buffer* view, int flags)
{
	if (!view) {
		PyErr_SetString(PyExc_ValueError, "NULL view in getbuffer");
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

	EVSpace_Vector* self = (EVSpace_Vector*)obj;
	view->obj = obj;
	view->buf = self->data;
	view->len = 3 * sizeof(double);
	view->readonly = 0;
	view->itemsize = sizeof(double);
	view->format = "d";
	view->ndim = 1;
	view->shape = shape;
	view->strides = NULL;
	view->suboffsets = NULL;
	view->internal = (void*)internal;

	Py_INCREF(self);
	return 0;
}

void buffer_release(PyObject* obj, Py_buffer* view)
{
	if (view->internal != NULL) {
		if (*((int*)view->internal) | BUFFER_RELEASE_SHAPE)
			free(view->shape);
		free(view->internal);
	}
}

static PyBufferProcs vector_buffer = {
	(getbufferproc)vector_buffer_get,
	(releasebufferproc)buffer_release
};

/* macros for dot and magnitude expressions */
#define VECTOR_DOT(l, r)	(Vector_GETX(l)*Vector_GETX(r) \
							+ Vector_GETY(l)*Vector_GETY(r) \
							+ Vector_GETZ(l)*Vector_GETZ(r))
#define VECTOR_MAG2(o)	(VECTOR_DOT(o, o))
#define VECTOR_MAG(o) (sqrt(VECTOR_MAG2(o)))


static PyObject* 
vector_magnitude(PyObject* self, PyObject* Py_UNUSED) 
{
	if (!Vector_Check(self)) {
		PyErr_SetString(PyExc_TypeError, "calling object must be EVector type");
		return NULL;
	}

	double mag = VECTOR_MAG((EVSpace_Vector*)self);
	return PyFloat_FromDouble(mag);
}

static PyObject* 
vector_magnitude_square(PyObject* self, PyObject* Py_UNUSED) 
{
	if (!Vector_Check(self)) {
		PyErr_SetString(PyExc_TypeError, "calling object must be EVector type");
		return NULL;
	}

	double mag2 = VECTOR_MAG2((EVSpace_Vector*)self);
	return PyFloat_FromDouble(mag2);	
}

static PyObject* 
vector_normalize(PyObject* self, PyObject* Py_UNUSED) 
{
	if (!Vector_Check(self)) {
		PyErr_SetString(PyExc_TypeError, "calling object must be EVector type");
		return NULL;
	}

	EVSpace_Vector* vector_self = (EVSpace_Vector*)self;
	double mag = VECTOR_MAG(vector_self);
	idiv_vector_scalar(vector_self, mag);
	Py_RETURN_NONE;
}

static PyObject* 
vector_reduce(PyObject* self, PyObject* Py_UNUSED) 
{
	EVSpace_Vector* self_vector = (EVSpace_Vector*)self;
	// need the extra tuple here to please the EVector constructor
	return Py_BuildValue("(O((ddd)))", Py_TYPE(self), Vector_GETX(self_vector), Vector_GETY(self_vector), Vector_GETZ(self_vector));
}

static PyMethodDef vector_methods[] = {
	{"mag", (PyCFunction)vector_magnitude, METH_NOARGS, PyDoc_STR("Compute the magnitude of a vector.")},
	{"mag2", (PyCFunction)vector_magnitude_square, METH_NOARGS, PyDoc_STR("Compute the square of the maginitude of a vector.")},
	{"normalize", (PyCFunction)vector_normalize, METH_NOARGS, PyDoc_STR("Normalize a vector in place.")},
	{"__reduce__", (PyCFunction)vector_reduce, METH_NOARGS, PyDoc_STR("__reduce__() -> (cls, state")},
	{NULL}
};

PyDoc_STRVAR(vector_doc, "");

static PyTypeObject EVSpace_VectorType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name		= "pyevspace.EVector",
	.tp_basicsize	= sizeof(EVSpace_Vector),
	.tp_itemsize	= 0,
	.tp_as_number	= &vector_as_number,
	.tp_as_sequence = &vector_as_sequence,
	.tp_str			= (reprfunc)vector_str,
	.tp_as_buffer	= &vector_buffer,
	.tp_flags		= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE,
	.tp_doc			= vector_doc,
	.tp_richcompare	= (richcmpfunc)vector_richcompare,
	.tp_iter		= vector_iter,
	.tp_iternext	= vector_next,
	.tp_methods		= vector_methods,
	.tp_new			= vector_new,
	.tp_free		= vector_free,
};


static PyObject* 
matrix_from_array(double (*state)[3], PyTypeObject* type) 
{
	EVSpace_Matrix* rtn = (EVSpace_Matrix*)type->tp_alloc(type, 0);

	double (*array)[3] = calloc(3, sizeof *array);
	if (!array)
		return PyErr_NoMemory();
	rtn->data = array;

	if (state)
		memcpy(rtn->data, state, 9 * sizeof(double));

	return (PyObject*)rtn;
}

static PyObject* 
matrix_steal_array(double (*array)[3], PyTypeObject* type) 
{
	assert(array != NULL);

	EVSpace_Matrix* rtn = (EVSpace_Matrix*)type->tp_alloc(type, 0);
	if (!rtn)
		return NULL;
	rtn->data = array;
	array = NULL;

	return (PyObject*)rtn;
}

/* macros for simplifying new_matrix call */
#define new_matrix(a) matrix_from_array(a, &EVSpace_MatrixType);
#define new_matrix_empty new_matrix(NULL)
#define new_matrix_steal(a) matrix_steal_array(a, &EVSpace_MatrixType);

// todo: try to get the sequences here from a buffer first?
static PyObject* 
matrix_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED) 
{
	PyObject* parameters[3] = { Py_None, Py_None, Py_None };

	if (!PyArg_ParseTuple(args, "|OOO", &parameters[0], &parameters[1], &parameters[2]))
		return NULL;

	int none_count = Py_IsNone(parameters[0]) + Py_IsNone(parameters[1]) + Py_IsNone(parameters[2]);
	if (none_count == 3) {
		return new_matrix_empty;
	}
	else if (none_count != 0) {
		PyErr_Format(PyExc_TypeError, "function takes exactly zero or three arguments (%i given)", 3 - none_count);
		return NULL;
	}

	double(*array)[3] = malloc(sizeof(*array) * 3);
	if (!array)
		return PyErr_NoMemory();
	PyObject* results[3] = {
		get_state_sequence(parameters[0], array[0]),
		get_state_sequence(parameters[1], array[1]),
		get_state_sequence(parameters[2], array[2])
	};

	if (!results[0] || !results[1] || !results[2]) {
		free(array);
		return NULL;
	}
	
	PyObject* rtn = new_matrix_steal(array);
	return rtn;
}

static void matrix_free(void* self) {
	free(((EVSpace_Matrix*)self)->data);
}

static PyObject* 
matrix_str(PyObject* self) 
{
	const char* format = "([%g, %g, %g]\n[%g, %g, %g]\n[%g, %g, %g])";
	const int buffer_size = snprintf(NULL, 0, format,
		Matrix_GET(self, 0, 0), Matrix_GET(self, 0, 1), Matrix_GET(self, 0, 2),
		Matrix_GET(self, 1, 0), Matrix_GET(self, 1, 1), Matrix_GET(self, 1, 2),
		Matrix_GET(self, 2, 0), Matrix_GET(self, 2, 1), Matrix_GET(self, 2, 2)
	);

	char* buffer = malloc(buffer_size + 1);
	if (!buffer)
		return PyErr_NoMemory();

	sprintf(buffer, format,
		Matrix_GET(self, 0, 0), Matrix_GET(self, 0, 1), Matrix_GET(self, 0, 2),
		Matrix_GET(self, 1, 0), Matrix_GET(self, 1, 1), Matrix_GET(self, 1, 2),
		Matrix_GET(self, 2, 0), Matrix_GET(self, 2, 1), Matrix_GET(self, 2, 2)
	);
	
	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);
	return rtn;
}

/* get a copy of the matrix data array */
static void 
get_matrix_state(const EVSpace_Matrix* mat, double (*state)[3]) 
{
	assert(double != NULL);
	memcpy(state, mat->data, 9 * sizeof(double));
}

/* factor is 1 for addition, -1 for subtraction */
static PyObject* 
add_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs, int factor) 
{
	assert(factor == 1 || factor == -1);

	double(*lhs_state)[3] = malloc(3 * sizeof * lhs_state);
	if (!lhs_state)
		return PyErr_NoMemory();
	get_matrix_state(lhs, lhs_state);
	const double(*rhs_state)[3] = rhs->data;

	lhs_state[0][0] += rhs_state[0][0] * factor;
	lhs_state[0][1] += rhs_state[0][1] * factor;
	lhs_state[0][2] += rhs_state[0][2] * factor;
	lhs_state[1][0] += rhs_state[1][0] * factor;
	lhs_state[1][1] += rhs_state[1][1] * factor;
	lhs_state[1][2] += rhs_state[1][2] * factor;
	lhs_state[2][0] += rhs_state[2][0] * factor;
	lhs_state[2][1] += rhs_state[2][1] * factor;
	lhs_state[2][2] += rhs_state[2][2] * factor;

	PyObject* rtn = new_matrix_steal(lhs_state);
	return rtn;
}

static PyObject* 
matrix_add(PyObject* lhs, PyObject* rhs) 
{
	if (Matrix_Check(lhs) && Matrix_Check(rhs))
		return add_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs, 1);

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
matrix_subtract(PyObject* lhs, PyObject* rhs) 
{
	if (Matrix_Check(lhs) && Matrix_Check(rhs))
		return add_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs, -1);

	Py_RETURN_NOTIMPLEMENTED;
}


static void
mult_mat_vec_states(const double(* const mat)[3], const double* const vec, double* const ans)
{
	assert(ans != NULL);

	ans[0] = mat[0][0] * vec[0] + mat[0][1] * vec[1] + mat[0][2] * vec[2];
	ans[1] = mat[1][0] * vec[0] + mat[1][1] * vec[1] + mat[1][2] * vec[2];
	ans[2] = mat[2][0] * vec[0] + mat[2][1] * vec[1] + mat[2][2] * vec[2];
}

static inline void 
imultiply_matrix_scalar_states(double (* const mat)[3], double rhs) 
{
	mat[0][0] *= rhs;
	mat[0][1] *= rhs;
	mat[0][2] *= rhs;
	mat[1][0] *= rhs;
	mat[1][1] *= rhs;
	mat[1][2] *= rhs;
	mat[2][0] *= rhs;
	mat[2][1] *= rhs;
	mat[2][2] *= rhs;
}

static PyObject* multiply_matrix_scalar(const EVSpace_Matrix* lhs, double rhs) {
	double(*lhs_state)[3] = malloc(3 * sizeof * lhs_state);
	if (!lhs_state)
		return NULL;
	get_matrix_state(lhs, lhs_state);
	
	imultiply_matrix_scalar_states(lhs_state, rhs);
	return new_matrix_steal(lhs_state);
}

static PyObject* 
multiply_matrix_vector(const EVSpace_Matrix* lhs, const EVSpace_Vector* rhs) 
{
	double* ans = malloc(3 * sizeof(double));
	if (!ans)
		return PyErr_NoMemory();

	mult_mat_vec_states(lhs->data, rhs->data, ans);

	return new_vector_steal(ans);
}

static PyObject* 
multiply_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) 
{
	double(*ans)[3] = malloc(3 * sizeof(*ans));
	if (!ans)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			double sum = 0;
			for (int k = 0; k < 3; k++) {
				sum += Matrix_GET(lhs, i, k) * Matrix_GET(rhs, k, j);
			}
			ans[i][j] = sum;
		}
	}

	return new_matrix_steal(ans);
}

static PyObject* 
matrix_multiply(PyObject* lhs, PyObject* rhs) 
{
	if (Matrix_Check(lhs)) {
		if (Vector_Check(rhs))
			return multiply_matrix_vector((EVSpace_Matrix*)lhs, (EVSpace_Vector*)rhs);

		if (Matrix_Check(rhs))
			return multiply_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs);
		
		if (PyNumber_Check(rhs)) {
			double scalar = get_double(rhs);
			if (scalar == -1.0 && PyErr_Occurred())
				return NULL;
			return multiply_matrix_scalar((EVSpace_Matrix*)lhs, scalar);
		}
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
matrix_divide(PyObject* lhs, PyObject* rhs) 
{
	if (Matrix_Check(lhs)) {
		// todo: just call PyFloat_AsDouble here
		if (PyNumber_Check(rhs)) {
			double scalar = get_double(rhs);
			if (scalar == -1.0 && PyErr_Occurred())
				return NULL;
			return multiply_matrix_scalar((EVSpace_Matrix*)lhs, 1.0 / scalar);
		}
	}

	Py_RETURN_NOTIMPLEMENTED;
}

/* factor is 1 for addition, -1 for subtraction */
static void 
iadd_matrix_matrix(EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs, int factor) 
{
	assert(factor == 1 || factor == -1);

	double(*lhs_state)[3] = lhs->data;
	const double(*rhs_state)[3] = rhs->data;

	lhs_state[0][0] += rhs_state[0][0] * factor;
	lhs_state[0][1] += rhs_state[0][1] * factor;
	lhs_state[0][2] += rhs_state[0][2] * factor;
	lhs_state[1][0] += rhs_state[1][0] * factor;
	lhs_state[1][1] += rhs_state[1][1] * factor;
	lhs_state[1][2] += rhs_state[1][2] * factor;
	lhs_state[2][0] += rhs_state[2][0] * factor;
	lhs_state[2][1] += rhs_state[2][1] * factor;
	lhs_state[2][2] += rhs_state[2][2] * factor;
}

static PyObject* 
matrix_iadd(PyObject* lhs, PyObject* rhs) 
{
	if (Matrix_Check(lhs) && Matrix_Check(rhs)) {
		iadd_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs, 1);
		return Py_NewRef(lhs);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
matrix_isubtract(PyObject* lhs, PyObject* rhs) 
{
	if (Matrix_Check(lhs) && Matrix_Check(rhs)) {
		iadd_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs, -1);
		return Py_NewRef(lhs);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static inline void 
imult_matrix_scalar_states(double (* const mat)[3], double scalar) 
{
	mat[0][0] *= scalar;
	mat[0][1] *= scalar;
	mat[0][2] *= scalar;
	mat[1][0] *= scalar;
	mat[1][1] *= scalar;
	mat[1][2] *= scalar;
	mat[2][0] *= scalar;
	mat[2][1] *= scalar;
	mat[2][2] *= scalar;
}

static PyObject* 
matrix_imultiply(PyObject* lhs, PyObject* rhs) 
{
	if (Matrix_Check(lhs)) {
		// todo: replace this with PyFloat_AsDouble
		if (PyNumber_Check(rhs)) {
			double scalar = get_double(rhs);
			if (scalar == -1.0 && PyErr_Occurred())
				return NULL;
			imult_matrix_scalar_states(((EVSpace_Matrix*)lhs)->data, scalar);
			return Py_NewRef(lhs);
		}
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
matrix_idivide(PyObject* lhs, PyObject* rhs) 
{
	// todo: dont check for number, just call PyFloat_AsDouble and let it handle it
	if (Matrix_Check(lhs) && PyNumber_Check(rhs)) {
		double scalar = get_double(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		imult_matrix_scalar_states(((EVSpace_Matrix*)lhs)->data, 1.0 / scalar);
		return Py_NewRef(lhs);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* 
neg_matrix(const EVSpace_Matrix* self) 
{
	double(*state)[3] = malloc(3 * sizeof * state);
	if (!state)
		return PyErr_NoMemory();
	get_matrix_state(self, state);

	state[0][0] = -state[0][0];
	state[0][1] = -state[0][1];
	state[0][2] = -state[0][2];
	state[1][0] = -state[1][0];
	state[1][1] = -state[1][1];
	state[1][2] = -state[1][2];
	state[2][0] = -state[2][0];
	state[2][1] = -state[2][1];
	state[2][2] = -state[2][2];

	return new_matrix_steal(state);
}

static PyObject* 
matrix_negative(PyObject* lhs) 
{
	if (Matrix_Check(lhs))
		return neg_matrix((EVSpace_Matrix*)lhs);

	Py_RETURN_NOTIMPLEMENTED;
}

static PyNumberMethods matrix_as_number = {
	.nb_add = (binaryfunc)matrix_add,
	.nb_subtract = (binaryfunc)matrix_subtract,
	.nb_multiply = (binaryfunc)matrix_multiply,
	.nb_negative = (unaryfunc)matrix_negative,
	.nb_inplace_add = (binaryfunc)matrix_iadd,
	.nb_inplace_subtract = (binaryfunc)matrix_isubtract,
	.nb_inplace_multiply = (binaryfunc)matrix_imultiply,
	.nb_true_divide = (binaryfunc)matrix_divide,
	.nb_inplace_true_divide = (binaryfunc)matrix_idivide,
};

static PyObject* 
matrix_get(PyObject* self, PyObject* args) 
{
	int row = -1, col = -1;
	if (!PyArg_ParseTuple(args, "ii", &row, &col))
		return NULL;

	if (row < 0 || row > 2 || col < 0 || col > 2) {
		PyErr_SetString(PyExc_IndexError, "row and column index must be in [0-2]");
		return NULL;
	}

	return PyFloat_FromDouble(Matrix_GET(self, row, col));
}

static int 
matrix_set(PyObject* self, PyObject* args, PyObject* value) 
{
	int row = -1, col = -1;
	if (!PyArg_ParseTuple(args, "ii", &row, &col))
		return -1;

	if (row < 0 || row > 2 || col < 0 || col > 2) {
		PyErr_SetString(PyExc_IndexError, "row and column index must be in [0-2]");
		return -1;
	}

	double value_double = get_double(value);
	if (value_double == -1 && PyErr_Occurred())
		return -1;

	Matrix_SET(self, row, col, value_double);
	return 0;
}

static PyMappingMethods matrix_as_mapping = {
	.mp_subscript = matrix_get,
	.mp_ass_subscript = matrix_set,
};

static int
matrix_buffer_get(PyObject* obj, Py_buffer* view, int flags)
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

	EVSpace_Matrix* self = (EVSpace_Matrix*)obj;
	view->obj = obj;
	view->buf = self->data;
	view->len = 9 * sizeof(double);
	view->readonly = 0;
	view->itemsize = sizeof(double);
	view->format = "d";
	view->ndim = 2;
	view->shape = shape;
	view->strides = NULL;
	view->suboffsets = NULL;
	view->internal = (void*)internal;

	Py_INCREF(self);
	return 0;
}

static PyBufferProcs matrix_buffer = {
	(getbufferproc)matrix_buffer_get,
	(releasebufferproc)buffer_release
};

static int 
matrix_equal(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) 
{
	return double_almost_eq(Matrix_GET(lhs, 0, 0), Matrix_GET(rhs, 0, 0))
		&& double_almost_eq(Matrix_GET(lhs, 0, 1), Matrix_GET(rhs, 0, 1))
		&& double_almost_eq(Matrix_GET(lhs, 0, 2), Matrix_GET(rhs, 0, 2))
		&& double_almost_eq(Matrix_GET(lhs, 1, 0), Matrix_GET(rhs, 1, 0))
		&& double_almost_eq(Matrix_GET(lhs, 1, 1), Matrix_GET(rhs, 1, 1))
		&& double_almost_eq(Matrix_GET(lhs, 1, 2), Matrix_GET(rhs, 1, 2))
		&& double_almost_eq(Matrix_GET(lhs, 2, 0), Matrix_GET(rhs, 2, 0))
		&& double_almost_eq(Matrix_GET(lhs, 2, 1), Matrix_GET(rhs, 2, 1))
		&& double_almost_eq(Matrix_GET(lhs, 2, 2), Matrix_GET(rhs, 2, 2));
}

static PyObject* 
matrix_richcompare(PyObject* self, PyObject* other, int op) 
{
	if (Matrix_Check(other)) {
		if (op == Py_EQ) {
			int result = matrix_equal((EVSpace_Matrix*)self, (EVSpace_Matrix*)other);
			return result ? Py_NewRef(Py_True) : Py_NewRef(Py_False);
		}
		else if (op == Py_NE) {
			int result = !matrix_equal((EVSpace_Matrix*)self, (EVSpace_Matrix*)other);
			return result ? Py_NewRef(Py_True) : Py_NewRef(Py_False);
		}
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* matrix_reduce(PyObject* self, PyObject* Py_UNUSED) {
	return Py_BuildValue("(O((ddd)(ddd)(ddd)))", Py_TYPE(self), 
		Matrix_GET(self, 0, 0), Matrix_GET(self, 0, 1), Matrix_GET(self, 0, 2),
		Matrix_GET(self, 1, 0), Matrix_GET(self, 1, 1), Matrix_GET(self, 1, 2),
		Matrix_GET(self, 2, 0), Matrix_GET(self, 2, 1), Matrix_GET(self, 2, 2));
}

static PyMethodDef matrix_methods[] = {
	{"__reduce__", (PyCFunction)matrix_reduce, METH_NOARGS, PyDoc_STR("__reduce__() -> (cls, state")},
	{NULL}
};

PyDoc_STRVAR(matrix_doc, "");

static PyTypeObject EVSpace_MatrixType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name		= "pyevspace.EMatrix",
	.tp_basicsize	= sizeof(EVSpace_Matrix),
	.tp_itemsize	= 0,
	.tp_as_number	= &matrix_as_number,
	.tp_as_mapping	= &matrix_as_mapping,
	.tp_str			= matrix_str,
	.tp_as_buffer	= &matrix_buffer,
	.tp_flags		= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_MAPPING,
	.tp_doc			= matrix_doc,
	.tp_richcompare	= (richcmpfunc)matrix_richcompare,
	.tp_methods		= matrix_methods,
	.tp_new			= matrix_new,
	.tp_free		= matrix_free,
};

/* C API capsule methods */
static PyObject* evspace_vadd(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs) {
	return add_vector_vector(lhs, rhs, 1);
}

static PyObject* evspace_vsub(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs) {
	return add_vector_vector(lhs, rhs, -1);
}

static void evspace_vadd_inplace(EVSpace_Vector* lhs, const EVSpace_Vector* rhs) {
	iadd_vector_vector(lhs, rhs, 1);
}

static void evspace_vsub_inplace(EVSpace_Vector* lhs, const EVSpace_Vector* rhs) {
	iadd_vector_vector(lhs, rhs, -1);
}

static inline double evspace_mag(const EVSpace_Vector* self) {
	return VECTOR_MAG(self);
}

static inline double evspace_mag2(const EVSpace_Vector* self) {
	return VECTOR_MAG2(self);
}

static void evspace_normalize(EVSpace_Vector* self) {
	double mag = VECTOR_MAG(self);
	idiv_vector_scalar(self, mag);
}

static inline double evspace_dot(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs) {
	return VECTOR_DOT(lhs, rhs);
}

static PyObject* evspace_cross(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs) {
	double* arr = malloc(3 * sizeof(double));
	if (!arr)
		return PyErr_NoMemory();

	arr[0] = Vector_GETY(lhs) * Vector_GETZ(rhs) - Vector_GETZ(lhs) * Vector_GETY(rhs);
	arr[1] = Vector_GETZ(lhs) * Vector_GETX(rhs) - Vector_GETX(lhs) * Vector_GETZ(rhs);
	arr[2] = Vector_GETX(lhs) * Vector_GETY(rhs) - Vector_GETY(lhs) * Vector_GETX(rhs);

	return new_vector_steal(arr);
}

static PyObject* evspace_norm(const EVSpace_Vector* self) {
	double mag = VECTOR_MAG(self);
	return div_vector_scalar(self, mag);
}

static double evspace_vang(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs) {
	double dot = VECTOR_DOT(lhs, rhs);
	double lhs_mag = VECTOR_MAG(lhs);
	double rhs_mag = VECTOR_MAG(rhs);
	// this can't be out of range since lhs or rhs don't change between dot and mag calls and they're both in R^3
	return acos(dot / (lhs_mag * rhs_mag));
}

/* removes exclude from vector */
static PyObject* evspace_vxcl(const EVSpace_Vector* vector, const EVSpace_Vector* exclude) {
	double scale = VECTOR_DOT(vector, exclude) / VECTOR_MAG2(exclude);
	double* arr = malloc(3 * sizeof(double));
	if (!arr)
		return PyErr_NoMemory();

	arr[0] = Vector_GETX(vector) - Vector_GETX(exclude) * scale;
	arr[1] = Vector_GETY(vector) - Vector_GETY(exclude) * scale;
	arr[2] = Vector_GETZ(vector) - Vector_GETZ(exclude) * scale;

	return new_vector_steal(arr);
}

static PyObject* evspace_madd(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) {
	return add_matrix_matrix(lhs, rhs, 1);
}

static PyObject* evspace_msub(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) {
	return add_matrix_matrix(lhs, rhs, -1);
}

static PyObject* evspace_mdiv(const EVSpace_Matrix* lhs, double rhs) {
	double(*lhs_state)[3] = malloc(3 * sizeof * lhs_state);
	if (!lhs_state)
		return NULL;
	get_matrix_state(lhs, lhs_state);

	imultiply_matrix_scalar_states(lhs_state, 1.0 / rhs);
	return new_matrix_steal(lhs_state);
}

static void evspace_madd_inplace(EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) {
	iadd_matrix_matrix(lhs, rhs, 1);
}

static void evspace_msub_inplace(EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) {
	iadd_matrix_matrix(lhs, rhs, -1);
}

static void evspace_mmult_inplace(EVSpace_Matrix* lhs, double rhs) {
	imult_matrix_scalar_states(lhs->data, rhs);
}

static void evspace_mdiv_inplace(EVSpace_Matrix* lhs, double rhs) {
	imult_matrix_scalar_states(lhs->data, 1.0 / rhs);
}

static double evspace_det(const EVSpace_Matrix* self) {
	double term_1 = Matrix_GET(self, 0, 0) * (Matrix_GET(self, 1, 1) * Matrix_GET(self, 2, 2) - Matrix_GET(self, 1, 2) * Matrix_GET(self, 2, 1));
	double term_2 = Matrix_GET(self, 0, 1) * (Matrix_GET(self, 1, 0) * Matrix_GET(self, 2, 2) - Matrix_GET(self, 1, 2) * Matrix_GET(self, 2, 0));
	double term_3 = Matrix_GET(self, 0, 2) * (Matrix_GET(self, 1, 0) * Matrix_GET(self, 2, 1) - Matrix_GET(self, 1, 1) * Matrix_GET(self, 2, 0));

	return term_1 - term_2 + term_3;
}

static PyObject* evspace_transpose(const EVSpace_Matrix* self) {
	double (*state)[3] = malloc(3 * sizeof(*state));
	if (!state)
		return PyErr_NoMemory();

	state[0][0] = Matrix_GET(self, 0, 0);
	state[0][1] = Matrix_GET(self, 1, 0);
	state[0][2] = Matrix_GET(self, 2, 0);
	state[1][0] = Matrix_GET(self, 0, 1);
	state[1][1] = Matrix_GET(self, 1, 1);
	state[1][2] = Matrix_GET(self, 2, 1);
	state[2][0] = Matrix_GET(self, 0, 2);
	state[2][1] = Matrix_GET(self, 1, 2);
	state[2][2] = Matrix_GET(self, 2, 2);

	return new_matrix_steal(state);
}

static inline EVSpace_CAPI* get_evspace_capi(void) {
	EVSpace_CAPI* capi = malloc(sizeof(EVSpace_CAPI));
	if (!capi) {
		PyErr_NoMemory();
		return NULL;
	}

	capi->VectorType = &EVSpace_VectorType;
	capi->MatrixType = &EVSpace_MatrixType;

	capi->Vector_FromArray = vector_from_array;
	capi->Matrix_FromArray = matrix_from_array;

	capi->EVSpace_Vector_add = evspace_vadd;
	capi->EVSpace_Vector_subtract = evspace_vsub;
	capi->EVSpace_Vector_multiply = mult_vector_scalar;
	capi->EVSpace_Vector_divide = div_vector_scalar;
	capi->EVSpace_Vector_iadd = evspace_vadd_inplace;
	capi->EVSpace_Vector_isubtract = evspace_vsub_inplace;
	capi->EVSpace_Vector_imultiply = imult_vector_scalar;
	capi->EVSpace_Vector_idivide = idiv_vector_scalar;
	capi->EVSpace_Vector_negative = neg_vector;

	capi->EVSpace_Matrix_add = evspace_madd;
	capi->EVSpace_Matrix_subtract = evspace_msub;
	capi->EVSpace_Matrix_multiply_vector = multiply_matrix_vector;
	capi->EVSpace_Matrix_multiply_matrix = multiply_matrix_matrix;
	capi->EVSpace_Matrix_multiply_scalar = multiply_matrix_scalar;
	capi->EVSpace_Matrix_divide = evspace_mdiv;
	capi->EVSpace_Matrix_iadd = evspace_madd_inplace;
	capi->EVSpace_Matrix_isubtract = evspace_msub_inplace;
	capi->EVSpace_Matrix_imultiply_scalar = evspace_mmult_inplace;
	capi->EVSpace_Matrix_idivide = evspace_mdiv_inplace;
	capi->EVSpace_Matrix_negative = neg_matrix;

	capi->EVSpace_mag = evspace_mag;
	capi->EVSpace_mag_squared = evspace_mag2;
	capi->EVSpace_normalize = evspace_normalize;

	capi->EVSpace_dot = evspace_dot;
	capi->EVSpace_cross = evspace_cross;
	capi->EVSpace_norm = evspace_norm;
	capi->EVSpace_vang = evspace_vang;
	capi->EVSpace_vxcl = evspace_vxcl;
	capi->EVSpace_det = evspace_det;
	capi->EVSpace_transpose = evspace_transpose;

	return capi;
}

static void evspace_destructor(PyObject* capi) {
	void* ptr = PyCapsule_GetPointer(capi, EVSpace_CAPSULE_NAME);
	free(ptr);
}

static PyObject* vector_dot(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size) {
	if (size != 2) {
		PyErr_SetString(PyExc_TypeError, "dot() takes exactly 2 arguments");
		return NULL;
	}

	EVSpace_Vector* lhs = (EVSpace_Vector*)args[0];
	EVSpace_Vector* rhs = (EVSpace_Vector*)args[1];
	if (!Vector_Check(lhs) || !Vector_Check(rhs)) {
		PyErr_SetString(PyExc_TypeError, "arguments must be EVector type");
		return NULL;
	}
	double dot = VECTOR_DOT(lhs, rhs);
	return PyFloat_FromDouble(dot);
}

static PyObject* vector_cross(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size) {
	if (size != 2) {
		PyErr_SetString(PyExc_TypeError, "cross() takes exactly two arguments");
		return NULL;
	}

	EVSpace_Vector* lhs = (EVSpace_Vector*)args[0];
	EVSpace_Vector* rhs = (EVSpace_Vector*)args[1];
	if (!Vector_Check(lhs) || !Vector_Check(rhs)) {
		PyErr_SetString(PyExc_TypeError, "arguments must be EVector type");
		return NULL;
	}
	return evspace_cross(lhs, rhs);
}

static PyObject* vector_norm(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size) {
	if (size != 1) {
		PyErr_SetString(PyExc_TypeError, "norm() takes exactly one argument");
		return NULL;
	}

	EVSpace_Vector* self = (EVSpace_Vector*)args[0];
	if (!Vector_Check(self)) {
		PyErr_SetString(PyExc_TypeError, "arguments must be EVector type");
		return NULL;
	}
	return evspace_norm(self);
}

static PyObject* vector_vang(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size) {
	if (size != 2) {
		PyErr_SetString(PyExc_TypeError, "vang() takes exactly one argument");
		return NULL;
	}

	EVSpace_Vector* lhs = (EVSpace_Vector*)args[0];
	EVSpace_Vector* rhs = (EVSpace_Vector*)args[1];
	if (!Vector_Check(lhs) || !Vector_Check(rhs)) {
		PyErr_SetString(PyExc_TypeError, "arguments must be EVector type");
		return NULL;
	}
	double angle = evspace_vang(lhs, rhs);
	return PyFloat_FromDouble(angle);
}

static PyObject* vector_vxcl(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size) {
	if (size != 2) {
		PyErr_SetString(PyExc_TypeError, "vxcl() takes exactly one argument");
		return NULL;
	}

	EVSpace_Vector* lhs = (EVSpace_Vector*)args[0];
	EVSpace_Vector* rhs = (EVSpace_Vector*)args[1];
	if (!Vector_Check(lhs) || !Vector_Check(rhs)) {
		PyErr_SetString(PyExc_TypeError, "arguments must be EVector type");
		return NULL;
	}
	return evspace_vxcl(lhs, rhs);
}

static PyObject* matrix_det(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size) {
	if (size != 1) {
		PyErr_SetString(PyExc_TypeError, "det() takes exactly one argument");
		return NULL;
	}

	if (!Matrix_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, "argument must be EMatrix type");
		return NULL;
	}

	double det = evspace_det((EVSpace_Matrix*)args[0]);
	return PyFloat_FromDouble(det);
}

static PyObject* matrix_transpose(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size) {
	if (size != 1) {
		PyErr_SetString(PyExc_TypeError, "transpose() takes exactly one argument+");
		return NULL;
	}

	if (!Matrix_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError, "argument must be EMatrix type");
		return NULL;
	}

	return evspace_transpose((EVSpace_Matrix*)args[0]);
}

static PyMethodDef evspace_methods[] = {
	{"dot", (PyCFunction)vector_dot, METH_FASTCALL, PyDoc_STR("Returns the dot product of two EVectors.")},
	{"cross", (PyCFunction)vector_cross, METH_FASTCALL, PyDoc_STR("Returns the cross product of two EVectors.")},
	{"norm", (PyCFunction)vector_norm, METH_FASTCALL, PyDoc_STR("Returns a normalized version of an EVector.")},
	{"vang", (PyCFunction)vector_vang, METH_FASTCALL, PyDoc_STR("Returns the shortest angle between two EVector's.")},
	{"vxcl", (PyCFunction)vector_vxcl, METH_FASTCALL, PyDoc_STR("vxcl(vector, exclude) -> vector with exclude excluded from it")},
	{"det", (PyCFunction)matrix_det, METH_FASTCALL, PyDoc_STR("Returns the determinate of a EMatrix.")},
	{"transpose", (PyCFunction)matrix_transpose, METH_FASTCALL, PyDoc_STR("Returns the transpose of an EMatrix.")},
	{NULL}
};

PyDoc_STRVAR(evspace_doc, "Module for a 3-dimensional Euclidean vector space with a vector and matrix type as well as necessary methods to use them.");

static PyModuleDef EVSpace_Module = {
	PyModuleDef_HEAD_INIT,
	.m_name = "_pyevspace",
	.m_doc = evspace_doc,
	.m_size = -1,
	.m_methods = evspace_methods,
};

PyMODINIT_FUNC
PyInit__pyevspace(void)
{	
	PyObject* m = NULL;
	EVSpace_CAPI* capi = NULL;
	PyObject* constant;

	if (PyType_Ready(&EVSpace_VectorType) < 0)
		return NULL;
	if (PyType_Ready(&EVSpace_MatrixType) < 0)
		return NULL;

	m = PyModule_Create(&EVSpace_Module);
	if (!m)
		return NULL;

	Py_INCREF(&EVSpace_VectorType);
	Py_INCREF(&EVSpace_MatrixType);

	double* buffer = calloc(3, sizeof(double));
	if (!buffer) {
		Py_DECREF(m);
		Py_DECREF(&EVSpace_VectorType);
		Py_DECREF(&EVSpace_MatrixType);
		return NULL;
	}

	buffer[0] = 1.0;
	constant = new_vector(buffer);
	if (!constant)
		goto error;
	if (PyDict_SetItemString(EVSpace_VectorType.tp_dict, "e1", constant) < 0)
		return NULL;
	Py_DECREF(constant);
	
	buffer[0] = 0.0, buffer[1] = 1.0;
	constant = new_vector(buffer);
	if (!constant)
		goto error;
	if (PyDict_SetItemString(EVSpace_VectorType.tp_dict, "e2", constant) < 0)
		goto error;
	Py_DECREF(constant);

	buffer[1] = 0.0, buffer[2] = 1.0;
	constant = new_vector(buffer);
	free(buffer);
	if (!constant)
		goto error;
	if (PyDict_SetItemString(EVSpace_VectorType.tp_dict, "e3", constant) < 0)
		goto error;
	Py_DECREF(constant);

	constant = new_matrix_empty;
	if (!constant)
		goto error;
	Matrix_SET(constant, 0, 0, 1.0);
	Matrix_SET(constant, 1, 1, 1.0);
	Matrix_SET(constant, 2, 2, 1.0);
	if (PyDict_SetItemString(EVSpace_MatrixType.tp_dict, "I", constant) < 0)
		goto error;
	Py_DECREF(constant);
	constant = NULL;

	if (PyModule_AddType(m, &EVSpace_VectorType) < 0)
		goto error;

	if (PyModule_AddType(m, &EVSpace_MatrixType) < 0)
		goto error;

	capi = get_evspace_capi();
	if (!capi)
		goto error;
	PyObject* capsule = PyCapsule_New(capi, EVSpace_CAPSULE_NAME, evspace_destructor);	
	if (!capsule)
		goto error;
	if (PyModule_AddObject(m, "evspace_CAPI", capsule) < 0)
		goto error;
	return m;

error:

	Py_DECREF(m);
	Py_DECREF(&EVSpace_VectorType);
	Py_DECREF(&EVSpace_MatrixType);
	Py_XDECREF(constant);
	if (capi)
		free(capi);

	return NULL;
}