#ifndef _PYPyEVSpace_API_H_
#define _PYPyEVSpace_API_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>

#define PYEVSPACE_CAPSULE_NAME "pyevspace._pyevspace._C_API"
#define PYEVSPACE_CAPSULE_VERSION 1

typedef struct
{
    int version;

    // PyTypeObjects
    PyTypeObject* Vector_Type;
    PyTypeObject* Matrix_Type;
    PyTypeObject* EulerAngles_Type;
    PyTypeObject* RotationOrder_Type;
    PyTypeObject* ReferenceFrame_Type;

    // evspace state getters
    int (*PyEVSpaceVector_GetState)(PyObject*, double[3]);
    int (*PyEVSpaceMatrix_GetState)(PyObject*, double[9]);
    int (*PyEVSpaceAngles_GetState)(PyObject*, double[3]);
    int (*PyEVSpaceOrder_GetState)(PyObject*, unsigned int[3]);
    int (*PyEVSpaceFrame_GetState)(PyObject*, unsigned int[3], double[3], double[3], int*);
    PyObject* (*PyEVSpaceFrame_GetOrder)(PyObject*);
    PyObject* (*PyEVSpaceFrame_GetAngles)(PyObject*);
    PyObject* (*PyEVSpaceFrame_GetOffset)(PyObject*);
    int (*PyEVSpaceFrame_GetIntrinsic)(PyObject*);

    // Python type constructors via state
    PyObject* (*PyEVSpaceVector_FromState)(double[3]);
    PyObject* (*PyEVSpaceMatrix_FromState)(double[9]);
    PyObject* (*PyEVSpaceAngles_FromState)(double[3]);
    PyObject* (*PyEVSpaceOrder_FromState)(unsigned int[3]);
    PyObject* (*PyEVSpaceFrame_FromState)(unsigned int[3], double[3], double*, int);

    // Python type modifiers
    int (*PyEVSpaceVector_SetState)(PyObject*, double[3]);
    int (*PyEVSpaceMatrix_SetState)(PyObject*, double[9]);
    int (*PyEVSpaceAngles_SetState)(PyObject*, double[3]);
    int (*PyEVSpaceFrame_SetAngles)(PyObject*, double[3]);
    int (*PyEVSpaceFrame_SetOffset)(PyObject*, double*);

} PyEVSpace_CAPI;

#ifndef _EVS_PYEVSPACE_IMPL

static PyEVSpace_CAPI* PyEVSpace_API;

static int
PyEVSpace_ImportCapsule(void)
{
    PyEVSpace_API = (PyEVSpace_CAPI*)(PyCapsule_Import(PYEVSPACE_CAPSULE_NAME, 0));
    return (PyEVSpace_API != NULL) ? 0: -1;
}

#ifdef __cplusplus
// todo: make this depend on a better macro, like a version
#if (defined(_EVSPACE_H_) || defined(PYEVSPACE_INCLUDE_HEADERS)) && !defined(PYEVSPACE_NO_INCLUDE_HEADERS)
}   // extern "C"

#include <evspace.hpp>
#include <cstring>      // std::memcpy

template<typename T, std::size_t N>
static inline std::array<T, N>
_PyEVSpace_PointerToArray(T arr[N])
{
    std::array<T, N> result;
    std::copy(arr, arr + N, result.begin());

    return result;
}

static inline int
PyEVSpace_ToVector(PyObject* obj, evspace::Vector& vector)
{
    double state[3];

    if (PyEVSpace_API->PyEVSpaceVector_GetState(obj, state) < 0) {
        return -1;
    }
    std::array<double, 3> arr = _PyEVSpace_PointerToArray<double, 3>(state);
    vector = evspace::Vector(arr);

    return 0;
}

// todo: make these const references

static inline PyObject*
PyEVSpaceVector_ToObject(const evspace::Vector& vector)
{
    double state[3];
    std::memcpy(state, vector.data().data(), 3 * sizeof(double));
    return PyEVSpace_API->PyEVSpaceVector_FromState(state);
}

static inline int
PyEVSpace_ToMatrix(PyObject* obj, evspace::Matrix& matrix)
{
    double state[9];

    if (PyEVSpace_API->PyEVSpaceMatrix_GetState(obj, state) < 0) {
        return -1;
    }
    std::array<double, 9> arr = _PyEVSpace_PointerToArray<double, 9>(state);
    matrix = evspace::Matrix(arr);

    return 0;
}

static inline PyObject*
PyEVSpaceMatrix_ToObject(const evspace::Matrix& matrix)
{
    double state[9];
    std::memcpy(state, matrix.data().data(), 3 * sizeof(double));
    return PyEVSpace_API->PyEVSpaceMatrix_FromState(state);
}

static inline int
PyEVSpace_ToAngles(PyObject* obj, evspace::EulerAngles& angles)
{
    double state[3];
    if (PyEVSpace_API->PyEVSpaceAngles_GetState(obj, state) < 0) {
        return -1;
    }
        
    angles = evspace::EulerAngles(state[0], state[1], state[2]);
    return 0;
}

static inline PyObject*
PyEVSpaceAngles_ToObject(const evspace::EulerAngles& angles)
{
    double tmp[3]{angles[0], angles[1], angles[2]};
    return PyEVSpace_API->PyEVSpaceAngles_FromState(tmp);
}

// no real way to convert to evspace::RotationOrder and it's meant to
// be a type anyway, not a concrete instance

static inline int
PyEVSpaceFrame_ToAngles(PyObject* obj, evspace::EulerAngles& angles)
{
    PyObject* angles_obj;
    double state[3];

    angles_obj = PyEVSpace_API->PyEVSpaceFrame_GetAngles(obj);
    if (!angles_obj) {
        return -1;
    }

    if (PyEVSpace_API->PyEVSpaceAngles_GetState(angles_obj, state) < 0) {
        Py_DECREF(angles_obj);
        return -1;
    }

    for (int i = 0; i < 3; i++) {
        angles[i] = state[i];
    }
    
    Py_DECREF(angles_obj);
    return 0;
}

static inline int
PyEVSpaceFrame_ToOffset(PyObject* obj, evspace::Vector& offset)
{
    PyObject* offset_obj;
    double state[3];

    offset_obj = PyEVSpace_API->PyEVSpaceFrame_GetOffset(obj);
    if (!offset_obj) {
        return -1;
    }

#if PY_VERSION_HEX >= 0x030a0000
    if (Py_IsNone(offset_obj))
#else
    if (offset_obj == Py_None)
#endif
    {
        for (int i = 0; i < 3; i++) offset[i] = 0.0;
        return 0;
    }

    if (PyEVSpace_API->PyEVSpaceVector_GetState(offset_obj, state) < 0) {
        Py_DECREF(offset_obj);
        return -1;
    }

    std::memcpy(offset.data().data(), state, 3 * sizeof(double));
    Py_DECREF(offset_obj);

    return 0;
}

static inline int
PyEVSpaceFrame_SetAngles(PyObject* obj, const evspace::EulerAngles& angles)
{
    PyObject* angles_obj;
    double state[3]{angles[0], angles[1], angles[2]};
    int result = 0;

    angles_obj = PyEVSpace_API->PyEVSpaceFrame_GetAngles(obj);
    if (!angles_obj) {
        return -1;
    }

    result = PyEVSpace_API->PyEVSpaceAngles_SetState(angles_obj, state);
    Py_DECREF(angles_obj);
    
    return result;
}

static inline int
PyEVSpaceFrame_SetOffset(PyObject* obj, const evspace::Vector& offset)
{
    PyObject* offset_obj;
    double state[3];
    int result = 0;

    offset_obj = PyEVSpace_API->PyEVSpaceFrame_GetOffset(obj);
    if (!offset_obj) {
        return -1;
    }

    std::memcpy(state, offset.data().data(), 3 * sizeof(double));
    result = PyEVSpace_API->PyEVSpaceFrame_SetOffset(obj, state);
    Py_DECREF(offset_obj);

    return result;
}

extern "C" {
#endif  // _EVSPACE_H_ || PYEVSPACE_INCLUDE_HEADERS && !PYEVSPACE_NO_INCLUDE_HEADERS
#endif  // __cplusplus

#endif  // _EVS_PYEVSPACE_IMPL

#ifdef __cplusplus
}
#endif

#endif  // _PYPyEVSpace_API_H_