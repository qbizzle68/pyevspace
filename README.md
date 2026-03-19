# PyEVSpace 0.15.0

[![Test](https://github.com/qbizzle68/pyevspace/actions/workflows/ci.yml/badge.svg)](https://github.com/qbizzle68/pyevspace/actions/workflows/ci.yml)
[![PyPi](https://img.shields.io/pypi/v/pyevspace?style=plastic)](https://pypi.org/project/pyevspace/#history)
![Supported Python versions](https://img.shields.io/pypi/pyversions/pyevspace?style=plastic)
[![License](https://img.shields.io/pypi/l/pyevspace?style-plastic)](https://opensource.org/licenses/MIT)

PyEVSpace is a Python Euclidean vector space package containing types
and methods for representing vector quantites and fasilitating rotating
them between reference frames. PyEVSpace is optimized for 3-dimensional
space only, and ideal for projects and apps modeling real world systems.


## Documentation

The full documentation of this project with both Python and C APIs can
be found [here](https://qbizzle68/pyevspace).

## Install

The python module can be installed with
```python
pip install pyevspace
```

Alternatively the repository can be downloaded or cloned using:
```bash
git clone https://github.com/qbizzle68/pyevspace.git
```

And installed with
```bash
pip install .
```

## Usage

The package defines a `Vector` and `Matrix` type, that behave like
containers representing those types.

```python
import pyevspace as evs

vector1 = evs.Vector(1, 2, 3)
vector2 = evs.Vector(4, 5, 6)

result = vector1 + vector2
result = 1.5 * vector1
dot_product = evs.vector_dot(vector1, vector2)
```

Matrices are used for rotations:

```python
matrix = evs.compute_rotation_matrix(math.pi / 2, evs.X_AXIS)
vector = evs.Vector(1, 2, 3)
rotated_vector = vector @ matrix
```

But a suite of functions can apply the rotations for you:

```python
vector = evs.Vector(1, 2, 3)
matrix = evs.compute_rotation_matrix(math.pi / 2, evs.Y_AXIS)
rotated_vector = evs.rotate_to(matrix, vector)

# or avoid computing the matrix yourself
rotated_vector = evs.rotate_to(math.pi / 2, evs.Y_AXIS, vector)
```

More complex rotations like Euler rotations are supported as well:

```python
vector = evs.Vector(1, 2, 3)
angles = evs.EulerAngles(1, 2, 3)
rotated_vector = rotate_from(evs.XYZ, angles, vector)
```

Checkout the [API reference](https://qbizzle.com/pyevspace/html/reference/index.html)
to learn more!

## License
[MIT](https://choosealicense.com/licenses/mit/)