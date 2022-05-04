# PyEVSpace

PyEVSpace is a Python Euclidean Vector Space module containing Euclidean vector and matrix types. 
The module is created by extending C code with the Python/C API to ensure the best performance and runs faster than
the same code implemented directly in python.
The python types are designed to be used to represent physical vectors and rotations between reference frames, 
however they can be utilized in other ways if so desired.

## Install

The python module can be installed with
```python
pip install pyevspace
```

Alternatively the repository can be downloaded or cloned using:
```bash
git clone https://github.com/qbizzle68/pyevspace.git
```
and built there using:
```bash
py setup.py build
```
This is only recommended if editing the source code and testing its functionality. Most users should install using `pip`.

## Usage

To use the module simply import the pyevspace module into your project:
```python
from pyevspace import *

# create vector
vec = EVector(1, 2, 3)

# create matrix from column vectors
mat = EMatrix( EVector(1, 3, 2), EVector(7, 4, 3), EVector(8, 4, 2) )

# rotate vector
rotated = mat @ vec
```
The module only contains two types and a handful of module level methods, so it is probably necessary to import everything.
However if you don't wish to overpopulate the namespace you can import using an alias:
```python
import pyevspace as evs

vec = evs.EVector()
```

## Examples

### Examples of numeric operators
```python
v1 = EVector(1, 2, 3)
v2 = EVector(4, 5, 6)

print(v1 * 2)
# prints [ 2.00000, 4.00000, 6.00000 ]

print(v1 + v2)
# prints [ 5.00000, 7.00000, 9.00000 ]

print(v1 - v2)
# prints [-3.00000, -3.00000, -3.00000 ]
```

### Examples of vector and matrix operators
```python
v1 = EVector(1, 2, 3)
v2 = EVector(4, 5, 6)
m1 = EMatrix( EVector(4, 2, 3), EVector(8, 5, 2), EVector(4, 2, 1) )

print( dot(v1, v2) )
# prints 32.0

print( cross(v1, v2) )
# prints [ -3.00000, 6.00000, -3.00000 ]

print( det(m1) )
# prints -8.0

print( transpose(m1) )
# prints 
# [ 4.00000, 2.00000, 3.00000 ]
# [ 8.00000, 5.00000, 2.00000 ]
# [ 4.00000, 2.00000, 1.00000 ]
```

## Methods and operators
`EVector` operators:
- `+` addition (other `EVector`)
- `+=` inplace addition (other `EVector`)
- `-` subtraction (other `EVector`)
- `-=` inplace subtraction (other `EVector`)
- `*` multiplication (floats or ints)
- `*=` inplace multiplication (floats or ints)
- `/` division (floats or ints)
- `/=` inplace division (floats or ints)

`EVector` methods:
- `mag()` magnitude of a vector
- `mag2()` square of the magnitude of a vector
- `normalize()` normalizes a vector
- `[0]` gets the first comonent of a vector
- `[1] = 4` sets the second comonent of a vector to 4

`EMatrix` operators:
- `+` addition (other `EMatrix`)
- `+=` inplace addition (other `EMatrix`)
- `-` subtraction (other `EMatrix`)
- `-=` inplace subtraction (other `EMatrix`)
- `*` multiplication (floats or ints)
- `*=` inplace multiplication (floats or ints)
- `@` matrix multiplication (other `EMatrix` or `EVector`)
- `@=` inplace matrix multiplication (other `EMatrix` or `EVector`)
- `/` division (floats or ints)
- `/=` inplace division (floats or ints)

`EMatrix` methods:
- `get(row, column)` gets a component of a matrix
- `set(row, column, value)` sets a comonent of a matrix to a value

module level methods:
- `dot(EVector, EVector)` vector dot product
- `cross(EVector, EVector)` vector cross product
- `norm(EVector)` returns a normalized vector
- `vang(EVector, EVector)` computes the angle between two vectors
- `vxcl(EVector, EVector)` computes a vector exculded from another
- `det(EMatrix)` computes the determinate of a matrix
- `transpose(EMatrix)` computes the transpose of a matrix

## License
[MIT](https://choosealicense.com/licenses/mit/)
