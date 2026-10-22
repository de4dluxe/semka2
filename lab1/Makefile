CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
SRC      = main.cpp $(wildcard src/*.cpp)

dbms: $(SRC) $(wildcard include/*.h)
	$(CXX) $(CXXFLAGS) $(SRC) -o dbms

go/dbms_go: $(wildcard go/*.go)
	cd go && go build -o dbms_go .

test: dbms go/dbms_go
	bash tests/run_tests.sh

clean:
	rm -f dbms go/dbms_go

.PHONY: test clean
