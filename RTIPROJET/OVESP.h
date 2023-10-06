#ifndef OVESP_H
#define OVESP_H
#define NB_MAX_CLIENTS 100

bool OVESP(char* requete, char* reponse,int socket);
bool OVESP_LoginPNC(const char* user,const char* password);
bool OVESP_LoginNC(const char* user,const char* password);

void OVESP_Close();

#endif
