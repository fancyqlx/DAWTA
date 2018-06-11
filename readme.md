# DAWTA
DAWTA is the experiment of the paper in INFOCOM 2018, you can find more details in：

Xuhui Gong, Qiang-Sheng Hua, Lixiang Qian, Dongxiao Yu, Hai Jin. Communication-Efficient and Privacy-Preserving Data Aggregation without Trusted Authority. The 37th International Conference on Computer Communications (INFOCOM 2018), April 15-19, 2018, Honolulu, Hawaii, USA.

# HOW TO

DAWTA is heavily realied on [socketx](https://github.com/fancyqlx/socketx), but you need not know anything about this library. We have written several python scripts to help you run experiments. Note that if you want to change the experiment or add some new experiments based on this framework, you should understand the process of this experiment and some knowledge about [socketx](https://github.com/fancyqlx/socketx), which may be a little complex compared to writing a new experiment by yourself. If you just want to run the experiment in our INFOCOM paper, the following introduction may be helpful.

1. make the project

DAWTA was organized by makefile and you can find all the compiling commands and rules in this file. Before you run the python scripts, you should make the project by just typing ***make*** command in root directory of DAWTA in the terminal.

2. run the experiments

After making the project, all the source codes should be compiled. If there are no errors, you can start the experiments by running python scripts.

- run.py: This is the startup script of the experiment. All the ****_run.py*** have the same function, which are provided for starting processes and running experiments. You can modify them as your purposes.

- statistic.py: This file is to statistic the experiment results after the experiments have been finished. The file will read the data from directory ***/data*** and output pictures into ***/data***. All the ****_statistic.py*** have the same function.

After you running all the python scripts, you can find desired pictures in ***/data*** directory. Good luck!

