import sys
import os

temp = open("temp", "w")

with open(sys.argv[1], "r") as outFile:
    with open(sys.argv[2], "r") as newLog:
        with open("temp", "w") as temp:
            outLines = outFile.readlines()
            newLines = newLog.readlines()

            for i in range(len(outLines)):
                line = outLines[i].strip() + ' ' + newLines[i]
                temp.write(line)

os.remove(sys.argv[1])
os.rename("temp", sys.argv[1])

