#ifndef _PYEVSPACEMODULE_H_
#define _PYEVSPACEMODULE_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#if PY_VERSION_HEX < 0x030a0000
#   include <structmember.h>
#endif

#include <evspace.hpp>
#include <array>

/* C++ types */

typedef struct {
    PyObject_HEAD
    evspace::Vector* vector;
} EVSpace_Vector;

typedef struct {
    PyObject_HEAD
    evspace::Matrix* matrix;
} EVSpace_Matrix;

typedef struct {
    PyObject_HEAD
    evspace::EulerAngles* angles;
} EVSpace_Angles;

typedef struct {
    PyObject_HEAD
    evspace::AxisDirection first;
    evspace::AxisDirection second;
    evspace::AxisDirection third;
} EVSpace_Order;

typedef struct {
    PyObject_HEAD
    evspace::AxisDirection first;
    evspace::AxisDirection second;
    evspace::AxisDirection third;
    std::array<double, 3> angles;
    evspace::Matrix* matrix;
    // Will possible be Py_None so needs to be PyObject type
    PyObject* offset;
    bool intrinsic;
} EVSpace_ReferenceFrame;

typedef struct {
    PyObject_HEAD
    EVSpace_Matrix* base;
    double* data;
    Py_ssize_t* shape;
    Py_ssize_t* strides;
    int ndim;
} EVSpace_MatrixView;

#define EVSpaceVector_VECTOR(o)         (*((o)->vector))
#define EVSpaceVector_X(o)              EVSpaceVector_VECTOR(o)[0]
#define EVSpaceVector_Y(o)              EVSpaceVector_VECTOR(o)[1]
#define EVSpaceVector_Z(o)              EVSpaceVector_VECTOR(o)[2]

#define EVSpaceMatrix_MATRIX(o)         (*((o)->matrix))

#define EVSpaceAngles_ANGLES(o)         (*((o)->angles))
#define EVSpaceAngles_ALPHA(o)          EVSpaceAngles_ANGLES(o)[0]
#define EVSpaceAngles_BETA(o)           EVSpaceAngles_ANGLES(o)[1]
#define EVSpaceAngles_GAMMA(o)          EVSpaceAngles_ANGLES(o)[2]

#define EVSpaceVector_Check(o)          PyObject_TypeCheck(o, &EVSpace_VectorType)
#define EVSpaceMatrix_Check(o)          PyObject_TypeCheck(o, &EVSpace_MatrixType)
#define EVSpaceAngles_Check(o)          PyObject_TypeCheck(o, &EVSpace_AnglesType)
#define EVSpaceOrder_Check(o)           PyObject_TypeCheck(o, &EVSpace_OrderType)
#define EVSpaceReferenceFrame_Check(o)  PyObject_TypeCheck(o, &EVSpace_ReferenceFrameType)

#define EVS_PyObject_Cast(o)            reinterpret_cast<PyObject*>(o)
#define EVSpaceVector_Cast(o)           reinterpret_cast<EVSpace_Vector*>(o)
#define EVSpaceMatrix_Cast(o)           reinterpret_cast<EVSpace_Matrix*>(o)

// todo: add capsule api 

typedef struct {

} EVSpace_CAPI;

#endif // _PYEVSPACEMODULE_H_