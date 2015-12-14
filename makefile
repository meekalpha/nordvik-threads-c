COMPILER = gcc
CFLAGS = -Wall -pedantic -pthread
CEXES = nordvik

all: ${CEXES}

nordvik: nordvik.c 
	${COMPILER} ${CFLAGS} nordvik.c -o nordvik

clean:
	rm -f *.o *~ ${CEXES}