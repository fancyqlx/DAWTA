import os
from multiprocessing import Process
import time
import re


def runParticipant():
    os.system("./src/baseline_participant 127.0.0.1 3333")

def runAggregator():
    os.system("./src/baseline_aggregator 3333")


def run():
    for i in xrange(0,100):
        aggregator = Process(target=runAggregator, args=())
        participant = Process(target=runParticipant, args=())

        aggregator.start()
        time.sleep(1)
        participant.start()

        print "begin: ", i

        aggregator.join()
        participant.join()


if __name__ == "__main__":
    
    filePath = "./src/config"
    for x in xrange(0,9):
        with open(filePath,"w+") as f:
            f.write("N="+str(20+x*10)+"\n")
            f.write("bits_of_N=64\n")
        #run experiment for one x
        run()

    

print "end....."