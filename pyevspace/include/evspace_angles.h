#ifndef EVSPACE_ANGLES_H
#define EVSPACE_ANGLES_H

#include <Python.h>
#include <evspacemodule.h>
//#include <math.h>

#define PI		3.14159265358979323846
#define PI_2	1.57079632679489661923

// forward declaration
static PyTypeObject EVSpace_AnglesType;
static PyTypeObject EVSpace_OrderType;

static EVSpace_Angles*
_angles_new(double alpha, double beta, double gamma, PyTypeObject* type)
{
	EVSpace_Angles* angles = (EVSpace_Angles*)type->tp_alloc(type, 0);
	if (!angles)
		return NULL;

	angles->alpha = alpha;
	angles->beta = beta;
	angles->gamma = gamma;

	return angles;
}

#define new_angle(a, b, g)	_angles_new(a, b, c, &EVSpace_AnglesType);
#define new_angle_empty		new_angles(0.0, 0.0, 0.0);

static PyObject*
angles_new(PyTypeObject* type, PyObject* args, PyObject* PyUNUSED(_))
{
	double alpha, beta, gamma;

	if (!PyArg_ParseTuple(args, "ddd", &alpha, &beta, &gamma) < 0)
		return NULL;

	return (PyObject*)_angles_new(alpha, beta, gamma, type);
}

static size_t
__angles_str_length(const EVSpace_Angles* angles)
{
	return snprintf(NULL, 0, "[%f, %f, %f]",
		angles->alpha, angles->beta, angles->gamma);
}

static PyObject*
angles_str(const EVSpace_Angles* angles)
{
	const size_t length = __angles_str_length(angles);

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
	const size_t length = __angles_str_length(angles);

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

// dont think we ever used this
// will expand an EVSpace_Order* to the axis for method calls
#define ORDER_AS_AXIS(o)	o->first, o->second, o->third

static EVSpace_Order*
_order_new(EVSpace_Axis first, EVSpace_Axis second, EVSpace_Axis third, 
	PyTypeObject* type)
{
	EVSpace_Order* order = (EVSpace_Order*)type->tp_alloc(type, 0);
	if (!order)
		return NULL;

	order->first = first;
	order->second = second;
	order->third = third;

	return order;
}

#define new_order(f, s, t)		_order_new(f, s, t, &EVSpace_OrderType)

static PyObject*
order_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED(_))
{
	EVSpace_Axis first, second, third;

	if (PyArg_ParseTuple(args, "iii",
		(int*)&first, (int*)&second, (int*)&third) < 0)
		return NULL;

	// do we need to check the values of enum type?
	return (PyObject*)_order_new(first, second, third, type);
}

static void
__order_axis_names(const EVSpace_Order* order, char* first, char* second, char* third)
{
	if (order->first == X_AXIS)
		sprintf(first, "X_Axis");
	else if (order->first == Y_AXIS)
		sprintf(first, "Y_Axis");
	else if (order->first == Z_AXIS)
		sprintf(first, "Z_Axis");

	if (order->second == X_AXIS)
		sprintf(second, "X_Axis");
	else if (order->second == Y_AXIS)
		sprintf(second, "Y_Axis");
	else if (order->second == Z_AXIS)
		sprintf(second, "Z_Axis");

	if (order->third == X_AXIS)
		sprintf(third, "X_Axis");
	else if (order->third == Y_AXIS)
		sprintf(third, "Y_Axis");
	else if (order->third == Z_AXIS)
		sprintf(third, "Z_Axis");
}

static PyObject*
order_str(const EVSpace_Order* order)
{
	char first[7], second[7], third[7];
	char* buffer = malloc(25);
	if (!buffer)
		return NULL;

	__order_axis_names(order, first, second, third);
	sprintf(buffer, "[%s, %s, %s]", first, second, third);

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);
	return rtn;
}

static PyObject*
order_repr(const EVSpace_Order* order)
{
	char first[7], second[7], third[7];
	char* buffer = malloc(32);
	if (!buffer)
		return NULL;

	__order_axis_names(order, first, second, third);
	sprintf(buffer, "order([%s, %s, %s])", first, second, third);

	PyObject* rtn = PyUnicode_FromString(buffer);
	free(buffer);
	return rtn;
}

static PyObject*
order_get_item(EVSpace_Order* self, Py_ssize_t index)
{
	EVSpace_Axis rtn;

	if (index == 0)
		rtn = self->first;
	else if (index == 1)
		rtn = self->second;
	else if (index == 2)
		rtn = self->third;
	else {
		PyErr_Format(PyExc_IndexError,
			"index (%i) must be in [0-2]", index);
		return NULL;
	}

	return PyLong_FromLong((long)rtn);
}

static int
order_set_item(EVSpace_Order* self, Py_ssize_t index, PyObject* value)
{
	int val = PyLong_AsLong(value);
	if (val == -1.0 && PyErr_Occurred())
		return -1;

	if (val < 0 || val > 2) {
		PyErr_SetString(PyExc_ValueError, "value must be 0, 1, or 2");
		return -1;
	}

	if (index == 0)
		self->first = (EVSpace_Axis)val;
	else if (index == 1)
		self->second = (EVSpace_Axis)val;
	else if (index == 2)
		self->third = (EVSpace_Axis)val;
	else {
		PyErr_Format(PyExc_IndexError,
			"index (%i) must be in [0-2]", index);
		return -1;
	}

	return 0;
}

#endif // EVSPACE_ANGLES_H
