from copy import copy
import math
from math import sqrt, pi, sin, cos
import pickle

import pytest

from pyevspace import (
    XYZ, EulerAngles, ReferenceFrame, Vector, Matrix, compute_rotation_matrix,
    rotate_between, rotate_from, rotate_to, vector_dot, vector_cross,
    vector_angle, vector_exclude, vector_proj
)
from .common import DummyIndex
from .inherited_types import DerivedMatrix, DerivedVector


def advance_ulps(x: float, n: int, direction: float) -> float:
    """Advance x by n ULPs towards direction."""
    y = x
    for _ in range(n):
        y = math.nextafter(y, direction)
    return y


class VectorValues:
    def __init__(self):
        self.v111 = Vector(1, 1, 1)
        self.v123 = Vector(1, 2, 3)
        self.v123n = Vector(-1, -2, -3)
        self.v34 = Vector(3, 4, 0)


@pytest.fixture
def vector_values() -> VectorValues:
    return VectorValues()


def test_vector_new() -> None:
    _ = Vector()
    _ = Vector(1, 2, 3)
    _ = Vector((1, 2, 3))
    _ = Vector(i for i in range(1, 4))

    with pytest.raises(TypeError):
        Vector(1, 2)

    with pytest.raises(ValueError):
        Vector((1, 2))

    with pytest.raises(TypeError):
        Vector(1, 2, 3, 4)

    with pytest.raises(ValueError):
        Vector((1, 2, 3, 4))

    with pytest.raises(TypeError):
        Vector('a', 2, 3)

    with pytest.raises(TypeError):
        Vector((1, 'b', 3))

    assert Vector() == Vector(0, 0, 0)
    assert Vector(0, 1, 2) == Vector({0: 'a', 1: 'b', 2: 'c'})


def test_vector_str_methods(vector_values):
    assert str(vector_values.v123) == '[1, 2, 3]'
    assert repr(vector_values.v123) == 'pyevspace.Vector(1, 2, 3)'

    v = Vector(1.1, 2.2, 3.3)
    assert str(v) == '[1.1, 2.2, 3.3]'
    assert repr(v) == 'pyevspace.Vector(1.1, 2.2, 3.3)'

    v = Vector(123456.1234, 1234567.891, 0.123456)
    assert str(v) == '[123456, 1.23457e+06, 0.123456]'
    assert repr(v) == 'pyevspace.Vector(123456, 1.23457e+06, 0.123456)'

    v = Vector(0.0001234567, 0.00001234567, 0.000001234567)
    assert str(v) == '[0.000123457, 1.23457e-05, 1.23457e-06]'
    assert repr(v) == 'pyevspace.Vector(0.000123457, 1.23457e-05, 1.23457e-06)'


def test_vector_iter(vector_values):
    vector_iter = iter(vector_values.v123)
    n = next(vector_iter)
    assert n == 1.0
    n = next(vector_iter)
    assert n == 2.0
    n = next(vector_iter)
    assert n == 3.0

    with pytest.raises(StopIteration):
        n = next(vector_iter)

    # Test with generation
    list_ = [i for i in vector_values.v123]
    assert list_ == [1.0, 2.0, 3.0]

    # Test 'in' operator
    assert 1 in vector_values.v123
    assert 0 not in vector_values.v123


def test_vector_add(vector_values):
    v = vector_values.v111 + vector_values.v123
    assert v == Vector(2, 3, 4)
    v = vector_values.v111 + vector_values.v123n
    assert v == Vector(0, -1, -2)

    with pytest.raises(TypeError):
        vector_values.v111 + 1

    with pytest.raises(TypeError):
        vector_values.v111 + 1.0

    with pytest.raises(TypeError):
        vector_values.v111 + 'a'


def test_vector_iadd(vector_values):
    v = Vector(1, 1, 1)
    v += vector_values.v123
    assert v == Vector(2, 3, 4)
    v = Vector(-1, -2, -3)
    v += vector_values.v123
    assert v == Vector(0, 0, 0)

    v = Vector(1, 2, 3)
    v += v
    assert v == Vector(2, 4, 6)

    with pytest.raises(TypeError):
        vector_values.v123 += 1

    with pytest.raises(TypeError):
        vector_values.v123 += 1.0

    with pytest.raises(TypeError):
        vector_values.v123 += 'a'

    with pytest.raises(TypeError):
        vector_values.v123 += [1, 2, 3]


def test_vector_subtract(vector_values: VectorValues) -> None:
    v = vector_values.v123 - vector_values.v111
    assert v == Vector(0, 1, 2)
    v = vector_values.v111 - vector_values.v123
    assert v == Vector(0, -1, -2)

    v = Vector(1, 2, 3)
    v -= v
    assert v == Vector(0, 0, 0)

    with pytest.raises(TypeError):
        vector_values.v111 - 1

    with pytest.raises(TypeError):
        vector_values.v111 - 1.0

    with pytest.raises(TypeError):
        vector_values.v111 - 'a'

    with pytest.raises(TypeError):
        vector_values.v111 - [1, 2, 3]


def test_vector_isubtract(vector_values: VectorValues) -> None:
    v = Vector(1, 1, 1)
    v -= vector_values.v123
    assert v == Vector(0, -1, -2)
    v = Vector(1, 1, 1)
    v -= vector_values.v123n
    assert v == Vector(2, 3, 4)

    with pytest.raises(TypeError):
        vector_values.v111 -= 1

    with pytest.raises(TypeError):
        vector_values.v111 -= 1.0

    with pytest.raises(TypeError):
        vector_values.v111 -= 'a'

    with pytest.raises(TypeError):
        vector_values.v111 -= [1, 2, 3]


def test_vector_multiply(vector_values: VectorValues) -> None:
    v = vector_values.v123 * 2
    assert v == Vector(2, 4, 6)

    v = vector_values.v123n * 0.5
    assert v == Vector(-0.5, -1, -1.5)

    # __rmul__
    v = 1.5 * vector_values.v123
    assert v == Vector(1.5, 3, 4.5)

    mat = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    v = vector_values.v123 @ mat
    assert v == Vector(30, 36, 42)

    with pytest.raises(TypeError):
        vector_values.v111 * 'a'

    with pytest.raises(TypeError):
        vector_values.v111 * Vector()

    with pytest.raises(TypeError):
        vector_values.v123 @ 4

    with pytest.raises(TypeError):
        2 @ vector_values.v123


def test_vector_imultiply(vector_values: VectorValues) -> None:
    v = Vector(1, 2, 3)
    v *= 2
    assert v == Vector(2, 4, 6)

    v = Vector(-1, -2, -3)
    v *= 0.5
    assert v == Vector(-0.5, -1, -1.5)

    mat = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    v = Vector(1, 2, 3)
    v @= mat
    assert v == Vector(30, 36, 42)

    with pytest.raises(TypeError):
        vector_values.v111 *= 'a'

    with pytest.raises(TypeError):
        vector_values.v123 *= vector_values.v111

    with pytest.raises(TypeError):
        vector_values.v123 @= 1


def test_vector_divide(vector_values: VectorValues) -> None:
    v = vector_values.v123 / 1
    assert v == Vector(1, 2, 3)

    v = vector_values.v123 / 0.5
    assert v == Vector(2, 4, 6)

    with pytest.raises(TypeError):
        vector_values.v123 /= 'a'

    with pytest.raises(TypeError):
        vector_values.v123 /= vector_values.v111


def test_vector_idivide(vector_values: VectorValues) -> None:
    v = Vector(1, 2, 3)
    v /= 2
    assert v == Vector(0.5, 1, 1.5)

    v = Vector(-1, -1, -1)
    v /= 0.5
    assert v == Vector(-2, -2, -2)

    with pytest.raises(TypeError):
        vector_values.v111 /= 'a'

    with pytest.raises(TypeError):
        vector_values.v111 /= vector_values.v123


def test_vector_mag(vector_values: VectorValues) -> None:
    v = Vector(3, 4, 0)
    assert v.magnitude() == 5

    v = Vector(-3, 4, 0)
    assert v.magnitude() == 5

    x = sqrt(1/3)
    v = Vector(x, x, x)
    assert v.magnitude() == pytest.approx(1)

    with pytest.raises(TypeError):
        vector_values.v111.magnitude(1)


def test_vector_mag_squared(vector_values: VectorValues) -> None:
    assert vector_values.v123.magnitude_squared() == 14
    assert vector_values.v123n.magnitude_squared() == 14

    with pytest.raises(TypeError):
        vector_values.v123.magnitude_squared(1)


def test_vector_normalize(vector_values: VectorValues) -> None:
    x = sqrt(1/3)
    v = Vector(x, x, x)
    v_norm = v.norm()
    answer = Vector(x, x, x)
    assert v_norm == pytest.approx(answer)

    v.normalize()
    assert v == pytest.approx(answer)

    v = Vector(3, 4, 0)
    v_norm = v.norm()
    answer = Vector(0.6, 0.8, 0)
    assert v_norm == pytest.approx(answer)

    v.normalize()
    assert v == pytest.approx(Vector(0.6, 0.8, 0))

    with pytest.raises(TypeError):
        vector_values.v111.normalize(1)

    with pytest.raises(TypeError):
        vector_values.v111.norm(0)


def test_vector_sequence(vector_values: VectorValues) -> None:
    assert len(vector_values.v111) == 3
    assert vector_values.v123[0] == 1
    assert vector_values.v123[1] == 2
    assert vector_values.v123[2] == 3
    assert vector_values.v123[-1] == 3
    assert vector_values.v123[-2] == 2
    assert vector_values.v123[-3] == 1

    # todo: test slice arg with __getitem__

    with pytest.raises(IndexError):
        vector_values.v111[3]

    with pytest.raises(IndexError):
        vector_values.v111[-4]

    with pytest.raises(TypeError):
        vector_values.v111['a']

    v = Vector(1, 2, 3)
    v[0] = 5
    v[1] = 6
    v[2] = 7
    assert v == Vector(5, 6, 7)

    v[-1] = 5
    v[-2] = 4
    v[-3] = 3
    assert v == Vector(3, 4, 5)

    with pytest.raises(IndexError):
        vector_values.v111[3] = 5

    with pytest.raises(IndexError):
        vector_values.v111[-4] = 5

    with pytest.raises(TypeError):
        vector_values.v111[0] = 'a'

    # __index__() support
    idx = DummyIndex(0)
    assert vector_values.v123[idx] == 1
    idx.value = -1
    assert vector_values.v123[idx] == 3
    idx.value = 3
    with pytest.raises(IndexError):
        vector_values.v123[idx]
    idx.value = -4
    with pytest.raises(IndexError):
        vector_values.v123[idx]


def test_vector_compare_ulp(vector_values: VectorValues) -> None:
    # trivial tests
    v = Vector(1, 2, 3)
    assert v.compare_to_ulp(vector_values.v123, 10)

    # advanced tests
    lhs = v
    rhs = Vector(1, 2, 3)

    # near zero
    lhs[0] = +0.0
    rhs[0] = -0.0
    assert lhs == rhs
    assert lhs.compare_to_ulp(rhs, 10)

    lhs[0] = 0.0
    rhs[0] = advance_ulps(0.0, 1, 1.0)
    assert lhs.compare_to_ulp(rhs, 1)

    rhs[0] = advance_ulps(0.0, 1, -1.0)
    assert lhs.compare_to_ulp(rhs, 2)

    rhs[0] = advance_ulps(0.0, 20, 1.0)
    assert lhs.compare_to_ulp(rhs, 20)
    assert lhs.compare_to_ulp(rhs, 19) is False

    # Around 1.0
    lhs[0] = rhs[0] = 1.0
    lhs[1] = 1.0
    rhs[1] = advance_ulps(1.0, 1, 2.0)
    assert lhs.compare_to_ulp(rhs, 1)

    rhs[1] = advance_ulps(1.0, 1, 0.0)
    assert lhs.compare_to_ulp(rhs, 1)

    rhs[1] = advance_ulps(1.0, 10, 2.0)
    assert lhs.compare_to_ulp(rhs, 10)

    rhs[1] = advance_ulps(1.0, 11, 2.0)
    assert lhs.compare_to_ulp(rhs, 11)

    lhs[1] = -1.0
    rhs[1] = advance_ulps(-1.0, 10, 0.0)
    assert lhs.compare_to_ulp(rhs, 10)

    rhs[1] = advance_ulps(-1.0, 11, 0.0)
    assert lhs.compare_to_ulp(rhs, 11)

    # medium magnitude
    lhs[1] = rhs[1] = 2.0
    lhs[2] = 1e6
    rhs[2] = advance_ulps(1e6, 10, math.inf)
    assert lhs.compare_to_ulp(rhs, 9) is False
    assert lhs.compare_to_ulp(rhs, 10)

    # large magnitude
    lhs[2] = 1e200
    rhs[2] = advance_ulps(1e200, 10, math.inf)
    assert lhs.compare_to_ulp(rhs, 10)
    assert lhs.compare_to_ulp(rhs, 9) is False

    # infinities and NaNs
    lhs[2] = math.inf
    rhs[2] = math.inf
    assert lhs.compare_to_ulp(rhs, 1)

    rhs[2] = -math.inf
    assert lhs.compare_to_ulp(rhs, 9999) is False

    lhs[2] = 3.0
    rhs[2] = math.nan
    assert lhs.compare_to_ulp(rhs, 9999) is False

    # exceptions
    with pytest.raises(TypeError):
        lhs.compare_to_ulp(rhs, 1.5)

    with pytest.raises(ValueError):
        lhs.compare_to_ulp(rhs, -1)


def test_vector_compare(vector_values: VectorValues) -> None:
    lhs = Vector(1, 2, 3)
    rhs = Vector(1, 2, 3)

    lhs == rhs
    assert lhs.compare_to_tol(rhs)

    lhs[0] = 0.0
    rhs[0] = 1e-15
    assert lhs == rhs
    assert lhs.compare_to_tol(rhs)

    lhs[0] = 1e-16
    rhs[0] = 2e-16
    assert lhs == rhs

    lhs[0] = -1e-16
    rhs[0] = 1e-16
    assert lhs == rhs

    lhs[0] = 0.0
    rhs[0] = 1e-14
    assert lhs != rhs
    assert lhs.compare_to_tol(rhs, 1e-9, 1e-14)

    lhs[0] = rhs[0] = 1.0
    lhs[1] = 1e10
    rhs[1] = 1e10 + 1.0     # relative error ~1e-10
    assert lhs == rhs

    lhs[1] = 1e20
    rhs[1] = 1e20 * 1.00000000001    # relative error ~1e-10
    assert lhs == rhs
    assert lhs.compare_to_tol(rhs, 1e-10, 1e-15)

    lhs[1] = 1e10
    rhs[1] = 1e10 + 100     # relative error ~1e-8
    assert lhs != rhs
    assert lhs.compare_to_tol(rhs, 1e-8, 1e-15)

    lhs[1] = 1e15
    rhs[1] = 1e15 * 1.000001    # relative error ~1e-5
    assert (lhs == rhs) is False
    assert lhs.compare_to_tol(rhs, 1e-5, 1e-15)

    # different magnitude scales (testing the sum formulation)
    lhs[1] = rhs[1] = 2.0
    lhs[2] = 1.0
    rhs[2] = 1.0 + 1e-10
    assert lhs == rhs

    lhs[2] = 1e-5
    rhs[2] = 1e-5 + 1e-15
    assert lhs == rhs

    lhs[2] = 1.0
    rhs[2] = 2.0
    assert lhs != rhs

    lhs[2] = 1e6
    rhs[2] = 2e6
    assert lhs != rhs

    # sign handling
    lhs[2] = rhs[2] = 3.0
    lhs[0] = 1.0
    rhs[0] = -1.0
    assert lhs != rhs

    lhs[0] = 1e-10
    rhs[0] = -1e-10
    assert lhs != rhs

    lhs[0] = 0.0
    rhs[0] = -0.0
    assert lhs == rhs

    lhs[0] = -1.0
    rhs[0] = -1.0 - 1e-10
    assert lhs == rhs

    lhs[0] = -1e10
    rhs[0] = -1e10 - 1.0
    assert lhs == rhs

    # transition region (where abs and rel solerances are similar)
    # for abs_tol = 1e-15 and rel_tol = 1e-9 this is around 1e-6
    transition = 1e-6
    lhs[0] = transition
    rhs[0] = transition + 1e-15
    assert lhs == rhs

    rhs[0] = transition + 1e-16
    assert lhs == rhs

    lhs[0] = transition
    rhs[0] = transition + 1e-10
    assert lhs.compare_to_tol(rhs, 1e-5, 1e-10)

    # scientific computing cases
    lhs[0] = sin(pi / 4.0)
    rhs[0] = cos(pi / 4.0)
    assert lhs == rhs

    lhs[0] = sin(pi / 6.0)
    rhs[0] = 0.5
    assert lhs == rhs

    lhs[0] = (sin(pi / 3.0) ** 2) + (cos(pi / 3.0) ** 2)
    rhs[0] = 1.0
    assert lhs == rhs

    # special values
    lhs[0] = math.inf
    rhs[0] = math.inf
    assert lhs == rhs

    rhs[0] = -math.inf
    assert lhs != rhs

    rhs[0] = 1e308
    assert lhs != rhs

    lhs[0] = 1.0
    rhs[0] = math.inf
    assert lhs != rhs

    lhs[0] = math.nan
    rhs[0] = math.nan
    assert lhs != rhs
    assert lhs.compare_to_tol(rhs, 0.0, 0.0) is False

    rhs[0] = 0.0
    assert lhs != rhs

    # boundary testing (just outside tolerance)
    lhs[0] = 1.0
    rhs[0] = 1.0 + 0.99e-9
    assert lhs == rhs

    rhs[0] = 1.0 + 1.01e-9
    assert lhs != rhs

    lhs[0] = 0.0
    rhs[0] = 0.99e-15
    assert lhs == rhs

    rhs[0] = 1.01e-15
    assert lhs != rhs
    assert lhs.compare_to_tol(rhs, 1e-9, 9.99e-14)

    # non-transitivity demonstration
    # this behaves as expected, not an error. mostly for showing it
    # does exist: might have a==b and b==c but not a==c
    lhs[0] = 0.0
    rhs[0] = 5e-16
    assert lhs == rhs

    lhs[0] = 1.01e-15
    assert lhs == rhs

    rhs[0] = 0.0
    assert lhs != rhs

    # exceptions
    with pytest.raises(TypeError):
        lhs.compare_to_tol(rhs, 'a', 0.1)

    with pytest.raises(TypeError):
        lhs.compare_to_tol(rhs, 0.1, [])

    with pytest.raises(ValueError):
        lhs.compare_to_tol(rhs, -0.1, 0.1)

    with pytest.raises(ValueError):
        lhs.compare_to_tol(rhs, 0.1, -0.1)


def test_vector_buffer() -> None:
    v = Vector(1, 2, 3)
    view = memoryview(v)
    assert view.strides == (8,)
    assert view.shape == (3,)
    assert view.ndim == 1
    assert view.format == 'd'
    assert view.nbytes == 24
    assert view.obj is v
    assert view.itemsize == 8
    assert view.c_contiguous is True

    assert view.tolist() == [1, 2, 3]

    view[2] = 5
    assert v[2] == 5

    with pytest.raises(IndexError):
        view[3]

    with pytest.raises(TypeError):
        view[0] = 'a'


def test_vector_pickle(vector_values: VectorValues) -> None:
    buffer = pickle.dumps(vector_values.v123)
    v = pickle.loads(buffer)
    assert v == vector_values.v123

    cpy = copy(vector_values.v123)
    assert cpy == vector_values.v123
    assert cpy is not vector_values.v123


def test_vector_dot(vector_values: VectorValues) -> None:
    dot = vector_dot(vector_values.v123, vector_values.v34)
    assert dot == 11

    dot = vector_dot(vector_values.v123, vector_values.v123)
    mag_squared = vector_values.v123.magnitude_squared()
    assert dot == mag_squared

    with pytest.raises(TypeError):
        vector_dot(vector_values.v123)

    with pytest.raises(TypeError):
        vector_dot(vector_values.v123, vector_values.v123, vector_values.v123)

    with pytest.raises(TypeError):
        vector_dot(0, vector_values.v123)

    with pytest.raises(TypeError):
        vector_dot(vector_values.v123, 1.0)


def test_vector_cross(vector_values: VectorValues) -> None:
    cross = vector_cross(vector_values.v123, vector_values.v34)
    assert cross == Vector(-12, 9, -2)

    cross = vector_cross(vector_values.v34, vector_values.v123)
    assert cross == Vector(12, -9, 2)

    with pytest.raises(TypeError):
        vector_cross(vector_values.v123)

    with pytest.raises(TypeError):
        vector_cross(vector_values.v123, vector_values.v123,
                     vector_values.v123)

    with pytest.raises(TypeError):
        vector_cross(0, vector_values.v123)

    with pytest.raises(TypeError):
        vector_cross(vector_values.v123, 1.0)


def test_vector_angle() -> None:
    v1 = Vector(1, 1, 0)
    v2 = Vector(1, 0, 0)
    assert vector_angle(v1, v2) == pytest.approx(pi / 4.0)

    v1 = Vector(0, 1, 0)
    assert vector_angle(v1, v2) == pytest.approx(pi / 2.0)
    assert vector_angle(v2, v1) == pytest.approx(pi / 2.0)

    with pytest.raises(TypeError):
        vector_angle(v1)

    with pytest.raises(TypeError):
        vector_angle(v1, v2, v1)

    with pytest.raises(TypeError):
        vector_angle(2, v1)

    with pytest.raises(TypeError):
        vector_angle(v1, 0.0)


def test_vector_exclude() -> None:
    v1 = Vector(1, 1, 0)
    v2 = Vector(0, 1, 0)
    assert vector_exclude(v1, v2) == Vector(1, 0, 0)

    v1 = Vector(1, 2, 0)
    assert vector_exclude(v1, v2) == Vector(1, 0, 0)

    with pytest.raises(TypeError):
        vector_exclude(v1)

    with pytest.raises(TypeError):
        vector_exclude(v1, v2, v2)

    with pytest.raises(TypeError):
        vector_exclude(1.0, v1)

    with pytest.raises(TypeError):
        vector_exclude(v2, 0)


def test_vector_projection() -> None:
    v1 = Vector(1, 1, 7)
    v2 = Vector(1, 1, 1)
    assert vector_proj(v1, v2) == Vector(3, 3, 3)
    assert vector_proj(Vector.E1, Vector.E2) == Vector(0, 0, 0)

    with pytest.raises(TypeError):
        vector_proj(v1)

    with pytest.raises(TypeError):
        vector_proj(v2, v1, v2)

    with pytest.raises(TypeError):
        vector_proj(v1, 1.5)

    with pytest.raises(TypeError):
        vector_proj(0, v2)


def test_vector_inheritance() -> None:
    derived = DerivedVector(1, 2, 3)

    assert type(derived) is DerivedVector
    assert issubclass(DerivedVector, Vector)
    assert isinstance(derived, Vector)

    assert list(derived) == [1, 2, 3]

    # Test new return types are constructed
    assert derived + derived
    assert derived - derived
    assert derived * 1.5
    assert derived / 1.5
    assert derived @ DerivedMatrix()
    assert -derived

    # Test global methods evaluate and don't raise
    assert vector_dot(derived, derived)
    assert vector_cross(derived, DerivedVector.E1)
    assert vector_exclude(derived, derived)
    assert vector_proj(derived, derived)
    assert vector_angle(derived, DerivedVector.E1)
    assert rotate_to(Matrix.IDENTITY, derived, offset=derived)
    assert rotate_to(1.5, derived, derived, offset=derived)
    assert rotate_from(Matrix.IDENTITY, derived, offset=derived)
    assert rotate_from(1.5, derived, derived, offset=derived)
    assert rotate_between(XYZ, EulerAngles(), XYZ, EulerAngles(), derived,
                          offset_from=derived, offset_to=derived)
    assert compute_rotation_matrix(1.5, derived)
    frame = ReferenceFrame(XYZ, EulerAngles(), offset=derived)
    assert frame
    assert frame.rotate_to(derived)
    assert frame.rotate_to(frame, derived)
    assert frame.rotate_from(derived)
    assert frame.rotate_from(frame, derived)
    frame.offset = derived
    assert type(frame.offset) is DerivedVector

    # Test repr
    assert repr(derived).startswith('tests.inherited_types')

    # Test __new__()
    obj = Vector.__new__(DerivedVector)
    obj.__init__(derived)

    assert type(derived) is DerivedVector
    assert obj == derived

    # Test __dict__ support
    assert derived.foo
