SRCDIR=../src
INCDIR=../include

all: friendme.o friends.o

friendme.o: $(SRCDIR)/friendme.c $(INCDIR)/friends.h
	gcc -Wall -Werror -g -std=gnu99 -c $(SRCDIR)/friendme.c

friends.o: $(SRCDIR)/friends.c $(INCDIR)/friends.h
	gcc -Wall -Werror -g -std=gnu99 -c $(SRCDIR)/friends.c

clean:
	rm *.o