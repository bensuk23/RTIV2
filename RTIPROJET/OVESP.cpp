#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mysql.h>
#include <mysql/mysql.h>
#include <pthread.h>

#include "OVESP.h"

//***** Etat du protocole : liste des clients loggés ****************

int clients[NB_MAX_CLIENTS];
int nbClients = 0;

int estPresent(int socket);
void ajoute(int socket);
void retire(int socket);
MYSQL* connexion;
char requete[256];


pthread_mutex_t mutexClients = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutexBD = PTHREAD_MUTEX_INITIALIZER;

//***** Parsing de la requete et creation de la reponse *************
bool OVESP(char* requete, char* reponse,int socket)
{

	pthread_mutex_lock(&mutexBD);


	connexion = mysql_init(NULL);
	if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
	{
	fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
	exit(1);  
	}
	// ***** Récupération nom de la requete *****************
	char *ptr = strtok(requete,"#");


	// ***** LOGIN ******************************************

	if (strcmp(ptr,"LOGIN") == 0)
	{
		char user[50], password[50] ,NCouPNC[50];
		strcpy(user,strtok(NULL,"#"));
		strcpy(password,strtok(NULL,"#"));
		strcpy(NCouPNC,strtok(NULL,"#"));
		if (strcmp(NCouPNC,"PNC")==0)
		{
			printf("\t[THREAD %p] LOGIN de %s\n",pthread_self(),user);

			if (estPresent(socket) >= 0) // client déjà loggé
			{
				sprintf(reponse,"LOGIN#ko#Client déjà loggé !");
				pthread_mutex_unlock(&mutexBD);
				return false;
			}
			else
			{
				if (OVESP_LoginPNC(user,password))
				{
					sprintf(reponse,"LOGIN#ok");
					ajoute(socket);
				}
				else
				{
					sprintf(reponse,"LOGIN#ko#Mauvais identifiants !");
					pthread_mutex_unlock(&mutexBD);
					return false;
				}
			}
		}
		else if (strcmp(NCouPNC,"NC")==0)
			{
				
				if (OVESP_LoginNC(user,password))
					{
						sprintf(reponse,"LOGIN#ok");
						ajoute(socket);
					}
					else
					{
						sprintf(reponse,"LOGIN#ko#le client existe deja !");
						pthread_mutex_unlock(&mutexBD);
						return false;
					}
			}
		pthread_mutex_unlock(&mutexBD);
	}

	// ***** LOGOUT *****************************************

	if (strcmp(ptr,"LOGOUT") == 0)
	{
		printf("\t[THREAD %p] LOGOUT\n",pthread_self());
		retire(socket);
		sprintf(reponse,"LOGOUT#ok");
		pthread_mutex_unlock(&mutexBD);
		return false;
	}


	return true;
}

//***** Traitement des requetes *************************************

bool OVESP_LoginPNC(const char* user,const char* password)
{

	int test =0;
	sprintf(requete,"select * from clients;");

	
	if (mysql_query(connexion,requete) != 0)
	{
		fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
		exit(1);
	}

	printf("Requete SELECT réussie.\n");

	// Affichage du Result Set
	MYSQL_RES *ResultSetA;

	if ((ResultSetA = mysql_store_result(connexion)) == NULL)
	{
		fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
		exit(1);
	}

	MYSQL_ROW ligneA;



	while ((ligneA = mysql_fetch_row(ResultSetA)) != NULL)
	{
		if (strcmp(user,ligneA[1])==0 && strcmp(password,ligneA[2])==0) 
		{
			test = 1;
			break;
		}
	}
	
	if (test == 1) return true;
	return false;

}

bool OVESP_LoginNC(const char* user,const char* password)
{
	
	int test = 0;

	sprintf(requete,"select * from clients;");

	
	if (mysql_query(connexion,requete) != 0)
	{
		fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
		exit(1);
	}

	printf("Requete SELECT réussie.\n");

	// Affichage du Result Set
	MYSQL_RES *ResultSetA;

	if ((ResultSetA = mysql_store_result(connexion)) == NULL)
	{
		fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
		exit(1);
	}

	MYSQL_ROW ligneA;



	while ((ligneA = mysql_fetch_row(ResultSetA)) != NULL)
	{
		if (strcmp(user,ligneA[1])==0 && strcmp(password,ligneA[2])==0) 
		{
			test = 1;
			break;
		}
	}
	
	if (test == 1) return false;


	sprintf(requete,"insert into clients values (NULL,'%s','%s');",user,password);
	if(mysql_query(connexion,requete) != 0)
	{
		fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
		return false;
	}

	return true;
}


//***** Gestion de l'état du protocole ******************************

int estPresent(int socket)
{
	
	
	int indice = -1;
	pthread_mutex_lock(&mutexClients);
	for(int i=0 ; i<nbClients ; i++)
	if (clients[i] == socket) { indice = i; break; }
	pthread_mutex_unlock(&mutexClients);
	return indice;
}

void ajoute(int socket)
{
	pthread_mutex_lock(&mutexClients);
	clients[nbClients] = socket;
	nbClients++;
	pthread_mutex_unlock(&mutexClients);
}

void retire(int socket)
{
	int pos = estPresent(socket);

	if (pos == -1) return;
	pthread_mutex_lock(&mutexClients);

	for (int i=pos ; i<=nbClients-2 ; i++)
	clients[i] = clients[i+1];nbClients--;
	pthread_mutex_unlock(&mutexClients);
}

//***** Fin prématurée **********************************************

void OVESP_Close()
{
	pthread_mutex_lock(&mutexClients);
	for (int i=0 ; i<nbClients ; i++)
	close(clients[i]);
	pthread_mutex_unlock(&mutexClients);
}