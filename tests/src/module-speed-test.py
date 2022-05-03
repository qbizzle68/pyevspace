import sys
sys.path.append(r'C:\dev\libs\pyevspace\pyevspace\bin\x64')
sys.path.append(r'C:\dev\libs\pyevspace-plain\pyevspace\src')
import pyevspace
import evector

from time import perf_counter
from random import random

COUNT = 100000
DATA_plain = [evector.EVector(random(), random(), random()) for _ in range(COUNT)]
DATA_c = [pyevspace.EVector(random(), random(), random()) for _ in range(COUNT)]

def test(fn, arr, name):
    start = perf_counter()
    result = fn(arr)
    duration = perf_counter() - start
    print('{} took {:.3f} seconds\n\n'.format(name, duration))

    #for d in result:
        #assert -1 <= d <= 1, " incorrect values"

if __name__ == "__main__":
    print("Running benchmarks with COUNT = {}".format(COUNT))

    test(lambda d: [x * 2.5 for x in d], DATA_plain, '__add__ in plain library')

    test(lambda d: [x * 2.5 for x in d], DATA_c, '__add__ in c extension')
    
