#ifndef EVSPACE_ROTATION_H
#define EVSPACE_ROTATION_H

#include <Python.h>
#include <evspacemodule.h>
#include <evspace_angles.h>
#include <evspace_vector.h>
#include <evspace_matrix.h>

// forward declaration
static PyTypeObject EVSpace_RotationType;

static EVSpace_Matrix*
_get_x_rotation(double angle)
{
	double c_angle = cos(angle);
	double s_angle = sin(angle);

	double* state = calloc(9, sizeof(double));
	if (!state)
		return NULL;

	state[EVSpace_RC_INDEX(0, 0)] = 1.0;
	state[EVSpace_RC_INDEX(1, 1)] = state[EVSpace_RC_INDEX(2, 2)] = c_angle;
	state[EVSpace_RC_INDEX(1, 2)] = -(state[EVSpace_RC_INDEX(2, 1)] = s_angle);

	EVSpace_Matrix* rtn = (EVSpace_Matrix*)new_matrix_steal(state);
	if (!rtn)
		free(state);
	return rtn;
}

static EVSpace_Matrix*
_get_y_rotation(double angle)
{
	double c_angle = cos(angle);
	double s_angle = sin(angle);

	double* state = calloc(9, sizeof(double));
	if (!state)
		return NULL;

	state[EVSpace_RC_INDEX(1, 1)] = 1.0;
	state[EVSpace_RC_INDEX(0, 0)] = state[EVSpace_RC_INDEX(2, 2)] = c_angle;
	state[EVSpace_RC_INDEX(2, 0)] = -(state[EVSpace_RC_INDEX(0, 2)] = s_angle);

	EVSpace_Matrix* rtn = (EVSpace_Matrix*)new_matrix_steal(state);
	if (!rtn)
		free(state);
	return rtn;
}

static EVSpace_Matrix*
_get_z_rotation(double angle)
{
	double c_angle = cos(angle);
	double s_angle = sin(angle);

	double* state = calloc(9, sizeof(double));
	if (!state)
		return NULL;

	state[EVSpace_RC_INDEX(2, 2)] = 1.0;
	state[EVSpace_RC_INDEX(0, 0)] = state[EVSpace_RC_INDEX(1, 1)] = c_angle;
	state[EVSpace_RC_INDEX(0, 1)] = -(state[EVSpace_RC_INDEX(1, 0)] = s_angle);

	EVSpace_Matrix* rtn = (EVSpace_Matrix*)new_matrix_steal(state);
	if (!rtn)
		free(state);
	return rtn;
}

static EVSpace_Matrix*
get_rotation_matrix(EVSpace_Axis axis, double angle)
{
	if (axis == X_AXIS)
		return _get_x_rotation(angle);
	else if (axis == Y_AXIS)
		return _get_y_rotation(angle);
	else if (axis == Z_AXIS)
		return _get_z_rotation(angle);
	else {
		PyErr_Format(PyExc_ValueError,
			"axis value (%i) must be in [0-2]", (int)axis);
		return NULL;
	}
}

static EVSpace_Matrix*
get_euler_matrix(const EVSpace_Order* order, const EVSpace_Angles* angles)
{
	EVSpace_Matrix* first_matrix
		= (EVSpace_Matrix*)get_rotation_matrix(order->first, angles->alpha);
	if (!first_matrix)
		return NULL;

	EVSpace_Matrix* second_matrix
		= (EVSpace_Matrix*)get_rotation_matrix(order->second, angles->beta);
	if (!second_matrix) {
		Py_DECREF(first_matrix);
		return NULL;
	}

	EVSpace_Matrix* third_matrix
		= (EVSpace_Matrix*)get_rotation_matrix(order->third, angles->gamma);
	if (!third_matrix) {
		Py_DECREF(first_matrix);
		Py_DECREF(second_matrix);
		return NULL;
	}

	EVSpace_Matrix* temp
		= (EVSpace_Matrix*)multiply_matrix_matrix(first_matrix, second_matrix);
	Py_DECREF(first_matrix);
	Py_DECREF(second_matrix);
	if (!temp) {
		Py_DECREF(third_matrix);
		return NULL;
	}

	EVSpace_Matrix* rtn
		= (EVSpace_Matrix*)multiply_matrix_matrix(temp, third_matrix);
	Py_DECREF(third_matrix);
	Py_DECREF(temp);

	return rtn;
}


/* constructor */

static PyObject*
new_rotation_ex(const EVSpace_Order* order, const EVSpace_Angles* angles, 
	PyTypeObject* type)
{
	EVSpace_Rotation* rot = (EVSpace_Rotation*)type->tp_alloc(type, 0);
	if (!rot)
		return NULL;

	rot->order = (EVSpace_Order*)order;
	rot->angles = (EVSpace_Angles*)angles;
	rot->matrix = get_euler_matrix(order, angles);

	return (PyObject*)rot;
}

static PyObject*
rotation_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED(_))
{
	PyObject* order = NULL, * angles = NULL;

	if (PyArg_ParseTuple(args, "OO", &order, &angles) < 0)
		return NULL;

	if (!Order_Check(order)) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.order type");
		return NULL;
	}
	if (!Angles_Check(angles)) {
		PyErr_SetString(PyExc_TypeError,
			"second argument must be pyevspace.angle type");
		return NULL;
	}

	return (PyObject*)new_rotation_ex((EVSpace_Order*)order,
		(EVSpace_Angles*)angles, type);
}

static PyObject*
rotation_angles_getter(PyObject* self, void* closure)
{
	return Py_NewRef((PyObject*)(((EVSpace_Rotation*)self)->angles));
}

static int
rotation_angles_setter(EVSpace_Rotation* self, EVSpace_Angles* arg, void* closure)
{
	if (!arg) {
		PyErr_SetString(PyExc_ValueError,
			"cannot delete angles attribute");
		return -1;
	}

	if (!Angles_Check(arg)) {
		PyErr_SetString(PyExc_TypeError,
			"value must be pyevspace.angles type");
		return -1;
	}

	EVSpace_Matrix* matrix
		= (EVSpace_Matrix*)get_euler_matrix(self->order, arg);
	if (!matrix)
		return -1;

	PyObject* tmp = (PyObject*)self->angles;
	Py_INCREF(arg);
	self->angles = arg;
	Py_XDECREF(tmp);

	tmp = (PyObject*)self->matrix;
	Py_INCREF(matrix);
	self->matrix = matrix;
	Py_XDECREF(tmp);

	return 0;
}

#define ROTATION_ANGLE_ALPHA	0
#define ROTATION_ANGLE_BETA		1
#define ROTATION_ANGLE_GAMMA	2

static PyObject*
rotation_subangle_getter(EVSpace_Rotation* self, void* closure)
{
	size_t which = (size_t)closure;
	assert(which == ROTATION_ANGLE_ALPHA || which == ROTATION_ANGLE_BETA
		|| which == ROTATION_ANGLE_GAMMA);

	double angle = 0.0;
	if (which == ROTATION_ANGLE_ALPHA)
		angle = self->angles->alpha;
	else if (which == ROTATION_ANGLE_BETA)
		angle = self->angles->beta;
	else if (which == ROTATION_ANGLE_GAMMA)
		angle = self->angles->gamma;

	return PyFloat_FromDouble(angle);
}

static int
rotation_subangle_setter(EVSpace_Rotation* self, PyObject* arg, void* closure)
{
	printf("in setter\n");
	size_t which = (size_t)closure;
	assert(which == ROTATION_ANGLE_ALPHA || which == ROTATION_ANGLE_BETA
		|| which == ROTATION_ANGLE_GAMMA);

	if (!arg) {
		PyErr_SetString(PyExc_ValueError,
			"cannot delete angles attribute");
		return -1;
	}

	printf("getting angle\n");
	double angle = PyFloat_AsDouble(arg);
	if (angle == -1.0 && PyErr_Occurred())
		return -1;
	printf("got angle\n");

	double* angle_addr = NULL;
	if (which == ROTATION_ANGLE_ALPHA)
		angle_addr = &self->angles->alpha;
	else if (which == ROTATION_ANGLE_BETA)
		angle_addr = &self->angles->beta;
	else if (which == ROTATION_ANGLE_GAMMA)
		angle_addr = &self->angles->gamma;

	double temp = *angle_addr;
	*angle_addr = angle;

	EVSpace_Matrix* matrix = get_euler_matrix(self->order, self->angles);
	if (!matrix) {
		*angle_addr = temp;
		return -1;
	}
	angle_addr = NULL;

	PyObject* tmp = (PyObject*)self->matrix;
	Py_INCREF(matrix);
	self->matrix = matrix;
	Py_XDECREF(tmp);

	return 0;
}


static EVSpace_Matrix*
get_matrix_from_to(const EVSpace_Order* order_from, const EVSpace_Angles* angles_from,
	const EVSpace_Order* order_to, const EVSpace_Angles* angles_to)
{
	EVSpace_Matrix* from = get_euler_matrix(order_from, angles_from);
	if (!from)
		return NULL;

//	EVSpace_Matrix* to = _get_euler_transpose(order_to, angles_to);
	EVSpace_Matrix* to = get_euler_matrix(order_to, angles_to);
	// todo: implement this !!!!!!!!!!!!!!!!!!!!!1
	//_transpose_inplace(to);
	if (!to) {
		Py_DECREF(from);
		return NULL;
	}

	EVSpace_Matrix* rtn = (EVSpace_Matrix*)multiply_matrix_matrix(to, from);
	Py_DECREF(from);
	Py_DECREF(to);
	return rtn;
}



/* rotation methods */

static EVSpace_Vector*
rotate_axis_to(EVSpace_Axis axis, double angle, const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_axis_from(EVSpace_Axis axis, double angle, const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_euler_to(const EVSpace_Order* order, const EVSpace_Angles* angles,
	const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_euler_from(const EVSpace_Order* order, const EVSpace_Angles* angles,
	const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_matrix_to(const EVSpace_Matrix* matrix, const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_matrix_from(const EVSpace_Matrix* matrix, const EVSpace_Vector* vector);



/* offset reference frames */

static EVSpace_Vector*
rotate_then_offset(const EVSpace_Matrix* matrix, const EVSpace_Vector* offset,
	const EVSpace_Vector* vector);

static EVSpace_Vector*
undo_rotate_then_offset(const EVSpace_Matrix* matrix, const EVSpace_Vector* offset,
	const EVSpace_Vector* vector);

#endif // EVSPACE_ROTATION_H
