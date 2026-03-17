=============================
Using PyEVSpace In Extensions
=============================

PyEVSpace is a CPython extension implemented in C++ and wraps the
`EVSpace C++ library <https://github.com/qbizzle68/evspace.git>`_.
Because of C++ ABI compatibility issues, the PyEVSpace C++ API cannot
be exposed directly via a PyCapsule. Because of this, the Python type structs
are only accessible or modifiable in specific ways. This section will describe
how to do this, and how you can use PyEVSpace within your own CPython
extensions (whether in C or C++).

.. toctree::
    :maxdepth: 2

    C++ ABI Compatibility <compatibility>
    C API Capsule <capsule>
    C++ Extension Features <features>
