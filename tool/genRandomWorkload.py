import random
import sys

addressRange = 62500000 # 250 GB
readRatio = 0.9 # % of read operations, 0 is write operation and 1 is read operation
maxRecords = 10000000
count = 0

while True:
    op = 0 if random.random() > readRatio else 1
    print("{} {}".format(random.randint(0,sys.maxsize % addressRange), op))
    count += 1
    if count == maxRecords:
        break
