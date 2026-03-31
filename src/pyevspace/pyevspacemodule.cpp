// Memory allocation should used using C-style allocators like malloc/calloc and
// use free() to release the memory. For allocating memory for C++ types, the `new`
// operator should be used, as the associated `delete` operator will call the
// destructor unlike free().

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#if PY_VERSION_HEX < 0x030c0000
#   include <structmember.h>
#endif

#include <pyevspacemodule.hpp>
#define _EVS_PYEVSPACE_IMPL
#include <pyevspace-api.hpp>
#include <cfloat>       // DBL_EPSILON
#include <cstdint>      // int64_t
#include <cstddef>      // offsetof
#include <array>        // std::array
#include <string_view>
#include <vector>

// todo: pick a single allocator function and use it consistently so
// the free/delete call is identical across the package. opt for malloc
// as poitner check is cleaner code that a lot of try blocks with std::bad_alloc

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
#define VECTOR_LENGTH_SMALL         0.000001
#define EVS_PYCMETHOD               METH_METHOD | METH_FASTCALL | METH_KEYWORDS

// Support using std::array as function args instead of raw pointers
typedef std::array<double, 3> varray_t;
typedef std::array<double, 9> marray_t;

// Need a forward declaration of the module definition. All NULL
// values to be initialized in the module initialization function.
static PyModuleDef EVSpace_Module = {
    PyModuleDef_HEAD_INIT,  // m_base
    "pyevspace",            // m_name
    NULL,                   // m_doc
    0,                      // m_size
    NULL,                   // m_methods
    NULL,                   // m_slots
    NULL,                   // m_traverse
    NULL,                   // m_clear
    NULL                    // m_free
};

typedef struct {
    PyTypeObject* Vector_Type;
    PyTypeObject* Matrix_Type;
    PyTypeObject* MatrixView_Type;
    PyTypeObject* EulerAngles_Type;
    PyTypeObject* RotationOrder_Type;
    PyTypeObject* ReferenceFrame_Type;
    PyEVSpace_CAPI* capsule;
} EVSpace_State;

// When it comes to module state functions, attempting to get the
// state from a type/object will result in an error if the type is
// not a subclass of a module type (except when compiled with
// CPython 3.10; see below). When not running CPython 3.10 it can
// be safe to assume an error in getting the module state with these
// functions means the type does not belong to the module, and is not
// a subclass of a module type, as the internally called functions
// will walk the MRO ladder to find the first type belonging to the
// module matching pyevspace's PyModuleDef definition. Success however,
// does not guarantee that the object/type is correct, as another
// invalid type of the module could be attempted to be used in an
// unsupported way (e.g. Vector.__new__(Matrix)). If the function whose
// parameters are being checked cannot guarantee the `self` or `type`
// values passed are at lease a subclass (i.e. internal functions not
// directly part of the Python layer, EVSpaceVector_New() for example),
// the parameter's type needs to be validated, usually with
// PyObject_IsInstance, PyObject_IsSubclass, or PyObject_TypeCheck.
//
// An error from the EVSpaceState_From* functions is likely not ideal
// to pass to the interpreter, as it wouldn't make sense to a user of
// the module. If the error really is a type mismatch, the function
// should clear the error state with PyErr_Clear() and set a well
// formed PyExc_TypeError to explain to the user where the error
// really comes from.
//
// In CPython 3.10, fetching the module state using the PyModDef is
// not supported, so the module must be looked up in sys.modules.
// This means using one of the EVSpaceState_From* functions should
// always succeed. This can be misleading to the above statement that
// an object type not apart of this module will cause the function
// to fail. Therefore, in CPython 3.10, any function getting the
// state using one of these functions that cannot guarantee the type
// of an argument (like `self`) must always directly check the
// type is a subclass of the expected type (or the exact type if
// the function requires it).

// Attempts to get the module of `obj` and retrieve the module state.
// Returns the module state on success or NULL on failure with an
// exception set. On CPython 3.10 this function gets the module via
// import and neither argument is used.s
static inline EVSpace_State*
EVSpaceState_FromObject(PyObject* obj)
{
    PyObject* module = NULL;
    EVSpace_State* state = NULL;

#if PY_VERSION_HEX >= 0x030b0000
    module = PyType_GetModuleByDef(Py_TYPE(obj), &EVSpace_Module);
    if (!module) {
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
#else
    PyObject* name = PyUnicode_FromString("pyevspace._pyevspace");
    if (!name) {
        return NULL;
    }

    module = PyImport_GetModule(name);
    Py_DECREF(name);

    if (!module) {
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
    Py_DECREF(module);
#endif

    if (!state && !PyErr_Occurred())
    {
        PyErr_SetString(PyExc_RuntimeError, "unable to get module state for pyevspace._pyevspace");
    }

    return state;
}

// Attempts to get the module of `left` first, and on failure the
// module on `right`. Returns the state from the module on success
// or NULL on failure with an exception set. On CPython 3.10 this
// function gets the module via import and neither argument is used.
static inline EVSpace_State*
EVSpaceState_FromObject2(PyObject* left, PyObject* right)
{
    PyObject* module = NULL;
    EVSpace_State* state = NULL;

#if PY_VERSION_HEX >= 0x030b0000
    module = PyType_GetModuleByDef(Py_TYPE(left), &EVSpace_Module);
    if (!module) {
        PyErr_Clear();
        module = PyType_GetModuleByDef(Py_TYPE(right), &EVSpace_Module);
    }
    if (!module) {
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
#else
    PyObject* name = PyUnicode_FromString("pyevspace._pyevspace");
    if (!name) {
        return NULL;
    }

    module = PyImport_GetModule(name);
    Py_DECREF(name);

    if (!module) {
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
    Py_DECREF(module);
#endif

    return state;
}

// Gets the module associated with `type` by walking the MRO ladder until
// the first super class of `type` in pyevspace is found. On success returns
// a pointer to the module state, on failure return NULL. On CPython 3.10 this
// function gets the module via import and the `type` argument is used (and may
// be set to NULL).
static inline EVSpace_State*
EVSpaceState_FromType(PyTypeObject* type)
{
    EVSpace_State* state = NULL;
    PyObject* module = NULL;

#if PY_VERSION_HEX >= 0x030b0000
    module = PyType_GetModuleByDef(type, &EVSpace_Module);

    if (!module) {
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
#else
    PyObject* name = PyUnicode_FromString("pyevspace._pyevspace");
    if (!name) {
        return NULL;
    }

    module = PyImport_GetModule(name);
    Py_DECREF(name);

    if (!module) {
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
    Py_DECREF(module);
#endif

    return state;
}

// Used by types supporting the buffer protocol. These objects
// share similar structure to memory allocated in the process
// of filling a Py_buffer object, so a single release function
// simplifies the buffer process.
static void
EVSpaceBuffer_Release(PyObject* Py_UNUSED(), Py_buffer* view)
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
// return NULL. `value` is only modified on success.
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

// Parse arg as an iterable and fill values with the contents. This
// only succeeds if there is exactly 3 elements in arg. On any error
// return 0 with an exception set, return 1 on success. The `values`
// array is not filled unless this function succeeds, and it must be
// able to hold at least 3 elements.
static int
EVSpaceIter_ParseObject(PyObject* arg, std::array<PyObject*, 3>& values)
{
    PyObject* tmp = NULL;
    std::vector<PyObject*> tmp_values;

    PyObject* iter = PyObject_GetIter(arg);
    if (!iter) {
        return 0;
    }

#if PY_VERSION_HEX >= 0x030e0000
    int result = 0;
    while ((result = PyIter_NextItem(iter, &tmp)) > 0)
    {
        tmp_values.push_back(tmp);
    }
    Py_DECREF(iter);

    if (result == -1) {
        goto error;
    }
#else
    while (tmp = PyIter_Next(iter))
    {
        tmp_values.push_back(tmp);
    }
    Py_DECREF(iter);

    if (PyErr_Occurred()) {
        goto error;
    }
#endif

    if (tmp_values.size() != 3)
    {
        PyErr_Format(PyExc_ValueError, "expected 3 items in iterable, found %d", tmp_values.size());
        goto error;
    }

    for (int i = 0; i < 3; i++) values[i] = tmp_values[i];
    return 1;

error:
    for (const auto vec : tmp_values) Py_DECREF(vec);
    return 0;
}

// Parse arg as an iterable and fill values with the contents. This
// only succeeds if there is exactly 3 elements in arg. On an error
// parsing the iterable return 0, on an error about the contents
// of the iterator (e.g. arg type, iterator length) return -1. On
// success return 1. The `values` array is not filled unless this
// function succeeds, and it must be able to hold at least 3 elements.
static int
EVSpaceIter_ParseDouble(PyObject* arg, varray_t& values)
{
    double tmp_double = 0;
    PyObject* tmp = NULL;
    std::vector<double> tmp_values;

    PyObject* iter = PyObject_GetIter(arg);
    if (!iter) {
        return 0;
    }

#if PY_VERSION_HEX >= 0x030e0000
    int result = 0;
    while ((result = PyIter_NextItem(iter, &tmp)) > 0)
    {
        if (!EVSpaceObject_AsDouble(tmp, tmp_double)) {
            Py_DECREF(iter);
            Py_DECREF(tmp);
            return -1;
        }
        Py_DECREF(tmp);
        tmp_values.push_back(tmp_double);
    }
    Py_DECREF(iter);

    if (result == -1) {
        return 0;
    }
#else
    while (tmp = PyIter_Next(iter))
    {
        if (!EVSpaceObject_AsDouble(tmp, tmp_double)) {
            Py_DECREF(iter);
            Py_DECREF(tmp);
            return -1;
        }
        Py_DECREF(tmp);
        tmp_values.push_back(tmp_double);
    }
    Py_DECREF(iter);

    if (PyErr_Occurred()) {
        return 0;
    }
#endif

    if (tmp_values.size() != 3)
    {
        PyErr_Format(PyExc_ValueError, "expected 3 items in iterable, found %d",
                     tmp_values.size());
        return -1;
    }

    for (int i = 0; i < 3; i++) values[i] = tmp_values[i];
    return 1;
}

// This function fits the `lenfunc` signature and always
// returns 3. All types of the module always have a length
// of 3 in sequence or mapping protocols and can therefore
// share the same length function.
static Py_ssize_t
EVSpaceObject_Length(PyObject* Py_UNUSED())
{
    return 3;
}

// Get the type name of `obj` as a const char* for printing or
// string formating.
static inline const char*
__EVSpace_GetTypeName(PyObject* obj)
{
    return Py_TYPE(obj)->tp_name;
}

//      EVSpace_Vector new functions
// These functions validate `type` is a subclass of EVSpace_Vector
// and allocate and initialize the underlying evspace::Vector object
// depending on the overloaded parameter types. The EVSpace_Vector.vector
// instance is allocated using `new` and should be freed with `delete`
// so the destructor is called.
//
// While the type supports subclassing, all methods attached to
// EVSpace_Vector strictly return EVSpace_Vector instances instead
// of subclassed types because any additional parameters to a subclasses
// __init__ methods are not known. Any subclass wanting to return an
// instance of that subclass should overload that function and use the
// returned values to initialize an instance of their type.

static EVSpace_Vector*
EVSpaceVector_New(PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state || !PyObject_IsSubclass((PyObject*)type, (PyObject*)state->Vector_Type))
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subclass of pyevspace.Vector",
                     type->tp_name);
        return NULL;
    }

    EXCEPTION_WRAPPER(
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }

        self->vector = new evspace::Vector();
        return self;
    )
}

[[maybe_unused]]
static EVSpace_Vector*
EVSpaceVector_New(const varray_t& array, PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state || !PyObject_IsSubclass((PyObject*)type, (PyObject*)state->Vector_Type))
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subclass of pyevspace.Vector", type->tp_name);
        return NULL;
    }

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
EVSpaceVector_New(const evspace::Vector& vector, PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state || !PyObject_IsSubclass((PyObject*)type, (PyObject*)state->Vector_Type))
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subclass of pyevspace.Vector", type->tp_name);
        return NULL;
    }

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
EVSpaceVector_New(evspace::Vector&& vector, PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state || !PyObject_IsSubclass((PyObject*)type, (PyObject*)state->Vector_Type))
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subclass of pyevspace.Vector", type->tp_name);
        return NULL;
    }

    EXCEPTION_WRAPPER(
        EVSpace_Vector* self = EVSpaceVector_Cast(type->tp_alloc(type, 0));
        if (!self) {
            return NULL;
        }

        self->vector = new evspace::Vector(std::move(vector));
        return self;
    )
}

// EVSpace_Vector slot functions

static PyObject*
Vector_new(PyTypeObject* type, PyObject* Py_UNUSED(1), PyObject* Py_UNUSED(2))
{
    EVSpace_Vector* self = EVSpaceVector_New(type);
    
    return EVS_PyObject_Cast(self);
}

static int
Vector_init(EVSpace_Vector* self, PyObject* args, PyObject* Py_UNUSED())
{
    //  Possible constructor signatures:
    //  Vector.__init__() -> None: ...
    //  Vector.__init__(iterable: Iterable) -> None: ...
    //  Vector.__init__(x: float, y: float, z: float) -> None: ...

    int result = 0;
    varray_t buffer{0.0};
    double* buffer_data = buffer.data();
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
        // Try parsing as a sequence
        if (PyArg_ParseTuple(args, "(ddd)", buffer_data,
                             buffer_data + 1, buffer_data + 2))
        {
            EVSpaceVector_X(self) = buffer[0];
            EVSpaceVector_Y(self) = buffer[1];
            EVSpaceVector_Z(self) = buffer[2];
            return 0;
        }

        // Try parsing as an iterable
        PyErr_Clear();

        if (!PyArg_ParseTuple(args, "O:pyevspace.Vector.__init__", &parameter)) {
            return -1;
        }

        result = EVSpaceIter_ParseDouble(PyTuple_GET_ITEM(args, 0), buffer);
        if (result == 1)
        {
            EVSpaceVector_X(self) = buffer[0];
            EVSpaceVector_Y(self) = buffer[1];
            EVSpaceVector_Z(self) = buffer[2];
            return 0;
        }
        else if (result == -1) {
            return -1;
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "expected a sequence or iterable");
            return -1;
        }
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

// Creates a const char* buffer containing the numeric values of the Vector
// class's str and repr methods. On failure an exception is set and NULL is
// returned. If the result is not NULL, it is up to the caller to free the
// memory by calling `free` on the returned pointer.
static inline const char*
_EVSpaceVector_String(const EVSpace_Vector* self)
{
    // CALLER MUST FREE BUFFER IF NOT NULL!!!!!!!!!!!!!

    char* buffer;
    const char* format = "%g, %g, %g";
    std::size_t buffer_size = snprintf(NULL, 0, format, EVSpaceVector_X(self),
        EVSpaceVector_Y(self), EVSpaceVector_Z(self));

    buffer = (char*)malloc(buffer_size + 1);
    if (!buffer) {
        PyErr_NoMemory();
        return NULL;
    }
    buffer[buffer_size] = '\0';

    if (sprintf(buffer, format, EVSpaceVector_X(self), EVSpaceVector_Y(self),
                    EVSpaceVector_Z(self)) < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to fill buffer on string operation");
        free((void*)buffer);
        return NULL;
    }

    return buffer;
}

static PyObject*
Vector_str(const EVSpace_Vector* self)
{
    PyObject* str = NULL;

    // We must free this when done!
    const char* buffer = _EVSpaceVector_String(self);
    if (!buffer) {
        return NULL;
    }

    str = PyUnicode_FromFormat("[%s]", buffer);
    free((void*)buffer);
    return str;
}

static PyObject*
Vector_repr(const EVSpace_Vector* self)
{
    const char* buffer = NULL;
    PyObject* repr = NULL;
    PyObject* type = (PyObject*)Py_TYPE(self);
    PyObject* module = PyObject_GetAttrString(type, "__module__");
    PyObject* qualname = PyObject_GetAttrString(type, "__qualname__");

    if (!module || !qualname) {
        Py_XDECREF(module);
        Py_XDECREF(qualname);
        return NULL;
    }

    // We must free this!
    buffer = _EVSpaceVector_String(self);
    if (!buffer)
    {
        Py_DECREF(module);
        Py_DECREF(qualname);
        return NULL;
    }

    repr = PyUnicode_FromFormat("%U.%U(%s)", module, qualname, buffer);
    Py_DECREF(module);
    Py_DECREF(qualname);
    free((void*)buffer);

    return repr;
}

static PyObject*
Vector_richcompare(PyObject* self, PyObject* other, int op)
{
    bool is_equal;
    EVSpace_Vector* lhs = NULL, *rhs = NULL;

    EVSpace_State* state = EVSpaceState_FromObject2(self, other);
    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }
    
    if (PyObject_TypeCheck(other, state->Vector_Type))
    {
        lhs = EVSpaceVector_Cast(self);
        rhs = EVSpaceVector_Cast(other);
        is_equal = EVSpaceVector_VECTOR(lhs) == EVSpaceVector_VECTOR(rhs);

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

//      EVSpace_Vector as number functions
// Supports __add__, __iadd__, __sub__, __isub__, __mul__, __rmul__,
// __imul__, __matmul__, __imatmul__, __truediv__, __itruediv__, and
// __neg__. Notably, __rmul__ is supported for scalar multiplication.
// Note for CPython 3.10 support, fetching state will always succeed,
// so explicit checks are needed on `self` for if the function returns
// Py_NOTIMPLEMENTED CPython will attempt the reflected version of the
// function. This check is needed to enforce the method is not implemented
// for unsupported types, where receiving a NULL state doesn't convey that.

static PyObject*
Vector_add(EVSpace_Vector* self, PyObject* other)
{
    EVSpace_Vector* rhs = NULL;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

#if PY_VERSION_HEX < 0x030b0000
    // state will always be pyevspace state, need to enforce `self` is
    // a subclass for __radd__ support
    if (!PyObject_TypeCheck((PyObject*)self, state->Vector_Type))
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.Vector.__radd__(): %s "
                     "is not a subclass of pyevspace.Vector",
                     __EVSpace_GetTypeName((PyObject*)self));
        return NULL;
    }
#endif

    if (PyObject_TypeCheck(other, state->Vector_Type))
    {
        rhs = EVSpaceVector_Cast(other);
        EXCEPTION_WRAPPER(
            evspace::Vector result = EVSpaceVector_VECTOR(self) + EVSpaceVector_VECTOR(rhs);
            
            return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result),
                                                       state->Vector_Type));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_subtract(EVSpace_Vector* self, PyObject* other)
{
    EVSpace_Vector* rhs = NULL;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state){
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

#if PY_VERSION_HEX < 0x030b0000
    // state will always be pyevspace state, need to enforce `self` is
    // a subclass for __rsub__ support
    if (!PyObject_TypeCheck((PyObject*)self, state->Vector_Type))
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.Vector.__rsub__(): %s "
                     "is not a subclass of pyevspace.Vector",
                     __EVSpace_GetTypeName((PyObject*)self));
        return NULL;
    }
#endif

    if (PyObject_TypeCheck(other, state->Vector_Type))
    {
        rhs = EVSpaceVector_Cast(other);
        EXCEPTION_WRAPPER(
            evspace::Vector result = EVSpaceVector_VECTOR(self) - EVSpaceVector_VECTOR(rhs);

            return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result),
                                                       state->Vector_Type));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_multiply(PyObject* lhs, PyObject* rhs)
{
    // Possible signatures:
    // Vector.__mul__(x: double) -> Vector: ...
    // float.__mul__(v: Vector) -> Vector: ...

    EVSpace_Vector* self = NULL;
    PyObject* scalar_result = NULL;
    double scalar = 0.0;

    EVSpace_State* state = EVSpaceState_FromObject2(lhs, rhs);
    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    // __mul__
    if (PyObject_TypeCheck(lhs, state->Vector_Type))
    {
        self = EVSpaceVector_Cast(lhs);
        scalar_result = EVSpaceObject_AsDouble(rhs, scalar);
    }
    // __rmul__
    else if (PyObject_TypeCheck(rhs, state->Vector_Type))
    {
        self = EVSpaceVector_Cast(rhs);
        scalar_result = EVSpaceObject_AsDouble(lhs, scalar);
    }
    else {
        Py_RETURN_NOTIMPLEMENTED;
    }

    if (!scalar_result)
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }
        else {
            return NULL;
        }
    }

    EXCEPTION_WRAPPER(
        evspace::Vector result = EVSpaceVector_VECTOR(self) * scalar;
        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result),
                                                   state->Vector_Type));
    )    
}

static PyObject*
Vector_matrix_multiply(EVSpace_Vector* self, PyObject* other)
{
    EVSpace_Matrix* matrix = NULL;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

#if PY_VERSION_HEX < 0x030b0000
    // state will always be pyevspace state, need to enforce `self` is
    // a subclass for __rmatmul__ support
    if (!PyObject_TypeCheck((PyObject*)self, state->Vector_Type))
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.Vector.__rmatmul__(): %s "
                     "is not a subclass of pyevspace.Vector",
                     __EVSpace_GetTypeName((PyObject*)self));
        return NULL;
    }
#endif

    if (PyObject_TypeCheck(other, state->Matrix_Type))
    {
        matrix = EVSpaceMatrix_Cast(other);
        EXCEPTION_WRAPPER(
            evspace::Vector result = EVSpaceVector_VECTOR(self) *
                                     EVSpaceMatrix_MATRIX(matrix);
            return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result),
                                                       state->Vector_Type));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_divide(EVSpace_Vector* self, PyObject* other)
{
    double scalar = 0;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    // need to safe guard against __rtruediv__ passing non Vector type as `self`
    if (PyObject_TypeCheck(self, state->Vector_Type))
    {
        if (!EVSpaceObject_AsDouble(other, scalar))
        {
            if (PyErr_ExceptionMatches(PyExc_TypeError))
            {
                PyErr_Clear();
                Py_RETURN_NOTIMPLEMENTED;
            }
            else {
                return NULL;
            }
        }

        EXCEPTION_WRAPPER(
            evspace::Vector answer = EVSpaceVector_VECTOR(self) / scalar;
            return EVS_PyObject_Cast(EVSpaceVector_New(std::move(answer),
                                                       state->Vector_Type));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_add(EVSpace_Vector* self, PyObject* other)
{
    EVSpace_Vector* rhs = NULL;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    rhs = EVSpaceVector_Cast(other);
    if (PyObject_TypeCheck(rhs, state->Vector_Type))
    {
        EVSpaceVector_VECTOR(self) += EVSpaceVector_VECTOR(rhs);
        
        return Py_NewRef(self);
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_subtract(EVSpace_Vector* self, PyObject* other)
{
    EVSpace_Vector* rhs = NULL;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    rhs = EVSpaceVector_Cast(other);
    if (PyObject_TypeCheck(rhs, state->Vector_Type))
    {
        EVSpaceVector_VECTOR(self) -= EVSpaceVector_VECTOR(rhs);
        
        return Py_NewRef(self);
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_multiply(EVSpace_Vector* self, PyObject* other)
{
    double scalar = 0;
    
    if (!EVSpaceObject_AsDouble(other, scalar))
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }

        return NULL;
    }

    EVSpaceVector_VECTOR(self) *= scalar;
    return Py_NewRef(self);
}

static PyObject*
Vector_inplace_multiply_matrix(EVSpace_Vector* self, PyObject* other)
{
    EVSpace_Matrix* rhs;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    if (PyObject_TypeCheck(other, state->Matrix_Type))
    {
        rhs = EVSpaceMatrix_Cast(other);
        EVSpaceVector_VECTOR(self) *= EVSpaceMatrix_MATRIX(rhs);

        return Py_NewRef(self);
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Vector_inplace_divide(EVSpace_Vector* self, PyObject* other)
{
    double scalar;

    if (!EVSpaceObject_AsDouble(other, scalar))
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }

        return NULL;
    }

    EVSpaceVector_VECTOR(self) /= scalar;

    return Py_NewRef(self);
}

static PyObject*
Vector_negative(EVSpace_Vector* self)
{
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    EXCEPTION_WRAPPER(
        evspace::Vector result = -EVSpaceVector_VECTOR(self);

        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result),
                                                   state->Vector_Type));
    )
}

/* Vector sequence functions */

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

    view->obj           = Py_NewRef(EVS_PyObject_Cast(obj));
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
        free(shape);
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

//      Vector instance methods
// Methods that require the module state must be defined as `CPyMethod`
// style signatures. This will pass the `defining_class` argument which
// enables the EVSpaceState_From* functions to properly get the module
// state to verify arguments and instantiate new objects.

static PyObject*
Vector_magnitude(EVSpace_Vector* self, PyObject* Py_UNUSED())
{
    return PyFloat_FromDouble(EVSpaceVector_VECTOR(self).magnitude());
}

static PyObject*
Vector_magnitude_square(EVSpace_Vector* self, PyObject* Py_UNUSED())
{
    return PyFloat_FromDouble(EVSpaceVector_VECTOR(self).magnitude_squared());
}

static PyObject*
Vector_normalize(EVSpace_Vector* self, PyObject* Py_UNUSED())
{
    EVSpaceVector_VECTOR(self).normalize();

    Py_RETURN_NONE;
}

static PyObject*
Vector_norm(EVSpace_Vector* self, PyTypeObject* defining_class,
            PyObject* const *Py_UNUSED(), Py_ssize_t nargs,
            PyObject* kwargs)
{
    if (nargs != 0 || kwargs != NULL)
    {
        PyErr_SetString(PyExc_TypeError, "Vector.norm() takes no arguments");
        return NULL;
    }

    EVSpace_State* state = (EVSpace_State*)PyType_GetModuleState(defining_class);
    if (!state) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Vector norm = EVSpaceVector_VECTOR(self).norm();
        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(norm),
                                                   state->Vector_Type));
    )
}

static PyObject*
Vector_reduce(EVSpace_Vector* self, PyTypeObject* defining_class,
              PyObject* const *Py_UNUSED(), Py_ssize_t nargs,
              PyObject* kwargs)
{
    EVSpace_State* state = NULL;

    if (nargs != 0 || kwargs != NULL)
    {
        PyErr_SetString(PyExc_TypeError, "pyevpace.Vector.__reduce__() "
                        "expects no arguments");
        return NULL;
    }

    state = (EVSpace_State*)PyType_GetModuleState(defining_class);
    if (!state) {
        return NULL;
    }

    return Py_BuildValue("(O(ddd))", state->Vector_Type, EVSpaceVector_X(self),
                         EVSpaceVector_Y(self), EVSpaceVector_Z(self));
}

static PyObject*
Vector_compare_to_tol(EVSpace_Vector* self, PyObject* args)
{
    double abs_tol = evspace::DEFAULT_ABS_TOL, rel_tol = evspace::DEFAULT_REL_TOL;
    EVSpace_Vector* rhs;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state) {
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "O!|dd:compare_to_tol", state->Vector_Type,
                          &rhs, &rel_tol, &abs_tol)) {
        return NULL;
    }

    if (rel_tol < 0) {
        PyErr_SetString(PyExc_ValueError, "pyevspace.Vector.compare_to_tol(): "
                        "rel_tol must be non-negative");
        return NULL;
    }
    if (abs_tol < 0) {
        PyErr_SetString(PyExc_ValueError, "pyevspace.Vector.compare_to_tol(): "
                        "abs_tol must be non-negative");
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
    int max_ulps = 0;
    EVSpace_Vector* rhs = NULL;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state) {
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "O!i:compare_to_ulp", state->Vector_Type,
                          &rhs, &max_ulps)) {
        return NULL;
    }

    if (max_ulps < 0) {
        PyErr_Format(PyExc_ValueError, "pyevspace.Vector.compare_to_ulp(): "
                     "max_ulps must be non-negative (got %i)", max_ulps);
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
Vector_dot(PyObject* module, PyObject* const* args, Py_ssize_t size)
{
    double dot_product = 0;
    EVSpace_State* state = NULL;
    EVSpace_Vector* lhs = NULL, *rhs = NULL;

    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.vector_dot() expected "
                     "exactly 2 arguments (%i given)", size);
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
    if (!state) {
        return NULL;
    }

    if (!PyObject_TypeCheck(args[0], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_dot(): first "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!PyObject_TypeCheck(args[1], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_dot(): second "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }

    lhs = EVSpaceVector_Cast(args[0]);
    rhs = EVSpaceVector_Cast(args[1]);
    dot_product = evspace::vector_dot(
        EVSpaceVector_VECTOR(lhs),
        EVSpaceVector_VECTOR(rhs)
    );

    return PyFloat_FromDouble(dot_product);
}

static PyObject*
Vector_cross(PyObject* module, PyObject* const* args, Py_ssize_t size)
{
    EVSpace_State* state = NULL;
    EVSpace_Vector* lhs = NULL, *rhs = NULL;

    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.vector_cross() expected "
                     "exactly 2 arguments (%i given)", size);
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
    if (!state) {
        return NULL;
    }

    if (!PyObject_TypeCheck(args[0], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_cross(): first "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!PyObject_TypeCheck(args[1], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_cross(): second "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }

    lhs = EVSpaceVector_Cast(args[0]);
    rhs = EVSpaceVector_Cast(args[1]);

    EXCEPTION_WRAPPER(
        evspace::Vector cross_product = evspace::vector_cross(
            EVSpaceVector_VECTOR(lhs), EVSpaceVector_VECTOR(rhs)
        );
        
        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(cross_product),
                                                   state->Vector_Type));
    )
}

static PyObject*
Vector_angle(PyObject* module, PyObject* const* args, Py_ssize_t size)
{
    double angle = 0;
    EVSpace_State* state = NULL;
    EVSpace_Vector* lhs = NULL, *rhs = NULL;

    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.vector_angle() expected "
                     "exactly 1 argument (%i given)", size);
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
    if (!state) {
        return NULL;
    }

    if (!PyObject_TypeCheck(args[0], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_angle(): first "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!PyObject_TypeCheck(args[1], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_angle(): second "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        lhs = EVSpaceVector_Cast(args[0]);
        rhs = EVSpaceVector_Cast(args[1]);
        angle = evspace::vector_angle(
            EVSpaceVector_VECTOR(lhs), EVSpaceVector_VECTOR(rhs)
        );

        return PyFloat_FromDouble(angle);
    )
}

static PyObject*
Vector_exclude(PyObject* module, PyObject* const* args, Py_ssize_t size)
{
    EVSpace_State* state = NULL;
    EVSpace_Vector* lhs = NULL, *rhs = NULL;

    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.vector_exclude() expected "
                     "exactly 1 argument (%i given)", size);
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
    if (!state) {
        return NULL;
    }

    if (!PyObject_TypeCheck(args[0], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_exclude(): first "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!PyObject_TypeCheck(args[1], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_exclude(): second "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        lhs = EVSpaceVector_Cast(args[0]);
        rhs = EVSpaceVector_Cast(args[1]);
        evspace::Vector exclude = evspace::vector_exclude(
            EVSpaceVector_VECTOR(lhs), EVSpaceVector_VECTOR(rhs)
        );

        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(exclude),
                                                   state->Vector_Type));
    )
}

static PyObject*
Vector_projection(PyObject* module, PyObject* const* args, Py_ssize_t size)
{
    EVSpace_State* state = NULL;
    EVSpace_Vector* lhs = NULL, *rhs = NULL;

    if (size != 2)
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.vector_proj() expected "
                     "exactly 2 arguments (%i given)", size);
        return NULL;
    }

    state = (EVSpace_State*)PyModule_GetState(module);
    if (!state) {
        return NULL;
    }

    if (!PyObject_TypeCheck(args[0], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_proj(): first "
                        "argument must be pyevspace.Vector type");
        return NULL;
    }
    else if (!PyObject_TypeCheck(args[1], state->Vector_Type))
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.vector_proj(): second "
                        "argument must by pyevspace.Vector type");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        lhs = EVSpaceVector_Cast(args[0]);
        rhs = EVSpaceVector_Cast(args[1]);
        evspace::Vector projection = evspace::vector_projection(
            EVSpaceVector_VECTOR(lhs), EVSpaceVector_VECTOR(rhs)
        );

        return EVS_PyObject_Cast(EVSpaceVector_New(std::move(projection),
                                                   state->Vector_Type));
    )
}

PyDoc_STRVAR(vector_doc, "Vector([{x, y, z | iterable}])\n\
\n\
The Vector can be constructed with an iterable of length 3, or\n\
directly with the x, y, and z components. All components must be\n\
numeric types. Alternatively the components will default to zero\n\
if no argument is specified.");

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

    {"norm", (PyCFunction)(void(*)(void))Vector_norm, EVS_PYCMETHOD, vector_norm_doc},

    {"__reduce__", (PyCFunction)(void(*)(void))Vector_reduce, EVS_PYCMETHOD, vector_reduce_doc},

    {"compare_to_tol", (PyCFunction)Vector_compare_to_tol, METH_VARARGS, vector_compare_to_tol_doc},

    {"compare_to_ulp", (PyCFunction)Vector_compare_to_ulp, METH_VARARGS, vector_compare_to_ulp_doc},

    {NULL, NULL, 0, NULL}
};

static PyType_Slot vector_slots[] = {
    {Py_tp_dealloc,                 (void*)Vector_dealloc},
    {Py_tp_repr,                    (void*)Vector_repr},
    {Py_nb_add,                     (void*)Vector_add},
    {Py_nb_subtract,                (void*)Vector_subtract},
    {Py_nb_multiply,                (void*)Vector_multiply},
    {Py_nb_negative,                (void*)Vector_negative},
    {Py_nb_inplace_add,             (void*)Vector_inplace_add},
    {Py_nb_inplace_subtract,        (void*)Vector_inplace_subtract},
    {Py_nb_inplace_multiply,        (void*)Vector_inplace_multiply},
    {Py_nb_true_divide,             (void*)Vector_divide},
    {Py_nb_inplace_true_divide,     (void*)Vector_inplace_divide},
    {Py_nb_matrix_multiply,         (void*)Vector_matrix_multiply},
    {Py_nb_inplace_matrix_multiply, (void*)Vector_inplace_multiply_matrix},
    {Py_sq_length,                  (void*)EVSpaceObject_Length},
    {Py_sq_item,                    (void*)Vector_get_item},
    {Py_sq_ass_item,                (void*)Vector_set_item},
    {Py_tp_str,                     (void*)Vector_str},
    {Py_bf_getbuffer,               (void*)Vector_get_buffer},
    {Py_bf_releasebuffer,           (void*)EVSpaceBuffer_Release},
    {Py_tp_doc,                     (void*)vector_doc},
    {Py_tp_richcompare,             (void*)Vector_richcompare},
    {Py_tp_methods,                 (void*)vector_methods},
    {Py_tp_init,                    (void*)Vector_init},
    {Py_tp_new,                     (void*)Vector_new},
    {0,                             NULL}
};

static PyType_Spec vector_spec = {
    "pyevspace.Vector",     /* name */
    sizeof(EVSpace_Vector), /* basicsize */
    0,                      /* itemsize */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE | Py_TPFLAGS_BASETYPE, /* flags */
    vector_slots            /* slots */
};

/* EVSpace_MatrixView */
//      EVSpace_MatrixView
// This acts simply as an exporter to an internal view of the EVSpace_Matrix
// type. This type shouldn't be directly exposed to Python via importing
// the module, however it will be exposed as the `obj` attribute of the
// `memoryview` object returned from the Matrix `mp_subscript` method.
//
// While the type supports subclassing, all methods attached to
// EVSpace_Matrix strictly return EVSpace_Matrix instances instead
// of subclassed types because any additional parameters to a subclasses
// __init__ methods are not known. Any subclass wanting to return an
// instance of that subclass should overload that function and use the
// returned values to initialize an instance of their type.

static EVSpace_MatrixView*
EVSpaceMatrixView_New(EVSpace_Matrix* base, PyTypeObject* type,
                      std::size_t ndim, std::size_t offset,
                      Py_ssize_t shape0, Py_ssize_t shape1,
                      Py_ssize_t strides0, Py_ssize_t strides1)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state)
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subclass of pyevspace._MatrixView",
                     type->tp_name);
        return NULL;
    }

    if (!PyObject_IsSubclass((PyObject*)type, (PyObject*)state->MatrixView_Type))
    {
        PyErr_Format(PyExc_TypeError, "%s is not a subclass of pyevspace._MatrixView",
                     type->tp_name);
        return NULL;
    }

    EVSpace_MatrixView* view = reinterpret_cast<EVSpace_MatrixView*>(type->tp_alloc(type, 0));

    if (!view) {
        return NULL;
    }

    view->base = base;
    view->data = EVSpaceMatrix_MATRIX(base).data().data() + offset;
    view->shape = reinterpret_cast<Py_ssize_t*>(malloc(ndim * sizeof(Py_ssize_t)));
    if (!view->shape) {
        Py_TYPE(view)->tp_dealloc(EVS_PyObject_Cast(view));
        PyErr_NoMemory();
        return NULL;
    }
    view->strides = reinterpret_cast<Py_ssize_t*>(malloc(ndim * sizeof(Py_ssize_t)));
    if (!view->strides) {
        Py_TYPE(view)->tp_dealloc(EVS_PyObject_Cast(view));
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
    Py_TYPE(self)->tp_free((EVS_PyObject_Cast(self)));
}

static int
MatrixView_GetBuffer(EVSpace_MatrixView* obj, Py_buffer* view, int flags)
{
    view->obj           = Py_NewRef(obj);
    view->buf           = obj->data;
    view->readonly      = 0;
    view->itemsize      = sizeof(double);
    view->shape         = (flags & PyBUF_ND) ? obj->shape : NULL;
    view->strides       = (flags & PyBUF_STRIDES) ? obj->strides : NULL;
    view->format        = (flags & PyBUF_FORMAT) ? (char*)"d" : NULL;
    view->suboffsets    = NULL;
    view->ndim          = obj->ndim;

    view->len           = sizeof(double) * obj->shape[0];
    if (obj->ndim > 1) {
        view->len *= obj->shape[1];
    }

    return 0;
}

static PyType_Slot matrixview_slots[] = {
    {Py_tp_dealloc,     (void*)MatrixView_dealloc},
    {Py_bf_getbuffer,   (void*)MatrixView_GetBuffer},
    {0,                 NULL}
};

static PyType_Spec matrixview_spec = {
    "pyevspace._MatrixView",    /* name */
    sizeof(EVSpace_MatrixView), /* basicsize */
    0,                          /* itemsize */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION, /* flags */
    matrixview_slots            /* slots */
};

//      EVSpace_Matrix new functions
// These functions validate `type` is a subclass of EVSpace_Matrix
// and allocate and initialize the underlying evspace::Matrix object
// depending on the overloaded parameter types. The EVSpace_Matrix.matrix
// instance is allocated using `new` and should be freed with `delete`
// so the destructor is called.

[[maybe_unused]]
static EVSpace_Matrix*
EVSpaceMatrix_New(PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state || !PyObject_IsSubclass((PyObject*)type, (PyObject*)state->Matrix_Type))
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.Matrix",
                     type->tp_name);
        return NULL;
    }

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
EVSpaceMatrix_New(const marray_t& array, PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state || !PyObject_IsSubclass((PyObject*)type, (PyObject*)state->Matrix_Type))
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.Matrix",
                     type->tp_name);
        return NULL;
    }

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
EVSpaceMatrix_New(const evspace::Matrix& matrix, PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state || !PyObject_IsSubclass((PyObject*)type, (PyObject*)state->Matrix_Type))
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.Matrix",
                     type->tp_name);
        return NULL;
    }

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
EVSpaceMatrix_New(evspace::Matrix&& matrix, PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state || !PyObject_IsSubclass((PyObject*)type, (PyObject*)state->Matrix_Type))
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.Matrix", type->tp_name);
        return NULL;
    }

    EVSpace_Matrix* self = EVSpaceMatrix_Cast(type->tp_alloc(type, 0));
    if (!self) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        self->matrix = new evspace::Matrix(std::move(matrix));
        return self;
    )
}

static PyObject*
Matrix_new(PyTypeObject* type, PyObject* Py_UNUSED(1), PyObject* Py_UNUSED(2))
{
    marray_t array{0.0};
    EVSpace_Matrix* self = EVSpaceMatrix_New(array, type);

    return EVS_PyObject_Cast(self);
}

static int
Matrix_init(EVSpace_Matrix* self, PyObject* args, PyObject* Py_UNUSED())
{
    // Possible signatures:
    // Matrix.__init__()
    // Matrix.__init__(row1: Iterable, row2: Iterable, row3: Iterable)
    // Matrix.__init__(array: Iterable, Iterable, Iterable)

    int result = 0;
    Py_ssize_t tuple_size = PyTuple_Size(args);
    // double* data = EVSpaceMatrix_MATRIX(self).data().data();
    PyObject* container = NULL;
    std::array<PyObject*, 3> items{NULL};
    std::array<varray_t, 3> values{0.0};

    if (tuple_size == -1) {
        return -1;
    }
    else if (tuple_size == 0) {
        // self is already initialized to 0's
        return 0;
    }
    else if (tuple_size != 3 && tuple_size != 1) {
        PyErr_Format(PyExc_ValueError, "Matrix.__init__() expected 0, 1, or 3 argument, not %i", tuple_size);
        return -1;
    }

    if (tuple_size == 1) {
        // strip the outer container from argument
        container = PyTuple_GET_ITEM(args, 0);
    }
    else {
        container = args;
    }

    if (EVSpaceIter_ParseObject(container, items) < 1)
    {
        return -1;
    }

    for (int i = 0; i < 3; i++)
    {
        result = EVSpaceIter_ParseDouble(items[i], values[i]);
        if (result == 1) {
            continue;
        }
        else if (result == 0) {
            PyErr_Clear();
            PyErr_Format(PyExc_TypeError, "Matrix.__init__() expected row parameter "
                            "%d to be sequence or iterable type", i);
        }
        Py_DECREF(items[0]);
        Py_DECREF(items[1]);
        Py_DECREF(items[2]);
        return -1;
    }

    Py_DECREF(items[0]);
    Py_DECREF(items[1]);
    Py_DECREF(items[2]);

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            EVSpaceMatrix_MATRIX(self)(i, j) = values[i][j];
        }
    }

    return 0;
}

static void
Matrix_dealloc(EVSpace_Matrix* self)
{
    delete self->matrix;
    Py_TYPE(self)->tp_free(EVS_PyObject_Cast(self));
}

// Creates and fills a string buffer for the inner data of a Matrix based
// on `format`. On failure set an exception and return NULL. If non-NULL,
// the caller must free the buffer by calling `free()`.
static inline const char*
_EVSpaceMatrix_String(const EVSpace_Matrix* matrix, const char* format)
{
    char* buffer = NULL;
    double* data = EVSpaceMatrix_MATRIX(matrix).data().data();
    const size_t buffer_size = snprintf(NULL, 0, format, data[0], data[1],
                                        data[2], data[3], data[4], data[5],
                                        data[6], data[7], data[8]);

    buffer = (char*)malloc(buffer_size + 1);
    if (!buffer) {
        PyErr_NoMemory();
        return NULL;
    }
    buffer[buffer_size] = '\0';

    if (sprintf(buffer, format, data[0], data[1], data[2], data[3], data[4],
                data[5], data[6], data[7], data[8]) < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "pyevspace: error filling Matrix string buffer");
        free((void*)buffer);
        return NULL;
    }

    return buffer;
}

static PyObject*
Matrix_str(const EVSpace_Matrix* self)
{
    PyObject* str = NULL;
    // We must free this when done!
    const char* buffer = _EVSpaceMatrix_String(
        self, "[[%g, %g, %g]\n [%g, %g, %g]\n [%g, %g, %g]]"
    );
    if (!buffer) {
        return NULL;
    }

    str = PyUnicode_FromString(buffer);
    free((void*)buffer);

    return str;
}

static PyObject*
Matrix_repr(const EVSpace_Matrix* self)
{
    const char* buffer = NULL;
    PyObject* repr = NULL;
    PyObject* type = (PyObject*)Py_TYPE(self);
    PyObject* module = PyObject_GetAttrString(type, "__module__");
    PyObject* qualname = PyObject_GetAttrString(type, "__qualname__");

    if (!module || !qualname)
    {
        Py_XDECREF(module);
        Py_XDECREF(qualname);
        return NULL;
    }

    // We must free this when done!
    buffer = _EVSpaceMatrix_String(self, "(%g, %g, %g), (%g, %g, %g), (%g, %g, %g)");
    if (!buffer)
    {
        Py_DECREF(module);
        Py_DECREF(qualname);
        return NULL;
    }

    repr = PyUnicode_FromFormat("%U.%U(%s)", module, qualname, buffer);
    Py_DECREF(module);
    Py_DECREF(qualname);
    free((void*)buffer);

    return repr;
}

static PyObject*
Matrix_richcompare(EVSpace_Matrix* self, PyObject* other, int op)
{
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    if (PyObject_TypeCheck(other, state->Matrix_Type))
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

//      EVSpace_Matrix as number functions
// Supports __add__, __iadd__, __sub__, __isub__, __mul__, __rmul__,
// __imul__, __matmul__, __imatmul__, __truediv__, __itruediv__, and
// __neg__. Notable, __rmul__ is supported for scalar multiplication.
// __imatmul__ must explicitly raise a PyExc_TypeError when the argument
// is an EVSpace_Vector type since the numeric operator mechanism will
// delegate to EVSpace_Vector @ EVSpace_Matrix, which will change the
// type of the left-hand operator, something we don't want to support.
// Note for CPython 3.10 support, fetching the state will always succeed,
// so explicity checks are needed on `self` for if the function returns
// Py_NOTIMPLEMENTED CPython will attempt the reflected version of the
// operation. This check is needed to enforce the method is not implemented
// for unsupported types, where receiving a NULL state doesn't convey that.

static PyObject*
Matrix_add(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_Matrix* rhs = NULL;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

#if PY_VERSION_HEX < 0x030b0000
    // state will always be pyevspace state, need to enforce `self` is
    // a subclass for __radd__ support
    if (!PyObject_TypeCheck((PyObject*)self, state->Matrix_Type))
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.Matrix.__radd__(): %s "
                     "is not a subclass of pyevspace.Matrix",
                     __EVSpace_GetTypeName((PyObject*)self));
        return NULL;
    }
#endif

    if (PyObject_TypeCheck(other, state->Matrix_Type))
    {
        rhs = EVSpaceMatrix_Cast(other);
        EXCEPTION_WRAPPER(
            evspace::Matrix result = EVSpaceMatrix_MATRIX(self) + EVSpaceMatrix_MATRIX(rhs);
            return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result),
                                                       state->Matrix_Type));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_subtract(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_Matrix* rhs = NULL;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state)
    {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }
    
#if PY_VERSION_HEX < 0x030b0000
    // state will always be pyevspace state, need to enforce `self` is
    // a subclass for __rsub__ support
    if (!PyObject_TypeCheck((PyObject*)self, state->Matrix_Type))
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.Matrix.__rsub__(): %s "
                     "is not a subclass of pyevspace.Matrix",
                     __EVSpace_GetTypeName((PyObject*)self));
        return NULL;
    }
#endif

    if (PyObject_TypeCheck(other, state->Matrix_Type))
    {
        rhs = EVSpaceMatrix_Cast(other);
        EXCEPTION_WRAPPER(
            evspace::Matrix result = EVSpaceMatrix_MATRIX(self) - EVSpaceMatrix_MATRIX(rhs);
            return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result),
                                                       state->Matrix_Type));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_multiply(PyObject* lhs, PyObject* rhs)
{
    double scalar = 0;
    PyObject *scalar_result = NULL;
    EVSpace_Matrix* self = NULL;
    EVSpace_State* state = EVSpaceState_FromObject2(lhs, rhs);

    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    // __mul__
    if (PyObject_TypeCheck(lhs, state->Matrix_Type))
    {
        self = EVSpaceMatrix_Cast(lhs);
        scalar_result = EVSpaceObject_AsDouble(rhs, scalar);
    }
    // __rmul__
    else
    {
        self = EVSpaceMatrix_Cast(rhs);
        scalar_result = EVSpaceObject_AsDouble(lhs, scalar);
    }

    if (!scalar_result)
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }

        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Matrix result = EVSpaceMatrix_MATRIX(self) * scalar;
        return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result),
                                                   state->Matrix_Type));
    )
}

static PyObject*
Matrix_matrix_multiply(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }
    
#if PY_VERSION_HEX < 0x030b0000
    // state will always be pyevspace state, need to enforce `self` is
    // a subclass for __rmatmul__ support
    if (!PyObject_TypeCheck((PyObject*)self, state->Matrix_Type))
    {
        PyErr_Format(PyExc_TypeError, "pyevspace.Matrix.__rmatmul__(): %s "
                     "is not a subclass of pyevspace.Matrix",
                     __EVSpace_GetTypeName((PyObject*)self));
        return NULL;
    }
#endif

    if (PyObject_TypeCheck(other, state->Matrix_Type))
    {
        EXCEPTION_WRAPPER(
            evspace::Matrix result = EVSpaceMatrix_MATRIX(self) *
                                     EVSpaceMatrix_MATRIX(EVSpaceMatrix_Cast(other));
            return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result),
                                                       state->Matrix_Type));
        )
    }
    else if (PyObject_TypeCheck(other, state->Vector_Type))
    {
        EXCEPTION_WRAPPER(
            evspace::Vector result = EVSpaceMatrix_MATRIX(self) *
                                     EVSpaceVector_VECTOR(EVSpaceVector_Cast(other));
            return EVS_PyObject_Cast(EVSpaceVector_New(std::move(result),
                                                       state->Vector_Type));
        )
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_divide(EVSpace_Matrix* self, PyObject* other)
{
    double scalar;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    if (!EVSpaceObject_AsDouble(other, scalar))
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }

        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Matrix result = EVSpaceMatrix_MATRIX(self) / scalar;
        return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result),
                                                   state->Matrix_Type));
    )
}

static PyObject*
Matrix_inplace_add(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_Matrix* rhs;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    if (PyObject_TypeCheck(other, state->Matrix_Type))
    {
        rhs = EVSpaceMatrix_Cast(other);
        EVSpaceMatrix_MATRIX(self) += EVSpaceMatrix_MATRIX(rhs);

        return Py_NewRef(self);
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_inplace_subtract(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_Matrix* rhs;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    if (PyObject_TypeCheck(other, state->Matrix_Type))
    {
        rhs = EVSpaceMatrix_Cast(other);
        EVSpaceMatrix_MATRIX(self) -= EVSpaceMatrix_MATRIX(rhs);

        return Py_NewRef(self);
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_inplace_multiply(EVSpace_Matrix* self, PyObject* other)
{
    double scalar = 0;

    if (!EVSpaceObject_AsDouble(other, scalar))
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }

        return NULL;
    }

    EVSpaceMatrix_MATRIX(self) *= scalar;

    return Py_NewRef(self);
}

static PyObject*
Matrix_inplace_multiply_matrix(EVSpace_Matrix* self, PyObject* other)
{
    EVSpace_Matrix* rhs = NULL;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    if (PyObject_TypeCheck(other, state->Matrix_Type))
    {
        rhs = EVSpaceMatrix_Cast(other);
        EVSpaceMatrix_MATRIX(self) *= EVSpaceMatrix_MATRIX(rhs);

        return Py_NewRef(self);
    }
    else if (PyObject_TypeCheck(other, state->Vector_Type))
    {
        // CPython will fall back to self = self * other when NotImplemented
        // is returned, so we must force the TypeError here.
        PyErr_Format(PyExc_TypeError, "unsupported operand type(s) for @=: '%s' and '%s'",
                     __EVSpace_GetTypeName((PyObject*)self), state->Vector_Type->tp_name);
        return NULL;
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject*
Matrix_inplace_divide(EVSpace_Matrix* self, PyObject* other)
{
    double scalar;

    if (!EVSpaceObject_AsDouble(other, scalar))
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
        {
            PyErr_Occurred();
            Py_RETURN_NOTIMPLEMENTED;
        }

        return NULL;
    }

    EVSpaceMatrix_MATRIX(self) /= scalar;

    return Py_NewRef(self);
}

static PyObject*
Matrix_negative(EVSpace_Matrix* self)
{
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        PyErr_Clear();
        Py_RETURN_NOTIMPLEMENTED;
    }

    EXCEPTION_WRAPPER(
        evspace::Matrix result = -EVSpaceMatrix_MATRIX(self);

        return EVS_PyObject_Cast(EVSpaceMatrix_New(result, state->Matrix_Type));
    )
}

/* Mapping and buffer methods */

// Converts `obj` to an index value using __index__() if necessary. If an exception
// that should be propagated (any exception other than PyExc_TypeError) is encountered
// return -1. If a PyExc_TypeError is encountered return 0 with the exception state
// cleared, otherwise fill `index` with the converted value and return 1.
static int
_EVSpace_AsIndex(PyObject* obj, Py_ssize_t* index)
{
    Py_ssize_t tmp_index = 0, stop = 0;
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
    Py_ssize_t index0{}, index1{};
    PyObject* key0 = NULL, *key1 = NULL;
    Py_ssize_t start0{}, stop0{}, step0{}, slicelength0{}, start1{}, stop1{},
        step1{}, slicelength1{};
    EVSpace_MatrixView* matrix_view = NULL;
    PyTypeObject* type = NULL;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state) {
        return NULL;
    }
    type = state->MatrixView_Type;

    // fixme: I think I way overthought this... PyArg_ParseTuple() might default int types
    // to call __index__ which may be able to simplify the logic here

    // single arg that supports __index__()
    switch (_EVSpace_AsIndex(indices, &start0))
    {
        case -1: return NULL;
        case 0: break;
        case 1:
            matrix_view = EVSpaceMatrixView_New(self, type, 1, 3 * start0, 3, -1,
                                                sizeof(double), -1);
            return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
    }

    // single arg as slice type
    if (PySlice_Check(indices))
    {
        if (PySlice_GetIndicesEx(indices, 3, &start0, &stop0, &step0, &slicelength0) < 0) {
            return NULL;
        }
        
        if (start0 == stop0) {
            matrix_view = EVSpaceMatrixView_New(self, type, 1, 0, 0, -1, sizeof(double), -1);
        }
        else {
            // Second dimensions here all have '3' hardcoded because all columns are included
            matrix_view = EVSpaceMatrixView_New(self, type, 2, 3 * start0, slicelength0, 3,
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
                matrix_view = EVSpaceMatrixView_New(self, type, 1, 0, 0, -1,
                                                    sizeof(double), -1);
                return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
            }

            // (slice, slice)
            if (PySlice_Check(key1))
            {
                if (PySlice_GetIndicesEx(key1, 3, &start1, &stop1, &step1, &slicelength1) < 0) {
                    return NULL;
                }

                if (start1 == stop1) {
                    matrix_view = EVSpaceMatrixView_New(self, type, 1, 0, 0, -1,
                                                        sizeof(double), -1);
                }
                else {
                    matrix_view = EVSpaceMatrixView_New(self, type, 2, 3 * start0 + start1,
                                                        slicelength0, slicelength1,
                                                        3 * step0 * sizeof(double),
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
                    matrix_view = EVSpaceMatrixView_New(self, type, 1, 3 * start0 + index1,
                                                        slicelength0, -1,
                                                        3 * step0 * sizeof(double), -1);
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
                matrix_view = EVSpaceMatrixView_New(self, type, 1, 0, 0, -1,
                                                    sizeof(double), -1);
            }
            else {
                matrix_view = EVSpaceMatrixView_New(self, type, 1, 3 * index0 + start1,
                                                    slicelength1, -1,
                                                    step1 * sizeof(double), -1);
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

    // This should be unreachable, but place an informative error in case it happens.
    PyErr_SetString(PyExc_RuntimeError, "pyevspace.Matirx.__getitem__() an unknown "
                    "error occurred parsing arguments");
    return NULL;
}

static int
Matrix_map_assignment(EVSpace_Matrix* self, PyObject* indices, PyObject* rhs)
{
    PyObject* key0 = NULL, *key1 = NULL;
    Py_ssize_t index0{}, index1{}, tmp{};
    double value{};

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
            PyErr_Format(PyExc_IndexError,
                         "first index must be in range [0-2] (got %i)", index0);
            return -1;
        }

        tmp = index1 + 1;
        PySlice_AdjustIndices(3, &index1, &tmp, 1);
        if (index1 == tmp || index1 < 0 || index1 > 2) {
            PyErr_Format(PyExc_IndexError,
                         "second index must be in range [0-2] (got %i)", index1);
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
    if (PyArg_ParseTuple(indices, "O!O!:pyevspace.Matrix.__getitem__",
                         &PySlice_Type, &key0, &PySlice_Type, &key1) ||
        PyArg_ParseTuple(indices, "nO!:pyevspace.Matrix.__getitem__",
                         &index0, &PySlice_Type, &key0) ||
        PyArg_ParseTuple(indices, "O!n:pyevspace.Matrix.__getitem__",
                         &PySlice_Type, &key0, &index0))
    {
        PyErr_SetString(PyExc_Warning, "slice assignment not available");
    }
    return -1;
}

/* Matrix sequence support. This is for internals only for iter support. */

static PyObject*
Matrix_get_item(EVSpace_Matrix* self, Py_ssize_t index_arg)
{
    EVSpace_MatrixView* matrix_view = NULL;
    Py_ssize_t index = index_arg;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state) {
        return NULL;
    }

    if (index < 0) {
        index += 3;
    }

    if (index < 0 || index > 2)
    {
        PyErr_Format(PyExc_IndexError, "index must be between [0-2], got %dl", index_arg);
        return NULL;
    }

    matrix_view = EVSpaceMatrixView_New(self, state->MatrixView_Type, 1, 3 * index,
                                        3, -1, sizeof(double), -1);
    return PyMemoryView_FromObject(EVS_PyObject_Cast(matrix_view));
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

    view->obj           = Py_NewRef(self);

    return 0;
}

/* Matrix instance methods */
//      Matrix instance methods
// Methods that require the module state must be defined as `CPyMethod`
// style signatures. This will pass the `defining_class` argument which
// enables the EVSpaceState_From* functions to properly get the module
// state to verify arguments and instantiate new objects.

static PyObject*
Matrix_reduce(EVSpace_Matrix* self, PyTypeObject* defining_class,
              PyObject* const *Py_UNUSED(), Py_ssize_t nargs,
              PyObject* kwargs)
{
    EVSpace_State* state = NULL;

    if (nargs != 0 || kwargs != NULL)
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.Matrix.__reduce__() expected no args");
        return NULL;
    }
    
    state = (EVSpace_State*)PyType_GetModuleState(defining_class);
    if (!state) {
        return NULL;
    }

    return Py_BuildValue("(O((ddd)(ddd)(ddd)))", state->Matrix_Type,
                        EVSpaceMatrix_MATRIX(self)(0, 0), EVSpaceMatrix_MATRIX(self)(0, 1),
                        EVSpaceMatrix_MATRIX(self)(0, 2), EVSpaceMatrix_MATRIX(self)(1, 0),
                        EVSpaceMatrix_MATRIX(self)(1, 1), EVSpaceMatrix_MATRIX(self)(1, 2),
                        EVSpaceMatrix_MATRIX(self)(2, 0), EVSpaceMatrix_MATRIX(self)(2, 1),
                        EVSpaceMatrix_MATRIX(self)(2, 2));
}

static PyObject*
Matrix_determinate(EVSpace_Matrix* self, PyObject* Py_UNUSED())
{
    double result = EVSpaceMatrix_MATRIX(self).determinate();

    return PyFloat_FromDouble(result);
}

static PyObject*
Matrix_transpose(EVSpace_Matrix* self, PyTypeObject* defining_class,
                 PyObject* const *Py_UNUSED(), Py_ssize_t nargs,
                 PyObject* kwargs)
{
    EVSpace_State* state = (EVSpace_State*)PyType_GetModuleState(defining_class);
    if (!state) {
        return NULL;
    }

    if (nargs != 0 || kwargs != NULL)
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.Matrix.transpose(): "
                        "expected no arguments");
        return NULL;
    }

    EXCEPTION_WRAPPER(
        evspace::Matrix result = EVSpaceMatrix_MATRIX(self).transpose();
        return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(result), state->Matrix_Type));
    )
}

static PyObject*
Matrix_transpose_inplace(EVSpace_Matrix* self, PyObject* Py_UNUSED())
{
    EVSpaceMatrix_MATRIX(self).transpose_inplace();

    Py_RETURN_NONE;
}

static PyObject*
Matrix_inverse(EVSpace_Matrix* self, PyTypeObject* defining_class,
               PyObject* const *Py_UNUSED(), Py_ssize_t nargs,
               PyObject* kwargs)
{
    EVSpace_State* state = (EVSpace_State*)PyType_GetModuleState(defining_class);
    if (!state) {
        return NULL;
    }

    if (nargs != 0 || kwargs != NULL)
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.Matrix.inverse(): "
                        "expected no arguments");
        return NULL;
    }

    // Nest try blocks so we only catch std::runtime_error thrown from Matrix.inverse()
    try {
        try {

            evspace::Matrix inverse = EVSpaceMatrix_MATRIX(self).inverse();
            return EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(inverse),
                                                       state->Matrix_Type));
        }
        catch (const std::bad_alloc&) {
            return PyErr_NoMemory();
        }
        catch (const std::runtime_error& e) {
            if (std::string_view(e.what()) == "Unable to invert singular matrix") {
                PyErr_SetString(PyExc_ValueError, "pyevspace.Matrix.inverse(): "
                                "unable to invert singular matrix");
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
    EVSpace_Matrix* rhs = NULL;
    int max_ulps{};
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "O!i:compare_to_ulp", state->Matrix_Type,
                          &rhs, &max_ulps)) {
        return NULL;
    }

    if (max_ulps < 0) {
        PyErr_Format(PyExc_ValueError, "pyevspace.Matrix.compare_to_ulp(): "
                     "max_ulps must be non-negative (got %i)", max_ulps);
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
    EVSpace_Matrix* rhs = NULL;
    double rel_tol = evspace::DEFAULT_REL_TOL, abs_tol = evspace::DEFAULT_ABS_TOL;

    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);
    if (!state) {
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "O!|dd:compare_to_tol", state->Matrix_Type, &rhs,
                          &rel_tol, &abs_tol)) {
        return NULL;
    }

    if (rel_tol < 0) {
        PyErr_SetString(PyExc_ValueError, "pyevspace.Matrix.compare_to_tol(): "
                        "rel_tol must be non-negative");
        return NULL;
    }
    if (abs_tol < 0) {
        PyErr_SetString(PyExc_ValueError, "pyevspace.Matrix.compare_to_tol(): "
                        "abs_tol must be non-negative");
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

    {"__reduce__", (PyCFunction)(void(*)(void))Matrix_reduce, EVS_PYCMETHOD, matrix_reduce_doc},

    {"transpose", (PyCFunction)(void(*)(void))Matrix_transpose, EVS_PYCMETHOD, matrix_transpose_doc},

    {"transpose_inplace", (PyCFunction)Matrix_transpose_inplace, METH_NOARGS, matrix_transpose_inplace_doc},

    {"determinate", (PyCFunction)Matrix_determinate, METH_NOARGS, matrix_determinate_doc},

    {"inverse", (PyCFunction)(void(*)(void))Matrix_inverse, EVS_PYCMETHOD, matrix_inverse_doc},

    {"compare_to_ulp", (PyCFunction)Matrix_compare_to_ulp, METH_VARARGS, matrix_compare_to_ulp_doc},

    {"compare_to_tol", (PyCFunction)Matrix_compare_to_tol, METH_VARARGS, matrix_compare_to_tol_doc},

    {NULL, NULL, 0, NULL}
};

PyDoc_STRVAR(matrix_doc, "pyevspace.matrix([row0: Iterable, row1: Iterable, row2: Iterable])\n\
\n\
The pyevspace.Matrix type can be constructed with rows of iterables all of\
length three, whose components are numeric types. Alternatively if no arguments\
are present each component is defaulet to zero.");

static PyType_Slot matrix_slots[] = {
    {Py_tp_dealloc,                 (void*)Matrix_dealloc},
    {Py_tp_repr,                    (void*)Matrix_repr},
    {Py_nb_add,                     (void*)Matrix_add},
    {Py_nb_subtract,                (void*)Matrix_subtract},
    {Py_nb_multiply,                (void*)Matrix_multiply},
    {Py_nb_negative,                (void*)Matrix_negative},
    {Py_nb_inplace_add,             (void*)Matrix_inplace_add},
    {Py_nb_inplace_subtract,        (void*)Matrix_inplace_subtract},
    {Py_nb_inplace_multiply,        (void*)Matrix_inplace_multiply},
    {Py_nb_true_divide,             (void*)Matrix_divide},
    {Py_nb_inplace_true_divide,     (void*)Matrix_inplace_divide},
    {Py_nb_matrix_multiply,         (void*)Matrix_matrix_multiply},
    {Py_nb_inplace_matrix_multiply, (void*)Matrix_inplace_multiply_matrix},
    // sequence protocol is only for internal CPython support
    {Py_sq_length,                  (void*)EVSpaceObject_Length},
    {Py_sq_item,                    (void*)Matrix_get_item},
    {Py_mp_length,                  (void*)EVSpaceObject_Length},
    {Py_mp_subscript,               (void*)Matrix_map_subscript},
    {Py_mp_ass_subscript,           (void*)Matrix_map_assignment},
    {Py_tp_str,                     (void*)Matrix_str},
    {Py_bf_getbuffer,               (void*)Matrix_get_buffer},
    {Py_bf_releasebuffer,           (void*)EVSpaceBuffer_Release},
    {Py_tp_doc,                     (void*)matrix_doc},
    {Py_tp_richcompare,             (void*)Matrix_richcompare},
    {Py_tp_methods,                 (void*)matrix_methods},
    {Py_tp_init,                    (void*)Matrix_init},
    {Py_tp_new,                     (void*)Matrix_new},
    {0,                             NULL}
};

static PyType_Spec matrix_spec = {
    "pyevspace.Matrix",     /* name */
    sizeof(EVSpace_Matrix), /* basicsize */
    0,                      /* itemsize */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE | Py_TPFLAGS_BASETYPE, /* flags */
    matrix_slots            /* slots */
};

/* EulerAngles type constructors */
//      EulerAngles new function(s)
// These functions validate `type` is a subclass of EVSpace_EulerAngles
// and allocate and initialize the underlying evspace::EulerAngles object
// to the parameter values. The EVSpace_EulerAngles.angles instance is
// allocated using `new` and should be freed with `delete`
// so the desstructor is called.
//
// While the type supports subclassing, all methods attached to
// EVSpace_EulerAngles strictly return EVSpace_EulerAngles instances instead
// of subclassed types because any additional parameters to a subclasses
// __init__ methods are not known. Any subclass wanting to return an
// instance of that subclass should overload that function and use the
// returned values to initialize an instance of their type.

static EVSpace_Angles*
EVSpaceAngles_New(double alpha, double beta, double gamma, PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state)
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.EulerAngles", type->tp_name);
        return NULL;
    }

    if (!PyObject_IsSubclass((PyObject*)type, (PyObject*)state->EulerAngles_Type))
    {
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.EulerAngles", type->tp_name);
        return NULL;
    }

    EVSpace_Angles* angles = reinterpret_cast<EVSpace_Angles*>(type->tp_alloc(type, 0));
    if (!angles) {
        return NULL;
    }

    EXCEPTION_WRAPPER(
        angles->angles = new evspace::EulerAngles(alpha, beta, gamma);
        return angles;
    )
}

static PyObject*
Angles_new(PyTypeObject* type, PyObject* Py_UNUSED(1), PyObject* Py_UNUSED(2))
{
    return reinterpret_cast<PyObject*>(EVSpaceAngles_New(0.0, 0.0, 0.0, type));
}

static int
Angles_init(EVSpace_Angles* self, PyObject* args, PyObject* Py_UNUSED())
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

// Creates and fills a string buffer for the inner data of an EulerAngles
// object. On failure set an exception and return NULL. If non-NULL, the
// caller must free the buffer by calling `free` on the returned pointer.
static inline const char*
_EVSpaceAngles_String(const EVSpace_Angles* angles)
{
    const char* format = "%g, %g, %g";
    char* buffer = NULL;
    std::size_t buffer_length = snprintf(NULL, 0, format, EVSpaceAngles_ALPHA(angles),
                                         EVSpaceAngles_BETA(angles), EVSpaceAngles_GAMMA(angles));

    buffer = (char*)malloc(buffer_length + 1);
    if (!buffer) {
        PyErr_NoMemory();
        return NULL;
    }
    buffer[buffer_length] = '\0';

    if (sprintf(buffer, format, EVSpaceAngles_ALPHA(angles), EVSpaceAngles_BETA(angles),
                EVSpaceAngles_GAMMA(angles)) < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "error filling string buffer");
        free((void*)buffer);
        return NULL;
    }

    return buffer;
}

static PyObject*
Angles_str(const EVSpace_Angles* angles)
{
    PyObject* str = NULL;
    // We must delete this!
    const char* buffer = _EVSpaceAngles_String(angles);
    if (!buffer) {
        return NULL;
    }

    str = PyUnicode_FromFormat("[%s]", buffer);
    free((void*)buffer);

    return str;
}

static PyObject*
Angles_repr(const EVSpace_Angles* self)
{
    const char* buffer = NULL;
    PyObject* repr = NULL;
    PyObject* type = (PyObject*)Py_TYPE(self);
    PyObject* module = PyObject_GetAttrString(type, "__module__");
    PyObject* qualname = PyObject_GetAttrString(type, "__qualname__");

    if (!module || !qualname) {
        Py_XDECREF(module);
        Py_XDECREF(qualname);
        return NULL;
    }

    // We must delete this!
    buffer = _EVSpaceAngles_String(self);
    if (!buffer)
    {
        Py_DECREF(module);
        Py_DECREF(qualname);
        return NULL;
    }

    repr = PyUnicode_FromFormat("%U.%U(%s)", module, qualname, buffer);
    Py_DECREF(module);
    Py_DECREF(qualname);
    free((void*)buffer);

    return repr;
}

static PyObject*
Angles_reduce(EVSpace_Angles* self, PyTypeObject* defining_class,
              PyObject* const *Py_UNUSED(), Py_ssize_t nargs,
              PyObject* kwargs)
{
    EVSpace_State* state = NULL;

    if (nargs != 0 || kwargs != NULL)
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.EulerAngles.__reduce__() "
                        "expected no args");
        return NULL;
    }
    
    state = (EVSpace_State*)PyType_GetModuleState(defining_class);
    if (!state) {
        return NULL;
    }

    return Py_BuildValue("(O(ddd))", state->EulerAngles_Type, EVSpaceAngles_ALPHA(self),
                         EVSpaceAngles_BETA(self), EVSpaceAngles_GAMMA(self));
}

static PyObject*
Angles_get_item(EVSpace_Angles* self, Py_ssize_t index)
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
    double value{};

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

PyDoc_STRVAR(angles_reduce_doc, "pyevspace.__reduce__() -> (cls, state)\n\
\n\
Allows pickle support of EulerAngles objects.");

static PyMethodDef angles_methods[] = {

    {"__reduce__", (PyCFunction)(void(*)(void))Angles_reduce, EVS_PYCMETHOD, angles_reduce_doc},

    {NULL, NULL, 0, NULL}
};

PyDoc_STRVAR(angles_doc, "pyevspace.EulerAngles(alpha: Real, beta: Real, gamma: Real)\n\
\n\
Sets the corresponding angles all of which are required. Unneeded\n\
angles should be set to zero.");

static PyType_Slot angles_slots[] = {
    {Py_tp_dealloc,             (void*)Angles_dealloc},
    {Py_tp_repr,                (void*)Angles_repr},
    {Py_sq_length,              (void*)EVSpaceObject_Length},
    {Py_sq_item,                (void*)Angles_get_item},
    {Py_sq_ass_item,            (void*)Angles_set_item},
    {Py_tp_str,                 (void*)Angles_str},
    {Py_tp_doc,                 (void*)angles_doc},
    {Py_tp_methods,             (void*)angles_methods},
    {Py_tp_init,                (void*)Angles_init},
    {Py_tp_new,                 (void*)Angles_new},
    {0,                         NULL}
};

static PyType_Spec angles_spec = {
    "pyevspace.EulerAngles",    /* name */
    sizeof(EVSpace_Angles),     /* basicsize */
    0,                          /* itemsize */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE | Py_TPFLAGS_BASETYPE, /* flags */
    angles_slots                /* slots */
};

//      EVSpace_Order new function
// Validate `type` is a subclass of EVSpace_Order and initialize
// the order values. This type has no members to be allocated
// and is an immutable type in the Python layer, so it should
// not be mutated within this file either. The EVSpace_Order
// struct members are `evspace::AxisDirection` types instead of
// plain `int` types so that they can be directly used in
// evspace functions.
//
// While the type supports subclassing, all methods attached to
// EVSpace_Order strictly return EVSpace_Order instances instead
// of subclassed types because any additional parameters to a subclasses
// __init__ methods are not known. Any subclass wanting to return an
// instance of that subclass should overload that function and use the
// returned values to initialize an instance of their type.

static EVSpace_Order*
EVSpaceOrder_New(evspace::AxisDirection first, evspace::AxisDirection second,
                  evspace::AxisDirection thrid, PyTypeObject* type)
{
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state)
    {
        PyErr_Clear();
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of "
                     "pyevspace.RotationOrder", type->tp_name);
        return NULL;
    }

    if (!PyObject_IsSubclass((PyObject*)type, (PyObject*)state->RotationOrder_Type))
    {
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.RotationOrder",
                     type->tp_name);
        return NULL;
    }

    EVSpace_Order* order = reinterpret_cast<EVSpace_Order*>(type->tp_alloc(type, 0));
    if (!order) {
        return NULL;
    }

    order->first = first;
    order->second = second;
    order->third = thrid;

    return order;
}

static PyObject*
Order_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    int first, second, third;
    
    if (!PyArg_ParseTuple(args, "iii", &first, &second, &third)) {
        return NULL;
    }

    if (first < 0 || first > 2) {
        PyErr_Format(PyExc_ValueError, "first axis must be in [0-2], got %d", first);
        return NULL;
    }

    if (second < 0 || second > 2) {
        PyErr_Format(PyExc_ValueError, "second axis must be in [0-2], got %d", second);
        return NULL;
    }

    if (third < 0 || third > 2) {
        PyErr_Format(PyExc_ValueError, "third axis must be in [0-2], got %d", third);
        return NULL;
    }

    return (PyObject*)EVSpaceOrder_New(
        static_cast<evspace::AxisDirection>(first),
        static_cast<evspace::AxisDirection>(second),
        static_cast<evspace::AxisDirection>(third),
        type
    );
}

static void
Order_dealloc(EVSpace_Order* self)
{
    Py_TYPE(self)->tp_free(EVS_PyObject_Cast(self));
}

static PyObject*
Order_str(const EVSpace_Order* self)
{
    char buffer[4] = "";
    buffer[0] = static_cast<int>(self->first) + 'X';
    buffer[1] = static_cast<int>(self->second) + 'X';
    buffer[2] = static_cast<int>(self->third) + 'X';

    return PyUnicode_FromString(buffer);
}

static PyObject*
Order_repr(const EVSpace_Order* self)
{
    PyObject* type = NULL, *module = NULL, *qualname = NULL;
    char buffer[] = "pyevspace.X_AXIS, pyevspace.X_AXIS, pyevspace.X_AXIS";
    
    // Axis name indices are at 10, 28, 46
    buffer[10] += static_cast<int>(self->first);
    buffer[28] += static_cast<int>(self->second);
    buffer[46] += static_cast<int>(self->third);

    type = (PyObject*)Py_TYPE(self);
    module = PyObject_GetAttrString(type, "__module__");
    qualname = PyObject_GetAttrString(type, "__qualname__");
    if (!module || !qualname)
    {
        Py_XDECREF(module);
        Py_XDECREF(qualname);
    }

    return PyUnicode_FromFormat("%U.%U(%s)", module, qualname, buffer);
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
    EVSpace_Order* rhs = NULL;
    bool is_equal = false;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return NULL;
    }

    if (PyObject_TypeCheck(other, state->RotationOrder_Type))
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
Order_reduce(EVSpace_Order* self, PyTypeObject* defining_class,
             PyObject* const *Py_UNUSED(), Py_ssize_t nargs,
             PyObject* kwargs)
{
    EVSpace_State* state = NULL;

    if (nargs != 0 || kwargs != NULL)
    {
        PyErr_SetString(PyExc_TypeError, "pyevspace.RotationOrder.__reduce__ "
                        "expected no arguments");
        return NULL;
    }

    state = (EVSpace_State*)PyType_GetModuleState(defining_class);
    if (!state) {
        return NULL;
    }

    return Py_BuildValue("(O(iii))", state->RotationOrder_Type,
                         self->first, self->second, self->third);
}

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

    {NULL, 0, 0, 0, NULL}
};

PyDoc_STRVAR(order_reduce_doc, "pyevspace.Matrix.__reduce__() -> (cls, state)\n\
\n\
Allows pickle support of Order instances.");

static PyMethodDef order_methods[] = {
    {"__reduce__", (PyCFunction)(void(*)(void))Order_reduce, EVS_PYCMETHOD, order_reduce_doc},

    {NULL, NULL, 0, NULL}
};

PyDoc_STRVAR(order_doc, "RotationOrder(axis1: int, axis2: int, axis3: int)\n\
\n\
All axes are required and should be one of the three enumerated\n\
axis types X_AXIS, Y_AXIS, or Z_AXIS.");

static PyType_Slot order_slots[] = {
    {Py_tp_dealloc,             (void*)Order_dealloc},
    {Py_tp_repr,                (void*)Order_repr},
    {Py_sq_length,              (void*)EVSpaceObject_Length},
    {Py_sq_item,                (void*)Order_get_item},
    {Py_tp_hash,                (void*)Order_hash},
    {Py_tp_str,                 (void*)Order_str},
    {Py_tp_doc,                 (void*)order_doc},
    {Py_tp_richcompare,         (void*)Order_richcompare},
    {Py_tp_methods,             (void*)order_methods},
    {Py_tp_members,             (void*)order_members},
    {Py_tp_new,                 (void*)Order_new},
    {0,                         NULL}
};

static PyType_Spec order_spec = {
    "pyevspace.RotationOrder",  /* name */
    sizeof(EVSpace_Order),      /* basicsize */
    0,                          /* itemsize */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE | Py_TPFLAGS_BASETYPE, /* flags */
    order_slots                 /* slots */
};

//      Rotation support methods
// These methods act as a facade to the evspace rotation methods. Some
// of these are simply meant to improve syntax, as the internals of
// several methods would appear all throughout the code base. Others
// are required to compute compound rotation matricies since they
// require template parameters which cannot be deduced at runtime.
// Functions that create dynamically allocated evspace types are
// allocated using `new` and should be freed with `delete` to ensure
// the destructor is called.

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
    evspace::Matrix* matrix_alpha, *matrix_beta, *matrix_gamma;

    EXCEPTION_WRAPPER(
        evspace::Matrix result{};
        
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
            return false;
        }

        error_message = PyUnicode_AsUTF8(str_obj);
        
        if (!error_message) {
            Py_DECREF(str_obj);
            return false;
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

//      EVSpace_ReferenceFrame new function
// Validate `type` is a subclass of EVSpace_ReferenceFrame and
// allocate and initialize internal memory. Using an EVSpace_Matrix
// object will be slower and unnecessary as we don't want it directly
// exposed anyway. Use an `evspace::Matrix` object allocated using
// `new` and free using `delete` so the destructor is called. The
// only internal needed to be a `PyObject*` is the offset member,
// as this can be equal to `Py_None`. The `evspace::ReferenceFrame`
// object cannot readily be used as it's a template class, and the
// axes values in the python module won't be known at compile time,
// but have to be usable at runtime. Because of this we will leverage
// some helper methods that replicate some of the internals of
// evspace in terms of generating the correct rotation matrices
// but using the interface we've designed here. EVSpace_ReferenceFrame
// also requires garbage collection support because of the ownership
// of another `PyObject*` instance.
//
// While the type supports subclassing, all methods attached to
// EVSpace_ReferenceFrame strictly return EVSpace_ReferenceFrame instances
// instead of subclassed types because any additional parameters to a
// subclasses __init__ methods are not known. Any subclass wanting to
// return an instance of that subclass should overload that function
// and use the returned values to initialize an instance of their type.


// Updates the internal rotation matrix of an EVSpace_ReferenceFrame
// object. On success set `frame->matrix` to the updated matrix and
// return 0. On failure `frame->matrix` is left untouched and -1 is
// returned. In case of failure the `frame` object is left in an
// invalid state: the internal rotation matrix may not reflect the
// rotation defined by the internal order and angles (if any angles
// were modified before this function was called, which should
// always be the case). Because the cause of the exception is unknown
// to any caller, an attempt to salvage the cause for error should be
// avoided and the exception should be propagated be returning the
// necessary value to indicate an error.
static int
EVSpaceReferenceFrame_UpdateMatrix(EVSpace_ReferenceFrame* frame)
{
    evspace::Matrix* tmp = _EVSpaceRotate_ComputeMatrix(
        frame->first, frame->second, frame->third, frame->angles[0],
        frame->angles[1], frame->angles[2], frame->intrinsic);

    if (tmp) {
        frame->matrix = tmp;
        return 0;
    }

    return -1;
}

static EVSpace_ReferenceFrame*
EVSpaceReferenceFrame_New(evspace::AxisDirection first, evspace::AxisDirection second,
                           evspace::AxisDirection third, const evspace::EulerAngles& angles,
                           PyObject* offset, bool intrinsic, PyTypeObject* type)
{
    EVSpace_ReferenceFrame* self = NULL;
    EVSpace_State* state = (EVSpace_State*)EVSpaceState_FromType(type);

    if (!state)
    {
        // Since this function can be called from anywhere, we should annotate a full
        // exception if `type` is not a correct subclass
        if (PyErr_Occurred())
        {
            PyErr_Clear();
            PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.ReferenceFrame",
                         type->tp_name);
        }
        return NULL;
    }

    if (!PyObject_IsSubclass((PyObject*)type, (PyObject*)state->ReferenceFrame_Type))
    {
        PyErr_Format(PyExc_TypeError, "%s is not a subtype of pyevspace.ReferenceFrame",
                     type->tp_name);
        return NULL;
    }
    
    self = (EVSpace_ReferenceFrame*)(type->tp_alloc(type, 0));
    if (!self) {
        return NULL;
    }

    self->first = first;
    self->second = second;
    self->third = third;

    self->angles[0] = (angles)[0];
    self->angles[1] = (angles)[1];
    self->angles[2] = (angles)[2];

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
        self->offset = Py_NewRef(Py_None);
    }
    else if (PyObject_TypeCheck(offset, state->Vector_Type))
    {
        self->offset = Py_NewRef(offset);
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

static PyObject*
ReferenceFrame_New(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    EVSpace_ReferenceFrame* self = (EVSpace_ReferenceFrame*)(type->tp_alloc(type, 0));

    if (!self) {
        return NULL;
    }

    try {
        self->matrix = new evspace::Matrix();
    }
    catch (const std::bad_alloc&) {
        Py_DECREF(self);
        return PyErr_NoMemory();
    }

    self->first = evspace::AxisDirection::X;
    self->second = evspace::AxisDirection::Y;
    self->third = evspace::AxisDirection::Z;

    self->angles[0] = 0.0;
    self->angles[1] = 0.0;
    self->angles[2] = 0.0;

    self->intrinsic = true;

    self->offset = Py_NewRef(Py_None);

    return (PyObject*)self;
}

static int
ReferenceFrame_init(EVSpace_ReferenceFrame* self, PyObject* args, PyObject* kwargs)
{
    EVSpace_Order* order = NULL;
    EVSpace_Angles* angles = NULL;
    PyObject* offset = Py_NewRef(Py_None);
    int intrinsic = true;
    static const char* kwargs_list[] = { "", "", "intrinsic", "offset", NULL };
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return -1;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!|$pO", const_cast<char**>(kwargs_list),
                                     state->RotationOrder_Type, &order, state->EulerAngles_Type,
                                     &angles, &intrinsic, &offset)) {
        return -1;
    }

    if (!(Py_IsNone(offset) || PyObject_TypeCheck(offset, state->Vector_Type)))
    {
        PyErr_Format(PyExc_TypeError, "offset must be a pyevspace.Vector type or None, got %s",
                     __EVSpace_GetTypeName(offset));
        return -1;
    }

    Py_SETREF(self->offset, Py_NewRef(offset));

    self->first = order->first;
    self->second = order->second;
    self->third = order->third;

    self->angles[0] = EVSpaceAngles_ALPHA(angles);
    self->angles[1] = EVSpaceAngles_BETA(angles);
    self->angles[2] = EVSpaceAngles_GAMMA(angles);

    self->intrinsic = intrinsic;

    if (EVSpaceReferenceFrame_UpdateMatrix(self) < 0)
    {
        return -1;
    }

    return 0;
}

static int
ReferenceFrame_traverse(EVSpace_ReferenceFrame* self, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(self));
    Py_VISIT(self->offset);
    return 0;
}

static int
ReferenceFrame_clear(EVSpace_ReferenceFrame* self)
{
    Py_CLEAR(self->offset);
    return 0;
}

static void
ReferenceFrame_Dealloc(EVSpace_ReferenceFrame* self)
{
    PyObject_GC_UnTrack(self);
    delete self->matrix;
    ReferenceFrame_clear(self);
    PyTypeObject* tp = Py_TYPE(self);
    Py_TYPE(self)->tp_free(EVS_PyObject_Cast(self));
    Py_DECREF(tp);
}

static PyObject*
ReferenceFrame_GetMatrix(EVSpace_ReferenceFrame* self, PyObject* Py_UNUSED())
{
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return NULL;
    }

    return reinterpret_cast<PyObject*>(EVSpaceMatrix_New(*self->matrix, state->Matrix_Type));
}

static PyObject*
ReferenceFrame_GetAngles(EVSpace_ReferenceFrame* self, PyObject* Py_UNUSED())
{
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return NULL;
    }

    return reinterpret_cast<PyObject*>(EVSpaceAngles_New(self->angles[0], self->angles[1],
                                                         self->angles[2], state->EulerAngles_Type));
}

static PyObject*
ReferenceFrame_SetAngles(EVSpace_ReferenceFrame* self, PyObject* args, PyObject* kwargs)
{
    double alpha{}, beta{}, gamma{};
    PyObject* arg1{NULL}, *arg2{NULL}, *arg3{NULL};
    EVSpace_Angles* angles{NULL};
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    static const char* kwarg_names[] = {"alpha", "beta", "gamma", NULL};
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return NULL;
    }

    if (arg_count == 1)
    {   
        if (PyArg_ParseTuple(args, "O!:set_angles", state->EulerAngles_Type, &angles))
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
        if (Py_IsNone(arg1)) {
            arg1 = NULL;
        }

        if (Py_IsNone(arg2)) {
            arg2 = NULL;
        }

        if (Py_IsNone(arg3))
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
ReferenceFrame_GetOrder(EVSpace_ReferenceFrame* self, void* Py_UNUSED())
{
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return NULL;
    }

    return reinterpret_cast<PyObject*>(
        EVSpaceOrder_New(self->first, self->second, self->third,
                         state->RotationOrder_Type)
    );
}

static PyObject*
ReferenceFrame_GetOffset(EVSpace_ReferenceFrame* self, void* Py_UNUSED())
{
    return Py_NewRef(self->offset);
}

static int
ReferenceFrame_SetOffset(EVSpace_ReferenceFrame* self, PyObject* arg,
                         void* Py_UNUSED())
{
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return -1;
    }

    if (!arg) {
        PyErr_SetString(PyExc_ValueError, "cannot delete offset attribute");
        return -1;
    }

    if (!Py_IsNone(arg) && !PyObject_TypeCheck(arg, state->Vector_Type))
    {
        PyErr_Format(PyExc_TypeError, "expected offset to be Vector type or "
                     "None, got %s", __EVSpace_GetTypeName(arg));
        return -1;
    }

    Py_SETREF(self->offset, Py_NewRef(arg));

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

    {NULL, 0, 0, 0, NULL}
};

static PyGetSetDef reference_frame_getset[] = {
    {"order", (getter)ReferenceFrame_GetOrder, NULL,
     PyDoc_STR("order of axes in the rotation"), NULL},

    {"offset", (getter)ReferenceFrame_GetOffset,
     (setter)ReferenceFrame_SetOffset,
     PyDoc_STR("offset of the reference frame origin"), NULL},

    {NULL, NULL, NULL, NULL, NULL}
};

static PyObject*
ReferenceFrame_RotateTo(EVSpace_ReferenceFrame* self, PyObject* args)
{
    PyObject* arg1 = NULL, *arg2 = NULL;
    EVSpace_Vector* vector = NULL, *offset = NULL;
    EVSpace_ReferenceFrame* other_frame = NULL;
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    evspace::Vector* tmp = NULL;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return NULL;
    }

    if (arg_count == 1)
    {
        arg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyObject_TypeCheck(arg1, state->Vector_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be Vector type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return NULL;
        }

        vector = EVSpaceVector_Cast(arg1);

        if (Py_IsNone(self->offset))
        {
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(self->matrix, vector->vector)),
                                  state->Vector_Type)
            );
        }
        else
        {
            offset = EVSpaceVector_Cast(self->offset);
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(
                    std::move(*_EVSpaceRotate_To(self->matrix, vector->vector, offset->vector)),
                    state->Vector_Type
                )
            );
        }
    }
    else if (arg_count == 2)
    {
        arg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyObject_TypeCheck(arg1, state->ReferenceFrame_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be ReferenceFrame type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return NULL;
        }

        arg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyObject_TypeCheck(arg2, state->Vector_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 2 to be Vector type, got %s",
                         __EVSpace_GetTypeName(arg2));
            return NULL;
        }

        other_frame = reinterpret_cast<EVSpace_ReferenceFrame*>(arg1);
        vector = EVSpaceVector_Cast(arg2);

        if (Py_IsNone(other_frame->offset))
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

        if (Py_IsNone(self->offset))
        {
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(self->matrix, tmp)),
                                  state->Vector_Type)
            );
        }
        else
        {
            offset = EVSpaceVector_Cast(self->offset);
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(self->matrix, tmp, offset->vector)),
                                  state->Vector_Type)
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
    PyObject* arg1 = NULL, *arg2 = NULL;
    EVSpace_Vector* vector = NULL, *offset = NULL;
    EVSpace_ReferenceFrame* other_frame = NULL;
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    evspace::Vector* tmp = NULL;
    EVSpace_State* state = EVSpaceState_FromObject((PyObject*)self);

    if (!state) {
        return NULL;
    }

    if (arg_count == 1)
    {
        arg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyObject_TypeCheck(arg1, state->Vector_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be Vector type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return NULL;
        }

        vector = EVSpaceVector_Cast(arg1);

        if (Py_IsNone(self->offset))
        {
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_From(self->matrix, vector->vector)),
                                  state->Vector_Type)
            );
        }
        else
        {
            offset = EVSpaceVector_Cast(self->offset);
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(
                    std::move(*_EVSpaceRotate_From(self->matrix, vector->vector, offset->vector)),
                    state->Vector_Type
                )
            );
        }
    }
    else if (arg_count == 2)
    {
        arg1 = PyTuple_GET_ITEM(args, 0);
        if (!PyObject_TypeCheck(arg1, state->ReferenceFrame_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be ReferenceFrame type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return NULL;
        }

        arg2 = PyTuple_GET_ITEM(args, 1);
        if (!PyObject_TypeCheck(arg2, state->Vector_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 2 to be Vector type, got %s",
                         __EVSpace_GetTypeName(arg2));
            return NULL;
        }

        other_frame = reinterpret_cast<EVSpace_ReferenceFrame*>(arg1);
        vector = EVSpaceVector_Cast(arg2);

        if (Py_IsNone(self->offset))
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

        if (Py_IsNone(other_frame->offset))
        {
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(other_frame->matrix, tmp)),
                                  state->Vector_Type)
            );
        }
        else
        {
            offset = EVSpaceVector_Cast(other_frame->offset);
            return reinterpret_cast<PyObject*>(
                EVSpaceVector_New(std::move(*_EVSpaceRotate_To(other_frame->matrix, tmp,
                                                               offset->vector)),
                                  state->Vector_Type)
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

PyDoc_STRVAR(refframe_set_angles_doc, "set_angles(angles: EulerAngles) -> None\n\
set_angles(*, alpha: float = None, beta: float = None, gamma: float = None) -> None\n\
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

    {"set_angles", (PyCFunction)(void(*)(void))ReferenceFrame_SetAngles,
     METH_VARARGS | METH_KEYWORDS, refframe_set_angles_doc},

    {"rotate_to", (PyCFunction)ReferenceFrame_RotateTo, METH_VARARGS,
     refframe_rotate_to_doc},

    {"rotate_from", (PyCFunction)ReferenceFrame_RotateFrom, METH_VARARGS,
     refframe_rotate_from_doc},

    {NULL, NULL, 0, NULL}
};

PyDoc_STRVAR(reference_frame_doc, "ReferenceFrame(order: RotationOrder, angles: EulerAngles, *, offset: Vector | None = None, intrinsic: bool = True)\n\
A class that represents a reference frame defined by the parameters. The class\n\
can then be used to rotate vectors to, from, or even between other ReferenceFrame\n\
object.");

static PyType_Slot frame_slots[] = {
    {Py_tp_dealloc,         (void*)ReferenceFrame_Dealloc},
    {Py_tp_traverse,        (void*)ReferenceFrame_traverse},
    {Py_tp_clear,           (void*)ReferenceFrame_clear},
    {Py_tp_doc,             (void*)reference_frame_doc},
    {Py_tp_methods,         (void*)reference_frame_methods},
    {Py_tp_members,         (void*)reference_frame_members},
    {Py_tp_getset,          (void*)reference_frame_getset},
    {Py_tp_new,             (void*)ReferenceFrame_New},
    {Py_tp_init,            (void*)ReferenceFrame_init},
    {0,                     NULL}
};

static PyType_Spec frame_spec = {
    "pyevspace.ReferenceFrame",     /* name */
    sizeof(EVSpace_ReferenceFrame), /* basicsize */
    0,                              /* itemsize */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,   /* flags */
    frame_slots                     /* slots */
};

//      Module rotation methods

// Analyze the args tuple and deduce the most likely intended signature
// and produce an informative type error. The `args` parameter MUST be
// a python tuple type. This function does not care about keyword arguments
// and keyword argument errors should be handled separately.
static void
__EVSpace_ComputeMatrixError(PyObject* args, EVSpace_State* state)
{
    PyObject* arg1 = NULL, *arg2 = NULL, *arg3 = NULL, *arg4 = NULL;
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    double dummy_float{};

    arg1 = PyTuple_GET_ITEM(args, 0);
    arg2 = PyTuple_GET_ITEM(args, 1);
    if (arg_count == 2)
    {
        if (EVSpaceObject_AsDouble(arg1, dummy_float))
        {
            if (!PyLong_Check(arg2) && !PyObject_TypeCheck(arg2, state->Vector_Type))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 2 to be an int or Vector type, got %s",
                             __EVSpace_GetTypeName(arg2));
                return;
            }
        }
        else if (PyObject_TypeCheck(arg1, state->RotationOrder_Type))
        {
            if (!PyObject_TypeCheck(arg2, state->EulerAngles_Type))
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
        if (!PyObject_TypeCheck(arg1, state->RotationOrder_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be a RotationOrder type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return;
        }
        else if (!PyObject_TypeCheck(arg2, state->EulerAngles_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 2 to be an EulerAngles type, got %s",
                         __EVSpace_GetTypeName(arg2));
            return;
        }
        else if (!PyObject_TypeCheck(arg3, state->RotationOrder_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 3 to be a RotationOrder type, got %s",
                         __EVSpace_GetTypeName(arg3));
            return;
        }
        else if (!PyObject_TypeCheck(arg4, state->EulerAngles_Type))
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
EVSpaceRotation_ComputeMatrix(PyObject* module, PyObject* args, PyObject* kwargs)
{
    PyObject* result = NULL;
    evspace::Matrix* matrix = NULL, *tmp = NULL;
    EVSpace_Vector* vector_axis = NULL;
    EVSpace_Order* order_from = NULL, *order_to = NULL;
    EVSpace_Angles* angles_from = NULL, *angles_to = NULL;
    double angle{};
    int axis{}, intrinsic_from = 1, intrinsic_to = 1;
    static const char* kwlist_empty[] = { "", "", NULL };
    static const char* kwlist_euler[] = { "", "", "intrinsic", NULL };
    static const char* kwlist_between[] = { "", "", "", "", "intrinsic_from", "intrinsic_to", NULL };
    EVSpace_State* state = (EVSpace_State*)PyModule_GetState(module);

    if (!state) {
        return NULL;
    }

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

        result = EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(*matrix), state->Matrix_Type));
        delete matrix;
        return result;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return NULL;
    }

    // (a: float, ax: Vector)
    PyErr_Clear();
    if (PyArg_ParseTupleAndKeywords(args, kwargs, "dO!:compute_rotation_matrix",
        const_cast<char**>(kwlist_empty), &angle, state->Vector_Type, &vector_axis))
    {
        matrix = _EVSpaceRotate_ComputeMatrix(angle, vector_axis->vector);
        if (!matrix) {
            return NULL;
        }

        result = EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(*matrix), state->Matrix_Type));
        delete matrix;
        return result;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return NULL;
    }
    
    // (o: RotationOrder, a: EulerAngles, i: bool = True)
    PyErr_Clear();
    if (PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!|$p", const_cast<char**>(kwlist_euler),
        state->RotationOrder_Type, &order_from, state->EulerAngles_Type, &angles_from,
        &intrinsic_from))
    {
        matrix = _EVSpaceRotate_ComputeMatrix(order_from, angles_from, intrinsic_from);
        if (!matrix) {
            return NULL;
        }

        result = EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(*matrix), state->Matrix_Type));
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
        state->RotationOrder_Type, &order_from, state->EulerAngles_Type, &angles_from,
        state->RotationOrder_Type, &order_to, state->EulerAngles_Type, &angles_to,
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

        result = EVS_PyObject_Cast(EVSpaceMatrix_New(std::move(*matrix), state->Matrix_Type));
        delete matrix;
        delete tmp;
        return result;
    }
    else if (__EVSpaceRotate_CheckKeywordException()) {
        return NULL;
    }

    PyErr_Clear();
    __EVSpace_ComputeMatrixError(args, state);

    return NULL;
}

// Checks the value of offset for Py_None and fills vector_offset if applicable.
// The `offset_not_none` is an easy way of directly telling callers of the
// function which signature of a rotation computation function to call. If the
// `offset` parameter is `Py_None` then `vector_offset` is not touched and
// `offset_not_none is set to true, otherwise `vector_offset` is filled and
// `offset_not_none is set to false. Returns -1 on error and 0 on success.
static inline int
__EVSpaceRotate_HandleOffset(PyObject* offset, evspace::Vector** vector_offset,
                             bool& offset_not_none, EVSpace_State* state)
{
    if (!Py_IsNone(offset))
    {
        if (!PyObject_TypeCheck(offset, state->Vector_Type))
        {
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

// Analyze the args tuple and deduce the most likely intended signature
// and produce an informative type error. The `args` parameter MUST be
// a python tuple type. This function does not care about keyword arguments
// and keyword argument errors should be handled separately.
static void
_EVSpaceRotate_ProcessArgsError(PyObject* args, EVSpace_State* state)
{
    Py_ssize_t arg_count = PyTuple_GET_SIZE(args);
    PyObject* arg1 = NULL, *arg2 = NULL, *arg3 = NULL;
    double dummy_float{};

    arg1 = PyTuple_GET_ITEM(args, 0);
    arg2 = PyTuple_GET_ITEM(args, 1);
    if (arg_count == 2)
    {
        if (!PyObject_TypeCheck(arg1, state->Matrix_Type))
        {
            PyErr_Format(PyExc_TypeError,
                         "expected argument 1 to be a Matrix type, got %s",
                         __EVSpace_GetTypeName(arg1));
            return;
        }

        if (!PyObject_TypeCheck(arg2, state->Vector_Type))
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
            if (!PyObject_TypeCheck(arg2, state->Matrix_Type) && !PyLong_Check(arg2))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 2 to be an int or Vector type, got %s",
                             __EVSpace_GetTypeName(arg2));
                return;
            }
            if (!PyObject_TypeCheck(arg3, state->Vector_Type))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 3 to be a Vector type, got %s",
                             __EVSpace_GetTypeName(arg3));
                return;
            }
        }
        if (PyObject_TypeCheck(arg1, state->RotationOrder_Type))
        {
            if (!PyObject_TypeCheck(arg2, state->EulerAngles_Type))
            {
                PyErr_Format(PyExc_TypeError,
                             "expected argument 2 to be an EulerAngles type, got %s",
                             __EVSpace_GetTypeName(arg2));
                return;
            }
            if (!PyObject_TypeCheck(arg3, state->Vector_Type))
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
                           bool& offset_not_none, const char* name, EVSpace_State* state)
{
    // signatures that we need to accept:
    // (matrix: Matrix, vector: Vector, *, offset: Vector = None)
    // (angle: float, axis: int, vector: Vector, *, offset: Vector = None)
    // (angle: float, axis: Vector, vector: Vector, *, offset: Vector = None)
    // (order: RotationOrder, angles: EulerAngles, vector: Vector, *, offset: Vector = None,
    //  intrinsic: bool = True)

    PyObject* offset = Py_None;
    evspace::Matrix* matrix_tmp = NULL;
    EVSpace_Matrix* matrix_wrapper = NULL;
    EVSpace_Vector* vector_wrapper = NULL, *vector_axis = NULL;
    EVSpace_Order* order = NULL;
    EVSpace_Angles* angles = NULL;
    double angle{};
    int axis{}, intrinsic = 1;
    char format[64];

    static const char* kwlist_matrix[] = { "", "", "offset", NULL };
    static const char* kwlist_axis[] = { "", "", "", "offset", NULL };
    static const char* kwlist_euler[] = { "", "", "", "offset", "intrinsic", NULL };

    // (m: Matrix, v: Vector, *, o: Vector = None)
    strcpy(format, "O!O!|$O:");
    strcat(format, name);
    if (PyArg_ParseTupleAndKeywords(args, kwargs, format, const_cast<char**>(kwlist_matrix),
        state->Matrix_Type, &matrix_wrapper, state->Vector_Type, &vector_wrapper, &offset))
    {
        if (__EVSpaceRotate_HandleOffset(offset, vector_offset, offset_not_none, state) < 0) {
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
        &angle, &axis, state->Vector_Type, &vector_wrapper, &offset))
    {
        if (__EVSpaceRotate_HandleOffset(offset, vector_offset, offset_not_none, state) < 0) {
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
        &angle, state->Vector_Type, &vector_axis, state->Vector_Type,
        &vector_wrapper, &offset))
    {
        if (__EVSpaceRotate_HandleOffset(offset, vector_offset, offset_not_none, state) < 0) {
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
        state->RotationOrder_Type, &order, state->EulerAngles_Type, &angles,
        state->Vector_Type, &vector_wrapper, &offset, &intrinsic))
    {
        if (__EVSpaceRotate_HandleOffset(offset, vector_offset, offset_not_none, state) < 0) {
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
    _EVSpaceRotate_ProcessArgsError(args, state);

    return -1;
}

static PyObject*
EVSpaceRotate_From(PyObject* module, PyObject* args, PyObject* kwargs)
{
    evspace::Matrix* matrix = NULL;
    evspace::Vector* vector = NULL, *vector_offset = NULL;
    bool offset_not_none = true;
    EVSpace_State* state = (EVSpace_State*)PyModule_GetState(module);

    EXCEPTION_WRAPPER(
        matrix = new evspace::Matrix();
        vector = new evspace::Vector();
        vector_offset = new evspace::Vector();
    )

    // Use _EVSpaceRotate_ProcessArgs to handle generating the matrix and
    // filling the vector and vector_offset values.
    if (_EVSpaceRotate_ProcessArgs(args, kwargs, &matrix, &vector,
                                   &vector_offset, offset_not_none, "rotate_from", state) < 0)
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
        EVSpace_Vector* rtn = EVSpaceVector_New(std::move(*result), state->Vector_Type);
        return EVS_PyObject_Cast(rtn);
    )
}

static PyObject*
EVSpaceRotate_To(PyObject* module, PyObject* args, PyObject* kwargs)
{
    evspace::Matrix* matrix = NULL;
    evspace::Vector* vector = NULL, *vector_offset = NULL;
    bool offset_not_none = true;
    EVSpace_State* state = (EVSpace_State*)PyModule_GetState(module);

    EXCEPTION_WRAPPER(
        matrix = new evspace::Matrix();
        vector = new evspace::Vector();
        vector_offset = new evspace::Vector();
    )

    // Use _EVSpaceRotate_ProcessArgs to handle generating the matrix and
    // filling the vector and vector_offset values.
    if (_EVSpaceRotate_ProcessArgs(args, kwargs, &matrix, &vector,
                                   &vector_offset, offset_not_none, "rotate_from", state) < 0)
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
        EVSpace_Vector* rtn = EVSpaceVector_New(std::move(*result), state->Vector_Type);
        return EVS_PyObject_Cast(rtn);
    )
}

static PyObject*
EVSpaceRotate_Between(PyObject* module, PyObject* args, PyObject* kwargs)
{
    EVSpace_Order* order_from = NULL, *order_to = NULL;
    EVSpace_Angles* angles_from = NULL, *angles_to = NULL;
    evspace::Matrix* matrix_from = NULL, *matrix_to = NULL;
    evspace::Vector* result = NULL, *vector_offset_from = NULL, *vector_offset_to = NULL;
    PyObject* offset_from = Py_None, *offset_to = Py_None;
    EVSpace_Vector* rtn = NULL, *vector_wrapper = NULL;
    int intrinsic_from = 1, intrinsic_to = 1;
    static const char* kwlist[] = {"", "", "", "", "", "intrinsic_from", "intrinsic_to",
                                   "offset_from", "offset_to", NULL};
    EVSpace_State* state = (EVSpace_State*)PyModule_GetState(module);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!O!O!O!|$ppOO:rotate_between", const_cast<char**>(kwlist),
        state->RotationOrder_Type, &order_from, state->EulerAngles_Type, &angles_from,
        state->RotationOrder_Type, &order_to, state->EulerAngles_Type, &angles_to,
        state->Vector_Type, &vector_wrapper, &intrinsic_from, &intrinsic_to,
        &offset_from, &offset_to))
    {
        return NULL;
    }

    if (!Py_IsNone(offset_from))
    {
        if (!PyObject_TypeCheck(offset_from, state->Vector_Type))
        {
            PyErr_SetString(PyExc_TypeError, "offset_from argument cannot be interpreted as Vector");
            return NULL;
        }
        vector_offset_from = EVSpaceVector_Cast(offset_from)->vector;
    }

    if (!Py_IsNone(offset_to))
    {
        if (!PyObject_TypeCheck(offset_to, state->Vector_Type))
        {
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

    rtn = EVSpaceVector_New(std::move(*result), state->Vector_Type);
    delete result;

    return EVS_PyObject_Cast(rtn);
}

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

    {"vector_dot", (PyCFunction)(void(*)(void))Vector_dot, METH_FASTCALL, vector_dot_doc},

    {"vector_cross", (PyCFunction)(void(*)(void))Vector_cross, METH_FASTCALL, vector_cross_doc},

    {"vector_angle", (PyCFunction)(void(*)(void))Vector_angle, METH_FASTCALL, vector_angle_doc},

    {"vector_exclude", (PyCFunction)(void(*)(void))Vector_exclude, METH_FASTCALL, vector_exclude_doc},

    {"vector_proj", (PyCFunction)(void(*)(void))Vector_projection, METH_FASTCALL, vector_projection_doc},

    {"compute_rotation_matrix", (PyCFunction)(void(*)(void))EVSpaceRotation_ComputeMatrix,
     METH_VARARGS | METH_KEYWORDS, compute_rotation_doc },

    {"rotate_from", (PyCFunction)(void(*)(void))EVSpaceRotate_From,
     METH_VARARGS | METH_KEYWORDS, rotate_from_doc},

    {"rotate_to", (PyCFunction)(void(*)(void))EVSpaceRotate_To,
     METH_VARARGS | METH_KEYWORDS, rotate_to_doc},

    {"rotate_between", (PyCFunction)(void(*)(void))EVSpaceRotate_Between,
     METH_VARARGS | METH_KEYWORDS, rotate_between_doc},

    {NULL, NULL, 0, NULL}
};

//      Capsule functions
// Some of these functions requier a PyTypeObject in order to retrieve
// the module state for validation and creating new instances of types
// of the module.

static int EVSpaceVector_GetState(PyObject* obj, double arr[3])
{
    EVSpace_Vector* vector;
    double* state;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->Vector_Type))
    {
        PyErr_Format(PyExc_TypeError,
                     "cannot interpret %s as Vector type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    vector = reinterpret_cast<EVSpace_Vector*>(obj);
    state = EVSpaceVector_VECTOR(vector).data().data();
    std::memcpy(arr, state, 3 * sizeof(double));

    return 0;
}

static int EVSpaceMatrix_GetState(PyObject* obj, double arr[9])
{
    EVSpace_Matrix* matrix;
    double* state;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->Matrix_Type))
    {
        PyErr_Format(PyExc_TypeError,
                     "cannot interpret %s as Matrix type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    matrix = reinterpret_cast<EVSpace_Matrix*>(obj);
    state = EVSpaceMatrix_MATRIX(matrix).data().data();
    std::memcpy(arr, state, 9 * sizeof(double));

    return 0;
}

static int EVSpaceAngles_GetState(PyObject* obj, double arr[3])
{
    EVSpace_Angles* angles;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->EulerAngles_Type))
    {
        PyErr_Format(PyExc_TypeError,
                     "cannot interpret %s as EulerAngles type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    angles = reinterpret_cast<EVSpace_Angles*>(obj);
    arr[0] = EVSpaceAngles_ALPHA(angles);
    arr[1] = EVSpaceAngles_BETA(angles);
    arr[2] = EVSpaceAngles_GAMMA(angles);

    return 0;
}

static int EVSpaceOrder_GetState(PyObject* obj, unsigned int arr[3])
{
    EVSpace_Order* order;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->RotationOrder_Type))
    {
        PyErr_Format(PyExc_TypeError,
                     "cannot interpret %s as RotationOrder type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    order = reinterpret_cast<EVSpace_Order*>(obj);
    arr[0] = static_cast<int>(order->first);
    arr[1] = static_cast<int>(order->second);
    arr[2] = static_cast<int>(order->third);

    return 0;
}

static int EVSpaceFrame_GetState(PyObject* obj, unsigned int order[3], double angles[3],
                                 double offset[3], int* intrinsic)
{
    EVSpace_ReferenceFrame* frame;
    EVSpace_Vector* offset_vector;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->ReferenceFrame_Type))
    {
        PyErr_Format(PyExc_TypeError,
                     "cannot interpret %s as ReferenceFrame type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    frame = reinterpret_cast<EVSpace_ReferenceFrame*>(obj);
    order[0] = static_cast<int>(frame->first);
    order[1] = static_cast<int>(frame->second);
    order[2] = static_cast<int>(frame->third);
    angles[0] = frame->angles[0];
    angles[1] = frame->angles[1];
    angles[2] = frame->angles[2];
    
    if (Py_IsNone(frame->offset))
    {
        offset[0] = offset[1] = offset[2] = 0.0;
    }
    else {
        offset_vector = reinterpret_cast<EVSpace_Vector*>(frame->offset);
        std::memcpy(offset,
                    EVSpaceVector_VECTOR(offset_vector).data().data(),
                    3 * sizeof(double));
    }

    *intrinsic = static_cast<int>(frame->intrinsic);

    return 0;
}

static PyObject*
EVSpaceFrame_GetOrder(PyObject* obj)
{
    EVSpace_ReferenceFrame* frame;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return NULL;
    }

    if (!PyObject_TypeCheck(obj, module_state->ReferenceFrame_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as ReferenceFrame type",
                     __EVSpace_GetTypeName);
        return NULL;
    }

    frame = reinterpret_cast<EVSpace_ReferenceFrame*>(obj);

    return EVS_PyObject_Cast(EVSpaceOrder_New(frame->first, frame->second, frame->third,
                                              module_state->RotationOrder_Type));
}

static PyObject*
EVSpaceFrame_GetAngles(PyObject* obj)
{
    EVSpace_ReferenceFrame* frame;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return NULL;
    }

    if (!PyObject_TypeCheck(obj, module_state->ReferenceFrame_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as ReferenceFrame type",
                     __EVSpace_GetTypeName);
        return NULL;
    }

    frame = reinterpret_cast<EVSpace_ReferenceFrame*>(obj);

    return EVS_PyObject_Cast(EVSpaceAngles_New(frame->angles[0], frame->angles[1],
                                               frame->angles[2], module_state->EulerAngles_Type));
}

static PyObject*
EVSpaceFrame_GetOffset(PyObject* obj)
{
    EVSpace_ReferenceFrame* frame;
    EVSpace_Vector* offset;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return NULL;
    }

    if (!PyObject_TypeCheck(obj, module_state->ReferenceFrame_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as ReferenceFrame type",
                     __EVSpace_GetTypeName);
        return NULL;
    }

    frame = reinterpret_cast<EVSpace_ReferenceFrame*>(obj);

    if (Py_IsNone(frame->offset))
    {
        Py_RETURN_NONE;
    }
    else {
        offset = reinterpret_cast<EVSpace_Vector*>(frame->offset);
        return EVS_PyObject_Cast(EVSpaceVector_New(*offset->vector, module_state->Vector_Type));
    }
}

static int
EVSpaceFrame_GetIntrinsic(PyObject* obj)
{
    EVSpace_ReferenceFrame* frame;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->ReferenceFrame_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as ReferenceFrame type",
                     __EVSpace_GetTypeName);
        return -1;
    }

    frame = reinterpret_cast<EVSpace_ReferenceFrame*>(obj);
    return static_cast<int>(frame->intrinsic);
}

PyObject* EVSpaceVector_FromState(PyTypeObject* type, double arr[3])
{
    // todo: evspace::Vector should be able to take a evspace::span_t
    EVSpace_Vector* rtn = NULL;

    EXCEPTION_WRAPPER(
        evspace::Vector vector = evspace::Vector(arr[0], arr[1], arr[2]);
        rtn = EVSpaceVector_New(std::move(vector), type);
    )

    return EVS_PyObject_Cast(rtn);
}

// todo: wrap all evspace calls in exceptions
PyObject* EVSpaceMatrix_FromState(PyTypeObject* type, double arr[9])
{
    EVSpace_Matrix* rtn = NULL;

    EXCEPTION_WRAPPER(
        evspace::span_t<double> span(arr, 9);
        evspace::Matrix matrix{span};
        rtn = EVSpaceMatrix_New(std::move(matrix), type);
    )        

    return EVS_PyObject_Cast(rtn);
}

PyObject* EVSpaceAngles_FromState(PyTypeObject* type, double arr[3])
{
    EVSpace_Angles* angles = NULL;
        
    angles = EVSpaceAngles_New(arr[0], arr[1], arr[2], type);

    return EVS_PyObject_Cast(angles);
}

PyObject* EVSpaceOrder_FromState(PyTypeObject* type, unsigned int arr[3])
{
    if (!(arr[0] <= 2)) {
        PyErr_Format(PyExc_ValueError, "first rotation order axis must be in [0-2], got %u",
                     arr[0]);
        return NULL;
    }
    if (!(arr[1] <= 2)) {
        PyErr_Format(PyExc_ValueError, "second rotation order axis must be in [0-2], got %u",
                     arr[1]);
        return NULL;
    }
    if (!(arr[2] <= 2)) {
        PyErr_Format(PyExc_ValueError, "third rotation order axis must be in [0-2], got %u",
                     arr[2]);
        return NULL;
    }
    EVSpace_Order* order = EVSpaceOrder_New(
        static_cast<evspace::AxisDirection>(arr[0]),
        static_cast<evspace::AxisDirection>(arr[1]),
        static_cast<evspace::AxisDirection>(arr[2]), type
    );

    return EVS_PyObject_Cast(order);
}

static PyObject*
EVSpaceFrame_FromState(PyTypeObject* type, unsigned int order[3], double angles[3],
                       double* offset, int intrinsic)
{
    EVSpace_Vector* offset_vector = NULL;
    PyObject* offset_arg = NULL;
    EVSpace_ReferenceFrame* frame = NULL;
    EVSpace_State* state = EVSpaceState_FromType(type);

    if (!state) {
        return NULL;
    }
    
    if (!(order[0] <= 2)) {
        PyErr_Format(PyExc_ValueError, "first rotation order axis must be in [0-2], got %u",
                     order[0]);
        return NULL;
    }
    if (!(order[1] <= 2)) {
        PyErr_Format(PyExc_ValueError, "second rotation order axis must be in [0-2], got %u",
                     order[1]);
        return NULL;
    }
    if (!(order[2] <= 2)) {
        PyErr_Format(PyExc_ValueError, "third rotation order axis must be in [0-2], got %u",
                     order[2]);
        return NULL;
    }
    
    EXCEPTION_WRAPPER(
        evspace::EulerAngles angles_arg = evspace::EulerAngles(angles[0], angles[1], angles[2]);

        if (!offset) {
            offset_arg = NULL;
        }
        else {
            evspace::Vector tmp_offset = evspace::Vector(offset[0], offset[1], offset[2]);
            offset_vector = EVSpaceVector_New(std::move(tmp_offset), state->Vector_Type);
            if (!offset_vector) {
                return NULL;
            }
            offset_arg = reinterpret_cast<PyObject*>(offset_vector);
        }
        
        frame = EVSpaceReferenceFrame_New(
            static_cast<evspace::AxisDirection>(order[0]),
            static_cast<evspace::AxisDirection>(order[1]),
            static_cast<evspace::AxisDirection>(order[2]),
            angles_arg, offset_arg, static_cast<bool>(intrinsic), type
        );
    )

    return EVS_PyObject_Cast(frame);
}

static int
EVSpaceVector_SetState(PyObject* obj, double state[3])
{
    EVSpace_Vector* vector = NULL;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->Vector_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as Vector type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    vector = reinterpret_cast<EVSpace_Vector*>(obj);
    double* ptr = EVSpaceVector_VECTOR(vector).data().data();
    std::memcpy(ptr, state, 3 * sizeof(double));

    return 0;
}

static int
EVSpaceMatrix_SetState(PyObject* obj, double state[9])
{
    EVSpace_Matrix* matrix = NULL;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->Matrix_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as Matrix type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    matrix = reinterpret_cast<EVSpace_Matrix*>(obj);
    double* ptr = EVSpaceMatrix_MATRIX(matrix).data().data();
    std::memcpy(ptr, state, 9 * sizeof(double));

    return 0;
}

static int
EVSpaceAngles_SetState(PyObject* obj, double state[3])
{
    EVSpace_Angles* angles = NULL;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->EulerAngles_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as EulerAgles type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    angles = reinterpret_cast<EVSpace_Angles*>(obj);
    EVSpaceAngles_ALPHA(angles) = state[0];
    EVSpaceAngles_BETA(angles) = state[1];
    EVSpaceAngles_GAMMA(angles) = state[2];

    return 0;
}

static int
EVSpaceFrame_SetAngles(PyObject* obj, double state[3])
{
    EVSpace_ReferenceFrame* frame = NULL;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->ReferenceFrame_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as a ReferenceFrame type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    frame = reinterpret_cast<EVSpace_ReferenceFrame*>(obj);
    std::memcpy(frame->angles.data(), state, 3 * sizeof(double));

    EVSpaceReferenceFrame_UpdateMatrix(frame);
    return 0;
}

static int
EVSpaceFrame_SetOffset(PyObject* obj, double* state)
{
    EVSpace_ReferenceFrame* frame = NULL;
    PyObject* vector = NULL;
    EVSpace_State* module_state = EVSpaceState_FromObject(obj);

    if (!module_state) {
        return -1;
    }

    if (!PyObject_TypeCheck(obj, module_state->ReferenceFrame_Type))
    {
        PyErr_Format(PyExc_TypeError, "cannot interpret %s as a ReferenceFrame type",
                     __EVSpace_GetTypeName(obj));
        return -1;
    }

    frame = reinterpret_cast<EVSpace_ReferenceFrame*>(obj);

    if (!state)
    {
        if (Py_IsNone(frame->offset))
        {
            return 0;
        }
    }

    try {
        evspace::Vector tmp{state[0], state[1], state[2]};
        vector = EVS_PyObject_Cast(EVSpaceVector_New(std::move(tmp), module_state->Vector_Type));
    }
    catch (const std::bad_alloc&) {
        PyErr_NoMemory();
        return -1;
    }
    
    
    if (!vector) {
        return -1;
    }
    Py_SETREF(frame->offset, vector);

    return 0;
}

/* module initialization */

template<typename first, typename second, typename third>
static int
_EVSpace_AddOrder(PyObject* module, EVSpace_State* state, const char* name)
{
    PyObject* order = EVS_PyObject_Cast(
        EVSpaceOrder_New(first::direction, second::direction,
                         third::direction, state->RotationOrder_Type)
    );

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

static int
EVSpace_InitializeConstants(PyObject* module)
{
    PyObject* tmp = NULL;
    EVSpace_State* state = (EVSpace_State*)PyModule_GetState(module);

    if (!state) {
        return -1;
    }

    tmp = (PyObject*)EVSpaceVector_New(evspace::Vector::e1, state->Vector_Type);
    if (!tmp) return -1;
    if (PyDict_SetItemString(state->Vector_Type->tp_dict, "E1", tmp) < 0) {
        return -1;
    }

    tmp = (PyObject*)EVSpaceVector_New(evspace::Vector::e2, state->Vector_Type);
    if (!tmp) return -1;
    if (PyDict_SetItemString(state->Vector_Type->tp_dict, "E2", tmp) < 0) {
        return -1;
    }

    tmp = (PyObject*)EVSpaceVector_New(evspace::Vector::e3, state->Vector_Type);
    if (!tmp) return -1;
    if (PyDict_SetItemString(state->Vector_Type->tp_dict, "E3", tmp) < 0) {
        return -1;
    }

    tmp = (PyObject*)EVSpaceMatrix_New(evspace::Matrix::IDENTITY, state->Matrix_Type);
    if (!tmp) return -1;
    if (PyDict_SetItemString(state->Matrix_Type->tp_dict, "IDENTITY", tmp) < 0) {
        return -1;
    }

    if (PyModule_AddIntConstant(module, "X_AXIS", (int)evspace::AxisDirection::X) < 0) {
        return -1;
    }

    if (PyModule_AddIntConstant(module, "Y_AXIS", (int)evspace::AxisDirection::Y) < 0) {
        return -1;
    }

    if (PyModule_AddIntConstant(module, "Z_AXIS", (int)evspace::AxisDirection::Z) < 0) {
        return -1;
    }

    if (_EVSpace_AddOrder<evspace::XAxis, evspace::YAxis, evspace::ZAxis>(module, state, "XYZ") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::XAxis, evspace::ZAxis, evspace::YAxis>(module, state, "XZY") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::YAxis, evspace::XAxis, evspace::ZAxis>(module, state, "YXZ") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::YAxis, evspace::ZAxis, evspace::XAxis>(module, state, "YZX") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::ZAxis, evspace::XAxis, evspace::YAxis>(module, state, "ZXY") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::ZAxis, evspace::YAxis, evspace::XAxis>(module, state, "ZYX") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::XAxis, evspace::YAxis, evspace::XAxis>(module, state, "XYX") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::XAxis, evspace::ZAxis, evspace::XAxis>(module, state, "XZX") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::YAxis, evspace::XAxis, evspace::YAxis>(module, state, "YXY") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::YAxis, evspace::ZAxis, evspace::YAxis>(module, state, "YZY") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::ZAxis, evspace::XAxis, evspace::ZAxis>(module, state, "ZXZ") < 0) {
        return -1;
    }
    if (_EVSpace_AddOrder<evspace::ZAxis, evspace::YAxis, evspace::ZAxis>(module, state, "ZYZ") < 0) {
        return -1;
    }

    return 0;
}

static void
_EVSpaceCapsule_Destructor(PyObject* capsule)
{
    PyEVSpace_CAPI* api = (PyEVSpace_CAPI*)PyCapsule_GetPointer(capsule, PYEVSPACE_CAPSULE_NAME);
    free(api);
}

static int
EVSpace_InitializeCapsule(PyObject* module)
{
    PyEVSpace_CAPI* evspace_api = NULL;
    EVSpace_State* state = (EVSpace_State*)PyModule_GetState(module);

    if (!state) {
        return -1;
    }

    evspace_api = (PyEVSpace_CAPI*)malloc(sizeof(PyEVSpace_CAPI));
    if (!evspace_api) {
        PyErr_NoMemory();
        return -1;
    }

    state->capsule = evspace_api;

    evspace_api->version = PYEVSPACE_CAPSULE_VERSION;

    evspace_api->Vector_Type = state->Vector_Type;
    evspace_api->Matrix_Type = state->Matrix_Type;
    evspace_api->EulerAngles_Type = state->EulerAngles_Type;
    evspace_api->RotationOrder_Type = state->RotationOrder_Type;
    evspace_api->ReferenceFrame_Type = state->ReferenceFrame_Type;

    evspace_api->PyEVSpaceVector_GetState = EVSpaceVector_GetState;
    evspace_api->PyEVSpaceMatrix_GetState = EVSpaceMatrix_GetState;
    evspace_api->PyEVSpaceAngles_GetState = EVSpaceAngles_GetState;
    evspace_api->PyEVSpaceOrder_GetState = EVSpaceOrder_GetState;
    evspace_api->PyEVSpaceFrame_GetState = EVSpaceFrame_GetState;
    evspace_api->PyEVSpaceFrame_GetOrder = EVSpaceFrame_GetOrder;
    evspace_api->PyEVSpaceFrame_GetAngles = EVSpaceFrame_GetAngles;
    evspace_api->PyEVSpaceFrame_GetOffset = EVSpaceFrame_GetOffset;
    evspace_api->PyEVSpaceFrame_GetIntrinsic = EVSpaceFrame_GetIntrinsic;

    evspace_api->PyEVSpaceVector_FromState = EVSpaceVector_FromState;
    evspace_api->PyEVSpaceMatrix_FromState = EVSpaceMatrix_FromState;
    evspace_api->PyEVSpaceAngles_FromState = EVSpaceAngles_FromState;
    evspace_api->PyEVSpaceOrder_FromState = EVSpaceOrder_FromState;
    evspace_api->PyEVSpaceFrame_FromState = EVSpaceFrame_FromState;

    evspace_api->PyEVSpaceVector_SetState = EVSpaceVector_SetState;
    evspace_api->PyEVSpaceMatrix_SetState = EVSpaceMatrix_SetState;
    evspace_api->PyEVSpaceAngles_SetState = EVSpaceAngles_SetState;
    evspace_api->PyEVSpaceFrame_SetAngles = EVSpaceFrame_SetAngles;
    evspace_api->PyEVSpaceFrame_SetOffset = EVSpaceFrame_SetOffset;

    PyObject* capsule = PyCapsule_New((void*)evspace_api, PYEVSPACE_CAPSULE_NAME,
                                      _EVSpaceCapsule_Destructor);
    // It's not clear that PyModule_AddObject() below will accept NULL, so we'll check
    // just to be safe.
    if (!capsule) {
        free(evspace_api);
        return -1;
    }
    
#if PY_VERSION_HEX >= 0x030d0000
    if (PyModule_Add(module, "_C_API", capsule) < 0) {
        free(evspace_api);
        return -1;
    }
#else
    if (PyModule_AddObject(module, "_C_API", capsule) < 0) {
        Py_XDECREF(capsule);
        free(evspace_api);
        return -1;
    }
#endif

    return 0;
}

static int EVSpace_Initialize(PyObject* module)
{
    EVSpace_State* state = (EVSpace_State*)PyModule_GetState(module);
    if (!state) {
        return -1;
    }

    state->Vector_Type = (PyTypeObject*)PyType_FromModuleAndSpec(module, &vector_spec, NULL);
    if (!state->Vector_Type) {
        return -1;
    }

    if (PyModule_AddObjectRef(module, "Vector", (PyObject*)state->Vector_Type) < 0) {
        return -1;
    }

    state->Matrix_Type = (PyTypeObject*)PyType_FromModuleAndSpec(module, &matrix_spec, NULL);
    if (!state->Matrix_Type) {
        return -1;
    }

    if (PyModule_AddObjectRef(module, "Matrix", (PyObject*)state->Matrix_Type) < 0) {
        return -1;
    }

    state->EulerAngles_Type = (PyTypeObject*)PyType_FromModuleAndSpec(module, &angles_spec, NULL);
    if (!state->EulerAngles_Type) {
        return -1;
    }

    if (PyModule_AddObjectRef(module, "EulerAngles", (PyObject*)state->EulerAngles_Type) < 0) {
        return -1;
    }

    state->RotationOrder_Type = (PyTypeObject*)PyType_FromModuleAndSpec(module, &order_spec, NULL);
    if (!state->RotationOrder_Type) {
        return -1;
    }

    if (PyModule_AddObjectRef(module, "RotationOrder", (PyObject*)state->RotationOrder_Type) < 0) {
        return -1;
    }

    state->ReferenceFrame_Type = (PyTypeObject*)PyType_FromModuleAndSpec(module, &frame_spec, NULL);
    if (!state->ReferenceFrame_Type) {
        return -1;
    }

    if (PyModule_AddObjectRef(module, "ReferenceFrame", (PyObject*)state->ReferenceFrame_Type) < 0) {
        return -1;
    }

    state->MatrixView_Type = (PyTypeObject*)PyType_FromModuleAndSpec(module, &matrixview_spec, NULL);
    if (!state->MatrixView_Type) {
        return -1;
    }
    
    return 0;
}

PyDoc_STRVAR(evspace_doc, "A 3-dimensional Euclidean vector space package for\
vector and matrix types with support for reference frames and Euler rotations.");

static int
EVSpace_traverse(PyObject* module, visitproc visit, void* arg)
{
    EVSpace_State* state = reinterpret_cast<EVSpace_State*>(PyModule_GetState(module));
    if (!state) {
        // Module's not initialized, nothing to traverse
        return 0;
    }

    Py_VISIT(state->Vector_Type);
    Py_VISIT(state->Matrix_Type);
    Py_VISIT(state->MatrixView_Type);
    Py_VISIT(state->EulerAngles_Type);
    Py_VISIT(state->RotationOrder_Type);
    Py_VISIT(state->ReferenceFrame_Type);
    
    return 0;
}

static int
EVSpace_clear(PyObject* module)
{
    EVSpace_State* state = reinterpret_cast<EVSpace_State*>(PyModule_GetState(module));
    if (!state) {
        // Module's not initialized, nothing to clear
        return 0;
    }

    Py_CLEAR(state->Vector_Type);
    Py_CLEAR(state->Matrix_Type);
    Py_CLEAR(state->MatrixView_Type);
    Py_CLEAR(state->EulerAngles_Type);
    Py_CLEAR(state->RotationOrder_Type);
    Py_CLEAR(state->ReferenceFrame_Type);
    
    return 0;
}

static PyModuleDef_Slot evspace_slots[] = {
    {Py_mod_exec, (void*)EVSpace_Initialize},
    {Py_mod_exec, (void*)EVSpace_InitializeCapsule},
    {Py_mod_exec, (void*)EVSpace_InitializeConstants},
#if PY_VERSION_HEX >= 0x030c0000
    {Py_mod_multiple_interpreters, Py_MOD_PER_INTERPRETER_GIL_SUPPORTED},
#endif
#if PY_VERSION_HEX >= 0x030d0000
    {Py_mod_gil, Py_MOD_GIL_USED},
#endif
    {0, NULL}
};

PyMODINIT_FUNC
PyInit__pyevspace(void)
{
    // These modify global state each time a module is created, but these values
    // are constants and are effectively no-ops.
    EVSpace_Module.m_doc        = evspace_doc;
    EVSpace_Module.m_size       = sizeof(EVSpace_State);
    EVSpace_Module.m_methods    = evspace_methods;
    EVSpace_Module.m_slots      = evspace_slots;
    EVSpace_Module.m_traverse   = EVSpace_traverse;
    EVSpace_Module.m_clear      = EVSpace_clear;

    return PyModuleDef_Init(&EVSpace_Module);
}
