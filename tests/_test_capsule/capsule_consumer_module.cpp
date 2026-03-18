#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define PYEVSPACE_INCLUDE_HEADERS
#include <pyevspace-api.hpp>
#include <evspace.hpp>

// These functions will take an argument equal to the type the function
// is testing. Then test various capsule and helper methods to validate
// they're functional, raising AssertionError when something fails for
// ideal integration with pytest. Then create several instances using 
// various capsule and helper methods and return them to be testing
// within Python.

static PyObject*
Test_VectorCapsule(PyObject* Py_UNUSED(), PyObject* args)
{
    // Capsule functions to test:
    //      PyEVSpaceVector_GetState
    //      PyEVSpaceVector_FromState
    //      PyEVSpaceVector_SetState
    // C++ helpers to test:
    //      PyEVSpace_ToVector
    //      PyEVSpaceVector_ToObject

    PyObject* vector, *vector_from_state = NULL, *vector_set_state = NULL,
        *vector_object = NULL, *rtn = NULL;
    double state[3];
    double tmp[3] = {0.0, 0.0, 0.0};

    if (!PyArg_ParseTuple(args, "O", &vector)) {
        return NULL;
    }

    // test type
    switch (PyObject_IsInstance(vector, (PyObject*)(PyEVSpace_API->Vector_Type)))
    {
        case -1: return NULL;
        case 0:
            PyErr_SetString(PyExc_AssertionError, "argument is not Vector type");
            return NULL;
        case 1: ;
    }

    // test C++ helper functions
    evspace::Vector v{1, 2, 3};
    evspace::Vector to_vector;
    if (PyEVSpace_ToVector(vector, to_vector) < 0) {
        return NULL;
    }
    if (v != to_vector) {
        PyErr_SetString(PyExc_AssertionError, "PyObject to evspace::Vector equivalence failed");
        return NULL;
    }
    vector_object = PyEVSpaceVector_ToObject(v);

    // test capsule functions
    if (PyEVSpace_API->PyEVSpaceVector_GetState(vector, state) < 0) {
        goto error;
    }
    for (int i = 0; i < 3; i++)
    {
        if (state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "vector state invalid at index %i", i);
            goto error;
        }
    }

    vector_from_state = PyEVSpace_API->PyEVSpaceVector_FromState(state);

    vector_set_state = PyEVSpace_API->PyEVSpaceVector_FromState(tmp);
    if (!vector_set_state) {
        goto error;
    }

    for (int i = 0; i < 3; i++) tmp[i] = i + 1;
    if (PyEVSpace_API->PyEVSpaceVector_SetState(vector_set_state, tmp) < 0) {
        goto error;
    }

    rtn = PyTuple_Pack(3, vector_from_state, vector_set_state, vector_object);
    Py_XDECREF(vector_from_state);
    Py_XDECREF(vector_set_state);
    Py_XDECREF(vector_object);

    return rtn;

error:
    Py_XDECREF(vector_from_state);
    Py_XDECREF(vector_set_state);
    Py_XDECREF(vector_object);
    return NULL;
}

static PyObject*
Test_MatrixCapsule(PyObject* Py_UNUSED(), PyObject* args)
{
    // Capsule functions to test:
    //      PyEVSpaceMatrix_GetState
    //      PyEVSpaceMatrix_FromState
    //      PyEVSPaceMatrix_SetState
    // C++ helpers to test:
    //      PyEVSpace_ToMatrix
    //      PyEVSpaceMatrix_ToObject

    PyObject* matrix, *matrix_from_state = NULL, *matrix_set_state = NULL,
        *matrix_object = NULL, *rtn = NULL;
    double state[9];
    double tmp[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (!PyArg_ParseTuple(args, "O", &matrix)) {
        return NULL;
    }

    // test types
    switch (PyObject_IsInstance(matrix, (PyObject*)PyEVSpace_API->Matrix_Type))
    {
        case -1: return NULL;
        case 0:
            PyErr_SetString(PyExc_AssertionError, "argument is not Matrix type");
        case 1: ;
    }

    // test helper functions
    evspace::Matrix m{1, 2, 3, 4, 5, 6, 7, 8, 9};
    evspace::Matrix to_matrix;
    if (PyEVSpace_ToMatrix(matrix, to_matrix) < 0) {
        return NULL;
    }
    if (m != to_matrix) {
        PyErr_SetString(PyExc_AssertionError, "PyObject to evspace::Matrix equivalence failed");
        return NULL;
    }
    matrix_object = PyEVSpaceMatrix_ToObject(m);

    // test capsule functions
    if (PyEVSpace_API->PyEVSpaceMatrix_GetState(matrix, state) < 0) {
        goto error;
    }
    for (int i = 0; i < 9; i++)
    {
        if (state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "matrix state invalid at index %i", i);
            goto error;
        }
    }

    matrix_from_state = PyEVSpace_API->PyEVSpaceMatrix_FromState(state);

    matrix_set_state = PyEVSpace_API->PyEVSpaceMatrix_FromState(tmp);
    if (!matrix_set_state) {
        goto error;
    }

    for (int i = 0; i < 9; i++) tmp[i] = i + 1;
    if (PyEVSpace_API->PyEVSpaceMatrix_SetState(matrix_set_state, tmp) < 0) {
        goto error;
    }

    rtn = PyTuple_Pack(3, matrix_from_state, matrix_set_state, matrix_object);
    Py_XDECREF(matrix_from_state);
    Py_XDECREF(matrix_set_state);
    Py_XDECREF(matrix_object);

    return rtn;
    
error:
    Py_XDECREF(matrix_from_state);
    Py_XDECREF(matrix_set_state);
    Py_XDECREF(matrix_object);
    return NULL;
}

static PyObject*
Test_AnglesCapsule(PyObject* Py_UNUSED(), PyObject* args)
{
    // Capsule functions to test:
    //      PyEVSpaceAngles_GetState
    //      PyEVSpaceAngles_FromState
    //      PyEVSpaceAngles_SetState

    PyObject* angles, *angles_from_state = NULL, *angles_set_state = NULL,
            *angles_object = NULL, *rtn = NULL;
    double state[3];
    double tmp[3] = {0, 0, 0};

    if (!PyArg_ParseTuple(args, "O", &angles)) {
        return NULL;
    }

    // test type
    switch (PyObject_IsInstance(angles, (PyObject*)PyEVSpace_API->EulerAngles_Type))
    {
        case -1: return NULL;
        case 0:
            PyErr_SetString(PyExc_AssertionError, "argument is not EulerAngles type");
            return NULL;
        case 1: ;
    }

    // test helper functions
    evspace::EulerAngles a{1, 2, 3};
    evspace::EulerAngles to_angles;
    if (PyEVSpace_ToAngles(angles, to_angles) < 0) {
        return NULL;
    }
    if (a[0] != to_angles[0] && a[1] != to_angles[1] && a[2] != to_angles[2]) {
        PyErr_SetString(PyExc_AssertionError, "PyObject to evspace::EulerAngles equivalence failed");
        return NULL;
    }
    angles_object = PyEVSpaceAngles_ToObject(a);

    // test capsule functions
    if (PyEVSpace_API->PyEVSpaceAngles_GetState(angles, state) < 0) {
        goto error;
    }
    for (int i = 0; i < 3; i++)
    {
        if (state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "angles state invalid at index %i", i);
            goto error;
        }
    }

    angles_from_state = PyEVSpace_API->PyEVSpaceAngles_FromState(state);

    angles_set_state = PyEVSpace_API->PyEVSpaceAngles_FromState(tmp);
    if (!angles_set_state) {
        goto error;
    }

    for (int i = 0; i < 3; i++) tmp[i] = i + 1;
    if (PyEVSpace_API->PyEVSpaceAngles_SetState(angles_set_state, tmp) < 0) {
        goto error;
    }

    rtn = PyTuple_Pack(3, angles_from_state, angles_set_state, angles_object);
    Py_XDECREF(angles_from_state);
    Py_XDECREF(angles_set_state);
    Py_XDECREF(angles_object);

    return rtn;

error:
    Py_XDECREF(angles_from_state);
    Py_XDECREF(angles_set_state);
    Py_XDECREF(angles_object);
    return NULL;
}

static PyObject*
Test_OrderCapsule(PyObject* Py_UNUSED(), PyObject* args)
{
    // Capsule functions to test:
    //      PyEVSpaceOrder_GetState
    //      PyEVSpaceOrder_FromState

    PyObject* order, *order_from_state = NULL, *rtn = NULL;
    unsigned int state[3];

    if (!PyArg_ParseTuple(args, "O", &order)) {
        return NULL;
    }

    // test type
    switch (PyObject_IsInstance(order, (PyObject*)PyEVSpace_API->RotationOrder_Type))
    {
        case -1: return NULL;
        case 0:
            PyErr_SetString(PyExc_AssertionError, "argument is not RotationOrder type");
            return NULL;
        case 1: ;
    }

    // test capsule functions
    if (PyEVSpace_API->PyEVSpaceOrder_GetState(order, state) < 0) {
        return NULL;
    }
    for (unsigned int i = 0; i < 3; i++)
    {
        if (state[i] != i)
        {
            PyErr_Format(PyExc_AssertionError, "order state invalid at index %i", i);
            return NULL;
        }
    }
    
    order_from_state = PyEVSpace_API->PyEVSpaceOrder_FromState(state);

    rtn = PyTuple_Pack(1, order_from_state);
    Py_XDECREF(order_from_state);

    return rtn;
}

static inline int
_Test_FrameCapsuleGetters(PyObject* frame, PyObject* frame_offset)
{
    PyObject* order, *angles, *offset;
    unsigned int order_state[3];
    double angles_state[3], offset_state[3];
    int intrinsic_state;

    // frame with no offset
    if (PyEVSpace_API->PyEVSpaceFrame_GetState(frame, order_state, angles_state,
                                             offset_state, &intrinsic_state) < 0) {
        return -1;
    }
    // order checks
    for (unsigned int i = 0; i < 3; i++)
    {
        if (order_state[i] != i) {
            PyErr_Format(PyExc_AssertionError, "order state (no offset) invalid at index %i", i);
            return -1;
        }
    }
    order = PyEVSpace_API->PyEVSpaceFrame_GetOrder(frame);
    if (!order) {
        return -1;
    }
    if (PyEVSpace_API->PyEVSpaceOrder_GetState(order, order_state) < 0) {
        return -1;
    }
    for (unsigned int i = 0; i < 3; i++)
    {
        if (order_state[i] != i) {
            PyErr_Format(PyExc_AssertionError, "get order state (no offset) invalid at index %i", i);
            return -1;
        }
    }
    // angle checks
    for (int i = 0; i < 3; i++)
    {
        if (angles_state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "angles state (no offset) invalid at index %i", i);
            return -1;
        }
    }
    angles = PyEVSpace_API->PyEVSpaceFrame_GetAngles(frame);
    if (!angles) {
        return -1;
    }
    if (PyEVSpace_API->PyEVSpaceAngles_GetState(angles, angles_state) < 0) {
        return -1;
    }
    for (int i = 0; i < 3; i++)
    {
        if (angles_state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "get angles state (no offset) invalid at index %i", i);
            return -1;
        }
    }
    // offset checks
    for (int i = 0; i < 3; i++)
    {
        if (offset_state[i] != 0)
        {
            PyErr_Format(PyExc_AssertionError, "offset state (no offset) invalid at index %i", i);
            return -1;
        }
    }
    offset = PyEVSpace_API->PyEVSpaceFrame_GetOffset(frame);
#if PY_VERSION_HEX >= 0x030a0000
    if (!Py_IsNone(offset))
#else
    if (offset != Py_None)
#endif
    {
        PyErr_SetString(PyExc_AssertionError, "get offset (no offset) is not None");
        return -1;
    }
    // check intrinsic
    if (intrinsic_state != 1)
    {
        PyErr_SetString(PyExc_AssertionError, "intrinsic state (no offset) is not True");
        return -1;
    }
    switch (PyEVSpace_API->PyEVSpaceFrame_GetIntrinsic(frame))
    {
        case -1: return -1;
        case 0:
            PyErr_SetString(PyExc_AssertionError, "get intrinsic state (no offset) is not True");
            return -1;
        case 1: ;
    }

    // with offset
    if (PyEVSpace_API->PyEVSpaceFrame_GetState(frame_offset, order_state, angles_state,
                                             offset_state, &intrinsic_state) < 0) {
        return -1;
    }
    // order checks
    for (unsigned int i = 0; i < 3; i++)
    {
        if (order_state[i] != i) {
            PyErr_Format(PyExc_AssertionError, "order state (offset) invalid at index %i", i);
            return -1;
        }
    }
    order = PyEVSpace_API->PyEVSpaceFrame_GetOrder(frame_offset);
    if (!order) {
        return -1;
    }
    if (PyEVSpace_API->PyEVSpaceOrder_GetState(order, order_state) < 0) {
        return -1;
    }
    for (unsigned int i = 0; i < 3; i++)
    {
        if (order_state[i] != i) {
            PyErr_Format(PyExc_AssertionError, "get order state (offset) invalid at index %i", i);
            return -1;
        }
    }
    // angle checks
    for (int i = 0; i < 3; i++)
    {
        if (angles_state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "angles state (offset) invalid at index %i", i);
            return -1;
        }
    }
    angles = PyEVSpace_API->PyEVSpaceFrame_GetAngles(frame_offset);
    if (!angles) {
        return -1;
    }
    if (PyEVSpace_API->PyEVSpaceAngles_GetState(angles, angles_state) < 0) {
        return -1;
    }
    for (int i = 0; i < 3; i++)
    {
        if (angles_state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "get angles state (offset) invalid at index %i", i);
            return -1;
        }
    }
    // offset checks
    for (int i = 0; i < 3; i++)
    {
        if (offset_state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "offset state (offset) invalid at index %i", i);
            return -1;
        }
    }
    offset = PyEVSpace_API->PyEVSpaceFrame_GetOffset(frame_offset);
    if (PyEVSpace_API->PyEVSpaceVector_GetState(offset, offset_state) < 0)
    {
        return -1;
    }
    for (int i = 0; i < 3; i++)
    {
        if (offset_state[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "get offset (offset) invalid at index %i", i);
            return -1;
        }
    }
    // check intrinsic
    if (intrinsic_state != 0)
    {
        PyErr_SetString(PyExc_AssertionError, "intrinsic state (offset) is not False");
        return -1;
    }
    switch (PyEVSpace_API->PyEVSpaceFrame_GetIntrinsic(frame_offset))
    {
        case -1: return -1;
        case 0: break;
        case 1:
            PyErr_SetString(PyExc_AssertionError, "get intrinsic state (offset) is not False");
            return -1;
    }

    return 0;
}

static inline PyObject*
Test_FrameCapsule(PyObject* Py_UNUSED(_), PyObject* args)
{
    // Capsule functions to test:
    //      PyEVSpaceFrame_GetState
    //      PyEVSpaceFrame_GetOrder
    //      PyEVSpaceFrame_GetAngles
    //      PyEVSpaceFrame_GetOffset
    //      PyEVSpaceFrame_FromState
    //      PyEVSpaceFrame_SetAngles
    //      PyEVSpaceFrame_SetOffset
    // C++ helper functions to test:
    //      PyEVSpaceFrame_ToAngles
    //      PyEVSpaceFrame_ToOffset
    //      PyEVSpaceFrame_SetAngles
    //      PyEVSpaceFrame_SetOffset

    PyObject* frame, *frame_offset, *frame_from = NULL, *frame_from_offset = NULL,
        *frame_set = NULL, *frame_set_offset = NULL, *frame_set_cpp = NULL, *rtn = NULL;
    unsigned int order_state[3]{0, 1, 2};
    double angles_state[3]{1, 2, 3}, offset_state[3]{1, 2, 3}, tmp[3] = {0.0};
    evspace::EulerAngles angles;
    evspace::Vector offset;

    if (!PyArg_ParseTuple(args, "OO", &frame, &frame_offset)) {
        return NULL;
    }

    // test type
    switch (PyObject_IsInstance(frame, (PyObject*)PyEVSpace_API->ReferenceFrame_Type))
    {
        case -1: return NULL;
        case 0:
            PyErr_SetString(PyExc_TypeError, "argument 1 is not RotationOrder type");
            return NULL;
        case 1: ;
    }
    switch (PyObject_IsInstance(frame_offset, (PyObject*)PyEVSpace_API->ReferenceFrame_Type))
    {
        case -1: return NULL;
        case 0:
            PyErr_SetString(PyExc_TypeError, "argument 2 is not RotationOrder type");
            return NULL;
        case 1: ;
    }

    // test capsule getters
    if (_Test_FrameCapsuleGetters(frame, frame_offset) < 0) {
        return NULL;
    }

    // test helpers
    if (PyEVSpaceFrame_ToAngles(frame, angles) < 0) {
        return NULL;
    }
    for (int i = 0; i < 3; i++)
    {
        if (angles[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "evspace::EulerAngles invalid at index %i", i);
            return NULL;
        }
    }

    if (PyEVSpaceFrame_ToOffset(frame, offset) < 0) {
        return NULL;
    }
    for (int i = 0; i < 3; i++)
    {
        if (offset[i] != 0)
        {
            PyErr_Format(PyExc_AssertionError, "evspace::Vector offset (no offset) invalid at index %i", i);
            return NULL;
        }
    }

    if (PyEVSpaceFrame_ToOffset(frame_offset, offset) < 0) {
        return NULL;
    }
    for (int i = 0; i < 3; i++)
    {
        if (offset[i] != i + 1)
        {
            PyErr_Format(PyExc_AssertionError, "evspace::Vector offset (offset) invalid at index %i", i);
            return NULL;
        }
    }

    frame_set_cpp = PyEVSpace_API->PyEVSpaceFrame_FromState(order_state, tmp, tmp, 0);
    if (!frame_set_cpp) {
        goto error;
    }
    if (PyEVSpaceFrame_SetAngles(frame_set_cpp, angles) < 0) {
        goto error;
    }
    if (PyEVSpaceFrame_SetOffset(frame_set_cpp, offset) < 0) {
        goto error;
    }

    // test capsule creation/modifiers
    frame_from = PyEVSpace_API->PyEVSpaceFrame_FromState(order_state, angles_state, NULL, 1);
    frame_from_offset = PyEVSpace_API->PyEVSpaceFrame_FromState(order_state, angles_state,
                                                              offset_state, 0);
    frame_set = PyEVSpace_API->PyEVSpaceFrame_FromState(order_state, tmp, NULL, 1);
    frame_set_offset = PyEVSpace_API->PyEVSpaceFrame_FromState(order_state, tmp, tmp, 0);
    if (!frame_set || !frame_set_offset) {
        goto error;
    }

    if (PyEVSpace_API->PyEVSpaceFrame_SetAngles(frame_set, angles_state) < 0) {
        goto error;
    }
    if (PyEVSpace_API->PyEVSpaceFrame_SetOffset(frame_set, NULL) < 0) {
        goto error;
    }
    if (PyEVSpace_API->PyEVSpaceFrame_SetAngles(frame_set_offset, angles_state) < 0) {
        goto error;
    }
    if (PyEVSpace_API->PyEVSpaceFrame_SetOffset(frame_set_offset, offset_state) < 0) {
        goto error;
    }

    rtn = Py_BuildValue("(OO)(OOO)", frame_from, frame_set, frame_from_offset,
                        frame_set_offset, frame_set_cpp);
    Py_XDECREF(frame_from);
    Py_XDECREF(frame_from_offset);
    Py_XDECREF(frame_set);
    Py_XDECREF(frame_set_offset);
    Py_XDECREF(frame_set_cpp);

    return rtn;


error:
    Py_XDECREF(frame_from);
    Py_XDECREF(frame_from_offset);
    Py_XDECREF(frame_set);
    Py_XDECREF(frame_set_offset);
    Py_XDECREF(frame_set_cpp);
    return NULL;
}

PyDoc_STRVAR(vector_capsule_doc, "vector_capsule_c(vector: Vector) -> tuple[Vector]\n\
\n\
Test Python level Vector types and return several Vectors to be verified\n\
at the Python level.");

PyDoc_STRVAR(matrix_capsule_doc, "matrix_capsule_c(matrix: Matrix) -> tuple[Matrix]:\n\
Test Python level Matrix types and return several Matrix objects to be\n\
verified at the Python level.");

PyDoc_STRVAR(angles_capsule_doc, "angles_capsule_c(angles: EulerAngles) -> tuple[EulerAngles]:\n\
Test Python level EulerAngles types and return several EulerAngles objects\n\
to be verified at the Python level.");

PyDoc_STRVAR(order_capsule_doc, "order_capsule_c(order: RotationOrder) -> tuple[RotationOrder]:\n\
Test Python level RotationOrder types and return several RotationOrder objects\n\
to be verified at the Python level.");

PyDoc_STRVAR(frame_capsule_doc, "frame_capsule_c(frame: ReferenceFrame, frame_offset: ReferenceFrame) -> tulpe[ReferenceFrame]:\n\
Test Python level ReferenceFrame types and return several ReferenceFrame\n\
objects to be verified at the Python level.");

static PyMethodDef consumer_methods[] = {

    {"vector_capsule_cpp", (PyCFunction)Test_VectorCapsule, METH_VARARGS, vector_capsule_doc},

    {"matrix_capsule_cpp", (PyCFunction)Test_MatrixCapsule, METH_VARARGS, matrix_capsule_doc},

    {"angles_capsule_cpp", (PyCFunction)Test_AnglesCapsule, METH_VARARGS, angles_capsule_doc},

    {"order_capsule_cpp", (PyCFunction)Test_OrderCapsule, METH_VARARGS, order_capsule_doc},

    {"frame_capsule_cpp", (PyCFunction)Test_FrameCapsule, METH_VARARGS, frame_capsule_doc},

    {NULL}
};

PyDoc_STRVAR(module_doc, "Module for testing C++ API capsule.");

static PyModuleDef Consumer_Module = {
    PyModuleDef_HEAD_INIT,
    "capsule_consumer_cpp", // m_name
    module_doc,             // m_doc
    -1,                     // m_size
    consumer_methods        // m_methods
};

PyMODINIT_FUNC
PyInit_capsule_consumer_cpp(void)
{
    PyObject* module = PyModule_Create(&Consumer_Module);

    if (PyEVSpace_ImportCapsule() < 0) {
        Py_XDECREF(module);
        return NULL;
    }

    if (PyEVSpace_API->version != PYEVSPACE_CAPSULE_VERSION)
    {
        PyErr_SetString(PyExc_AssertionError, "C API capsule version does not match");
        return NULL;
    }
    
    return module;
}