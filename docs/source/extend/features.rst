.. role:: c(code)
    :language: c

=======================
C++ Additional Features
=======================

Extending With EVSpace
======================

Including EVSpace
-----------------

If users wish to include the `EVSpace library <https://github.com/qbizzle68/evspace>`_
source in their extensions, there are additional C++ helper functions
that can be of use. The :file:`pyevspace-api.hpp` header will detect
if the  :file:`evspace.hpp` header has already been included, and if
so will make the functions documented on this page available. If you
do not want these functions defined in your source code, define the
:c:macro:`PYEVSPACE_NO_INCLUDE_HEADERS` macro before including the
:file:`pyevspace-api.hpp` header. If you do not include
:file:`evspace.hpp` before including the PyEVSpace API header, defining the
:c:macro:`PYEVSPACE_INCLUDE_HEADERS` before including the PyEVSpace header
will include :file:`evspace.hpp`, and make the helper functions available.
The EVSpace header is necessary when providing these functions because they
directly handle EVSpace types. To understand why these functions are necessary
to interface with the extension module with EVSpace types, read the
:doc:`C++ ABI compatibilitiy issues <compatibility>` page.

.. note::

    The C :c:type:`PyCapsule` is always available to import from the PyEVSpace
    module, and none of the above actions modify this. The functionality defined
    on this page is only for extension types compiled as a C++ project, because
    projects targeting C cannot include a C++ library like EVSpace, and so these
    functions must not be defined for them. Therefore, the contents of this page
    are only applicable when the :c:`__cplusplus` macro is defined, which is done
    automatically by compilers targeting C++.

When adding the EVSpace source code along with the PyEVSpace API header,
you will need to add these include directories to the compiler path:

* :file:`<path-to-pyevspace>/include/`
* :file:`<path-to-evspace>/include/`
* :file:`<path-to-evspace>/external/`

The PyEVSpace test suite includes a small extension module for testing the C
capsule and the functions documented here. `That file <https://github.com/qbizzle68/pyevspace/blob/master/tests/_test_capsule/capsule_consumer_module.cpp>`_
may serve as a reference for how to use these functions in practice, but note
the examples in the tests are trivial and only meant to ensure proper behavior
during tests.

Additional Features Documentation
=================================

Macros
------

.. c:macro:: PYEVSPACE_INCLUDE_HEADERS

    When defined before including :file:`pyevspace-api.hpp`, explicitly
    include the :file:`evspace.hpp` header and define the helper functions
    documented on this page.

.. c:macro:: PYEVSPACE_NO_INCLUDE_HEADERS

    When defined before including :file:`pyevspace-api.hpp`, **do not**
    include the :file:`evspace.hpp` header or define the helper functions
    documented on this page.

To EVSpace Conversions
----------------------

.. todo: add references to the evspace documentation when it is finished

.. cpp:function:: int PyEVSpace_ToVector(PyObject* obj, evspace::Vector& vector)

    Modify `vector` so that its internal state matches the state of a
    :py:type:`pyevspace.Vector` type. Return `0` on success, and `-1` on failure
    without modifying `vector`.

    :param obj: must be a :c:type:`PyObject`\* with the same type as :c:var:`PyEVSpace_CAPI.Vector_Type`
    :param vector: the vector instance to set equal to `obj`
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. cpp:function:: int PyEVSpace_ToMatrix(PyObject* obj, evspace::Matrix& matrix)

    Modify `matrix` so that its internal state matches the state of a
    :py:type:`pyevspace.Matrix` type. Return `0` on success, and `-1` on failure
    without modifying `matrix`.

    :param obj: must be a :c:type:`PyObject`\* with the same type as :c:var:`PyEVSpace_CAPI.Matrix_Type`
    :param matrix: the matrix instance to set equal to `obj`
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. cpp:function:: int PyEVSpace_ToAngles(PyObject* obj, evspace::EulerAngles& angles)

    Modify `angles` so that its internal state matches the state of a
    :py:type:`pyevspace.EulerAngles` type. Return `0` on success, and `-1` on failure
    without modifying `angles`.

    :param obj: must be a :c:type:`PyObject`\* with the same type as :c:var:`PyEVSpace_CAPI.EulerAngles_Type`
    :param angles: the angles instance to set equal to `obj`
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

To :c:type:`PyObject`\* Conversions
-----------------------------------

.. cpp:function:: PyObject* PyEVSpaceVector_ToObject(evspace::Vector vector)

    Creates a :py:type:`pyevspace.Vector` object as a :c:type:`PyObject`\*,
    specifically a :c:var:`PyEVSpace_CAPI.Vector_Type`, whose internal state
    is equal to the internal state of `vector`. This function returns a *new
    reference* owned by the caller, or NULL on error.

    :param vector: the vector whose state is used when creating the PyObject
    :return: a new reference to a :c:var:`Vector_Type` as a PyObject*
    :retval NULL: on failure with an appropriate exception set

.. cpp:function:: PyObject* PyEVSpaceMatrix_ToObject(evspace::Matrix matrix)

    Creates a :py:type:`pyevspace.Matrix` object as a :c:type:`PyObject`\*,
    specifically a :c:var:`PyEVSpace_CAPI.Matrix_Type`, whose internal state
    is equal to the internal state of `matrix`. This function returns a *new
    reference* owned by the caller, or NULL on error.

    :param matrix: the matrix whose state is used when creating the PyObject
    :return: a new reference to a :c:var:`Matrix_Type` as a PyObject*
    :retval NULL: on failure with an appropriate exception set

.. cpp:function:: PyObject* PyEVSpaceAngles_ToObject(evspace::EulerAngles angles)

    Creates a :py:type:`pyevspace.EulerAngles` object as a :c:type:`PyObject`\*,
    specifically a :c:var:`PyEVSpace_CAPI.EulerAngles_Type`, whose internal state
    is equal to the internal state of `angles`. This function returns a *new
    reference* owned by the caller, or NULL on error.

    :param angles: the angles whose state is used when creating the PyObject
    :return: a new reference to a :c:var:`EulerAngles_Type` as a PyObject*
    :retval NULL: on failure with an appropriate exception set

ReferenceFrame Components
-------------------------

.. cpp:function:: int PyEVSpaceFrame_ToAngles(PyObject* obj, evspace::EulerAngles& angles)

    Retrieve the rotation angles state from a :py:type:`pyevspace.ReferenceFrame` object
    as a :c:type:`PyObject`\*, by setting the state of `angles` equal to the angles state in `obj`.
    Returns `0` on success, `-1` on failure.

    :param obj: must be a PyObject* with the same type as :c:var:`PyEVSpace_CAPI.ReferenceFrame_Type`
    :param angles: output variable to set equal to the `obj` angles state
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. cpp:function:: int PyEVSpaceFrame_ToOffset(PyObject* obj, evspace::Vector& offset)

    Retrive the rotation order state from a :py:type:`pyevspace.ReferenceFrame` object
    as a :c:type:`PyObject`\*, by setting the state of `offset` equal to the offset state in `obj`.
    
    .. note::

        The internal value for :py:attr:`pyevspace.ReferenceFrame.offset` may
        be :py:data:`None` when no offset is given during construction. Since
        the state must be returned as a :c:`evspace::Vector` instance,
        there is no clean way of signaling when the value is :py:data:`None`.
        Since this value is mathematically analogous to the zero vector, when
        the offset is :py:data:`None`, all `offset` components will be set to zero.

    This function returns `0` on success, `-1` on failure.
        
    :param obj: must be a PyObject* with the same type as :c:var:`PyEVSpace_CAPI.ReferenceFrame_Type`
    :param offset: output variable to set equal to the `obj` offset state
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. cpp:function:: int PyEVSpaceFrame_SetAngles(PyObject* obj, const evspace::EulerAngles& angles)

    Set the internal Euler angles state of a :py:type:`pyevspace.ReferenceFrame`
    object as a :c:type:`PyObject`\* (`obj`) from a :c:`evspace::EulerAngles` object.
    Returns `0` on success, `-1` on failure.

    :param obj: must be a PyObject* with the same type as :c:var:`PyEVSpace_CAPI.ReferenceFrame_Type`
    :param angles: angles variable whose state the `obj` angles state will be set to
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. cpp:function:: int PyEVSpaceFrame_SetOffset(PyObject* obj, const evspace::Vector& offset)

    Set the internal offset vector state of a :py:type:`pyevspace.ReferenceFrame`
    object as a :c:type:`PyObject`\* (`obj`) from a :c:`evspace::Vector` object. Returns
    `0` on success, `-1` on failure.

    .. note::

        There is no clean way to set the internal offset value equal to :c:var:`Py_None`.
        This is mathematically analogous to the zero vector, so all components of `offset`
        should be set to `0.0` in this case. Keep in mind, if setting the
        :py:attr:`ReferenceFrame.offset` vector using this method, there may be no offset
        applied, but the returned value is not :py:data:`None` as it would be when setting
        to :py:data:`None` during construction.

    This function returns `0` on success, `-1` on failure.

    :param obj: must be a PyObject* with the same type as :c:var:`PyEVSpace_CAPI.ReferenceFrame_Type`
    :param offset: offset variable whose state the `obj` offset state will be set to
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set
