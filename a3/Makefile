FLAGS = -Wall -std=gnu99 -g
DEPENDENCIES = helper.h psort.c

all: psort

psort: psort.o helper.o
	gcc ${FLAGS} -o $@ $^

%.o: %.c ${DEPENDENCIES}
	gcc ${FLAGS} -c $<

clean:
	rm -f *.o psort helper
