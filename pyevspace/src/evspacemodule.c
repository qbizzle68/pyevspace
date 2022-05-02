#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

typedef struct {
	PyObject_HEAD
	double m_arr[3];
} EVector;

static PyTypeObject EVectorType;

typedef struct {
	PyObject_HEAD
	double m_arr[3][3];
} EMatrix;

static PyTypeObject EMatrixType;

#define EVSPACE_MODULE
#include <evspacemodule.h>

#define EVSpace_RADIANS_TO_DEGREES 180.0 / 3.14159265358979323846

/*************************************/
/*	implimentation of C API methods  */
/*************************************/

// MAJOR TODO: change naming convention to EVSpace_Mat_XXX or EVSpace_Vec_XXX for ALL methods not just numeric methods

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

static EMatrix* EVSpace_Madd(const EMatrix* lhs, const EMatrix* rhs)
{
	EMatrix* rtn = (EMatrix*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			rtn->m_arr[i][j] = lhs->m_arr[i][j] + rhs->m_arr[i][j];
	}

	return rtn;
}

static EMatrix* EVSpace_Msub(const EMatrix* lhs, const EMatrix* rhs)
{
	EMatrix* rtn = (EMatrix*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			rtn->m_arr[i][j] = lhs->m_arr[i][j] - rhs->m_arr[i][j];
	}

	return rtn;
}

static EMatrix* EVSpace_Mmultm(const EMatrix* lhs, const EMatrix* rhs)
{
	EMatrix* rtn = (EMatrix*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			double sum = 0;
			for (int k = 0; k < 3; k++)
				sum += lhs->m_arr[i][k] * rhs->m_arr[k][j];
			rtn->m_arr[i][j] = sum;
		}
	}

	return rtn;
}

static EVector* EVSpace_Mmultv(const EMatrix* lhs, const EVector* rhs)
{
	EVector* rtn = (EVector*)rhs->ob_base.ob_type->tp_new(rhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	for (int i = 0; i < 3; i++) {
		double sum = 0;
		for (int j = 0; j < 3; j++)
			sum += lhs->m_arr[i][j] * rhs->m_arr[j];
		rtn->m_arr[i] = sum;
	}

	return rtn;
}

static EMatrix* EVSpace_Mmultd(const EMatrix* lhs, double rhs)
{
	EMatrix* rtn = (EMatrix*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			rtn->m_arr[i][j] = lhs->m_arr[i][j] * rhs;
	}

	return rtn;
}

static EMatrix* EVSpace_Mneg(const EMatrix* mat)
{
	EMatrix* rtn = (EMatrix*)mat->ob_base.ob_type->tp_new(mat->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			rtn->m_arr[i][j] = -mat->m_arr[i][j];
	}

	return rtn;
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
	EMatrix* tmp = EVSpace_Mmultm(lhs, rhs);
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

static void EVSpace_Mimultd(EMatrix* lhs, double rhs)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			lhs->m_arr[i][j] *= rhs;
	}
}

static EMatrix* EVSpace_Mdiv(const EMatrix* lhs, double rhs)
{
	EMatrix* rtn = (EMatrix*)lhs->ob_base.ob_type->tp_new(lhs->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			rtn->m_arr[i][j] = lhs->m_arr[i][j] / rhs;
	}

	return rtn;
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

static EMatrix* EVSpace_Trans(const EMatrix* mat)
{
	EMatrix* rtn = (EMatrix*)mat->ob_base.ob_type->tp_new(mat->ob_base.ob_type, NULL, NULL);
	if (!rtn)
		return NULL;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			rtn->m_arr[i][j] = mat->m_arr[j][i];
	}

	return rtn;
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

/****************************************************/
/************* EVector Numeric Methods **************/
/****************************************************/

static PyObject* VOperator_Add(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Vadd(self, (EVector*)arg);
}

static PyObject* VOperator_Sub(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Vsub(self, (EVector*)arg);
}

static PyObject* VOperator_Mult(EVector* self, PyObject* arg)
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

	return (PyObject*)EVSpace_Vmult(self, rhs);
}

static PyObject* VOperator_Neg(EVector* self)
{
	return (PyObject*)EVSpace_Vneg(self);
}

static PyObject* VOperator_Abs(EVector* self)
{
	return (PyObject*)PyFloat_FromDouble(EVSpace_Vabs(self));
}

static PyObject* VOperator_Iadd(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Viadd(self, (EVector*)arg);
	return Py_NewRef(self);
}

static PyObject* VOperator_Isub(EVector* self, PyObject* arg)
{
	if (!PyObject_TypeCheck(arg, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Visub(self, (EVector*)arg);
	return Py_NewRef(self);
}

static PyObject* VOperator_Imult(EVector* self, PyObject* arg)
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

	EVSpace_Vimult(self, rhs);
	return Py_NewRef(self);
}

static PyObject* VOperator_Div(EVector* self, PyObject* arg)
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

	return (PyObject*)EVSpace_Vdiv(self, rhs);
}

static PyObject* VOperator_Idiv(EVector* self, PyObject* arg)
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

	EVSpace_Vidiv(self, rhs);
	return Py_NewRef(self);
}

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

/****************************************************/
/************** EVector Class Methods ***************/
/****************************************************/

static PyObject* EVector_Mag(EVector* self, PyObject* UNUSED)
{
	return PyFloat_FromDouble(EVSpace_Mag(self));
}

static PyObject* EVector_Mag2(EVector* self, PyObject* UNUSED)
{
	return PyFloat_FromDouble(EVSpace_Mag2(self));
}

static PyObject* EVector_Normalize(EVector* self, PyObject* UNUSED)
{
	EVSpace_Inorm(self);
	Py_RETURN_NONE;
}

/* This is for debugging reference counts */
// todo: delete this when finished 
static PyObject* Ref_check(EVector* UNUSED, EVector* arg)
{
	printf("reference count: %i\n", (int)arg->ob_base.ob_refcnt);
	Py_RETURN_NONE;
}

static PyMethodDef EVector_Methods[] = {
	{"mag", (PyCFunction)EVector_Mag, METH_NOARGS, "Returns the magnitude of an EVector."},
	{"mag2", (PyCFunction)EVector_Mag2, METH_NOARGS, "Returns the square of the magnitude of an EVector."},
	{"normalize", (PyCFunction)EVector_Normalize, METH_NOARGS, "Normalized an EVector."},
	{"ref", (PyCFunction)Ref_check, METH_O, "prints the reference count."},
	{NULL}
};

/****************************************************/
/************* EVector Static Methods ***************/
/****************************************************/

static PyObject* EVector_Dot(PyObject* UNUSED, PyObject *const *args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_SetString(PyExc_IndexError, "Number of arguments should be 2.");
		return NULL;
	}
 
	EVector* lhs = (EVector*)args[0];
	EVector* rhs = (EVector*)args[1];

	if (!lhs && !rhs) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}
	if (!PyObject_TypeCheck(rhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Dot(lhs, rhs));
}

static PyObject* EVector_Cross(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_SetString(PyExc_IndexError, "Number of arguments should be 2.");
		return NULL;
	}

	EVector* lhs = (EVector*)args[0];
	EVector* rhs = (EVector*)args[1];

	if (!lhs && !rhs) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}
	if (!PyObject_TypeCheck(rhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return NULL;
	}

	EVector* rtn = EVSpace_Cross(lhs, rhs);

	return (PyObject*)(rtn);
}

static PyObject* EVector_Norm(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_SetString(PyExc_IndexError, "Number of arguments should be 2.");
		return NULL;
	}

	EVector* lhs = (EVector*)args[0];

	if (!lhs) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}

	return (PyObject*)EVSpace_Norm(lhs);
}

static PyObject* EVector_Vang(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_SetString(PyExc_IndexError, "Number of arguments should be 2.");
		return NULL;
	}

	EVector* lhs = (EVector*)args[0];
	EVector* rhs = (EVector*)args[1];

	if (!lhs) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}
	if (!PyObject_TypeCheck(rhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Vang(lhs, rhs));
}

static PyObject* EVector_Vxcl(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_SetString(PyExc_IndexError, "Number of arguments should be 2.");
		return NULL;
	}

	EVector* lhs = (EVector*)args[0];
	EVector* rhs = (EVector*)args[1];

	if (!lhs) {
		PyErr_SetString(PyExc_ValueError, "Arguments cannot be NULL.");
		return NULL;
	}
	if (!PyObject_TypeCheck(lhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return NULL;
	}
	if (!PyObject_TypeCheck(rhs, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return NULL;
	}

	return (PyObject*)(EVSpace_Vxcl(lhs, rhs));
}

static PyMethodDef EVector_ModuleMethods[] = {
	{"dot", (PyCFunction)EVector_Dot, METH_FASTCALL, "Return the dot product of two EVectors."},
	{"cross", (PyCFunction)EVector_Cross, METH_FASTCALL, "Return the cross product of two EVectors."},
	{"norm", (PyCFunction)EVector_Norm, METH_FASTCALL, "Returns a normalized version of an EVector."},
	{"vang", (PyCFunction)EVector_Vang, METH_FASTCALL, "Return the shortest angle between two EVector's."},
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
	if (index < -3 || index > 2) {
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
		return PyBool_FromLong(EVSpace_Veq((EVector*)self, (EVector*)other));
	case Py_NE:
		return PyBool_FromLong(EVSpace_Vne((EVector*)self, (EVector*)other));
	default:
		Py_RETURN_NOTIMPLEMENTED;
	}
}

static PyTypeObject EVectorType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name		= "pyevspace.EVector",
	.tp_doc			= PyDoc_STR("Eulcidean Vector"),
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

static PyObject* MOperator_Add(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Madd(self, (EMatrix*)args);
}

static PyObject* MOperator_Sub(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	return (PyObject*)EVSpace_Msub(self, (EMatrix*)args);
}

static PyObject* MOperator_Mult(EMatrix* self, PyObject* args)
{
	int option = -1;
	if (PyFloat_CheckExact(args))
		option = 0;
	else if (PyLong_CheckExact(args))
		option = 1;
	else if (PyObject_TypeCheck(args, self->ob_base.ob_type))
		option = 2;
	else if (PyObject_TypeCheck(args, &EVectorType))
		option = 3;
	else
		Py_RETURN_NOTIMPLEMENTED;

	double rhs;
	switch (option) {
	case 0:
		return (PyObject*)EVSpace_Mmultd(self, PyFloat_AS_DOUBLE(args));
	case 1:
		rhs = PyLong_AsDouble(args);
		if (PyErr_Occurred())
			return NULL;

		return (PyObject*)EVSpace_Mmultd(self, rhs);
	case 2:
		return (PyObject*)EVSpace_Mmultm(self, (EMatrix*)args);
	default:
		return (PyObject*)EVSpace_Mmultv(self, (EVector*)args);
	}
}

static PyObject* MOperator_Neg(EMatrix* self, PyObject* UNUSED)
{
	return (PyObject*)EVSpace_Mneg(self);
}

static PyObject* MOperator_Iadd(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Miadd(self, (EMatrix*)args);
	return Py_NewRef(self);
}

static PyObject* MOperator_Isub(EMatrix* self, PyObject* args)
{
	if (!PyObject_TypeCheck(args, self->ob_base.ob_type))
		Py_RETURN_NOTIMPLEMENTED;

	EVSpace_Misub(self, (EMatrix*)args);
	return Py_NewRef(self);
}

static PyObject* MOperator_Imult(EMatrix* self, PyObject* args)
{
	int option = -1;
	if (PyObject_TypeCheck(args, self->ob_base.ob_type))
		option = 0;
	else if (PyFloat_CheckExact(args))
		option = 1;
	else if (PyLong_CheckExact(args))
		option = 2;
	else
		Py_RETURN_NOTIMPLEMENTED;

	double rhs;
	switch (option) {
	case 0:
		EVSpace_Mimultm(self, (EMatrix*)args);
		return Py_NewRef(self);
	case 1:
		EVSpace_Mimultd(self, PyFloat_AS_DOUBLE(args));
		return Py_NewRef(self);
	default: // todo: does this matter which is default?
		rhs = PyLong_AsDouble(args);
		if (PyErr_Occurred())
			return NULL;

		EVSpace_Mimultd(self, rhs);
		return Py_NewRef(self);
	}
}

static PyObject* MOperator_Div(EMatrix* self, PyObject* args)
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

	return (PyObject*)EVSpace_Mdiv(self, rhs);
}

static PyObject* MOperator_Idiv(EMatrix* self, PyObject* args)
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

	EVSpace_Midiv(self, rhs);
	return Py_NewRef(self);
}

// todo: split the multiply into multiply(float/int) and matrix multiply(vector/matrix)
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

/****************************************************/
/************** EMatrix Class Methods ***************/
/****************************************************/

static PyObject* EMatrix_set(EMatrix* self, PyObject* args)
{
	int i, j;
	double val;

	if (!PyArg_ParseTuple(args, "lld", &i, &j, &val))
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

	return PyFloat_FromDouble(self->m_arr[i][j]);
}

// todo: can we manage these with a sequence like protocol?
static PyMethodDef EMatrix_Methods[] = {
	{"set", (PyCFunction)EMatrix_set, METH_VARARGS, "Sets a matrix comonent to a given value."},
	{"get", (PyCFunction)EMatrix_get, METH_VARARGS, "Returns a matrix comonent."},
	{NULL}
};

/****************************************************/
/************* EMatrix Module Methods ***************/
/****************************************************/

static PyObject* EMatrix_det(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_SetString(PyExc_IndexError, "Number of arguments should be 1");
		return NULL;
	}

	EMatrix* mat = (EMatrix*)args[0];

	if (!PyObject_TypeCheck(mat, &EMatrixType)) {
		PyErr_SetString(PyExc_TypeError, "Argument should be EMatrix type.");
		return NULL;
	}

	return PyFloat_FromDouble(EVSpace_Det(mat));
}

static PyObject* EMatrix_trans(PyObject* UNUSED, PyObject* const* args, Py_ssize_t size)
{
	if (size != 1) {
		PyErr_SetString(PyExc_IndexError, "Number of arguments should be 1");
		return NULL;
	}

	EMatrix* mat = (EMatrix*)args[0];

	if (!PyObject_TypeCheck(mat, &EMatrixType)) {
		PyErr_SetString(PyExc_TypeError, "Argument should be EMatrix type.");
		return NULL;
	}

	return (PyObject*)EVSpace_Trans(mat);
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
	EVector* c0 = NULL, *c1 = NULL, *c2 = NULL;

	if (!PyArg_ParseTuple(args, "|OOO", &c0, &c1, &c2)) // todo: allow this to be a list or evector
		return -1;

	if (c0 && !PyObject_TypeCheck(c0, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be EVector type.");
		return -1;
	}
	else if (c1 && !PyObject_TypeCheck(c1, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be EVector type.");
		return -1;
	}
	else if (c2 && !PyObject_TypeCheck(c2, &EVectorType)) {
		PyErr_SetString(PyExc_TypeError, "Third argument must be EVector type.");
		return -1;
	}

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

	switch (op) {
	case Py_EQ:
		return PyBool_FromLong(EVSpace_Meq((EMatrix*)self, (EMatrix*)other));
	case Py_NE:
		return PyBool_FromLong(EVSpace_Mne((EMatrix*)self, (EMatrix*)other));
	default:
		Py_RETURN_NOTIMPLEMENTED;
	}
}

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
	m = PyModule_Create(&EVSpacemodule);
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