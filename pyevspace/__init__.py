"""Module for a 3-dimensional Euclidean vector space with a vector and matrix type as well as necessary methods to use 
them."""

try:
    from _pyevspace import *
except ImportError:
    print("unable to import extension module '_pyevspace' (ignore if building extension)")
