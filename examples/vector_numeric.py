import sys
sys.path.append(r'..\pyevspace\bin\x64')
from pyevspace import *

vec1 = EVector(1, 2, 3)
vec2 = EVector(3, 4, 5)

# addition
ans = vec1 + vec2
print("addition\n", ans)
ans += vec1
print("inplace addition\n", ans)

# subtraction
ans = vec2 - vec1
print("subtraction\n", ans)
ans -= vec1
print("inplace subtraction\n", ans)

# scalar multiplication
ans = vec1 * 2.5
print("scalar multiplication\n", ans)
ans *= 0.5
print("inplace scalar multiplication\n", ans)

# scalar division
ans = vec2 / 0.5
print("scalar division\n", ans)
ans /= 3.5
print("inplace scalar division\n", ans)
