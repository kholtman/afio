CC=gcc

all: cmpstat
	 bash ./regtest

cmpstat: cmpstat.c
	gcc -o cmpstat cmpstat.c

2gb:	
	gcc -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE makesparse.c   -o makesparse	
	bash  ./regtest2gb

sizeof:
	gcc statsize.c   -o statsize
	gcc -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE statsize.c   -o statsize64	
	echo normal compile env:
	./statsize
	echo large file compile env:
	./statsize64
	rm statsize statsize64
