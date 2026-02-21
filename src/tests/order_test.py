from copy import copy
import pickle

import pytest

from pyevspace import RotationOrder, XYZ, YZX, ZXZ, X_AXIS, Y_AXIS, Z_AXIS


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
    assert s == '[X_AXIS, Y_AXIS, Z_AXIS]'
    s = str(YZX)
    assert s == '[Y_AXIS, Z_AXIS, X_AXIS]'
    s = str(ZXZ)
    assert s == '[Z_AXIS, X_AXIS, Z_AXIS]'


def test_order_repr():
    s = repr(XYZ)
    assert s == 'RotationOrder(X_AXIS, Y_AXIS, Z_AXIS)'
    s = repr(YZX)
    assert s == 'RotationOrder(Y_AXIS, Z_AXIS, X_AXIS)'
    s = repr(ZXZ)
    assert s == 'RotationOrder(Z_AXIS, X_AXIS, Z_AXIS)'


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


# todo: should RotationOrder be able to be created for persistance?
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
