#include "windowclient.h"
#include "ui_windowclient.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "TCP.h"
#include "OVESP.h"
#include <QMessageBox>
#include <string>
using namespace std;

extern WindowClient *w;
int sClient;
ARTICLE articleEnCours;
char * adresseIP = "192.168.161.161";
int port = 50000;
float totalCaddie = 0.0;



#define REPERTOIRE_IMAGES "images/"
void CONSULTLOGIN();
void Echange(char* requete, char* reponse);

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);


    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    ui->pushButtonPayer->setText("Confirmer achat");
    setPublicite("!!! Bienvenue sur le Maraicher en ligne !!!");


    if((sClient = ClientSocket(adresseIP,port) )== -1)
    {
      perror("Erreur de ClientSocket");
      exit(1);
    }
    printf("Ca marche(%d)\n",sClient);
    



}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{

  char requete[200],reponse[200],LOGOUPASLOG[50],buff[100];


  // ***** Construction de la requete *********************
  if(isNouveauClientChecked())
  {
    printf("clienttest1 ");
    strcpy(LOGOUPASLOG,"NC");
  }
  else
  {
    printf("clienttest2 ");
    
    strcpy(LOGOUPASLOG,"PNC");
  }
  sprintf(requete,"LOGIN#%s#%s#%s",getNom(),getMotDePasse(),LOGOUPASLOG);


  // ***** Envoi requete + réception réponse **************

  Echange(requete,reponse);
  printf("clienttest3 ");


  // ***** Parsing de la réponse **************************


  char *ptr = strtok(reponse,"#"); // entête = LOGIN (normalement...)

  ptr = strtok(NULL,"#"); // statut = ok ou ko


  if (strcmp(ptr,"ok") == 0) 
  {
    QMessageBox::information(this,";)","Login OK.\n");
    printf("clienttest1 ");
    loginOK();
    CONSULTLOGIN();
  }

  if (strcmp(ptr,"ko") == 0) 
  {
    ptr = strtok(NULL,"#"); // raison du ko

    sprintf(buff,"Login PASOK : %s",ptr);
    QMessageBox::information(this,";)",ptr);
 
  }



}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************
  sprintf(requete,"LOGOUT");
  // ***** Envoi requete + réception réponse **************
  Echange(requete,reponse);
  // ***** Parsing de la réponse **************************
  QMessageBox::information(this,";)","Vous etes bien deconnecter");

  logoutOK();
  // pas vraiment utile...
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{


  char requete[200],reponse[200];

  // ***** Construction de la requete *********************
  if((articleEnCours.id + 1) != 22)
  {
    sprintf(requete,"CONSULT#%d",articleEnCours.id+1);
    // ***** Envoi requete + réception réponse **************
    Echange(requete,reponse);


      // ***** Parsing de la réponse **************************


    char *ptr = strtok(reponse,"#"); // entête = CONSULT (normalement...)

    ptr = strtok(NULL,"#"); // statut = ID ou −1

    if (strcmp(ptr,"-1") == 0) 
    {
      QMessageBox::information(this,";)","Erreur article non trouvé .\n");

    }  
    else
    {
      articleEnCours.id =atoi(ptr);


      strcpy(articleEnCours.intitule,strtok(NULL,"#"));


      ptr = strtok(NULL,"#"); // prix

      articleEnCours.prix= atof(ptr);

      ptr = strtok(NULL,"#"); // stock

      articleEnCours.stock = atoi(ptr);



      strcpy(articleEnCours.image,strtok(NULL,"#"));
     

      w->setArticle(articleEnCours.intitule,articleEnCours.prix,articleEnCours.stock,articleEnCours.image);
    }
  }
  else
  {
    QMessageBox::information(this,";)","Vous arriver à la fin ");

  }

  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************
  if((articleEnCours.id -1) != 0)
  {
    sprintf(requete,"CONSULT#%d",articleEnCours.id-1);
    // ***** Envoi requete + réception réponse **************
    Echange(requete,reponse);


      // ***** Parsing de la réponse **************************


    char *ptr = strtok(reponse,"#"); // entête = CONSULT (normalement...)

    ptr = strtok(NULL,"#"); // statut = ID ou −1

    if (strcmp(ptr,"-1") == 0) 
    {
      QMessageBox::information(this,";)","Erreur article non trouvé .\n");
    }  
    else
    {
      articleEnCours.id =atoi(ptr);


      strcpy(articleEnCours.intitule,strtok(NULL,"#"));


      ptr = strtok(NULL,"#"); // prix

      articleEnCours.prix= atof(ptr);

      ptr = strtok(NULL,"#"); // stock

      articleEnCours.stock = atoi(ptr);


      strcpy(articleEnCours.image,strtok(NULL,"#"));
     


      w->setArticle(articleEnCours.intitule,articleEnCours.prix,articleEnCours.stock,articleEnCours.image);
    }
  }
  else
  {
    QMessageBox::information(this,";)","Vous etes revenu au debut ");

  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************

    sprintf(requete,"ACHAT#%d#%d",articleEnCours.id,getQuantite());
    // ***** Envoi requete + réception réponse **************
    Echange(requete,reponse);


      // ***** Parsing de la réponse **************************


    char *ptr = strtok(reponse,"#"); // entête = ACHAT (normalement...)

    ptr = strtok(NULL,"#"); // statut = ID ou −1

    if (strcmp(ptr,"-1") == 0) 
    {
      QMessageBox::information(this,";)","Erreur article non trouvé .\n");

    }  
    else
    {
      articleEnCours.id =atoi(ptr); 



      ptr = strtok(NULL,"#"); // statut = Quantite ou 0

      articleEnCours.stock = atoi(ptr);


      ptr = strtok(NULL,"#"); // prix

      articleEnCours.prix= atof(ptr);

      


      if(articleEnCours.stock > 0)
      {
        w->videTablePanier();
        totalCaddie = 0;
        w->ajouteArticleTablePanier(articleEnCours.intitule,articleEnCours.stock,articleEnCours.prix);

        totalCaddie += articleEnCours.stock * articleEnCours.prix;

        w->setTotal(totalCaddie);

        QMessageBox::information(this,";)","Bien ajouter au panier .\n");
      }
      else
      {
        QMessageBox::information(this,";)","Pas assez de stock  .\n");
      }


      

    }
 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{

}



void Echange(char* requete, char* reponse)
{
  int nbEcrits, nbLus;
  // ***** Envoi de la requete ****************************

  if ((nbEcrits = Send(sClient,requete,strlen(requete))) == -1)
  {
    perror("Erreur de Send");
    close(sClient);
    exit(1);
  }


  // ***** Attente de la reponse **************************
  if ((nbLus = Receive(sClient,reponse)) < 0)
  {
    perror("Erreur de Receive");
    close(sClient);
    exit(1);
  }

  if (nbLus == 0)
  {
    printf("Serveur arrete, pas de reponse reçue...\n");
    close(sClient);
    exit(1);
  }
  reponse[nbLus] = 0;
}

void CONSULTLOGIN()
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************

  sprintf(requete,"CONSULT#1");
  // ***** Envoi requete + réception réponse **************

  Echange(requete,reponse);
  

    // ***** Parsing de la réponse **************************


  char *ptr = strtok(reponse,"#"); // entête = CONSULT (normalement...)
  printf("TEST 1Adresse IP distante du client : \n");

  ptr = strtok(NULL,"#"); // statut = ID ou −1


    articleEnCours.id =atoi(ptr);
    printf("TEST 2Adresse IP distante du client : \n");


    strcpy(articleEnCours.intitule,strtok(NULL,"#"));

    printf("TEST 3Adresse IP distante du client : \n");
    

    ptr = strtok(NULL,"#"); // stock

    articleEnCours.stock = atoi(ptr);
    printf("TEST 4Adresse IP distante du client : \n");

    ptr = strtok(NULL,"#"); // prix

    articleEnCours.prix= 10.0;
    printf("TEST 5Adresse IP distante du client : \n");



    printf("TEST 6Adresse IP distante du client : \n");
    strcpy(articleEnCours.image,strtok(NULL,"#"));
    printf("TEST 7Adresse IP distante du client : \n");
    printf("\n\n\n\n\n\n1:%d 2:%s 3:%f 4:%d 5:%s\n\n\n\n\n\n",articleEnCours.id,articleEnCours.intitule,articleEnCours.prix,articleEnCours.stock,articleEnCours.image);
   

    w->setArticle(articleEnCours.intitule,articleEnCours.prix,articleEnCours.stock,articleEnCours.image);
  
}
 

  

