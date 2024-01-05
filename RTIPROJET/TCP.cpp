#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>	// pour memset 
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "TCP.h"



int ServerSocket(int port)
{
	int sEcoute;
	char * portStr;
	char  portC[6];
	printf("pid = %d\n",getpid());

	// Creation de la socket
	if ((sEcoute = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Erreur de socket()");
		exit(1);
	}

	printf("socket creee = %d\n",sEcoute);

	// Construction de l'adresse
	struct addrinfo hints;
	struct addrinfo *results;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV; // pour une connexion passive
	sprintf(portC, "%d", port);
	portStr = portC;

	printf("test = %s\n",portStr);
	if (getaddrinfo(NULL,portStr,&hints,&results) != 0)
	exit(1);

	// Affichage du contenu de l'adresse obtenue
	char host[NI_MAXHOST];
	char portS[NI_MAXSERV];
	getnameinfo(results->ai_addr,results->ai_addrlen,host,NI_MAXHOST,portS,NI_MAXSERV,NI_NUMERICSERV | NI_NUMERICHOST);
	printf("Mon Adresse IP: %s -- Mon Port: %s\n",host,portS);

	// Liaison de la socket à l'adresse
	if (bind(sEcoute,results->ai_addr,results->ai_addrlen) < 0)
	{
		perror("Erreur de bind()");
		exit(1);
	}
	freeaddrinfo(results);
	printf("bind() reussi !\n");

	return sEcoute;
}

int Accept(int sEcoute,char *ipClient)
{

	char host[NI_MAXHOST];
	char port[NI_MAXSERV];
	// Mise à l'écoute de la socket
	if (listen(sEcoute,SOMAXCONN) == -1)
	{
		perror("Erreur de listen()");
		exit(1);
	}
	printf("listen() reussi !\n");

	// Attente d'une connexion
	int sService;
	if ((sService = accept(sEcoute,NULL,NULL)) == -1)
	{
		perror("Erreur de accept()");
		exit(1);
	}
	printf("accept() reussi !\n");
	printf("socket de service = %d\n",sService);

	// Recuperation d'information sur le client connecte
	struct sockaddr_in adrClient;
	socklen_t adrClientLen = sizeof(struct sockaddr_in); // nécessaire
	getpeername(sService,(struct sockaddr*)&adrClient,&adrClientLen);
	getnameinfo((struct sockaddr*)&adrClient,adrClientLen,host,NI_MAXHOST,port,NI_MAXSERV,NI_NUMERICSERV | NI_NUMERICHOST);
	printf("Client connecte --> Adresse IP: %s -- Port: %s\n",host,port);
	

    if (host == NULL) {
        perror("Erreur lors de la récupération de l'adresse IP du client");
        close(sEcoute);
        close(sService);
        exit(EXIT_FAILURE);
    }
    strcpy(ipClient , host);


    printf("Adresse IP distante du client : %s\n", ipClient);
	
	return sService;
}
int ClientSocket(char* ipServeur,int portServeur)
{
	int sService;
	char * portStr;
	char portC [6];
    if ((sService = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Erreur de socket()"); 
        exit(1);
    }

    struct addrinfo hints; 
    struct addrinfo *results;
    sprintf(portC, "%d", portServeur);
    portStr = portC ; 
    memset(&hints,0,sizeof(struct addrinfo)); 
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV; // pour une connexion passive
    if (getaddrinfo(ipServeur,portStr,&hints,&results) != 0)
    {
        close(sService); exit(1);
    }
    if (connect(sService,results->ai_addr,results->ai_addrlen) == -1)
	{
		perror("Erreur de connect()");
		exit(1);
	}

	return sService;
}
int Receive(int sSocket,char* data)
{
	bool fini = false;
	int nbLus, i = 0;
	char lu1,lu2;


	while(!fini)
	{
		if ((nbLus = read(sSocket,&lu1,1)) == -1)
			return -1;
		if (nbLus == 0) return i; // connexion fermee par client
		
		if (lu1 == '+')
		{

			if ((nbLus = read(sSocket,&lu2,1)) == -1)
				return -1;

			if (nbLus == 0) return i; // connexion fermee par client

			if (lu2 == ')') fini = true;
			else
			{
				data[i] = lu1;
				data[i+1] = lu2;
				i += 2;
			}
		}
		else
		{
			data[i] = lu1;
			i++;
		}
	}
	return i;
}

int Send(int sSocket,char* data,int taille)
{
	if (taille > TAILLE_MAX_DATA)
	return -1;

	// Preparation de la charge utile
	char trame[TAILLE_MAX_DATA+2];

	memcpy(trame,data,taille);

	trame[taille] = '+';

	trame[taille+1] = ')';


	// Ecriture sur la socket

	return write(sSocket,trame,taille+2)-2;

}