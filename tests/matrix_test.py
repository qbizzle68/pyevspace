from copy import copy
import pickle
import math

import pytest

from pyevspace import Matrix, Vector, rotate_from, rotate_to
from .inherited_types import DerivedMatrix, DerivedVector
from .common import DummyIndex


def advance_ulps(x: float, n: int, direction: float) -> float:
    """Advance x by n ULPs towards direction."""
    for _ in range(n):
        x = math.nextafter(x, direction)
    return x


class MatrixValues:
    def __init__(self):
        self.mi = Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1))
        self.m123 = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        self.m147 = Matrix((1, 4, 7), (2, 5, 8), (3, 6, 9))
        self.v123 = Vector(1, 2, 3)
        self.m2 = Matrix((2, 4, 6), (8, 10, 12), (14, 16, 18))


@pytest.fixture()
def matrix_values():
    return MatrixValues()


def test_matrix_construction(subtests, matrix_values: MatrixValues):
    m = Matrix.__new__(Matrix)
    for i in range(3):
        for j in range(3):
            with subtests.test(i=i, j=j):
                assert m[i, j] == 0

    m = Matrix()
    for i in range(3):
        for j in range(3):
            with subtests.test(i=i, j=j):
                assert m[i, j] == 0

    for i in range(3):
        for j in range(3):
            value = (i * 3) + j + 1
            with subtests.test(i=i, j=j):
                assert matrix_values.m123[i, j] == value

    with pytest.raises(TypeError):
        Matrix(1, (1, 2, 3), (4, 5, 6))

    with pytest.raises(ValueError):
        Matrix((1, 2), (3, 4, 5), (6, 7, 8))

    with pytest.raises(ValueError):
        Matrix((1, 2, 3), (4, 5, 6, 7), (8, 9, 10))

    with pytest.raises(TypeError):
        Matrix((1, 2, 'a'), (3, 4, 5), (6, 7, 8))


def test_matrix_strings(matrix_values: MatrixValues) -> None:
    assert str(matrix_values.m123) == '[[1, 2, 3]\n [4, 5, 6]\n [7, 8, 9]]'
    assert (repr(matrix_values.m123) ==
            'pyevspace.Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))')

    m = Matrix((1.1, 2.2, 3.3), (4.4, 5.5, 6.6), (7.7, 8.8, 9.9))
    assert str(m) == '[[1.1, 2.2, 3.3]\n [4.4, 5.5, 6.6]\n [7.7, 8.8, 9.9]]'
    assert (repr(m) == 'pyevspace.Matrix((1.1, 2.2, 3.3), (4.4, 5.5, 6.6),'
                       ' (7.7, 8.8, 9.9))')


def test_matrix_add(matrix_values: MatrixValues) -> None:
    ans = Matrix((2, 6, 10), (6, 10, 14), (10, 14, 18))
    assert matrix_values.m123 + matrix_values.m147 == ans

    with pytest.raises(TypeError):
        matrix_values.m123 + 1

    with pytest.raises(TypeError):
        matrix_values.m123 + 'a'

    with pytest.raises(TypeError):
        matrix_values.m123 + [1, 2, 3]

    with pytest.raises(TypeError):
        1.0 + matrix_values


def test_matrix_iadd(matrix_values: MatrixValues) -> None:
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m += matrix_values.m147
    assert m == Matrix((2, 6, 10), (6, 10, 14), (10, 14, 18))

    with pytest.raises(TypeError):
        matrix_values.m123 += 1

    with pytest.raises(TypeError):
        matrix_values.m123 += 'a'

    with pytest.raises(TypeError):
        matrix_values.m123 += (1, 2, 3)


def test_matrix_subtract(matrix_values: MatrixValues) -> None:
    m = matrix_values.m123 - matrix_values.m147
    assert m == Matrix((0, -2, -4), (2, 0, -2), (4, 2, 0))

    with pytest.raises(TypeError):
        matrix_values.m123 - 1

    with pytest.raises(TypeError):
        1 - matrix_values.m123

    with pytest.raises(TypeError):
        matrix_values.m123 - (1, 2, 3)


def test_matrix_isubtract(matrix_values: MatrixValues) -> None:
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m -= matrix_values.m147
    assert m == Matrix((0, -2, -4), (2, 0, -2), (4, 2, 0))

    with pytest.raises(TypeError):
        matrix_values.m123 -= 1

    with pytest.raises(TypeError):
        matrix_values.m123 -= (1, 2, 3)

    with pytest.raises(TypeError):
        x = 1
        x -= matrix_values.m123


def test_matrix_multiply(matrix_values: MatrixValues) -> None:
    assert matrix_values.m123 * 2 == matrix_values.m2
    assert matrix_values.m123 * 2.0 == matrix_values.m2
    assert 2 * matrix_values.m123 == matrix_values.m2

    assert matrix_values.m123 @ matrix_values.v123 == Vector(14, 32, 50)

    answer = Matrix((14, 32, 50), (32, 77, 122), (50, 122, 194))
    assert matrix_values.m123 @ matrix_values.m147 == answer

    with pytest.raises(TypeError):
        matrix_values.m123 * (1, 2, 3)

    with pytest.raises(TypeError):
        matrix_values.m123 * 'a'

    with pytest.raises(TypeError):
        matrix_values.m123 @ 1

    with pytest.raises(TypeError):
        1 @ matrix_values.m123


def test_matrix_imultiply(matrix_values: MatrixValues) -> None:
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m *= 2
    assert m == matrix_values.m2

    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m @= matrix_values.m147
    answer = Matrix((14, 32, 50), (32, 77, 122), (50, 122, 194))
    assert m == answer

    # Important test to ensure matrix data isn't touched until
    # multiplication is complete
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m @= m
    answer = Matrix((30, 36, 42), (66, 81, 96), (102, 126, 150))
    assert m == answer

    with pytest.raises(TypeError):
        matrix_values.m123 *= matrix_values.m123

    with pytest.raises(TypeError):
        matrix_values.m123 @= 1.0

    with pytest.raises(TypeError):
        matrix_values.m123 @= matrix_values.v123


def test_matrix_divide(matrix_values: MatrixValues) -> None:
    answer = Matrix((0.5, 1, 1.5), (2, 2.5, 3), (3.5, 4, 4.5))
    assert matrix_values.m123 / 2 == answer

    assert matrix_values.m123 / 0.5 == matrix_values.m2

    with pytest.raises(TypeError):
        matrix_values.m123 / matrix_values.v123

    with pytest.raises(TypeError):
        matrix_values.m123 / 'a'

    with pytest.raises(TypeError):
        x = 5
        x / matrix_values.m123


def test_matrix_idivide(matrix_values: MatrixValues) -> None:
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    answer = Matrix((0.5, 1, 1.5), (2, 2.5, 3), (3.5, 4, 4.5))
    m /= 2
    assert m == answer

    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m /= 0.5
    assert m == matrix_values.m2

    with pytest.raises(TypeError):
        matrix_values.m123 /= matrix_values.m147

    with pytest.raises(TypeError):
        matrix_values.m123 /= [1, 2, 3]


def test_matrix_get(matrix_values: MatrixValues) -> None:
    assert matrix_values.m123[0, 0] == 1
    assert matrix_values.m123[1, 2] == 6
    assert matrix_values.m123[-1, 1] == 8
    assert matrix_values.m123[2, -2] == 8
    assert matrix_values.m123[-3, -3] == 1

    with pytest.raises(TypeError):
        matrix_values.m123['a', 0]

    with pytest.raises(TypeError):
        matrix_values.m123(0, [1])

    with pytest.raises(IndexError):
        matrix_values.m123[0, 3]

    with pytest.raises(IndexError):
        matrix_values.m123[4, 1]

    with pytest.raises(IndexError):
        matrix_values.m123[2, -4]

    with pytest.raises(IndexError):
        matrix_values.m123[-5, 0]

    with pytest.raises(IndexError):
        matrix_values.m123[-4, -4]

    # Test MatrixView returned memoryviews
    view = matrix_values.m123[0]
    assert view.strides == (8,)
    assert view.ndim == 1
    assert view.shape == (3,)
    assert view.format == 'd'
    assert view.tolist() == [1, 2, 3]

    view = matrix_values.m123[-1]
    assert view.strides == (8,)
    assert view.ndim == 1
    assert view.shape == (3,)
    assert view.format == 'd'
    assert view.tolist() == [7, 8, 9]

    view = matrix_values.m123[0:2, 1]
    assert view.strides == (24,)
    assert view.ndim == 1
    assert view.shape == (2,)
    assert view.format == 'd'
    assert view.tolist() == [2, 5]

    view = matrix_values.m123[0, 1:3]
    assert view.strides == (8,)
    assert view.ndim == 1
    assert view.shape == (2,)
    assert view.format == 'd'
    assert view.tolist() == [2, 3]

    # zero length views
    view = matrix_values.m123[0:2, 0:2]
    assert view.strides == (24, 8)
    assert view.ndim == 2
    assert view.shape == (2, 2)
    assert view.format == 'd'
    assert view.tolist() == [[1, 2], [4, 5]]

    view = matrix_values.m123[0, 1:1]
    assert view.strides == (8,)
    assert view.ndim == 1
    assert view.shape == (0,)
    assert view.format == 'd'
    assert view.tolist() == []

    view = matrix_values.m123[0:0, 1]
    assert view.strides == (8,)
    assert view.ndim == 1
    assert view.shape == (0,)
    assert view.format == 'd'
    assert view.tolist() == []

    view = matrix_values.m123[0:0, 1:1]
    assert view.strides == (8,)
    assert view.ndim == 1
    assert view.shape == (0,)
    assert view.format == 'd'
    assert view.tolist() == []

    # __index__ support
    idx = DummyIndex(0)
    assert matrix_values.m123[idx].tolist() == [1, 2, 3]
    idx.value = -1
    assert matrix_values.m123[idx].tolist() == [7, 8, 9]
    idx.value = 3
    with pytest.raises(IndexError):
        matrix_values.m123[idx]
    idx.value = -4
    with pytest.raises(IndexError):
        matrix_values.m123[idx]

    idx.value = 0
    assert matrix_values.m123[idx, idx] == 1.0
    assert matrix_values.m123[idx, 0:1].tolist() == [1.0]
    assert matrix_values.m123[0:1, idx].tolist() == [1.0]

    # overflow errors
    idx.value = 1 << 65
    with pytest.raises(OverflowError):
        matrix_values.m123[idx]
    with pytest.raises(OverflowError):
        matrix_values.m123[1 << 65]


def test_matrix_set() -> None:
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m[0, 0] = 10
    assert m[0, 0] == 10

    m[-1, 2] = 11
    assert m[2, 2] == 11

    m[1, -2] = 12
    assert m[1, 1] == 12

    m[-2, -1] = 13
    assert m[1, 2] == 13

    with pytest.raises(TypeError):
        m['a', 2] = 1

    with pytest.raises(TypeError):
        m[1, [3]] = 1

    with pytest.raises(IndexError):
        m[3, 0] = 5

    with pytest.raises(IndexError):
        m[1, 4] = 6

    with pytest.raises(IndexError):
        m[-4, 2] = 7

    with pytest.raises(IndexError):
        m[0, -5] = 8

    # todo: add slice assignment when implemented


def test_matrix_compare(matrix_values: MatrixValues) -> None:
    # trivial comparisons
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    assert m == matrix_values.m123
    assert m != matrix_values.m147
    assert m.compare_to_ulp(matrix_values.m123, 10)
    assert m.compare_to_ulp(Matrix(), 10) is False

    m = Matrix((2, 4, 6), (8, 10, 12), (14, 16, 18))
    m /= 2.0
    assert m == matrix_values.m123
    assert m != matrix_values.m147
    assert m.compare_to_ulp(matrix_values.m123, 10)

    # Vector extensively tests the advanced component comparisons
    # only need to check a few here to ensure matrix also uses
    # ULP based mechanics

    lhs = m
    rhs = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    lhs[0, 0] = 1.0
    rhs[0, 0] = advance_ulps(1.0, 1, 2.0)
    assert lhs == rhs

    rhs[0, 0] = advance_ulps(1.0, 1, 0.0)
    assert lhs == rhs
    assert lhs.compare_to_ulp(rhs, 1)

    rhs[0, 0] = math.inf
    assert lhs != rhs
    assert lhs.compare_to_ulp(rhs, 11) is False

    lhs[0, 0] = math.inf
    assert lhs == rhs

    lhs[0, 0] = -math.inf
    assert lhs != rhs

    lhs[0, 0] = 1.0
    rhs[0, 0] = math.nan
    assert lhs != rhs

    rhs[0, 0] = math.sin(math.pi / 4.0)
    lhs[0, 0] = math.cos(math.pi / 4.0)
    lhs.compare_to_tol(rhs)

    lhs[0, 0] = math.sin(math.pi / 6.0)
    rhs[0, 0] = 0.5
    lhs.compare_to_tol(rhs)


def test_matrix_view(matrix_values: MatrixValues) -> None:
    view = memoryview(matrix_values.m123)
    assert view.ndim == 2
    assert view.shape == (3, 3)
    assert view.strides == (24, 8)
    assert view.nbytes == 72
    assert view.format == 'd'
    assert len(view) == 3
    assert view.tolist() == [[1, 2, 3], [4, 5, 6], [7, 8, 9]]

    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    view = memoryview(m)
    m[1, 1] = 10
    assert view[1, 1] == 10
    view[0, 0] = 11
    assert m[0, 0] == 11


def test_matrix_pickle(matrix_values: MatrixValues) -> None:
    buffer = pickle.dumps(matrix_values.m123)
    m = pickle.loads(buffer)
    assert m == matrix_values.m123

    cpy = copy(matrix_values.m123)
    assert cpy == matrix_values.m123
    assert cpy is not matrix_values.m123


def test_matrix_determinate(matrix_values: MatrixValues) -> None:
    assert matrix_values.m123.determinate() == 0

    m = Matrix((2, 6, 4), (7, 3, 1), (8, 0, 0))
    assert m.determinate() == -48

    with pytest.raises(TypeError):
        m.determinate(0)


def test_matrix_transpose(matrix_values: MatrixValues) -> None:
    assert matrix_values.m123.transpose() == matrix_values.m147

    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m.transpose_inplace()
    assert m == matrix_values.m147

    with pytest.raises(TypeError):
        m.transpose(0)

    with pytest.raises(TypeError):
        m.transpose_inplace('a')


def test_matrix_inheritance() -> None:
    derived = DerivedMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    derived_vector = DerivedVector()

    assert type(derived) is DerivedMatrix
    assert isinstance(derived, Matrix)
    assert issubclass(DerivedMatrix, Matrix)

    assert list(derived[0]) == [1, 2, 3]
    assert list(derived[1]) == [4, 5, 6]
    assert list(derived[2]) == [7, 8, 9]

    # Test new return types
    assert derived + derived
    assert derived - derived
    assert -derived
    assert derived * 1.5
    assert derived / 1.5
    assert derived @ derived_vector
    assert derived @ derived
    assert derived.compare_to_tol(derived)
    assert derived.compare_to_ulp(derived, 10)
    assert derived.transpose()
    non_singular = DerivedMatrix((1, 0, 0), (0, 1, 0), (0, 0, 1))
    assert non_singular.inverse()
    assert derived.determinate() == 0.0

    # Test global methods
    assert rotate_from(derived, Vector.E1)
    assert rotate_to(derived, Vector.E2)

    # Test repr
    assert repr(derived).startswith('tests.inherited_types')

    # test __new__
    obj = DerivedMatrix.__new__(DerivedMatrix)
    obj.__init__((1, 2, 3), (4, 5, 6), (7, 8, 9))

    assert type(obj) is DerivedMatrix
    assert obj == derived

    # Test __dict__ support
    assert derived.foo
