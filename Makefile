all : client_serveur.exe annuaire.exe

client_serveur.exe: socket.o client_serveur.o
	gcc -o client_serveur.exe socket.o client_serveur.o -lpthread

annuaire.exe: socket.o annuaire.o
	gcc -o annuaire.exe socket.o annuaire.o -lpthread

socket.o: socket.c
	gcc -o socket.o -c socket.c -W -Wall -ansi -pedantic

client_serveur.o: client_serveur.c socket.h
	gcc -o client_serveur.o -c client_serveur.c -W -Wall -ansi -pedantic

annuaire.o: annuaire.c socket.h
	gcc -o annuaire.o -c annuaire.c -W -Wall -ansi -pedantic

clean:
	rm *.o *.exe
