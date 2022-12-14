import unittest
from vector_test import Test_vector
from matrix_test import Test_matrix
from module_test import Test_evspace
from angles_test import Test_angles


def get_suite():

    testClasses = [Test_vector, Test_matrix, Test_evspace, Test_angles]

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
