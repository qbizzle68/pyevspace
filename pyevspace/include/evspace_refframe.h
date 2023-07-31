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


static EVSpace_ReferenceFrame*
_reference_frame_new(EVSpace_Order* order, EVSpace_Angles* angles, 
    EVSpace_Vector* offset, PyTypeObject* type)
{
    EVSpace_ReferenceFrame* rot = (EVSpace_ReferenceFrame*)type->tp_alloc(type, 0);
    if (!rot) {
        return NULL;
    }

    rot->matrix = _get_euler_matrix(order, angles);
    if (!rot->matrix) {
        return NULL;
    }

    // Create a copy of the angles type to ensure each EVSpace_Angles type has a single master.
    EVSpace_Angles* tmp = new_angle(angles->alpha, angles->beta, angles->gamma);
    if (!tmp) {
        Py_DECREF(rot->matrix);
        return NULL;
    }
    rot->angles = tmp;
    rot->angles->master = rot;

    rot->order = order;
    Py_INCREF(rot->order);
    rot->offset = offset;
    Py_XINCREF(rot->offset);

    return rot;
}

static PyObject*
refframe_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    static char* kwlist[] = { "", "", "offset", NULL };
    PyObject* order = NULL, * angles = NULL, * offset = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|$O", kwlist, 
                                     &order, &angles, &offset))
    {
        return NULL;
    }

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
                                           (EVSpace_Angles*)angles,
                                           (EVSpace_Vector*)offset, type);
}

static PyObject*
refframe_angles_getter(EVSpace_ReferenceFrame* self, void* closure)
{
#if PY_VERSION_HEX >= 0x03100000
    return Py_NewRef(self->angles);
#else
    Py_INCREF(self->angles);
    return (PyObject*)self->angles;
#endif
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

    EVSpace_Matrix*
    matrix = (EVSpace_Matrix*)_get_euler_matrix(self->order, arg);
    if (!matrix) {
        return -1;
    }

    // Copy the angles to enforce self is the instance's only master.
    EVSpace_Angles* angs_copy = new_angle(arg->alpha, arg->beta, arg->gamma);
    if (!angs_copy) {
        Py_DECREF(matrix);
        return -1;
    }

    self->angles->master = NULL;
    PyObject* tmp = (PyObject*)self->angles;
    self->angles = angs_copy;
    Py_XDECREF(tmp);

    tmp = (PyObject*)self->matrix;
    self->matrix = matrix;
    Py_XDECREF(tmp);

    return 0;
}

static PyObject*
refframe_offset_getter(EVSpace_ReferenceFrame* self, void* Py_UNUSED(_))
{
    if (self->offset) {
#if PY_VERSION_HEX >= 0x03100000
        return Py_NewRef(self->offset);
#else
        if (self->offset) {
            Py_INCREF(self->offset);
            return (PyObject*)self->offset;
        }
        else {
            Py_INCREF(Py_None);
            return Py_None;
        }
#endif
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

static EVSpace_Vector*
_refframe_rotate_to(const EVSpace_ReferenceFrame* frame,
                    const EVSpace_Vector* vector)
{
    if (frame->offset) {
        return _rotate_offset_to(frame->matrix, frame->offset, vector);
    }
    else {
        return _rotate_matrix_to(frame->matrix, vector);
    }
}

static EVSpace_Vector*
_refframe_rotate_from(const EVSpace_ReferenceFrame* frame,
                      const EVSpace_Vector* vector)
{
    if (frame->offset) {
        return _rotate_offset_from(frame->matrix, frame->offset, vector);
    }
    else {
        return _matrix_multiply_v(frame->matrix, vector);
    }
}

static PyObject*
refframe_rotate_to(EVSpace_ReferenceFrame* self, PyObject* vector)
{
    if (!Vector_Check(vector)) {
        PyErr_SetString(PyExc_TypeError,
                        "argument must be pyevspace.Vector type");
        return NULL;
    }

    return (PyObject*)_refframe_rotate_to(self, (EVSpace_Vector*)vector);
}

static PyObject*
refframe_rotate_from(EVSpace_ReferenceFrame* self, PyObject* vector)
{
    if (!Vector_Check(vector)) {
        PyErr_SetString(PyExc_TypeError,
                        "argument must be pyevspace.Vector type");
        return NULL;
    }

    return (PyObject*)_refframe_rotate_from(self, (EVSpace_Vector*)vector);
}

static EVSpace_Vector*
_refframe_to_frame(const EVSpace_ReferenceFrame* self,
                   const EVSpace_ReferenceFrame* frame,
                   const EVSpace_Vector* vector)
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
                     const EVSpace_ReferenceFrame* frame,
                     const EVSpace_Vector* vector)
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

    return (PyObject*)_refframe_to_frame(self,
                                         (EVSpace_ReferenceFrame*)args[0],
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

    return (PyObject*)_refframe_from_frame(self,
                                           (EVSpace_ReferenceFrame*)args[0],
                                           (EVSpace_Vector*)args[1]);
}

#endif // EVSPACE_ROTATION_H