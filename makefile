CC=g++
SOURCES=./src/config/*.cpp ./src/entity/*.cpp ./src/network/*.cpp ./src/protocol/*.cpp ./src/repo/*.cpp ./src/router/*.cpp 
BUILD=./build
FLAGS=-lcrypto -ljsoncpp -g -std=c++2a

build: 			build_server build_client

build_server: 	$(SOURCES)
				$(CC) -o ./$(BUILD)/server ./src/cmd/server.cpp $(SOURCES) $(FLAGS)

build_client: 	$(SOURCES)
				$(CC) -o ./$(BUILD)/client ./src/cmd/client.cpp $(SOURCES) $(FLAGS)

clean:
				rm -v ./$(BUILD)/*


