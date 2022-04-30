#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

typedef struct {
	PyObject_HEAD
	double m_arr[3];
} EVector;

typedef struct {
	PyObject_HEAD
	double m_arr[3][3];
} EMatrix;

#define EVSPACE_MODULE
#include <evspacemodule.h>

#define EVSpace_RADIANS_TO_DEGREES 180.0 / 3.14159265358979323846

/*************************************/
/*	implimentation of C API methods  */
/*************************************/

/************** EVector **************/

static EVector* EVSpace_Vadd(const EVector* lhs, const EVector* rhs)
{
	EVector* rtn = (EVector*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	rtn->m_arr[0] = lhs->m_arr[0] + rhs->m_arr[0];
	rtn->m_arr[1] = lhs->m_arr[1] + rhs->m_arr[1];
	rtn->m_arr[2] = lhs->m_arr[2] + rhs->m_arr[2];
	return rtn;
}

static EVector* EVSpace_Vsub(const EVector* lhs, const EVector* rhs)
{
	EVector* rtn = (EVector*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	rtn->m_arr[0] = lhs->m_arr[0] - rhs->m_arr[0];
	rtn->m_arr[1] = lhs->m_arr[1] - rhs->m_arr[1];
	rtn->m_arr[2] = lhs->m_arr[2] - rhs->m_arr[2];
	return rtn;
}

static EVector* EVSpace_Vmult(const EVector* lhs, double rhs)
{
	EVector* rtn = (EVector*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	rtn->m_arr[0] = lhs->m_arr[0] * rhs;
	rtn->m_arr[1] = lhs->m_arr[1] * rhs;
	rtn->m_arr[2] = lhs->m_arr[2] * rhs;
	return rtn;
}

static EVector* EVSpace_Vneg(const EVector* lhs)
{
	EVector* rtn = (EVector*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	rtn->m_arr[0] = -lhs->m_arr[0];
	rtn->m_arr[1] = -lhs->m_arr[1];
	rtn->m_arr[2] = -lhs->m_arr[2];
	return rtn;
}

static double EVSpace_Vabs(const EVector* vec)
{
	return sqrt(vec->m_arr[0] * vec->m_arr[0]
		+ vec->m_arr[1] * vec->m_arr[1]
		+ vec->m_arr[2] * vec->m_arr[2]);
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

static EVector* EVSpace_Vdiv(const EVector* lhs, double rhs)
{
	EVector* rtn = (EVector*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	rtn->m_arr[0] = lhs->m_arr[0] / rhs;
	rtn->m_arr[1] = lhs->m_arr[1] / rhs;
	rtn->m_arr[2] = lhs->m_arr[2] / rhs;

	return rtn;
}

static void EVSpace_Vidiv(EVector* lhs, double rhs)
{
	lhs->m_arr[0] /= rhs;
	lhs->m_arr[1] /= rhs;
	lhs->m_arr[2] /= rhs;
}

static int EVSpace_Veq(const EVector* lhs, const EVector* rhs)
{
	static double epsilon = 1e-6;
	if ((fabs(lhs->m_arr[0] - rhs->m_arr[0]) < epsilon) 
		&& (fabs(lhs->m_arr[1] - rhs->m_arr[1]) < epsilon) 
		&& (fabs(lhs->m_arr[2] - rhs->m_arr[2]) < epsilon))
		return 1;
	return 0;
}

static int EVSpace_Vne(const EVector* lhs, const EVector* rhs)
{
	if (EVSpace_Veq(lhs, rhs))
		return 0;
	return 1;
}

static double EVSpace_Dot(const EVector* lhs, const EVector* rhs)
{
	return (lhs->m_arr[0] * rhs->m_arr[0])
		+ (lhs->m_arr[1] * rhs->m_arr[1])
		+ (lhs->m_arr[2] * rhs->m_arr[2]);
}

static EVector* EVSpace_Cross(const EVector* lhs, const EVector* rhs)
{
	EVector* rtn = (EVector*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	rtn->m_arr[0] = ((lhs->m_arr[1] * rhs->m_arr[2]) - (lhs->m_arr[2] * rhs->m_arr[1]));
	rtn->m_arr[1] = ((lhs->m_arr[2] * rhs->m_arr[0]) - (lhs->m_arr[0] * rhs->m_arr[2]));
	rtn->m_arr[2] = ((lhs->m_arr[0] * rhs->m_arr[1]) - (lhs->m_arr[1] * rhs->m_arr[0]));

	return rtn;
}

static double EVSpace_Mag(const EVector* vec)
{
	return sqrt(EVSpace_Dot(vec, vec));
}

static double EVSpace_Mag2(const EVector* vec)
{
	return EVSpace_Dot(vec, vec);
}

static EVector* EVSpace_Norm(const EVector* vec)
{
	return EVSpace_Vdiv(vec, sqrt(EVSpace_Dot(vec, vec)));
}

static void EVSpace_Inorm(EVector* rtn)
{
	EVSpace_Vidiv(rtn, sqrt(EVSpace_Dot(rtn, rtn)));
}

static double EVSpace_Vang(const EVector* lhs, const EVector* rhs)
{
	double theta = acos(EVSpace_Dot(lhs, rhs) / (EVSpace_Mag(lhs) * EVSpace_Mag(rhs)));
	return theta * EVSpace_RADIANS_TO_DEGREES;
}

static EVector* EVSpace_Vxcl(const EVector* vec, const EVector* xcl)
{
	EVector* rtn = (EVector*)vec->ob_base.ob_type->tp_new(vec->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	double scale = EVSpace_Dot(vec, xcl) / EVSpace_Mag2(xcl);
	rtn->m_arr[0] = vec->m_arr[0] - (xcl->m_arr[0] * scale);
	rtn->m_arr[1] = vec->m_arr[1] - (xcl->m_arr[1] * scale);
	rtn->m_arr[2] = vec->m_arr[2] - (xcl->m_arr[2] * scale);

	return rtn;
}

/*********************************************/
/***************** EMatrix *******************/
/*********************************************/

static void EVSpace_Madd(EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			ans->m_arr[i][j] = lhs->m_arr[i][j] + rhs->m_arr[i][j];
	}
}

static void EVSpace_Msub(EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			ans->m_arr[i][j] = lhs->m_arr[i][j] - rhs->m_arr[i][j];
	}
}

static void EVSpace_Mmultm(EMatrix* ans, const EMatrix* lhs, const EMatrix* rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			double sum = 0;
			for (int k = 0; k < 3; k++)
				sum += lhs->m_arr[i][k] * rhs->m_arr[k][j];
			ans->m_arr[i][j] = sum;
		}
	}
}

static void EVSpace_Mmultv(EVector* ans, const EMatrix* lhs, const EVector* rhs)
{
	for (int i = 0; i < 3; i++) {
		double sum = 0;
		for (int j = 0; j < 3; j++)
			sum += lhs->m_arr[i][j] * rhs->m_arr[j];
		ans->m_arr[i] = sum;
	}
}

static void EVSpace_Mmultd(EMatrix* ans, const EMatrix* lhs, double rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			ans->m_arr[i][j] = lhs->m_arr[i][j] * rhs;
	}
}

static void EVSpace_Mneg(EMatrix* ans, const EMatrix* vec)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			ans->m_arr[i][j] = -vec->m_arr[i][j];
	}
}

static void EVSpace_Miadd(EMatrix* lhs, const EMatrix* rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] += rhs->m_arr[i][j];
	}
}

static void EVSpace_Misub(EMatrix* lhs, const EMatrix* rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] -= rhs->m_arr[i][j];
	}
}

static void EVSpace_Mimultm(EMatrix* lhs, const EMatrix* rhs)
{
	// todo: return the pointer so this is faster
	EMatrix* tmp = (EMatrix*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!tmp) {
		// todo: is lhs guaranteed to not be NULL?
		Py_XDECREF(lhs);
		lhs = NULL;
		return;
	}

	Py_INCREF(tmp);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			tmp->m_arr[i][j] = lhs->m_arr[i][j];
	}

	EVSpace_Mmultm(lhs, tmp, rhs);
	// todo: can i just delete tmp so we dont have to keep track of references?
	Py_DECREF(tmp);
}

static void EVSpace_Mimultd(EMatrix* lhs, double rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] *= rhs;
	}
}

static void EVSpace_Mdiv(EMatrix* ans, const EMatrix* lhs, double rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			ans->m_arr[i][j] = lhs->m_arr[i][j] / rhs;
	}
}

static void EVSpace_Midiv(EMatrix* lhs, double rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] /= rhs;
	}
}

static int EVSpace_Meq(const EMatrix* lhs, const EMatrix* rhs)
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

static int EVSpace_Mne(const EMatrix* lhs, const EMatrix* rhs)
{
	if (EVSpace_Meq(lhs, rhs) == 1)
		return 0;
	return 1;
}

static double EVSpace_Det(const EMatrix* lhs)
{
	return lhs->m_arr[0][0] * ((lhs->m_arr[1][1] * lhs->m_arr[2][2]) - (lhs->m_arr[1][2] * lhs->m_arr[2][1]))
		+ lhs->m_arr[0][1] * ((lhs->m_arr[1][2] * lhs->m_arr[2][0]) - (lhs->m_arr[1][0] * lhs->m_arr[2][2]))
		+ lhs->m_arr[0][2] * ((lhs->m_arr[1][0] * lhs->m_arr[2][1]) - (lhs->m_arr[1][1] * lhs->m_arr[2][0]));
}

static void EVSpace_Trans(EMatrix* ans, const EMatrix* mat)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			ans->m_arr[i][j] = mat->m_arr[j][i];
	}
}

static void EVSpace_Mset(EMatrix* self, int i, int j, double val)
{
	self->m_arr[i][j] = val;
}

static double EVSpace_Mget(EMatrix* self, int i, int j)
{
	return self->m_arr[i][j];
}

/****************************************************/
/*	Implimenting C API into Python number metehods  */
/****************************************************/

/****************************************************/
/************** EVector implementations *************/
/****************************************************/

static PyObject* VOperator_Add(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVector* rtn = EVSpace_Vadd(self, (EVector*)arg);
	// no need to error check, if rtn is NULL, we return it anyway
	return (PyObject*)rtn;
}

static PyObject* VOperator_Sub(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVector* rtn = EVSpace_Vsub(self, (EVector*)arg);
	// no need to error check, if rtn is NULL, we return it anyway
	return (PyObject*)(rtn);
}

static PyObject* VOperator_Mult(EVector* self, PyObject* arg)
{
	double rhs;

	if (PyFloat_Check(arg))
		rhs = PyFloat_AS_DOUBLE(arg);
	else if (PyLong_Check(arg)) {
		rhs = PyLong_AsDouble(arg);
		if (rhs < 0)
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	EVector* rtn = EVSpace_Vmult(self, rhs);
	// no need to error check, if rtn is NULL, we return it anyway
	return (PyObject*)(rtn);
}

static PyObject* VOperator_Neg(EVector* self)
{
	EVector* rtn = EVSpace_Vneg(self);
	// no need to error check, if rtn is NULL, we return it anyway
	return (PyObject*)(rtn);
}

static PyObject* VOperator_Abs(EVector* self)
{
	PyObject* rtn = PyFloat_FromDouble(EVSpace_Vabs(self));
	// no need to error check, if rtn is NULL, we return it anyway
	return (PyObject*)(rtn);
}

static PyObject* VOperator_Iadd(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Viadd(self, (EVector*)arg); // no errors to report here
	
	return (PyObject*)(self);
}

/* This is for debugging reference counts */
// todo: delete this when finished 
static PyObject* Ref_check(EVector* UNUSED, EVector* arg)
{
	printf("reference count: %i\n", (int)arg->ob_base.ob_refcnt);
	Py_RETURN_NONE;
}

static PyObject* VOperator_Isub(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Visub(self, (EVector*)arg);

	return (PyObject*)(self);
}

static PyObject* VOperator_Imult(EVector* self, PyObject* arg)
{
	double rhs;
	if (PyFloat_Check(arg))
		rhs = PyFloat_AS_DOUBLE(arg);
	else if (PyLong_Check(arg)) {
		rhs = PyLong_AsDouble(arg);
		if (rhs < 0)
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Vimult(self, rhs);

	return (PyObject*)(self);
}

static PyObject* VOperator_Div(EVector* self, PyObject* arg)
{
	double rhs;
	if (PyFloat_Check(arg))
		rhs = PyFloat_AS_DOUBLE(arg);
	else if (PyLong_Check(arg)) {
		rhs = PyLong_AsDouble(arg);
		if (rhs < 0)
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	EVector* rtn = EVSpace_Vdiv(self, rhs);
	// no need to error check, if rtn is NULL, we return it anyway
	return (PyObject*)(rtn);
}

static PyObject* VOperator_Idiv(EVector* self, PyObject* arg)
{
	double rhs;
	if (PyFloat_Check(arg))
		rhs = PyFloat_AS_DOUBLE(arg);
	else if (PyLong_Check(arg)) {
		rhs = PyLong_AsDouble(arg);
		if (rhs < 0)
			return NULL;
	}
	else
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Vidiv(self, rhs);
	
	return (PyObject*)(self);
}

// todo: find out what all other methods do, and see if we need to/can implement them
static PyNumberMethods EVector_NBMethods = {
	.nb_add = (binaryfunc)VOperator_Add,
	.nb_subtract = (binaryfunc)VOperator_Sub,
	.nb_multiply = (binaryfunc)VOperator_Mult,
	.nb_negative = (unaryfunc)VOperator_Neg,
	.nb_absolute = (unaryfunc)VOperator_Abs,
	.nb_inplace_add = (binaryfunc)VOperator_Iadd,
	.nb_inplace_subtract = (binaryfunc)VOperator_Isub,
	.nb_inplace_multiply = (binaryfunc)VOperator_Imult,
	.nb_true_divide = (binaryfunc)VOperator_Div,
	.nb_inplace_true_divide = (binaryfunc)VOperator_Idiv,
};

/*	Class Methods  */
static PyObject* EVector_Dot(EVector* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return PyFloat_FromDouble(EVSpace_Dot(self, (EVector*)args));
}

static PyObject* EVector_Cross(EVector* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) 
		Py_RETURN_NOTIMPLEMENTED;

	EVector* rtn = EVSpace_Cross(self, (EVector*)args);

	return (PyObject*)(rtn);
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
	EVector* rtn = EVSpace_Norm(self);
	return (PyObject*)(rtn);
}

static PyObject* EVector_Normalize(EVector* self, PyObject* UNUSED)
{
	EVSpace_Inorm(self);
	Py_RETURN_NONE;
}

static PyObject* EVector_Vang(EVector* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Vang(self, (EVector*)args));
}

static PyObject* EVector_Vxcl(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EVector type.");
		return NULL;
	}

	EVector* rtn = EVSpace_Vxcl(self, (EVector*)arg);

	return (PyObject*)(rtn);
}

static PyObject* EVector_getx(EVector* self, void* closure)
{
	return PyFloat_FromDouble(self->m_arr[0]);
}

static int EVector_setx(EVector* self, PyObject* value, void* closure)
{
	if (value == NULL) {
		PyErr_SetString(PyExc_TypeError, "Cannot delete the x attribute.");
		return -1;
	}
	if (PyFloat_Check(value)) {
		self->m_arr[0] = PyFloat_AS_DOUBLE(value); // no need to error check
		return 0;
	}
	else if (PyLong_Check(value)) {
		double rhs = PyLong_AsDouble(value); // can raise OverflowError
		if (rhs < 0)
			return (int)rhs;

		self->m_arr[0] = rhs;
		return 0;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "Value must be either Float or Int type.");
		return -1;
	}
}

static PyObject* EVector_gety(EVector* self, void* closure)
{
	return PyFloat_FromDouble(self->m_arr[1]);
}

static int EVector_sety(EVector* self, PyObject* value, void* closure)
{
	if (value == NULL) {
		PyErr_SetString(PyExc_TypeError, "Cannot delete the y attribute.");
		return -1;
	}
	if (PyFloat_Check(value)) {
		self->m_arr[1] = PyFloat_AS_DOUBLE(value); // no need to error check
		return 0;
	}
	else if (PyLong_Check(value)) {
		double rhs = PyLong_AsDouble(value); // can raise OverflowError
		if (rhs < 0)
			return (int)rhs;

		self->m_arr[1] = rhs;
		return 0;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "Value must be either Float or Int type.");
		return -1;
	}
}

static PyObject* EVector_getz(EVector* self, void* closure)
{
	return PyFloat_FromDouble(self->m_arr[2]);
}

static int EVector_setz(EVector* self, PyObject* value, void* closure)
{
	if (value == NULL) {
		PyErr_SetString(PyExc_TypeError, "Cannot delete the z attribute.");
		return -1;
	}
	if (PyFloat_Check(value)) {
		self->m_arr[2] = PyFloat_AS_DOUBLE(value); // no need to error check
		return 0;
	}
	else if (PyLong_Check(value)) {
		double rhs = PyLong_AsDouble(value); // can raise OverflowError
		if (rhs < 0)
			return (int)rhs;

		self->m_arr[2] = rhs;
		return 0;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "Value must be either Float or Int type.");
		return -1;
	}
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
		return PyBool_FromLong(EVSpace_Veq((EVector*)self, (EVector*)other));
	case Py_NE:
		return PyBool_FromLong(EVSpace_Vne((EVector*)self, (EVector*)other));
	default:
		Py_RETURN_NOTIMPLEMENTED;
	}
}

static PyGetSetDef EVector_getsetters[] = {
	{"x", (getter) EVector_getx, (setter) EVector_setx, "X-Component", NULL},
	{"y", (getter) EVector_gety, (setter) EVector_sety, "Y-Component", NULL},
	{"z", (getter) EVector_getz, (setter) EVector_setz, "Z-Component", NULL},
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
	{"ref", (PyCFunction)Ref_check, METH_O, "prints the reference count."},
	{NULL}
};

// todo: learn about the other .tp_XXXX values and see if we can add more
static PyTypeObject EVectorType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name		= "pyevspace.EVector",
	.tp_doc			= PyDoc_STR("Eulcidean Vector"),
	.tp_basicsize	= sizeof(EVector),
	.tp_itemsize	= 0,
	.tp_flags		= Py_TPFLAGS_DEFAULT,
	.tp_new			= PyType_GenericNew,
	.tp_init		= (initproc)EVector_init,
	.tp_getset		= EVector_getsetters,
	.tp_methods		= EVector_Methods,
	.tp_str			= (reprfunc)EVector_str,
	.tp_as_number	= &EVector_NBMethods,
	.tp_richcompare	= (richcmpfunc)EVector_richcompare,
};

/****************************************************/
/************** EMatrix implementation **************/
/****************************************************/

static PyObject* MOperator_Add(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EMatrix type.");
		return NULL;
	}

	// todo: does this need to be reference counted?
	PyTypeObject* type = self->ob_base.ob_type;
	EMatrix* rtn = (EMatrix*)type->tp_new(type, NULL, NULL);

	if (!rtn)
		return NULL;
	Py_INCREF(rtn); // todo: to we need to increment this?

	EVSpace_Madd(rtn, self, (EMatrix*)args);
	return (PyObject*)rtn;
}

static PyObject* MOperator_Sub(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EMatrix type.");
		return NULL;
	}

	// todo: does this need to be incremented?
	PyTypeObject* type = self->ob_base.ob_type;
	EMatrix* rtn = (EMatrix*)type->tp_new(type, NULL, NULL);

	if (!rtn)
		return NULL;
	Py_INCREF(rtn); // todo: to we need this?

	EVSpace_Msub(rtn, self, (EMatrix*)args);
	return (PyObject*)rtn;
}

static PyObject* MOperator_Mult(EMatrix* self, PyObject* args)
{
	// todo: do we need to count this?
	PyTypeObject* type = self->ob_base.ob_type;
	PyObject* rtn = NULL;

	int option = -1;
	if (PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		option = 0;
		rtn = type->tp_new(type, NULL, NULL);
	}
	else if (PyObject_TypeCheck(args, &EVectorType)) {
		option = 1;
		rtn = EVectorType.tp_new(&EVectorType, NULL, NULL);
	}
	else if (PyFloat_Check(args) || PyLong_Check(args)) {
		option = 2;
		rtn = type->tp_new(type, NULL, NULL);
	}
	else {
		PyErr_SetString(PyExc_TypeError, "Argument must be EMatrix, EVector or float type.");
		return NULL;
	}

	if (!rtn)
		return NULL;
	Py_INCREF(rtn); // todo: do we need this?

	switch (option) {
	case 0:
		EVSpace_Mmultm((EMatrix*)rtn, self, (EMatrix*)args);
		break;
	case 1:
		EVSpace_Mmultv((EVector*)rtn, self, (EVector*)args);
		break;
	case 2:
		EVSpace_Mmultd((EMatrix*)rtn, self, PyFloat_AsDouble(args));
		break;
	// no need for default
	}
	return (PyObject*)rtn;
}

static PyObject* MOperator_Neg(EMatrix* self, PyObject* UNUSED)
{
	// todo: do we need to count this?
	PyTypeObject* type = self->ob_base.ob_type;
	EMatrix* rtn = (EMatrix*)type->tp_new(type, NULL, NULL);

	if (!rtn)
		return NULL;
	Py_INCREF(rtn); // todo: do we need this?

	EVSpace_Mneg(rtn, self);
	return (PyObject*)rtn;
}

static PyObject* MOperator_Iadd(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EMatrix type.");
		return NULL;
	}

	EVSpace_Miadd(self, (EMatrix*)args);
	return (PyObject*)self;
}

static PyObject* MOperator_Isub(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be EMatrix type.");
		return NULL;
	}

	EVSpace_Misub(self, (EMatrix*)args);
	return (PyObject*)self;
}

static PyObject* MOperator_Imult(EMatrix* self, PyObject* args)
{
	int option = -1;
	if (PyObject_TypeCheck(args, self->ob_base.ob_type))
		option = 0;
	if (PyFloat_CheckExact(args)) // todo: do we need to check for long? can we do it in one call?
		option = 1;

	switch (option) {
	case 0:
		EVSpace_Mimultm(self, (EMatrix*)args);
		break;
	case 1:
		EVSpace_Mimultd(self, PyFloat_AS_DOUBLE(args)); // todo: will this work with a long?
		break;
	}
	return (PyObject*)self;
}

static PyObject* MOperator_Div(EMatrix* self, PyObject* args)
{
	// todo: find the right check for this
	if (!PyFloat_CheckExact(args) && !PyLong_CheckExact(args)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be float type.");
		return NULL;
	}

	// todo: do we need to count this?
	PyTypeObject* type = self->ob_base.ob_type;
	EMatrix* rtn = (EMatrix*)type->tp_new(type, NULL, NULL);

	if (!rtn)
		return NULL;
	Py_INCREF(rtn); // todo: do we need this?

	EVSpace_Mdiv(rtn, self, PyFloat_AsDouble(args));
	return (PyObject*)rtn;
}

static PyObject* MOperator_Idiv(EMatrix* self, PyObject* args)
{
	// todo: find the right check for this
	if (!PyFloat_CheckExact(args) && !PyLong_CheckExact(args)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be float type.");
		return NULL;
	}

	EVSpace_Midiv(self, PyFloat_AsDouble(args));
	return (PyObject*)self;
}

// todo: find out what the rest of these do and see if we need to/can implement them.
static PyNumberMethods EMatrix_NBMethods = {
	.nb_add = (binaryfunc)MOperator_Add,
	.nb_subtract = (binaryfunc)MOperator_Sub,
	.nb_multiply = (binaryfunc)MOperator_Mult,
	.nb_negative = (unaryfunc)MOperator_Neg,
	.nb_inplace_add = (binaryfunc)MOperator_Iadd,
	.nb_inplace_subtract = (binaryfunc)MOperator_Isub,
	.nb_inplace_multiply = (binaryfunc)MOperator_Imult,
	.nb_true_divide = (binaryfunc)MOperator_Div,
	.nb_inplace_true_divide = (binaryfunc)MOperator_Idiv,
};

static PyObject* EMatrix_det(EMatrix* self, PyObject* UNUSED)
{
	// todo: does this get incremented?
	PyObject* rtn = PyFloat_FromDouble(EVSpace_Det(self));

	if (!rtn)
		return NULL;
	Py_INCREF(rtn);

	return rtn;
}

static PyObject* EMatrix_trans(EMatrix* self, PyObject* UNUSED)
{
	// todo: do we need to count this?
	PyTypeObject* type = self->ob_base.ob_type;
	EMatrix* rtn = (EMatrix*)type->tp_new(type, NULL, NULL);

	if (!rtn)
		return NULL;
	Py_INCREF(rtn); // todo: do we need this?

	EVSpace_Trans(rtn, self);
	return (PyObject*)rtn;
}

static PyObject* EMatrix_set(EMatrix* self, PyObject* args)
{
	int i, j;
	double val;

	if (!PyArg_ParseTuple(args, "lld", &i, &j, &val)) // will a long work for this?
		return NULL;

	if (i < 0 || i > 3) {
		PyErr_SetString(PyExc_ValueError, "Row index out of bounds.");
		return NULL;
	}
	if (j < 0 || j > 3) {
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

	if (i < 0 || i > 3) {
		PyErr_SetString(PyExc_ValueError, "Row index out of bounds.");
		return NULL;
	}
	if (j < 0 || j > 3) {
		PyErr_SetString(PyExc_ValueError, "Column index out of bounds.");
		return NULL;
	}

	PyObject* rtn = PyFloat_FromDouble(self->m_arr[i][j]);
	Py_INCREF(rtn); // todo: does pyfloat_fromdouble increment this or do we need to?
	return rtn;
}

static int EMatrix_init(EMatrix* self, PyObject* args, PyObject* UNUSED)
{
	EVector* c0 = NULL, *c1 = NULL, *c2 = NULL;

	// todo: does parsetuple increment these addresses?
	if (!PyArg_ParseTuple(args, "|OOO", &c0, &c1, &c2))
		return -1;
	/*Py_INCREF(c0); dont need these if they're borrowed references from args
	Py_INCREF(c1);
	Py_INCREF(c2);*/

	// todo: can we do this more efficiently? we must be able to, too many loops here
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			self->m_arr[i][j] = 0;
	}

	if (c0 != NULL) {
		for (int i = 0; i < 3; i++)
			self->m_arr[i][0] = c0->m_arr[i];
	}
	if (c1 != NULL) {
		for (int i = 0; i < 3; i++)
			self->m_arr[i][1] = c1->m_arr[i];
	}
	if (c2 != NULL) {
		for (int i = 0; i < 3; i++)
			self->m_arr[i][2] = c2->m_arr[i];
	}

	return 0;
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

	// todo: use switch here?
	if (op == Py_EQ) return PyBool_FromLong(EVSpace_Meq((EMatrix*)self, (EMatrix*)other));
	else if (op == Py_NE) return PyBool_FromLong(EVSpace_Mne((EMatrix*)self, (EMatrix*)other));
	else return Py_NotImplemented;
}

// todo: craete a get/set method for the hidden array attribute
static PyMethodDef EMatrix_Methods[] = {
	{"det", (PyCFunction)EMatrix_det, METH_NOARGS, "Returns the determinate of a matrix."},
	{"transpose", (PyCFunction)EMatrix_trans, METH_NOARGS, "Returns the transpose of a matrix."},
	{"set", (PyCFunction)EMatrix_set, METH_VARARGS, "Sets a matrix comonent to a given value."},
	{"get", (PyCFunction)EMatrix_get, METH_VARARGS, "Returns a matrix comonent."},
	{NULL}
};

// todo: leanr about the rest of the .tp_XXX methods do see what else can help us.
static PyTypeObject EMatrixType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyevspace.EMatrix",
	.tp_doc = PyDoc_STR("Eulcidean Matrix"),
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

// todo: anything else we need to do here?
static PyModuleDef EVSpacemodule = {
	PyModuleDef_HEAD_INIT,
	.m_name = "pyevspace",
	.m_doc = "Module library for a Euclidean vector space with vector and matrix types.",
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
		goto error;
	e1->m_arr[0] = 1;
	if (PyDict_SetItemString(EVectorType.tp_dict, "e1", (PyObject*)e1) < 0)
		goto error;

	// create e2 vector
	e2 = (EVector*)EVectorType.tp_new(&EVectorType, NULL, NULL);
	if (!e2)
		goto error;
	e2->m_arr[1] = 1;
	if (PyDict_SetItemString(EVectorType.tp_dict, "e2", (PyObject*)e2) < 0)
		goto error;

	// craete e3 vector
	e3 = (EVector*)EVectorType.tp_new(&EVectorType, NULL, NULL);
	if (!e3)
		goto error;
	e3->m_arr[2] = 1;
	if (PyDict_SetItemString(EVectorType.tp_dict, "e3", (PyObject*)e3) < 0)
		goto error;

	// create identity matrix
	I = (EMatrix*)EMatrixType.tp_new(&EMatrixType, NULL, NULL);
	if (!I)
		goto error;
	I->m_arr[0][0] = I->m_arr[1][1] = I->m_arr[2][2] = 1.0;
	if (PyDict_SetItemString(EMatrixType.tp_dict, "I", (PyObject*)I) < 0)
		goto error;

	// create module
	m = PyModule_Create(&EVSpacemodule);
	if (!m)
		goto error;

	// EVector 
	EVSpace_API[EVSpace_Vadd_NUM]	= (void*)EVSpace_Vadd;
	EVSpace_API[EVSpace_Vsub_NUM]	= (void*)EVSpace_Vsub;
	EVSpace_API[EVSpace_Vmult_NUM]	= (void*)EVSpace_Vmult;
	EVSpace_API[EVSpace_Vneg_NUM]	= (void*)EVSpace_Vneg;
	EVSpace_API[EVSpace_Vabs_NUM]	= (void*)EVSpace_Vabs;
	EVSpace_API[EVSpace_Viadd_NUM]	= (void*)EVSpace_Viadd;
	EVSpace_API[EVSpace_Visub_NUM]	= (void*)EVSpace_Visub;
	EVSpace_API[EVSpace_Vimult_NUM]	= (void*)EVSpace_Vimult;
	EVSpace_API[EVSpace_Vdiv_NUM]	= (void*)EVSpace_Vdiv;
	EVSpace_API[EVSpace_Vidiv_NUM]	= (void*)EVSpace_Vidiv;
	EVSpace_API[EVSpace_Veq_NUM]	= (void*)EVSpace_Veq;
	EVSpace_API[EVSpace_Vne_NUM]	= (void*)EVSpace_Vne;
	EVSpace_API[EVSpace_Dot_NUM ]	= (void*)EVSpace_Dot;
	EVSpace_API[EVSpace_Cross_NUM]	= (void*)EVSpace_Cross;
	EVSpace_API[EVSpace_Mag_NUM]	= (void*)EVSpace_Mag;
	EVSpace_API[EVSpace_Mag2_NUM]	= (void*)EVSpace_Mag2;
	EVSpace_API[EVSpace_Norm_NUM]	= (void*)EVSpace_Norm;
	EVSpace_API[EVSpace_Inorm_NUM]	= (void*)EVSpace_Inorm;
	EVSpace_API[EVSpace_Vang_NUM]	= (void*)EVSpace_Vang;
	EVSpace_API[EVSpace_Vxcl_NUM]	= (void*)EVSpace_Vxcl;
	// EMatrix
	EVSpace_API[EVSpace_Madd_NUM]	= (void*)EVSpace_Madd;
	EVSpace_API[EVSpace_Msub_NUM]	= (void*)EVSpace_Msub;
	EVSpace_API[EVSpace_Mmultm_NUM] = (void*)EVSpace_Mmultm;
	EVSpace_API[EVSpace_Mmultv_NUM] = (void*)EVSpace_Mmultv;
	EVSpace_API[EVSpace_Mmultd_NUM] = (void*)EVSpace_Mmultd;
	EVSpace_API[EVSpace_Mneg_NUM]	= (void*)EVSpace_Mneg;
	EVSpace_API[EVSpace_Miadd_NUM]	= (void*)EVSpace_Miadd;
	EVSpace_API[EVSpace_Misub_NUM]	= (void*)EVSpace_Misub;
	EVSpace_API[EVSpace_Mimultm_NUM] = (void*)EVSpace_Mimultm;
	EVSpace_API[EVSpace_Mimultd_NUM] = (void*)EVSpace_Mimultd;
	EVSpace_API[EVSpace_Mdiv_NUM]	= (void*)EVSpace_Mdiv;
	EVSpace_API[EVSpace_Midiv_NUM]	= (void*)EVSpace_Midiv;
	EVSpace_API[EVSpace_Meq_NUM]	= (void*)EVSpace_Meq;
	EVSpace_API[EVSpace_Mne_NUM]	= (void*)EVSpace_Mne;
	EVSpace_API[EVSpace_Det_NUM]	= (void*)EVSpace_Det;
	EVSpace_API[EVSpace_Trans_NUM]	= (void*)EVSpace_Trans;
	EVSpace_API[EVSpace_Mset_NUM]	= (void*)EVSpace_Mset;

	// create capsule
	c_api_object = PyCapsule_New((void*)EVSpace_API, "evspace._C_API", NULL);

	// add capsule to module
	Py_INCREF(c_api_object);
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

	Py_XDECREF(m);
	Py_XDECREF(c_api_object);
	Py_XDECREF(e1);
	Py_XDECREF(e2);
	Py_XDECREF(e3);
	Py_XDECREF(I);

	return NULL;
}