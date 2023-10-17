#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
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
bool OVESP(char* requete, char* reponse,int socket,int * nbArticles, ARTICLE articles[])
{

	pthread_mutex_lock(&mutexBD);
	char id[5],quantite[5];


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


	// ***** CONSULT *****************************************

	if (strcmp(ptr,"CONSULT") == 0)
	{

		printf("\t[THREAD %p] CONSULT\n",pthread_self());

		strcpy(id,strtok(NULL,"#"));
		ARTICLE articleEnCoursC;
		articleEnCoursC = OVESP_CONSULT(id);


		if(articleEnCoursC.id == 0)
		{

			sprintf(reponse,"CONSULT#-1");
			return false;
		}
		else
		{
			sprintf(reponse,"CONSULT#%d#%s#%d#%f#%s",articleEnCoursC.id,articleEnCoursC.intitule,articleEnCoursC.stock,articleEnCoursC.prix,articleEnCoursC.image);
		}

		pthread_mutex_unlock(&mutexBD);


	}

	if (strcmp(ptr,"ACHAT") == 0)
	{

		printf("\t[THREAD %p] ACHAT\n",pthread_self());

		strcpy(id,strtok(NULL,"#"));
		ARTICLE articleEnCoursC;


		strcpy(quantite,strtok(NULL,"#"));


		articleEnCoursC = OVESP_ACHAT(id,quantite);


		if(articleEnCoursC.id == 0)
		{

			sprintf(reponse,"ACHAT#-1");
			return false;
		}
		else
		{

			

			articles[* nbArticles].id = articleEnCoursC.id;
			strcpy(articles[* nbArticles].intitule , articleEnCoursC.intitule);
			articles[* nbArticles].stock = articleEnCoursC.stock;
			articles[* nbArticles].prix = articleEnCoursC.prix;
			(*nbArticles)++;

			sprintf(reponse,"ACHAT#%d#%d",articleEnCoursC.id,articleEnCoursC.stock);

			
		}
		

		pthread_mutex_unlock(&mutexBD);


	}

	if (strcmp(ptr,"CANCEL") == 0)
	{
		strcpy(id,strtok(NULL,"#"));
		char stuck[20]; // Make sure this is large enough to hold the resulting string
		char idi[20]; 
		// Using sprintf to convert integer to string

		printf("testtttt avant ;%d",*nbArticles);
		sprintf(idi, "%d", articles[atoi(id)].id);
		sprintf(stuck, "%d", articles[atoi(id)].stock);


		OVESP_CANCEL(idi,stuck);


		for(int i = atoi(id); i < (*nbArticles); i++)
        {
         articles[i] = articles[i+1];
        }

        articles[*nbArticles]={0};

        if((*nbArticles-1) >= 0)
        {
          (*nbArticles)--;
        }

        printf("testtttt apres ;%d",*nbArticles);
        

		sprintf(reponse,"CANCEL#OUI");
		pthread_mutex_unlock(&mutexBD);


	}

	if (strcmp(ptr,"CANCELALL") == 0)
	{
		char stuck[20]; 
		char id[20]; 
		printf("TEST CANCEL:\n");
	      	

	
		for(int i = 0;i<*nbArticles;i++)
	      {
	      	printf("TEST CANCEL2:\n");
	      	sprintf(id, "%d", articles[i].id);
	      	printf("%s",id);
	      	sprintf(stuck, "%d", articles[i].stock);
	      	printf("%s",stuck);

	        OVESP_CANCEL(id,stuck);
	        articles[i] = {0};

	      }
	      *nbArticles =0;
		     sprintf(reponse,"CANCEL#OUIALL");
	      pthread_mutex_unlock(&mutexBD);
	}
	if (strcmp(ptr,"CONFIRMER") == 0)
	{
		time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
		
		char date_str[80];

		double totalCaddieTEMP = 0.0;
		strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);

		int idcli = estPresent(socket);
		int idF = 0;

		char requete[256];


	  for (int i = 0; i < *nbArticles; i++) 
	  {

	  	totalCaddieTEMP += articles[i].stock * articles[i].prix;
	  }	

	  sprintf(requete,"insert into Facture values (NULL,%d,'%s',%lf,%d);",idcli,date_str,totalCaddieTEMP,0);

	  if (mysql_query(connexion,requete) != 0)
		{
		  fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
		  exit(1);
		}
		printf("Requete INSERT réussie.\n");



 		sprintf(requete,"select * from Facture where date = '%s' AND idClient = %d AND montant = %.2lf;",date_str,idcli,totalCaddieTEMP);

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


      idF = atoi(ligneA[0]);
      
      printf("Requete SELECT SELECTSELECTSELECT %d réussie.\n",idF);



    }



		for(int i = 0;i<*nbArticles;i++)
    {

    	sprintf(requete,"insert into Vente values (%d,%d,%d);",idF,articles[i].id,articles[i].stock);
    	articles[i] = {0};

    	if (mysql_query(connexion,requete) != 0)
			{


			  fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
			  exit(1);
			}
			printf("Requete INSERT TEST3réussie.\n");
    }
    *nbArticles =0;


   

    sprintf(reponse,"CONFIRMER#CONFIRMERALL");

    pthread_mutex_unlock(&mutexBD);


	}

	for (int i = 0; i < *nbArticles; i++) 
		{
	        printf("Article %d:\n", i + 1);
	        printArticle(&articles[i]);
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

ARTICLE OVESP_CONSULT(const char* id)
{
	// Acces BD
	ARTICLE articleEnCours;
                      sprintf(requete,"select * from articles where id=%d;",atoi(id));

                      if (mysql_query(connexion,requete) != 0)
                      {
                      fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
                      exit(1);
                      }

                      printf("Requete SELECT réussie.\n");

                      // Affichage du Result Set
                      MYSQL_RES *ResultSet;

                      if ((ResultSet = mysql_store_result(connexion)) == NULL)
                      {
                      fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
                      exit(1);
                      }

                      MYSQL_ROW ligne;

                      

                      while ((ligne = mysql_fetch_row(ResultSet)) != NULL)
                      {

                      articleEnCours.id = atoi(ligne[0]);
                      strcpy(articleEnCours.intitule , ligne[1]);
                      articleEnCours.prix = atof(ligne[2]);
                      articleEnCours.stock = atoi(ligne[3]);
                      strcpy(articleEnCours.image , ligne[4]);
                      
                      }
                      
                      return articleEnCours;


}

ARTICLE OVESP_ACHAT(const char* id, const char* quantite)
{
	// Acces BD
	ARTICLE articleEnCours;
              
          sprintf(requete,"select * from articles where id=%d;",atoi(id));

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


	          articleEnCours.id = atoi(ligneA[0]);
	          strcpy(articleEnCours.intitule , ligneA[1]);


	          if(atoi(ligneA[3]) < atoi(quantite))
	          {
	            articleEnCours.stock = 0;
	            printf("test réussie.\n");
	          }
	          else
	          {
	            
	            articleEnCours.stock = atoi(quantite);
	            sprintf(requete,"update articles set stock = stock - %s where id=%d;",quantite,articleEnCours.id);

	            if (mysql_query(connexion,requete) != 0)
	            {
	              fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));

	              exit(1);
	            }
	            printf("Requete UPDATE réussie.\n");
	          }
	      

			articleEnCours.prix = atof(ligneA[2]);



          }


          return articleEnCours;


}

void OVESP_CANCEL(const char* id,const char* stuck)
{
	printf("Requete UPDATE réussie. %s %s\n",stuck,id);
	sprintf(requete,"update articles set stock = stock + %s where id=%s;",stuck,id);

	if (mysql_query(connexion,requete) != 0)
	{
	  fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
	  exit(1);
	}

	printf("Requete UPDATE réussie.\n");

}



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

void printArticle(const ARTICLE *article) 
{
    printf("ID: %d\n", article->id);
    printf("Intitule: %s\n", article->intitule);
    printf("Prix: %.2f\n", article->prix);
    printf("Stock: %d\n", article->stock);
    printf("Image: %s\n", article->image);
    printf("\n");
}