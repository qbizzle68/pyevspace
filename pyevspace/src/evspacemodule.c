#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct {
	PyObject_HEAD
	double m_arr[3];
} EVector;

#define EVSPACE_MODULE
#include <evspacemodule.h>

#define EVSpace_RADIANS_TO_DEGREES 180.0 / 3.14159265358979323846

/*	implimentation of C API methods  */

static void EVSpace_Add(EVector* rtn, const EVector* lhs, const EVector* rhs)
{
	rtn->m_arr[0] = lhs->m_arr[0] + rhs->m_arr[0];
	rtn->m_arr[1] = lhs->m_arr[1] + rhs->m_arr[1];
	rtn->m_arr[2] = lhs->m_arr[2] + rhs->m_arr[2];
}

static void EVSpace_Sub(EVector* rtn, const EVector* lhs, const EVector* rhs)
{
	rtn->m_arr[0] = lhs->m_arr[0] - rhs->m_arr[0];
	rtn->m_arr[1] = lhs->m_arr[1] - rhs->m_arr[1];
	rtn->m_arr[2] = lhs->m_arr[2] - rhs->m_arr[2];
}

static void EVSpace_Mult(EVector* rtn, const EVector* lhs, double rhs)
{
	rtn->m_arr[0] = lhs->m_arr[0] * rhs;
	rtn->m_arr[1] = lhs->m_arr[1] * rhs;
	rtn->m_arr[2] = lhs->m_arr[2] * rhs;
}

static void EVSpace_Neg(EVector* rtn, const EVector* lhs)
{
	rtn->m_arr[0] = -lhs->m_arr[0];
	rtn->m_arr[1] = -lhs->m_arr[1];
	rtn->m_arr[2] = -lhs->m_arr[2];
}

static double EVSpace_Abs(const EVector* vec)
{
	double mag2 = vec->m_arr[0] * vec->m_arr[0]
		+ vec->m_arr[1] * vec->m_arr[1]
		+ vec->m_arr[2] * vec->m_arr[2];

	return sqrt(mag2);
}

static void EVSpace_Iadd(EVector* lhs, const EVector* rhs)
{
	lhs->m_arr[0] += rhs->m_arr[0];
	lhs->m_arr[1] += rhs->m_arr[1];
	lhs->m_arr[2] += rhs->m_arr[2];
}

static void EVSpace_Isub(EVector* lhs, const EVector* rhs)
{
	lhs->m_arr[0] -= rhs->m_arr[0];
	lhs->m_arr[1] -= rhs->m_arr[1];
	lhs->m_arr[2] -= rhs->m_arr[2];
}

static void EVSpace_Imult(EVector* lhs, double rhs)
{
	lhs->m_arr[0] *= rhs;
	lhs->m_arr[1] *= rhs;
	lhs->m_arr[2] *= rhs;
}

static void EVSpace_Div(EVector* rtn, const EVector* lhs, double rhs)
{
	rtn->m_arr[0] = lhs->m_arr[0] / rhs;
	rtn->m_arr[1] = lhs->m_arr[1] / rhs;
	rtn->m_arr[2] = lhs->m_arr[2] / rhs;
}

static void EVSpace_Idiv(EVector* lhs, const EVector* rhs)
{
	lhs->m_arr[0] /= rhs->m_arr[0];
	lhs->m_arr[1] /= rhs->m_arr[1];
	lhs->m_arr[2] /= rhs->m_arr[2];
}

static bool EVSpace_ET(const EVector* lhs, const EVector* rhs)
{
	return (lhs->m_arr[0] == rhs->m_arr[0])
		&& (lhs->m_arr[1] == rhs->m_arr[1])
		&& (lhs->m_arr[2] == rhs->m_arr[2]);
}

static bool EVSpace_NE(const EVector* lhs, const EVector* rhs)
{
	return (lhs->m_arr[0] == rhs->m_arr[0])
		&& (lhs->m_arr[1] == rhs->m_arr[1])
		&& (lhs->m_arr[2] == rhs->m_arr[2]);
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
	double theta = acos(dot(lhs, rhs) / (EVSpace_Mag(lhs) * EVSpace_Mag(rhs)));
	return theta * EVSpace_RADIANS_TO_DEGREES;
}

static void EVSpace_Vxcl(EVector* ans, const EVector* vec, const EVector* xcl)
{
	double scale = EVSpace_Dot(vec, xcl) / EVSpace_Mag2(xcl);
	ans->m_arr[0] = vec->m_arr[0] - (xcl->m_arr[0] * scale);
	ans->m_arr[1] = vec->m_arr[1] - (xcl->m_arr[1] * scale);
	ans->m_arr[2] = vec->m_arr[2] - (xcl->m_arr[2] * scale);
}

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

static PyObject* Operator_Add(EVector* self, PyObject* arg)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_INCREF(rtn);
	Py_DECREF(type);

	if (!rtn) {
		Py_DECREF(rtn);
		return NULL;
	}

	EVector_Add(rtn, self, (EVector*)arg);

	return (PyObject*)rtn;
}

static PyObject* Operator_Sub(EVector* self, PyObject* arg)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_INCREF(rtn);
	Py_DECREF(type);

	if (!rtn) {
		Py_DECREF(rtn);
		return NULL;
	}

	EVector_Sub(rtn, self, (EVector*)arg);

	return (PyObject*)rtn;
}

static PyObject* Operator_Mult(EVector* self, PyObject* arg)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_INCREF(rtn);
	Py_DECREF(type);

	if (!rtn) {
		Py_DECREF(rtn);
		return NULL;
	}

	double rhs;
	if (!PyArg_ParseTuple(arg, "d", &rhs))
		return NULL;

	EVector_Mult(rtn, self, rhs);

	return (PyObject*)rtn;
}

static PyObject* Operator_Neg(EVector* self)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_INCREF(rtn);
	Py_DECREF(type);

	if (!rtn) {
		Py_DECREF(rtn);
		return NULL;
	}

	EVector_Neg(rtn, self);

	return (PyObject*)rtn;
}

static PyObject* Operator_Abs(EVector* self)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_INCREF(rtn);
	Py_DECREF(type);

	if (!rtn) {
		Py_DECREF(rtn);
		return NULL;
	}

	EVector_Abs(self);

	return (PyObject*)rtn;
}

static PyObject* Operator_Iadd(EVector* self, PyObject* arg)
{
	EVector_Iadd(self, (EVector*)arg);

	return (PyObject*)self;
}

static PyObject* Operator_Isub(EVector* self, PyObject* arg)
{
	EVector_Isub(self, (EVector*)arg);

	return (PyObject*)self;
}

static PyObject* Operator_Imult(EVector* self, PyObject* arg)
{
	double rhs;
	if (!PyArg_ParseTuple(arg, "d", &rhs))
		return NULL;
		
	EVector_Imult(self, rhs);

	return (PyObject*)self;
}

static PyObject* Operator_Div(EVector* self, PyObject* arg)
{
	PyTypeObject* type = self->ob_base.ob_type;
	Py_INCREF(type);
	EVector* rtn = (EVector*)type->tp_new(type, NULL, NULL);
	Py_INCREF(rtn);
	Py_DECREF(type);

	if (!rtn) {
		Py_DECREF(rtn);
		return NULL;
	}

	double rhs;
	if (!PyArg_ParseTuple(arg, "d", &rhs))
		return NULL;

	EVector_Div(rtn, self, rhs);

	return (PyObject*)rtn;
}

static PyObject* Operator_Idiv(EVector* self, PyObject* arg)
{
	double rhs;
	if (!PyArg_ParseTuple(arg, "d", &rhs))
		return NULL;

	EVector_Idiv(self, rhs);

	return (PyObject*)self;
}

static PyNumberMethods EVector_NBMethods =
{
	Operator_Add,	/*  nb_add  */
	Operator_Sub,	/*  nb_sub  */
	Operator_Mult,	/*  nb_mult  */
	0,				/*  nb_remainder  */
	0,				/*  nb_divmod  */
	0,				/*  nb_power  */
	Operator_Neg,	/*  nb_negative  */
	0,				/*  nb_positive  */
	Operator_Abs	/*  nb_absolute  */
	0,				/*  nb_bool  */
	0,				/*  nb_invert  */
	0,				/*  nb_lshift  */
	0,				/*  nb_rshift  */
	0,				/*  nb_and  */
	0,				/*  nb_xor  */
	0,				/*  nb_or  */
	0,				/*  nb_int  */
	0,				/*  nb_reserved  */
	0,				/*  nb_float  */
	Operator_Iadd,	/*  nb_inplace_add  */
	Operator_Isub,	/*  nb_inplace_subtract  */
	Operator_Imult,	/*  nb_inplace_multiply  */
	0,				/*  nb_inplace_remainder  */
	0,				/*  nb_inplace_power  */
	0,				/*  nb_inplace_lshift  */
	0,				/*  nb_inplace_rshift  */
	0,				/*  nb_inplace_and  */
	0,				/*  nb_inplace_xor  */
	0,				/*  nb_inplace_or  */
	0,				/*  nb_floor_divide  */
	Operator_Div,	/*  nb_true_divide  */
	0,				/*  nb_inplace_floor_divide  */
	Operator_Idiv,	/*  nb_inplace_true_divide  */
	0,				/*  nb_index  */
	0,				/*  nb_matrix_multiply  */
	0				/*  nb_inplace_matrix_multiply  */
};
