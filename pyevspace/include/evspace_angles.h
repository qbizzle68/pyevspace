#ifndef EVSPACE_ANGLES_H
#define EVSPACE_ANGLES_H

#include <Python.h>
#include <evspacemodule.h>

// forward declaration
static PyTypeObject EVSpace_AnglesType;
static PyTypeObject EVSpace_OrderType;

static EVSpace_Angles*
_angles_new(double alpha, double beta, double gamma, PyTypeObject* type)
{
    EVSpace_Angles* angles = (EVSpace_Angles*)type->tp_alloc(type, 0);
    if (!angles) {
        return NULL;
    }

    angles->alpha = alpha;
    angles->beta = beta;
    angles->gamma = gamma;
    angles->master = NULL;

    return angles;
}

#define new_angle(a, b, g)	_angles_new(a, b, g, &EVSpace_AnglesType);
#define new_angle_empty		new_angles(0.0, 0.0, 0.0);

#include <evspace_rotation.h>
#include <evspace_refframe.h>

static PyObject*
angles_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED(_))
{
    double alpha, beta, gamma;

    if (PyTuple_GET_SIZE(args) == 0) {
        return (PyObject*)_angles_new(0.0, 0.0, 0.0, type);
    }

    if (!PyArg_ParseTuple(args, "ddd", &alpha, &beta, &gamma))
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
    if (!buffer) {
        return NULL;
    }

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
    if (!buffer) {
        return NULL;
    }

    sprintf(buffer, "Angles(%f, %f, %f)", angles->alpha,
            angles->beta, angles->gamma);

    PyObject* rtn = PyUnicode_FromString(buffer);
    free(buffer);
    return rtn;
}

static PyObject*
angles_reduce(const EVSpace_Angles* self, PyObject* Py_UNUSED(_))
{
    return Py_BuildValue("(O(ddd))", Py_TYPE(self), self->alpha, self->beta,
                         self->gamma);
}

static PyObject*
angles_get_item(const EVSpace_Angles* self, Py_ssize_t index)
{
    if (index == 0) {
        return PyFloat_FromDouble(self->alpha);
    }
    else if (index == 1) {
        return PyFloat_FromDouble(self->beta);
    }
    else if (index == 2) {
        return PyFloat_FromDouble(self->gamma);
    }
    else {
        PyErr_Format(PyExc_IndexError,
            "index (%i) must be in [0-2]",
            index);
        return NULL;
    }
}

#define ROTATION_ANGLE_ALPHA    0
#define ROTATION_ANGLE_BETA	    1
#define ROTATION_ANGLE_GAMMA    2

static PyObject*
angles_getter(EVSpace_Angles* self, void* closure)
{
    PyObject* rtn = angles_get_item(self, (Py_ssize_t)(closure));
    return rtn;
}

static int
angles_setter(EVSpace_Angles* self, PyObject* rhs, void* closure)
{
    double value = PyFloat_AsDouble(rhs);
    if (value == -1.0 && PyErr_Occurred()) {
        return -1;
    }

    double* angle_addr = NULL;
    size_t which = (size_t)(closure);
    if (which == ROTATION_ANGLE_ALPHA) {
        angle_addr = &self->alpha;
    }
    else if (which == ROTATION_ANGLE_BETA) {
        angle_addr = &self->beta;
    }
    else if (which == ROTATION_ANGLE_GAMMA) {
        angle_addr = &self->gamma;
    }

    // Track the original value to revert if computing matrix fails.
    double old_value = *angle_addr;
    *angle_addr = value;
    EVSpace_ReferenceFrame* master = (EVSpace_ReferenceFrame*)self->master;

    if (self->master) {
        if (_refframe_update_matrix(master) < 0) {
            *angle_addr = old_value;
            return -1;
        }
    }
    angle_addr = NULL;

    return 0;
}

static int
angles_set_item(EVSpace_Angles* self, Py_ssize_t index, PyObject* value)
{
    double dbl_value = PyFloat_AsDouble(value);
    if (dbl_value == -1.0 && PyErr_Occurred()) {
        return -1;
    }

    if (index == ROTATION_ANGLE_ALPHA) {
        self->alpha = dbl_value;
    }
    else if (index == ROTATION_ANGLE_BETA) {
        self->beta = dbl_value;
    }
    else if (index == ROTATION_ANGLE_GAMMA) {
        self->gamma = dbl_value;
    }
    else {
        PyErr_Format(PyExc_IndexError,
                     "index (%i) must be in [0-2]",
                     index);
        return -1;
    }

    return 0;
}

static EVSpace_Order*
_order_new(EVSpace_Axis first, EVSpace_Axis second, EVSpace_Axis third,
           PyTypeObject* type)
{
    EVSpace_Order* order = (EVSpace_Order*)type->tp_alloc(type, 0);
    if (!order) {
        return NULL;
    }

    order->first = first;
    order->second = second;
    order->third = third;

    return order;
}

#define new_order(f, s, t)      _order_new(f, s, t, &EVSpace_OrderType)

static PyObject*
order_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED(_))
{
    EVSpace_Axis first, second, third;

    if (PyArg_ParseTuple(args, "iii", (int*)&first, (int*)&second, 
                         (int*)&third) < 0) {
        return NULL;
    }

    // do we need to check the values of enum type?
    return (PyObject*)_order_new(first, second, third, type);
}

static void
__order_axis_names(const EVSpace_Order* order, char* first, char* second, char* third)
{
    if (order->first == X_AXIS) {
        sprintf(first, "X_AXIS");
    }
    else if (order->first == Y_AXIS) {
        sprintf(first, "Y_AXIS");
    }
    else if (order->first == Z_AXIS) {
        sprintf(first, "Z_AXIS");
    }

    if (order->second == X_AXIS) {
        sprintf(second, "X_AXIS");
    }
    else if (order->second == Y_AXIS) {
        sprintf(second, "Y_AXIS");
    }
    else if (order->second == Z_AXIS) {
        sprintf(second, "Z_AXIS");
    }

    if (order->third == X_AXIS) {
        sprintf(third, "X_AXIS");
    }
    else if (order->third == Y_AXIS) {
        sprintf(third, "Y_AXIS");
    }
    else if (order->third == Z_AXIS) {
        sprintf(third, "Z_AXIS");
    }
}

static PyObject*
order_str(const EVSpace_Order* order)
{
    char first[7], second[7], third[7];

    char* buffer = malloc(25);
    if (!buffer) {
        return NULL;
    }

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
    if (!buffer) {
        return NULL;
    }

    __order_axis_names(order, first, second, third);
    sprintf(buffer, "Order(%s, %s, %s)", first, second, third);

    PyObject* rtn = PyUnicode_FromString(buffer);
    free(buffer);
    return rtn;
}

static PyObject*
order_get_item(EVSpace_Order* self, Py_ssize_t index)
{
    EVSpace_Axis rtn;

    if (index == 0) {
        rtn = self->first;
    }
    else if (index == 1) {
        rtn = self->second;
    }
    else if (index == 2) {
        rtn = self->third;
    }
    else {
        PyErr_Format(PyExc_IndexError,
                     "index (%i) must be in [0-2]", 
                     index);
        return NULL;
    }

    return PyLong_FromLong((long)rtn);
}

static int
order_set_item(EVSpace_Order* self, Py_ssize_t index, PyObject* value)
{
    int val = PyLong_AsLong(value);
    if (val == -1.0 && PyErr_Occurred()) {
        return -1;
    }

    if (val < 0 || val > 2) {
        PyErr_SetString(PyExc_ValueError,
                        "value must be pyevspace.X_AXIS, "
                        "pyevspace.Y_AXIS or pyevspace.Z_AXIS");
        return -1;
    }

    if (index == 0) {
        self->first = (EVSpace_Axis)val;
    }
    else if (index == 1) {
        self->second = (EVSpace_Axis)val;
    }
    else if (index == 2) {
        self->third = (EVSpace_Axis)val;
    }
    else {
        PyErr_Format(PyExc_IndexError,
                     "index (%i) must be in [0-2]", 
                     index);
        return -1;
    }

    return 0;
}

static PyObject*
order_reduce(const EVSpace_Order* self, PyObject* Py_UNUSED(_))
{
    return Py_BuildValue("(O(iii))", Py_TYPE(self), self->first, self->second,
                         self->third);
}

#endif // EVSPACE_ANGLES_H