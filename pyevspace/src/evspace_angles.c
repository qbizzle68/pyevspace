

#ifdef __EVSPACE_SOURCE_INCLUDE__

#ifndef EVSPACE_ANGLES_C
#define EVSPACE_ANGLES_C

#include <evspace_angles.h>

// check if we ended up using this
static double
modulate_angle(double angle)
{
	if (angle == PI)
		return 0;
	else if (angle > PI)
		return fmod(angle, PI);
	else {
		return fmod(angle, PI) + PI;
	}
}

static PyObject*
new_angles_ex(double alpha, double beta, double gamma, PyTypeObject* type)
{
	EVSpace_Angles* angles = (EVSpace_Angles*)type->tp_alloc(type, 0);
	if (!angles)
		return NULL;

	angles->alpha = alpha;
	angles->beta = beta;
	angles->gamma = gamma;

	return (PyObject*)angles;
}

#define new_angle(a, b, g)	new_angles_ex(a, b, c, &EVSpace_AnglesType);
#define new_angle_empty		new_angles(0.0, 0.0, 0.0);

static PyObject*
angles_new(PyTypeObject* type, PyObject* args, PyObject* PyUNUSED(_))
{
	double alpha, beta, gamma;

	if (!PyArg_ParseTuple(args, "ddd", &alpha, &beta, &gamma) < 0)
		return NULL;

	return new_angles_ex(alpha, beta, gamma, type);
}

static size_t
angle_str_length(const EVSpace_Angles* angles)
{
	return snprintf(NULL, 0, "[%f, %f, %f]",
		angles->alpha, angles->beta, angles->gamma);
}

static PyObject*
angles_str(const EVSpace_Angles* angles)
{
	const size_t length = angle_str_length(angles);

	char* buffer = malloc(length + 1);
	if (!buffer)
		return NULL;

	sprintf(buffer, "[%f, %f, %f]", angles->alpha,
		angles->beta, angles->gamma);

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);
	return rtn;
}

static PyObject*
angles_repr(const EVSpace_Angles* angles)
{
	const size_t length = angle_str_length(angles);

	// 8 extra chars for name, 1 for null char
	char* buffer = malloc(length + 9);
	if (!buffer)
		return NULL;

	sprintf(buffer, "angles([%f, %f, %f])", angles->alpha,
		angles->beta, angles->gamma);

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);
	return rtn;
}

static PyObject*
angles_reduce(const EVSpace_Angles* self, PyObject* Py_UNUSED(_))
{
	return Py_BuildValue(
		"(O(ddd))",
		Py_TYPE(self),
		self->alpha,
		self->beta,
		self->gamma);
}

static PyObject*
angles_get_item(const EVSpace_Angles* self, Py_ssize_t index)
{
	if (index == 0)
		return PyFloat_FromDouble(self->alpha);
	else if (index == 1)
		return PyFloat_FromDouble(self->beta);
	else if (index == 2)
		return PyFloat_FromDouble(self->gamma);
	else {
		PyErr_Format(PyExc_IndexError,
			"index (%i) must be in [0-2]", index);
		return NULL;
	}
}

static int
angles_set_item(EVSpace_Angles* self, Py_ssize_t index, PyObject* value)
{
	double dbl_value = PyFloat_AsDouble(value);
	if (dbl_value == -1.0 && PyErr_Occurred())
		return -1;

	if (index == 0)
		self->alpha = dbl_value;
	else if (index == 1)
		self->beta = dbl_value;
	else if (index == 2)
		self->gamma = dbl_value;
	else {
		PyErr_Format(PyExc_IndexError,
			"index (%i) must be in [0-2]", index);
		return -1;
	}

	return 0;
}


#endif // EVSPACE_ANGLES_C
#endif // __EVSPACE_SOURCE_INCLUDE__
