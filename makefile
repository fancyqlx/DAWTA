DIR_SRC = ./src
DIR_LIB = ./libs
SRC = $(wildcard $(DIR_SRC)/*.cpp)
SRC_OBJ = $(patsubst %.cpp, %.o, ${SRC})

VPATH = ./socketx/src:./bigint:./cryptopp
CFLAGS += -I./socketx/src -I./bigint -I./cryptopp -std=c++11 \
	-pthread -L$(DIR_LIB) -lsocketx -lbigint -lcryptopp

DAWTA: $(DIR_SRC)/aggregator $(DIR_SRC)/participant

$(DIR_SRC)/aggregator: $(DIR_SRC)/aggregator.o $(DIR_SRC)/dawta.o
	g++ $(DIR_SRC)/aggregator.o $(DIR_SRC)/dawta.o $(CFLAGS) -o $(DIR_SRC)/aggregator

$(DIR_SRC)/participant: $(DIR_SRC)/participant.o
	g++ $(DIR_SRC)/participant.o $(DIR_SRC)/dawta.o $(CFLAGS) -o $(DIR_SRC)/participant

$(DIR_SRC)/%.o:$(DIR_SRC)/%.cpp $(DIR_SRC)/dawta.hpp
	g++ $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(SRC_OBJ) $(DIR_SRC)/aggregator $(DIR_SRC)/participant