from pyevspace import EVector, EMatrix

"""Here is some example code for the arithmetic capabilities of vectors and matrices
of the pyevspace module. Running this file from the command line will print similar
code to the console as well."""

def example():
    # initializing
    vector1 = EVector((1, 2, 3))
    vector2 = EVector((4, 5, 6))
    matrix1 = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    matrix2 = EMatrix((1, 2, 3), (1, 2, 3), (1, 2, 3))

    # addition and subtraction
    # both vectors and matrices can be added or subtracted to their own types.
    # each operation adds or subtracts component wise
    vectorAdd = vector1 + vector2
    matrixAdd = matrix1 + matrix2
    vectorSub = vector1 - vector2
    matrixSub = matrix1 - matrix2
    # vectors and matrices can only add their own type to themselves
    badVectorAdd = vector1 + matrix1  # raises TypeError
    badMatrixAdd = matrix1 + 1  # raises TypeError

    # negative
    # both types can be negated as well, which negates each component of the vector or matrix
    vectorNeg = -vector1  # vectorNeg == EVector((-1, -2, -3))

    # scalar multiplication and division
    # both vectors and matrices support scalar multiplication
    vectorMult = vector1 * 2
    matrixMult = matrix1 * 3.14
    # both types also support scalar division
    vectorDiv = vector1 / 10
    # the scalar must be a numeric type (i.e. a float or provide __float__() or __index__())

    # inplace operators
    # both types also support inplace operators for addition, subtraction and
    # scalar multiplication and division

    # matrix multiplication
    # the matrix class supports left-hand multiplication of matrices on
    # other matrices and vectors
    newMatrix = matrix1 * matrix2
    newVector = matrix1 * vector1
    # note the vectors must be on the right of the matrix
    badVector = vector1 * matrix1  # this will cause a type error

    # comparison
    # both types support equality and inequality operators for their respective types
    vecEq = vector1 == vector2
    vecNe = vector1 != vector2
    matEq = matrix1 == matrix2
    matNe = matrix1 != matrix2


if __name__ == '__main__':
    string = """
# initializing
>>> vector1 = EVector((1, 2, 3))
>>> vector2 = EVector((4, 5, 6))
>>> matrix1 = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
>>> matrix2 = EMatrix((1, 2, 3), (1, 2, 3), (1, 2, 3))

# addition and subtraction
# both vectors and matrices can be added or subtracted to their own types.
# each operation adds or subtracts component wise
>>> vectorAdd = vector1 + vector2
>>> matrixAdd = matrix1 + matrix2
>>> vectorSub = vector1 - vector2
>>> matrixSub = matrix1 - matrix2
# vectors and matrices can only add their own type to themselves
>>> badVectorAdd = vector1 + matrix1  # raises TypeError
>>> badMatrixAdd = matrix1 + 1  # raises TypeError

# negative
# both types can be negated as well, which negates each component of the vector or matrix
>>> vectorNeg = -vector1  # vectorNeg == EVector((-1, -2, -3))

# scalar multiplication and division
# both vectors and matrices support scalar multiplication
>>> vectorMult = vector1 * 2
>>> matrixMult = matrix1 * 3.14
# both types also support scalar division
>>> vectorDiv = vector1 / 10
# the scalar must be a numeric type (i.e. a float or provide __float__() or __index__())

# inplace operators
# both types also support inplace operators for addition, subtraction and
# scalar multiplication and division

# matrix multiplication
# the matrix class supports left-hand multiplication of matrices on
# other matrices and vectors
>>> newMatrix = matrix1 * matrix2
>>> newVector = matrix1 * vector1
# note the vectors must be on the right of the matrix
>>> badVector = vector1 * matrix1  # this will cause a type error

# comparison
# both types support equality and inequality operators for their respective types
>>> vecEq = vector1 == vector2
>>> vecNe = vector1 != vector2
>>> matEq = matrix1 == matrix2
>>> matNe = matrix1 != matrix2"""
    
    print(string)
