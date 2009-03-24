/**
 * @file: client_serveur.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 20/03/2009
 */

#include "client_serveur.h"
#define NBTHREAD 10
#define TAILLE_BUFF 100

int main()
{
    int res;

    if ( (res = fork()) < 0)
    {
        printf("Erreur dans le fork !!!!! \n");
        exit(1);
    }

    else if (res == 0)
    {
        applicationServeur();
    }
    else
    {
        applicationClient();
    }

    return 0;
}

/**
* code des fonctions et procédures coté serveur
*/

void applicationServeur()
{
    /* déclatation des variables */
    pthread_t variableThread;
    pthread_t tabThread[NBTHREAD];
    FileAttenteClients listeAttenteClient;
    int i;

    /* initialisation de la liste d'attente */
    initialisationListeAttenteClient(listeAttenteClient);

    /* signalisation des fichiers disponibles à l'annuaire*/
    signalisationFichierAnnuaire();

    /* création du thread pour écouter les demandes clients */
    pthread_create(variableThread, NULL, (void*) threadDialogueClient, (void*) &listeAttenteClient);

    /* création des threads pour envoyer des blocs */
    i = 0;
    while (i < NBTHREAD)
    {
        pthread_create(tabThread[i], NULL, (void*) threadEnvoiMessage, (void*) &listeAttenteClient);
        i++;
    }

    /* surveillance du clavier pour détecter s'il y a une frappe */
    while (1)
    {
        char buff[TAILLE_BUFF];
        fgets(buff, TAILLE_BUFF, stdin);
        /* Le dernier carractère est un retour chariot */
        buff[strlen(buff)-1] = '\0';

        if (strcmp(buff, "fin serveur") == 0)
        {
            break;
        }
    }

    /* arrêt du serveur */
        /* arrêt de tous les threads lancés */
    /***************************************/
        /* suppression de la liste d'attente */
    arretServeur();


}

void signalisationFichierAnnuaire()
{

}

void initialisationListeAttenteClient(FileAttenteClients listeAttente)
{

}

void threadDialogueClient(FileAttenteClients* listeAttente)
{

}

void threadEnvoiMessage(FileAttenteClients* listeAttente)
{

}

void arretServeur()
{

}

/**
* code des fonctions et procédures coté client
*/

void applicationClient()
{

}



/*****************
* Fin de Fichier
*****************/
