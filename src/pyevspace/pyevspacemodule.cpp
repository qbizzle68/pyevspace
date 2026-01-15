#include <pyevspacemodule.hpp>

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#if PY_VERSION_HEX < 0x03120000
#   include <structmember.h>
#endif

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

    int result;
    while ((result = PyIter_NextItem(iterator, &item)) > 0)
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
    if (result == -1) {
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

        // todo: make evspace::Vector take an array
        self->vector = new evspace::Vector(array[0], array[1], array[2]);
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
    // todo: evspace doesn't use ulp based comparisons, so the tests
    // currently might fail until that's implemented

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

static PyMethodDef vector_methods[] = {

    {"magnitude", (PyCFunction)Vector_magnitude, METH_NOARGS, vector_mag_doc},

    {"magnitude_squared", (PyCFunction)Vector_magnitude_square, METH_NOARGS, vector_mag2_doc},

    {"normalize", (PyCFunction)Vector_normalize, METH_NOARGS, vector_normalize_doc},

    {"norm", (PyCFunction)Vector_norm, METH_NOARGS, vector_norm_doc},

    {"__reduce__", (PyCFunction)Vector_reduce, METH_NOARGS, vector_reduce_doc},

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
            return NULL;
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

static PyObject*
Matrix_map_subscript(EVSpace_Matrix* self, PyObject* indices)
{
    int index0, index1, result;
    PyObject* key0, *key1, *slice0, *slice1;
    Py_ssize_t start0, stop0, step0, slicelength0, start1, stop1, step1, slicelength1;
    EVSpace_MatrixView* matrix_view;

    if (PyLong_Check(indices))
    {
        start0 = PyLong_AsInt(indices);
        if (start0 == -1 && PyErr_Occurred()) {
            return NULL;
        }

        stop0 = start0 + 1;
        step0 = 1;
        PySlice_AdjustIndices(3, &start0, &stop0, step0);
        // todo: do we raise an error here or just return the empty view like below?
        if (start0 == stop0) {
            PyErr_Format(PyExc_IndexError, "index %i out of bounds, must "
                                           "be between [0-2]", start0);
            return NULL;
        }

        matrix_view = EVSpaceMatrixView_New(self, 1, 3 * start0, 3, -1, sizeof(double), -1);
    }
    else if (PySlice_Check(indices))
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
    }
    else if (PyTuple_Check(indices))
    {
        // possible combination is (int, int), (int, slice), (slice, int), (slice, slice)
        if (PyArg_ParseTuple(indices, "OO:pyevspace.Matrix.__getitem__", &key0, &key1) < 0) {
            return NULL;
        }

        if (PyLong_Check(key0))
        {
            index0 = PyLong_AsInt(key0);
            if (index0 == -1 && PyErr_Occurred()) {
                return NULL;
            }

            start0 = index0;
            stop0 = start0 + 1;
            PySlice_AdjustIndices(3, &start0, &stop0, 1);
            // I think we can rely on start0 != stop0 signaling valid index
            if (start0 == stop0) {
                PyErr_Format(PyExc_IndexError, "first index (got %i) out of range",
                                index0);
                return NULL;
            }

            // (int, int)
            if (PyLong_Check(key1))
            {
                index1 = PyLong_AsInt(key1);
                if (index0 == -1 && PyErr_Occurred()) {
                    return NULL;
                }

                start1 = index1;
                stop1 = index1 + 1;
                PySlice_AdjustIndices(3, &start1, &stop1, 1);

                if (start1 == stop1) {
                    PyErr_Format(PyExc_IndexError, "second index (got %i) out of range",
                                 index1);
                    return NULL;
                }

                return PyFloat_FromDouble(EVSpaceMatrix_MATRIX(self)(start0, start1));
            }
            // (int, slice)
            else if (PySlice_Check(key1))
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
            }
            else {
                PyErr_SetString(PyExc_TypeError, "second index must be int or slice type");
            }
        }
        else if (PySlice_Check(key0))
        {
            if (PySlice_GetIndicesEx(key0, 3, &start0, &stop0, &step0, &slicelength0) < 0) {
                return NULL;
            }

            if (start0 == stop0) {
                matrix_view = EVSpaceMatrixView_New(self, 1, 0, 0, -1, sizeof(double), -1);
                return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
            }
            
            // (slice, int)
            if (PyLong_Check(key1))
            {
                index1 = PyLong_AsInt(key1);
                if (index1 == -1 && PyErr_Occurred()) {
                    return NULL;
                }
                
                matrix_view = EVSpaceMatrixView_New(self, 1, 3 * start0 + index1, slicelength0,
                                                        -1, 3 * step0 * sizeof(double), -1);
            }
            // (slice, slice)
            else if (PySlice_Check(key1))
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
            }
            else {
                PyErr_SetString(PyExc_TypeError, "second index must be int or slice type");
                return NULL;
            }
        }
        else {
            PyErr_SetString(PyExc_TypeError, "first index must be int or slice type");
            return NULL;
        }
    }
    else {
        PyErr_SetString(PyExc_TypeError, "first index must be int or slice type");
        return NULL;
    }

    return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
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
            // todo: can we use data() here to optimized?
            EVSpaceMatrix_MATRIX(self)(index0, index1) = value;

            return 0;
        }
        catch (const std::exception &e) {
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return -1;
        }
    }

    // will this raise an exception that will be propagated?
    PyErr_Clear();
    if (PyArg_ParseTuple(indices, "O!O!:pyevspace.Matrix.__getitem__", &PySlice_Type, &key0, &PySlice_Type, &key1) ||
        PyArg_ParseTuple(indices, "nO!:pyevspace.Matrix.__getitem__", &index0, &PySlice_Type, &key0) ||
        PyArg_ParseTuple(indices, "O!n:pyevspace.Matrix.__getitem__", &PySlice_Type, &key0, &index0))
    {
        PyErr_SetString(PyExc_Warning, "slice assignment not yet implemented");
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

static PyMethodDef matrix_methods[] = {

    {"__reduce__", (PyCFunction)Matrix_reduce, METH_NOARGS, matrix_reduce_doc},

    {"transpose", (PyCFunction)Matrix_transpose, METH_NOARGS, matrix_transpose_doc},

    {"transpose_inplace", (PyCFunction)Matrix_transpose_inplace, METH_NOARGS, matrix_transpose_inplace_doc},

    {"determinate", (PyCFunction)Matrix_determinate, METH_NOARGS, matrix_determinate_doc},

    {"inverse", (PyCFunction)Matrix_inverse, METH_NOARGS, matrix_inverse_doc},

    {NULL}
};

PyDoc_STRVAR(matrix_doc, "pyevspace.matrix([row0: Iterable, row1: Iterable, row2: Iterable])\n\
\n\
The pyevspace.Matrix type can be constructed with rows of iterables all of\
length three, whose components are numeric types. Alternatively if no arguments\
are present each component is defaulet to zero.");

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

static PyMethodDef evspace_methods[] = {

    {"vector_dot", (PyCFunction)Vector_dot, METH_FASTCALL, vector_dot_doc},

    {"vector_cross", (PyCFunction)Vector_cross, METH_FASTCALL, vector_cross_doc},

    {"vector_angle", (PyCFunction)Vector_angle, METH_FASTCALL, vector_angle_doc},

    {"vector_exclude", (PyCFunction)Vector_exclude, METH_FASTCALL, vector_exclude_doc},

    {"vector_proj", (PyCFunction)Vector_projection, METH_FASTCALL, vector_projection_doc},

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

static PyTypeObject* const EVSpace_Types[] = {
    &EVSpace_VectorType,
    &EVSpace_MatrixViewType,
    &EVSpace_MatrixType,
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
