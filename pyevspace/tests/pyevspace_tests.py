import unittest
from .vector_test import TestVector
from .matrix_test import TestMatrix
# from .module_test import Test_evspace
from .angles_test import TestAngles
from .rotation_test import TestRotation


def get_suite():

    testClasses = [TestVector, TestMatrix, TestAngles, TestRotation]

    loader = unittest.TestLoader()

    suitesList = []
    for testClass in testClasses:
        print('loading ', testClass.__name__)
        suite = loader.loadTestsFromTestCase(testClass)
        suitesList.append(suite)

    print('suites loaded')
    bigSuite = unittest.TestSuite(suitesList)

    return bigSuite


def execute_tests():

    bigSuite = get_suite()

    runner = unittest.TextTestRunner()
    runner.run(bigSuite)


if __name__ == '__main__':
    execute_tests()
