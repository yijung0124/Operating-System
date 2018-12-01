CC = gcc
CFLAGS = -g -Wall
LFLAGS = 
INCLUDES = -I/
LIBS = -lpthread -lrt
SRC = prob1.c
OBJ = $(SRC:.c=.o)
MAIN = prob1

.PHONY: all show clean ps pdf showps

all:	$(MAIN)   barber 
		@echo	The solution to Problem 1 has been compiled

$(MAIN): $(OBJ)
		$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJ) $(LFLAGS) $(LIBS)

.c.o:
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
		$(RM) *.o *~ $(MAIN)
		rm -f share barber *.o

depend: $(SRC)
		makedepend $(INCLUDES) $^

barber: barber.c
	gcc barber.c -o barber -std=c99 -g -lpthread


