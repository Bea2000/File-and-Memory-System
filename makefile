CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

fileSystem: main.o osrms_API/osrms_API.o osrms_File/osrms_File.o
	$(CC) $(CFLAGS) -o fileSystem main.o osrms_API/osrms_API.o osrms_File/osrms_File.o

main.o: main.c osrms_API/osrms_API.h osrms_File/osrms_File.h
	$(CC) $(CFLAGS) -c main.c

osrms_API/osrms_API.o: osrms_API/osrms_API.c osrms_API/osrms_API.h osrms_File/osrms_File.h
	$(CC) $(CFLAGS) -c osrms_API/osrms_API.c -o osrms_API/osrms_API.o

osrms_File/osrms_File.o: osrms_File/osrms_File.c osrms_File/osrms_File.h
	$(CC) $(CFLAGS) -c osrms_File/osrms_File.c -o osrms_File/osrms_File.o

clean:
	rm -f *.o osrms_API/*.o osrms_File/*.o fileSystem

.PHONY: clean
