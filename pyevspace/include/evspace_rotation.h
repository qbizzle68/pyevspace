#ifndef EVSPACE_ROTATION_H
#define EVSPACE_ROTATION_H

#include <Python.h>
#include <evspacemodule.h>
#include <evspace_angles.h>
#include <evspace_vector.h>
#include <evspace_matrix.h>

// forward declaration
static PyTypeObject EVSpace_ReferenceFrameType;

/* methods needed for construction */

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
_get_rotation_matrix(EVSpace_Axis axis, double angle)
{
	if (axis == X_AXIS)
		return _get_x_rotation(angle);
	else if (axis == Y_AXIS)
		return _get_y_rotation(angle);
	else if (axis == Z_AXIS)
		return _get_z_rotation(angle);
	else {
		// todo: want this error to print with format "%_AXIS" with axis value
		PyErr_Format(PyExc_ValueError,
			"axis value (%i) must be in [0-2]", (int)axis);
		return NULL;
	}
}

static EVSpace_Matrix*
_get_euler_matrix(const EVSpace_Order* order, const EVSpace_Angles* angles)
{
	EVSpace_Matrix* first_matrix
		= (EVSpace_Matrix*)_get_rotation_matrix(order->first, angles->alpha);
	if (!first_matrix)
		return NULL;

	EVSpace_Matrix* second_matrix
		= (EVSpace_Matrix*)_get_rotation_matrix(order->second, angles->beta);
	if (!second_matrix) {
		Py_DECREF(first_matrix);
		return NULL;
	}

	EVSpace_Matrix* third_matrix
		= (EVSpace_Matrix*)_get_rotation_matrix(order->third, angles->gamma);
	if (!third_matrix) {
		Py_DECREF(first_matrix);
		Py_DECREF(second_matrix);
		return NULL;
	}

	EVSpace_Matrix* temp
		= (EVSpace_Matrix*)_matrix_multiply_m(first_matrix, second_matrix);
	Py_DECREF(first_matrix);
	Py_DECREF(second_matrix);
	if (!temp) {
		Py_DECREF(third_matrix);
		return NULL;
	}

	EVSpace_Matrix* rtn
		= (EVSpace_Matrix*)_matrix_multiply_m(temp, third_matrix);
	Py_DECREF(third_matrix);
	Py_DECREF(temp);

	return rtn;
}


/* constructor */

static PyObject*
_reference_frame_new(EVSpace_Order* order, EVSpace_Angles* angles, 
	EVSpace_Vector* offset, PyTypeObject* type)
{
	EVSpace_ReferenceFrame* rot = (EVSpace_ReferenceFrame*)type->tp_alloc(type, 0);
	if (!rot)
		return NULL;

	rot->order = order;
	rot->angles = angles;
	rot->matrix = _get_euler_matrix(order, angles);
	rot->offset = offset;

	return (PyObject*)rot;
}

static inline void
__transpose_inplace(double* state)
{
	double temp[3] = { state[EVSpace_RC_INDEX(0, 1)],
		state[EVSpace_RC_INDEX(0, 2)],
		state[EVSpace_RC_INDEX(1, 2)] };

	state[EVSpace_RC_INDEX(0, 1)] = state[EVSpace_RC_INDEX(1, 0)];
	state[EVSpace_RC_INDEX(0, 2)] = state[EVSpace_RC_INDEX(2, 0)];
	state[EVSpace_RC_INDEX(1, 2)] = state[EVSpace_RC_INDEX(2, 1)];
	state[EVSpace_RC_INDEX(1, 0)] = temp[0];
	state[EVSpace_RC_INDEX(2, 0)] = temp[1];
	state[EVSpace_RC_INDEX(2, 1)] = temp[2];
}

static EVSpace_Matrix*
_get_matrix_from_to(const EVSpace_Order* order_from, const EVSpace_Angles* angles_from,
	const EVSpace_Order* order_to, const EVSpace_Angles* angles_to)
{
	EVSpace_Matrix* from = _get_euler_matrix(order_from, angles_from);
	if (!from)
		return NULL;

	EVSpace_Matrix* to = _get_euler_matrix(order_to, angles_to);
	if (!to) {
		Py_DECREF(from);
		return NULL;
	}
	__transpose_inplace(Matrix_DATA(to));

	EVSpace_Matrix* rtn = _matrix_multiply_m(to, from);
	Py_DECREF(from);
	Py_DECREF(to);
	return rtn;
}



/* rotation methods */

static EVSpace_Vector*
_rotate_axis_to(EVSpace_Axis axis, double angle, const EVSpace_Vector* vector)
{
	EVSpace_Matrix* matrix = _get_rotation_matrix(axis, angle);
	if (!matrix)
		return NULL;

	__transpose_inplace(Matrix_DATA(matrix));

	EVSpace_Vector* rtn = _matrix_multiply_v(matrix, vector);
	Py_DECREF(matrix);
	return rtn;
}

static EVSpace_Vector*
_rotate_axis_from(EVSpace_Axis axis, double angle, const EVSpace_Vector* vector)
{
	EVSpace_Matrix* matrix = _get_rotation_matrix(axis, angle);
	if (!matrix)
		return NULL;

	EVSpace_Vector* rtn = _matrix_multiply_v(matrix, vector);
	Py_DECREF(matrix);
	return rtn;
}

static EVSpace_Vector*
_rotate_euler_to(const EVSpace_Order* order, const EVSpace_Angles* angles,
	const EVSpace_Vector* vector)
{
	EVSpace_Matrix* matrix = _get_euler_matrix(order, angles);
	if (!matrix)
		return NULL;

	__transpose_inplace(Matrix_DATA(matrix));

	EVSpace_Vector* rtn = _matrix_multiply_v(matrix, vector);
	Py_DECREF(matrix);
	return rtn;
}

static EVSpace_Vector*
_rotate_euler_from(const EVSpace_Order* order, const EVSpace_Angles* angles,
	const EVSpace_Vector* vector)
{
	EVSpace_Matrix* matrix = _get_euler_matrix(order, angles);
	if (!matrix)
		return NULL;

	EVSpace_Vector* rtn = _matrix_multiply_v(matrix, vector);
	Py_DECREF(matrix);
	return rtn;
}

static EVSpace_Vector*
_rotate_matrix_to(const EVSpace_Matrix* matrix, const EVSpace_Vector* vector)
{
	EVSpace_Matrix* transpose = _matrix_transpose(matrix);
	if (!transpose)
		return (EVSpace_Vector*)PyErr_NoMemory();

	EVSpace_Vector* rtn = _matrix_multiply_v(transpose, vector);
	Py_DECREF(transpose);
	return rtn;
}

static EVSpace_Vector*
_rotate_matrix_from(const EVSpace_Matrix* matrix, const EVSpace_Vector* vector)
{
	return _matrix_multiply_v(matrix, vector);
}



/* offset reference frames */

static EVSpace_Vector*
_rotate_offset_to(const EVSpace_Matrix* matrix, const EVSpace_Vector* offset,
	const EVSpace_Vector* vector)
{
	EVSpace_Matrix* transpose = _matrix_transpose(matrix);
	if (!transpose)
		return NULL;

	EVSpace_Vector* rotated_vector = _matrix_multiply_v(transpose, vector);
	if (!rotated_vector) {
		Py_DECREF(transpose);
		return NULL;
	}

	EVSpace_Vector* rotated_offset = _matrix_multiply_v(transpose, offset);
	if (!rotated_offset) {
		Py_DECREF(transpose);
		Py_DECREF(rotated_vector);
		return NULL;
	}

	EVSpace_Vector* rtn = _vector_subtract(rotated_vector, rotated_offset);
	Py_DECREF(transpose);
	Py_DECREF(rotated_vector);
	Py_DECREF(rotated_offset);
	return rtn;
}

static EVSpace_Vector*
_rotate_offset_from(const EVSpace_Matrix* matrix, const EVSpace_Vector* offset,
	const EVSpace_Vector* vector)
{
	EVSpace_Matrix* transpose = _matrix_transpose(matrix);
	if (!transpose)
		return NULL;

	EVSpace_Vector* rotated_offset = _matrix_multiply_v(transpose, offset);
	Py_DECREF(transpose);
	if (!rotated_offset)
		return NULL;

	EVSpace_Vector* rotated_vector = _vector_add(vector, rotated_offset);
	Py_DECREF(rotated_offset);
	if (!rotated_vector)
		return NULL;

	EVSpace_Vector* rtn = _matrix_multiply_v(matrix, rotated_vector);
	Py_DECREF(rotated_vector);
	return rtn;
}




static PyObject*
reference_frame_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "", "", "offset", NULL };
	PyObject* order = NULL, * angles = NULL, * offset = NULL;

	//if (PyArg_ParseTuple(args, "OO", &order, &angles) < 0)
	if (PyArg_ParseTupleAndKeywords(args, kwds, "OO|O", kwlist, 
									&order, &angles, &offset) < 0)
		return NULL;

	if (!Order_Check(order)) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.Order type");
		return NULL;
	}
	if (!Angles_Check(angles)) {
		PyErr_SetString(PyExc_TypeError,
			"second argument must be pyevspace.Angles type");
		return NULL;
	}

	return (PyObject*)_reference_frame_new((EVSpace_Order*)order,
		(EVSpace_Angles*)angles, (EVSpace_Vector*)offset, type);
}

static PyObject*
reference_frame_angles_getter(PyObject* self, void* closure)
{
	return Py_NewRef((PyObject*)(((EVSpace_ReferenceFrame*)self)->angles));
}

static int
reference_frame_angles_setter(EVSpace_ReferenceFrame* self, EVSpace_Angles* arg, void* closure)
{
	if (!arg) {
		PyErr_SetString(PyExc_ValueError,
			"cannot delete angles attribute");
		return -1;
	}

	if (!Angles_Check(arg)) {
		PyErr_SetString(PyExc_TypeError,
			"value must be pyevspace.Angles type");
		return -1;
	}

	EVSpace_Matrix* matrix
		= (EVSpace_Matrix*)_get_euler_matrix(self->order, arg);
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
reference_frame_subangle_getter(EVSpace_ReferenceFrame* self, void* closure)
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
reference_frame_subangle_setter(EVSpace_ReferenceFrame* self, PyObject* arg, void* closure)
{
	size_t which = (size_t)closure;
	assert(which == ROTATION_ANGLE_ALPHA || which == ROTATION_ANGLE_BETA
		|| which == ROTATION_ANGLE_GAMMA);

	if (!arg) {
		PyErr_SetString(PyExc_ValueError,
			"cannot delete angles attribute");
		return -1;
	}

	double angle = PyFloat_AsDouble(arg);
	if (angle == -1.0 && PyErr_Occurred())
		return -1;

	double* angle_addr = NULL;
	if (which == ROTATION_ANGLE_ALPHA) {
		angle_addr = &self->angles->alpha;
	}
	else if (which == ROTATION_ANGLE_BETA) {
		angle_addr = &self->angles->beta;
	}
	else {
		angle_addr = &self->angles->gamma;
	}

	double temp = *angle_addr;
	*angle_addr = angle;

	EVSpace_Matrix* matrix = _get_euler_matrix(self->order, self->angles);
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

static PyObject*
reference_frame_offset_getter(EVSpace_ReferenceFrame* self, void* Py_UNUSED(_))
{
	return (PyObject*)self->offset;
}

static int
reference_frame_offset_setter(EVSpace_ReferenceFrame* self, PyObject* arg, 
	void* Py_UNUSED(_))
{
	if (!arg) {
		PyErr_SetString(PyExc_ValueError,
			"cannot delete angles attribute");
		return -1;
	}

	if (!Vector_Check(arg)) {
		PyErr_SetString(PyExc_TypeError,
			"value must be pyevspace.Vector type");
		return -1;
	}

	EVSpace_Vector* tmp = self->offset;
	Py_INCREF(arg);
	self->offset = (EVSpace_Vector*)arg;
	Py_DECREF(tmp);

	return 0;
}

static PyObject*
get_rotation_matrix(PyObject* Py_UNUSED(_), PyObject* args)
{
	EVSpace_Axis axis;
	double angle = 0;

	if (PyArg_ParseTuple(args, "id", &(int)axis, &angle) < 0)
		return NULL;

	return (PyObject*)_get_rotation_matrix(axis, angle);
}

static PyObject*
get_euler_matrix(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_Format(PyExc_TypeError,
			"getMatrixEuler() expected exactly 2 arguments \
				(%i given)", size);
		return NULL;
	}

	if (!Order_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.Order type");
		return NULL;
	}
	if (!Angles_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError,
			"second parameter must be pyevspace.Angles type");
		return NULL;
	}

	return (PyObject*)_get_euler_matrix((EVSpace_Order*)args[0],
		(EVSpace_Angles*)args[1]);
}

static PyObject*
get_matrix_from_to(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 4) {
		PyErr_Format(PyExc_TypeError,
			"getMatrixFromTo() expected exactly 2 arguments \
				(%i given)", size);
		return NULL;
	}

	if (!Order_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.Order type");
		return NULL;
	}
	if (!Angles_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError,
			"second argument must be pyevspace.Angles type");
		return NULL;
	}
	if (!Order_Check(args[2])) {
		PyErr_SetString(PyExc_TypeError,
			"third argument must be pyevspace.Order type");
		return NULL;
	}
	if (!Angles_Check(args[3])) {
		PyErr_SetString(PyExc_TypeError,
			"fourth argument must be pyevspace.Angles type");
		return NULL;
	}

	return (PyObject*)_get_matrix_from_to((EVSpace_Order*)args[0],
		(EVSpace_Angles*)args[1],
		(EVSpace_Order*)args[2],
		(EVSpace_Angles*)args[3]);
}

static PyObject*
rotate_axis_to(PyObject* Py_UNUSED(_), PyObject* args)
{
	EVSpace_Axis axis = 0;
	double angle = 0;
	EVSpace_Vector* vector;

	if (!PyArg_ParseTuple(args, "idO", &axis, &angle, &vector))
		return NULL;

	return (PyObject*)_rotate_axis_to(axis, angle, vector);
}

static PyObject*
rotate_axis_from(PyObject* Py_UNUSED(_), PyObject* args)
{
	EVSpace_Axis axis = 0;
	double angle = 0;
	EVSpace_Vector* vector;

	if (!PyArg_ParseTuple(args, "idO", &axis, &angle, &vector))
		return NULL;

	return (PyObject*)_rotate_axis_from(axis, angle, vector);
}

static PyObject*
rotate_euler_to(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 3) {
		PyErr_Format(PyExc_TypeError,
			"rotateEulerTo() expected exactly 3 arguments \
			(%i given)", size);
		return NULL;
	}

	if (!Order_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.Order type");
		return NULL;
	}
	if (!Angles_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError,
			"second argument must be pyevspace.Angles type");
		return NULL;
	}
	if (!Vector_Check(args[2])) {
		PyErr_SetString(PyExc_TypeError,
			"third argument must be pyevspace.Vector type");
		return NULL;
	}

	return (PyObject*)_rotate_euler_to((EVSpace_Order*)args[0], 
									   (EVSpace_Angles*)args[1], 
									   (EVSpace_Vector*)args[2]);
}

static PyObject*
rotate_euler_from(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 3) {
		PyErr_Format(PyExc_TypeError,
			"rotateEulerFrom() expected exactly 3 arguments \
			(%i given)", size);
		return NULL;
	}

	if (!Order_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.Order type");
		return NULL;
	}
	if (!Angles_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError,
			"second argument must be pyevspace.Angles type");
		return NULL;
	}
	if (!Vector_Check(args[2])) {
		PyErr_SetString(PyExc_TypeError,
			"third argument must be pyevspace.Vector type");
		return NULL;
	}

	return (PyObject*)_rotate_euler_from((EVSpace_Order*)args[0],
										 (EVSpace_Angles*)args[1],
										 (EVSpace_Vector*)args[2]);
}

static PyObject*
rotate_matrix_to(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_Format(PyExc_TypeError,
			"rotateMatrixTo() expected exactly 2 arguments \
			(%i given)", size);
		return NULL;
	}

	if (!Matrix_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first parameter must be pyevspace.Matrix type");
		return NULL;
	}
	if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError,
			"second parameter must be pyevspace.Vector type");
		return NULL;
	}

	return (PyObject*)_rotate_matrix_to((EVSpace_Matrix*)args[0], 
										(EVSpace_Vector*)args[1]);
}

static PyObject*
rotate_matrix_from(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 2) {
		PyErr_Format(PyExc_TypeError,
			"rotateMatrixTo() expected exactly 2 arguments \
			(%i given)", size);
		return NULL;
	}

	if (!Matrix_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.Matrix type");
		return NULL;
	}
	if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError,
			"second argument must be pyevspace.Vector type");
		return NULL;
	}

	return (PyObject*)_rotate_matrix_from((EVSpace_Matrix*)args[0], 
										  (EVSpace_Vector*)args[1]);
}

static PyObject*
rotate_offset_to(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 3) {
		PyErr_Format(PyExc_TypeError,
			"rotateOffsetTo() expected exactly 3 arguments \
			(%i given)", size);
		return NULL;
	}

	if (!Matrix_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.Matrix type");
		return NULL;
	}
	if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError,
			"second argument must be pyevspace.Vector type");
		return NULL;
	}
	if (!Vector_Check(args[2])) {
		PyErr_SetString(PyExc_TypeError,
			"third argument must be pyevspace.Vector type");
		return NULL;
	}

	return (PyObject*)_rotate_offset_to((EVSpace_Matrix*)args[0],
										(EVSpace_Vector*)args[1],
										(EVSpace_Vector*)args[2]);
}

static PyObject*
rotate_offset_from(PyObject* Py_UNUSED(_), PyObject* const* args, Py_ssize_t size)
{
	if (size != 3) {
		PyErr_Format(PyExc_TypeError,
			"rotateOffsetFrom() expected exactly 3 arguments \
			(%i given)", size);
		return NULL;
	}

	if (!Matrix_Check(args[0])) {
		PyErr_SetString(PyExc_TypeError,
			"first argument must be pyevspace.Matrix type");
		return NULL;
	}
	if (!Vector_Check(args[1])) {
		PyErr_SetString(PyExc_TypeError,
			"second argument must be pyevspace.Vector type");
		return NULL;
	}
	if (!Vector_Check(args[2])) {
		PyErr_SetString(PyExc_TypeError,
			"third argument must be pyevspace.Vector type");
		return NULL;
	}

	return (PyObject*)_rotate_offset_from((EVSpace_Matrix*)args[0],
										  (EVSpace_Vector*)args[1],
										  (EVSpace_Vector*)args[2]);
}

static PyObject*
reference_frame_rotate_to(EVSpace_ReferenceFrame* self, PyObject* vector)
{
	if (!Vector_Check(vector))
	{
		PyErr_SetString(PyExc_TypeError,
			"argument must be pyevspace.Vector type");
		return NULL;
	}

	if (self->offset) {
		return (PyObject*)_rotate_offset_to(self->matrix, self->offset, 
											(EVSpace_Vector*)vector);
	}
	else {
		return (PyObject*)_rotate_matrix_to(self->matrix, 
											(EVSpace_Vector*)vector);
		/*EVSpace_Matrix* transpose = _matrix_transpose(self->matrix);
		if (!transpose)
			return NULL;

		EVSpace_Vector* rtn = _matrix_multiply_v(transpose,
												 (EVSpace_Vector*)self);
		Py_DECREF(transpose);
		return (PyObject*)rtn;*/
	}
}

static PyObject*
reference_frame_rotate_from(EVSpace_ReferenceFrame* self, PyObject* vector)
{
	if (!Vector_Check(vector))
	{
		PyErr_SetString(PyExc_TypeError,
			"argument must be pyevspace.Vector type");
		return NULL;
	}

	if (self->offset) {
		return (PyObject*)_rotate_offset_from(self->matrix, self->offset, 
											  (EVSpace_Vector*)vector);
	}
	else {
		return (PyObject*)_matrix_multiply_v(self->matrix,
											 (EVSpace_Vector*)vector);
	}

	//EVSpace_Vector* rtn = _matrix_multiply_v(self->matrix,
											 //(EVSpace_Vector*)self);
	//return (PyObject*)rtn;
}

#endif // EVSPACE_ROTATION_H
