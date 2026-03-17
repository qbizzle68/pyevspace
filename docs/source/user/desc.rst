PyEVSpace User Guide
====================

What is PyEVSPace?
------------------
PyEVSpace is a 3-dimensional Euclidean vector space Python package that
can be used as a framework for apps using physical systems. Vectors describe
states of real world objects and can be transformed between useful reference
frames using PyEVSpace's suite of rotation functions. Users can generate
rotation matrices as linear operators to use themselves, or use simplified
rotation functions that abstract away complexities for those without a strong
knowledge of vector spaces. PyEVSpace also provides basic but useful linear
algebra functions for vector and matrix objects.

PyEVSpace is designed to be performant, and for this reason is implemented in
C++. As a basic framework for an application, this library should not be the
bottleneck for performance, and along with simplifing the underlying concepts
and being easy to use, this is the main goal of the project. To help achieve
this, PyEVSpace strictly only supports vectors and matrices in 3-dimensions.
This library is not for projects that need variable length vectors, arrays,
or matrices (such as commonly used libraries like `NumPy <https://numpy.org>`_)
and is in no way meant to replace them. PyEVSpace was also designed with
compatibility in mind, and defines protocols (like the buffer protocol) as well
as rich constructor support to interface cleanly with other array like libraries.

If this describes what your project needs, :doc:`install <install>` PyEVSpace
and checkout the :doc:`basic usage <usage>` to get started, or checkout the
:doc:`API reference </reference/index>` for more information about specific types
or functions.
