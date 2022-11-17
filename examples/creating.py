from pyevspace import EVector, EMatrix

'''Here is some example code for creating EVector and EMatrix instances. Running
this file from the command line will print similar code to the console as well.'''

def example():
    # create a vector
    vector = EVector((1, 2, 3))
    # vectors are created from any iterable object with a length of exactly 3
    ls = [7, 8, 9]
    vector2 = EVector(ls)
    # if creating a vector from values, they must be wrapped in an iterable
    # this raises a TypeError exception !!!
    vector3 = EVector(1, 2, 3)  # bad constructor call
    # values also must be a numeric type. if they are not a float type then
    # the constructor will invoke __float__() if available, then __index__()
    # if still needed. otherwise a TypeError exception will be raised.
    vector4 = EVector('a', [0, 2], range(2))  # all of these values would cause
    #                                           a TypeError exception

    # creating a matrix
    matrix = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    # similarly to vectors, matrices are created from exactly 3 iterable objects, that each
    # have a length of exactly 3
    ls = [1, 2, 3]
    tp = (4, 5, 6)
    v = EVector((7, 8, 9))
    matrix2 = EMatrix(ls, tp, v)
    # similarly to a vector, a matrix values must be numeric, meaning they are a float type,
    # or they supply a __float__() or __index__() method (used in that order).

    # get / set components
    # both vectors and matrix elements can be accessed with the [] operator
    a = vector[0]  # returns 1
    b = vector2[-1]  # returns 9
    c = matrix[2, 1]  # return 8
    # both vector and matrix elements can be set with the [] operator
    vector[0] = 5
    vector2[-2] = 7
    matrix[1, 1] = 0
    # similar to construction, all vector and matrix elements must be numeric
    vector[2] = 'a'  # raises TypeError

    # vector as iterable
    # a vector also acts as an iterator and can be used anywhere an iterable can be used
    sumValue = 0
    for v in vector:
        sumValue += v
    # sum is now equal to vector[0] + vector[1] + vector[2]
    # the in operator can also be used to find if an element is in a vector
    result = 5 in vector  # result will be true if 5 is an element of the vector

    ignore = [vector3, vector4, matrix2, a, b, c, result]


if __name__ == '__main__':
    string = """
# create a vector
>>> vector = EVector((1, 2, 3))
# vectors are created from any iterable object with a length of exactly 3
>>> ls = [7, 8, 9]
>>> vector2 = EVector(ls)
# if creating a vector from values, they must be wrapped in an iterable
# this raises a TypeError exception !!!
>>> vector3 = EVector(1, 2, 3)  # bad constructor call
# values also must be a numeric type. if they are not a float type then
# the constructor will invoke __float__() if available, then __index__()
# if still needed. otherwise a TypeError exception will be raised.
>>> vector4 = EVector('a', [0, 2], range(2))  # all of these values would cause
#                                               a TypeError exception

# creating a matrix
>>> matrix = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
# similarly to vectors, matrices are created from any iterable object with a length of
# exactly 3, whose values are any iterable object with a length of exactly 3
>>> ls = [1, 2, 3]
>>> tp = (4, 5, 6)
>>> v = EVector((7, 8, 9))
>>> matrix2 = EMatrix((ls, tp, v))
# similarly to a vector, a matrix values must be numeric, meaning they are a float type,
# or they supply a __float__() or __index__() method (used in that order).

# get / set components
# both vectors and matrix elements can be accessed with the [] operator
>>> x = vector[0]  # returns 1
>>> x = vector2[-1]  # returns 9
>>> x = matrix[2, 1]  # return 8
# both vector and matrix elements can be set with the [] operator
>>> vector[0] = 5
>>> vector2[-2] = 7
>>> matrix[1, 1] = 0
# similar to construction, all vector and matrix elements must be numeric
>>> vector[2] = 'a'  # raises TypeError

# vector as iterable
# a vector also acts as an iterator and can be used anywhere an iterable can be used
>>> sumValue = 0
>>> for v in vector:
>>>     sumValue += v
# sum is now equal to vector[0] + vector[1] + vector[2]
# the in operator can also be used to find if an element is in a vector
>>> result = 5 in vector  # result will be true if 5 is an element of the vector"""

    print(string)
