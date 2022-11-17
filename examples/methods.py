from pyevspace import *

"""Here is some code to show what module methods are provided by the module.
Running this file from the command line will output similar code."""

def example():
    # initialize
    vector1 = EVector((1, 2, 3))
    vector2 = EVector((4, 5, 6))
    matrix1 = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))

    # dot product   dot(EVector, EVector) -> float
    # the dot() method computes the dot product of two vectors
    dotProduct = dot(vector1, vector2)  # dotProduct = 32

    # cross product     cross(EVector, EVector) -> EVector
    # the cross() method computes the cross product of two vectors
    # using a right-handed coordinate system.
    crossVector = cross(vector1, vector2)
    # to get the cross product in a left-handed coordinate frame
    # use the vector negative to invert the direction
    crossLeft = -cross(vector1, vector2)

    # normal vector     norm(EVector) -> EVector
    # the norm() method returns a normalized vector of its argument
    normalVector = norm(vector1)  # normalVector has magnitude 1
    # the EVector class also has a class method to normalize a vector in place
    vector3 = EVector((1, 2, 3))
    vector3.normalize()  # vector3 has magnitude 1

    # vector angle      vang(EVector, EVector) -> float
    # the vang() method computes the shortest angle between two vectors. for
    # two vectors in a plane, there are two angles between them, which add to 2pi.
    # the vang() methods always returns the shortest one. ensuring the shortest angle
    # is the angle you may need is project dependent and out of the scope of this example
    vectorAngle = vang(vector1, vector2)

    # vector exclude    vxcl(EVector, EVector) -> EVector
    # the vxcl() method returns a vector equal to one vector with all components of another
    # removed from it. in essence, it returns the projection of one vector onto a plane with
    # the other vector as its normal vector.
    vectorExclude = vxcl(EVector((1, 2, 3)), EVector((0, 1, 0)))  # vectorExclude = (1, 0, 3)

    # determinate   det(EMatrix) -> float
    # the det() method computes the determinate of a matrix, useful for determining if
    # a matrix is invertible
    determinate = det(matrix1)

    # transpose     transpose(EMatrix) -> EMatrix
    # the transpose() method returns a new matrix that is the transpose of its argument
    trans = transpose(matrix1)

    ignore = [dotProduct, crossVector, crossLeft, normalVector, vectorAngle, vectorExclude, determinate, trans]


if __name__ == '__main__':
    string = """
# initialize
>>> vector1 = EVector((1, 2, 3))
>>> vector2 = EVector((4, 5, 6))
>>> matrix1 = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))

# dot product   dot(EVector, EVector) -> float
# the dot() method computes the dot product of two vectors
>>> dotProduct = dot(vector1, vector2)  # dotProduct = 32

# cross product     cross(EVector, EVector) -> EVector
# the cross() method computes the cross product of two vectors
# using a right-handed coordinate system.
>>> crossVector = cross(vector1, vector2)
# to get the cross product in a left-handed coordinate frame
# use the vector negative to invert the direction
>>> crossLeft = -cross(vector1, vector2)

# normal vector     norm(EVector) -> EVector
# the norm() method returns a normalized vector of its argument
>>> normalVector = norm(vector1)  # normalVector has magnitude 1
# the EVector class also has a class method to normalize a vector in place
>>> vector3 = EVector((1, 2, 3))
>>> vector3.normalize()  # vector3 has magnitude 1

# vector angle      vang(EVector, EVector) -> float
# the vang() method computes the shortest angle between two vectors. for
# two vectors in a plane, there are two angles between them, which add to 2pi.
# the vang() methods always returns the shortest one. ensuring the shortest angle
# is the angle you may need is project dependent and out of the scope of this example
>>> vectorAngle = vang(vector1, vector2)

# vector exclude    vxcl(EVector, EVector) -> EVector
# the vxcl() method returns a vector equal to one vector with all components of another
# removed from it. in essence, it returns the projection of one vector onto a plane with
# the other vector as its normal vector.
>>> vectorExclude = vxcl(EVector((1, 2, 3)), EVector((0, 1, 0)))  # vectorExclude = (1, 0, 3)

# determinate   det(EMatrix) -> float
# the det() method computes the determinate of a matrix, useful for determining if
# a matrix is invertible
>>> determinate = det(matrix1)

# transpose     transpose(EMatrix) -> EMatrix
# the transpose() method returns a new matrix that is the transpose of its argument
>>> trans = transpose(matrix1)"""

    print(string)
