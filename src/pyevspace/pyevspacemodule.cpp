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

// todo: remove this when don't developing
#include <iostream>
#include <cstdio>

#define EXCEPTION_WRAPPER(o)    try {\
    o\
}\
catch (const std::bad_alloc&) {\
    return PyErr_NoMemory();\
}\
catch (const std::exception& e) {\
    PyErr_SetString(PyExc_RuntimeError, e.what());\
    return NULL;\
}

static PyTypeObject EVSpace_VectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
};
#define BUFFER_EMPTY            0x0
#define BUFFER_NO_SHAPE         (~(1 << 1))
#define BUFFER_RELEASE_SHAPE    (1 << 1)
#define BUFFER_NO_STRIDES       (~(1 << 2))
#define BUFFER_RELEASE_STRIDES  (1 << 2)

// todo: can this be made const double?
typedef std::array<double, 3> varray_t;

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
    PyObject* iterator = PyObject_GetIter(obj);
    if (!iterator) {
        return -1;
    }

    PyObject *item;
    Py_ssize_t count = 0;
    PyObject* tmp[3]{NULL};

    while (item = PyIter_Next(iterator))
    {
        if (count > 2)
        {
            PyErr_SetString(PyExc_ValueError,
                            "iterable must have exactly 3 items "
                            "(iterable contains more than 3)");
            Py_XDECREF(tmp[0]);
            Py_XDECREF(tmp[1]);
            Py_XDECREF(tmp[2]);
            Py_DECREF(iterator);
            return -1;
        }

        tmp[count++] = item;
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
    try {
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }

        self->vector = new evspace::Vector();
        return self;
    }
    catch (const std::bad_alloc&) {
        return EVSpaceVector_Cast(PyErr_NoMemory());
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static EVSpace_Vector*
_EVSpaceVector_New(const varray_t array, PyTypeObject* type) noexcept
{
    try {
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }

        // todo: make evspace::Vector take an array
        self->vector = new evspace::Vector(array[0], array[1], array[2]);
        return self;
    }
    catch (const std::bad_alloc&) {
        return EVSpaceVector_Cast(PyErr_NoMemory());
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static EVSpace_Vector*
_EVSpaceVector_New(const evspace::Vector& vector, PyTypeObject* type) noexcept
{
    try {
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }
        self->vector = new evspace::Vector(vector);
        return self;
    }
    catch (const std::bad_alloc&) {
        return EVSpaceVector_Cast(PyErr_NoMemory());
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// Force call of the new constructor
static EVSpace_Vector*
_EVSpaceVector_New(evspace::Vector&& vector, PyTypeObject* type) noexcept
{
    try {
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }
        self->vector = new evspace::Vector(std::move(vector));
        return self;
    }
    catch (const std::bad_alloc&) {
        return EVSpaceVector_Cast(PyErr_NoMemory());
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
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
    if (PyErr_Occurred()) {
        std::cout << "An error is set while entering Vector_init()\n";
    }

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
        if (!PyArg_ParseTuple(args, "O", &parameter)) {
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

    try {
        char* buffer = new char[buffer_size + 1];
        if (sprintf(buffer, format, EVSpaceVector_X(self),
                    EVSpaceVector_Y(self), EVSpaceVector_Z(self)) < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to fill buffer on string operation");
            delete [] buffer;
            return NULL;
        }

        PyObject* rtn = PyUnicode_FromString(buffer);
        delete [] buffer;

        return rtn;
    }
    catch (const std::bad_alloc&) {
        return PyErr_NoMemory();
    }
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
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

    EVSpace_Vector* rhs = EVSpaceVector_Cast(other);
    bool is_equal = EVSpaceVector_VECTOR(self) == EVSpaceVector_VECTOR(rhs);
    
    if (EVSpaceVector_Check(rhs))
    {
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

// static PyObject*
// Vector_matrix_multiply(EVSpace_Vector* self, PyObject* other)
// {
//     if (EVSpaceVector_Check(self)) {
//         if (EVSpaceMatrix_Check(other))
//         {

//         }
//     }
// }

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

#if PY_VERSION_HEX >= 0x03100000
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

#if PY_VERSION_HEX >= 0x03100000
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

#if PY_VERSION_HEX >= 0x03100000
        return Py_NewRef(self);
#else
        Py_INCREF(self);
        return (PyObject*)self;
#endif
    }

    Py_RETURN_NOTIMPLEMENTED;
}

// static PyObject*
// Vector_inplace_multiply_matrix(EVSpace_Vector* self, PyObject* arg)
// {
//     EVSpace_Matrix* rhs;
//     if (EVSpaceVector_Check(self)) {
//         rhs = EVSpaceMatrix_Cast(arg);
//         EVSpaceVector_VECTOR(self) *= EVSpaceMatrix_MATRIX(rhs);

// #if PY_VERSION_HEX >= 0x03100000
//         return Py_NewRef(self);
// #else
//         Py_INCREF(self);
//         return (PyObject*)self;
// #endif
//     }

//     Py_RETURN_NOTIMPLEMENTED;
// }

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

#if PY_VERSION_HEX >= 0x03100000
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

// todo: expose the data buffer of evspace::Vector to make this work
/*static int
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
    *internal = BUFFER_NO_STRIDES;

#if PY_VERSION_HEX >= 0x03100000
    view->obj           = Py_NewRef(EVS_PyObject_Cast(obj));
#else
    Py_INCREF(obj);
    view->obj           = EVS_PyObject_Cast(obj);
#endif
    view->buf           = EVSpaceVector_VECTOR(obj).data();
    view->len           = sizeof(double) * 3;
    view->readonly      = 0;
    view->itemsize      = sizeof(double);
    view->ndim          = 1;

    if (flags & PyBUF_ND) {
        view->shape     = shape;
        *internal |= BUFFER_RELEASE_SHAPE;
    }
    else {
        view->shape    = NULL;
        delete shape;
        *internal |= BUFFER_NO_SHAPE;
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

    // view->strides       = &view->itemsize;
    view->suboffsets    = NULL;
    view->internal      = reinterpret_cast<void*>(internal);

    return 0;
}*/

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

// todo: improve this
PyDoc_STRVAR(evspace_doc, "A 3-dimensional Eulclidean vector space module with \
a vector and matrix tpye as well as necessary methods to use them.");

static PyModuleDef EVSpace_Module = {
    PyModuleDef_HEAD_INIT,
    "pyevspace",        // m_name
    evspace_doc,        // m_doc
    -1,                 // m_size
    evspace_methods     // m_methods
};

static PyTypeObject* const EVSpace_Types[] = {
    &EVSpace_VectorType,
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
    // vector_as_number.nb_matrix_multiply         = (binaryfunc)Vector_matrix_multiply;
    // vector_as_number.nb_inplace_matrix_multiply = (binaryfunc)Vector_inplace_multiply_matrix;

    EVSpace_VectorType.tp_name          = "pyevspace.Vector";
    EVSpace_VectorType.tp_basicsize     = sizeof(EVSpace_Vector);
    EVSpace_VectorType.tp_itemsize      = 0;
    EVSpace_VectorType.tp_dealloc       = (destructor)Vector_dealloc;
    EVSpace_VectorType.tp_repr          = (reprfunc)Vector_repr;
    EVSpace_VectorType.tp_as_number     = &vector_as_number;
    EVSpace_VectorType.tp_as_sequence   = &vector_as_sequence;
    EVSpace_VectorType.tp_str           = (reprfunc)Vector_str;
    // EVSpace_VectorType.tp_as_buffer     = &vector_buffer;
#if PY_VERSION_HEX >= 0x03100000
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
