"""A Python 3-dimensional Euclidean vector space."""

import importlib.metadata

__version__ = importlib.metadata.version(__package__)

try:
    from .core import *
except ModuleNotFoundError:
    # todo: import pure python version here
    pass
