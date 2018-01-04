DIR_SRC = ./src
DIR_LIB = ./libs
SRC = $(wildcard $(DIR_SRC)/*.cpp)
SRC_OBJ = $(patsubst %.cpp, %.o, ${SRC})

VPATH = ./socketx/src:./bigint:./cryptopp
CFLAGS += -I./socketx/src -I./bigint -I./cryptopp -std=c++11 \
	-pthread -L$(DIR_LIB) -lsocketx -lbigint -lcryptopp -g

DAWTA: $(DIR_SRC)/aggregator $(DIR_SRC)/anti_aggregator $(DIR_SRC)/participant \
	$(DIR_SRC)/baseline_participant $(DIR_SRC)/baseline_aggregator

$(DIR_SRC)/aggregator: $(DIR_SRC)/dawta.o $(DIR_SRC)/aggregator.o
	g++ $(DIR_SRC)/dawta.o $(DIR_SRC)/aggregator.o $(CFLAGS) -o $(DIR_SRC)/aggregator

$(DIR_SRC)/anti_aggregator: $(DIR_SRC)/dawta.o $(DIR_SRC)/anti_aggregator.o
	g++ $(DIR_SRC)/dawta.o $(DIR_SRC)/anti_aggregator.o $(CFLAGS) -o $(DIR_SRC)/anti_aggregator

$(DIR_SRC)/participant: $(DIR_SRC)/dawta.o $(DIR_SRC)/participant.o
	g++ $(DIR_SRC)/dawta.o $(DIR_SRC)/participant.o $(CFLAGS) -o $(DIR_SRC)/participant

$(DIR_SRC)/baseline_aggregator: $(DIR_SRC)/dawta.o $(DIR_SRC)/baseline_aggregator.o
	g++ $(DIR_SRC)/dawta.o $(DIR_SRC)/baseline_aggregator.o $(CFLAGS) -o $(DIR_SRC)/baseline_aggregator

$(DIR_SRC)/baseline_participant: $(DIR_SRC)/dawta.o $(DIR_SRC)/baseline_participant.o
	g++ $(DIR_SRC)/dawta.o $(DIR_SRC)/baseline_participant.o $(CFLAGS) -o $(DIR_SRC)/baseline_participant

$(DIR_SRC)/%.o:$(DIR_SRC)/%.cpp
	g++ $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(DIR_SRC)/aggregator $(DIR_SRC)/anti_aggregator $(DIR_SRC)/participant \
	$(DIR_SRC)/baseline_participant $(DIR_SRC)/baseline_aggregator