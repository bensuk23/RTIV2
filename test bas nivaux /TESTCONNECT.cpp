#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>

int main(int argc,char* argv[])
{
	if (argc != 3)
	{
		printf("Erreur, usage :\n");
		printf("DemandeConnexion ipServeur portServeur\n");
		exit(1);
	}

	int sClient;
	printf("pid = %d\n",getpid());

	// Creation de la socket
	if ((sClient = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{perror("Erreur de socket()");
	exit(1);
	}

	printf("socket creee = %d\n",sClient);

	// Construction de l'adresse du serveur
	struct addrinfo hints;
	struct addrinfo *results;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;

	if (getaddrinfo(argv[1],argv[2],&hints,&results) != 0)
	exit(1);

	// Demande de connexion
	if (connect(sClient,results->ai_addr,results->ai_addrlen) == -1)
	{
	perror("Erreur de connect()");
	exit(1);
	}

	printf("connect() reussi !");
	pause();
}