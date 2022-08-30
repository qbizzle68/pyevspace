from pyevspace import *

vec1 = EVector(1, 2, 3)
vec2 = EVector(3, 4, 5)
vec3 = EVector(7, 6, 5)

mat1 = EMatrix([vec1, vec2, vec3])
mat2 = EMatrix([vec2, vec1, vec3])

print("mat1\n", mat1)
print("mat2\n", mat2)
print()

# addition
ans = mat1 + mat2
print("addition: mat1 + mat2\n", ans)
ans += mat1
print("inplace addition: ans += mat1\n", ans)

print()
# subtraction
ans = mat2 - mat1
print("subtraction: mat2 - mat1\n", ans)
ans -= mat1
print("inplace subtraction: ans -= mat1\n", ans)

print()
# scalar multiplication
ans = mat1 * 2.5
print("scalar multiplication: mat1 * 2.5\n", ans)
ans *= 0.5
print("inplace scalar multiplication: ans *= 0.5\n", ans)

print()
# matrix multiplication
ans = mat1 @ mat2
print("matrix multiplcation: mat1 @ mat2\n", ans)
ans @= mat1
print("inplace matrix multiplication: ans @= mat1\n", ans)

print()
# vector multiplication
ans = mat1 @ vec1
print("vector multiplation: mat1 @ vec1\n", ans)

print()
# scalar division
ans = mat2 / 0.5
print("scalar division: mat2 / 0.5\n", ans)
ans /= 3.5
print("inplace scalar division: ans /= 3.5\n", ans)
