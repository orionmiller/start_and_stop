# Author: Orion Miller (aomiller)

CC = gcc
CFLAGS = -g -Wall -Werror

#Dependencies
RCOPY_DEPEND = rcopy.c rcopy.h librcp.o
SERVER_DEPEND = server.c server.h librcp.o
LIB_RCP_DEPEND = librcp.c librcp.h safe.o #checksum.c checksum.h # cpe464.h libcpe464.1.2.a
CPE_464_LIB_DEPEND = cpe464.h libcpe464.1.2.a
SAFE_DEPEND = safe.c safe.h

#Sources
RCOPY_SRCS = rcopy.c -l librcp.o
SERVER_SRCS = server.c -l librcp.o
LIB_RCP_SRCS = librcp.c -l safe.o #checksum.c
SAFE_SRCS = safe.c


all: server rcopy librcp.o safe.o

rcopy: $(RCOPY_DEPEND)
	$(CC) $(CFLAGS) $(RCOPY_SRCS) -o $@

server: $(SERVER_DEPEND)
	$(CC) $(CFLAGS) $(SERVER_SRCS) -o $@

librcp.o: $(LIB_RCP_DEPEND)
	$(CC) $(CFLAGS) -c $(LIB_RCP_SRCS) -o $@

safe.o: $(SAFE_DEPEND)
	$(CC) $(CFLAGS) -c $(SAFE_SRCS) -o $@

cpe464lib: 
	@make -f ./cpe464_lib/lib.mk
	@rm -f example.mk

clean:
	@rm -f *~ *.o

allclean:
	@rm -f *~ rcopy server *~ *.o example.mk cpe464.h libcpe464.1.2.a

test:
	@echo "Needs a test functionality"