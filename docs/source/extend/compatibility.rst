=====================
C++ ABI Compatibility
=====================

Compatibility Issues
--------------------

Because the C++ ABI is not guaranteed across builds due to differences
such as varying compilers and compiler flags, the underlying C++ types
found within PyEVSpace cannot be exposed to consumers of the extension
via a C :c:type:`PyCapsule`. Most packages are installed via wheels from
`PyPi <https://pypi.org>`_ that are built using different toolchains.
These binaries are not guaranteed to have compiled the underlying C++
objects the same way, and they become incompatible at the ABI level.

PyEVSpace is a wrapper library around `EVSpace <https://github.com/qbizzle68/pyevspace.git>`_,
a C++ Euclidian vector space library almost identical to PyEVSpace,
but written in C++. This makes providing a capsule for others writing
extension modules a bit difficult.

Luckily for us, the C ABI is much more stable, and doesn't have the same
compatibility issues C++ gives us. Because of this, the :doc:`C capsule <capsule>`
provided by PyEVSpace leverages C primitives to provide several accessor/modifier
functions for managing the internal state of the underlying C++ types. This
is required anyway for users writing extensions in C, as they couldn't
provide prototypes of the EVSpace library anyway, but it also helps solve the
compatibility issues.

Additional C++ Features
-----------------------

.. todo: provide .inv files when docs for evspace are done

For C++ users willing to include the EVSpace headers within their extension,
there are also some :doc:`C++ features <features>` that can be enabled from the
:file:`include/pyevspace-api.hpp` header. These are functions that accept EVSpace
types (e.g. :cpp:expr:`evspace::Vector`), and interface with the capsule directly to
simplify your codebase and replace numerous capsule function calls. These
functions act as a facade between the PyEVSpace's compiled EVSpace types, and
the EVSpace types that will be compiled in your extension module, as they only
communicate across the Python boundary using the stable C ABI, avoiding any
C++ ABI incompatibility.
