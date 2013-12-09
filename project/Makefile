# Makefile for Embedded Linux OpenCV project

# directory to store files in
DIRECTORY = ~/embedded_linux/project
# compiler flags (to link opencv libraries)
CFLAGS = -lopencv_core -lopencv_imgproc -lopencv_highgui -I /usr/local/include -L /usr/local/lib

all: install

install: project.o main.o
	mkdir -p $(DIRECTORY)
	g++ main.o project.o $(CFLAGS) -o opencv
	rm -rf *.o

project.o: project.cpp
	g++ -c project.cpp $(CFLAGS) -o project.o

main.o:	main.cpp
	g++ -c main.cpp $(CFLAGS) -o main.o

clean: 	
	rm -rf *.o opencv