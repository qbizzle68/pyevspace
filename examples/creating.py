import sys
sys.path.append(r'..\pyevspace\bin\x64')
from pyevspace import *

# create a vector
vec1 = EVector(1, 2, 3) # using ints
vec2 = EVector(1.1, 2.2, 3.3) # using floats
vec3 = EVector(1, 2.2, 3) # using both
print("vec1\n", vec1)
print("vec2\n", vec2)
print("vec3\n", vec3)

# create a matrix
mat1 = EMatrix(vec1, vec2, vec3) # with crated vectors
mat2 = EMatrix(vec1, vec2, EVector(2, 4, 6)) # with EVector initializer
print("mat1\n", mat1)
print("mat2\n", mat2)

# set components
vec1[0] = 10 # set the x component
print("new vec1\n", vec1)
vec1[2] = 34 # set the z component
print("new vec1\n", vec1)
vec1[-1] = 23 # also set the z component
print("new vec1\n", vec1)
mat1.set(0, 0, 12) # set first row, first column to 12
print("new mat1\n", mat1)

# get components
x = vec1[2] # get the z component
print("vec1[2] =", x) # prints 23
y = mat1.get(1, 2) # set the second row, third column
print("mat.get(1, 2) =", y) # prints 2.2
