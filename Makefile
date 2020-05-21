#INCLUDES = -I./../../opencv/build/include
#default : displacement.cpp
#	g++ $(INCLUDES) displacement.cpp -o displacement
# path for opencv/build.
# mingw32-make.exe in Windows.
CC = g++
CFLAGS = -g -Wall
SRCS = displacement.cpp
PROG = displacement

OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)

.PHONY: all clean

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

all: $(PROG)

clean:
	rm -f $(OBJS) $(PROG)