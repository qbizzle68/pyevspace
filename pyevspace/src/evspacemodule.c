#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

typedef struct {
	PyObject_HEAD
	double m_arr[3];
} EVector;

#define EVSPACE_MODULE
#include <evspacemodule.h>

#define EVSpace_RADIANS_TO_DEGREES 180.0 / 3.14159265358979323846

/*	implimentation of C API methods  */

static void EVSpace_Vadd(EVector* rtn, const EVector* lhs, const EVector* rhs)
{
	rtn->m_arr[0] = lhs->m_arr[0] + rhs->m_arr[0];
	rtn->m_arr[1] = lhs->m_arr[1] + rhs->m_arr[1];
	rtn->m_arr[2] = lhs->m_arr[2] + rhs->m_arr[2];
}

static void EVSpace_Vsub(EVector* rtn, const EVector* lhs, const EVector* rhs)
{
	rtn->m_arr[0] = lhs->m_arr[0] - rhs->m_arr[0];
	rtn->m_arr[1] = lhs->m_arr[1] - rhs->m_arr[1];
	rtn->m_arr[2] = lhs->m_arr[2] - rhs->m_arr[2];
}

static void EVSpace_Vmult(EVector* rtn, const EVector* lhs, double rhs)
{
	rtn->m_arr[0] = lhs->m_arr[0] * rhs;
	rtn->m_arr[1] = lhs->m_arr[1] * rhs;
	rtn->m_arr[2] = lhs->m_arr[2] * rhs;
}

static void EVSpace_Vneg(EVector* rtn, const EVector* lhs)
{
	rtn->m_arr[0] = -lhs->m_arr[0];
	rtn->m_arr[1] = -lhs->m_arr[1];
	rtn->m_arr[2] = -lhs->m_arr[2];
}

static double EVSpace_Vabs(const EVector* vec)
{
	double mag2 = vec->m_arr[0] * vec->m_arr[0]
		+ vec->m_arr[1] * vec->m_arr[1]
		+ vec->m_arr[2] * vec->m_arr[2];

	return sqrt(mag2);
}

static void EVSpace_Viadd(EVector* lhs, const EVector* rhs)
{
	lhs->m_arr[0] += rhs->m_arr[0];
	lhs->m_arr[1] += rhs->m_arr[1];
	lhs->m_arr[2] += rhs->m_arr[2];
}

static void EVSpace_Visub(EVector* lhs, const EVector* rhs)
{
	lhs->m_arr[0] -= rhs->m_arr[0];
	lhs->m_arr[1] -= rhs->m_arr[1];
	lhs->m_arr[2] -= rhs->m_arr[2];
}

static void EVSpace_Vimult(EVector* lhs, double rhs)
{
	lhs->m_arr[0] *= rhs;
	lhs->m_arr[1] *= rhs;
	lhs->m_arr[2] *= rhs;
}

static void EVSpace_Vdiv(EVector* rtn, const EVector* lhs, double rhs)
{
	rtn->m_arr[0] = lhs->m_arr[0] / rhs;
	rtn->m_arr[1] = lhs->m_arr[1] / rhs;
	rtn->m_arr[2] = lhs->m_arr[2] / rhs;
}

static void EVSpace_Vidiv(EVector* lhs, double rhs)
{
	lhs->m_arr[0] /= rhs;
	lhs->m_arr[1] /= rhs;
	lhs->m_arr[2] /= rhs;
}

static bool EVSpace_Veq(const EVector* lhs, const EVector* rhs)
{
	return (lhs->m_arr[0] == rhs->m_arr[0])
		&& (lhs->m_arr[1] == rhs->m_arr[1])
		&& (lhs->m_arr[2] == rhs->m_arr[2]);
}

static bool EVSpace_Vne(const EVector* lhs, const EVector* rhs)
{
	return !(EVSpace_Veq(lhs, rhs));
}

static double EVSpace_Dot(const EVector* lhs, const EVector* rhs)
{
	return (lhs->m_arr[0] * rhs->m_arr[0])
		+ (lhs->m_arr[1] * rhs->m_arr[1])
		+ (lhs->m_arr[2] * rhs->m_arr[2]);
}

static void EVSpace_Cross(EVector* rtn, const EVector* lhs, const EVector* rhs)
{
	rtn->m_arr[0] = ((lhs->m_arr[1] * rhs->m_arr[2]) - (lhs->m_arr[2] * rhs->m_arr[1]));
	rtn->m_arr[1] = ((lhs->m_arr[0] * rhs->m_arr[2]) - (lhs->m_arr[2] * rhs->m_arr[0]));
	rtn->m_arr[2] = ((lhs->m_arr[0] * rhs->m_arr[1]) - (lhs->m_arr[1] * rhs->m_arr[0]));
}

static double EVSpace_Mag(const EVector* vec)
{
	return sqrt(EVSpace_Dot(vec, vec));
}

static double EVSpace_Mag2(const EVector* vec)
{
	return EVSpace_Dot(vec, vec);
}

static void EVSpace_Norm(EVector* rtn, const EVector* vec)
{
	double mag = sqrt(EVSpace_Dot(vec, vec));
	rtn->m_arr[0] = vec->m_arr[0] / mag;
	rtn->m_arr[1] = vec->m_arr[1] / mag;
	rtn->m_arr[2] = vec->m_arr[2] / mag;
}

static void EVSpace_Inorm(EVector* rtn)
{
	double mag = sqrt(EVSpace_Dot(rtn, rtn));
	rtn->m_arr[0] /= mag;
	rtn->m_arr[1] /= mag;
	rtn->m_arr[2] /= mag;
}

static double EVSpace_Vang(const EVector* lhs, const EVector* rhs)
{
	double theta = acos(EVSpace_Dot(lhs, rhs) / (EVSpace_Mag(lhs) * EVSpace_Mag(rhs)));
	return theta * EVSpace_RADIANS_TO_DEGREES;
}

static void EVSpace_Vxcl(EVector* ans, const EVector* vec, const EVector* xcl)
{
	double scale = EVSpace_Dot(vec, xcl) / EVSpace_Mag2(xcl);
	ans->m_arr[0] = vec->m_arr[0] - (xcl->m_arr[0] * scale);
	ans->m_arr[1] = vec->m_arr[1] - (xcl->m_arr[1] * scale);
	ans->m_arr[2] = vec->m_arr[2] - (xcl->m_arr[2] * scale);
}

/*	Implimenting C API into Python number metehods  */
static PyObject* VOperator_Add(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn)
		return NULL;

	Py_INCREF(rtn);
	EVSpace_Vadd(rtn, self, (EVector*)arg);

	return (PyObject*)rtn;
}

static PyObject* VOperator_Sub(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn) 
		return NULL;

	Py_INCREF(rtn);
	EVSpace_Vsub(rtn, self, (EVector*)arg);

	return (PyObject*)rtn;
}

static PyObject* VOperator_Mult(EVector* self, PyObject* arg)
{
	double rhs = PyFloat_AsDouble(arg);
	if (PyErr_Occurred() != NULL)
		return NULL;

	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn) 
		return NULL;

	Py_INCREF(rtn);
	EVSpace_Vmult(rtn, self, rhs);

	return (PyObject*)rtn;
}

static PyObject* VOperator_Neg(EVector* self)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn) 
		return NULL;

	Py_INCREF(rtn);
	EVSpace_Vneg(rtn, self);

	return (PyObject*)rtn;
}

static PyObject* VOperator_Abs(EVector* self)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn)
		return NULL;

	Py_INCREF(rtn);
	EVSpace_Vabs(self);

	return (PyObject*)rtn;
}

static PyObject* VOperator_Iadd(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}
	EVSpace_Viadd(self, (EVector*)arg);

	Py_INCREF(self);
	return (PyObject*)self;
}

static PyObject* VOperator_Isub(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}
	EVSpace_Visub(self, (EVector*)arg);

	Py_INCREF(self);
	return (PyObject*)self;
}

static PyObject* VOperator_Imult(EVector* self, PyObject* arg)
{
	double rhs = PyFloat_AsDouble(arg);
	if (PyErr_Occurred() != NULL)
		return NULL;
		
	EVSpace_Vimult(self, rhs);

	Py_INCREF(self);
	return (PyObject*)self;
}

static PyObject* VOperator_Div(EVector* self, PyObject* arg)
{
	double rhs = PyFloat_AsDouble(arg);
	if (PyErr_Occurred() != NULL)
		return NULL;

	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn)
		return NULL;

	EVSpace_Vdiv(rtn, self, rhs);

	Py_INCREF(rtn);
	return (PyObject*)rtn;
}

static PyObject* VOperator_Idiv(EVector* self, PyObject* arg)
{
	double rhs = PyFloat_AsDouble(arg);
	if (PyErr_Occurred() != NULL)
		return NULL;

	EVSpace_Vidiv(self, rhs);

	Py_INCREF(self);
	return (PyObject*)self;
}

static PyNumberMethods EVector_NBMethods = {
	.nb_add = VOperator_Add,
	.nb_subtract = VOperator_Sub,
	.nb_multiply = VOperator_Mult,
	.nb_negative = VOperator_Neg,
	.nb_absolute = VOperator_Abs,
	.nb_inplace_add = VOperator_Iadd,
	.nb_inplace_subtract = VOperator_Isub,
	.nb_inplace_multiply = VOperator_Imult,
	.nb_true_divide = VOperator_Div,
	.nb_inplace_true_divide = VOperator_Idiv,
};

//static PyNumberMethods EVector_NBMethods =
//{
//	VOperator_Add,	/*  nb_add  */
//	VOperator_Sub,	/*  nb_sub  */
//	VOperator_Mult,	/*  nb_mult  */
//	0,				/*  nb_remainder  */
//	0,				/*  nb_divmod  */
//	0,				/*  nb_power  */
//	VOperator_Neg,	/*  nb_negative  */
//	0,				/*  nb_positive  */
//	VOperator_Abs,	/*  nb_absolute  */
//	0,				/*  nb_bool  */
//	0,				/*  nb_invert  */
//	0,				/*  nb_lshift  */
//	0,				/*  nb_rshift  */
//	0,				/*  nb_and  */
//	0,				/*  nb_xor  */
//	0,				/*  nb_or  */
//	0,				/*  nb_int  */
//	0,				/*  nb_reserved  */
//	0,				/*  nb_float  */
//	VOperator_Iadd,	/*  nb_inplace_add  */
//	VOperator_Isub,	/*  nb_inplace_subtract  */
//	VOperator_Imult,/*  nb_inplace_multiply  */
//	0,				/*  nb_inplace_remainder  */
//	0,				/*  nb_inplace_power  */
//	0,				/*  nb_inplace_lshift  */
//	0,				/*  nb_inplace_rshift  */
//	0,				/*  nb_inplace_and  */
//	0,				/*  nb_inplace_xor  */
//	0,				/*  nb_inplace_or  */
//	0,				/*  nb_floor_divide  */
//	VOperator_Div,	/*  nb_true_divide  */
//	0,				/*  nb_inplace_floor_divide  */
//	VOperator_Idiv,	/*  nb_inplace_true_divide  */
//	0,				/*  nb_index  */
//	0,				/*  nb_matrix_multiply  */
//	0				/*  nb_inplace_matrix_multiply  */
//};

/*	Class Methods  */
static PyObject* EVector_Dot(EVector* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Dot(self, (EVector*)args));
}

static PyObject* EVector_Cross(EVector* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn)
		return NULL;

	Py_INCREF(rtn);
	EVSpace_Cross(rtn, self, (EVector*)args);
	return (PyObject*)rtn;
}

static PyObject* EVector_Mag(EVector* self, PyObject* UNUSED)
{
	return PyFloat_FromDouble(EVSpace_Mag(self));
}

static PyObject* EVector_Mag2(EVector* self, PyObject* UNUSED)
{
	return PyFloat_FromDouble(EVSpace_Mag2(self));
}

static PyObject* EVector_Norm(EVector* self, PyObject* UNUSED)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn)
		return NULL;

	Py_INCREF(rtn);
	EVSpace_Norm(rtn, self);
	return (PyObject*)rtn;
}

static PyObject* EVector_Normalize(EVector* self, PyObject* UNUSED)
{
	EVSpace_Inorm(self);
	return (PyObject*)self;
}

static PyObject* EVector_Vang(EVector* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Vang(self, (EVector*)args));
}

static PyObject* EVector_Vxcl(EVector* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_DECREF(type);

	if (!rtn)
		return NULL;

	Py_INCREF(rtn);
	EVSpace_Vxcl(rtn, self, (EVector*)args);

	return (PyObject*)rtn;
}

/*	Type Methods  */
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

	if (ok < 0 || ok > 50)
		return NULL; // todo: do we raise an exception here?

	return Py_BuildValue("s#", buffer, strlen(buffer));
}

static PyObject* EVector_richcompare(PyObject* self, PyObject* other, int op)
{
	if (PyObject_TypeCheck(other, self->ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	if (op == Py_EQ) return PyBool_FromLong(EVSpace_Veq((EVector*)self, (EVector*)other));
	else if (op == Py_NE) return PyBool_FromLong(EVSpace_Vne((EVector*)self, (EVector*)other));
	else return Py_NotImplemented;
}

static PyMemberDef EVector_Members[] = {
	{"x", T_DOUBLE, offsetof(EVector, m_arr), 0, "x-component"},
	{"y", T_DOUBLE, offsetof(EVector, m_arr) + sizeof(double), 0, "y-component"},
	{"z", T_DOUBLE, offsetof(EVector, m_arr) + (2 * sizeof(double)), 0, "z-component"},
	{NULL}
};

static PyMethodDef EVector_Methods[] = {
	{"dot", (PyCFunction)EVector_Dot, METH_O, "Return the dot product of two EVectors."},
	{"cross", (PyCFunction)EVector_Cross, METH_O, "Return the cross product of two EVectors."},
	{"mag", (PyCFunction)EVector_Mag, METH_NOARGS, "Returns the magnitude of an EVector."},
	{"mag2", (PyCFunction)EVector_Mag2, METH_NOARGS, "Returns the square of the magnitude of an EVector."},
	{"norm", (PyCFunction)EVector_Norm, METH_NOARGS, "Returns a normalized version of an EVector."},
	{"normalize", (PyCFunction)EVector_Normalize, METH_NOARGS, "Normalized an EVector."},
	{"vang", (PyCFunction)EVector_Vang, METH_O, "Return the shortest angle between two EVector's."},
	{"vxcl", (PyCFunction)EVector_Vxcl, METH_O, "Returns a vector exculded from another."},
	{NULL}
};

static PyTypeObject EVectorType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name		= "pyevspace.EVector",
	.tp_doc			= PyDoc_STR("Eulcidean Vector"),
	.tp_basicsize	= sizeof(EVector),
	.tp_itemsize	= 0,
	.tp_flags		= Py_TPFLAGS_DEFAULT,
	.tp_new			= PyType_GenericNew,
	.tp_init		= (initproc)EVector_init,
	.tp_members		= EVector_Members,
	.tp_methods		= EVector_Methods,
	.tp_str			= (reprfunc)EVector_str,
	.tp_as_number	= &EVector_NBMethods,
	.tp_richcompare	= (richcmpfunc)EVector_richcompare,
};

static PyModuleDef EVSpacemodule = {
	PyModuleDef_HEAD_INIT,
	.m_name = "pyevspace",
	.m_doc = "Module library for a Euclidean vector space with vector and matrix types.",
	.m_size = -1,
};

PyMODINIT_FUNC
PyInit_pyevspace(void)
{
	PyObject* m;
	static void* EVSpace_API[EVSpace_API_pointers];
	PyObject* c_api_object;

	if (PyType_Ready(&EVectorType) < 0)
		return NULL;

	m = PyModule_Create(&EVSpacemodule);
	if (m == NULL)
		return NULL;

	EVSpace_API[EVSpace_Vadd_NUM] = (void*)EVSpace_Vadd;
	EVSpace_API[EVSpace_Vsub_NUM] = (void*)EVSpace_Vsub;
	EVSpace_API[EVSpace_Vmult_NUM] = (void*)EVSpace_Vmult;
	EVSpace_API[EVSpace_Vneg_NUM] = (void*)EVSpace_Vneg;
	EVSpace_API[EVSpace_Vabs_NUM] = (void*)EVSpace_Vabs;
	EVSpace_API[EVSpace_Viadd_NUM] = (void*)EVSpace_Viadd;
	EVSpace_API[EVSpace_Visub_NUM] = (void*)EVSpace_Visub;
	EVSpace_API[EVSpace_Vimult_NUM] = (void*)EVSpace_Vimult;
	EVSpace_API[EVSpace_Vdiv_NUM] = (void*)EVSpace_Vdiv;
	EVSpace_API[EVSpace_Vidiv_NUM] = (void*)EVSpace_Vidiv;
	EVSpace_API[EVSpace_Veq_NUM] = (void*)EVSpace_Veq;
	EVSpace_API[EVSpace_Vne_NUM] = (void*)EVSpace_Vne;
	EVSpace_API[EVSpace_Dot_NUM ] = (void*)EVSpace_Dot;
	EVSpace_API[EVSpace_Cross_NUM] = (void*)EVSpace_Cross;
	EVSpace_API[EVSpace_Mag_NUM] = (void*)EVSpace_Mag;
	EVSpace_API[EVSpace_Mag2_NUM] = (void*)EVSpace_Mag2;
	EVSpace_API[EVSpace_Norm_NUM] = (void*)EVSpace_Norm;
	EVSpace_API[EVSpace_Inorm_NUM] = (void*)EVSpace_Inorm;
	EVSpace_API[EVSpace_Vang_NUM] = (void*)EVSpace_Vang;
	EVSpace_API[EVSpace_Vxcl_NUM] = (void*)EVSpace_Vxcl;

	c_api_object = PyCapsule_New((void*)EVSpace_API, "evspace._C_API", NULL);

	if (PyModule_AddObject(m, "_C_API", c_api_object) < 0) {
		Py_XDECREF(c_api_object);
		Py_DECREF(m);
		return NULL;
	}

	Py_INCREF(&EVectorType);
	if (PyModule_AddObject(m, "EVector", (PyObject*)&EVectorType) < 0) {
		Py_DECREF(&EVectorType);
		Py_DECREF(m);
		return NULL;
	}

	return m;
}
