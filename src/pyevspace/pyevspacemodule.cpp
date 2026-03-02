#define PY_SSIZE_T_CLEAN
#include <Python.h>
#if PY_VERSION_HEX < 0x030c0000
#   include <structmember.h>
#endif

#include <pyevspacemodule.hpp>
#include <cfloat>       // DBL_EPSILON
#include <cstdint>      // int64_t
#include <cstddef>      // offsetof
#include <array>        // std::array
#include <string_view>

// todo: remove this when done developing
#include <iostream>
#include <cstdio>

static PyTypeObject EVSpace_VectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyTypeObject EVSpace_MatrixType = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyTypeObject EVSpace_MatrixViewType = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyTypeObject EVSpace_AnglesType = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyTypeObject EVSpace_OrderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyTypeObject EVSpace_ReferenceFrameType = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

// Don't return PyErr_NoMemory() so this can be used in
// methods that be used anywhere a pointer is returned.
#define EXCEPTION_WRAPPER(o)    try {\
    o\
}\
catch (const std::bad_alloc&) {\
    PyErr_NoMemory();\
    return NULL;\
}\
catch (const std::exception& e) {\
    PyErr_SetString(PyExc_RuntimeError, e.what());\
    return NULL;\
}
#define EVS_BUFFER_EMPTY            0x0
#define EVS_BUFFER_NO_SHAPE         (~(1 << 1))
#define EVS_BUFFER_RELEASE_SHAPE    (1 << 1)
#define EVS_BUFFER_NO_STRIDES       (~(1 << 2))
#define EVS_BUFFER_RELEASE_STRIDES  (1 << 2)
//  minimum Vector length to allow for extrinsic single axis rotation
#define VECTOR_LENGTH_SMALL     0.000001

typedef std::array<double, 3> varray_t;
typedef std::array<double, 9> marray_t;

static void
EVSpaceBuffer_Release(PyObject* obj, Py_buffer* view)
{
    if (view->internal != NULL)
    {
        int internal = *(reinterpret_cast<int*>(view->internal));

        if (internal & EVS_BUFFER_RELEASE_SHAPE) {
            free(view->shape);
        }
        if (internal & EVS_BUFFER_RELEASE_STRIDES) {
            free(view->strides);
        }
        free(view->internal);
    }
}

// Parses obj into a C double type. On success return obj otherwise
// return NULL.
static inline PyObject*
EVSpaceObject_AsDouble(PyObject* obj, double& value)
{
    double tmp = PyFloat_AsDouble(obj);
    if (tmp == -1.0 && PyErr_Occurred()) {
        return NULL;
    }

    value = tmp;
    return obj;
}

// Iterate over obj to fill the items array. The function fails if
// obj is not iterable or the number of items in obj is not exactly
// 3. Exception is set and return -1 on failure, otherwise items is
// filled and return 0 on success. items is only modified on success.
static int
EVSpaceIterable_GetItems(PyObject* obj, std::array<PyObject*, 3>& items)
{
    PyObject *item;
    Py_ssize_t count = 0;
    PyObject* tmp[3]{NULL};

    if (PyTuple_Check(obj) && PyTuple_GET_SIZE(obj) == 3)
    {
#if PY_VERSION_HEX >= 0x030a0000
        items[0] = Py_NewRef(PyTuple_GET_ITEM(obj, 0));
        items[1] = Py_NewRef(PyTuple_GET_ITEM(obj, 1));
        items[2] = Py_NewRef(PyTuple_GET_ITEM(obj, 2));
#else
        items[0] = PyTuple_GET_ITEM(obj, 0);
        Py_INCREF(items[0]);
        items[1] = PyTuple_GET_ITEM(obj, 1);
        Py_INCREF(items[1]);
        items[2] = PyTuple_GET_ITEM(obj, 2);
        Py_INCREF(items[2]);
#endif
        return 0;
    }
    else if (PyList_Check(obj) && PyList_GET_SIZE(obj) == 3)
    {
#if PY_VERSION_HEX >= 0x030a0000
        items[0] = Py_NewRef(PyList_GET_ITEM(obj, 0));
        items[1] = Py_NewRef(PyList_GET_ITEM(obj, 1));
        items[2] = Py_NewRef(PyList_GET_ITEM(obj, 2));
#else
        items[0] = PyList_GET_ITEM(obj, 0);
        Py_INCREF(items[0]);
        items[1] = PyList_GET_ITEM(obj, 1);
        Py_INCREF(items[1]);
        items[2] = PyList_GET_ITEM(obj, 2);
        Py_INCREF(items[2]);
#endif
        return 0;
    }

    PyObject* iterator = PyObject_GetIter(obj);
    if (!iterator) {
        return -1;
    }

#if PY_VERSION_HEX >= 0x030e0000
    int result;
    while ((result = PyIter_NextItem(iterator, &item)) > 0)
#else
    while ((item = PyIter_Next(iterator)))
#endif
    {
        if (count > 2)
        {
            PyErr_SetString(PyExc_ValueError,
                            "iterable must have exactly 3 items "
                            "(iterable contains more than 3)");
            Py_XDECREF(item);
            Py_XDECREF(tmp[0]);
            Py_XDECREF(tmp[1]);
            Py_XDECREF(tmp[2]);
            Py_DECREF(iterator);
            return -1;
        }

        tmp[count++] = item;
    }

#if PY_VERSION_HEX >= 0x030e0000
    if (result == -1)
#else
    if (PyErr_Occurred())
#endif
    {
        Py_XDECREF(tmp[0]);
        Py_XDECREF(tmp[1]);
        Py_XDECREF(tmp[2]);
        Py_XDECREF(iterator);
        return -1;
    }

    if (count != 3)
    {
        PyErr_Format(PyExc_ValueError,
            "iterable must have exactly 3 items, not %i",
            count);
        Py_XDECREF(tmp[0]);
        Py_XDECREF(tmp[1]);
        Py_XDECREF(tmp[2]);
        Py_DECREF(iterator);
        return -1;
    }

    items[0] = tmp[0];
    items[1] = tmp[1];
    items[2] = tmp[2];
    Py_DECREF(iterator);

    return 0;
}

// Parse items as doubles and populates array on success. If any item of
// items fails conversion to a C double then return -1 otherwise return 0.
// array is not modified unless the function succeeds.
static int
EVSpaceItems_AsArray(std::array<PyObject*, 3> items, varray_t& array)
{
    double tmp[3]{0.0};
    if (EVSpaceObject_AsDouble(items[0], tmp[0]) != items[0]) {
        return -1;
    }
    if (EVSpaceObject_AsDouble(items[1], tmp[1]) != items[1]) {
        return -1;
    }
    if (EVSpaceObject_AsDouble(items[2], tmp[2]) != items[2]) {
        return -1;
    }

    array[0] = tmp[0];
    array[1] = tmp[1];
    array[2] = tmp[2];

    return 0;
}

/* Rotation forward declarations */

static evspace::Vector* _EVSpaceRotate_From(const evspace::Matrix*, const evspace::Vector*);
static evspace::Vector* _EVSpaceRotate_From(const evspace::Matrix*, const evspace::Vector*,
                                            const evspace::Vector*);
static evspace::Vector* _EVSpaceRotate_To(const evspace::Matrix*, const evspace::Vector*);
static evspace::Vector* _EVSpaceRotate_To(const evspace::Matrix*, const evspace::Vector*,
                                          const evspace::Vector*);
static evspace::Vector* _EVSpaceRotate_Between(const evspace::Matrix*, const evspace::Matrix*,
                                               const evspace::Vector*, const evspace::Vector*,
                                               const evspace::Vector*);
static evspace::Matrix* _EVSpaceRotate_ComputeMatrix(double, evspace::AxisDirection);
static evspace::Matrix* _EVSpaceRotate_ComputeMatrix(double, const evspace::Vector*);
static evspace::Matrix* _EVSpaceRotate_ComputeMatrix(const EVSpace_Order*,
                                                     const EVSpace_Angles*, bool);
static evspace::Matrix* _EVSpaceRotate_ComputeMatrix(evspace::AxisDirection, evspace::AxisDirection,
                                                     evspace::AxisDirection, double, double, double,
                                                     bool);
static bool __EVSpaceRotate_CheckKeywordException(void);
static inline const char* __EVSpace_GetTypeName(PyObject*);

/* EVSpaceVector new functions */

static EVSpace_Vector*
_EVSpaceVector_New(PyTypeObject* type) noexcept
{
    EXCEPTION_WRAPPER(
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }

        self->vector = new evspace::Vector();
        return self;
    )
}

static EVSpace_Vector*
_EVSpaceVector_New(const varray_t array, PyTypeObject* type) noexcept
{
    EXCEPTION_WRAPPER(
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }

        self->vector = new evspace::Vector(array);
        return self;
    )
}

static EVSpace_Vector*
_EVSpaceVector_New(const evspace::Vector& vector, PyTypeObject* type) noexcept
{
    EXCEPTION_WRAPPER(
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }

        self->vector = new evspace::Vector(vector);
        return self;
    )
}

// Force call of the new constructor
static EVSpace_Vector*
_EVSpaceVector_New(evspace::Vector&& vector, PyTypeObject* type) noexcept
{
    EXCEPTION_WRAPPER(
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }

        self->vector = new evspace::Vector(std::move(vector));
        return self;
    )
}

/* Helper methods to _EVSpaceVector_New by not needing the PyTypeObject* argument */

static inline EVSpace_Vector*
EVSpaceVector_New() noexcept
{
    return _EVSpaceVector_New(&EVSpace_VectorType);
}

static inline EVSpace_Vector*
EVSpaceVector_New(const varray_t array) noexcept
{
    return _EVSpaceVector_New(array, &EVSpace_VectorType);
}

static inline EVSpace_Vector*
EVSpaceVector_New(const evspace::Vector& vector) noexcept
{
    return _EVSpaceVector_New(vector, &EVSpace_VectorType);
}

static inline EVSpace_Vector*
EVSpaceVector_New(evspace::Vector&& vector) noexcept
{
    return _EVSpaceVector_New(std::move(vector), &EVSpace_VectorType);
}

static PyObject*
Vector_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED)
{
    EVSpace_Vector* self = EVSpaceVector_New();
    
    return EVS_PyObject_Cast(self);
}

static int
Vector_init(EVSpace_Vector* self, PyObject* args, PyObject* Py_UNUSED)
{
    //  Possible constructor signatures:
    //  Vector.__init__() -> None: ...
    //  Vector.__init__(iterable: Iterable) -> None: ...
    //  Vector.__init__(x: float, y: float, z: float) -> None: ...

    varray_t buffer{};
    PyObject* parameter = NULL;

    Py_ssize_t tuple_size = PyTuple_Size(args);
    if (tuple_size == -1) {
        return -1;
    }
    else if (tuple_size == 0) {
        // self is already initialized to zero values
        return 0;
    }
    else if (tuple_size == 1)
    {
        // Single arg must be an iterable
        if (!PyArg_ParseTuple(args, "O:pyevspace.Vector.__init__", &parameter)) {
            return -1;
        }

        std::array<PyObject*, 3> items{NULL};
        if (EVSpaceIterable_GetItems(parameter, items) < 0) {
            return -1;
        }
        
        if (EVSpaceItems_AsArray(items, buffer) < 0) {
            Py_DECREF(items[0]);
            Py_DECREF(items[1]);
            Py_DECREF(items[2]);
            return -1;
        }
        
        EVSpaceVector_X(self) = buffer[0];
        EVSpaceVector_Y(self) = buffer[1];
        EVSpaceVector_Z(self) = buffer[2];
    }
    else {
        if (!PyArg_ParseTuple(args, "ddd", buffer.data(),
                               buffer.data() + 1, buffer.data() + 2)) {
            return -1;
        }
        EVSpaceVector_X(self) = buffer[0];
        EVSpaceVector_Y(self) = buffer[1];
        EVSpaceVector_Z(self) = buffer[2];
    }

    return 0;
}

static void
Vector_dealloc(EVSpace_Vector* self)
{
    delete self->vector;
    Py_TYPE(self)->tp_free(EVS_PyObject_Cast(self));

}

// Creates a python unicode string as a PyObject* from the C style format
// string on the data in self->vector. On failure an exception is set and
// NULL is returned, otherwise a PyObject* object equal to a unicode string
// object is returned.
static inline PyObject*
_EVSpaceVector_String(const EVSpace_Vector* self, const char* format)
{
    std::size_t buffer_size = snprintf(NULL, 0, format, EVSpaceVector_X(self),
        EVSpaceVector_Y(self), EVSpaceVector_Z(self));

    EXCEPTION_WRAPPER(
        char* buffer = new char[buffer_size + 1];
        if (sprintf(buffer, format, EVSpaceVector_X(self),
                    EVSpaceVector_Y(self), EVSpaceVector_Z(self)) < 0)
        {
            PyErr_SetString(PyExc_RuntimeError, "failed to fill buffer on string operator");
            delete [] buffer;
            return NULL;
        }

        PyObject* rtn = PyUnicode_FromString(buffer);
        delete [] buffer;

        return rtn;
    )
}

static PyObject*
Vector_str(const EVSpace_Vector* self)
{
    return _EVSpaceVector_String(self, "[%g, %g, %g]");
}

static PyObject*
Vector_repr(const EVSpace_Vector* self)
{
    return _EVSpaceVector_String(self, "Vector(%g, %g, %g)");
}

static PyObject*
Vector_iter(EVSpace_Vector* self)
{
    return PySeqIter_New(EVS_PyObject_Cast(self));
}

static PyObject*
Vector_richcompare(EVSpace_Vector* self, PyObject* other, int op)
{
    EVSpace_Vector* rhs;
    bool is_equal;
    
    if (EVSpaceVector_Check(other))
    {
        rhs = EVSpaceVector_Cast(other);
        is_equal = EVSpaceVector_VECTOR(self) == EVSpaceVector_VECTOR(rhs);

        if (op == Py_EQ)
        {
            if (is_equal) {
                Py_RETURN_TRUE;
            }
            else {
                Py_RETURN_FALSE;
            }
        }
        else if (op == Py_NE)
        {
            if (!is_equal) {
                Py_RETURN_TRUE;
            }
            else {
                Py_RETURN_FALSE;
            }
        }
    }

    Py_RETURN_NOTIMPLEMENTED;
}

/* Vector as number functions */

static PyObject*
Vector_add(EVSpace_Vector* self, PyObject* other)
{
    if (EVSpaceVector_Check(EVS_PyObject_Cast(self)) && EVSpaceVector_Check(other)) {
        EVSpace_Vector* other_vector = EVSpaceVector_Cast(other);

        evspace::Vector result = EVSpaceVector_VECTOR(self) + EVSpaceVector_VECTOR(other_vector);

        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result)));
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_subtract(EVSpace_Vector* self, PyObject* other)
{
    if (EVSpaceVector_Check(EVS_PyObject_Cast(self)) && EVSpaceVector_Check(other)) {
        EVSpace_Vector* other_vector = EVSpaceVector_Cast(other);

        EXCEPTION_WRAPPER(
            evspace::Vector result = EVSpaceVector_VECTOR(self) - EVSpaceVector_VECTOR(other_vector);

            return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result)));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_multiply(PyObject* lhs, PyObject* rhs)
{
    // Possible signatures:
    // __mul__(x: double) -> Vector: ...
    // __mul__(v: Vector) -> Vector: ...

    EVSpace_Vector* self;
    PyObject* scalar_result;
    double scalar;

    // __mul__
    if (PyObject_TypeCheck(lhs, &EVSpace_VectorType)) {
        self = EVSpaceVector_Cast(lhs);
        scalar_result = EVSpaceObject_AsDouble(rhs, scalar);
    }
    // __rmul__
    else
    {
        self = EVSpaceVector_Cast(rhs);
        scalar_result = EVSpaceObject_AsDouble(lhs, scalar);
    }

    if (!scalar_result) {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }

        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Vector result = EVSpaceVector_VECTOR(self) * scalar;
        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result)));
    )    
}

static PyObject*
Vector_matrix_multiply(EVSpace_Vector* self, PyObject* other)
{
    if (EVSpaceVector_Check(self)) {
        if (EVSpaceMatrix_Check(other))
        {
            EXCEPTION_WRAPPER(
                evspace::Vector result = EVSpaceVector_VECTOR(self) *
                                         EVSpaceMatrix_MATRIX(EVSpaceMatrix_Cast(other));
                return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result)));
            )
        }
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_divide(EVSpace_Vector* self, PyObject* other)
{
    PyObject* result;
    double scalar;

    if (EVSpaceVector_Check(self))
    {
        result = EVSpaceObject_AsDouble(other, scalar);
        if (!result) {
            if (PyErr_ExceptionMatches(PyExc_TypeError)) {
                PyErr_Clear();
                Py_RETURN_NOTIMPLEMENTED;
            }

            return NULL;
        }
        
        EXCEPTION_WRAPPER(
            evspace::Vector answer = EVSpaceVector_VECTOR(self) / scalar;
            return EVS_PyObject_Cast(EVSpaceVector_New(answer));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_add(EVSpace_Vector* self, PyObject* other)
{
    EVSpace_Vector* rhs = EVSpaceVector_Cast(other);
    if (EVSpaceVector_Check(self) && EVSpaceVector_Check(other))
    {
        EVSpaceVector_VECTOR(self) += EVSpaceVector_VECTOR(rhs);

#if PY_VERSION_HEX >= 0x030a0000
            return Py_NewRef(self);
#else
            Py_INCREF(self);
            return EVS_PyObject_Cast(self);
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_subtract(EVSpace_Vector* self, PyObject* other)
{
    EVSpace_Vector* rhs = EVSpaceVector_Cast(other);
    if (EVSpaceVector_Check(self) && EVSpaceVector_Check(other))
    {
        EVSpaceVector_VECTOR(self) -= EVSpaceVector_VECTOR(rhs);

#if PY_VERSION_HEX >= 0x030a0000
            return Py_NewRef(self);
#else
            Py_INCREF(self);
            return EVS_PyObject_Cast(self);
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_multiply(EVSpace_Vector* self, PyObject* other)
{
    double scalar;
    PyObject* result;

    if (EVSpaceVector_Check(self))
    {
        result = EVSpaceObject_AsDouble(other, scalar);

        if (!result)
        {
            if (PyErr_ExceptionMatches(PyExc_TypeError)) {
                PyErr_Clear();
                Py_RETURN_NOTIMPLEMENTED;
            }

            return NULL;
        }

        EVSpaceVector_VECTOR(self) *= scalar;

#if PY_VERSION_HEX >= 0x030a0000
        return Py_NewRef(self);
#else
        Py_INCREF(self);
        return (PyObject*)self;
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_multiply_matrix(EVSpace_Vector* self, PyObject* arg)
{
    EVSpace_Matrix* rhs;
    if (EVSpaceVector_Check(self) && EVSpaceVector_Check(arg)) {
        rhs = EVSpaceMatrix_Cast(arg);
        EVSpaceVector_VECTOR(self) *= EVSpaceMatrix_MATRIX(rhs);

#if PY_VERSION_HEX >= 0x030a0000
        return Py_NewRef(self);
#else
        Py_INCREF(self);
        return (PyObject*)self;
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_divide(EVSpace_Vector* self, PyObject* other)
{
    double scalar;

    if (EVSpaceVector_Check(self))
    {
        if (!EVSpaceObject_AsDouble(other, scalar))
        {
            if (PyErr_ExceptionMatches(PyExc_TypeError)) {
                PyErr_Clear();
                Py_RETURN_NOTIMPLEMENTED;
            }

            return NULL;
        }

        EVSpaceVector_VECTOR(self) /= scalar;

#if PY_VERSION_HEX >= 0x030a0000
        return Py_NewRef(self);
#else
        Py_INCREF(self);
        return (PyObject*)self;
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_negative(EVSpace_Vector* self)
{
    if (!EVSpaceVector_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "unary '__neg__' requires a pyevspace.Vector object");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Vector result = -EVSpaceVector_VECTOR(self);

        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result)));
    )
}

/* Vector sequence functions */

static Py_ssize_t
Vector_length(EVSpace_Vector* self)
{
    return 3;
}

static PyObject*
Vector_get_item(EVSpace_Vector* self, Py_ssize_t index)
{
    if (index < 0 || index > 2)
    {
        PyErr_Format(PyExc_IndexError,
                     "index (%i) must be in [0-2]",
                    index);
        return NULL;
    }

    try {
        double item = EVSpaceVector_VECTOR(self)[index];
        return PyFloat_FromDouble(item);
    }
    catch (const std::out_of_range&) {
        PyErr_Format(PyExc_IndexError, "index (%i) out of bounds", index);
        return NULL;
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static int
Vector_set_item(EVSpace_Vector* self, Py_ssize_t index, PyObject* arg)
{
    if (index < 0 || index > 2)
    {
        PyErr_Format(PyExc_IndexError,
                     "index (%i) must be in [0-2]",
                     index);
        return -1;
    }

    double value;
    if (!EVSpaceObject_AsDouble(arg, value)) {
        return -1;
    }

    try {
        EVSpaceVector_VECTOR(self)[index] = value;
        return 0;
    }
    catch (const std::out_of_range&) {
        PyErr_Format(PyExc_IndexError, "index (%i) out of bounds", index);
        return -1;
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
}

/* Vector buffer functions */

static int
Vector_get_buffer(EVSpace_Vector* obj, Py_buffer* view, int flags)
{
    if (!view)
    {
        PyErr_SetString(PyExc_ValueError, "NULL view in getbuffer");
        return -1;
    }

    Py_ssize_t* shape = reinterpret_cast<Py_ssize_t*>(malloc(sizeof(Py_ssize_t)));
    if (!shape) {
        PyErr_NoMemory();
        return -1;
    }
    *shape = 3;

    int* internal = reinterpret_cast<int*>(malloc(sizeof(int)));
    if (!internal) {
        PyErr_NoMemory();
        return -1;
    }
    *internal = EVS_BUFFER_NO_STRIDES;

#if PY_VERSION_HEX >= 0x030a0000
    view->obj           = Py_NewRef(EVS_PyObject_Cast(obj));
#else
    Py_INCREF(obj);
    view->obj           = EVS_PyObject_Cast(obj);
#endif
    view->buf           = EVSpaceVector_VECTOR(obj).data().data();
    view->len           = sizeof(double) * 3;
    view->readonly      = 0;
    view->itemsize      = sizeof(double);
    view->ndim          = 1;

    if (flags & PyBUF_ND) {
        view->shape     = shape;
        *internal |= EVS_BUFFER_RELEASE_SHAPE;
    }
    else {
        view->shape    = NULL;
        delete shape;
        *internal |= EVS_BUFFER_NO_SHAPE;
    }

    if (flags & PyBUF_STRIDES) {
        view->strides   = &view->itemsize;
    }
    else {
        view->strides   = NULL;
    }
    
    if (flags & PyBUF_FORMAT) {
        view->format    = (char*)"d";
    }
    else {
        view->format    = NULL;
    }

    view->suboffsets    = NULL;
    view->internal      = reinterpret_cast<void*>(internal);

    return 0;
}

/* Vector class methods */

static PyObject*
Vector_magnitude(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
    if (!EVSpaceVector_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "calling object must be pyevspace.Vector type");
        return NULL;
    }

    return PyFloat_FromDouble(EVSpaceVector_VECTOR(self).magnitude());
}

static PyObject*
Vector_magnitude_square(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
    if (!EVSpaceVector_Check(self))
    {
        PyErr_SetString(PyExc_TypeError, "calling object must be pyevspace.Vector type");
        return NULL;
    }

    return PyFloat_FromDouble(EVSpaceVector_VECTOR(self).magnitude_squared());
}

static PyObject*
Vector_normalize(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
    if (!EVSpaceVector_Check(self))
    {
        PyErr_SetString(PyExc_TypeError, "calling object must be pyevspace.Vector type");
        return NULL;
    }

    EVSpaceVector_VECTOR(self).normalize();

    Py_RETURN_NONE;
}

static PyObject*
Vector_norm(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
    if (!EVSpaceVector_Check(self))
    {
        PyErr_SetString(PyExc_TypeError, "calling object must be pyevspace.Vector type");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Vector norm = EVSpaceVector_VECTOR(self).norm();
        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(norm)));
    )
}

static PyObject*
Vector_reduce(EVSpace_Vector* self, PyObject* Py_UNUSED)
{
    return Py_BuildValue("(O(ddd))", Py_TYPE(self), EVSpaceVector_X(self),
                         EVSpaceVector_Y(self), EVSpaceVector_Z(self));
}

static PyObject*
Vector_compare_to_tol(EVSpace_Vector* self, PyObject* args)
{
    if (!EVSpaceVector_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "calling object must be Vector type");
        return NULL;
    }

    double abs_tol = evspace::DEFAULT_ABS_TOL, rel_tol = evspace::DEFAULT_REL_TOL;
    EVSpace_Vector* rhs;
    if (!PyArg_ParseTuple(args, "O!|dd:compare_to_tol", &EVSpace_VectorType,
                          &rhs, &rel_tol, &abs_tol)) {
        return NULL;
    }

    if (rel_tol < 0) {
        PyErr_SetString(PyExc_ValueError, "rel_tol must be non-negative");
        return NULL;
    }
    if (abs_tol < 0) {
        PyErr_SetString(PyExc_ValueError, "abs_tol must be non-negative");
        return NULL;
    }

    if (EVSpaceVector_VECTOR(self).compare_to(EVSpaceVector_VECTOR(rhs),
                                              rel_tol, abs_tol)) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }

}

static PyObject*
Vector_compare_to_ulp(EVSpace_Vector* self, PyObject* args)
{
    if (!EVSpaceVector_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "calling object must be Vector type");
        return NULL;
    }
    
    int max_ulps;
    EVSpace_Vector* rhs;
    if (!PyArg_ParseTuple(args, "O!i:compare_to_ulp", &EVSpace_VectorType, &rhs, &max_ulps)) {
        return NULL;
    }

    if (max_ulps < 0) {
        PyErr_Format(PyExc_ValueError, "max_ulps must be non-negative (got %i)", max_ulps);
        return NULL;
    }

    if (EVSpaceVector_VECTOR(self).compare_to(EVSpaceVector_VECTOR(rhs), max_ulps)) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

/* Vector related module level functions */

static PyObject*
Vector_dot(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.dot() expected exactly 2 arguments (%i given)", size);
        return NULL;
    }

    if (!EVSpaceVector_Check(args[0]))
    {
        PyErr_SetString(PyExc_TypeError, "first argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!EVSpaceVector_Check(args[1]))
    {
        PyErr_SetString(PyExc_TypeError, "second argument must be pyevspace.Vector type");
        return NULL;
    }

    EVSpace_Vector* lhs = EVSpaceVector_Cast(args[0]);
    EVSpace_Vector* rhs = EVSpaceVector_Cast(args[1]);
    double dot_product = evspace::vector_dot(
        EVSpaceVector_VECTOR(lhs),
        EVSpaceVector_VECTOR(rhs)
    );

    return PyFloat_FromDouble(dot_product);
}

static PyObject*
Vector_cross(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.cross() expected exactly 2 arguments "
                                      "(%i given)", size);
        return NULL;
    }

    if (!EVSpaceVector_Check(args[0]))
    {
        PyErr_SetString(PyExc_TypeError, "first argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!EVSpaceVector_Check(args[1]))
    {
        PyErr_SetString(PyExc_TypeError, "second argument must be pyevspace.Vector type");
        return NULL;
    }

    EVSpace_Vector* lhs = EVSpaceVector_Cast(args[0]);
    EVSpace_Vector* rhs = EVSpaceVector_Cast(args[1]);

    EXCEPTION_WRAPPER(
        evspace::Vector cross_product = evspace::vector_cross(
            EVSpaceVector_VECTOR(lhs), EVSpaceVector_VECTOR(rhs)
        );
        
        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(cross_product)));
    )
}

static PyObject*
Vector_angle(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.vang() expected exactly 1 argument "
                                      "(%i given)", size);
        return NULL;
    }

    if (!EVSpaceVector_Check(args[0]))
    {
        PyErr_SetString(PyExc_TypeError, "first argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!EVSpaceVector_Check(args[1]))
    {
        PyErr_SetString(PyExc_TypeError, "second argument must be pyevspace.Vector type");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        EVSpace_Vector* lhs = EVSpaceVector_Cast(args[0]);
        EVSpace_Vector* rhs = EVSpaceVector_Cast(args[1]);
        double angle = evspace::vector_angle(
            EVSpaceVector_VECTOR(lhs), EVSpaceVector_VECTOR(rhs)
        );

        return PyFloat_FromDouble(angle);
    )
}

static PyObject*
Vector_exclude(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.vector_exclude() expected exactly "
                                      "1 argument (%i given)", size);
        return NULL;
    }

    if (!EVSpaceVector_Check(args[0]))
    {
        PyErr_SetString(PyExc_TypeError, "first argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!EVSpaceVector_Check(args[1]))
    {
        PyErr_SetString(PyExc_TypeError, "second argument must be pyevspace.Vector type");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        EVSpace_Vector* lhs = EVSpaceVector_Cast(args[0]);
        EVSpace_Vector* rhs = EVSpaceVector_Cast(args[1]);
        evspace::Vector exclude = evspace::vector_exclude(
            EVSpaceVector_VECTOR(lhs), EVSpaceVector_VECTOR(rhs)
        );

        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(exclude)));
    )
}

static PyObject*
Vector_projection(PyObject* Py_UNUSED, PyObject* const* args, Py_ssize_t size)
{
    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.vector_proj() expected exactly "
                                      "2 arguments (%i given)", size);
        return NULL;
    }

    if (!EVSpaceVector_Check(args[0]))
    {
        PyErr_SetString(PyExc_TypeError, "first argument must be pyevspace.Vector type");
        return NULL;
    }
    if (!EVSpaceVector_Check(args[1]))
    {
        PyErr_SetString(PyExc_TypeError, "second argument must by pyevspace.Vector type");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        EVSpace_Vector* lhs = EVSpaceVector_Cast(args[0]);
        EVSpace_Vector* rhs = EVSpaceVector_Cast(args[1]);
        evspace::Vector projection = evspace::vector_projection(
            EVSpaceVector_VECTOR(lhs), EVSpaceVector_VECTOR(rhs)
        );

        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(projection)));
    )
}

PyDoc_STRVAR(vector_doc, "Vector([{x, y, z | iterable}])\n\
\n\
The Vector can be constructed with an iterable of length 4, or\n\
directly with the x, y, and z components. All components must be\n\
numeric types. Alternatively the components will default to zero\n\
if no argument is specified.");

static PyNumberMethods vector_as_number;

static PySequenceMethods vector_as_sequence = {
    (lenfunc)Vector_length,                 /* sq_length */
    0,                                      /* sq_concat */
    0,                                      /* sq_repeat */
    (ssizeargfunc)Vector_get_item,          /* sq_item */
    0,                                      /* was_sq_slice */
    (ssizeobjargproc)Vector_set_item,       /* sq_ass_item */
};

static PyBufferProcs vector_buffer = {
    (getbufferproc)Vector_get_buffer,       /* bf_getbuffer */
    (releasebufferproc)EVSpaceBuffer_Release /* bf_releasebuffer */
};

PyDoc_STRVAR(vector_mag_doc, "magnitude() -> float\n\
\n\
Computes the vector magnitude of the calling pyevspace.Vector object.");

PyDoc_STRVAR(vector_mag2_doc, "magnitude_squared() -> float\n\
\n\
Computes the square of the magnitude of a vector. Eliminates\n\
round-off error when squaring the result of pyevspace.Vector.magnitude().");

PyDoc_STRVAR(vector_normalize_doc, "normalize() -> None\n\
\n\
Modifies the calling pyevspace.Vector to a length of 1, while preserving\n\
its direction.");

PyDoc_STRVAR(vector_norm_doc, "norm() -> Vector\n\
\n\
Creates a normalized version of a pyevspace.Vector. Differs from\n\
pyevspace.Vector.normalize() by returning a new pyevspace.Vector\n\
and the calling object remains unchanged.");

PyDoc_STRVAR(vector_reduce_doc, "__reduce__() -> (cls, (x, y, z))\n\
\n\
Allows pickling of the pyevspace.Vector type.");

PyDoc_STRVAR(vector_compare_to_ulp_doc, "compare_to_ulp(rhs: Vector, max_ulps: int) -> bool\n\
\n\
Compares self to rhs using component wise ULP based mechanics. self is\n\
considered equal to rhs if, for each respsective component, the difference\n\
between binary representations is less than or equal to max_ulps ULPs.");

PyDoc_STRVAR(vector_compare_to_tol_doc, "compare_to_tol(rhs: Vector, rel_tol: float = 1e-9, abs_tol: float = 1e-15) -> bool\n\
\n\
Compare self to rhs using absolute and relative tolerances. self is \n\
considered equal to rhs if, for each respective component, the difference is\n\
within abs_tol + rel_tol * max(abs(v_i), abs(u_i)).");

static PyMethodDef vector_methods[] = {

    {"magnitude", (PyCFunction)Vector_magnitude, METH_NOARGS, vector_mag_doc},

    {"magnitude_squared", (PyCFunction)Vector_magnitude_square, METH_NOARGS, vector_mag2_doc},

    {"normalize", (PyCFunction)Vector_normalize, METH_NOARGS, vector_normalize_doc},

    {"norm", (PyCFunction)Vector_norm, METH_NOARGS, vector_norm_doc},

    {"__reduce__", (PyCFunction)Vector_reduce, METH_NOARGS, vector_reduce_doc},

    {"compare_to_tol", (PyCFunction)Vector_compare_to_tol, METH_VARARGS, vector_compare_to_tol_doc},

    {"compare_to_ulp", (PyCFunction)Vector_compare_to_ulp, METH_VARARGS, vector_compare_to_ulp_doc},

    {NULL}
};

/* EVSpace_MatrixView */

static EVSpace_MatrixView*
EVSpaceMatrixView_New(EVSpace_Matrix* base, std::size_t ndim, std::size_t offset,
                      Py_ssize_t shape0, Py_ssize_t shape1,
                      Py_ssize_t strides0, Py_ssize_t strides1)
{
    EVSpace_MatrixView* view = reinterpret_cast<EVSpace_MatrixView*>(
        EVSpace_MatrixViewType.tp_alloc(&EVSpace_MatrixViewType, 0)
    );

    view->base = base;
    view->data = EVSpaceMatrix_MATRIX(base).data().data() + offset;
    view->shape = reinterpret_cast<Py_ssize_t*>(malloc(ndim * sizeof(Py_ssize_t)));
    if (!view->shape) {
        view->ob_base.ob_type->tp_dealloc(EVS_PyObject_Cast(view));
        PyErr_NoMemory();
        return NULL;
    }
    view->strides = reinterpret_cast<Py_ssize_t*>(malloc(ndim * sizeof(Py_ssize_t)));
    if (!view->strides) {
        view->ob_base.ob_type->tp_dealloc(EVS_PyObject_Cast(view));
        PyErr_NoMemory();
        free(view->shape);
        return NULL;
    }

    view->shape[0] = shape0;
    view->strides[0] = strides0;
    if (ndim > 1) {
        view->shape[1] = shape1;
        view->strides[1] = strides1;
    }
    view->ndim = ndim;

    Py_INCREF(base);
    return view;
}

static void
MatrixView_dealloc(EVSpace_MatrixView* self)
{
    if (self->shape != NULL) {
        free(self->shape);
    }
    if (self->strides != NULL) {
        free(self->strides);
    }
    self->data = NULL;

    Py_DECREF(self->base);
    Py_TYPE(self)->tp_free(self);
}

static int
MatrixView_GetBuffer(EVSpace_MatrixView* obj, Py_buffer* view, int flags)
{
#if PY_VERSION_HEX >= 0x030a0000
    view->obj           = Py_NewRef(obj);
#else
    Py_INCREF(obj);
    view->obj           = EVS_PyObject_Cast(obj);
#endif
    view->buf           = obj->data;
    view->readonly      = 0;
    view->itemsize      = sizeof(double);
    view->shape         = (flags & PyBUF_ND) ? obj->shape : NULL;
    view->strides       = (flags & PyBUF_STRIDES) ? obj->strides : NULL;
    view->format        = (flags & PyBUF_FORMAT) ? (char*)"d" : NULL;
    view->suboffsets    = NULL;
    view->ndim          = obj->ndim;

    // len = product(shape) * itemsize
    view->len           = sizeof(double) * obj->shape[0];
    if (obj->ndim > 1) {
        view->len *= obj->shape[1];
    }

    return 0;
}

static PyBufferProcs matrixview_as_buffer = {
    (getbufferproc)MatrixView_GetBuffer,    /* bf_getbuffer */
    NULL,                                   /* bf_releasebuffer */
};

/* EVSpace_Matrix constructors */

static EVSpace_Matrix*
_EVSpaceMatrix_New(PyTypeObject* type) noexcept
{
    EVSpace_Matrix* self = EVSpaceMatrix_Cast(type->tp_alloc(type, 0));
    if (!self) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        self->matrix = new evspace::Matrix();
        return self;
    )
}

static EVSpace_Matrix*
_EVSpaceMatrix_New(const marray_t array, PyTypeObject* type) noexcept
{
    EVSpace_Matrix* self = EVSpaceMatrix_Cast(type->tp_alloc(type, 0));
    if (!self) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        self->matrix = new evspace::Matrix(array);
        return self;
    )
}

static EVSpace_Matrix*
_EVSpaceMatrix_New(const evspace::Matrix& matrix, PyTypeObject* type) noexcept
{
    EVSpace_Matrix* self = EVSpaceMatrix_Cast(type->tp_alloc(type, 0));
    if (!self) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        self->matrix = new evspace::Matrix(matrix);
        return self;
    )
}

static EVSpace_Matrix*
_EVSpaceMatrix_New(evspace::Matrix&& matrix, PyTypeObject* type) noexcept
{
    EVSpace_Matrix* self = EVSpaceMatrix_Cast(type->tp_alloc(type, 0));
    if (!self) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        self->matrix = new evspace::Matrix(std::move(matrix));
        return self;
    )
}

/* Constructor wrappers that provide the `type` argument to constructors above */
static EVSpace_Matrix*
EVSpaceMatrix_New() noexcept
{
    return _EVSpaceMatrix_New(&EVSpace_MatrixType);
}

static EVSpace_Matrix*
EVSpaceMatrix_New(const marray_t array) noexcept
{
    return _EVSpaceMatrix_New(array, &EVSpace_MatrixType);
}

static EVSpace_Matrix*
EVSpaceMatrix_New(const evspace::Matrix& matrix) noexcept
{
    return _EVSpaceMatrix_New(matrix, &EVSpace_MatrixType);
}

static EVSpace_Matrix*
EVSpaceMatrix_New(evspace::Matrix&& matrix) noexcept
{
    return _EVSpaceMatrix_New(std::move(matrix), &EVSpace_MatrixType);
}

static PyObject*
Matrix_new(PyTypeObject* type, PyObject* arg, PyObject* Py_UNUSED)
{
    marray_t array{};
    EVSpace_Matrix* self = EVSpaceMatrix_New(array);

    return EVS_PyObject_Cast(self);
}

static int
Matrix_init(EVSpace_Matrix* self, PyObject* args, PyObject* Py_UNUSED)
{
    // Possible signatures:
    // Matrix.__init__()
    // Matrix.__init__(row1: Iterable, row2: Iterable, row3: Iterable)
    // Matrix.__init__(array: Iterable, Iterable, Iterable)

    Py_ssize_t tuple_size = PyTuple_Size(args);
    if (tuple_size == -1) {
        return -1;
    }
    else if (tuple_size == 0) {
        // self is already initialized to 0's
        return 0;
    }
    else if (tuple_size != 3 && tuple_size != 1) {
        PyErr_Format(PyExc_ValueError, "expected 0 or 3 argument, not %i", tuple_size);
        return -1;
    }

    std::array<PyObject*, 3> items{NULL};
    PyObject* container;

    if (tuple_size == 1) {
        // strip the outer container from argument
        if (!PyArg_ParseTuple(args, "O:pyevspace.Matrix.__init__", &container)) {
            return -1;
        }
    }
    else {
        container = args;
    }

    if (EVSpaceIterable_GetItems(container, items) < 0) {
        return -1;
    }

    std::array<PyObject*, 3> sub_items{NULL};
    std::array<varray_t, 3> values;
    int result;

    for (int i = 0; i < 3; i++)
    {
        if (EVSpaceIterable_GetItems(items[i], sub_items) < 0) {
            Py_DECREF(items[0]);
            Py_DECREF(items[1]);
            Py_DECREF(items[2]);
            return -1;
        }

        result = EVSpaceItems_AsArray(sub_items, values[i]);
        Py_DECREF(sub_items[0]);
        Py_DECREF(sub_items[1]);
        Py_DECREF(sub_items[2]);

        if (result < 0)
        {
            Py_DECREF(items[0]);
            Py_DECREF(items[1]);
            Py_DECREF(items[2]);
            return -1;
        }
    }

    Py_DECREF(items[0]);
    Py_DECREF(items[1]);
    Py_DECREF(items[2]);

    try {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                EVSpaceMatrix_MATRIX(self)(i, j) = values[i][j];
            }
        }
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }

    return 0;
}

static void
Matrix_dealloc(EVSpace_Matrix* self)
{
    delete self->matrix;
    Py_TYPE(self)->tp_free(EVS_PyObject_Cast(self));
}

static inline PyObject*
_EVSpaceMatrix_String(const EVSpace_Matrix* matrix, const char* format)
{
    const size_t buffer_size = snprintf(NULL, 0, format,
        EVSpaceMatrix_MATRIX(matrix)(0, 0), EVSpaceMatrix_MATRIX(matrix)(0, 1),
        EVSpaceMatrix_MATRIX(matrix)(0, 2), EVSpaceMatrix_MATRIX(matrix)(1, 0),
        EVSpaceMatrix_MATRIX(matrix)(1, 1), EVSpaceMatrix_MATRIX(matrix)(1, 2),
        EVSpaceMatrix_MATRIX(matrix)(2, 0), EVSpaceMatrix_MATRIX(matrix)(2, 1),
        EVSpaceMatrix_MATRIX(matrix)(2, 2)
    );
    char* buffer = reinterpret_cast<char*>(malloc(buffer_size + 1));
    if (!buffer) {
        return PyErr_NoMemory();
    }

    if (sprintf(buffer, format, EVSpaceMatrix_MATRIX(matrix)(0, 0),
        EVSpaceMatrix_MATRIX(matrix)(0, 1), EVSpaceMatrix_MATRIX(matrix)(0, 2),
        EVSpaceMatrix_MATRIX(matrix)(1, 0), EVSpaceMatrix_MATRIX(matrix)(1, 1),
        EVSpaceMatrix_MATRIX(matrix)(1, 2), EVSpaceMatrix_MATRIX(matrix)(2, 0),
        EVSpaceMatrix_MATRIX(matrix)(2, 1), EVSpaceMatrix_MATRIX(matrix)(2, 2)) < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "error filling string buffer");
        delete buffer;
        return NULL;
    }
    PyObject* rtn = PyUnicode_FromString(buffer);
    delete buffer;

    return rtn;
}

static PyObject*
Matrix_str(const EVSpace_Matrix* self)
{
    return _EVSpaceMatrix_String(self, "[[%g, %g, %g]\n[%g, %g, %g]\n[%g, %g, %g]]");
}

static PyObject*
Matrix_repr(const EVSpace_Matrix* self)
{
    return _EVSpaceMatrix_String(self, "Matrix([%g, %g, %g], [%g, %g, %g], [%g, %g, %g])");
}

static PyObject*
Matrix_richcompare(EVSpace_Matrix* self, PyObject* other, int op)
{
    if (EVSpaceMatrix_Check(other))
    {
        bool result = (EVSpaceMatrix_MATRIX(self) ==
                       EVSpaceMatrix_MATRIX(EVSpaceMatrix_Cast(other)));

        if (op == Py_EQ)
        {
            if (result) {
                Py_RETURN_TRUE;
            }
            else {
                Py_RETURN_FALSE;
            }
        }
        else if (op == Py_NE)
        {
            if (result) {
                Py_RETURN_FALSE;
            }
            else {
                Py_RETURN_TRUE;
            }
        }
    }

    Py_RETURN_NOTIMPLEMENTED;
}

/* Matrix number method */

static PyObject*
Matrix_add(EVSpace_Matrix* self, PyObject* other) noexcept
{
    EVSpace_Matrix* rhs = EVSpaceMatrix_Cast(other);
    if (EVSpaceMatrix_Check(self) && EVSpaceMatrix_Check(rhs))
    {
        EXCEPTION_WRAPPER(
            evspace::Matrix result = EVSpaceMatrix_MATRIX(self) + EVSpaceMatrix_MATRIX(rhs);
            return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result)));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_subtract(EVSpace_Matrix* self, PyObject* other) noexcept
{
    EVSpace_Matrix* rhs = EVSpaceMatrix_Cast(other);
    if (EVSpaceMatrix_Check(self) && EVSpaceMatrix_Check(rhs))
    {
        EXCEPTION_WRAPPER(
            evspace::Matrix result = EVSpaceMatrix_MATRIX(self) - EVSpaceMatrix_MATRIX(rhs);
            return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result)));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_multiply(PyObject* lhs, PyObject* rhs)
{
    PyObject* scalar_result;
    double scalar;
    
    EVSpace_Matrix* self;
    // Matrix.__mul__(rhs)
    if (EVSpaceMatrix_Check(lhs))
    {
        self = EVSpaceMatrix_Cast(lhs);
        scalar_result = EVSpaceObject_AsDouble(rhs, scalar);
    }
    // lhs.__mul__(self)
    else
    {
        self = EVSpaceMatrix_Cast(rhs);
        scalar_result = EVSpaceObject_AsDouble(lhs, scalar);
    }

    if (!scalar_result) {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }

        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Matrix result = EVSpaceMatrix_MATRIX(self) * scalar;
        return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result)));
    )
}

static PyObject*
Matrix_matrix_multiply(EVSpace_Matrix* self, PyObject* other)
{
    if (EVSpaceMatrix_Check(self))
    {
        if (EVSpaceMatrix_Check(other))
        {
            EXCEPTION_WRAPPER(
                evspace::Matrix result = EVSpaceMatrix_MATRIX(self) *
                                         EVSpaceMatrix_MATRIX(EVSpaceMatrix_Cast(other));
                return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result)));
            )
        }
        else if (EVSpaceVector_Check(other))
        {
            EXCEPTION_WRAPPER(
                evspace::Vector result = EVSpaceMatrix_MATRIX(self) *
                                         EVSpaceVector_VECTOR(EVSpaceVector_Cast(other));
                return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result)));
            )
        }
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_divide(EVSpace_Matrix* self, PyObject* other)
{
    double scalar;

    if (EVSpaceMatrix_Check(self))
    {
        if (!EVSpaceObject_AsDouble(other, scalar))
        {
            if (PyErr_ExceptionMatches(PyExc_TypeError)) {
                PyErr_Clear();
                Py_RETURN_NOTIMPLEMENTED;
            }

            return NULL;
        }

        EXCEPTION_WRAPPER(
            evspace::Matrix result = EVSpaceMatrix_MATRIX(self) / scalar;
            return EVS_PyObject_Cast(EVSpaceMatrix_New(result));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_inplace_add(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_Matrix* rhs = EVSpaceMatrix_Cast(other);
    if (EVSpaceMatrix_Check(self) && EVSpaceMatrix_Check(rhs))
    {
        EVSpaceMatrix_MATRIX(self) += EVSpaceMatrix_MATRIX(rhs);

#if PY_VERSION_HEX >= 0x030a0000
        return Py_NewRef(self);
#else
        Py_INCREF(self);
        return EVS_PyObject_Cast(self);
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_inplace_subtract(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_Matrix* rhs = EVSpaceMatrix_Cast(other);

    if (EVSpaceMatrix_Check(self) && EVSpaceMatrix_Check(rhs))
    {
        EVSpaceMatrix_MATRIX(self) -= EVSpaceMatrix_MATRIX(rhs);

#if PY_VERSION_HEX >= 0x030a0000
        return Py_NewRef(self);
#else
        Py_INCREF(self);
        return EVS_PyObject_Cast(self);
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_inplace_multiply(EVSpace_Matrix* self, PyObject* other)
{
    double scalar;

    if (EVSpaceMatrix_Check(self))
    {
        if (!EVSpaceObject_AsDouble(other, scalar))
        {
            if (PyErr_ExceptionMatches(PyExc_TypeError)) {
                PyErr_Clear();
                Py_RETURN_NOTIMPLEMENTED;
            }

            return NULL;
        }

        EVSpaceMatrix_MATRIX(self) *= scalar;

#if PY_VERSION_HEX >= 0x030a0000
        return Py_NewRef(self);
#else
        Py_INCREF(self);
        return EVS_PyObject_Cast(self);
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_inplace_multiply_matrix(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_Matrix* rhs = EVSpaceMatrix_Cast(other);

    if (EVSpaceMatrix_Check(self))
    {
        if (EVSpaceMatrix_Check(other))
        {
            EVSpaceMatrix_MATRIX(self) *= EVSpaceMatrix_MATRIX(rhs);

#if PY_VERSION_HEX >= 0x030a0000
            return Py_NewRef(self);
#else
            Py_INCREF(self);
            return EVS_PyObject_Cast(self);
#endif
        }
        else if (EVSpaceVector_Check(other))
        {
            // Python will fall back to self = self * other when NotImplemented
            // is returned, so we must force the TypeError here.
            PyErr_Format(PyExc_TypeError,
                         "unsupported operand type(S) for @=: '%s' and '%s'",
                         self->ob_base.ob_type->tp_name,
                         EVSpace_VectorType.tp_name);
            return NULL;
        }
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_inplace_divide(EVSpace_Matrix* self, PyObject* other)
{
    double scalar;

    if (EVSpaceMatrix_Check(self))
    {
        if (!EVSpaceObject_AsDouble(other, scalar))
        {
            if (PyErr_ExceptionMatches(PyExc_TypeError)) {
                PyErr_Occurred();
                Py_RETURN_NOTIMPLEMENTED;
            }

            return NULL;
        }

        EVSpaceMatrix_MATRIX(self) /= scalar;

#if PY_VERSION_HEX >= 0x030a0000
        return Py_NewRef(self);
#else
        Py_INCREF(self);
        return EVS_PyObject_Cast(self);
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_negative(EVSpace_Matrix* self)
{
    if (!EVSpaceMatrix_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "pyevspace.Matrix.__neg__ must be called "
                                         "on pyevspace.Matrix type");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Matrix result = -EVSpaceMatrix_MATRIX(self);
        return EVS_PyObject_Cast(EVSpaceMatrix_New(result));
    )
}

/* Mapping and buffer methods */

static Py_ssize_t
Matrix_length(EVSpace_Matrix* self)
{
    return 3;
}

static int
_EVSpaceMatrix_GetBuffer(EVSpace_Matrix* obj, Py_buffer* view)
{
    // Sets up a basic Py_buffer view with the caller needing to fill
    // in the more detailed attributes. Return -1 on error.

    int* internal = reinterpret_cast<int*>(malloc(sizeof(int)));
    if (!internal)
    {
        PyErr_NoMemory();
        return -1;
    }
    *internal = EVS_BUFFER_EMPTY;

#if PY_VERSION_HEX >= 0x030a0000
    view->obj           = Py_NewRef(obj);
#else
    Py_INCREF(obj);
    view->obj           = EVS_PyObject_Cast(obj);
#endif
    view->buf           = EVSpaceMatrix_MATRIX(obj).data().data();
    view->len           = sizeof(double) * 9;
    view->readonly      = 0;
    view->itemsize      = sizeof(double);
    view->suboffsets    = NULL;
    view->internal      = reinterpret_cast<void*>(internal);

    return 0;
}

// Converts `obj` to an index value using __index__() if necessary. If an exception
// that should be propagated (any exception other than PyExc_TypeError) is encountered
// return -1. If a PyExc_TypeError is encountered return 0 with the exception state
// cleared, otherwise fill `index` with the converted value and return 1.
static int
_EVSpace_AsIndex(PyObject* obj, Py_ssize_t* index)
{
    Py_ssize_t tmp_index, stop;
    PyObject* as_index = PyNumber_Index(obj);

    if (!as_index) {
        PyErr_Clear();
        return 0;
    }

    tmp_index = PyLong_AsSsize_t(as_index);
    Py_DECREF(as_index);
    if (tmp_index == -1 && PyErr_Occurred())
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError)) {
            PyErr_Clear();
            return 0;
        }
        else {   
            return -1;
        }
    }

    stop = tmp_index + 1;
    PySlice_AdjustIndices(3, &tmp_index, &stop, 1);
    if (tmp_index == stop)
    {
        PyErr_Format(PyExc_IndexError, "index %i out of bounds, must be in [0-2]", tmp_index);
        return -1;
    }

    *index = tmp_index;
    return 1;
}

static PyObject*
Matrix_map_subscript(EVSpace_Matrix* self, PyObject* indices)
{
    Py_ssize_t index0, index1, result;
    PyObject* key0, *key1, *slice0, *slice1;
    Py_ssize_t start0, stop0, step0, slicelength0, start1, stop1, step1, slicelength1;
    EVSpace_MatrixView* matrix_view;

    // fixme: I think I way overthought this... PyArg_ParseTuple() might default int types
    // to call __index__ which may be able to simplify the logic here

    // single arg that supports __index__()
    switch (_EVSpace_AsIndex(indices, &start0))
    {
        case -1: return NULL;
        case 0: break;
        case 1:
            matrix_view = EVSpaceMatrixView_New(self, 1, 3 * start0, 3, -1, sizeof(double), -1);
            return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
    }

    // single arg as slice type
    if (PySlice_Check(indices))
    {
        if (PySlice_GetIndicesEx(indices, 3, &start0, &stop0, &step0, &slicelength0) < 0) {
            return NULL;
        }
        
        if (start0 == stop0) {
            matrix_view = EVSpaceMatrixView_New(self, 1, 0, 0, -1, sizeof(double), -1);
        }
        else {
            // Second dimensions here all have '3' hardcoded because all columns are included
            matrix_view = EVSpaceMatrixView_New(self, 2, 3 * start0, slicelength0, 3,
                                                3 * step0 * sizeof(double), sizeof(double));
        }
        return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
    }

    // multiple args as tuple
    if (PyTuple_Check(indices))
    {
        // possible combination is (int, int), (int, slice), (slice, int), (slice, slice)
        if (PyArg_ParseTuple(indices, "OO:pyevspace.Matrix.__getitem__", &key0, &key1) < 0) {
            return NULL;
        }

        // (slice, ...)
        if (PySlice_Check(key0))
        {
            if (PySlice_GetIndicesEx(key0, 3, &start0, &stop0, &step0, &slicelength0) < 0) {
                return NULL;
            }

            if (start0 == stop0) {
                matrix_view = EVSpaceMatrixView_New(self, 1, 0, 0, -1, sizeof(double), -1);
                return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
            }

            // (slice, slice)
            if (PySlice_Check(key1))
            {
                if (PySlice_GetIndicesEx(key1, 3, &start1, &stop1, &step1, &slicelength1) < 0) {
                    return NULL;
                }

                if (start1 == stop1) {
                    matrix_view = EVSpaceMatrixView_New(self, 1, 0, 0, -1, sizeof(double), -1);
                }
                else {
                    matrix_view = EVSpaceMatrixView_New(self, 2, 3 * start0 + start1, slicelength0,
                                                        slicelength1, 3 * step0 * sizeof(double),
                                                        step1 * sizeof(double));
                }
                return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
            }
            // (slice, int)
            switch (_EVSpace_AsIndex(key1, &index1))
            {
                case -1: return NULL;
                case 0: 
                    PyErr_SetString(PyExc_TypeError, "second index must be int or slice type");
                    return NULL;
                case 1:
                    matrix_view = EVSpaceMatrixView_New(self, 1, 3 * start0 + index1, slicelength0,
                                                        -1, 3 * step0 * sizeof(double), -1);
                    return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
            }
        }

        // (int, ...)
        switch (_EVSpace_AsIndex(key0, &index0))
        {
            case -1: return NULL;
            case 0: 
                PyErr_SetString(PyExc_TypeError, "first index must be int or slice type");
                return NULL;
            case 1: break;
        }

        // (int, slice)
        if (PySlice_Check(key1))
        {
            if (PySlice_GetIndicesEx(key1, 3, &start1, &stop1, &step1, &slicelength1) < 0) {
                return NULL;
            }

            if (start1 == stop1) {
                matrix_view = EVSpaceMatrixView_New(self, 1, 0, 0, -1,
                                                    sizeof(double), -1);
            }
            else {
                matrix_view = EVSpaceMatrixView_New(self, 1, 3 * index0 + start1, slicelength1,
                                                    -1, step1 * sizeof(double), -1);
            }
            return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
        }

        // (int, int)
        switch (_EVSpace_AsIndex(key1, &index1))
        {
            case -1: return NULL;
            case 0:
                PyErr_SetString(PyExc_TypeError, "second index must be int or slice type");
                return NULL;
            case 1: return PyFloat_FromDouble(EVSpaceMatrix_MATRIX(self)(index0, index1));
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "index must be int or slice type");
        return NULL;
    }
}

static int
Matrix_map_assignment(EVSpace_Matrix* self, PyObject* indices, PyObject* rhs)
{
    PyObject* key0, *key1;
    Py_ssize_t index0, index1, tmp;
    double value;

    // If indices is (int, int) we don't want a memoryview
    if (PyArg_ParseTuple(indices, "nn:pyevspace.Matrix.__setitem__", &index0, &index1))
    {
        value = PyFloat_AsDouble(rhs);
        if (value == -1 && PyErr_Occurred()) {
            return -1;
        }

        tmp = index0 + 1;
        PySlice_AdjustIndices(3, &index0, &tmp, 1);
        if (index0 == tmp || index0 < 0 || index0 > 2) {
            PyErr_Format(PyExc_IndexError, "first index must be in range [0-2] (got %i)", index0);
            return -1;
        }

        tmp = index1 + 1;
        PySlice_AdjustIndices(3, &index1, &tmp, 1);
        if (index1 == tmp || index1 < 0 || index1 > 2) {
            PyErr_Format(PyExc_IndexError, "second index must be in range [0-2] (got %i)", index1);
            return -1;
        }

        try {
            EVSpaceMatrix_MATRIX(self)(index0, index1) = value;

            return 0;
        }
        catch (const std::exception &e) {
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return -1;
        }
    }

    // todo: implement slice assignment
    PyErr_Clear();
    if (PyArg_ParseTuple(indices, "O!O!:pyevspace.Matrix.__getitem__", &PySlice_Type, &key0, &PySlice_Type, &key1) ||
        PyArg_ParseTuple(indices, "nO!:pyevspace.Matrix.__getitem__", &index0, &PySlice_Type, &key0) ||
        PyArg_ParseTuple(indices, "O!n:pyevspace.Matrix.__getitem__", &PySlice_Type, &key0, &index0))
    {
        PyErr_SetString(PyExc_Warning, "slice assignment not available");
    }
    return -1;
}

static int
Matrix_get_buffer(EVSpace_Matrix* self, Py_buffer* view, int flags)
{
    Py_ssize_t* shape = NULL, *strides = NULL;

    if (!view) {
        PyErr_SetString(PyExc_ValueError, "NULL view in getbuffer");
        return -1;
    }

    int internal_tmp = EVS_BUFFER_EMPTY;

    view->len           = sizeof(double) * 9;
    view->readonly      = 0;
    view->itemsize      = sizeof(double);
    view->format        = (flags & PyBUF_FORMAT) ? (char*)"d" : NULL;
    view->suboffsets    = NULL;
    view->ndim          = 2;
    view->buf           = EVSpaceMatrix_MATRIX(self).data().data();

    if (flags & PyBUF_ND) {
        Py_ssize_t* shape = reinterpret_cast<Py_ssize_t*>(malloc(2 * sizeof(Py_ssize_t)));
        if (!shape) {
            PyErr_NoMemory();
            return -1;
        }
        shape[0] = shape[1] = 3;
        view->shape = shape;
        internal_tmp |= EVS_BUFFER_RELEASE_SHAPE;
    }
    else {
        view->shape = NULL;
    }

    if (flags & PyBUF_STRIDES) {
        Py_ssize_t* strides = reinterpret_cast<Py_ssize_t*>(malloc(2 * sizeof(Py_ssize_t)));
        if (!strides) {
            PyErr_NoMemory();
            if (shape) free(shape);
            return -1;
        }
        strides[0] = sizeof(double) * 3;
        strides[1] = sizeof(double);
        view->strides = strides;
        internal_tmp |= EVS_BUFFER_RELEASE_STRIDES;
    }
    else {
        view->strides = NULL;
    }

    int* internal = reinterpret_cast<int*>(malloc(sizeof(int)));
    if (!internal) {
        PyErr_NoMemory();
        if (shape) free(shape);
        if (strides) free(strides);
        return -1;
    }
    *internal = internal_tmp;
    view->internal = reinterpret_cast<void*>(internal);

#if PY_VERSION_HEX >= 0x030a0000
    view->obj           = Py_NewRef(self);
#else
    Py_INCREF(self);
    view->obj           = EVS_PyObject_Cast(self);
#endif

    return 0;
}

/* Matrix instance methods */

static PyObject*
Matrix_reduce(EVSpace_Matrix* self, PyObject* Py_UNUSED)
{
    return Py_BuildValue("(O((ddd)(ddd)(ddd)))", Py_TYPE(self),
                        EVSpaceMatrix_MATRIX(self)(0, 0), EVSpaceMatrix_MATRIX(self)(0, 1),
                        EVSpaceMatrix_MATRIX(self)(0, 2), EVSpaceMatrix_MATRIX(self)(1, 0),
                        EVSpaceMatrix_MATRIX(self)(1, 1), EVSpaceMatrix_MATRIX(self)(1, 2),
                        EVSpaceMatrix_MATRIX(self)(2, 0), EVSpaceMatrix_MATRIX(self)(2, 1),
                        EVSpaceMatrix_MATRIX(self)(2, 2));
}

static PyObject*
Matrix_determinate(EVSpace_Matrix* self)
{
    if (!EVSpaceMatrix_Check(self))
    {
        PyErr_Format(PyExc_TypeError,
                     "calling object must be a %s type",
                     Py_TYPE(self)->tp_name);
        return NULL;
    }

    double result = EVSpaceMatrix_MATRIX(self).determinate();

    return PyFloat_FromDouble(result);
}

static PyObject*
Matrix_transpose(EVSpace_Matrix* self)
{
    if (!EVSpaceMatrix_Check(self))
    {
        PyErr_Format(PyExc_TypeError,
                     "calling object must be a %s type",
                     Py_TYPE(self)->tp_name);
        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Matrix result = EVSpaceMatrix_MATRIX(self).transpose();
        return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result)));
    )
}

static PyObject*
Matrix_transpose_inplace(EVSpace_Matrix* self)
{
    if (!EVSpaceMatrix_Check(self))
    {
        PyErr_Format(PyExc_TypeError,
                     "calling object must be a %s type",
                     Py_TYPE(self)->tp_name);
        return NULL;
    }

    EVSpaceMatrix_MATRIX(self).transpose_inplace();

    Py_RETURN_NONE;
}

static PyObject*
Matrix_inverse(EVSpace_Matrix* self)
{
    if (!EVSpaceMatrix_Check(self))
    {
        PyErr_Format(PyExc_TypeError,
                     "calling object must be a %s type",
                     Py_TYPE(self)->tp_name);
        return NULL;
    }

    // Nest try blocks so we only catch std::runtime_error thrown from Matrix.inverse()
    try {
        try {

            evspace::Matrix inverse = EVSpaceMatrix_MATRIX(self).inverse();
            return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(inverse)));
        }
        catch (const std::bad_alloc&) {
            return PyErr_NoMemory();
        }
        catch (const std::runtime_error& e) {
            if (std::string_view(e.what()) == "Unable to invert singular matrix") {
                PyErr_SetString(PyExc_ValueError, "unable to invert singular matrix");
                return NULL;
            }
            else {
                throw;
            }
        }
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject*
Matrix_compare_to_ulp(EVSpace_Matrix* self, PyObject* args)
{
    if (!EVSpaceMatrix_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "calling object must be a Matrix type");
        return NULL;
    }

    EVSpace_Matrix* rhs;
    int max_ulps;
    if (!PyArg_ParseTuple(args, "O!i:compare_to_ulp", &EVSpace_MatrixType, &rhs, &max_ulps)) {
        return NULL;
    }

    if (max_ulps < 0) {
        PyErr_Format(PyExc_ValueError, "max_ulps must be non-negative (got %i)", max_ulps);
        return NULL;
    }

    if (EVSpaceMatrix_MATRIX(self).compare_to(EVSpaceMatrix_MATRIX(rhs), max_ulps)) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

static PyObject*
Matrix_compare_to_tol(EVSpace_Matrix* self, PyObject* args)
{
    EVSpace_Matrix* rhs;
    double rel_tol = evspace::DEFAULT_REL_TOL, abs_tol = evspace::DEFAULT_ABS_TOL;

    if (!EVSpaceMatrix_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "calling object must be a Matrix type");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "O!|dd:compare_to_tol", &EVSpace_MatrixType, &rhs,
                          &rel_tol, &abs_tol)) {
        return NULL;
    }

    if (rel_tol < 0) {
        PyErr_SetString(PyExc_ValueError, "rel_tol must be non-negative");
        return NULL;
    }
    if (abs_tol < 0) {
        PyErr_SetString(PyExc_ValueError, "abs_tol must be non-negative");
        return NULL;
    }

    if (EVSpaceMatrix_MATRIX(self).compare_to(EVSpaceMatrix_MATRIX(rhs),
                                              rel_tol, abs_tol)) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

static PyNumberMethods matrix_as_number;

static PyMappingMethods matrix_as_map = {
    (lenfunc)Matrix_length,             /* mp_length */
    (binaryfunc)Matrix_map_subscript,   /* mp_subscript */
    (objobjargproc)Matrix_map_assignment, /* mp_ass_subscript */
};

static PyBufferProcs matrix_as_buffer = {
    (getbufferproc)Matrix_get_buffer,           /* bf_getbuffer */
    (releasebufferproc)EVSpaceBuffer_Release    /* bf_releasebuffer */
};

PyDoc_STRVAR(matrix_reduce_doc, "pyevspace.Matrix.__reduce() -> (cls, state)\n\
\n\
Allows pickle support of the pyevspace.Matrix type.");

PyDoc_STRVAR(matrix_transpose_doc, "pyevspace.Matrix.transpose() -> pyevspace.Matrix\n\
\n\
Returns the transpose of the matrix, where the returned matrix has\n\
its rows and columns inverted.");

PyDoc_STRVAR(matrix_transpose_inplace_doc, "pyevspace.Matrix.transpose_inplace() -> None\n\
\n\
Transposes the called pyevspace.Matrix object in place.");

PyDoc_STRVAR(matrix_determinate_doc, "pyevspace.Matrix.determinate() -> float\n\
\n\
Computes the determinate of the matrix.");

PyDoc_STRVAR(matrix_inverse_doc, "pyevspace.Matrix.inverse() -> pyevspace.Matrix\n\
\n\
Computes the inverse of a matrix");

PyDoc_STRVAR(matrix_compare_to_ulp_doc, "compare_to_ulp(rhs: Matrix, max_ulps: int) -> bool\n\
\n\
Compares self to rhs using component wise ULP based mechanics. self is\n\
considered equal to rhs if, for each respsective component, the difference\n\
between binary representations is less than or equal to max_ulps ULPs.");

PyDoc_STRVAR(matrix_compare_to_tol_doc, "compare_to_tol(rhs: Matrix, rel_tol: float = 1e-9, abs_tol: float = 1e-15) -> bool\n\
\n\
Compares self to rhs using relative and absolute tolerances. self is\n\
considered equal to rhs if, for each respective component, the difference\n\
is within abs_tol + rel_tol * max(abs(m_ij), abs(m_ij)).");

static PyMethodDef matrix_methods[] = {

    {"__reduce__", (PyCFunction)Matrix_reduce, METH_NOARGS, matrix_reduce_doc},

    {"transpose", (PyCFunction)Matrix_transpose, METH_NOARGS, matrix_transpose_doc},

    {"transpose_inplace", (PyCFunction)Matrix_transpose_inplace, METH_NOARGS, matrix_transpose_inplace_doc},

    {"determinate", (PyCFunction)Matrix_determinate, METH_NOARGS, matrix_determinate_doc},

    {"inverse", (PyCFunction)Matrix_inverse, METH_NOARGS, matrix_inverse_doc},

    {"compare_to_ulp", (PyCFunction)Matrix_compare_to_ulp, METH_VARARGS, matrix_compare_to_ulp_doc},

    {"compare_to_tol", (PyCFunction)Matrix_compare_to_tol, METH_VARARGS, matrix_compare_to_tol_doc},

    {NULL}
};

PyDoc_STRVAR(matrix_doc, "pyevspace.matrix([row0: Iterable, row1: Iterable, row2: Iterable])\n\
\n\
The pyevspace.Matrix type can be constructed with rows of iterables all of\
length three, whose components are numeric types. Alternatively if no arguments\
are present each component is defaulet to zero.");

/* EulerAngles type constructors */

static EVSpace_Angles*
_EVSpace_Angles_New(double alpha, double beta, double gamma, PyTypeObject* type)
{
    EVSpace_Angles* angles = reinterpret_cast<EVSpace_Angles*>(type->tp_alloc(type, 0));
    if (!angles) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        angles->angles = new evspace::EulerAngles(alpha, beta, gamma);
        return angles;
    )
}

static inline EVSpace_Angles*
EVSpaceAngles_New()
{
    return _EVSpace_Angles_New(0.0, 0.0, 0.0, &EVSpace_AnglesType);
}

static inline EVSpace_Angles*
EVSpaceAngles_New(double alpha, double beta, double gamma)
{
    return _EVSpace_Angles_New(alpha, beta, gamma, &EVSpace_AnglesType);
}

static PyObject*
Angles_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED)
{
    return reinterpret_cast<PyObject*>(EVSpaceAngles_New(0.0, 0.0, 0.0));
}

static int
Angles_init(EVSpace_Angles* self, PyObject* args, PyObject* Py_UNUSED)
{
    double alpha, beta, gamma;

    if (PyTuple_GET_SIZE(args) == 0) {
        // default init already sets values to zero
        return 0;
    }
    else if (!PyArg_ParseTuple(args, "ddd", &alpha, &beta, &gamma)) {
        return -1;
    }

    EVSpaceAngles_ALPHA(self) = alpha;
    EVSpaceAngles_BETA(self) = beta;
    EVSpaceAngles_GAMMA(self) = gamma;

    return 0;
}

static void
Angles_dealloc(EVSpace_Angles* self)
{
    delete self->angles;
    Py_TYPE(self)->tp_free(EVS_PyObject_Cast(self));
}

static inline PyObject*
_EVSpaceAngles_String(const EVSpace_Angles* angles, const char* format)
{
    std::size_t buffer_length = snprintf(NULL, 0, format,
                                         EVSpaceAngles_ALPHA(angles), EVSpaceAngles_BETA(angles),
                                         EVSpaceAngles_GAMMA(angles));
    char* buffer = (char*)malloc(buffer_length + 1);
    if (!buffer) {
        return NULL;
    }

    int bytes_written = sprintf(buffer, format, EVSpaceAngles_ALPHA(angles),
                                EVSpaceAngles_BETA(angles), EVSpaceAngles_GAMMA(angles));
    if (bytes_written < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "error filling string buffer");
        delete buffer;
        return NULL;
    }

    PyObject* rtn = PyUnicode_FromString(buffer);
    free(buffer);

    return rtn;
}

static PyObject*
Angles_str(const EVSpace_Angles* angles)
{
    return _EVSpaceAngles_String(angles, "[%f, %f, %f]");
}

static PyObject*
Angles_repr(const EVSpace_Angles* angles)
{
    return _EVSpaceAngles_String(angles, "EulerAngles(%f, %f, %f)");
}

static PyObject*
Angles_reduce(const EVSpace_Angles* self, PyObject* Py_UNUSED)
{
    return Py_BuildValue("(O(ddd))", Py_TYPE(self), EVSpaceAngles_ALPHA(self),
                         EVSpaceAngles_BETA(self), EVSpaceAngles_GAMMA(self));
}

static PyObject*
Angles_get_item(const EVSpace_Angles* self, Py_ssize_t index)
{
    if (index < 0 || index > 2)
    {
        PyErr_Format(PyExc_IndexError, "index (%i) must be in [0-2]", index);
        return NULL;
    }

    return PyFloat_FromDouble(EVSpaceAngles_ANGLES(self)[index]);
}

static int
Angles_set_item(EVSpace_Angles* self, Py_ssize_t index, PyObject* arg)
{
    double value;

    if (!EVSpaceObject_AsDouble(arg, value)) {
        return -1;
    }

    if (index < 0 || index > 2)
    {
        PyErr_Format(PyExc_IndexError, "index (%i) must be in [0-2]", index);
        return -1;
    }

    EVSpaceAngles_ANGLES(self)[index] = value;

    return 0;
}

static PySequenceMethods angles_as_sequence = {
    (lenfunc)Vector_length,             /* sq_length */
    0,                                  /* sq_concat */
    0,                                  /* sq_repeat */
    (ssizeargfunc)Angles_get_item,      /* sq_item */
    0,                                  /* was_sq_slice */
    (ssizeobjargproc)Angles_set_item    /* sq_ass_item */
};

PyDoc_STRVAR(angles_reduce_doc, "pyevspace.__reduce__() -> (cls, state)\n\
\n\
Allows pickle support of EulerAngles objects.");

static PyMethodDef angles_methods[] = {

    {"__reduce__", (PyCFunction)Angles_reduce, METH_NOARGS, angles_reduce_doc},

    {NULL}
};

PyDoc_STRVAR(angles_doc, "pyevspace.EulerAngles(alpha: Real, beta: Real, gamma: Real)\n\
\n\
Sets the corresponding angles all of which are required. Unneeded\n\
angles should be set to zero.");

/* EulerOrder type */

static EVSpace_Order*
_EVSpaceOrder_New(evspace::AxisDirection first, evspace::AxisDirection second,
                  evspace::AxisDirection thrid, PyTypeObject* type)
{
    EVSpace_Order* order = reinterpret_cast<EVSpace_Order*>(type->tp_alloc(type, 0));
    if (!order) {
        return NULL;
    }

    order->first = first;
    order->second = second;
    order->third = thrid;

    return order;
}

static inline EVSpace_Order*
EVSpaceOrder_New(evspace::AxisDirection first, evspace::AxisDirection second,
                 evspace::AxisDirection third)
{
    return _EVSpaceOrder_New(first, second, third, &EVSpace_OrderType);
}

static PyObject*
Order_new(PyTypeObject* type, PyObject* Py_UNUSED(0), PyObject* Py_UNUSED(1))
{
    return type->tp_alloc(type, 0);
}

static int
Order_init(EVSpace_Order* self, PyObject* args, PyObject* Py_UNUSED)
{
    int first, second, third;
    
    if (!PyArg_ParseTuple(args, "iii", &first, &second, &third)) {
        return -1;
    }

    if (first < 0 || first > 2) {
        PyErr_Format(PyExc_ValueError, "first axis must be in [0-2], got %d", first);
        return -1;
    }

    if (second < 0 || second > 2) {
        PyErr_Format(PyExc_ValueError, "second axis must be in [0-2], got %d", second);
        return -1;
    }

    if (third < 0 || third > 2) {
        PyErr_Format(PyExc_ValueError, "third axis must be in [0-2], got %d", third);
        return -1;
    }

    self->first = static_cast<evspace::AxisDirection>(first);
    self->second = static_cast<evspace::AxisDirection>(second);
    self->third = static_cast<evspace::AxisDirection>(third);

    return 0;
}

static void
Order_dealloc(EVSpace_Order* self)
{
    Py_TYPE(self)->tp_free(EVS_PyObject_Cast(self));
}

static inline char*
_EVSpaceAxis_GetName(evspace::AxisDirection direction, char* string)
{
    if (direction == evspace::AxisDirection::X) {
        sprintf(string, "X_AXIS");
    }
    else if (direction == evspace::AxisDirection::Y) {
        sprintf(string, "Y_AXIS");
    }
    else if (direction == evspace::AxisDirection::Z) {
        sprintf(string, "Z_AXIS");
    }

    return string;
}

static PyObject*
_EVSpaceOrder_String(const EVSpace_Order* order, const char* format, std::size_t alloc_size)
{
    char first[7], second[7], third[7];

    char* buffer = reinterpret_cast<char*>(malloc(alloc_size));
    if (!buffer) {
        return NULL;
    }

    if (sprintf(buffer, format,
            _EVSpaceAxis_GetName(order->first, first),
            _EVSpaceAxis_GetName(order->second, second),
            _EVSpaceAxis_GetName(order->third, third)) < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "unable to allocate string buffer");
        free(buffer);
        return NULL;
    }

    PyObject* rtn = PyUnicode_FromString(buffer);
    free(buffer);
    return rtn;
}

static PyObject*
Order_str(const EVSpace_Order* order)
{
    return _EVSpaceOrder_String(order, "[%s, %s, %s]", 25);
}

static PyObject*
Order_repr(const EVSpace_Order* order)
{
    return _EVSpaceOrder_String(order, "RotationOrder(%s, %s, %s)", 40);
}

static PyObject*
Order_get_item(EVSpace_Order* self, Py_ssize_t index)
{
    evspace::AxisDirection rtn;

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
        PyErr_Format(PyExc_IndexError, "index (%i) must be in [0-2]", index);
        return NULL;
    }

    return PyLong_FromLong(static_cast<long>(rtn));
}

static PyObject*
Order_richcompare(EVSpace_Order* self, PyObject* other, int op)
{
    EVSpace_Order* rhs;
    bool is_equal;

    if (EVSpaceOrder_Check(other))
    {
        rhs = reinterpret_cast<EVSpace_Order*>(other);
        is_equal = (self->first == rhs->first &&
                    self->second == rhs->second &&
                    self->third == rhs->third);

        if (op == Py_EQ)
        {
            if (is_equal) {
                Py_RETURN_TRUE;
            }
            else {
                Py_RETURN_FALSE;
            }
        }
        else if (op == Py_NE)
        {
            if (!is_equal) {
                Py_RETURN_TRUE;
            }
            else {
                Py_RETURN_FALSE;
            }
        }
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static Py_hash_t
Order_hash(EVSpace_Order* self)
{
    PyObject* t = Py_BuildValue(
        "(iii)",
        static_cast<int>(self->first),
        static_cast<int>(self->second),
        static_cast<int>(self->third)
    );
    if (!t) {
        return -1;
    }

    Py_hash_t hash = PyObject_Hash(t);
    Py_DECREF(t);
    return hash;
}

static PyObject*
Order_reduce(const EVSpace_Order* self, PyObject* Py_UNUSED)
{
    return Py_BuildValue("(O(iii))", Py_TYPE(self), self->first, self->second, self->third);
}

static PySequenceMethods order_as_sequence = {
    (lenfunc)Vector_length,             /* sq_length */
    0,                                  /* sq_concat */
    0,                                  /* sq_repeat */
    (ssizeargfunc)Order_get_item,       /* sq_item */
};

static PyMemberDef order_members[] = {

    {
#if PY_VERSION_HEX < 0x030c0000
    "first", T_INT, offsetof(EVSpace_Order, first), READONLY,
#else
    "first", Py_T_INT, offsetof(EVSpace_Order, first), Py_READONLY,
#endif
     PyDoc_STR("first axis of a rotation")
    },
    {
#if PY_VERSION_HEX < 0x030c0000
    "second", T_INT, offsetof(EVSpace_Order, second), READONLY,
#else
    "second", Py_T_INT, offsetof(EVSpace_Order, second), Py_READONLY,
#endif
     PyDoc_STR("second axis of a rotation")
    },
    {
#if PY_VERSION_HEX < 0x030c0000
    "third", T_INT, offsetof(EVSpace_Order, third), READONLY,
#else
    "third", Py_T_INT, offsetof(EVSpace_Order, third), Py_READONLY,
#endif
     PyDoc_STR("third axis of a rotation")
    },

     {NULL}
};

PyDoc_STRVAR(order_reduce_doc, "pyevspace.Matrix.__reduce__() -> (cls, state)\n\
\n\
Allows pickle support of Order instances.");

static PyMethodDef order_methods[] = {
    
    {"__reduce__", (PyCFunction)Order_reduce, METH_NOARGS, order_reduce_doc},

    {NULL}
};

PyDoc_STRVAR(order_doc, "RotationOrder(axis1: int, axis2: int, axis3: int)\n\
\n\
All axes are required and should be one of the three enumerated\n\
axis types X_AXIS, Y_AXIS, or Z_AXIS.");

/* ReferencceFrame type */

static int
EVSpaceReferenceFrame_UpdateMatrix(EVSpace_ReferenceFrame* frame)
{
    frame->matrix = _EVSpaceRotate_ComputeMatrix(frame->first, frame->second, frame->third,
                                                 frame->angles[0], frame->angles[1],
                                                 frame->angles[2], frame->intrinsic);
    return (!frame->matrix) ? -1 : 0;
}

static EVSpace_ReferenceFrame*
_EVSpaceReferenceFrame_New(evspace::AxisDirection first, evspace::AxisDirection second,
                           evspace::AxisDirection third, evspace::EulerAngles* angles,
                           PyObject* offset, bool intrinsic, PyTypeObject* type)
{
    EVSpace_ReferenceFrame* self = (EVSpace_ReferenceFrame*)(type->tp_alloc(type, 0));
    if (!self) {
        return NULL;
    }

    self->first = first;
    self->second = second;
    self->third = third;

    self->angles[0] = (*angles)[0];
    self->angles[1] = (*angles)[1];
    self->angles[2] = (*angles)[2];

    self->intrinsic = intrinsic;

    try {
        self->matrix = new evspace::Matrix();
    }
    catch (const std::bad_alloc&) {
        Py_DECREF(self);
        PyErr_NoMemory();
        return NULL;
    }

    if (!offset) {
#if PY_VERSION_HEX >= 0x030a0000
        self->offset = Py_NewRef(Py_None);
#else
        Py_INCREF(Py_None);
        self->offset = Py_None;
#endif
    }
    else if (EVSpaceVector_Check(offset))
    {
#if PY_VERSION_HEX >= 0x030a0000
        self->offset = Py_NewRef(offset);
#else
        Py_INCREF(offset);
        self->offset = offset;
#endif
    }
    else {
        PyErr_Format(PyExc_TypeError, "offset must be Vector type or None, got %s",
                     __EVSpace_GetTypeName(offset));
        Py_DECREF(self);
        return NULL;
    }

    if (EVSpaceReferenceFrame_UpdateMatrix(self) < 0)
    {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}

static EVSpace_ReferenceFrame*
EVSpaceReferenceFrame_New(evspace::AxisDirection first, evspace::AxisDirection second,
                          evspace::AxisDirection third, evspace::EulerAngles* angles,
                          PyObject* offset, bool intrinsic)
{
    // offset should be NULL to represent no offset value i.e. offset should by Py_None
    return _EVSpaceReferenceFrame_New(first, second, third, angles, offset,
                                      intrinsic, &EVSpace_ReferenceFrameType);
}

static PyObject*
ReferenceFrame_New(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    EVSpace_Order* order;
    EVSpace_Angles* angles;
    PyObject* offset = NULL;
    int intrinsic = true;

    static const char* kwargs_list[] = { "", "", "intrinsic", "offset", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!|$pO", const_cast<char**>(kwargs_list),
                                     &EVSpace_OrderType, &order, &EVSpace_AnglesType, &angles,
                                     &intrinsic, &offset)) {
        return NULL;
    }

    // If offset is explicitly set to None re-set it to NULL
#if PY_VERSION_HEX >= 0x030a0000
    if (offset && Py_IsNone(offset))
#else
    if (offset && offset == Py_None)
#endif
    {
        offset = NULL;
    }
    else if (offset && !EVSpaceVector_Check(offset))
    {
        PyErr_Format(PyExc_TypeError, "offset must be a Vector type or None, got %s",
                     __EVSpace_GetTypeName(offset));
        return NULL;
    }

    return reinterpret_cast<PyObject*>(
        EVSpaceReferenceFrame_New(order->first, order->second, order->third,
                                  angles->angles, offset, static_cast<bool>(intrinsic))
    );
}

// Not initializing a ReferenceFrame on allocation leaves things in such
// an invalid state I'm not implementing the __init__ method for now.

static void
ReferenceFrame_Dealloc(EVSpace_ReferenceFrame* self)
{
    free(self->matrix);
    Py_DECREF(self->offset);
    Py_TYPE(self)->tp_free(EVS_PyObject_Cast(self));
}

static PyObject*
ReferenceFrame_GetMatrix(EVSpace_ReferenceFrame* self, PyObject* Py_UNUSED())
{
    return reinterpret_cast<PyObject*>(EVSpaceMatrix_New(*self->matrix));
}

static PyObject*
ReferenceFrame_GetAngles(EVSpace_ReferenceFrame* self, PyObject* Py_UNUSED())
{
    return reinterpret_cast<PyObject*>(EVSpaceAngles_New(self->angles[0],
                                                         self->angles[1],
                                                         self->angles[2]));
}

static PyObject*
ReferenceFrame_SetAngles(EVSpace_ReferenceFrame* self, PyObject* args, PyObject* kwargs)
{
    double alpha, beta, gamma;
    PyObject* arg1 = NULL, *arg2 = NULL, *arg3 = NULL;
    EVSpace_Angles* angles;
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);

    static const char* kwarg_names[] = {"alpha", "beta", "gamma", NULL};

    if (arg_count == 1)
    {   
        if (PyArg_ParseTuple(args, "O!:set_angles", &EVSpace_AnglesType, &angles))
        {
            self->angles[0] = (*angles->angles)[0];
            self->angles[1] = (*angles->angles)[1];
            self->angles[2] = (*angles->angles)[2];
            
            EVSpaceReferenceFrame_UpdateMatrix(self);
            
            Py_INCREF(Py_None);
            Py_RETURN_NONE;
        }
        else {
            return NULL;
        }
    }
    else if (arg_count > 1) {
        PyErr_Format(PyExc_TypeError, "expected 1 position argument, got %d", arg_count);
        return NULL;
    }
    
    if (PyArg_ParseTupleAndKeywords(args, kwargs, "|$OOO:set_angles",
                                    const_cast<char**>(kwarg_names),
                                    &arg1, &arg2, &arg3))
    {
    // Reset args to NULL if they're None
#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(arg1))
#else
        if (arg1 == Py_None)
#endif
        {
            arg1 = NULL;
        }

#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(arg2))
#else
        if (arg2 == Py_None)
#endif
        {
            arg2 = NULL;
        }

#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(arg3))
#else
        if (arg3 == Py_None)
#endif
        {
            arg3 = NULL;
        }


        if (arg1 && !EVSpaceObject_AsDouble(arg1, alpha)) {
            return NULL;
        }
        if (arg2 && !EVSpaceObject_AsDouble(arg2, beta)) {
            return NULL;
        }
        if (arg3 && !EVSpaceObject_AsDouble(arg3, gamma)) {
            return NULL;
        }

        if (arg1) {
            self->angles[0] = alpha;
        }
        if (arg2) {
            self->angles[1] = beta;
        }
        if (arg3) {
            self->angles[2] = gamma;
        }

        EVSpaceReferenceFrame_UpdateMatrix(self);

        Py_INCREF(Py_None);
        Py_RETURN_NONE;
    }
    else {
        return NULL;
    }
}

static PyObject*
ReferenceFrame_GetOrder(EVSpace_ReferenceFrame* self, void* closure)
{
    return reinterpret_cast<PyObject*>(
        EVSpaceOrder_New(self->first, self->second, self->third)
    );
}

static PyObject*
ReferenceFrame_GetOffset(EVSpace_ReferenceFrame* self, void* closure)
{
#if PY_VERSION_HEX >= 0x030a0000
    return Py_NewRef(self->offset);
#else
    Py_INCREF(self->offset);
    return self->offset;
#endif
}

static int
ReferenceFrame_SetOffset(EVSpace_ReferenceFrame* self, PyObject* arg,
                         void* closure)
{
    if (!arg) {
        PyErr_SetString(PyExc_ValueError, "cannot delete offset attribute");
        return -1;
    }

#if PY_VERSION_HEX >= 0x030a0000
    if (!Py_IsNone(arg) && !EVSpaceVector_Check(arg))
#else
    if ((arg != Py_None) && !EVSpaceVector_Check(arg))
#endif
    {
        PyErr_Format(PyExc_TypeError, "expected offset to be Vector type or None, got %s",
                     __EVSpace_GetTypeName(arg));
        return -1;
    }

#if PY_VERSION_HEX >= 0x030a0000
    Py_SETREF(self->offset, Py_NewRef(arg));
#else
    Py_INCREF(arg);
    Py_SETREF(self->offset, arg);
#endif

    return 0;
}

static PyMemberDef reference_frame_members[] = {
    {
#if PY_VERSION_HEX >= 0x030c0000
    "intrinsic", Py_T_BOOL, offsetof(EVSpace_ReferenceFrame, intrinsic), Py_READONLY,
#else
    "intrinsic", T_BOOL, offsetof(EVSpace_ReferenceFrame, intrinsic), READONLY,
#endif
    PyDoc_STR("rotation intrinsic value")},

    { NULL }
};

static PyGetSetDef reference_frame_getset[] = {
    {"order", (getter)ReferenceFrame_GetOrder, NULL,
     PyDoc_STR("order of axes in the rotation"), NULL},

    {"offset", (getter)ReferenceFrame_GetOffset,
     (setter)ReferenceFrame_SetOffset,
     PyDoc_STR("offset of the reference frame origin"), NULL},

    { NULL }
};

static PyObject*
ReferenceFrame_RotateTo(EVSpace_ReferenceFrame* self, PyObject* args)
{
    PyObject* arg1, *arg2;
    EVSpace_Vector* vector, *offset;
    EVSpace_ReferenceFrame* other_frame;
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    evspace::Vector* tmp;

    if (arg_count == 1)
    {
        arg1 = PyTuple_GET_ITEM(args, 0);
        if (!EVSpaceVector_Check(arg1))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be Vector type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return NULL;
        }

        vector = EVSpaceVector_Cast(arg1);

#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(self->offset))
#else
        if (self->offset == Py_None)
#endif
        {
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(self->matrix, vector->vector)))
            );
        }
        else
        {
            offset = EVSpaceVector_Cast(self->offset);
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(self->matrix, vector->vector, offset->vector)))
            );
        }
    }
    else if (arg_count == 2)
    {
        arg1 = PyTuple_GET_ITEM(args, 0);
        if (!EVSpaceReferenceFrame_Check(arg1))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be ReferenceFrame type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return NULL;
        }

        arg2 = PyTuple_GET_ITEM(args, 1);
        if (!EVSpaceVector_Check(arg2))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 2 to be Vector type, got %s",
                         __EVSpace_GetTypeName(arg2));
            return NULL;
        }

        other_frame = reinterpret_cast<EVSpace_ReferenceFrame*>(arg1);
        vector = EVSpaceVector_Cast(arg2);

#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(other_frame->offset))
#else
        if (other_frame->offset == Py_None)
#endif
        {
            tmp = _EVSpaceRotate_From(other_frame->matrix, vector->vector);
            if (!tmp) {
                return NULL;
            }
        }
        else
        {
            offset = EVSpaceVector_Cast(other_frame->offset);
            tmp = _EVSpaceRotate_From(other_frame->matrix, vector->vector, offset->vector);
            if (!tmp) {
                return NULL;
            }
        }

#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(self->offset))
#else
        if (self->offset == Py_None)
#endif
        {
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(self->matrix, tmp)))
            );
        }
        else
        {
            offset = EVSpaceVector_Cast(self->offset);
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(self->matrix, tmp, offset->vector)))
            );
        }
    }
    else
    {
        PyErr_Format(PyExc_TypeError,
                     "expected 1 or 2 arguments, got %d",
                     arg_count);
        return NULL;
    }
}
static PyObject*
ReferenceFrame_RotateFrom(EVSpace_ReferenceFrame* self, PyObject* args)
{
    PyObject* arg1, *arg2;
    EVSpace_Vector* vector, *offset;
    EVSpace_ReferenceFrame* other_frame;
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    evspace::Vector* tmp;

    if (arg_count == 1)
    {
        arg1 = PyTuple_GET_ITEM(args, 0);
        if (!EVSpaceVector_Check(arg1))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be Vector type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return NULL;
        }

        vector = EVSpaceVector_Cast(arg1);

#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(self->offset))
#else
        if (self->offset == Py_None)
#endif
        {
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_From(self->matrix, vector->vector)))
            );
        }
        else
        {
            offset = EVSpaceVector_Cast(self->offset);
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_From(self->matrix, vector->vector, offset->vector)))
            );
        }
    }
    else if (arg_count == 2)
    {
        arg1 = PyTuple_GET_ITEM(args, 0);
        if (!EVSpaceReferenceFrame_Check(arg1))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be ReferenceFrame type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return NULL;
        }

        arg2 = PyTuple_GET_ITEM(args, 1);
        if (!EVSpaceVector_Check(arg2))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 2 to be Vector type, got %s",
                         __EVSpace_GetTypeName(arg2));
            return NULL;
        }

        other_frame = reinterpret_cast<EVSpace_ReferenceFrame*>(arg1);
        vector = EVSpaceVector_Cast(arg2);

#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(self->offset))
#else
        if (self->offset == Py_None)
#endif
        {
            tmp = _EVSpaceRotate_From(self->matrix, vector->vector);
            if (!tmp) {
                return NULL;
            }
        }
        else
        {
            offset = EVSpaceVector_Cast(self->offset);
            tmp = _EVSpaceRotate_From(self->matrix, vector->vector, offset->vector);
            if (!tmp) {
                return NULL;
            }
        }

#if PY_VERSION_HEX >= 0x030a0000
        if (Py_IsNone(other_frame->offset))
#else
        if (other_frame->offset == Py_None)
#endif
        {
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(other_frame->matrix, tmp)))
            );
        }
        else
        {
            offset = EVSpaceVector_Cast(other_frame->offset);
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(other_frame->matrix, tmp,
                                                               offset->vector)))
            );
        }
    }
    else
    {
        PyErr_Format(PyExc_TypeError,
                     "expected 1 or 2 arguments, got %d",
                     arg_count);
        return NULL;
    }
}

PyDoc_STRVAR(refframe_get_matrix_doc, "get_matrix() -> Matrix\n\
\n\
Get a copy of the internal matrix representation of the current rotation state.");

PyDoc_STRVAR(refframe_get_angles_doc, "get_angles() -> EulerAngles\n\
\n\
Get a new EulerAngles object representing the current values of the rotation angles.");

PyDoc_STRVAR(refframe_set_angles_doc, "get_angles(angles: EulerAngles) -> None\n\
get_angles(alpha: float = None, beta: float = None, gamma: float = None) -> None\n\
\n\
Update rotation angles and recompute internal rotation matrix.");

PyDoc_STRVAR(refframe_rotate_to_doc, "rotate_to(vector: Vector) -> Vector\n\
rotate_to(frame: ReferenceFrame, vector: Vector) -> Vector\n\
\n\
Rotate a vector to the reference frame. If another ReferenceFrame is provided,\n\
vector will be rotated from that reference frame to this reference frame.");

PyDoc_STRVAR(refframe_rotate_from_doc, "rotate_from(vector: Vector) -> Vector\n\
rotate_from(frame: ReferenceFrame, vector: Vector) -> Vector\n\
\n\
Rotate a vector from the reference frame. If another ReferenceFrame is provided,\n\
vector will be rotated to that reference frame from this reference frame.");

static PyMethodDef reference_frame_methods[] = {

    {"get_matrix", (PyCFunction)ReferenceFrame_GetMatrix, METH_NOARGS,
     refframe_get_matrix_doc},

    {"get_angles", (PyCFunction)ReferenceFrame_GetAngles, METH_NOARGS,
     refframe_get_angles_doc},

    {"set_angles", (PyCFunction)ReferenceFrame_SetAngles,
     METH_VARARGS | METH_KEYWORDS, refframe_set_angles_doc},

    {"rotate_to", (PyCFunction)ReferenceFrame_RotateTo, METH_VARARGS,
     refframe_rotate_to_doc},

    {"rotate_from", (PyCFunction)ReferenceFrame_RotateFrom, METH_VARARGS,
     refframe_rotate_from_doc},

    {NULL}
};


/* rotation functions */

static inline evspace::Vector*
_EVSpaceRotate_From(const evspace::Matrix* matrix, const evspace::Vector* vector)
{
    EXCEPTION_WRAPPER(
        evspace::Vector* result
            = new evspace::Vector(evspace::rotate_from(*matrix, *vector));
        return result;
    )
}

static inline evspace::Vector*
_EVSpaceRotate_From(const evspace::Matrix* matrix, const evspace::Vector* vector,
                    const evspace::Vector* offset)
{
    EXCEPTION_WRAPPER(
        evspace::Vector* result
            = new evspace::Vector(evspace::rotate_from(*matrix, *vector, *offset));
        return result;
    )
}

static inline evspace::Vector*
_EVSpaceRotate_To(const evspace::Matrix* matrix, const evspace::Vector* vector)
{
    EXCEPTION_WRAPPER(
        evspace::Vector* result
            = new evspace::Vector(evspace::rotate_to(*matrix, *vector));
        return result;
    )
}

static inline evspace::Vector*
_EVSpaceRotate_To(const evspace::Matrix* matrix, const evspace::Vector* vector,
                  const evspace::Vector* offset)
{
    EXCEPTION_WRAPPER(
        evspace::Vector* result
            = new evspace::Vector(evspace::rotate_to(*matrix, *vector, *offset));
        return result;
    )
}

static inline evspace::Vector*
_EVSpaceRotate_Between(const evspace::Matrix* matrix_from, const evspace::Matrix* matrix_to,
                       const evspace::Vector* offset_from, const evspace::Vector* offset_to,
                       const evspace::Vector* vector)
{
    evspace::Vector* tmp, *result;

    // due to the implementation of evspace the only rotate_between() methods are templates
    // so we need to implement the rotate between logic here

    tmp = offset_from ? _EVSpaceRotate_From(matrix_from, vector, offset_from) :
                        _EVSpaceRotate_From(matrix_from, vector);
    if (!tmp) {
        return NULL;
    }

    result = offset_to ? _EVSpaceRotate_To(matrix_to, tmp, offset_to) :
                         _EVSpaceRotate_To(matrix_to, tmp);
    delete tmp;

    return result;
}

static inline evspace::Matrix*
_EVSpaceRotate_ComputeMatrix(double angle, evspace::AxisDirection direction)
{
    evspace::Matrix* matrix;

    try {
        if (direction == evspace::AxisDirection::X) {
            matrix = new evspace::Matrix(evspace::compute_rotation_matrix<evspace::XAxis>(angle));
        }
        else if (direction == evspace::AxisDirection::Y) {
            matrix = new evspace::Matrix(evspace::compute_rotation_matrix<evspace::YAxis>(angle));
        }
        else if (direction == evspace::AxisDirection::Z) {
            matrix = new evspace::Matrix(evspace::compute_rotation_matrix<evspace::ZAxis>(angle));
        }
        else {
            PyErr_Format(PyExc_ValueError,
                "invalid axis enum value, must be in [0-2], got %i",
                direction
            );
            return NULL;
        }

        return matrix;
    }
    catch (const std::bad_alloc&) {
        PyErr_NoMemory();
        return NULL;
    }
    catch (const std::runtime_error& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static inline evspace::Matrix*
_EVSpaceRotate_ComputeMatrix(double angle, const evspace::Vector* vector)
{
    const evspace::Vector* rotation_vector;
    evspace::Matrix* matrix;
    bool needs_freeing = false;

    EXCEPTION_WRAPPER(
        if (vector->magnitude() < VECTOR_LENGTH_SMALL) {
            rotation_vector = new evspace::Vector(vector->norm());
            needs_freeing = true;
        }
        else {
            rotation_vector = vector;
        }

        matrix = new evspace::Matrix(
            evspace::compute_rotation_matrix(angle, *rotation_vector)
        );
        
        if (needs_freeing) {
            delete rotation_vector;
        }
        return matrix;
    )
}

static evspace::Matrix*
_EVSpaceRotate_ComputeMatrix(evspace::AxisDirection first, evspace::AxisDirection second,
                             evspace::AxisDirection third, double alpha, double beta,
                             double gamma, bool intrinsic)
{
    evspace::Matrix* matrix_alpha, *matrix_beta, *matrix_gamma, result;

    EXCEPTION_WRAPPER(
        matrix_alpha = _EVSpaceRotate_ComputeMatrix(alpha, first);
        if (!matrix_alpha) {
            return NULL;
        }

        matrix_beta = _EVSpaceRotate_ComputeMatrix(beta, second);
        if (!matrix_beta) {
            delete matrix_alpha;
            return NULL;
        }

        matrix_gamma = _EVSpaceRotate_ComputeMatrix(gamma, third);
        if (!matrix_gamma) {
            delete matrix_alpha;
            delete matrix_beta;
            return NULL;
        }

        result = intrinsic ?
            (*matrix_alpha) * (*matrix_beta) * (*matrix_gamma) :
            (*matrix_gamma) * (*matrix_beta) * (*matrix_alpha);
        
        return new evspace::Matrix(std::move(result));
    )
}

static inline evspace::Matrix*
_EVSpaceRotate_ComputeMatrix(const EVSpace_Order* order,
                             const EVSpace_Angles* angles, bool intrinsic)
{
    return _EVSpaceRotate_ComputeMatrix(order->first, order->second, order->third,
                                        EVSpaceAngles_ALPHA(angles), EVSpaceAngles_BETA(angles),
                                        EVSpaceAngles_GAMMA(angles), intrinsic);
}

// Get the type name of `obj` as a const char* for printing or
// string formating.
static inline const char*
__EVSpace_GetTypeName(PyObject* obj)
{
    return Py_TYPE(obj)->tp_name;
}

// Analyze the args tuple and deduce the most likely intended signature
// and produce an informative type error. The `args` parameter MUST be
// a python tuple type. This function does not care about keyword arguments
// and keyword argument errors should be handled separately.
static void
__EVSpace_ComputeMatrixError(PyObject* args)
{
    PyObject* arg1, *arg2, *arg3, *arg4;
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    double dummy_float;

    arg1 = PyTuple_GET_ITEM(args, 0);
    arg2 = PyTuple_GET_ITEM(args, 1);
    if (arg_count == 2)
    {
        if (EVSpaceObject_AsDouble(arg1, dummy_float))
        {
            if (!PyLong_Check(arg2) && !EVSpaceVector_Check(arg2))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 2 to be an int or Vector type, got %s",
                             __EVSpace_GetTypeName(arg2));
                return;
            }
        }
        else if (EVSpaceOrder_Check(arg1))
        {
            if (!EVSpaceAngles_Check(arg2))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 2 to be an EulerAngles type, got %s",
                             __EVSpace_GetTypeName(arg2));
                return;
            }
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be a float or RotationOrder type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return;
        }
    }
    else if (arg_count == 4)
    {
        arg3 = PyTuple_GET_ITEM(args, 2);
        arg4 = PyTuple_GET_ITEM(args, 3);
        if (!EVSpaceOrder_Check(arg1))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be a RotationOrder type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return;
        }
        else if (!EVSpaceAngles_Check(arg2))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 2 to be an EulerAngles type, got %s",
                         __EVSpace_GetTypeName(arg2));
            return;
        }
        else if (!EVSpaceOrder_Check(arg3))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 3 to be a RotationOrder type, got %s",
                         __EVSpace_GetTypeName(arg3));
            return;
        }
        else if (!EVSpaceAngles_Check(arg4))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 4 to be an EulerAngles type, got %s",
                          __EVSpace_GetTypeName(arg4));
            return;
        }
    }
    else
    {
        PyErr_Format(PyExc_TypeError,
                     "expected 2 or 4 positional arguments, got %d", arg_count);
        return;
    }

    // We shouldn't be here if at least one combination above fails, give
    // an error so we know something went wrong.
    PyErr_SetString(PyExc_RuntimeError, "unknown error parsing arguments");
}

static PyObject*
EVSpaceRotation_ComputeMatrix(PyObject* Py_UNUSED(_), PyObject* args, PyObject* kwargs)
{
    PyObject* result;
    evspace::Matrix* matrix, *tmp;
    EVSpace_Vector* vector_axis;
    EVSpace_Order* order_from, *order_to;
    EVSpace_Angles* angles_from, *angles_to;
    double angle;
    int axis, intrinsic_from = 1, intrinsic_to = 1;

    static const char* kwlist_empty[] = { "", "", NULL };
    static const char* kwlist_euler[] = { "", "", "intrinsic", NULL };
    static const char* kwlist_between[] = { "", "", "", "", "intrinsic_from", "intrinsic_to", NULL };

    // (a: float, ax: int)
    if (PyArg_ParseTupleAndKeywords(args, kwargs, "di:compute_rotation_matrix",
        const_cast<char**>(kwlist_empty), &angle, &axis))
    {
        if (axis < 0 || axis > 2) {
            PyErr_Format(PyExc_ValueError, "axis must be in [0-2] (got %i)", axis);
            return NULL;
        }

        matrix = _EVSpaceRotate_ComputeMatrix(angle, static_cast<evspace::AxisDirection>(axis));
        if (!matrix) {
            return NULL;
        }

        result = EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(*matrix)));
        delete matrix;
        return result;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return NULL;
    }

    // (a: float, ax: Vector)
    PyErr_Clear();
    if (PyArg_ParseTupleAndKeywords(args, kwargs, "dO!:compute_rotation_matrix",
        const_cast<char**>(kwlist_empty), &angle, &EVSpace_VectorType, &vector_axis))
    {
        matrix = _EVSpaceRotate_ComputeMatrix(angle, vector_axis->vector);
        if (!matrix) {
            return NULL;
        }

        result = EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(*matrix)));
        delete matrix;
        return result;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return NULL;
    }
    
    // (o: RotationOrder, a: EulerAngles, i: bool = True)
    PyErr_Clear();
    if (PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!|$p", const_cast<char**>(kwlist_euler),
        &EVSpace_OrderType, &order_from, &EVSpace_AnglesType, &angles_from, &intrinsic_from))
    {
        matrix = _EVSpaceRotate_ComputeMatrix(order_from, angles_from, intrinsic_from);
        if (!matrix) {
            return NULL;
        }

        result = EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(*matrix)));
        delete matrix;
        return result;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return NULL;
    }

    // (of: RotationOrder, af: EulerAngles, ot: RotationOrder, at: EulerAngles,
    //  if: bool = True, it: bool = True)
    PyErr_Clear();
    if (PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!O!O!|$pp", const_cast<char**>(kwlist_between),
        &EVSpace_OrderType, &order_from, &EVSpace_AnglesType, &angles_from,
        &EVSpace_OrderType, &order_to, &EVSpace_AnglesType, &angles_to,
        &intrinsic_from, &intrinsic_to))
    {
        tmp = _EVSpaceRotate_ComputeMatrix(order_from, angles_from, intrinsic_from);
        if (!tmp) {
            return NULL;
        }

        matrix = _EVSpaceRotate_ComputeMatrix(order_to, angles_to, intrinsic_to);
        if (!matrix) {
            return NULL;
        }

        matrix->transpose_inplace();
        (*matrix) *= *tmp;

        result = EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(*matrix)));
        delete matrix;
        delete tmp;
        return result;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return NULL;
    }

    PyErr_Clear();
    __EVSpace_ComputeMatrixError(args);

    return NULL;
}

// Checks the value of offset for Py_None and fills vector_offset if applicable.
// offset_not_none is set and returns -1 on error and 0 on success.
static inline int
__EVSpaceRotate_HandleOffset(PyObject* offset, evspace::Vector** vector_offset,
                             bool& offset_not_none)
{
#if PY_VERSION_HEX >= 0x030a0000
    if (!Py_IsNone(offset))
#else
    if (offset != Py_None)
#endif
    {
        if (!EVSpaceVector_Check(offset)) {
            PyErr_SetString(PyExc_TypeError, "offset keyword argument expected pyevspace.Vector type");
            return -1;
        }
        else {
            **vector_offset = *EVSpaceVector_Cast(offset)->vector;
            offset_not_none = true;
        }
    }
    else {
        offset_not_none = false;
    }

    return 0;
}

// Checks the exception state and determines if the set exception was
// from a keyword argument error. I really hate this function and
// not entirely sure it's correct but I would like to return TypeError
// on invalid keyword arguments first since taking the time to validate
// potentially complex signatures is pointless when an invalid keyword
// argument exists.
static bool
__EVSpaceRotate_CheckKeywordException(void)
{
    const char* error_message = NULL;
    bool keyword_error = false;
    PyObject* exc;
    static const char* error_template = "keyword argument";

#if PY_VERSION_HEX < 0x030c0000
    PyObject* type, *traceback;
    PyErr_Fetch(&type, &exc, &traceback);
#else
    PyObject* exc_args;
    exc = PyErr_GetRaisedException();
#endif

    if (exc)
    {
#if PY_VERSION_HEX < 0x030c0000
        if (!PyErr_GivenExceptionMatches(type, PyExc_TypeError))
        {
            Py_XDECREF(exc);
            Py_XDECREF(type);
            Py_XDECREF(traceback);

            return false;
        }

        // This must outlive error_message as the garbage collection will release the memory for error_message
        PyObject* str_obj = PyObject_Str(exc);
        if (!str_obj) {
            return NULL;
        }

        error_message = PyUnicode_AsUTF8(str_obj);
        
        if (!error_message) {
            Py_DECREF(str_obj);
            return NULL;
        }

        keyword_error = (error_message != NULL &&
                         strstr(error_message, error_template));
        Py_DECREF(str_obj);
#else
        if (!PyErr_GivenExceptionMatches(exc, PyExc_TypeError))
        {
            Py_XDECREF(exc);
            return false;
        }

        exc_args = PyException_GetArgs(exc);
        if (PyArg_ParseTuple(exc_args, "s", &error_message))
        {
            keyword_error = (error_message != NULL &&
                             strstr(error_message, error_template));
        }
        Py_XDECREF(exc_args);
#endif
    }

    if (keyword_error)
    {
#if PY_VERSION_HEX < 0x030c0000
        PyErr_Restore(type, exc, traceback);
    }
    else {
        Py_XDECREF(exc);
        Py_XDECREF(type);
        Py_XDECREF(traceback);
#else
        PyErr_SetRaisedException(exc);
    }
    else {
        Py_XDECREF(exc);
#endif
    }

    return keyword_error;
}

// Analyze the args tuple and deduce the most likely intended signature
// and produce an informative type error. The `args` parameter MUST be
// a python tuple type. This function does not care about keyword arguments
// and keyword argument errors should be handled separately.
static void
_EVSpaceRotate_ProcessArgsError(PyObject* args)
{
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    PyObject* arg1, *arg2, *arg3;
    double dummy_float;

    arg1 = PyTuple_GET_ITEM(args, 0);
    arg2 = PyTuple_GET_ITEM(args, 1);
    if (arg_count == 2)
    {
        if (!EVSpaceMatrix_Check(arg1))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be a Matrix type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return;
        }

        if (!EVSpaceVector_Check(arg2))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 2 to be a Vector type, got %s",
                         __EVSpace_GetTypeName(arg2));
            return;
        }
    }
    else if (arg_count == 3)
    {
        arg3 = PyTuple_GET_ITEM(args, 2);

        // First assume first arg should be float or Order to determine intended
        // signature, then keep checking args to produce error message. If
        // neither type for first give generic-ish error
        if (EVSpaceObject_AsDouble(arg1, dummy_float))
        {
            if (!EVSpaceVector_Check(arg2) && !PyLong_Check(arg2))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 2 to be an int or Vector type, got %s",
                             __EVSpace_GetTypeName(arg2));
                return;
            }
            else if (!EVSpaceVector_Check(arg3))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 3 to be a Vector type, got %s",
                             __EVSpace_GetTypeName(arg3));
                return;
            }
        }
        else if (EVSpaceOrder_Check(arg1))
        {
            if (!EVSpaceAngles_Check(arg2))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 2 to be an EulerAngles type, got %s",
                             __EVSpace_GetTypeName(arg2));
                return;
            }
            else if (!EVSpaceVector_Check(arg3))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 3 to be a Vector type, got %s",
                             __EVSpace_GetTypeName(arg3));
                return;
            }
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be a float or RotationOrder type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return;
        }
    }
    else
    {
        PyErr_Format(PyExc_TypeError,
                     "expected 2 or 3 positional arguments, got %d", arg_count);
        return;
    }

    
    // We shouldn't be here if at least one combination above fails, give
    // an error so we know something went wrong.
    PyErr_SetString(PyExc_RuntimeError, "unknown error parsing arguments");
}

// Generates the rotation matrix based on args and kwargs while also filling
// vector and vector_offset to their provided values in args. The variable
// offset_not_none is set to true or false depending on if the `offset` keyword
// argument is None or not. `name` is the originating function name used for
// error messaging.
static int
_EVSpaceRotate_ProcessArgs(PyObject* args, PyObject* kwargs, evspace::Matrix** matrix,
                           evspace::Vector** vector, evspace::Vector** vector_offset,
                           bool& offset_not_none, const char* name)
{
    // signatures that we need to accept:
    // (matrix: Matrix, vector: Vector, *, offset: Vector = None)
    // (angle: float, axis: int, vector: Vector, *, offset: Vector = None)
    // (angle: float, axis: Vector, vector: Vector, *, offset: Vector = None)
    // (order: RotationOrder, angles: EulerAngles, vector: Vector, *, offset: Vector = None,
    //  intrinsic: bool = True)

    PyObject* offset = Py_None;
    evspace::Matrix* matrix_tmp;
    // evspace::Vector* vector_tmp;
    EVSpace_Matrix* matrix_wrapper;
    EVSpace_Vector* vector_wrapper, *vector_axis;
    EVSpace_Order* order;
    EVSpace_Angles* angles;
    double angle;
    int axis, intrinsic = 1;
    char format[64];

    static const char* kwlist_matrix[] = { "", "", "offset", NULL };
    static const char* kwlist_axis[] = { "", "", "", "offset", NULL };
    static const char* kwlist_euler[] = { "", "", "", "offset", "intrinsic", NULL };

    // (m: Matrix, v: Vector, *, o: Vector = None)
    strcpy(format, "O!O!|$O:");
    strcat(format, name);
    if (PyArg_ParseTupleAndKeywords(args, kwargs, format, const_cast<char**>(kwlist_matrix),
        &EVSpace_MatrixType, &matrix_wrapper, &EVSpace_VectorType, &vector_wrapper, &offset))
    {
        if (__EVSpaceRotate_HandleOffset(offset, vector_offset, offset_not_none) < 0) {
            return -1;
        }

        **matrix = EVSpaceMatrix_MATRIX(matrix_wrapper);
        **vector = EVSpaceVector_VECTOR(vector_wrapper);

        return 0;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return -1;
    }

    // (a: float, x: int, v: Vector, *, o: Vector = None)
    PyErr_Clear();
    strcpy(format, "diO!|$O:");
    strcat(format, name);
    if (PyArg_ParseTupleAndKeywords(args, kwargs, format, const_cast<char**>(kwlist_axis),
        &angle, &axis, &EVSpace_VectorType, &vector_wrapper, &offset))
    {
        if (__EVSpaceRotate_HandleOffset(offset, vector_offset, offset_not_none) < 0) {
            return -1;
        }

        matrix_tmp = _EVSpaceRotate_ComputeMatrix(
            angle, static_cast<evspace::AxisDirection>(axis));
        if (!matrix_tmp) {
            return -1;
        }

        **matrix = std::move(*matrix_tmp);
        delete matrix_tmp;
        **vector = EVSpaceVector_VECTOR(vector_wrapper);

        return 0;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return -1;
    }

    // (a: float, x: Vector, v: Vector, *, o: Vector = None)
    PyErr_Clear();
    strcpy(format, "dO!O!|$O:");
    strcat(format, name);
    if (PyArg_ParseTupleAndKeywords(args, kwargs, format, const_cast<char**>(kwlist_axis),
        &angle, &EVSpace_VectorType, &vector_axis, &EVSpace_VectorType,
        &vector_wrapper, &offset))
    {
        if (__EVSpaceRotate_HandleOffset(offset, vector_offset, offset_not_none) < 0) {
            return -1;
        }

        matrix_tmp = _EVSpaceRotate_ComputeMatrix(angle, &EVSpaceVector_VECTOR(vector_axis));
        if (!matrix_tmp) {
            return -1;
        }

        **matrix = std::move(*matrix_tmp);
        delete matrix_tmp;
        **vector = EVSpaceVector_VECTOR(vector_wrapper);

        return 0;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return -1;
    }

    // (o: RotationOrder, a: EulerAngles, v: Vector, *, off: Vector = None, i: bool = True)
    PyErr_Clear();
    strcpy(format, "O!O!O!|$Op:");
    strcat(format, name);
    if (PyArg_ParseTupleAndKeywords(args, kwargs, format, const_cast<char**>(kwlist_euler),
        &EVSpace_OrderType, &order, &EVSpace_AnglesType, &angles,
        &EVSpace_VectorType, &vector_wrapper, &offset, &intrinsic))
    {
        if (__EVSpaceRotate_HandleOffset(offset, vector_offset, offset_not_none) < 0) {
            return -1;
        }

        matrix_tmp = _EVSpaceRotate_ComputeMatrix(order, angles, intrinsic);
        if (!matrix_tmp) {
            return -1;
        }

        **matrix = std::move(*matrix_tmp);
        delete matrix_tmp;
        **vector = EVSpaceVector_VECTOR(vector_wrapper);

        return 0;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return -1;
    }
    
    PyErr_Clear();
    _EVSpaceRotate_ProcessArgsError(args);

    return -1;
}

static PyObject*
EVSpaceRotate_From(PyObject* Py_UNUSED, PyObject* args, PyObject* kwargs)
{
    evspace::Matrix* matrix;
    evspace::Vector* vector, *vector_offset;
    bool offset_not_none = true;

    EXCEPTION_WRAPPER(
        matrix = new evspace::Matrix();
        vector = new evspace::Vector();
        vector_offset = new evspace::Vector();
    )

    // Use _EVSpaceRotate_ProcessArgs to handle generating the matrix and
    // filling the vector and vector_offset values.
    if (_EVSpaceRotate_ProcessArgs(args, kwargs, &matrix, &vector,
                                   &vector_offset, offset_not_none, "rotate_from") < 0)
    {
        delete matrix;
        delete vector;
        delete vector_offset;
        return NULL;
    }
    
    evspace::Vector* result;
    if (offset_not_none) {
        result = _EVSpaceRotate_From(matrix, vector, vector_offset);
    }
    else {
        result = _EVSpaceRotate_From(matrix, vector);
    }
    delete matrix;
    delete vector;
    delete vector_offset;

    if (!result) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        EVSpace_Vector* rtn = EVSpaceVector_New(std::move(*result));
        return EVS_PyObject_Cast(rtn);
    )
}

static PyObject*
EVSpaceRotate_To(PyObject* Py_UNUSED, PyObject* args, PyObject* kwargs)
{
    evspace::Matrix* matrix;
    evspace::Vector* vector, *vector_offset;
    bool offset_not_none = true;

    EXCEPTION_WRAPPER(
        matrix = new evspace::Matrix();
        vector = new evspace::Vector();
        vector_offset = new evspace::Vector();
    )

    // Use _EVSpaceRotate_ProcessArgs to handle generating the matrix and
    // filling the vector and vector_offset values.
    if (_EVSpaceRotate_ProcessArgs(args, kwargs, &matrix, &vector,
                                   &vector_offset, offset_not_none, "rotate_from") < 0)
    {
        delete matrix;
        delete vector;
        delete vector_offset;
        return NULL;
    }
    
    evspace::Vector* result;
    if (offset_not_none) {
        result = _EVSpaceRotate_To(matrix, vector, vector_offset);
    }
    else {
        result = _EVSpaceRotate_To(matrix, vector);
    }
    delete matrix;
    delete vector;
    delete vector_offset;

    if (!result) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        EVSpace_Vector* rtn = EVSpaceVector_New(std::move(*result));
        return EVS_PyObject_Cast(rtn);
    )
}

static PyObject*
EVSpaceRotate_Between(PyObject* Py_UNUSED(_), PyObject* args, PyObject* kwargs)
{
    EVSpace_Order* order_from, *order_to;
    EVSpace_Angles* angles_from, *angles_to;
    evspace::Matrix* matrix_from, *matrix_to;
    evspace::Vector* result, *vector_offset_from = NULL, *vector_offset_to = NULL;
    PyObject* offset_from = Py_None, *offset_to = Py_None;
    EVSpace_Vector* rtn, *vector_wrapper;
    int intrinsic_from = 1, intrinsic_to = 1;

    static const char* kwlist[] = {"", "", "", "", "", "intrinsic_from", "intrinsic_to",
                                   "offset_from", "offset_to", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!O!O!O!|$ppOO:rotate_between", const_cast<char**>(kwlist),
        &EVSpace_OrderType, &order_from, &EVSpace_AnglesType, &angles_from,
        &EVSpace_OrderType, &order_to, &EVSpace_AnglesType, &angles_to,
        &EVSpace_VectorType, &vector_wrapper, &intrinsic_from, &intrinsic_to,
        &offset_from, &offset_to))
    {
        return NULL;
    }

#if PY_VERSION_HEX >= 0x030a0000
    if (!Py_IsNone(offset_from))
#else
    if (offset_from != Py_None)
#endif
    {
        if (!EVSpaceVector_Check(offset_from)) {
            PyErr_SetString(PyExc_TypeError, "offset_from argument cannot be interpreted as Vector");
            return NULL;
        }
        vector_offset_from = EVSpaceVector_Cast(offset_from)->vector;
    }

#if PY_VERSION_HEX >= 0x030a0000
    if (!Py_IsNone(offset_to))
#else
    if (offset_from != Py_None)
#endif
    {
        if (!EVSpaceVector_Check(offset_to)) {
            PyErr_SetString(PyExc_TypeError, "offset_to argument cannot be interpreted as Vector");
            return NULL;
        }
        vector_offset_to = EVSpaceVector_Cast(offset_to)->vector;
    }

    matrix_from = _EVSpaceRotate_ComputeMatrix(order_from, angles_from, intrinsic_from);
    if (!matrix_from) {
        return NULL;
    }

    matrix_to = _EVSpaceRotate_ComputeMatrix(order_to, angles_to, intrinsic_to);
    if (!matrix_to) {
        delete matrix_from;
        return NULL;
    }

    result = _EVSpaceRotate_Between(matrix_from, matrix_to, vector_offset_from,
                                    vector_offset_to, vector_wrapper->vector);
    delete matrix_from;
    delete matrix_to;
    if (!result) {
        return NULL;
    }

    rtn = EVSpaceVector_New(std::move(*result));
    delete result;

    return EVS_PyObject_Cast(rtn);
}

/* global module level methods */

PyDoc_STRVAR(vector_dot_doc, "pyevspace.vector_dot(lhs: pyevspace.Vector, rhs: pyevspace.Vector) -> float\n\
\n\
Computes the vector dot product of two pyevspace.Vectors.");

PyDoc_STRVAR(vector_cross_doc, "pyevspace.vector_cross(lhs: pyevspace.Vector, rhs: pyevspace.Vector) -> pyevspace.Vector\n\
\n\
Computes the vector cross product of two pyevspace.Vectors using the right-hand rule.");

PyDoc_STRVAR(vector_angle_doc, "pyevspace.vector_angle(from: pyevspace.Vector, to: pyevspace.Vector) -> float\n\
\n\
Determines the smallest angle between two vectors. The arguments\n\
are commutative, i.e. order of `from` and `to` does not matter.");

PyDoc_STRVAR(vector_exclude_doc, "pyevspace.vector_exclude(vector: pyevspace.Vector, exclude: pyevspace.Vector) -> pyevspace.Vector\n\
\n\
Removes all portions of `exclude` from `vector`. This is equivalent to\n\
projecting `vector` onto the plane whose normal vector is `exclude`. The\n\
result of this function and `exclude` will have a dot product of zero.");

PyDoc_STRVAR(vector_projection_doc, "pyevspace.vector_proj(project: pyevspace.Vector, onto: pyevspace.Vector) -> pyevspace.Vector\n\
\n\
Projects the vector `project` onto the vector `onto`. Equivalent to\n\
the vector dot product of `project` and `onto` divided by the magnitude\n\
of `onto`.");

PyDoc_STRVAR(compute_rotation_doc,
    "compute_rotation_matrix(angle: float, axis: int | Vector) -> Vector\n\
compute_rotation_matrix(order: RotationOrder, angles: EulerAngles, *, intrinsic: bool = True) -> Vector\n\
compute_rotation_matrix(order_from: RotationOrder, angles_from: EulerAngles, order_to: RotationOrder, angles_to: EulerAngles, *, intrinsic_from: bool = True, intrinic_to: bool = True) -> Vector\n\
Computes the rotation matrix for the reference frame defined by the arguments.");

PyDoc_STRVAR(rotate_from_doc,
"rotate_from(matrix: Matrix, vector: Vector, *, offset: Vector = None) -> Vector\n\
rotate_from(angle: float, axis: int | Vector, vector: Vector, *, offset: Vector = None) -> Vector)\n\
rotate_from(order: RotationOrder, angles: EulerAngles, vector: Vector, *, offset: Vector = None,\
intrinsic: bool = True) -> Vector\n\
\n\
Rotates a vector from a reference frame defined by the combination of function arguments.");

PyDoc_STRVAR(rotate_to_doc,
"rotate_to(matrix: Matrix, vector: Vector, *, offset: Vector = None) -> Vector\n\
rotate_from(angle: float, axis: int | Vector, vector: Vector, *, offset: Vector = None) -> Vector\n\
rotate_from(order: RotationOrder, angles: EulerAngles, vector: Vector, *, offset: Vector = None) -> Vector,\
intrinsic: bool = True) -> Vector\n\
\n\
Rotates a vector to a reference frame defined by the function arguments.");

PyDoc_STRVAR(rotate_between_doc,
"rotate_between(order_from: RotationOrder, angles_from: EulerAngles, order_to: RotationOrder,\
angles_to: EulerAngles, vector: Vector, *, intrinsic_from: bool = True, intrinsic_to: bool = True,\
offset_from: Vector = None, offset_to: Vector = None) -> Vector\n\
\n\
Rotates vector between reference frames defined by the rotation axis orders and angles.");

static PyMethodDef evspace_methods[] = {

    {"vector_dot", (PyCFunction)Vector_dot, METH_FASTCALL, vector_dot_doc},

    {"vector_cross", (PyCFunction)Vector_cross, METH_FASTCALL, vector_cross_doc},

    {"vector_angle", (PyCFunction)Vector_angle, METH_FASTCALL, vector_angle_doc},

    {"vector_exclude", (PyCFunction)Vector_exclude, METH_FASTCALL, vector_exclude_doc},

    {"vector_proj", (PyCFunction)Vector_projection, METH_FASTCALL, vector_projection_doc},

    {"compute_rotation_matrix", (PyCFunction)(void(*)(void))EVSpaceRotation_ComputeMatrix,
     METH_VARARGS | METH_KEYWORDS, compute_rotation_doc },

    {"rotate_from", (PyCFunction)(void(*)(void))EVSpaceRotate_From,
     METH_VARARGS | METH_KEYWORDS, rotate_from_doc},

    {"rotate_to", (PyCFunction)(void(*)(void))EVSpaceRotate_To,
     METH_VARARGS | METH_KEYWORDS, rotate_to_doc},

    {"rotate_between", (PyCFunction)(void(*)(void))EVSpaceRotate_Between,
     METH_VARARGS | METH_KEYWORDS, rotate_between_doc},

    {NULL}
};

PyDoc_STRVAR(evspace_doc, "A 3-dimensional Euclidean vector space package for\
vector and matrix types with support for reference frames and Euler rotations.");

static PyModuleDef EVSpace_Module = {
    PyModuleDef_HEAD_INIT,
    "pyevspace",        // m_name
    evspace_doc,        // m_doc
    -1,                 // m_size
    evspace_methods     // m_methods
};

static PyObject*
_initialize_vector_constants(PyObject* dict)
{
    EVSpace_Vector* tmp;
    EXCEPTION_WRAPPER(
        tmp = EVSpaceVector_New(evspace::Vector::e1);
        if (PyDict_SetItemString(dict, "E1", EVS_PyObject_Cast(tmp)) < 0) {
            return NULL;
        }

        tmp = EVSpaceVector_New(evspace::Vector::e2);
        if (PyDict_SetItemString(dict, "E2", EVS_PyObject_Cast(tmp)) < 0) {
            return NULL;
        }

        tmp = EVSpaceVector_New(evspace::Vector::e3);
        if (PyDict_SetItemString(dict, "E3", EVS_PyObject_Cast(tmp)) < 0) {
            return NULL;
        }
    )

    return dict;
}

static PyObject*
_initialize_matrix_constants(PyObject* dict)
{
    EVSpace_Matrix* tmp;

    EXCEPTION_WRAPPER(
        tmp = EVSpaceMatrix_New(evspace::Matrix::IDENTITY);

        if (PyDict_SetItemString(dict, "IDENTITY", EVS_PyObject_Cast(tmp)) < 0) {
            return NULL;
        }
    )

    return dict;
}

template<typename T>
struct _EVSpaceOrder_InitDelegate {
    static inline int add_order(PyObject* module, const char* name);
};

template<typename first, typename second, typename third>
struct _EVSpaceOrder_InitDelegate<evspace::RotationOrder<first, second, third>> {

    static inline int add_order(PyObject* module, const char* name)
    {
        // Adds the templated RotationOrder to module with name as identifier. No owned
        // references exist after calling this function so there is no need to cleanup
        // on module initialization failure.

        PyObject* order = EVS_PyObject_Cast(EVSpaceOrder_New(first::direction,
                                                             second::direction,
                                                             third::direction));
        
#if PY_VERSION_HEX >= 0x030d0000
        if (PyModule_Add(module, name, order) < 0) {
            return -1;
        }
#else
        if (PyModule_AddObject(module, name, order) < 0) {
            Py_XDECREF(order);
            return -1;
        }
#endif

        return 0;
    }
};

template<typename T>
static int
_EVSpaceOrder_AddToModule(PyObject* module, const char* name) {
    return _EVSpaceOrder_InitDelegate<T>::add_order(module, name);
}

static PyObject*
_initialize_order_constants(PyObject* module)
{
    if (PyModule_AddIntConstant(module, "X_AXIS", (int)evspace::AxisDirection::X) < 0) {
        return NULL;
    }

    if (PyModule_AddIntConstant(module, "Y_AXIS", (int)evspace::AxisDirection::Y) < 0) {
        return NULL;
    }

    if (PyModule_AddIntConstant(module, "Z_AXIS", (int)evspace::AxisDirection::Z) < 0) {
        return NULL;
    }

    if (_EVSpaceOrder_AddToModule<evspace::XYZ>(module, "XYZ") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::XZY>(module, "XZY") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::YXZ>(module, "YXZ") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::YZX>(module, "YZX") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::ZXY>(module, "ZXY") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::ZYX>(module, "ZYX") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::XYX>(module, "XYX") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::XZX>(module, "XZX") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::YXY>(module, "YXY") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::YZY>(module, "YZY") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::ZXZ>(module, "ZXZ") < 0) { return module; }
    if (_EVSpaceOrder_AddToModule<evspace::ZYZ>(module, "ZYZ") < 0) { return module; }

    return module;
}

static PyTypeObject* const EVSpace_Types[] = {
    &EVSpace_VectorType,
    &EVSpace_MatrixViewType,
    &EVSpace_MatrixType,
    &EVSpace_AnglesType,
    &EVSpace_OrderType,
    &EVSpace_ReferenceFrameType,
};

static int initialize_module(PyObject* module)
{
    vector_as_number.nb_add                     = (binaryfunc)Vector_add;
    vector_as_number.nb_subtract                = (binaryfunc)Vector_subtract;
    vector_as_number.nb_multiply                = (binaryfunc)Vector_multiply;
    vector_as_number.nb_negative                = (unaryfunc)Vector_negative;
    vector_as_number.nb_inplace_add             = (binaryfunc)Vector_inplace_add;
    vector_as_number.nb_inplace_subtract        = (binaryfunc)Vector_inplace_subtract;
    vector_as_number.nb_inplace_multiply        = (binaryfunc)Vector_inplace_multiply;
    vector_as_number.nb_true_divide             = (binaryfunc)Vector_divide;
    vector_as_number.nb_inplace_true_divide     = (binaryfunc)Vector_inplace_divide;
    vector_as_number.nb_matrix_multiply         = (binaryfunc)Vector_matrix_multiply;
    vector_as_number.nb_inplace_matrix_multiply = (binaryfunc)Vector_inplace_multiply_matrix;

    EVSpace_VectorType.tp_name          = "pyevspace.Vector";
    EVSpace_VectorType.tp_basicsize     = sizeof(EVSpace_Vector);
    EVSpace_VectorType.tp_itemsize      = 0;
    EVSpace_VectorType.tp_dealloc       = (destructor)Vector_dealloc;
    EVSpace_VectorType.tp_repr          = (reprfunc)Vector_repr;
    EVSpace_VectorType.tp_as_number     = &vector_as_number;
    EVSpace_VectorType.tp_as_sequence   = &vector_as_sequence;
    EVSpace_VectorType.tp_str           = (reprfunc)Vector_str;
    EVSpace_VectorType.tp_as_buffer     = &vector_buffer;
#if PY_VERSION_HEX >= 0x030a0000
    EVSpace_VectorType.tp_flags         = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE;
#else
    EVSpace_VectorType.tp_flags         = Py_TPFLAGS_DEFAULT;
#endif
    EVSpace_VectorType.tp_doc           = vector_doc;
    EVSpace_VectorType.tp_richcompare   = (richcmpfunc)&Vector_richcompare;
    EVSpace_VectorType.tp_iter          = (getiterfunc)Vector_iter;
    EVSpace_VectorType.tp_methods       = vector_methods;
    EVSpace_VectorType.tp_init          = (initproc)Vector_init;
    EVSpace_VectorType.tp_new           = (newfunc)Vector_new;

    EVSpace_MatrixViewType.tp_name      = "pyevspace._MatrixView";
    EVSpace_MatrixViewType.tp_basicsize = sizeof(EVSpace_MatrixView);
    EVSpace_MatrixViewType.tp_itemsize  = 0;
    EVSpace_MatrixViewType.tp_dealloc   = (destructor)MatrixView_dealloc;
    EVSpace_MatrixViewType.tp_as_buffer = &matrixview_as_buffer;
    EVSpace_MatrixViewType.tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION;

    matrix_as_number.nb_add             = (binaryfunc)Matrix_add;
    matrix_as_number.nb_subtract        = (binaryfunc)Matrix_subtract;
    matrix_as_number.nb_multiply        = (binaryfunc)Matrix_multiply;
    matrix_as_number.nb_negative        = (unaryfunc)Matrix_negative;
    matrix_as_number.nb_inplace_add     = (binaryfunc)Matrix_inplace_add;
    matrix_as_number.nb_inplace_subtract = (binaryfunc)Matrix_inplace_subtract;
    matrix_as_number.nb_inplace_multiply = (binaryfunc)Matrix_inplace_multiply;
    matrix_as_number.nb_true_divide     = (binaryfunc)Matrix_divide;
    matrix_as_number.nb_inplace_true_divide = (binaryfunc)Matrix_inplace_divide;
    matrix_as_number.nb_matrix_multiply = (binaryfunc)Matrix_matrix_multiply;
    matrix_as_number.nb_inplace_matrix_multiply = (binaryfunc)Matrix_inplace_multiply_matrix;

    EVSpace_MatrixType.tp_name          = "pyevspace.Matrix";
    EVSpace_MatrixType.tp_basicsize     = sizeof(EVSpace_Matrix);
    EVSpace_MatrixType.tp_itemsize      = 0;
    EVSpace_MatrixType.tp_dealloc       = (destructor)Matrix_dealloc;
    EVSpace_MatrixType.tp_repr          = (reprfunc)Matrix_repr;
    EVSpace_MatrixType.tp_as_number     = &matrix_as_number;
    EVSpace_MatrixType.tp_as_mapping    = &matrix_as_map;
    EVSpace_MatrixType.tp_str           = (reprfunc)Matrix_str;
    EVSpace_MatrixType.tp_as_buffer     = &matrix_as_buffer;
#if PY_VERSION_HEX >= 0x030a0000
    EVSpace_MatrixType.tp_flags         = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE;
#else
    EVSpace_MatrixType.tp_flags         = Py_TPFLAGS_DEFAULT;
#endif
    EVSpace_MatrixType.tp_doc           = matrix_doc;
    EVSpace_MatrixType.tp_richcompare   = (richcmpfunc)&Matrix_richcompare;
    EVSpace_MatrixType.tp_methods       = matrix_methods;
    EVSpace_MatrixType.tp_init          = (initproc)Matrix_init;
    EVSpace_MatrixType.tp_new           = (newfunc)Matrix_new;

    EVSpace_AnglesType.tp_name          = "pyevspace.EulerAngles";
    EVSpace_AnglesType.tp_basicsize     = sizeof(EVSpace_Angles);
    EVSpace_AnglesType.tp_itemsize      = 0;
    EVSpace_AnglesType.tp_dealloc       = (destructor)Angles_dealloc;
    EVSpace_AnglesType.tp_repr          = (reprfunc)Angles_repr;
    EVSpace_AnglesType.tp_as_sequence   = &angles_as_sequence;
    EVSpace_AnglesType.tp_str           = (reprfunc)Angles_str;
#if PY_VERSION_HEX >= 0x030a0000
    EVSpace_AnglesType.tp_flags         = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE;
#else
    EVSpace_AnglesType.tp_flags         = Py_TPFLAGS_DEFAULT;
#endif
    EVSpace_AnglesType.tp_doc           = angles_doc;
    EVSpace_AnglesType.tp_methods       = angles_methods;
    EVSpace_AnglesType.tp_init          = (initproc)Angles_init;
    EVSpace_AnglesType.tp_new           = (newfunc)Angles_new;

    EVSpace_OrderType.tp_name           = "pyevspace.RotationOrder";
    EVSpace_OrderType.tp_basicsize      = sizeof(EVSpace_Order);
    EVSpace_OrderType.tp_itemsize       = 0;
    EVSpace_OrderType.tp_dealloc        = (destructor)Order_dealloc;
    EVSpace_OrderType.tp_repr           = (reprfunc)Order_repr;
    EVSpace_OrderType.tp_as_sequence    = &order_as_sequence;
    EVSpace_OrderType.tp_hash           = (hashfunc)Order_hash;
    EVSpace_OrderType.tp_str            = (reprfunc)Order_str;
#if PY_VERSION_HEX >= 0x030a0000
    EVSpace_OrderType.tp_flags          = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE;
#else
    EVSpace_OrderType.tp_flags          = Py_TPFLAGS_DEFAULT;
#endif
    EVSpace_OrderType.tp_doc            = order_doc;
    EVSpace_OrderType.tp_richcompare    = (richcmpfunc)&Order_richcompare;
    EVSpace_OrderType.tp_methods        = order_methods;
    EVSpace_OrderType.tp_members        = order_members;
    EVSpace_OrderType.tp_init           = (initproc)Order_init;
    EVSpace_OrderType.tp_new            = (newfunc)Order_new;

    EVSpace_ReferenceFrameType.tp_name      = "pyevspace.ReferenceFrame";
    EVSpace_ReferenceFrameType.tp_basicsize = sizeof(EVSpace_ReferenceFrame);
    EVSpace_ReferenceFrameType.tp_itemsize  = 0;
    EVSpace_ReferenceFrameType.tp_dealloc   = (destructor)ReferenceFrame_Dealloc;
    EVSpace_ReferenceFrameType.tp_flags     = Py_TPFLAGS_DEFAULT;
    EVSpace_ReferenceFrameType.tp_methods   = reference_frame_methods;
    EVSpace_ReferenceFrameType.tp_members   = reference_frame_members;
    EVSpace_ReferenceFrameType.tp_getset    = reference_frame_getset;
    EVSpace_ReferenceFrameType.tp_new       = (newfunc)ReferenceFrame_New;

    int count = sizeof(EVSpace_Types) / sizeof(EVSpace_Types[0]);
    for (int i = 0; i < count; i++)
    {
#if PY_VERSION_HEX >= 0x03090000
        if (PyModule_AddType(module, EVSpace_Types[i]) < 0) {
            return -1;
        }
#else
        const char* type_name = Py_TYPE(EVSpace_Types[i]).tp_name;
        const char* name = strrchr(type_name, '.');
        if (name == nullptr) {
            return -1;
        }
        else {
            name += 1;
        }

        if (PyType_Ready(EVSpace_Types[i]) < 0) {
            return -1;
        }

        if (PyModule_AddObject(module, name (PyObject*)EVSpace_Types[i]) < 0) {
            return -1;
        }
#endif
    }

    PyObject* dict = EVSpace_VectorType.tp_dict;
    if (!_initialize_vector_constants(dict)) {
        return -1;
    }

    dict = EVSpace_MatrixType.tp_dict;
    if (!_initialize_matrix_constants(dict)) {
        return -1;
    }

    if (!_initialize_order_constants(module)) {
        return -1;
    }

    return 0;
}

PyMODINIT_FUNC
PyInit__pyevspace(void)
{
    PyObject* module = PyModule_Create(&EVSpace_Module);
    if (!module) {
        return NULL;
    }

    if (initialize_module(module) < 0) {
        Py_DECREF(module);
        return NULL;
    }

    return module;
}
