import unittest
from vector_test import Test_evector
from matrix_test import Test_ematrix
from module_test import Test_evspace


def get_suite():

    testClasses = [Test_evector, Test_ematrix, Test_evspace]

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
