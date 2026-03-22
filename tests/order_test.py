from copy import copy
import pickle

import pytest

from pyevspace import (
    EulerAngles, ReferenceFrame, RotationOrder, XYZ, YZX, ZXZ, X_AXIS, Y_AXIS,
    Z_AXIS, Vector, compute_rotation_matrix, rotate_between, rotate_from,
    rotate_to
)
from .inherited_types import DerivedRotationOrder
from .common import DummyIndex


def test_order_new():
    order = RotationOrder(0, 1, 2)
    assert order.first == X_AXIS
    assert order.second == Y_AXIS
    assert order.third == Z_AXIS

    with pytest.raises(TypeError):
        RotationOrder(1.1, 1, 2)

    with pytest.raises(TypeError):
        RotationOrder(0, 'a', 2)

    with pytest.raises(TypeError):
        RotationOrder(0, 1, [])

    with pytest.raises(ValueError):
        RotationOrder(-1, 0, 1)

    with pytest.raises(ValueError):
        RotationOrder(0, 3, 1)

    with pytest.raises(ValueError):
        RotationOrder(1, 0, 4)


def test_order_str():
    s = str(XYZ)
    assert s == 'XYZ'
    s = str(YZX)
    assert s == 'YZX'
    s = str(ZXZ)
    assert s == 'ZXZ'


def test_order_repr():
    s = repr(XYZ)
    assert s == 'pyevspace.RotationOrder(pyevspace.X_AXIS, pyevspace.Y_AXIS,'\
                ' pyevspace.Z_AXIS)'
    s = repr(YZX)
    assert s == 'pyevspace.RotationOrder(pyevspace.Y_AXIS, pyevspace.Z_AXIS,'\
                ' pyevspace.X_AXIS)'
    s = repr(ZXZ)
    assert s == 'pyevspace.RotationOrder(pyevspace.Z_AXIS, pyevspace.X_AXIS,'\
                ' pyevspace.Z_AXIS)'


def test_order_get_set():
    assert XYZ[0] == 0
    assert XYZ[1] == 1
    assert XYZ[2] == 2
    assert ZXZ[0] == 2
    assert YZX[-1] == 0
    assert YZX[-2] == 2
    assert YZX[-3] == 1

    with pytest.raises(IndexError):
        XYZ[3]

    with pytest.raises(IndexError):
        XYZ[-4]

    with pytest.raises(TypeError):
        XYZ[0] = 1

    # __index__() support
    idx = DummyIndex(0)
    assert XYZ[idx] == 0
    idx.value = -1
    assert XYZ[idx] == 2
    idx.value = 3
    with pytest.raises(IndexError):
        XYZ[idx]
    idx.value = -4
    with pytest.raises(IndexError):
        XYZ[idx]


def test_order_compare():
    new_XYZ = RotationOrder(X_AXIS, Y_AXIS, Z_AXIS)

    assert new_XYZ == XYZ
    assert new_XYZ != ZXZ
    assert XYZ == XYZ
    assert XYZ != ZXZ

    assert hash(new_XYZ) == hash(XYZ)
    assert hash(XYZ) != hash(ZXZ)

    d = {XYZ: 'foo'}
    assert d.get(new_XYZ) == 'foo'


def test_order_persistance(subtests):
    buffer = pickle.dumps(XYZ)
    o = pickle.loads(buffer)

    for i in range(3):
        with subtests.test(index=i):
            o[i] == i

    o = copy(YZX)
    assert o[0] == 1
    assert o[1] == 2
    assert o[2] == 0


def test_order_inheritance() -> None:
    derived = DerivedRotationOrder(0, 1, 2)

    assert type(derived) is DerivedRotationOrder
    assert isinstance(derived, RotationOrder)
    assert issubclass(DerivedRotationOrder, RotationOrder)

    # Test global Methods
    assert compute_rotation_matrix(derived, EulerAngles())
    assert compute_rotation_matrix(derived, EulerAngles(),
                                   derived, EulerAngles())
    assert rotate_to(derived, EulerAngles(), Vector.E1)
    assert rotate_from(derived, EulerAngles(), Vector.E1)
    assert rotate_between(derived, EulerAngles(), derived, EulerAngles(),
                          Vector.E1)
    assert ReferenceFrame(derived, EulerAngles())

    # Test repr
    assert repr(derived).startswith('tests.inherited_types')

    # Test __new__()
    obj = RotationOrder.__new__(DerivedRotationOrder)
    obj.__init__(0, 1, 2)

    assert type(obj) is DerivedRotationOrder
    assert obj == derived

    # Test __dict__ suppport
    assert derived.foo
