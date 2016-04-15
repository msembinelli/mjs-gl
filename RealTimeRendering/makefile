#Variables
CC = g++
CFLAGS = -std=c++11 -g -O0 -Wall -Wextra
LIBS = -lGL -lglfw -lGraphicsMagick++
INCLUDES = -I/usr/include/GraphicsMagick
EXE = -o Assignment5
all:
	$(CC) $(CFLAGS) *.cpp $(EXE) $(LIBS) $(INCLUDES)

clean:
	rm -rf *.o
	