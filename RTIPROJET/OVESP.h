#ifndef OVESP_H
#define OVESP_H
#define NB_MAX_CLIENTS 100

typedef struct
{
  int   id;
  char  intitule[20];
  float prix;
  int   stock;  
  char  image[20];
} ARTICLE;

bool OVESP(char* requete, char* reponse,int socket, int * nbArticles, ARTICLE articles[]);
bool OVESP_LoginPNC(const char* user,const char* password);
bool OVESP_LoginNC(const char* user,const char* password);
ARTICLE OVESP_CONSULT(const char* id);
ARTICLE OVESP_ACHAT(const char* id, const char* quantite);
void OVESP_CANCEL(const char* id,const char* stuck);
void OVESP_CADDIE();
void printArticle(const ARTICLE *article); 


void OVESP_Close();





#endif
