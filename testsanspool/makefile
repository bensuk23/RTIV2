#.SILENT:

PROGRAMS= Client Serveur

ALL:$(PROGRAMS)

TCP.o:	TCP.cpp
				
	g++ -c TCP.cpp

Client:		CLILIB.cpp TCP.o
	g++  CLILIB.cpp TCP.o -o Client


Serveur:	SERVLIB.cpp TCP.o
	g++ SERVLIB.cpp TCP.o -o Serveur

