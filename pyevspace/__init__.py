"""Module for a 3-dimensional Euclidean vector space with a vector and matrix type as well as necessary methods to use them.

"""

try:
    from _pyevspace import *
    _extension_import = True
except ImportError:
    _extension_import = False

__version__ = '0.0.12.1'