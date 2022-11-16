import unittest
from vector_test import Test_evector
from matrix_test import Test_ematrix

def execute_tests():

    testClasses = [Test_evector, Test_ematrix]

    loader = unittest.TestLoader()

    suitesList = []
    for testClass in testClasses:
        print('loading ', testClass.__name__)
        suite = loader.loadTestsFromTestCase(testClass)
        suitesList.append(suite)

    print('suites loaded')
    bigSuite = unittest.TestSuite(suitesList)

    runner = unittest.TextTestRunner()
    results = runner.run(bigSuite)


if __name__ == '__main__':
    execute_tests()
