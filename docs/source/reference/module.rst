====================
Module API Reference
====================

.. currentmodule:: pyevspace

OverView
========

The PyEVSpace module has a very simple structure to understand. The types
and functions are divided into their respective API references that are
organized based on their specific purpose. The :doc:`Vector </reference/vector>`
and :doc:`Matrix </reference/matrix>` API's describe how the respective types
can be used, and you'll find they are very intuitive to understand if you
already have some knowledge of vector spaces.

The :doc:`rotation API </reference/rotation>` describes all of the global
constants and functions that are used to support rotation of vectors. There
are two other types, the :class:`EulerAngles` and :class:`RotationOrder` types
explained in the rotation API that facilitate defining reference frames for
these rotations.

Finally, the :doc:`ReferenceFrame API </reference/refframe>` describes how
:class:`ReferenceFrame` objects can encapsulate rotations from the
:doc:`rotation API </reference/rotation>` which can help make your code
cleaner and easier to maintain.

Sub-interpreter Support
=======================

.. versionadded:: 0.16.0
    This module now supports multi-phase initialization and sub-interpreter support.

The module uses `multi-phase initialization <https://docs.python.org/3.13/c-api/module.html#multi-phase-initialization>`_
when being imported. What this means is when an interpreter imports the module,
a new module object (and the objects defining the modules types) is created.
Without this, a new interpreter within the same process from, for example
:mod:`concurrent.interpreters` (only available since Python 3.14), can be used to
spawn a new sub-interpreter within the same process. Global changes made to
the module on a single-phase initialized module would appear in both interpreters.
A major feature of sub-interpreters being isolation, this isn't a very good
consequence, and multi-phase initialization protects against this. To illustrate:

.. code-block:: python

    >>> import pyevspace
    >>> from concurrent.interpreters import create
    >>>
    >>> def run():
    ...     import pyevspace
    ...     return id(pyevspace)
    ...
    >>> interp = create()
    >>> result = interp.call(run)
    >>> print(result == id(pyevspace))
    False

.. note::
    If experimenting with the above example, don't forget
    :meth:`concurrent.interpreters.Interpreter.close`.

The imported modules are not the same object, and changes to one (like modifying
:attr:`__dict__` for example) will not appear in the other interpreter. This is
also true for the types of the module:

.. code-block:: python

    >>> import pyevspace
    >>> from concurrent.interpreters import create
    >>>
    >>> def run():
    ...     import pyevspace
    ...     pyevspace.Vector.foo = 1
    ...
    >>> interp = create()
    >>> interp.call(run)
    >>> print(pyevspace.Vector.foo)
    AttributeError: type object 'pyevspace.Vector' has no attribute 'foo'

If the import mechanics are not important to you then there's no need to worry about
it. The above information is provided for those that may need to know,
but the rest of the module behaves the same regardless of single or multi-phase
initialization.
