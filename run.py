import os
from multiprocessing import Process
import time


def runParticipant():
    os.system("./src/participant 127.0.0.1 3333")

def runAggregator():
    os.system("./src/aggregator 3333")

def runAntiAggregator():
    os.system("./src/anti_aggregator 3333")

if __name__ == "__main__":
    
    for i in xrange(0,100):
        aggregator = Process(target=runAggregator, args=())
        participant = Process(target=runParticipant, args=())

        aggregator.start()
        time.sleep(1)
        participant.start()

        print "begin: ", i

        aggregator.join()
        participant.join()


    for i in xrange(0,100):
        anti_aggregator = Process(target=runAntiAggregator, args=())
        participant = Process(target=runParticipant, args=())

        anti_aggregator.start()
        time.sleep(1)
        participant.start()

        print "begin: ", i

        anti_aggregator.join()
        participant.join()

print "end....."