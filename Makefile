# Author: Orion Miller (aomiller)

CC = gcc
CFLAGS = -g -Wall -Werror

#Dependencies
RCOPY_DEPEND = rcopy.c rcopy.h librcp.o
SERVER_DEPEND = server.c server.h librcp.o
LIB_RCP_DEPEND = librcp.c librcp.h cpe464.h libcpe464.a

#Sources
RCOPY_SRCS = rcopy.c librcp.o
SERVER_SRCS = server.c librcp.o
LIB_RCP_SRCS = librcp.c checksum.c 

all: server rcopy librcp

rcopy: $(RCOPY_DEPEND)
	$(CC) $(CFLAGS) $(RCOPY_SRCS) -o $@

server: $(SERVER_DEPEND)
	$(CC) $(CFLAGS) $(SERVER_SRCS) -o $@

librcp.o: $(LIB_RCP_DEPEND)
	$(CC) $(CFLAGS) -c $(LIB_RCP_SRCS) -o $@

cpe464lib:
	@make -f ./cpe464_lib/lib.mk

clean:
	rm -f *~ *.o

allclean:
	rm -f *~ rcopy server *~ *.o

test:
	@echo "Needs a test functionality"