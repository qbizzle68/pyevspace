#ifndef EVSPACE_ROTATION_H
#define EVSPACE_ROTATION_H

#include <Python.h>
#include <evspacemodule.h>
#include <evspace_angles.h>
#include <evspace_vector.h>
#include <evspace_matrix.h>
#include <evspace_rotation.h>

// forward declaration
static PyTypeObject EVSpace_ReferenceFrameType;


static PyObject*
_reference_frame_new(EVSpace_Order* order, EVSpace_Angles* angles, 
    EVSpace_Vector* offset, PyTypeObject* type)
{
    EVSpace_ReferenceFrame* rot = (EVSpace_ReferenceFrame*)type->tp_alloc(type, 0);
    if (!rot)
        return NULL;

    rot->matrix = _get_euler_matrix(order, angles);
    if (!rot->matrix) {
        return NULL;
    }
    //Py_INCREF(rot->matrix);
    rot->order = order;
    Py_INCREF(rot->order);
    rot->angles = angles;
    Py_INCREF(rot->angles);
    rot->offset = offset;
    Py_XINCREF(rot->offset);

    return (PyObject*)rot;
}

static PyObject*
refframe_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    static char* kwlist[] = { "", "", "offset", NULL };
    PyObject* order = NULL, * angles = NULL, * offset = NULL;

    //if (PyArg_ParseTuple(args, "OO", &order, &angles) < 0)
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|$O", kwlist, 
                                    &order, &angles, &offset))
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
    if (offset) {
        if (!Vector_Check(offset)) {
            PyErr_SetString(PyExc_TypeError,
                "offset argument must be pyevspace.Vector type");
            return NULL;
        }
    }

    return (PyObject*)_reference_frame_new((EVSpace_Order*)order,
        (EVSpace_Angles*)angles, (EVSpace_Vector*)offset, type);
}

static PyObject*
refframe_angles_getter(EVSpace_ReferenceFrame* self, void* closure)
{
    return Py_NewRef(self->angles);
}

static int
refframe_angles_setter(EVSpace_ReferenceFrame* self, EVSpace_Angles* arg, 
    void* closure)
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
refframe_subangle_getter(EVSpace_ReferenceFrame* self, void* closure)
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
refframe_subangle_setter(EVSpace_ReferenceFrame* self, PyObject* arg, 
    void* closure)
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
refframe_offset_getter(EVSpace_ReferenceFrame* self, void* Py_UNUSED(_))
{
    if (self->offset) {
        return Py_NewRef(self->offset);
    }
    else {
        Py_RETURN_NONE;
    }
}

static int
refframe_offset_setter(EVSpace_ReferenceFrame* self, PyObject* arg, 
    void* Py_UNUSED(_))
{
    if (arg && !Vector_Check(arg)) {
        PyErr_SetString(PyExc_TypeError,
            "value must be pyevspace.Vector type");
        return -1;
    }

    EVSpace_Vector* tmp = self->offset;
    Py_XINCREF(arg);
    self->offset = (EVSpace_Vector*)arg;
    Py_XDECREF(tmp);

    return 0;
}

static PyObject*
refframe_rotate_to(EVSpace_ReferenceFrame* self, PyObject* vector)
{
    if (!Vector_Check(vector)) {
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
    }
}

static PyObject*
refframe_rotate_from(EVSpace_ReferenceFrame* self, PyObject* vector)
{
    if (!Vector_Check(vector)) {
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
}

static EVSpace_Vector*
_refframe_to_frame(const EVSpace_ReferenceFrame* self, 
    const EVSpace_ReferenceFrame* frame, const EVSpace_Vector* vector)
{
    EVSpace_Vector *tmp, *rtn;

    if (self->offset) {
        tmp = _rotate_offset_from(self->matrix, self->offset, vector);
    }
    else {
        tmp = _rotate_matrix_from(self->matrix, vector);
    }

    if (!tmp) {
        return NULL;
    }

    if (frame->offset) {
        rtn = _rotate_offset_to(frame->matrix, frame->offset, tmp);
    }
    else {
        rtn = _rotate_matrix_to(frame->matrix, tmp);
    }

    Py_DECREF(tmp);
    return rtn;
}

static EVSpace_Vector*
_refframe_from_frame(const EVSpace_ReferenceFrame* self,
    const EVSpace_ReferenceFrame* frame, const EVSpace_Vector* vector)
{
    EVSpace_Vector *tmp, *rtn;

    if (frame->offset) {
        tmp = _rotate_offset_from(frame->matrix, frame->offset, vector);
    }
    else {
        tmp = _rotate_matrix_from(frame->matrix, vector);
    }

    if (!tmp) {
        return NULL;
    }

    if (self->offset) {
        rtn = _rotate_offset_to(self->matrix, self->offset, tmp);
    }
    else {
        rtn = _rotate_matrix_to(self->matrix, tmp);
    }

    Py_DECREF(tmp);
    return rtn;
}

static PyObject*
refframe_to_frame(EVSpace_ReferenceFrame* self, PyObject* const* args,
    Py_ssize_t size)
{
    if (size != 2) {
        PyErr_Format(PyExc_TypeError,
            "rotateToFrame() expected exactly 2 arguments (%i given)",
            size);
        return NULL;
    }

    if (!ReferenceFrame_Check(args[0])) {
        PyErr_SetString(PyExc_TypeError,
            "first argument must be pyevspace.ReferenceFrame type");
        return NULL;
    }
    if (!Vector_Check(args[1])) {
        PyErr_SetString(PyExc_TypeError,
            "second argument must be pyevspace.Vector type");
        return NULL;
    }

    return (PyObject*)_refframe_to_frame(self, (EVSpace_ReferenceFrame*)args[0], 
        (EVSpace_Vector*)args[1]);
}

static PyObject*
refframe_from_frame(EVSpace_ReferenceFrame* self, PyObject* const* args,
    Py_ssize_t size)
{
    if (size != 2) {
        PyErr_Format(PyExc_TypeError,
            "rotateFromFrame() expected exactly 2 arguments (%i given)",
            size);
        return NULL;
    }

    if (!ReferenceFrame_Check(args[0])) {
        PyErr_SetString(PyExc_TypeError,
            "first argument must be pyevspace.ReferenceFrame type");
        return NULL;
    }
    if (!Vector_Check(args[1])) {
        PyErr_SetString(PyExc_TypeError,
            "second argument must be pyevspace.Vector type");
        return NULL;
    }

    return (PyObject*)_refframe_from_frame(self, (EVSpace_ReferenceFrame*)args[0], 
        (EVSpace_Vector*)args[1]);
}

#endif // EVSPACE_ROTATION_H
