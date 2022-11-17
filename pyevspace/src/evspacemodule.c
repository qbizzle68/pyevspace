#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* don't need the extra's for the C API */
//#define _EVSPACE_IMPL
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
static PyObject* new_vector_ex(double x, double y, double z, PyTypeObject* type) {
	EVSpace_Vector* self = (EVSpace_Vector*)(type->tp_alloc(type, 0));
	if (!self)
		return NULL;

	self->data = PyMem_Malloc(3 * sizeof(double));
	if (!self->data)
		return PyErr_NoMemory();

	Vector_SETX(self, x);
	Vector_SETY(self, y);
	Vector_SETZ(self, z);

	return (PyObject*)self;
}

static void vector_free(void* self) {
	PyMem_Free(((EVSpace_Vector*)self)->data);
}

/* macros to simplify the constructor calls */
#define new_vector(x, y, z)	new_vector_ex(x, y, z, &EVSpace_VectorType)
#define new_vector_empty new_vector_ex(0, 0, 0, &EVSpace_VectorType)

/* get double from PyObject */
static double get_double(PyObject* arg) {
	double value = PyFloat_AsDouble(arg);
	if (value == -1.0 && PyErr_Occurred()) {
		value = PyLong_AsDouble(arg);
		if (value == -1.0 && PyErr_Occurred())
			PyErr_SetString(PyExc_TypeError, "a numeric value is required");
	}
	return value;
}

/* extract floats from sequence */
static PyObject* get_state_sequence(PyObject* arg, double* x, double* y, double* z) {
	char* err = "";
	// what do we do with err?
	PyObject* fast_sequence = PySequence_Fast(arg, err);
	if (!fast_sequence) {
		if (PyErr_Occurred() == PyExc_TypeError)
			PyErr_SetString(PyExc_TypeError, "parameter must be a sequence");
		return NULL;
	}
	
	double xVal = 0, yVal = 0, zVal = 0;
	if (PySequence_Fast_GET_SIZE(fast_sequence) == 3) {
		PyObject** items = PySequence_Fast_ITEMS(fast_sequence);
		xVal = get_double(items[0]);
		if (xVal == -1.0 && PyErr_Occurred())
			goto error;
		yVal = get_double(items[1]);
		if (yVal == -1.0 && PyErr_Occurred())
			goto error;
		zVal = get_double(items[2]);
		if (zVal == -1.0 && PyErr_Occurred())
			goto error;

		*x = xVal;
		*y = yVal;
		*z = zVal;
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

static PyObject* vector_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED) {
	PyObject* parameter = Py_None;

	if (!PyArg_ParseTuple(args, "|O", &parameter))
		return NULL;
	
	if (Py_IsNone(parameter)) {
		return new_vector_ex(0.0, 0.0, 0.0, type);
	}
	
	double x, y, z;
	PyObject* result = get_state_sequence(parameter, &x, &y, &z);
	if (!result)
		return NULL;

	return new_vector_ex(x, y, z, type);
}

/* EVector type methdos */
#define VECTOR_STR_BUFFER_SIZE 100
static PyObject* vector_str(const EVSpace_Vector* self) {
	char buffer[VECTOR_STR_BUFFER_SIZE];
	int ok = snprintf(
		buffer,
		VECTOR_STR_BUFFER_SIZE,
		"[%f, %f, %f]", Vector_GETX(self), Vector_GETY(self), Vector_GETZ(self)
	);

	if (ok < 0 || ok > VECTOR_STR_BUFFER_SIZE) {
		PyErr_SetString(PyExc_MemoryError, "buffer too small for print data");
		return NULL;
	}

	// can we guarantee buffer is utf-8 encoded?
	return PyUnicode_FromString(buffer);
}

static PyObject* vector_iter(PyObject* self) {
	Py_INCREF(self);
	((EVSpace_Vector*)self)->itr_number = 0;
	return self;
}

static PyObject* vector_next(PyObject* self) {
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

/* macro for simplifying equality expression */
#define Vector_EQ(s, o) (Vector_GETX(s) == Vector_GETX(o) \
					  && Vector_GETY(s) == Vector_GETY(o) \
					  && Vector_GETZ(s) == Vector_GETZ(o))

static PyObject* vector_richcompare(PyObject* self, PyObject* other, int op) {
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
static PyObject* add_vector_vector(const EVSpace_Vector* lhs, const EVSpace_Vector* rhs, int factor) {
	assert(factor == 1 || factor == -1);
	return new_vector(
		Vector_GETX(lhs) + Vector_GETX(rhs) * factor,
		Vector_GETY(lhs) + Vector_GETY(rhs) * factor,
		Vector_GETZ(lhs) + Vector_GETZ(rhs) * factor
	);
}

static PyObject* vector_add(PyObject* lhs, PyObject* rhs) {
	if (Vector_Check(lhs) && Vector_Check(rhs)) {
		return add_vector_vector((EVSpace_Vector*)lhs, (EVSpace_Vector*)rhs, 1);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* vector_subtract(PyObject* lhs, PyObject* rhs) {
	if (Vector_Check(lhs) && Vector_Check(rhs)) {
		return add_vector_vector((EVSpace_Vector*)lhs, (EVSpace_Vector*)rhs, -1);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* mult_vector_scalar(const EVSpace_Vector* lhs, double rhs) {
	return new_vector(
		Vector_GETX(lhs) * rhs,
		Vector_GETY(lhs) * rhs,
		Vector_GETZ(lhs) * rhs
	);
}

static PyObject* vector_multiply(PyObject* lhs, PyObject* rhs) {
	if (Vector_Check(lhs)) {
		double scalar = get_double(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		return mult_vector_scalar((EVSpace_Vector*)lhs, scalar);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* div_vector_scalar(const EVSpace_Vector* lhs, double rhs) {
	return new_vector(
		Vector_GETX(lhs) / rhs,
		Vector_GETY(lhs) / rhs,
		Vector_GETZ(lhs) / rhs
	);
}

static PyObject* vector_divide(PyObject* lhs, PyObject* rhs) {
	if (Vector_Check(lhs)) {
		double scalar = get_double(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		return div_vector_scalar((EVSpace_Vector*)lhs, scalar);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

/* factor is 1 for addition and -1 for subtraction */
static void iadd_vector_vector(EVSpace_Vector* lhs, const EVSpace_Vector* rhs, int factor) {
	assert(factor == 1 || factor == -1);
	Vector_SETX(lhs, Vector_GETX(lhs) + Vector_GETX(rhs) * factor);
	Vector_SETY(lhs, Vector_GETY(lhs) + Vector_GETY(rhs) * factor);
	Vector_SETZ(lhs, Vector_GETZ(lhs) + Vector_GETZ(rhs) * factor);
}

static PyObject* vector_iadd(PyObject* lhs, PyObject* rhs) {
	if (Vector_Check(lhs) && Vector_Check(rhs)) {
		iadd_vector_vector((EVSpace_Vector*)lhs, (EVSpace_Vector*)rhs, 1);
		return Py_NewRef(lhs);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* vector_isubtract(PyObject* lhs, PyObject* rhs) {
	if (Vector_Check(lhs) && Vector_Check(rhs)) {
		iadd_vector_vector((EVSpace_Vector*)lhs, (EVSpace_Vector*)rhs, -1);
		return Py_NewRef(lhs);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static void imult_vector_scalar(EVSpace_Vector* lhs, double rhs) {
	Vector_SETX(lhs, Vector_GETX(lhs) * rhs);
	Vector_SETY(lhs, Vector_GETY(lhs) * rhs);
	Vector_SETZ(lhs, Vector_GETZ(lhs) * rhs);
}

static PyObject* vector_imultiply(PyObject* lhs, PyObject* rhs) {
	if (Vector_Check(lhs)) {
		double scalar = get_double(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		imult_vector_scalar((EVSpace_Vector*)lhs, scalar);
		return Py_NewRef(lhs);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static void idiv_vector_scalar(EVSpace_Vector* lhs, double rhs) {
	Vector_SETX(lhs, Vector_GETX(lhs) / rhs);
	Vector_SETY(lhs, Vector_GETY(lhs) / rhs);
	Vector_SETZ(lhs, Vector_GETZ(lhs) / rhs);
}

static PyObject* vector_idivide(PyObject* lhs, PyObject* rhs) {
	if (Vector_Check(lhs)) {
		double scalar = get_double(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		idiv_vector_scalar((EVSpace_Vector*)lhs, scalar);
		return Py_NewRef(lhs);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* neg_vector(const EVSpace_Vector* self) {
	return new_vector(
		-Vector_GETX(self),
		-Vector_GETY(self),
		-Vector_GETZ(self)
	);
}

static PyObject* vector_negative(PyObject* self) {
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
static Py_ssize_t vector_length(EVSpace_Vector* self) {
	return 3;
}

static PyObject* vector_get(EVSpace_Vector* self, Py_ssize_t index) {
	if (index < 0 || index > 2) {
		PyErr_SetString(PyExc_IndexError, "index must be in [0-2]");
		return NULL;
	}
	return PyFloat_FromDouble(self->data[index]);
}

static int vector_set(EVSpace_Vector* self, Py_ssize_t index, PyObject* arg) {
	if (index < 0 || index > 2) {
		PyErr_SetString(PyExc_IndexError, "index must be in [0-2]");
		return -1;
	}

	double value = get_double(arg);
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


/* macros for dot and magnitude expressions */
#define VECTOR_DOT(l, r)	(Vector_GETX(l)*Vector_GETX(r) \
							+ Vector_GETY(l)*Vector_GETY(r) \
							+ Vector_GETZ(l)*Vector_GETZ(r))
#define VECTOR_MAG2(o)	(VECTOR_DOT(o, o))
#define VECTOR_MAG(o) (sqrt(VECTOR_MAG2(o)))


static PyObject* vector_magnitude(PyObject* self, PyObject* Py_UNUSED) {
	if (!Vector_Check(self)) {
		PyErr_SetString(PyExc_TypeError, "calling object must be EVector type");
		return NULL;
	}

	double mag = VECTOR_MAG((EVSpace_Vector*)self);
	return PyFloat_FromDouble(mag);
}

static PyObject* vector_magnitude_square(PyObject* self, PyObject* Py_UNUSED) {
	if (!Vector_Check(self)) {
		PyErr_SetString(PyExc_TypeError, "calling object must be EVector type");
		return NULL;
	}

	double mag2 = VECTOR_MAG2((EVSpace_Vector*)self);
	return PyFloat_FromDouble(mag2);	
}

static PyObject* vector_normalize(PyObject* self, PyObject* Py_UNUSED) {
	if (!Vector_Check(self)) {
		PyErr_SetString(PyExc_TypeError, "calling object must be EVector type");
		return NULL;
	}

	EVSpace_Vector* vector_self = (EVSpace_Vector*)self;
	double mag = VECTOR_MAG(vector_self);
	idiv_vector_scalar(vector_self, mag);
	Py_RETURN_NONE;
}

static PyObject* vector_reduce(PyObject* self, PyObject* Py_UNUSED) {
	EVSpace_Vector* self_vector = (EVSpace_Vector*)self;
	/* need the extra tuple here to please the EVector constructor */
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
	.tp_name = "pyevspace.EVector",
	.tp_basicsize = sizeof(EVSpace_Vector),
	.tp_itemsize = 0,
	.tp_as_number = &vector_as_number,
	.tp_as_sequence = &vector_as_sequence,
	.tp_str = (reprfunc)vector_str,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE,
	.tp_doc = vector_doc,
	.tp_richcompare = (richcmpfunc)vector_richcompare,
	.tp_iter = vector_iter,
	.tp_iternext = vector_next,
	.tp_methods = vector_methods,
	.tp_new = vector_new,
	.tp_free = vector_free,
};


static PyObject* matrix_from_array(double* array, PyTypeObject* type) {
	EVSpace_Matrix* rtn = (EVSpace_Matrix*)type->tp_alloc(type, 0);

	rtn->data = PyMem_Calloc(9, sizeof(double));
	if (!rtn->data)
		return PyErr_NoMemory();

	if (array)
		memcpy(rtn->data, array, 9 * sizeof(double));

	return (PyObject*)rtn;
}

static PyObject* matrix_steal_array(double* array, PyTypeObject* type) {
	assert(array != NULL);

	EVSpace_Matrix* rtn = (EVSpace_Matrix*)type->tp_alloc(type, 0);
	rtn->data = array;
	array = NULL;

	return (PyObject*)rtn;
}

/* macros for simplifying new_matrix call */
#define new_matrix(a) matrix_from_array(a, &EVSpace_MatrixType);
#define new_matrix_empty new_matrix(NULL)
#define new_matrix_steal(a) matrix_steal_array(a, &EVSpace_MatrixType);

static PyObject* matrix_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED) {
	PyObject* parameters[3] = { Py_None, Py_None, Py_None };

	if (!PyArg_ParseTuple(args, "|OOO", &parameters[0], &parameters[1], &parameters[2]))
		return NULL;

	int none_count = Py_IsNone(parameters[0]) + Py_IsNone(parameters[1]) + Py_IsNone(parameters[2]);
	if (none_count == 3) {
		return new_matrix_empty;
	}
	else if (none_count != 0) {
		PyErr_SetString(PyExc_TypeError, "function takes exactly 3 arguments");
		return NULL;
	}

	double* array = PyMem_Malloc(9 * sizeof(double));
	if (!array)
		return PyErr_NoMemory();
	PyObject* results[3] = {
		get_state_sequence(parameters[0], array, array+1, array+2),
		get_state_sequence(parameters[1], array+3, array+4, array+5),
		get_state_sequence(parameters[2], array+6, array+7, array+8)
	};

	if (!results[0] || !results[1] || !results[2]) {
		PyMem_Free(array);
		return NULL;
	}
	
	PyObject* rtn = new_matrix_steal(array);
	return rtn;
}

static void matrix_free(void* self) {
	PyMem_Free((EVSpace_Matrix*)self);
}

// make this 3 times the vector buffer size
#define MATRIX_STR_BUFFER_SIZE VECTOR_STR_BUFFER_SIZE * 3
static PyObject* matrix_str(PyObject* self) {
	char buffer[MATRIX_STR_BUFFER_SIZE];
	int ok = snprintf(
		buffer,
		MATRIX_STR_BUFFER_SIZE,
		"([%f, %f, %f]\n[%f, %f, %f]\n[%f, %f, %f])",
		Matrix_GET(self, 0, 0), Matrix_GET(self, 0, 1), Matrix_GET(self, 0, 2),
		Matrix_GET(self, 1, 0), Matrix_GET(self, 1, 1), Matrix_GET(self, 1, 2),
		Matrix_GET(self, 2, 0), Matrix_GET(self, 2, 1), Matrix_GET(self, 2, 2)
	);

	if (ok < 0 || ok > MATRIX_STR_BUFFER_SIZE) {
		PyErr_SetString(PyExc_BufferError, "Buffer too small to create string.");
		return NULL;
	}

	// can we guarantee buffer is utf-8 encoded?
	return PyUnicode_FromString(buffer);
}

/* get a copy of the matrix data array. caller must free memory when done with PyMem_Free() */
static double* get_matrix_state(const EVSpace_Matrix* mat) {
	double* array = PyMem_Malloc(9 * sizeof(double));

	if (!array)
		return (double*)PyErr_NoMemory();
	memcpy(array, mat->data, 9 * sizeof(double));

	return array;
}

/* factor is 1 for addition, -1 for subtraction */
static PyObject* add_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs, int factor) {
	assert(factor == 1 || factor == -1);

	double* lhs_state = get_matrix_state(lhs);
	if (!lhs_state)
		return NULL;
	double const* rhs_state = rhs->data;

	lhs_state[0] += rhs_state[0] * factor;
	lhs_state[1] += rhs_state[1] * factor;
	lhs_state[2] += rhs_state[2] * factor;
	lhs_state[3] += rhs_state[3] * factor;
	lhs_state[4] += rhs_state[4] * factor;
	lhs_state[5] += rhs_state[5] * factor;
	lhs_state[6] += rhs_state[6] * factor;
	lhs_state[7] += rhs_state[7] * factor;
	lhs_state[8] += rhs_state[8] * factor;

	PyObject* rtn = new_matrix_steal(lhs_state);
	return rtn;
}

static PyObject* matrix_add(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs) && Matrix_Check(rhs))
		return add_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs, 1);

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* matrix_subtract(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs) && Matrix_Check(rhs))
		return add_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs, -1);

	Py_RETURN_NOTIMPLEMENTED;
}


static double* mult_mat_vec_states(const double* const mat, const double* const vec) {
	double* ans = PyMem_Malloc(3 * sizeof(double));
	
	ans[0] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2];
	ans[1] = mat[3] * vec[0] + mat[4] * vec[1] + mat[5] * vec[2];
	ans[2] = mat[6] * vec[0] + mat[7] * vec[1] + mat[8] * vec[2];

	return ans;
}

static inline void imultiply_matrix_scalar_states(double* const mat, double rhs) {
	mat[0] *= rhs;
	mat[1] *= rhs;
	mat[2] *= rhs;
	mat[3] *= rhs;
	mat[4] *= rhs;
	mat[5] *= rhs;
	mat[6] *= rhs;
	mat[7] *= rhs;
	mat[8] *= rhs;
}

static PyObject* multiply_matrix_scalar(const EVSpace_Matrix* lhs, double rhs) {
	double* lhs_state = get_matrix_state(lhs);
	if (!lhs_state)
		return NULL;
	
	imultiply_matrix_scalar_states(lhs_state, rhs);
	return new_matrix_steal(lhs_state);
}

static PyObject* multiply_matrix_vector(const EVSpace_Matrix* lhs, const EVSpace_Vector* rhs) {
	double* ans = PyMem_Malloc(3 * sizeof(double));

	ans[0] = lhs->data[0] * rhs->data[0] + lhs->data[1] * rhs->data[1] + lhs->data[2] * rhs->data[2];
	ans[1] = lhs->data[3] * rhs->data[0] + lhs->data[4] * rhs->data[1] + lhs->data[5] * rhs->data[2];
	ans[2] = lhs->data[6] * rhs->data[0] + lhs->data[7] * rhs->data[1] + lhs->data[8] * rhs->data[2];

	/* todo: replace this if we implement stealing the pointer for vectors */
	return new_vector(ans[0], ans[1], ans[2]);
}

static PyObject* multiply_matrix_matrix(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) {
	double* ans = PyMem_Malloc(9 * sizeof(double));
	if (!ans)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			double sum = 0;
			for (int k = 0; k < 3; k++) {
				sum += lhs->data[ROWCOL_TOINDEX(i, k)] * rhs->data[ROWCOL_TOINDEX(k, j)];
			}
			ans[ROWCOL_TOINDEX(i, j)] = sum;
		}
	}

	return new_matrix_steal(ans);
}

static PyObject* matrix_multiply(PyObject* lhs, PyObject* rhs) {
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

static PyObject* matrix_divide(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs)) {
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
static void iadd_matrix_matrix(EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs, int factor) {
	assert(factor == 1 || factor == -1);

	double* lhs_state = lhs->data;
	double const* rhs_state = rhs->data;

	lhs_state[0] += rhs_state[0] * factor;
	lhs_state[1] += rhs_state[1] * factor;
	lhs_state[2] += rhs_state[2] * factor;
	lhs_state[3] += rhs_state[3] * factor;
	lhs_state[4] += rhs_state[4] * factor;
	lhs_state[5] += rhs_state[5] * factor;
	lhs_state[6] += rhs_state[6] * factor;
	lhs_state[7] += rhs_state[7] * factor;
	lhs_state[8] += rhs_state[8] * factor;
}

static PyObject* matrix_iadd(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs) && Matrix_Check(rhs)) {
		iadd_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs, 1);
		return Py_NewRef(lhs);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* matrix_isubtract(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs) && Matrix_Check(rhs)) {
		iadd_matrix_matrix((EVSpace_Matrix*)lhs, (EVSpace_Matrix*)rhs, -1);
		return Py_NewRef(lhs);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static inline void imult_matrix_scalar_states(double * const mat, double scalar) {
	mat[0] *= scalar;
	mat[1] *= scalar;
	mat[2] *= scalar;
	mat[3] *= scalar;
	mat[4] *= scalar;
	mat[5] *= scalar;
	mat[6] *= scalar;
	mat[7] *= scalar;
	mat[8] *= scalar;
}

static PyObject* matrix_imultiply(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs)) {

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

static PyObject* matrix_idivide(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs) && PyNumber_Check(rhs)) {
		double scalar = get_double(rhs);
		if (scalar == -1.0 && PyErr_Occurred())
			return NULL;
		imult_matrix_scalar_states(((EVSpace_Matrix*)lhs)->data, 1.0 / scalar);
		return Py_NewRef(lhs);
	}

	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* neg_matrix(const EVSpace_Matrix* self) {
	double* state = get_matrix_state((EVSpace_Matrix*)self);

	state[0] = -state[0];
	state[1] = -state[1];
	state[2] = -state[2];
	state[3] = -state[3];
	state[4] = -state[4];
	state[5] = -state[5];
	state[6] = -state[6];
	state[7] = -state[7];
	state[8] = -state[8];

	return new_matrix_steal(state);
}

static PyObject* matrix_negative(PyObject* lhs) {
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

static PyObject* matrix_get(PyObject* self, PyObject* args) {
	int row = -1, col = -1;
	if (!PyArg_ParseTuple(args, "ii", &row, &col))
		return NULL;

	if (row < 0 || row > 2 || col < 0 || col > 2) {
		PyErr_SetString(PyExc_IndexError, "row and column index must be in [0-2]");
		return NULL;
	}

	return PyFloat_FromDouble(Matrix_GET(self, row, col));
}

static int matrix_set(PyObject* self, PyObject* args, PyObject* value) {
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

static int matrix_equal(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) {
	return Matrix_GET(lhs, 0, 0) == Matrix_GET(rhs, 0, 0)
		&& Matrix_GET(lhs, 0, 1) == Matrix_GET(rhs, 0, 1)
		&& Matrix_GET(lhs, 0, 2) == Matrix_GET(rhs, 0, 2)
		&& Matrix_GET(lhs, 1, 0) == Matrix_GET(rhs, 1, 0)
		&& Matrix_GET(lhs, 1, 1) == Matrix_GET(rhs, 1, 1)
		&& Matrix_GET(lhs, 1, 2) == Matrix_GET(rhs, 1, 2)
		&& Matrix_GET(lhs, 2, 0) == Matrix_GET(rhs, 2, 0)
		&& Matrix_GET(lhs, 2, 1) == Matrix_GET(rhs, 2, 1)
		&& Matrix_GET(lhs, 2, 2) == Matrix_GET(rhs, 2, 2);
}

static PyObject* matrix_richcompare(PyObject* self, PyObject* other, int op) {
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
	.tp_name = "pyevspace.EMatrix",
	.tp_basicsize = sizeof(EVSpace_Matrix) + 9 * sizeof(double),
	.tp_itemsize = 0,
	.tp_as_number = &matrix_as_number,
	//.tp_as_sequence = &matrix_as_sequence,
	.tp_as_mapping = &matrix_as_mapping,
	.tp_str = matrix_str,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_MAPPING,
	.tp_doc = matrix_doc,
	.tp_richcompare = (richcmpfunc)matrix_richcompare,
	.tp_methods = &matrix_methods,
	.tp_new = matrix_new,
	.tp_free = matrix_free,
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
	double x = Vector_GETY(lhs) * Vector_GETZ(rhs) - Vector_GETZ(lhs) * Vector_GETY(rhs);
	double y = Vector_GETZ(lhs) * Vector_GETX(rhs) - Vector_GETX(lhs) * Vector_GETZ(rhs);
	double z = Vector_GETX(lhs) * Vector_GETY(rhs) - Vector_GETY(lhs) * Vector_GETX(rhs);
	return new_vector(x, y, z);
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
	double x = Vector_GETX(vector) - Vector_GETX(exclude) * scale;
	double y = Vector_GETY(vector) - Vector_GETY(exclude) * scale;
	double z = Vector_GETZ(vector) - Vector_GETZ(exclude) * scale;
	return new_vector(x, y, z);
}

static PyObject* evspace_madd(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) {
	return add_matrix_matrix(lhs, rhs, 1);
}

static PyObject* evspace_msub(const EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs) {
	return add_matrix_matrix(lhs, rhs, -1);
}

static PyObject* evspace_mdiv(const EVSpace_Matrix* lhs, double rhs) {
	double* lhs_state = get_matrix_state(lhs);
	if (!lhs_state)
		return NULL;

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
	double* state = PyMem_Malloc(9 * sizeof(double));
	if (!state)
		return PyErr_NoMemory();

	state[ROWCOL_TOINDEX(0, 0)] = self->data[ROWCOL_TOINDEX(0, 0)];
	state[ROWCOL_TOINDEX(0, 1)] = self->data[ROWCOL_TOINDEX(1, 0)];
	state[ROWCOL_TOINDEX(0, 2)] = self->data[ROWCOL_TOINDEX(2, 0)];
	state[ROWCOL_TOINDEX(1, 0)] = self->data[ROWCOL_TOINDEX(0, 1)];
	state[ROWCOL_TOINDEX(1, 1)] = self->data[ROWCOL_TOINDEX(1, 1)];
	state[ROWCOL_TOINDEX(1, 2)] = self->data[ROWCOL_TOINDEX(2, 1)];
	state[ROWCOL_TOINDEX(2, 0)] = self->data[ROWCOL_TOINDEX(0, 2)];
	state[ROWCOL_TOINDEX(2, 1)] = self->data[ROWCOL_TOINDEX(1, 2)];
	state[ROWCOL_TOINDEX(2, 2)] = self->data[ROWCOL_TOINDEX(2, 2)];

	return new_matrix_steal(state);
}

static inline EVSpace_CAPI* get_evspace_capi(void) {
	EVSpace_CAPI* capi = PyMem_Malloc(sizeof(EVSpace_CAPI));
	if (!capi) {
		PyErr_NoMemory();
		return NULL;
	}
	capi->VectorType = &EVSpace_VectorType;
	capi->MatrixType = &EVSpace_MatrixType;

	capi->Vector_FromValues = new_vector_ex;
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
	//capi->EVSpace_Matrix_imultiply_matrix = NULL;
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
	PyMem_Free(ptr);
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
	.m_name = "pyevspace",
	.m_doc = evspace_doc,
	.m_size = -1,
	.m_methods = &evspace_methods,
};

PyMODINIT_FUNC
PyInit_pyevspace(void)
{	
	PyObject* m = NULL;
	EVSpace_CAPI* capi = NULL;

	if (PyType_Ready(&EVSpace_VectorType) < 0)
		return NULL;
	if (PyType_Ready(&EVSpace_MatrixType) < 0)
		return NULL;

	m = PyModule_Create(&EVSpace_Module);
	if (!m)
		return NULL;

	Py_INCREF(&EVSpace_VectorType);
	Py_INCREF(&EVSpace_MatrixType);

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
	if (capi) {
		PyMem_Free(capi);
		Py_DECREF(capi);
	}

	return NULL;
}


#ifdef _NO_INCLUDE

/* matrix C API constructor, array should be NULL for empty matrix */
static PyObject* matrix_new(double** array, PyTypeObject* type) {
	EVSpace_Matrix* rtn = (EVSpace_Matrix*)type->tp_alloc(type, 0);

	if (!array)
		memcpy(rtn->data, array, sizeof(rtn->data));
	
	/*if (!array) {
		Matrix_SET(rtn, 0, 0, array[0][0]);
		Matrix_SET(rtn, 0, 1, array[0][1]);
		Matrix_SET(rtn, 0, 2, array[0][2]);
		Matrix_SET(rtn, 1, 0, array[1][0]);
		Matrix_SET(rtn, 1, 1, array[1][1]);
		Matrix_SET(rtn, 1, 2, array[1][2]);
		Matrix_SET(rtn, 2, 0, array[2][0]);
		Matrix_SET(rtn, 2, 1, array[2][1]);
		Matrix_SET(rtn, 2, 2, array[2][2]);
	}*/
	return (PyObject*)rtn;
}

#define new_matrix(a) new_matrix(a, &EVSpace_MatrixType)
#define new_matrix_empty new_matrix(NULL, &EVSpace_MatrixType)

// todo: do we sue this for our matrix numeric methods?
static double** matrix_get_state(const EVSpace_Matrix* matrix) {
	double** state;
	memcpy(state, matrix->data, sizeof(matrix->data));
	return state;
}


/* helper methods to execute code for each matrix component */
//static inline void _mat_add_ex(const PyObject* lhs, const PyObject* rhs, PyObject* answer) {
//	Matrix_SET(answer, 0, 0, Matrix_GET(lhs, 0, 0) + Matrix_GET(rhs, 0, 0));
//	Matrix_SET(answer, 0, 1, Matrix_GET(lhs, 0, 1) + Matrix_GET(rhs, 0, 1));
//	Matrix_SET(answer, 0, 2, Matrix_GET(lhs, 0, 2) + Matrix_GET(rhs, 0, 2));
//	Matrix_SET(answer, 1, 0, Matrix_GET(lhs, 1, 0) + Matrix_GET(rhs, 1, 0));
//	Matrix_SET(answer, 1, 1, Matrix_GET(lhs, 1, 1) + Matrix_GET(rhs, 1, 1));
//	Matrix_SET(answer, 1, 2, Matrix_GET(lhs, 1, 2) + Matrix_GET(rhs, 1, 2));
//	Matrix_SET(answer, 2, 0, Matrix_GET(lhs, 2, 0) + Matrix_GET(rhs, 2, 0));
//	Matrix_SET(answer, 2, 1, Matrix_GET(lhs, 2, 1) + Matrix_GET(rhs, 2, 1));
//	Matrix_SET(answer, 2, 2, Matrix_GET(lhs, 2, 2) + Matrix_GET(rhs, 2, 2));
//}

//static inline void _mat_sub_ex(const PyObject* lhs, const PyObject* rhs, PyObject* answer) {
//	Matrix_SET(answer, 0, 0, Matrix_GET(lhs, 0, 0) - Matrix_GET(rhs, 0, 0));
//	Matrix_SET(answer, 0, 1, Matrix_GET(lhs, 0, 1) - Matrix_GET(rhs, 0, 1));
//	Matrix_SET(answer, 0, 2, Matrix_GET(lhs, 0, 2) - Matrix_GET(rhs, 0, 2));
//	Matrix_SET(answer, 1, 0, Matrix_GET(lhs, 1, 0) - Matrix_GET(rhs, 1, 0));
//	Matrix_SET(answer, 1, 1, Matrix_GET(lhs, 1, 1) - Matrix_GET(rhs, 1, 1));
//	Matrix_SET(answer, 1, 2, Matrix_GET(lhs, 1, 2) - Matrix_GET(rhs, 1, 2));
//	Matrix_SET(answer, 2, 0, Matrix_GET(lhs, 2, 0) - Matrix_GET(rhs, 2, 0));
//	Matrix_SET(answer, 2, 1, Matrix_GET(lhs, 2, 1) - Matrix_GET(rhs, 2, 1));
//	Matrix_SET(answer, 2, 2, Matrix_GET(lhs, 2, 2) - Matrix_GET(rhs, 2, 2));
//}

//static inline void _mat_mmult_s_ex(const PyObject* lhs, double rhs, PyObject* answer) {
//	Matrix_SET(answer, 0, 0, Matrix_GET(lhs, 0, 0) * rhs);
//	Matrix_SET(answer, 0, 1, Matrix_GET(lhs, 0, 0) * rhs);
//	Matrix_SET(answer, 0, 2, Matrix_GET(lhs, 0, 0) * rhs);
//	Matrix_SET(answer, 1, 0, Matrix_GET(lhs, 0, 0) * rhs);
//	Matrix_SET(answer, 1, 1, Matrix_GET(lhs, 0, 0) * rhs);
//	Matrix_SET(answer, 1, 2, Matrix_GET(lhs, 0, 0) * rhs);
//	Matrix_SET(answer, 2, 0, Matrix_GET(lhs, 0, 0) * rhs);
//	Matrix_SET(answer, 2, 1, Matrix_GET(lhs, 0, 0) * rhs);
//	Matrix_SET(answer, 2, 2, Matrix_GET(lhs, 0, 0) * rhs);
//}

//static inline double** _matrix_add_states(double** lhs_state, double** rhs_state) {
//	return (double[3][3]) {
//		{lhs_state[0][0] + rhs_state[0][0], lhs_state[0][1] + rhs_state[0][1], lhs_state[0][2] + rhs_state[0][2]},
//		{lhs_state[1][0] + rhs_state[1][0], lhs_state[1][1] + rhs_state[1][1], lhs_state[1][2] + rhs_state[1][2]},
//		{lhs_state[2][0] + rhs_state[2][0], lhs_state[2][1] + rhs_state[2][1], lhs_state[2][2] + rhs_state[2][2]}
//	};
//}
//
//static inline double** _matrix_sub_states(double** lhs_state, double** rhs_state) {
//	return (double[3][3]) {
//		{lhs_state[0][0] - rhs_state[0][0], lhs_state[0][1] - rhs_state[0][1], lhs_state[0][2] - rhs_state[0][2]},
//		{ lhs_state[1][0] - rhs_state[1][0], lhs_state[1][1] - rhs_state[1][1], lhs_state[1][2] - rhs_state[1][2] },
//		{ lhs_state[2][0] - rhs_state[2][0], lhs_state[2][1] - rhs_state[2][1], lhs_state[2][2] - rhs_state[2][2] }
//	};
//}

#define free_matrix_state(s) free(s[0]); free(s[1]); free(s[2]); free(s);

/* matrix C API number method implementation */
static PyObject* evspace_madd(const PyObject* lhs, const PyObject* rhs) {
	const double** lhs_state = ((EVSpace_Matrix*)lhs)->data; // matrix_get_state((EVSpace_Matrix*)lhs);
	const double** rhs_state = ((EVSpace_Matrix*)rhs)->data; // matrix_get_state((EVSpace_Matrix*)rhs);

	double add_state[3][3] = {
		{lhs_state[0][0] + rhs_state[0][0], lhs_state[0][1] + rhs_state[0][1], lhs_state[0][2] + rhs_state[0][2]},
		{ lhs_state[1][0] + rhs_state[1][0], lhs_state[1][1] + rhs_state[1][1], lhs_state[1][2] + rhs_state[1][2] },
		{ lhs_state[2][0] + rhs_state[2][0], lhs_state[2][1] + rhs_state[2][1], lhs_state[2][2] + rhs_state[2][2] }
	};

	EVSpace_Matrix* rtn = new_matrix(add_state);
	free_matrix_state(add_state)
	return (PyObject*)rtn;

	//EVSpace_Matrix* rtn = new_matrix_empty;
	//if (!rtn)
	//	return NULL;

	///*Matrix_SET(rtn, 0, 0, Matrix_GET(lhs, 0, 0) + Matrix_GET(rhs, 0, 0));
	//Matrix_SET(rtn, 0, 1, Matrix_GET(lhs, 0, 1) + Matrix_GET(rhs, 0, 1));
	//Matrix_SET(rtn, 0, 2, Matrix_GET(lhs, 0, 2) + Matrix_GET(rhs, 0, 2));
	//Matrix_SET(rtn, 1, 0, Matrix_GET(lhs, 1, 0) + Matrix_GET(rhs, 1, 0));
	//Matrix_SET(rtn, 1, 1, Matrix_GET(lhs, 1, 1) + Matrix_GET(rhs, 1, 1));
	//Matrix_SET(rtn, 1, 2, Matrix_GET(lhs, 1, 2) + Matrix_GET(rhs, 1, 2));
	//Matrix_SET(rtn, 2, 0, Matrix_GET(lhs, 2, 0) + Matrix_GET(rhs, 2, 0));
	//Matrix_SET(rtn, 2, 1, Matrix_GET(lhs, 2, 1) + Matrix_GET(rhs, 2, 1));
	//Matrix_SET(rtn, 2, 2, Matrix_GET(lhs, 2, 2) + Matrix_GET(rhs, 2, 2));*/
	//_mat_add_ex(lhs, rhs, rtn);

	//return (PyObject*)rtn;
}

static PyObject* evspace_msub(const PyObject* lhs, const PyObject* rhs) {
	const double** lhs_state = ((EVSpace_Matrix*)lhs)->data;
	const double** rhs_state = ((EVSpace_Matrix*)rhs)->data;

	double sub_state[3][3] = {
		{ lhs_state[0][0] - rhs_state[0][0], lhs_state[0][1] - rhs_state[0][1], lhs_state[0][2] - rhs_state[0][2] },
		{ lhs_state[1][0] - rhs_state[1][0], lhs_state[1][1] - rhs_state[1][1], lhs_state[1][2] - rhs_state[1][2] },
		{ lhs_state[2][0] - rhs_state[2][0], lhs_state[2][1] - rhs_state[2][1], lhs_state[2][2] - rhs_state[2][2] }
	};

	EVSpace_Matrix* rtn = new_matrix(sub_state);
	free_matrix_state(sub_state)
	return (PyObject*)rtn;
	
	//EVSpace_Matrix* rtn = new_matrix_empty;
	//if (!rtn)
	//	return NULL;

	///*Matrix_SET(rtn, 0, 0, Matrix_GET(lhs, 0, 0) - Matrix_GET(rhs, 0, 0));
	//Matrix_SET(rtn, 0, 1, Matrix_GET(lhs, 0, 1) - Matrix_GET(rhs, 0, 1));
	//Matrix_SET(rtn, 0, 2, Matrix_GET(lhs, 0, 2) - Matrix_GET(rhs, 0, 2));
	//Matrix_SET(rtn, 1, 0, Matrix_GET(lhs, 1, 0) - Matrix_GET(rhs, 1, 0));
	//Matrix_SET(rtn, 1, 1, Matrix_GET(lhs, 1, 1) - Matrix_GET(rhs, 1, 1));
	//Matrix_SET(rtn, 1, 2, Matrix_GET(lhs, 1, 2) - Matrix_GET(rhs, 1, 2));
	//Matrix_SET(rtn, 2, 0, Matrix_GET(lhs, 2, 0) - Matrix_GET(rhs, 2, 0));
	//Matrix_SET(rtn, 2, 1, Matrix_GET(lhs, 2, 1) - Matrix_GET(rhs, 2, 1));
	//Matrix_SET(rtn, 2, 2, Matrix_GET(lhs, 2, 2) - Matrix_GET(rhs, 2, 2));*/
	//_mat_sub_ex(lhs, rhs, rtn);

	//return (PyObject*)rtn;
}

//#define MatrixRow_ByVector(m, v, r) Matrix_GET(m, r, 0) * Vector_GETX(v) \
//									+ Matrix_GET(m, r, 1) * Vector_GETY(v) \
//									+ Matrix_GET(m, r, 2) * Vector_GETZ(v)

static PyObject* evspace_mmult_v(const PyObject* lhs, const PyObject* rhs) {
	//const double** lhs_state = ((EVSpace_Matrix*)lhs)->data;
	//const double* rhs_state = ((EVSpace_Vector*)rhs)->data;
	
	double x = Matrix_GET(lhs, 0, 0) * Vector_GETX(rhs) 
		+ Matrix_GET(lhs, 0, 1) * Vector_GETY(rhs) 
		+ Matrix_GET(lhs, 0, 2) * Vector_GETZ(rhs); 
	double y = Matrix_GET(lhs, 1, 0) * Vector_GETX(rhs)
		+ Matrix_GET(lhs, 1, 1) * Vector_GETY(rhs)
		+ Matrix_GET(lhs, 1, 2) * Vector_GETZ(rhs); 
	double z = Matrix_GET(lhs, 2, 0) * Vector_GETX(rhs)
		+ Matrix_GET(lhs, 2, 1) * Vector_GETY(rhs)
		+ Matrix_GET(lhs, 2, 2) * Vector_GETZ(rhs);

	return new_vector(x, y, z);


	/*double x = MatrixRow_ByVector(lhs, rhs, 0);
	double y = MatrixRow_ByVector(lhs, rhs, 1);
	double z = MatrixRow_ByVector(lhs, rhs, 2);

	return new_vector(x, y, z);*/
}

//#define MatrixRow_ByMatrix(m1, m2, r, c) Matrix_GET(m1, r, 0) * Matrix_GET(m2, 0, c) \
//										+ Matrix_GET(m1, r, 1) * Matrix_GET(m2, 1, c) \
//										+ Matrix_GET(m1, r, 2) * Matrix_GET(m2, 2, c)
#define matrix_state_mult(s1, s2, r, c) s1[r][0] * s2[0][c] + s1[r][1] * s2[1][c] + s1[r][2] * s2[2][c]

static PyObject* evspace_mmult_m(const PyObject* lhs, const PyObject* rhs) {
	const double** lhs_state = ((EVSpace_Matrix*)lhs)->data;
	const double** rhs_state = ((EVSpace_Matrix*)rhs)->data;

	const double state[3][3] = {
		{matrix_state_mult(lhs_state, rhs_state, 0, 0), matrix_state_mult(lhs_state, rhs_state, 0, 1), matrix_state_mult(lhs_state, rhs_state, 0, 2)},
		{matrix_state_mult(lhs_state, rhs_state, 1, 0), matrix_state_mult(lhs_state, rhs_state, 1, 1), matrix_state_mult(lhs_state, rhs_state, 1, 2)},
		{matrix_state_mult(lhs_state, rhs_state, 2, 0), matrix_state_mult(lhs_state, rhs_state, 2, 1), matrix_state_mult(lhs_state, rhs_state, 2, 2)}
	};

	/*const double s00 = lhs_state[0][0] * rhs_state[0][0] + lhs_state[0][1] * rhs_state[1][0] + lhs_state[0][2] * rhs_state[2][0];
	const double s01 = lhs_state[0][0] * rhs_state[0][1] + lhs_state[0][1] * rhs_state[1][1] + lhs_state[0][2] * rhs_state[2][1];
	const double s02 = lhs_state[0][0] * rhs_state[0][2] + lhs_state[0][1] * rhs_state[1][2] + lhs_state[0][2] * rhs_state[2][2];
	const double s10 = lhs_state[1][0] * rhs_state[0][0] + lhs_state[1][1] * rhs_state[1][0] + lhs_state[1][2] * rhs_state[2][0];
	const double s11 = lhs_state[1][0] * rhs_state[0][1] + lhs_state[1][1] * rhs_state[1][1] + lhs_state[1][2] * rhs_state[2][1];
	const double s12 = lhs_state[1][0] * rhs_state[0][2] + lhs_state[1][1] * rhs_state[1][2] + lhs_state[1][2] * rhs_state[2][2];
	const double s20 = lhs_state[2][0] * rhs_state[0][0] + lhs_state[2][1] * rhs_state[1][0] + lhs_state[2][2] * rhs_state[2][0];
	const double s21 = lhs_state[2][0] * rhs_state[0][1] + lhs_state[2][1] * rhs_state[1][1] + lhs_state[2][2] * rhs_state[2][1];
	const double s22 = lhs_state[2][0] * rhs_state[0][2] + lhs_state[2][1] * rhs_state[1][2] + lhs_state[2][2] * rhs_state[2][2];*/

	//const double state[3][3] = { {s00, s01, s02}, {s10, s11, s12}, {s20, s21, s22} };
	PyObject* rtn = new_matrix(state);
	free_matrix_state(state)
	return rtn;


	/*EVSpace_Matrix* rtn = (EVSpace_Matrix*)new_matrix_empty;
	if (!rtn)
		return NULL;

	Matrix_SET(rtn, 0, 0, MatrixRow_ByMatrix(lhs, rhs, 0, 0));
	Matrix_SET(rtn, 0, 1, MatrixRow_ByMatrix(lhs, rhs, 0, 1));
	Matrix_SET(rtn, 0, 2, MatrixRow_ByMatrix(lhs, rhs, 0, 2));
	Matrix_SET(rtn, 1, 0, MatrixRow_ByMatrix(lhs, rhs, 1, 0));
	Matrix_SET(rtn, 1, 1, MatrixRow_ByMatrix(lhs, rhs, 1, 1));
	Matrix_SET(rtn, 1, 2, MatrixRow_ByMatrix(lhs, rhs, 1, 2));
	Matrix_SET(rtn, 2, 0, MatrixRow_ByMatrix(lhs, rhs, 2, 0));
	Matrix_SET(rtn, 2, 1, MatrixRow_ByMatrix(lhs, rhs, 2, 1));
	Matrix_SET(rtn, 2, 2, MatrixRow_ByMatrix(lhs, rhs, 2, 2));

	return (PyObject*)rtn;*/
}

static PyObject* evspace_mmult_s(const PyObject* lhs, double rhs) {
	const double** lhs_state = ((EVSpace_Matrix*)lhs)->data;

	const double mult_state[3][3] = {
		{lhs_state[0][0] * rhs, lhs_state[0][1] * rhs, lhs_state[0][2] * rhs},
		{lhs_state[1][0] * rhs, lhs_state[1][1] * rhs, lhs_state[1][2] * rhs},
		{lhs_state[2][0] * rhs, lhs_state[2][1] * rhs, lhs_state[2][2] * rhs}
	};

	PyObject* rtn = new_matrix(mult_state);
	free_matrix_state(mult_state)
	return rtn;


	//EVSpace_Matrix* rtn = (EVSpace_Matrix*)new_matrix_empty;
	//if (!rtn)
	//	return NULL;

	///*Matrix_SET(rtn, 0, 0, Matrix_GET(lhs, 0, 0) * rhs);
	//Matrix_SET(rtn, 0, 1, Matrix_GET(lhs, 0, 0) * rhs);
	//Matrix_SET(rtn, 0, 2, Matrix_GET(lhs, 0, 0) * rhs);
	//Matrix_SET(rtn, 1, 0, Matrix_GET(lhs, 0, 0) * rhs);
	//Matrix_SET(rtn, 1, 1, Matrix_GET(lhs, 0, 0) * rhs);
	//Matrix_SET(rtn, 1, 2, Matrix_GET(lhs, 0, 0) * rhs);
	//Matrix_SET(rtn, 2, 0, Matrix_GET(lhs, 0, 0) * rhs);
	//Matrix_SET(rtn, 2, 1, Matrix_GET(lhs, 0, 0) * rhs);
	//Matrix_SET(rtn, 2, 2, Matrix_GET(lhs, 0, 0) * rhs);*/
	//_mat_mmult_s_ex(lhs, rhs, rtn);

	//return (PyObject*)rtn;
}

static PyObject* evspace_mdiv(const PyObject* lhs, double rhs) {
	const double** lhs_state = ((EVSpace_Matrix*)lhs)->data;

	const double mult_state[3][3] = {
		{lhs_state[0][0] / rhs, lhs_state[0][1] / rhs, lhs_state[0][2] / rhs},
		{lhs_state[1][0] / rhs, lhs_state[1][1] / rhs, lhs_state[1][2] / rhs},
		{lhs_state[2][0] / rhs, lhs_state[2][1] / rhs, lhs_state[2][2] / rhs}
	};

	PyObject* rtn = new_matrix(mult_state);
	return rtn;


	/*EVSpace_Matrix* rtn = (EVSpace_Matrix*)new_matrix_empty;
	if (!rtn)
		return NULL;

	Matrix_SET(rtn, 0, 0, Matrix_GET(lhs, 0, 0) / rhs);
	Matrix_SET(rtn, 0, 1, Matrix_GET(lhs, 0, 1) / rhs);
	Matrix_SET(rtn, 0, 2, Matrix_GET(lhs, 0, 2) / rhs);
	Matrix_SET(rtn, 1, 0, Matrix_GET(lhs, 1, 0) / rhs);
	Matrix_SET(rtn, 1, 1, Matrix_GET(lhs, 1, 1) / rhs);
	Matrix_SET(rtn, 1, 2, Matrix_GET(lhs, 1, 2) / rhs);
	Matrix_SET(rtn, 2, 0, Matrix_GET(lhs, 2, 0) / rhs);
	Matrix_SET(rtn, 2, 1, Matrix_GET(lhs, 2, 1) / rhs);
	Matrix_SET(rtn, 2, 2, Matrix_GET(lhs, 2, 2) / rhs);

	return (PyObject*)rtn;*/
}

static void evspace_madd_inplace(PyObject* lhs, const PyObject* rhs) {
	EVSpace_Matrix* lhs_matrix = (EVSpace_Matrix*)lhs;
	EVSpace_Matrix* rhs_matrix = (EVSpace_Matrix*)rhs;

	Matrix_SET(lhs_matrix, 0, 0, Matrix_GET(lhs_matrix, 0, 0) + Matrix_GET(rhs_matrix, 0, 0));
	Matrix_SET(lhs_matrix, 0, 1, Matrix_GET(lhs_matrix, 0, 1) + Matrix_GET(rhs_matrix, 0, 1));
	Matrix_SET(lhs_matrix, 0, 2, Matrix_GET(lhs_matrix, 0, 2) + Matrix_GET(rhs_matrix, 0, 2));
	Matrix_SET(lhs_matrix, 1, 0, Matrix_GET(lhs_matrix, 1, 0) + Matrix_GET(rhs_matrix, 1, 0));
	Matrix_SET(lhs_matrix, 1, 1, Matrix_GET(lhs_matrix, 1, 1) + Matrix_GET(rhs_matrix, 1, 1));
	Matrix_SET(lhs_matrix, 1, 2, Matrix_GET(lhs_matrix, 1, 2) + Matrix_GET(rhs_matrix, 1, 2));
	Matrix_SET(lhs_matrix, 2, 0, Matrix_GET(lhs_matrix, 2, 0) + Matrix_GET(rhs_matrix, 2, 0));
	Matrix_SET(lhs_matrix, 2, 1, Matrix_GET(lhs_matrix, 2, 1) + Matrix_GET(rhs_matrix, 2, 1));
	Matrix_SET(lhs_matrix, 2, 2, Matrix_GET(lhs_matrix, 2, 2) + Matrix_GET(rhs_matrix, 2, 2));
}

static void evspace_msub_inplace(PyObject* lhs, const PyObject* rhs) {
	EVSpace_Matrix* lhs_matrix = (EVSpace_Matrix*)lhs;
	EVSpace_Matrix* rhs_matrix = (EVSpace_Matrix*)rhs;

	Matrix_SET(lhs_matrix, 0, 0, Matrix_GET(lhs_matrix, 0, 0) - Matrix_GET(rhs_matrix, 0, 0));
	Matrix_SET(lhs_matrix, 0, 1, Matrix_GET(lhs_matrix, 0, 1) - Matrix_GET(rhs_matrix, 0, 1));
	Matrix_SET(lhs_matrix, 0, 2, Matrix_GET(lhs_matrix, 0, 2) - Matrix_GET(rhs_matrix, 0, 2));
	Matrix_SET(lhs_matrix, 1, 0, Matrix_GET(lhs_matrix, 1, 0) - Matrix_GET(rhs_matrix, 1, 0));
	Matrix_SET(lhs_matrix, 1, 1, Matrix_GET(lhs_matrix, 1, 1) - Matrix_GET(rhs_matrix, 1, 1));
	Matrix_SET(lhs_matrix, 1, 2, Matrix_GET(lhs_matrix, 1, 2) - Matrix_GET(rhs_matrix, 1, 2));
	Matrix_SET(lhs_matrix, 2, 0, Matrix_GET(lhs_matrix, 2, 0) - Matrix_GET(rhs_matrix, 2, 0));
	Matrix_SET(lhs_matrix, 2, 1, Matrix_GET(lhs_matrix, 2, 1) - Matrix_GET(rhs_matrix, 2, 1));
	Matrix_SET(lhs_matrix, 2, 2, Matrix_GET(lhs_matrix, 2, 2) - Matrix_GET(rhs_matrix, 2, 2));
}

static void evspace_mmult_m_inplace(PyObject* lhs, const PyObject* rhs) {
	double** lhs_state = ((EVSpace_Matrix*)lhs)->data;
	const double** rhs_state = ((EVSpace_Matrix*)rhs)->data;

	const double state[3][3] = {
		{matrix_state_mult(lhs_state, rhs_state, 0, 0), matrix_state_mult(lhs_state, rhs_state, 0, 1), matrix_state_mult(lhs_state, rhs_state, 0, 2)},
		{matrix_state_mult(lhs_state, rhs_state, 1, 0), matrix_state_mult(lhs_state, rhs_state, 1, 1), matrix_state_mult(lhs_state, rhs_state, 1, 2)},
		{matrix_state_mult(lhs_state, rhs_state, 2, 0), matrix_state_mult(lhs_state, rhs_state, 2, 1), matrix_state_mult(lhs_state, rhs_state, 2, 2)}
	};

	memcpy(lhs_state, state, sizeof(lhs_state));


	/*const double ans[3][3] = {
		{MatrixRow_ByMatrix(lhs, rhs, 0, 0), MatrixRow_ByMatrix(lhs, rhs, 0, 1), MatrixRow_ByMatrix(lhs, rhs, 0, 2)},
		{MatrixRow_ByMatrix(lhs, rhs, 1, 0), MatrixRow_ByMatrix(lhs, rhs, 1, 1), MatrixRow_ByMatrix(lhs, rhs, 1, 2)},
		{MatrixRow_ByMatrix(lhs, rhs, 2, 0), MatrixRow_ByMatrix(lhs, rhs, 2, 1), MatrixRow_ByMatrix(lhs, rhs, 2, 2)}
	};

	Matrix_SET(lhs, 0, 0, ans[0][0]);
	Matrix_SET(lhs, 0, 1, ans[0][1]);
	Matrix_SET(lhs, 0, 2, ans[0][2]);
	Matrix_SET(lhs, 1, 0, ans[1][0]);
	Matrix_SET(lhs, 1, 1, ans[1][1]);
	Matrix_SET(lhs, 1, 2, ans[1][2]);
	Matrix_SET(lhs, 2, 0, ans[2][0]);
	Matrix_SET(lhs, 2, 1, ans[2][1]);
	Matrix_SET(lhs, 2, 2, ans[2][2]);*/
}

static void evspace_mmult_s_inplace(PyObject* lhs, double rhs) {
	EVSpace_Matrix* lhs_matrix = (EVSpace_Matrix*)lhs;

	Matrix_SET(lhs_matrix, 0, 0, Matrix_GET(lhs, 0, 0) * rhs);
	Matrix_SET(lhs_matrix, 0, 1, Matrix_GET(lhs, 0, 1) * rhs);
	Matrix_SET(lhs_matrix, 0, 2, Matrix_GET(lhs, 0, 2) * rhs);
	Matrix_SET(lhs_matrix, 1, 0, Matrix_GET(lhs, 1, 0) * rhs);
	Matrix_SET(lhs_matrix, 1, 1, Matrix_GET(lhs, 1, 1) * rhs);
	Matrix_SET(lhs_matrix, 1, 2, Matrix_GET(lhs, 1, 2) * rhs);
	Matrix_SET(lhs_matrix, 2, 0, Matrix_GET(lhs, 2, 0) * rhs);
	Matrix_SET(lhs_matrix, 2, 1, Matrix_GET(lhs, 2, 1) * rhs);
	Matrix_SET(lhs_matrix, 2, 2, Matrix_GET(lhs, 2, 2) * rhs);
}

static PyObject* evspace_mneg(const PyObject* self) {
	const double** self_state = ((EVSpace_Matrix*)self)->data;
	const double neg_state[3][3] = {
		{-self_state[0][0], -self_state[0][1], -self_state[0][2]},
		{-self_state[1][0], -self_state[1][1], -self_state[1][2]},
		{-self_state[2][0], -self_state[2][1], -self_state[2][2]}
	};

	return new_matrix(neg_state);
}


/* get EVSpace_Matrix from argument */
static PyObject* get_other_matrix(PyObject* args) {
	PyObject* other;
	if (!PyArg_ParseTuple(args, "O", &other))
		return NULL;

	return Vector_CheckExact(other) ? other : NULL;
}


/* python interface to matrix numeric methods */
static PyObject* matrix_add(PyObject* self, PyObject* args) {
	PyObject* other = get_other_matrix(args);
	if (!other)
		return NULL;

	return evspace_madd(self, other);
}

static PyObject* matrix_subtract(PyObject* self, PyObject* args) {
	PyObject* other = get_other_matrix(args);
	if (!other)
		return NULL;

	return evspace_msub(self, other);
}

static PyObject* matrix_multiply_v(PyObject* self, PyObject* args) {
	PyObject* other = get_other_vector(args);
	if (!other)
		return NULL;

	return evspace_mmult_v(self, other);
}

static PyObject* matrix_multiply_m(PyObject* self, PyObject* args) {
	PyObject* other = get_other_matrix(args);
	if (!other)
		return NULL;

	return evspace_mmult_m(self, other);
}

static PyObject* matrix_multiply_s(PyObject* self, PyObject* args) {
	double scalar = get_other_numeric(args);
	if (scalar == -1.0 && PyErr_Occurred())
		return NULL;

	return evspace_mmult_s(self, scalar);
}

static PyObject* matrix_divide(PyObject* self, PyObject* args) {
	double scalar = get_other_numeric(args);
	if (scalar == -1.0 && PyErr_Occurred())
		return NULL;

	return evspace_mdiv(self, scalar);
}

static PyObject* matrix_iadd(PyObject* self, PyObject* args) {
	PyObject* other = get_other_matrix(args);
	if (!other)
		return NULL;

	evspace_madd_inplace(self, other);
	return self;
}

static PyObject* matrix_isubtract(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs) && Matrix_Check(rhs)) {
		evspace_msub_inplace(lhs, rhs);
		return lhs;
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* matrix_imultiply(PyObject* lhs, PyObject* rhs) {
	if (Matrix_Check(lhs)) {
		double scalar = PyFloat_AsDouble(rhs);
		if (scalar == -1 && PyErr_Occurred()) {
			PyErr_Clear;
			if (PyLong_Check(rhs)) {
				scalar = PyLong_AsDouble(rhs);
			}
		}


		if (PyNumber_Check(rhs)) {
			double scalar = 0;
			if (PyFloat_Check(rhs)) {
				scalar = PyFloat_AS_DOUBLE(rhs);
			}
		}
	}
}

#endif



#ifdef _NO_INCLUDE

/*************************************/
/*	implimentation of C API methods  */
/*************************************/

// MAJOR TODO: change naming convention to EVSpace_Mat_XXX or EVSpace_Vec_XXX for ALL methods not just numeric methods

/************** EVector **************/

static EVector* EVSpace_Vector_add(const EVector* lhs_matrix, const EVector* rhs_matrix)
{
	EVector* lhs_matrix = (EVector*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs_matrix)
		return NULL;

	lhs_matrix->m_arr[0] = lhs_matrix->m_arr[0] + rhs_matrix->m_arr[0];
	lhs_matrix->m_arr[1] = lhs_matrix->m_arr[1] + rhs_matrix->m_arr[1];
	lhs_matrix->m_arr[2] = lhs_matrix->m_arr[2] + rhs_matrix->m_arr[2];

	return lhs_matrix;
}

static EVector* EVSpace_Vector_sub(const EVector* lhs, const EVector* rhs)
{
	EVector* lhs = (EVector*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	lhs->m_arr[0] = lhs->m_arr[0] - rhs->m_arr[0];
	lhs->m_arr[1] = lhs->m_arr[1] - rhs->m_arr[1];
	lhs->m_arr[2] = lhs->m_arr[2] - rhs->m_arr[2];

	return lhs;
}

static EVector* EVSpace_Vector_mult(const EVector* lhs, double rhs)
{
	EVector* lhs = (EVector*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	lhs->m_arr[0] = lhs->m_arr[0] * rhs;
	lhs->m_arr[1] = lhs->m_arr[1] * rhs;
	lhs->m_arr[2] = lhs->m_arr[2] * rhs;

	return lhs;
}

static EVector* EVSpace_Vector_neg(const EVector* lhs)
{
	EVector* lhs = (EVector*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	lhs->m_arr[0] = -lhs->m_arr[0];
	lhs->m_arr[1] = -lhs->m_arr[1];
	lhs->m_arr[2] = -lhs->m_arr[2];

	return lhs;
}

static double EVSpace_Vector_abs(const EVector* vec)
{
	return sqrt(vec->m_arr[0] * vec->m_arr[0]
		+ vec->m_arr[1] * vec->m_arr[1]
		+ vec->m_arr[2] * vec->m_arr[2]);
}

static void EVSpace_Vector_iadd(EVector* lhs_matrix, const EVector* rhs_matrix)
{
	lhs_matrix->m_arr[0] += rhs_matrix->m_arr[0];
	lhs_matrix->m_arr[1] += rhs_matrix->m_arr[1];
	lhs_matrix->m_arr[2] += rhs_matrix->m_arr[2];
}

static void EVSpace_Vector_isub(EVector* lhs_matrix, const EVector* rhs_matrix)
{
	lhs_matrix->m_arr[0] -= rhs_matrix->m_arr[0];
	lhs_matrix->m_arr[1] -= rhs_matrix->m_arr[1];
	lhs_matrix->m_arr[2] -= rhs_matrix->m_arr[2];
}

static void EVSpace_Vector_imult(EVector* lhs_matrix, double rhs_matrix)
{
	lhs_matrix->m_arr[0] *= rhs_matrix;
	lhs_matrix->m_arr[1] *= rhs_matrix;
	lhs_matrix->m_arr[2] *= rhs_matrix;
}

static EVector* EVSpace_Vector_div(const EVector* lhs_matrix, double rhs_matrix)
{
	EVector* lhs_matrix = (EVector*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs_matrix)
		return NULL;

	lhs_matrix->m_arr[0] = lhs_matrix->m_arr[0] / rhs_matrix;
	lhs_matrix->m_arr[1] = lhs_matrix->m_arr[1] / rhs_matrix;
	lhs_matrix->m_arr[2] = lhs_matrix->m_arr[2] / rhs_matrix;

	return lhs_matrix;
}

static void EVSpace_Vector_idiv(EVector* lhs_matrix, double rhs_matrix)
{
	lhs_matrix->m_arr[0] /= rhs_matrix;
	lhs_matrix->m_arr[1] /= rhs_matrix;
	lhs_matrix->m_arr[2] /= rhs_matrix;
}

static int EVSpace_Vector_eq(const EVector* lhs_matrix, const EVector* rhs_matrix)
{
	static double epsilon = 1e-6;
	if ((fabs(lhs_matrix->m_arr[0] - rhs_matrix->m_arr[0]) < epsilon) 
		&& (fabs(lhs_matrix->m_arr[1] - rhs_matrix->m_arr[1]) < epsilon) 
		&& (fabs(lhs_matrix->m_arr[2] - rhs_matrix->m_arr[2]) < epsilon))
		return 1;
	return 0;
}

static int EVSpace_Vector_ne(const EVector* lhs_matrix, const EVector* rhs_matrix)
{
	if (EVSpace_Vector_eq(lhs_matrix, rhs_matrix))
		return 0;
	return 1;
}

static double EVSpace_Vector_dot(const EVector* lhs_matrix, const EVector* rhs_matrix)
{
	return (lhs_matrix->m_arr[0] * rhs_matrix->m_arr[0])
		+ (lhs_matrix->m_arr[1] * rhs_matrix->m_arr[1])
		+ (lhs_matrix->m_arr[2] * rhs_matrix->m_arr[2]);
}

static EVector* EVSpace_Vector_cross(const EVector* lhs_matrix, const EVector* rhs_matrix)
{
	EVector* lhs_matrix = (EVector*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs_matrix)
		return NULL;

	lhs_matrix->m_arr[0] = ((lhs_matrix->m_arr[1] * rhs_matrix->m_arr[2]) - (lhs_matrix->m_arr[2] * rhs_matrix->m_arr[1]));
	lhs_matrix->m_arr[1] = ((lhs_matrix->m_arr[2] * rhs_matrix->m_arr[0]) - (lhs_matrix->m_arr[0] * rhs_matrix->m_arr[2]));
	lhs_matrix->m_arr[2] = ((lhs_matrix->m_arr[0] * rhs_matrix->m_arr[1]) - (lhs_matrix->m_arr[1] * rhs_matrix->m_arr[0]));

	return lhs_matrix;
}

static double EVSpace_vector_mag(const EVector* vec)
{
	return sqrt(EVSpace_Vector_dot(vec, vec));
}

static double EVSpace_Vector_mag2(const EVector* vec)
{
	return EVSpace_Vector_dot(vec, vec);
}

static EVector* EVSpace_Vector_norm(const EVector* vec)
{
	return EVSpace_Vector_div(vec, sqrt(EVSpace_Vector_dot(vec, vec)));
}

static void EVSpace_Vector_inorm(EVector* lhs_matrix)
{
	EVSpace_Vector_idiv(lhs_matrix, sqrt(EVSpace_Vector_dot(lhs_matrix, lhs_matrix)));
}

static double EVSpace_Vector_vang(const EVector* lhs_matrix, const EVector* rhs_matrix)
{
	EVector* rhsNorm = EVSpace_Vector_norm(rhs_matrix);
	if (EVSpace_Vector_norm(lhs_matrix) == rhsNorm)
		return 0;
	else if (EVSpace_Vector_norm(EVSpace_Vector_neg(lhs_matrix)) == rhsNorm)
		return PI;
	return acos(EVSpace_Vector_dot(lhs_matrix, rhs_matrix) / (EVSpace_vector_mag(lhs_matrix) * EVSpace_vector_mag(rhs_matrix)));
}

static EVector* EVSpace_Vector_vxcl(const EVector* vec, const EVector* xcl)
{
	EVector* lhs_matrix = (EVector*)vec->ob_base.ob_type->tp_new(vec->ob_base.ob_type, NULL, NULL);
	if (!lhs_matrix)
		return NULL;

	double scale = EVSpace_Vector_dot(vec, xcl) / EVSpace_Vector_mag2(xcl);
	lhs_matrix->m_arr[0] = vec->m_arr[0] - (xcl->m_arr[0] * scale);
	lhs_matrix->m_arr[1] = vec->m_arr[1] - (xcl->m_arr[1] * scale);
	lhs_matrix->m_arr[2] = vec->m_arr[2] - (xcl->m_arr[2] * scale);

	return lhs_matrix;
}

/*********************************************/
/***************** EMatrix *******************/
/*********************************************/

static EMatrix* EVSpace_Matrix_add(const EMatrix* lhs, const EMatrix* rhs)
{
	EMatrix* lhs = (EMatrix*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] = lhs->m_arr[i][j] + rhs->m_arr[i][j];
	}

	return lhs;
}

static EMatrix* EVSpace_Matrix_sub(const EMatrix* lhs, const EMatrix* rhs)
{
	EMatrix* lhs = (EMatrix*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] = lhs->m_arr[i][j] - rhs->m_arr[i][j];
	}

	return lhs;
}

static EMatrix* EVSpace_Matrix_multm(const EMatrix* lhs, const EMatrix* rhs)
{
	EMatrix* lhs = (EMatrix*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			double sum = 0;
			for (int k = 0; k < 3; k++)
				sum += lhs->m_arr[i][k] * rhs->m_arr[k][j];
			lhs->m_arr[i][j] = sum;
		}
	}

	return lhs;
}

static EVector* EVSpace_Matrix_multv(const EMatrix* lhs, const EVector* rhs)
{
	EVector* lhs = (EVector*)rhs_matrix->ob_base.ob_type->tp_new(rhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	for (int i = 0; i < 3; i++) {
		double sum = 0;
		for (int j = 0; j < 3; j++)
			sum += lhs->m_arr[i][j] * rhs->m_arr[j];
		lhs->m_arr[i] = sum;
	}

	return lhs;
}

static EMatrix* EVSpace_Matrix_multd(const EMatrix* lhs, double rhs)
{
	EMatrix* lhs = (EMatrix*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] = lhs->m_arr[i][j] * rhs;
	}

	return lhs;
}

static EMatrix* EVSpace_Matrix_neg(const EMatrix* mat)
{
	EMatrix* lhs_matrix = (EMatrix*)mat->ob_base.ob_type->tp_new(mat->ob_base.ob_type, NULL, NULL);
	if (!lhs_matrix)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs_matrix->m_arr[i][j] = -mat->m_arr[i][j];
	}

	return lhs_matrix;
}

static void EVSpace_Matrix_iadd(EMatrix* lhs, const EMatrix* rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] += rhs->m_arr[i][j];
	}
}

static void EVSpace_Matrix_isub(EMatrix* lhs, const EMatrix* rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] -= rhs->m_arr[i][j];
	}
}

static void EVSpace_Matrix_imultm(EMatrix* lhs, const EMatrix* rhs)
{
	EMatrix* tmp = EVSpace_Matrix_multm(lhs, rhs);
	if (!tmp) {
		lhs = NULL;
		return;
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] = tmp->m_arr[i][j];
	}

	tmp->ob_base.ob_type->tp_dealloc((PyObject*)tmp);
}

static void EVSpace_Matrix_imultd(EMatrix* lhs, double rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] *= rhs;
	}
}

static EMatrix* EVSpace_Matrix_div(const EMatrix* lhs, double rhs)
{
	EMatrix* lhs = (EMatrix*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);
	if (!lhs)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] = lhs->m_arr[i][j] / rhs;
	}

	return lhs;
}

static void EVSpace_Matrix_idiv(EMatrix* lhs, double rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] /= rhs;
	}
}

static int EVSpace_Matrix_eq(const EMatrix* lhs, const EMatrix* rhs)
{
	static double epsilon = 1e-6;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (fabs(lhs->m_arr[i][j] - rhs->m_arr[i][j]) > epsilon) 
				return 0;
		}
	}
	return 1;
}

static int EVSpace_Matrix_ne(const EMatrix* lhs, const EMatrix* rhs)
{
	if (EVSpace_Matrix_eq(lhs, rhs) == 1)
		return 0;
	return 1;
}

static double EVSpace_Matrix_det(const EMatrix* lhs)
{
	return lhs->m_arr[0][0] * ((lhs->m_arr[1][1] * lhs->m_arr[2][2]) - (lhs->m_arr[1][2] * lhs->m_arr[2][1]))
		+ lhs->m_arr[0][1] * ((lhs->m_arr[1][2] * lhs->m_arr[2][0]) - (lhs->m_arr[1][0] * lhs->m_arr[2][2]))
		+ lhs->m_arr[0][2] * ((lhs->m_arr[1][0] * lhs->m_arr[2][1]) - (lhs->m_arr[1][1] * lhs->m_arr[2][0]));
}

static EMatrix* EVSpace_Matrix_trans(const EMatrix* mat)
{
	EMatrix* lhs_matrix = (EMatrix*)mat->ob_base.ob_type->tp_new(mat->ob_base.ob_type, NULL, NULL);
	if (!lhs_matrix)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs_matrix->m_arr[i][j] = mat->m_arr[j][i];
	}

	return lhs_matrix;
}

/****************************************************/
/*	Implimenting C API into Python number metehods  */
/****************************************************/

/****************************************************/
/************** EVector implementations *************/
/****************************************************/

/****************************************************/
/************* EVector Numeric Methods **************/
/****************************************************/

static PyObject* EVector_add(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Vector_add(self, (EVector*)arg);
}

static PyObject* EVector_sub(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Vector_sub(self, (EVector*)arg);
}

static PyObject* EVector_mult(EVector* self, PyObject* arg)
{
	double rhs;
	
	if (PyFloat_CheckExact(arg))
		rhs = PyFloat_AS_DOUBLE(arg);
	else if (PyLong_CheckExact(arg)) {
		rhs = PyLong_AsDouble(arg);
		if (PyErr_Occurred())
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Vector_mult(self, rhs);
}

static PyObject* EVector_neg(EVector* self)
{
	return (PyObject*)EVSpace_Vector_neg(self);
}

static PyObject* EVector_abs(EVector* self)
{
	return (PyObject*)PyFloat_FromDouble(EVSpace_Vector_abs(self));
}

static PyObject* EVector_iadd(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Vector_iadd(self, (EVector*)arg);
	return Py_NewRef(self);
}

static PyObject* EVector_isub(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Vector_isub(self, (EVector*)arg);
	return Py_NewRef(self);
}

static PyObject* EVector_imult(EVector* self, PyObject* arg)
{
	double rhs;
	if (PyFloat_CheckExact(arg))
		rhs = PyFloat_AS_DOUBLE(arg);
	else if (PyLong_CheckExact(arg)) {
		rhs = PyLong_AsDouble(arg);
		if (PyErr_Occurred())
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Vector_imult(self, rhs);
	return Py_NewRef(self);
}

static PyObject* EVector_div(EVector* self, PyObject* arg)
{
	double rhs;
	if (PyFloat_CheckExact(arg))
		rhs = PyFloat_AS_DOUBLE(arg);
	else if (PyLong_CheckExact(arg)) {
		rhs = PyLong_AsDouble(arg);
		if (PyErr_Occurred())
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Vector_div(self, rhs);
}

static PyObject* EVector_idiv(EVector* self, PyObject* arg)
{
	double rhs;
	if (PyFloat_CheckExact(arg))
		rhs = PyFloat_AS_DOUBLE(arg);
	else if (PyLong_CheckExact(arg)) {
		rhs = PyLong_AsDouble(arg);
		if (PyErr_Occurred())
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Vector_idiv(self, rhs);
	return Py_NewRef(self);
}

static PyNumberMethods EVector_NBMethods = {
	.nb_add = (binaryfunc)EVector_add,
	.nb_subtract = (binaryfunc)EVector_sub,
	.nb_multiply = (binaryfunc)EVector_mult,
	.nb_negative = (unaryfunc)EVector_neg,
	.nb_absolute = (unaryfunc)EVector_abs,
	.nb_inplace_add = (binaryfunc)EVector_iadd,
	.nb_inplace_subtract = (binaryfunc)EVector_isub,
	.nb_inplace_multiply = (binaryfunc)EVector_imult,
	.nb_true_divide = (binaryfunc)EVector_div,
	.nb_inplace_true_divide = (binaryfunc)EVector_idiv,
};

/****************************************************/
/************** EVector Class Methods ***************/
/****************************************************/

static PyObject* EVector_Mag(EVector* self, PyObject* UNUSED)
{
	return PyFloat_FromDouble(EVSpace_vector_mag(self));
}

static PyObject* EVector_Mag2(EVector* self, PyObject* UNUSED)
{
	return PyFloat_FromDouble(EVSpace_Vector_mag2(self));
}

static PyObject* EVector_Normalize(EVector* self, PyObject* UNUSED)
{
	EVSpace_Vector_inorm(self);
	Py_RETURN_NONE;
}

static PyObject* EVector_Copy(EVector* self, PyObject* UNUSED)
{
	EVector* lhs_matrix = (EVector*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);

	lhs_matrix->m_arr[0] = self->m_arr[0];
	lhs_matrix->m_arr[1] = self->m_arr[1];
	lhs_matrix->m_arr[2] = self->m_arr[2];

	return (PyObject*)lhs_matrix;
}

static PyMethodDef EVector_Methods[] = {
	{"mag", (PyCFunction)EVector_Mag, METH_NOARGS, "Returns the magnitude of an EVector."},
	{"mag2", (PyCFunction)EVector_Mag2, METH_NOARGS, "Returns the square of the magnitude of an EVector."},
	{"normalize", (PyCFunction)EVector_Normalize, METH_NOARGS, "Normalizes an EVector."},
	{"copy", (PyCFunction)EVector_Copy, METH_NOARGS, "Returns a deep copy of an EVector."},
	{NULL}
};

/****************************************************/
/************* EVector Module Methods ***************/
/****************************************************/

static PyObject* EVector_Dot(PyObject* UNUSED, PyObject *const *args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_SetString(PyExc_TypeError, "dot() takes exactly 2 arguments");
		return NULL;
	}
 
	EVector* lhs_matrix = (EVector*)args[0];
	EVector* rhs_matrix = (EVector*)args[1];

	if (!lhs_matrix && !rhs_matrix) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}
	if (!PyObject_TypeCheck(rhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Vector_dot(lhs_matrix, rhs_matrix));
}

static PyObject* EVector_Cross(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_SetString(PyExc_TypeError, "cross() takes exactly two arguments");
		return NULL;
	}

	EVector* lhs_matrix = (EVector*)args[0];
	EVector* rhs_matrix = (EVector*)args[1];

	if (!lhs_matrix && !rhs_matrix) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}
	if (!PyObject_TypeCheck(rhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return NULL;
	}

	EVector* lhs_matrix = EVSpace_Vector_cross(lhs_matrix, rhs_matrix);

	return (PyObject*)(lhs_matrix);
}

static PyObject* EVector_Norm(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_SetString(PyExc_TypeError, "norm() takes exactly one argument");
		return NULL;
	}

	EVector* lhs_matrix = (EVector*)args[0];

	if (!lhs_matrix) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}

	return (PyObject*)EVSpace_Vector_norm(lhs_matrix);
}

static PyObject* EVector_Vang(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_SetString(PyExc_TypeError, "vang takes exactly two arguments");
		return NULL;
	}

	EVector* lhs_matrix = (EVector*)args[0];
	EVector* rhs_matrix = (EVector*)args[1];

	if (!lhs_matrix && !rhs_matrix) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}
	if (!PyObject_TypeCheck(rhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Vector_vang(lhs_matrix, rhs_matrix));
}

static PyObject* EVector_Vxcl(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_SetString(PyExc_TypeError, "vxcl() takes exactly two arguments");
		return NULL;
	}

	EVector* lhs_matrix = (EVector*)args[0];
	EVector* rhs_matrix = (EVector*)args[1];

	if (!lhs_matrix && !rhs_matrix) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}
	if (!PyObject_TypeCheck(rhs_matrix, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return NULL;
	}

	return (PyObject*)(EVSpace_Vector_vxcl(lhs_matrix, rhs_matrix));
}

static PyMethodDef EVector_ModuleMethods[] = {
	{"dot", (PyCFunction)EVector_Dot, METH_FASTCALL, "Returns the dot product or inner produce of two EVectors."},
	{"cross", (PyCFunction)EVector_Cross, METH_FASTCALL, "Returns the cross product of two EVectors."},
	{"norm", (PyCFunction)EVector_Norm, METH_FASTCALL, "Returns a normalized version of an EVector."},
	{"vang", (PyCFunction)EVector_Vang, METH_FASTCALL, "Returns the shortest angle between two EVector's."},
	{"vxcl", (PyCFunction)EVector_Vxcl, METH_FASTCALL, "Returns a vector exculded from another."},
	{NULL}
};

/****************************************************/
/************* EVector Sequence Methods *************/
/****************************************************/

static Py_ssize_t EVector_length(EVector* self)
{
	return 3;
}

static PyObject* EVector_get(EVector* self, Py_ssize_t index)
{
	if (index < 0 || index > 2) {
		PyErr_SetString(PyExc_ValueError, "Index out of bounds.");
		return NULL;
	}

	return PyFloat_FromDouble(self->m_arr[index]);
}

static int EVector_set(EVector* self, Py_ssize_t index, PyObject* val)
{
	if (index < 0 || index > 2) {
		PyErr_SetString(PyExc_ValueError, "Index out of bounds.");
		return -1;
	}

	if (PyFloat_CheckExact(val))
		self->m_arr[index] = PyFloat_AS_DOUBLE(val);
	else if (PyLong_CheckExact(val)) {
		double rhs = PyLong_AsDouble(val);
		if (PyErr_Occurred())
			return -1;
		self->m_arr[index] = rhs;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "Assignment value must be float or int type.");
	}

	return 0;
}

static PySequenceMethods EVector_sequence = {
	.sq_length = (lenfunc)EVector_length,
	.sq_item = (ssizeargfunc)EVector_get,
	.sq_ass_item = (ssizeobjargproc)EVector_set,
};

/****************************************************/
/************** EVector Type Methods ****************/
/****************************************************/

static int EVector_init(EVector* self, PyObject* args, PyObject* kwds)
{
	double x = 0, y = 0, z = 0;
	static char* kwlist[] = { "x", "y", "z", NULL };
		
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ddd", kwlist, &x, &y, &z))
		return -1;

	self->m_arr[0] = x;
	self->m_arr[1] = y;
	self->m_arr[2] = z;

	return 0;
}

static PyObject* EVector_str(const EVector* vec)
{
	char buffer[50]; // todo: find the accurate number here
	int ok = snprintf(buffer, 50, "[ %f, %f, %f ]", vec->m_arr[0], vec->m_arr[1], vec->m_arr[2]);

	if (ok < 0 || ok > 50) {
		PyErr_SetString(PyExc_BufferError, "Buffer too small to create string.");
		return NULL;
	}
	
	return Py_BuildValue("s#", buffer, strlen(buffer));
}

static PyObject* EVector_richcompare(PyObject* self, PyObject* other, int op)
{
	if (!PyObject_TypeCheck(other, self->ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	switch (op) {
	case Py_EQ:
		return PyBool_FromLong(EVSpace_Vector_eq((EVector*)lhs_matrix, (EVector*)rhs_matrix));
	case Py_NE:
		return PyBool_FromLong(EVSpace_Vector_ne((EVector*)lhs_matrix, (EVector*)rhs_matrix));
	default:
		Py_RETURN_NOTIMPLEMENTED;
	}
}

PyDoc_STRVAR(evector_doc, "Euclidean vector object to represent a spatial vector in three dimensional space.");

static PyTypeObject EVectorType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name		= "pyevspace.EVector",
	.tp_doc			= evector_doc,
	.tp_basicsize	= sizeof(EVector),
	.tp_itemsize	= 0,
	.tp_flags		= Py_TPFLAGS_DEFAULT, // todo: do we want to set Py_TPFLAGS_SEQUENCE flag?
	.tp_new			= PyType_GenericNew,
	.tp_init		= (initproc)EVector_init,
	.tp_methods		= EVector_Methods,
	.tp_str			= (reprfunc)EVector_str,
	.tp_as_number	= &EVector_NBMethods,
	.tp_richcompare	= (richcmpfunc)EVector_richcompare,
	.tp_as_sequence = &EVector_sequence,
};

/****************************************************/
/************** EMatrix implementation **************/
/****************************************************/

/****************************************************/
/************* EMatrix Numeric Methods **************/
/****************************************************/

static PyObject* EMatrix_add(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Matrix_add(self, (EMatrix*)args);
}

static PyObject* EMatrix_sub(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Matrix_sub(self, (EMatrix*)args);
}

static PyObject* EMatrix_mult(EMatrix* self, PyObject* args)
{
	if (PyFloat_CheckExact(args))
		return (PyObject*)EVSpace_Matrix_multd(self, PyFloat_AS_DOUBLE(args));
	else if (PyLong_CheckExact(args)) {
		double rhs = PyLong_AsDouble(args);
		if (PyErr_Occurred())
			return NULL;

		return (PyObject*)EVSpace_Matrix_multd(self, PyLong_AsDouble(args));
	}
	else
		Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* EMatrix_mmult(EMatrix* self, PyObject* args)
{
	if (PyObject_TypeCheck(args, &EMatrixType))
		return (PyObject*)EVSpace_Matrix_multm(self, (EMatrix*)args);
	else if (PyObject_TypeCheck(args, &EVectorType))
		return (PyObject*)EVSpace_Matrix_multv(self, (EVector*)args);
	else
		Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* EMatrix_neg(EMatrix* self, PyObject* UNUSED)
{
	return (PyObject*)EVSpace_Matrix_neg(self);
}

static PyObject* EMatrix_iadd(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Matrix_iadd(self, (EMatrix*)args);
	return Py_NewRef(self);
}

static PyObject* EMatrix_isub(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Matrix_isub(self, (EMatrix*)args);
	return Py_NewRef(self);
}

static PyObject* EMatrix_imult(EMatrix* self, PyObject* args)
{
	if (PyFloat_CheckExact(args)) {
		EVSpace_Matrix_imultd(self, PyFloat_AS_DOUBLE(args));
		return Py_NewRef(self);
	}
	else if (PyLong_CheckExact(args)) {
		double rhs = PyLong_AsDouble(args);
		if (PyErr_Occurred())
			return NULL;

		EVSpace_Matrix_imultd(self, rhs);
		return Py_NewRef(self);
	}
	else
		Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* EMatrix_mimult(EMatrix* self, PyObject* args)
{
	if (PyObject_TypeCheck(args, &EMatrixType)) {
		EVSpace_Matrix_imultm(self, (EMatrix*)args);
		return Py_NewRef(self);
	}
	else
		Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* EMatrix_div(EMatrix* self, PyObject* args)
{
	double rhs;
	if (PyFloat_CheckExact(args))
		rhs = PyFloat_AS_DOUBLE(args);
	else if (PyLong_CheckExact(args)) {
		rhs = PyLong_AsDouble(args);
		if (PyErr_Occurred())
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Matrix_div(self, rhs);
}

static PyObject* EMatrix_idiv(EMatrix* self, PyObject* args)
{
	double rhs;
	if (PyFloat_CheckExact(args))
		rhs = PyFloat_AS_DOUBLE(args);
	else if (PyLong_CheckExact(args)) {
		rhs = PyLong_AsDouble(args);
		if (PyErr_Occurred())
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Matrix_idiv(self, rhs);
	return Py_NewRef(self);
}

// todo: split the multiply into multiply(float/int) and matrix multiply(vector/matrix)
static PyNumberMethods EMatrix_NBMethods = {
	.nb_add = (binaryfunc)EMatrix_add,
	.nb_subtract = (binaryfunc)EMatrix_sub,
	.nb_multiply = (binaryfunc)EMatrix_mult,
	.nb_negative = (unaryfunc)EMatrix_neg,
	.nb_inplace_add = (binaryfunc)EMatrix_iadd,
	.nb_inplace_subtract = (binaryfunc)EMatrix_isub,
	.nb_inplace_multiply = (binaryfunc)EMatrix_imult,
	.nb_true_divide = (binaryfunc)EMatrix_div,
	.nb_inplace_true_divide = (binaryfunc)EMatrix_idiv,
	.nb_matrix_multiply = (binaryfunc)EMatrix_mmult,
	.nb_inplace_matrix_multiply = (binaryfunc)EMatrix_mimult,
};

/****************************************************/
/************** EMatrix Class Methods ***************/
/****************************************************/

static PyObject* EMatrix_set(EMatrix* self, PyObject* args)
{
	int i, j;
	double val;

	if (!PyArg_ParseTuple(args, "lld", &i, &j, &val))
		return NULL;

	if (i < 0 || i > 2) {
		PyErr_SetString(PyExc_ValueError, "Row index out of bounds.");
		return NULL;
	}
	if (j < 0 || j > 2) {
		PyErr_SetString(PyExc_ValueError, "Column index out of bounds.");
		return NULL;
	}

	self->m_arr[i][j] = val;
	Py_RETURN_NONE;
}

static PyObject* EMatrix_get(EMatrix* self, PyObject* args)
{
	int i, j;

	if (!PyArg_ParseTuple(args, "ll", &i, &j))
		return NULL;

	if (i < 0 || i > 2) {
		PyErr_SetString(PyExc_ValueError, "Row index out of bounds.");
		return NULL;
	}
	if (j < 0 || j > 2) {
		PyErr_SetString(PyExc_ValueError, "Column index out of bounds.");
		return NULL;
	}

	return PyFloat_FromDouble(self->m_arr[i][j]);
}

static PyObject* EMatrix_copy(EMatrix* self, PyObject* UNUSED)
{
	EMatrix* lhs_matrix = (EMatrix*)lhs_matrix->ob_base.ob_type->tp_new(lhs_matrix->ob_base.ob_type, NULL, NULL);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs_matrix->m_arr[i][j] = self->m_arr[i][j];
	}

	return (PyObject*)lhs_matrix;
}

// todo: can we manage these with a sequence like protocol?
static PyMethodDef EMatrix_Methods[] = {
	{"set", (PyCFunction)EMatrix_set, METH_VARARGS, "Sets a matrix comonent to a given value."},
	{"get", (PyCFunction)EMatrix_get, METH_VARARGS, "Returns a matrix comonent."},
	{"copy", (PyCFunction)EMatrix_copy, METH_NOARGS, "Returns a deep copy of an EMatrix."},
	{NULL}
};

/****************************************************/
/************* EMatrix Module Methods ***************/
/****************************************************/

static PyObject* EMatrix_det(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_SetString(PyExc_TypeError, "det takes exactly one argument");
		return NULL;
	}

	EMatrix* mat = (EMatrix*)args[0];

	if (!PyObject_TypeCheck(mat, &EMatrixType)) {
		PyErr_SetString(PyExc_TypeError, "Argument should be EMatrix type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Matrix_det(mat));
}

static PyObject* EMatrix_trans(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_SetString(PyExc_TypeError, "transpose() takes exactly one argument");
		return NULL;
	}

	EMatrix* mat = (EMatrix*)args[0];

	if (!PyObject_TypeCheck(mat, &EMatrixType)) {
		PyErr_SetString(PyExc_TypeError, "Argument should be EMatrix type.");
		return NULL;
	}

	return (PyObject*)EVSpace_Matrix_trans(mat);
}

static PyMethodDef EMatrix_ModuleMethods[] = {
	{"det", (PyCFunction)EMatrix_det, METH_FASTCALL, "Returns the determinate of a matrix."},
	{"transpose", (PyCFunction)EMatrix_trans, METH_FASTCALL, "Returns the transpose of a matrix."},
	{NULL}
};

/****************************************************/
/************** EMatrix Type Methods ****************/
/****************************************************/

static int EMatrix_init(EMatrix* self, PyObject* args, PyObject* UNUSED)
{
	PyObject* sqnce = NULL;

	if (!PyArg_ParseTuple(args, "|O", &sqnce))
		return -1;

	// default constructor
	if (!sqnce) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++)
				self->m_arr[i][j] = 0.0;
		}
		return 0;
	}

	if (!PySequence_Check(sqnce)) {
		PyErr_SetString(PyExc_TypeError, "Initializing argument must be a sequence type.");
		return -1;
	}

	PyObject* c0 = NULL, * c1 = NULL, * c2 = NULL, * c3 = NULL;
	c0 = PySequence_GetItem(sqnce, 0);
	if (!c0) {
		PyErr_SetString(PyExc_Exception, "Failed to get first column sequence.");
		goto failed;
	}
	else if (!PySequence_Check(c0)) {
		PyErr_SetString(PyExc_TypeError, "First element of initializing argument must be a sequence type.");
		goto failed;
	}
	c1 = PySequence_GetItem(sqnce, 1);
	if (!c1) {
		PyErr_SetString(PyExc_Exception, "Failed to get second column sequence.");
		goto failed;
	}
	else if (!PySequence_Check(c1)) {
		PyErr_SetString(PyExc_TypeError, "Second element of initializing argument must be a sequence type.");
		goto failed;
	}
	c2 = PySequence_GetItem(sqnce, 2);
	if (!c1) {
		PyErr_SetString(PyExc_Exception, "Failed to get third column sequence.");
		goto failed;
	}
	else if (!PySequence_Check(c2)) {
		PyErr_SetString(PyExc_TypeError, "Third element of initializing argument must be a sequence type.");
		goto failed;
	}

	PyObject* columns[3] = {c0, c1, c2};
	for (int i = 0; i < 3; i++) { // columns
		for (int j = 0; j < 3; j++) { // rows
			PyObject* pyVal = PySequence_GetItem(columns[i], j);
			if (!pyVal) {
				PyErr_SetString(PyExc_Exception, "Failed to get value from sequence.");
				goto failed;
			}

			double val = PyFloat_AsDouble(pyVal);
			Py_DECREF(pyVal);
			if (val == -1.0 && PyErr_Occurred()) {
				goto failed;
			}
			else self->m_arr[j][i] = val;
		}
	}

	Py_DECREF(c0);
	Py_DECREF(c1);
	Py_DECREF(c2);

	return 0;

failed:
	Py_XDECREF(c0);
	Py_XDECREF(c1);
	Py_XDECREF(c2);
	return -1;
}

static PyObject* EMatrix_str(const EMatrix* mat)
{
	const static int sz_buffer = 200;
	char buffer[200]; // todo: find the accurate number here
	int ok = snprintf(buffer, sz_buffer, "[ %f, %f, %f ]\n[ %f, %f, %f ]\n[ %f, %f, %f ]",
		mat->m_arr[0][0], mat->m_arr[0][1], mat->m_arr[0][2],
		mat->m_arr[1][0], mat->m_arr[1][1], mat->m_arr[1][2],
		mat->m_arr[2][0], mat->m_arr[2][1], mat->m_arr[2][2]);

	if (ok < 0 || ok > sz_buffer) {
		PyErr_SetString(PyExc_BufferError, "Buffer too small to create string.");
		return NULL;
	}

	return Py_BuildValue("s#", buffer, strlen(buffer));
}

static PyObject* EMatrix_richcompare(EMatrix* self, PyObject* other, int op)
{
	if (!PyObject_TypeCheck(other, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EMatrix type.");
		return NULL;
	}

	switch (op) {
	case Py_EQ:
		return PyBool_FromLong(EVSpace_Matrix_eq((EMatrix*)lhs_matrix, (EMatrix*)rhs_matrix));
	case Py_NE:
		return PyBool_FromLong(EVSpace_Matrix_ne((EMatrix*)lhs_matrix, (EMatrix*)rhs_matrix));
	default:
		Py_RETURN_NOTIMPLEMENTED;
	}
}

PyDoc_STRVAR(ematrix_doc, "Euclidean matrix object used to operate on or rotate a euclidean vector.");

static PyTypeObject EMatrixType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyevspace.EMatrix",
	.tp_doc = ematrix_doc,
	.tp_basicsize = sizeof(EMatrix),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = PyType_GenericNew,
	.tp_init = (initproc)EMatrix_init,
	.tp_methods = EMatrix_Methods,
	.tp_str = (reprfunc)EMatrix_str,
	.tp_as_number = &EMatrix_NBMethods,
	.tp_richcompare = (richcmpfunc)EMatrix_richcompare,
};

PyDoc_STRVAR(evspace_doc, "Module library for a Euclidean vector space with a vector and matrix type as well as necessary methods to use them.");

static PyModuleDef EVSpace_Module = {
	PyModuleDef_HEAD_INIT,
	.m_name = "pyevspace",
	.m_doc = evspace_doc,
	.m_size = -1,
};

// todo: can we document all the methods for the IDE?
PyMODINIT_FUNC
PyInit_pyevspace(void)
{
	PyObject* m = NULL;
	static void* EVSpace_API[EVSpace_API_pointers];
	PyObject* c_api_object = NULL;
	EVector* e1 = NULL, *e2 = NULL, *e3 = NULL;
	EMatrix* I = NULL;
	
	if (PyType_Ready(&EVectorType) < 0)
		return NULL;
	if (PyType_Ready(&EMatrixType) < 0)
		return NULL;

	// create e1 vector
	e1 = (EVector*)EVectorType.tp_new(&EVectorType, NULL, NULL);
	if (!e1)
		return NULL;
	e1->m_arr[0] = 1;
	if (PyDict_SetItemString(EVectorType.tp_dict, "e1", (PyObject*)e1) < 0)
		return NULL;
	Py_DECREF(e1);

	// create e2 vector
	e2 = (EVector*)EVectorType.tp_new(&EVectorType, NULL, NULL);
	if (!e2)
		return NULL;
	e2->m_arr[1] = 1;
	if (PyDict_SetItemString(EVectorType.tp_dict, "e2", (PyObject*)e2) < 0)
		return NULL;
	Py_DECREF(e2);

	// craete e3 vector
	e3 = (EVector*)EVectorType.tp_new(&EVectorType, NULL, NULL);
	if (!e3)
		return NULL;
	e3->m_arr[2] = 1;
	if (PyDict_SetItemString(EVectorType.tp_dict, "e3", (PyObject*)e3) < 0)
		return NULL;
	Py_DECREF(e3);

	// create identity matrix
	I = (EMatrix*)EMatrixType.tp_new(&EMatrixType, NULL, NULL);
	if (!I)
		return NULL;
	I->m_arr[0][0] = I->m_arr[1][1] = I->m_arr[2][2] = 1.0;
	if (PyDict_SetItemString(EMatrixType.tp_dict, "I", (PyObject*)I) < 0)
		return NULL;
	Py_DECREF(I);

	// create module
	m = PyModule_Create(&EVSpace_Module);
	if (!m)
		goto error;

	// add module level class functions
	if (PyModule_AddFunctions(m, EMatrix_ModuleMethods) < 0) {
		Py_DECREF(m);
		return NULL;
	}
	if (PyModule_AddFunctions(m, EVector_ModuleMethods) < 0) {
		Py_DECREF(m);
		return NULL;
	}

	// EVector 
	EVSpace_API[EVSpace_Vector_add_NUM]		= (void*)EVSpace_Vector_add;
	EVSpace_API[EVSpace_Vector_sub_NUM]		= (void*)EVSpace_Vector_sub;
	EVSpace_API[EVSpace_Vector_mult_NUM]	= (void*)EVSpace_Vector_mult;
	EVSpace_API[EVSpace_Vector_neg_NUM]		= (void*)EVSpace_Vector_neg;
	EVSpace_API[EVSpace_Vector_abs_NUM]		= (void*)EVSpace_Vector_abs;
	EVSpace_API[EVSpace_Vector_iadd_NUM]	= (void*)EVSpace_Vector_iadd;
	EVSpace_API[EVSpace_Vector_isub_NUM]	= (void*)EVSpace_Vector_isub;
	EVSpace_API[EVSpace_Vector_imult_NUM]	= (void*)EVSpace_Vector_imult;
	EVSpace_API[EVSpace_Vector_div_NUM]		= (void*)EVSpace_Vector_div;
	EVSpace_API[EVSpace_Vector_idiv_NUM]	= (void*)EVSpace_Vector_idiv;
	EVSpace_API[EVSpace_Vector_eq_NUM]		= (void*)EVSpace_Vector_eq;
	EVSpace_API[EVSpace_Vector_ne_NUM]		= (void*)EVSpace_Vector_ne;
	EVSpace_API[EVSpace_Vector_dot_NUM ]	= (void*)EVSpace_Vector_dot;
	EVSpace_API[EVSpace_Vector_cross_NUM]	= (void*)EVSpace_Vector_cross;
	EVSpace_API[EVSpace_Vector_mag_NUM]		= (void*)EVSpace_vector_mag;
	EVSpace_API[EVSpace_Vector_mag2_NUM]	= (void*)EVSpace_Vector_mag2;
	EVSpace_API[EVSpace_Vector_norm_NUM]	= (void*)EVSpace_Vector_norm;
	EVSpace_API[EVSpace_Vector_inorm_NUM]	= (void*)EVSpace_Vector_inorm;
	EVSpace_API[EVSpace_Vector_vang_NUM]	= (void*)EVSpace_Vector_vang;
	EVSpace_API[EVSpace_Vector_vxcl_NUM]	= (void*)EVSpace_Vector_vxcl;
	// EMatrix
	EVSpace_API[EVSpace_Matrix_add_NUM]		= (void*)EVSpace_Matrix_add;
	EVSpace_API[EVSpace_Matrix_sub_NUM]		= (void*)EVSpace_Matrix_sub;
	EVSpace_API[EVSpace_Matrix_multm_NUM]	= (void*)EVSpace_Matrix_multm;
	EVSpace_API[EVSpace_Matrix_multv_NUM]	= (void*)EVSpace_Matrix_multv;
	EVSpace_API[EVSpace_Matrix_multd_NUM]	= (void*)EVSpace_Matrix_multd;
	EVSpace_API[EVSpace_Matrix_neg_NUM]		= (void*)EVSpace_Matrix_neg;
	EVSpace_API[EVSpace_Matrix_iadd_NUM]	= (void*)EVSpace_Matrix_iadd;
	EVSpace_API[EVSpace_Matrix_isub_NUM]	= (void*)EVSpace_Matrix_isub;
	EVSpace_API[EVSpace_Matrix_imultm_NUM]	= (void*)EVSpace_Matrix_imultm;
	EVSpace_API[EVSpace_Matrix_imultd_NUM]	= (void*)EVSpace_Matrix_imultd;
	EVSpace_API[EVSpace_Matrix_div_NUM]		= (void*)EVSpace_Matrix_div;
	EVSpace_API[EVSpace_Matrix_idiv_NUM]	= (void*)EVSpace_Matrix_idiv;
	EVSpace_API[EVSpace_Matrix_eq_NUM]		= (void*)EVSpace_Matrix_eq;
	EVSpace_API[EVSpace_Matrix_ne_NUM]		= (void*)EVSpace_Matrix_ne;
	EVSpace_API[EVSpace_Matrix_det_NUM]		= (void*)EVSpace_Matrix_det;
	EVSpace_API[EVSpace_Matrix_trans_NUM]	= (void*)EVSpace_Matrix_trans;

	// create capsule
	c_api_object = PyCapsule_New((void*)EVSpace_API, "evspace._C_API", NULL);
	if (!c_api_object)
		goto error;

	// add capsule to module
	if (PyModule_AddObject(m, "_C_API", c_api_object) < 0)
		goto error;

	// add EVector to module
	Py_INCREF(&EVectorType);
	if (PyModule_AddObject(m, "EVector", (PyObject*)&EVectorType) < 0)
		goto error;

	// add EMatrix to module
	Py_INCREF(&EMatrixType);
	if (PyModule_AddObject(m, "EMatrix", (PyObject*)&EMatrixType) < 0)
		goto error;
		
	return m;

error:

	Py_DECREF(m);
	Py_XDECREF(c_api_object);
	Py_DECREF(&EVectorType);
	Py_DECREF(&EMatrixType);

	return NULL;
}

#endif