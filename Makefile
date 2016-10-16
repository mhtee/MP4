CXX = g++ -std=c++11
CPPFLAGS = -Werror -g

SRCS = main.cpp

all:
	$(CXX) $(CPPFLAGS) $(SRCS) -o shell

