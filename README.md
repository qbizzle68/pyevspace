# PyEVSpace 0.14.1

This version of PyEVSpace is no functionally different than 0.14.0.
This version was published to pypi using automated scripting workflows,
and its existence means everything works!!

PyEVSpace is a Python Euclidean vector space package containing types
and methods for representing vector quantites and fasilitating rotating
them between reference frames. PyEVSpace is designed for 3-dimensional
space only, which allows for optimum speed since size checks do not
occur.


## Documentation

The full documentation of this project with both Python and C APIs can
be found [here](https://qbizzle68.github.io/pyevspace/html/index.html).

## Install

The python module can be installed with
```python
pip install pyevspace
```

Alternatively the repository can be downloaded or cloned using:
```bash
git clone https://github.com/qbizzle68/pyevspace.git
```
It can be used as is within Visual Studio, or built inplace using the
*setup.py* if needed.

## Usage

To use the module simply import the pyevspace module into your project:
```python
import pyevspace as evs
from math import pi

vec = evs.Vector(1, 2, 3)

rotatedVec = evs.rotateAxisTo(evs.X_AXIS, pi/2)
```

Matrices can be created from iterables, where each iterable represents
a row of the matrix
```python
import pyevspace as evs

mat = evs.Matrix((0, 0, 1), (0, -1, 0), (1, 0, 0))

rotatedVec = evs.rotateMatrixFrom(mat, Vector(1, 1, 1))
```

The Order and Angles types can be used to create an Euler rotation 
matrix. All twelve Euler rotations are already defined in the module,
so you shouldn't need to instantiate an Order object. The Angles
object holds the angles for each rotation in the Euler rotation, in
the order of the axis rotations (in radians).
```python
import pyevspace as evs

angs = Angles(1.1, 4.5, 3.14)
mat = getMatrixEuler(XYZ, angs)

rotatedVec = mat * Vector(1, 0, 2)
```

There are many methods that handle the rotations for you, check the
official documentation to learn more about them.

## Examples

### Examples of numeric operators
```python
v1 = Vector(1, 2, 3)
v2 = Vector(4, 5, 6)

print(v1 * 2)
# prints [2, 4, 6]

print(v1 + v2)
# prints [5, 7, 9]

print(v1 - v2)
# prints [-3, -3, -3]
```

### Examples of vector and matrix operators
```python
v1 = Vector(1, 2, 3)
v2 = Vector(4, 5, 6)
m1 = Matrix(Vector(4, 2, 3), Vector(8, 5, 2), Vector(4, 2, 1))

print(dot(v1, v2))
# prints 32.0

print(cross(v1, v2))
# prints [ -3.00000, 6.00000, -3.00000 ]

print(det(m1))
# prints -8.0

print(transpose(m1))
# prints 
# ([4, 2, 3],
# [8, 5, 2],
# [4, 2, 1])
```

## License
[MIT](https://choosealicense.com/licenses/mit/)