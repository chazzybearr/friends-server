PORT=56524
CFLAGS= -DPORT=\$(PORT) -g -std=gnu99 -Wall -Werror
SRCDIR=src
BUILDDIR=build
INCDIR=include

friend_server:
	$(MAKE) -C $(BUILDDIR)
	gcc $(CFLAGS) -o friend_server $(SRCDIR)/friend_server.c $(BUILDDIR)/friends.o $(BUILDDIR)/friendme.o

$(BUILDDIR)/friendme.o: $(SRCDIR)/friendme.c $(INCDIR)/friends.h
	gcc -Wall -Werror -g -std=gnu99 -c $(SRCDIR)/friendme.c

$(BUILDDIR)/friends.o: $(SRCDIR)/friends.c $(INCDIR)/friends.h
	gcc -Wall -Werror -g -std=gnu99 -c $(SRCDIR)/friends.c

clean:
	rm friend_server
	make clean -C $(BUILDDIR)
