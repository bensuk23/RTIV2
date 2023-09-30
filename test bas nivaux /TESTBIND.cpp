#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>	// pour memset 
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>

int main()
{
	int sServeur;

	printf("pid = %d\n",getpid());

	// Creation de la socket
	if ((sServeur = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Erreur de socket()"); exit(1);
	}
	printf("socket creee = %d\n",sServeur);

	// Construction de l'adresse 

	struct addrinfo hints; 
	struct addrinfo *results;

	memset(&hints,0,sizeof(struct addrinfo)); hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV; // pour une connexion passive
	if (getaddrinfo(NULL,"50000",&hints,&results) != 0)
	{
		close(sServeur); exit(1);
	}

	// Affichage du contenu de l'adresse obtenue (optionnel, c’est juste pour info)

	char host[NI_MAXHOST]; char port[NI_MAXSERV]; struct addrinfo* info;
	getnameinfo(results->ai_addr,results->ai_addrlen, host,NI_MAXHOST,port,NI_MAXSERV,NI_NUMERICSERV | NI_NUMERICHOST);
	printf("Mon Adresse IP: %s -- Mon Port: %s\n",host,port);

	// Liaison de la socket à l'adresse
	if (bind(sServeur,results->ai_addr,results->ai_addrlen) < 0)
	{
	perror("Erreur de bind()"); exit(1);
	}
	freeaddrinfo(results);
	printf("bind() reussi !\n");

	pause();
	printf("apres pause");
}