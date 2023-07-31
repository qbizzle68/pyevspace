#ifndef EVSPACE_API_H
#define EVSPACE_API_H

#include <Python.h>

/* C types */

typedef struct {
    PyObject_HEAD
    double* data;           /* x, y, z component of vectors */
} EVSpace_Vector;

typedef struct {
    PyObject_HEAD
    double* data;           /* row by column ordering */
} EVSpace_Matrix;

typedef struct {
    PyObject_HEAD
    double alpha;
    double beta;
    double gamma;
    // Keep track of the reference frame for callback.
    void* master;
} EVSpace_Angles;

typedef enum {
    X_AXIS = 0,
    Y_AXIS = 1,
    Z_AXIS = 2
} EVSpace_Axis;

typedef struct {
    PyObject_HEAD
    EVSpace_Axis first;
    EVSpace_Axis second;
    EVSpace_Axis third;
} EVSpace_Order;

typedef struct {
    PyObject_HEAD
    EVSpace_Order* order;
    EVSpace_Angles* angles;
    EVSpace_Matrix* matrix;
    EVSpace_Vector* offset;
} EVSpace_ReferenceFrame;


/* define macros for accessing EVSpace_Vector array values */
#define EVSpace_VECTOR_X(o)     (((EVSpace_Vector*)o)->data[0])
#define EVSpace_VECTOR_Y(o)     (((EVSpace_Vector*)o)->data[1])
#define EVSpace_VECTOR_Z(o)     (((EVSpace_Vector*)o)->data[2])


/* define macros for accessing EVSpace_Matrix values */
#define EVSpace_RC_INDEX(r, c)          (3 * r + c)
#define EVSpace_MATRIX_COMP(o, r, c) \
        (((EVSpace_Matrix*)o)->data[EVSpace_RC_INDEX(r, c)])

#define Vector_Check(o)     PyObject_TypeCheck(o, &EVSpace_VectorType)
#define Matrix_Check(o)     PyObject_TypeCheck(o, &EVSpace_MatrixType)
#define Angles_Check(o)     PyObject_TypeCheck(o, &EVSpace_AnglesType)
#define Order_Check(o)      PyObject_TypeCheck(o, &EVSpace_OrderType)
#define ReferenceFrame_Check(o)	\
                            PyObject_TypeCheck(o, &EVSpace_ReferenceFrameType)


/* Define structure for C API */
typedef struct {
    /* type objects */
    PyTypeObject* VectorType;
    PyTypeObject* MatrixType;
    PyTypeObject* AnglesType;
    PyTypeObject* OrderType;
    PyTypeObject* RefFrameType;

    /* constructors */
    EVSpace_Vector* (*Vector_FromArray)(double*, PyTypeObject*);
    EVSpace_Vector* (*Vector_StealArray)(double*, PyTypeObject*);
    EVSpace_Matrix* (*Matrix_FromArray)(double*, PyTypeObject*);
    EVSpace_Matrix* (*Matrix_StealArray)(double*, PyTypeObject*);
    EVSpace_Angles* (*Angles_New)(double, double, double, PyTypeObject*);
    EVSpace_Order*  (*Order_New)(EVSpace_Axis, EVSpace_Axis, EVSpace_Axis,
                                 PyTypeObject*);
    EVSpace_ReferenceFrame* (*RefFrame_New)(EVSpace_Order*, EVSpace_Angles*,
                                            EVSpace_Vector*, PyTypeObject*);

    /* vector number methods */
    EVSpace_Vector* (*Vector_add)(const EVSpace_Vector*,
                                  const EVSpace_Vector*);
    EVSpace_Vector* (*Vector_subtract)(const EVSpace_Vector*,
                                       const EVSpace_Vector*);
    EVSpace_Vector* (*Vector_multiply)(const EVSpace_Vector*, double);
    EVSpace_Vector* (*Vector_multiply_m)(const EVSpace_Vector*,
                                         const EVSpace_Matrix*);
    EVSpace_Vector* (*Vector_divide)(const EVSpace_Vector*, double);
    void (*Vector_iadd)(EVSpace_Vector*, const EVSpace_Vector*);
    void (*Vector_isubtract)(EVSpace_Vector*, const EVSpace_Vector*);
    void (*Vector_imultiply)(EVSpace_Vector*, double);
    void (*Vector_idivide)(EVSpace_Vector*, double);
    EVSpace_Vector* (*Vector_negative)(const EVSpace_Vector*);

    /* matrix number methods */
    EVSpace_Matrix* (*Matrix_add)(const EVSpace_Matrix*,
                                  const EVSpace_Matrix*);
    EVSpace_Matrix* (*Matrix_subtract)(const EVSpace_Matrix*, 
                                       const EVSpace_Matrix*);
    EVSpace_Vector* (*Matrix_multiply_v)(const EVSpace_Matrix*,
                                         const EVSpace_Vector*);
    EVSpace_Matrix* (*Matrix_multiply_m)(const EVSpace_Matrix*,
                                         const EVSpace_Matrix*);
    EVSpace_Matrix* (*Matrix_multiply_s)(const EVSpace_Matrix*, double);
    EVSpace_Matrix* (*Matrix_divide)(const EVSpace_Matrix*, double);
    void (*Matrix_iadd)(EVSpace_Matrix*, const EVSpace_Matrix*);
    void (*Matrix_isubtract)(EVSpace_Matrix*, const EVSpace_Matrix*);
    void (*Matrix_imultiply)(EVSpace_Matrix*, double);
    void (*Matrix_idivide)(EVSpace_Matrix*, double);
    EVSpace_Matrix* (*Matrix_negative)(const EVSpace_Matrix*);

    /* vector class methods */
    double (*Vector_mag)(const EVSpace_Vector*);
    double (*Vector_mag2)(const EVSpace_Vector*);
    void (*Vector_normalize)(EVSpace_Vector*);

    /* module methods */
    double (*Vector_dot)(const EVSpace_Vector*, const EVSpace_Vector*);
    EVSpace_Vector* (*Vector_cross)(const EVSpace_Vector*,
                                    const EVSpace_Vector*);
    EVSpace_Vector* (*Vector_norm)(const EVSpace_Vector*);
    double (*Vector_vang)(const EVSpace_Vector*, const EVSpace_Vector*);
    EVSpace_Vector* (*Vector_vxcl)(const EVSpace_Vector*,
                                   const EVSpace_Vector*);
    EVSpace_Vector* (*Vector_proj)(const EVSpace_Vector*,
                                   const EVSpace_Vector*);
    double (*Matrix_det)(const EVSpace_Matrix*);
    EVSpace_Matrix* (*Matrix_transpose)(const EVSpace_Matrix*);

    /* rotation orders */
    const EVSpace_Order* Order_XYZ;
    const EVSpace_Order* Order_XZY;
    const EVSpace_Order* Order_YXZ;
    const EVSpace_Order* Order_YZX;
    const EVSpace_Order* Order_ZXY;
    const EVSpace_Order* Order_ZYX;
    const EVSpace_Order* Order_XYX;
    const EVSpace_Order* Order_XZX;
    const EVSpace_Order* Order_YXY;
    const EVSpace_Order* Order_YZY;
    const EVSpace_Order* Order_ZXZ;
    const EVSpace_Order* Order_ZYZ;

    /* rotation matrix methods */
    EVSpace_Matrix* (*Get_matrix)(EVSpace_Axis, double);
    EVSpace_Matrix* (*Get_euler)(const EVSpace_Order*, const EVSpace_Angles*);
    EVSpace_Matrix* (*Get_from_to)(const EVSpace_Order*,
                                   const EVSpace_Angles*,
                                   const EVSpace_Order*, 
                                   const EVSpace_Angles*);

    /* rotate vector methods */
    EVSpace_Vector* (*Rotate_axis_to)(EVSpace_Axis, double, 
                                      const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_axis_from)(EVSpace_Axis, double, 
                                        const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_euler_to)(const EVSpace_Order*, 
                                       const EVSpace_Angles*,
                                       const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_euler_from)(const EVSpace_Order*, 
                                         const EVSpace_Angles*,
                                         const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_matrix_to)(const EVSpace_Matrix*, 
                                        const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_matrix_from)(const EVSpace_Matrix*, 
                                          const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_offset_to)(const EVSpace_Matrix*, 
                                        const EVSpace_Vector*,
                                        const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_offset_from)(const EVSpace_Matrix*, 
                                          const EVSpace_Vector*,
                                          const EVSpace_Vector*);

    /* reference frame methods */
    EVSpace_Vector* (*Rotate_ref_to)(const EVSpace_ReferenceFrame*,
                                     const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_ref_from)(const EVSpace_ReferenceFrame*,
                                       const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_ref_to_ref)(const EVSpace_ReferenceFrame*,
                                         const EVSpace_ReferenceFrame*,
                                         const EVSpace_Vector*);
    EVSpace_Vector* (*Rotate_ref_from_ref)(const EVSpace_ReferenceFrame*,
                                           const EVSpace_ReferenceFrame*,
                                           const EVSpace_Vector*);

} EVSpace_CAPI;

#define Evs_CAPSULE_NAME "pyevspace.evspace_CAPI"


/* This is only used by the API and should not be included in
    * evspacemodule.c since it doesn't use the capsule.
    */
#ifndef _EVSPACE_IMPL

/* Define global variable for the C API and a macro for setting it. */
static EVSpace_CAPI* EVSpaceAPI = NULL;

#define EVSpace_IMPORT EVSpaceAPI \
            = (EVSpace_CAPI *)PyCapsule_Import(EVSpace_CAPSULE_NAME, 0)

    /* macros for simplified constructor calls */
#define Evs_Vector_FromArray(arr)       \
        EVSpaceAPI->Vector_FromArray(arr, EVSpaceAPI->VectorType)
#define Evs_Vector_StealArray(arr)      \
        EVSpaceAPI->Vector_StealArray(arr, EVSpaceAPI->VectorType)
#define Evs_Matrix_FromArray(arr)       \
        EVSpaceAPI->Matrix_FromArray(arr, EVSpaceAPI->MatrixType)
#define Evs_Matrix_StealArray(arr)      \
        EVSpaceAPI->Matrix_StealArray(arr, EVSpaceAPI->MatrixType)
#define Evs_Angles_New(a, b, g)         \
        EVSpaceAPI->Angles_New(a, b, g, EVSpaceAPI->AnglesType)
#define Evs_Order_New(o, a)             \
        EVSpaceAPI->Order_New(o, a, EVSpaceAPI->OrderType)
#define Evs_RefFrame_New(o, a, v)       \
        EVSpaceAPI->RefFrame_New(o, a, v, EVSpaceAPI->RefFrameType)

/* macros for C API */
#define Evs_Vector_add(rhs, lhs)        EVSpaceAPI->Vector_add(rhs, lhs)
#define Evs_Vector_subtract(rhs, lhs)   EVSpaceAPI->Vector_subtract(rhs, lhs)
#define Evs_Vector_multiply(rhs, lhs)   EVSpaceAPI->Vector_multiply(rhs, lhs)
#define Evs_Vector_multiply_m(rhs, lhs) EVSpaceAPI->Vector_multiply_m(rhs, lhs)
#define Evs_Vector_divide(rhs, lhs)     EVSpaceAPI->Vector_divide(rhs, lhs)
#define Evs_Vector_iadd(self, other)    EVSpaceAPI->Vector_iadd(self, other)
#define Evs_Vector_isubtract(self, other)   \
        EVSpaceAPI->Vector_isubtract(self, other)
#define Evs_Vector_imultilpy(self, other)   \
        EVSpaceAPI->Vector_imultiply(self, other)
#define Evs_Vector_idivide(self, other) EVSpaceAPI->Vector_idivide(self, other)
#define Evs_Vector_negative(self)       EVSpaceAPI->Vector_negative(self)

#define Evs_Matrix_add(rhs, lhs)        EVSpaceAPI->Matrix_add(rhs, lhs)
#define Evs_Matrix_subtract(rhs, lhs)   EVSpaceAPI->Matrix_subtract(rhs, lhs)
#define Evs_Matrix_multiply_v(rhs, lhs) EVSpaceAPI->Matrix_multiply_v(rhs, lhs)
#define Evs_Matrix_multiply_m(rhs, lhs) EVSpaceAPI->Matrix_multiply_m(rhs, lhs)
#define Evs_Matrix_multiply_s(rhs, lhs) EVSpaceAPI->Matrix_multiply_s(rhs, lhs)
#define Evs_Matrix_divide(rhs, lhs)     EVSpaceAPI->Matrix_divide(rhs, lhs)
#define Evs_Matrix_iadd(self, other)    EVSpaceAPI->Matrix_iadd(self, other)
#define Evs_Matrix_isubtract(self, other)   \
        EVSpaceAPI->Matrix_isubtract(self, other)
#define Evs_Matrix_imultiply(self, other)   \
        EVSpaceAPI->Matrix_imultiply(self, other)
#define Evs_Matrix_idivide(self, other)     \
        EVSpaceAPI->Matrix_idivide(self, other)
#define Evs_Matrix_negative(self)       EVSpaceAPI->Matrix_negative(self)

#define Evs_mag(self)               EVSpaceAPI->Vector_mag(self)
#define Evs_mag2(self)              EVSpaceAPI->Vector_mag2(self)
#define Evs_normalize(self)         EVSpaceAPI->Vector_normalize(self)

#define Evs_dot(lhs, rhs)           EVSpaceAPI->Vector_dot(lhs, rhs)
#define Evs_cross(lhs, rhs)         EVSpaceAPI->Vector_cross(lhs, rhs)
#define Evs_norm(self)              EVSpaceAPI->Vector_norm(self)
#define Evs_vang(lhs, rhs)          EVSpaceAPI->Vector_vang(lhs, rhs)
#define Evs_vxcl(vec, xcl)          EVSpaceAPI->Vector_vxcl(vec, xcl)
#define Evs_proj(proj, onto)        EVSpaceAPI->Vector_proj(proj, onto)
#define Evs_det(lhs)                EVSpaceAPI->Matrix_det(lhs)
#define Evs_transpose(self)         EVSpaceAPI->Matrix_transpose(self)

#define Evs_XYZ     EVSpaceAPI->Order_XYZ
#define Evs_XZY     EVSpaceAPI->Order_XZY
#define Evs_YXZ     EVSpaceAPI->Order_YXZ
#define Evs_YZX     EVSpaceAPI->Order_YZX
#define Evs_ZXY     EVSpaceAPI->Order_ZXY
#define Evs_ZYX     EVSpaceAPI->Order_ZYX
#define Evs_XYX     EVSpaceAPI->Order_XYX
#define Evs_XZX     EVSpaceAPI->Order_XZX
#define Evs_YXY     EVSpaceAPI->Order_YXY
#define Evs_YZY     EVSpaceAPI->Order_YZY
#define Evs_ZXZ     EVSpaceAPI->Order_ZXZ
#define Evs_ZYZ     EVSpaceAPI->Order_ZYZ

#define Evs_get_matrix(ax, ag)      EVSpace_API->Get_matrix(ax, ag)
#define Evs_get_euler(o, a)         EVSpace_API->Get_euler(o, a)
#define Evs_from_to(of, af, ot, at) EVSpace_API->Get_from_to(of, af, ot, at)

#define Evs_axis_to(ax, ag, v)      EVSpaceAPI->Rotate_axis_to(ax, ag, v)
#define Evs_axis_from(ax, ag, v)    EVSpaceAPI->Rotate_axis_from(ax, ag, v)
#define Evs_euler_to(o, a, v)       EVSpaceAPI->Rotate_euler_to(o, a, v)
#define Evs_euler_from(o, a, v)     EVSpaceAPI->Rotate_euler_from(o, a, v)
#define Evs_matrix_to(m, v)         EVSpaceAPI->Rotate_matrix_to(m, v)
#define Evs_matrix_from(m, v)       EVSpaceAPI->Rotate_matrix_from(m, v)
#define Evs_offset_to(m, o, v)      EVSpaceAPI->Rotate_offset_to(m, o, v)
#define Evs_offset_from(m, o, v)    EVSpaceAPI->Rotate_offset_from(m, o, v)

#define Evs_ref_to(f, v)            EVSpaceAPI->Rotate_ref_to(f, v)
#define Evs_ref_from(f, v)          EVSpaceAPI->Rotate_ref_from(f, v)
#define Evs_ref_to_ref(s, f, v)     EVSpaceAPI->Rotate_ref_to_ref(s, f, v)
#define Evs_ref_from_ref(s, f, v)   EVSpaceAPI->Rotate_ref_from_ref(s, f, v)

#endif // !defined(_EVSPACE_IMPL)

#endif // !defined(EVSPACE_API_H)