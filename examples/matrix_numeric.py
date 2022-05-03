import sys
sys.path.append(r'..\pyevspace\bin\x64')
from pyevspace import *

vec1 = EVector(1, 2, 3)
vec2 = EVector(3, 4, 5)
vec3 = EVector(7, 6, 5)

mat1 = EMatrix(vec1, vec2, vec3)
mat2 = EMatrix(vec2, vec1, vec3)

# addition
ans = mat1 + mat2
print("addition\n", ans)
ans += mat1
print("inplace addition\n", ans)

# subtraction
ans = mat2 - mat1
print("subtraction\n", ans)
ans -= mat1
print("inplace subtraction\n", ans)

# scalar multiplication
ans = mat1 * 2.5
print("scalar multiplication\n", ans)
ans *= 0.5
print("inplace scalar multiplication\n", ans)

# matrix multiplication
ans = mat1 @ mat2
print("matrix multiplcation\n", ans)
ans @= mat1
print("inplace matrix multiplication\n", ans)

# vector multiplication
ans = mat1 @ vec1
print("vector multiplation\n", ans)

# scalar division
ans = mat2 / 0.5
print("scalar division\n", ans)
ans /= 3.5
print("inplace scalar division\n", ans)
