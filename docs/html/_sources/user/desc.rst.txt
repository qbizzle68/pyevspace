PyEVSpace User Guide
====================

What is PyEVSpace?
------------------
PyEVSpace is a Python Euclidean vector space package containing types and
methods for representing vector quantites and facilitating rotations
between reference frames. PyEVSpace is designed for 3-dimensional space
only, which allows for optimum speed since size checks to not occur.

Vector Space
------------
A vector space is a set whose elements (vectors) obey certain 
`axioms <https://en.wikipedia.org/wiki/Vector_space#Definition_and_basic_properties>`_
like addition and scaling. The components of vectors are usually elements
of a field, namely the complex or real numbers. Vector spaces can be 
varrying dimensions, which generally is the number of *directions*
associated with the vector space.

Euclidean Vector Space
----------------------
A Euclidean vector space is a vector space intended to represent physical
space. Mathematically it is essentially identical to general vector spaces,
they only differ conceptually. For example, the infinte dimensional
vector space of polynomials is used to solve linear systems of equations,
whereas a Euclidean vector space is a finite vector space used to model 
physical space with Euclidean vectors, whose length and direction are meant
to represent physical quantities with magnitude and direction like a velocity 
or force.

A Python Euclidean Vector Space
-------------------------------
PyEVSpace is simply a Euclidean vector space implemented for Python.
Its 3-dimensions are meant to represent physical quantites of our
world that have both a length and a direction. These can be as simple
as a position of something or a more nuanced value like angular momentum.
Only 3-dimensional vectors are supported because PyEVSpace is designed
with real world physical principles in mind. By limiting the dimensionality
of the vector space, a significant performance can be gained by eliminating
size checking for every single operation.

Why Use PyEVSpace?
------------------
PyEVSpace is a compiled library written in C, and is optimized with
performance in mind. This in combination with the fixed dimension
of the vector space means the library performs extremely fast. This 
makes PyEVSpace the perfect package to provide the framework for
your Python project, or a simple tool to glue together an existing
project with its small footprint. The interface is simple enough
for the basic user to understand, but also offers more advanced
functionality like providing memory buffers to the underlying blocks
of memory for the vectors and matrices.

To learn more checkout the API documentation and the examples section
for relavent examples.