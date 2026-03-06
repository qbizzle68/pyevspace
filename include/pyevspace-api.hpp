#ifndef _PYEVSPACE_API_H_
#define _PYEVSPACE_API_H_
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
    int (*EVSpaceVector_GetState)(PyObject*, double[3]);
    int (*EVSpaceMatrix_GetState)(PyObject*, double[9]);
    int (*EVSpaceAngles_GetState)(PyObject*, double[3]);
    int (*EVSpaceOrder_GetState)(PyObject*, unsigned int[3]);
    int (*EVSpaceFrame_GetState)(PyObject*, unsigned int[3], double[3], double[3], int*);

    // Python type constructors via state
    PyObject* (*EVSpaceVector_FromState)(double, double, double);
    PyObject* (*EVSpaceVector_FromStateArray)(double[3]);
    PyObject* (*EVSpaceMatrix_FromState)(double[9]);
    PyObject* (*EVSpaceAngles_FromState)(double, double, double);
    PyObject* (*EVSpaceAngles_FromStateArray)(double[3]);
    PyObject* (*EVSpaceOrder_FromState)(unsigned int, unsigned int, unsigned int);
    PyObject* (*EVSpaceOrder_FromStateArray)(unsigned int[3]);
    PyObject* (*EVSpaceFrame_FromState)(unsigned int, unsigned int, unsigned int,
                                        double, double, double, double*, int);
    PyObject* (*EVSpaceFrame_FromStateArray)(unsigned int[3], double[3], double*, int);

} EVSpace_CAPI;

#ifndef _EVS_PYEVSPACE_IMPL

static EVSpace_CAPI* EVSpace_API;

static int
import_pyevspace(void)
{
    EVSpace_API = static_cast<EVSpace_CAPI*>(PyCapsule_Import(PYEVSPACE_CAPSULE_NAME, 0));
    return (EVSpace_API != NULL) ? 0: -1;
}

#endif  // _EVS_PYEVSPACE_IMPL

#ifdef __cplusplus
}
#endif

#endif  // _PYEVSPACE_API_H_