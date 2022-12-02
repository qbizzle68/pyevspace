.. _evector:

=====================================
A Euclidean vector (:class:`EVector`)
=====================================

A three-dimensional array object meant to represent a physical vector in 
a Euclidean vector space. The internal data buffer is a C type double, 
which supports Python :class:`float` and :class:`int` types. If the data
type is not a Python :class:`float`, the :class:`EVector` falls back to 
the :func:`__float__` and then :func:`__index__` methods if implemented.
