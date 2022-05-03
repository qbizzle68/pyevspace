import sys
sys.path.append(r'..\pyevspace\bin\x64')
from pyevspace import *

vec1 = EVector(1, 2, 3)
vec2 = EVector(2, 3, 4)
vec3 = EVector(3, 9, 5)

mat1 = EMatrix(vec1, vec2, vec3)

# magnitude
x = vec1.mag()
print("magnitude\n", x)
x = vec1.mag2()
print("magnitude squared\n", x)

# norm
n = norm(vec1)
print("norm\n", n)
n = vec1 * 1
n.normalize()
print("normalized\n", n)

# dot
vdot = dot(vec1, vec2)
print("dot product\n", vdot)

# cross
vcross = cross(vec1, vec2)
print("cross product\n", vcross)

# vector angle (shortest angle between vectors)
angle = vang(vec1, vec2)
print("vector angle\n", angle)

# vector exclude (project a vector onto a plane normal to another vector)
vproj = vxcl(vec1, vec2)
print("vector exclude\n", vproj)

# matrix determinate
d = det(mat1)
print("determinate\n", d)

# matrix transpose (make rows to columns and columns to rows)
t = transpose(mat1)
print("transpose\n", t)
