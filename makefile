#.SILENT:

PROGRAMS= Client Serveur

ALL:$(PROGRAMS)

TCP.o:	TCP.cpp
				
	g++ -c TCP.cpp

SMOP.o:	SMOP.cpp
				
	g++ -c SMOP.cpp	

Client:		CLILIBPOOL.cpp TCP.o SMOP.o
	g++  CLILIBPOOL.cpp TCP.o SMOP.o -lpthread -o Client


Serveur:	SERVLIBPOOL.cpp TCP.o SMOP.o
	g++ SERVLIBPOOL.cpp TCP.o SMOP.o -lpthread -o Serveur

