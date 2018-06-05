SRC = $(wildcard *.cpp)
OBJ = $(SRC:%.cpp=%.o)
HPP = $(wildcard *.hpp)

ifndef CXX
CXX = g++
endif

LIBS = -lboost_program_options -lfcgi -ljsoncpp -lstdc++ -lpthread
INCS = -I /usr/include/jsoncpp
CXXFLAGS = -std=c++11 -O3

all: statserver

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCS) -c $<

statserver: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJ) statserver *core *~ *.bak

$(OBJ):	$(HPP) 




