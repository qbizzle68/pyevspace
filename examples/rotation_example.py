import sys
sys.path.append(r'..\pyevspace\bin\x64')
from pyevspace import *
import math

''' This example is to show how the library can be used for rotations.
    The Rodrigues rotation formula shows how to rotate a vector around another vector
    and can be found at https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula '''
    
# rotate the x-axis 90 degrees around the z-axis, which will end up at the y-axis.
# basic formula is R = I + (sin(t))K + (1-cos(t))KK

theta = math.radians(90)

# set-up the K matrix
K = EMatrix()
K.set(0, 1, -1)
K.set(1, 0, 1)

# compute the first order term
term2 = K * math.sin(theta)
# compute the second order term
term3 = K @ K * (1 - math.cos(theta))

# compute the rotation matrix
R = EMatrix.I + term2 + term3

# rotate the x-axis vector
rotated_vector = R @ EVector(1, 0, 0)

# should print [ 0, 1, 0 ]
print(rotated_vector)
