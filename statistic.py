import os
import re
import matplotlib.pyplot as plt
import numpy as np

def getInput():
    N, bits = input("<N> <bits>: ")
    return N, bits

def getData(line):
    obj = re.search(r"Bits=(.*), Time=(.*)", line)
    return int(obj.group(1)), int(obj.group(2))

def average(f):
    lines = f.readlines()
    sum_bits = 0
    sum_time = 0
    num = 0
    for line in lines:
        bits, time = getData(line)
        sum_bits += bits
        sum_time += time
        num += 1
    if num == 0:
        return sum_bits, sum_time
    return sum_bits/num, sum_time/num

def travelFiles_byN(anti):
    dirPath = "./data"
    N = 20
    bits = 64
    x = []
    y = []
    z = []
    files = os.listdir(dirPath)
    for i in xrange(0,10):
        filename = "results_N="+str(N+i*10)+"_bits="+str(bits)
        if anti:
            filename = "anti_results_N="+str(N+i*10)+"_bits="+str(bits)
        for fn in files:
            if (not os.path.isdir(fn)) and fn == filename:
                with open(dirPath+"/"+filename,"r") as f:
                    print "statistic file: ", filename
                    x.append(N+i*10)
                    datay, dataz = average(f)
                    y.append(datay)
                    z.append(dataz)
    return x, y, z

''' def travelFiles_bybits():
    dirPath = "./data"
    N = 20
    bits = 8
    x = []
    y = []
    z = []
    files = os.listdir(dirPath)
    for i in xrange(0,8):
        filename = "results_N="+str(N)+"_bits="+str(bits+i*8)
        for fn in files:
             if (not os.path.isdir(fn)) and fn == filename:
                with open(dirPath+"/"+filename,"r") as f:
                    print "statistic file: ", filename
                    x.append(bits+i*8)
                    datay, dataz = average(f)
                    y.append(datay)
                    z.append(dataz)
    return x, y, z '''

def drawPic(x,y,anti_x,anti_y,labelx,labely,filename):
    plt.plot(x,y,"r-o",anti_x,anti_y,"--o")
    plt.xlabel(labelx)
    plt.ylabel(labely)
    plt.savefig("./data/"+filename+".png")
    plt.show()
    

if __name__ == "__main__":
    x,y,z = travelFiles_byN(0)
    anti_x, anti_y, anti_z = travelFiles_byN(1)
    drawPic(x,y,anti_x,anti_y,"n","Message Compelxity (bits)","n-bits")
    drawPic(x,z,anti_x,anti_z,"n","Time (ms)","n-time")

    ''' x,y,z = travelFiles_bybits()
    drawPic(x,y,"M (bits)","Message Compelxity (bits)","M-bits")
    drawPic(x,z,"M (bits)","Time (ms)","M-time") '''



