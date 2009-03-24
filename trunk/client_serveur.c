/**
 * @file: client_serveur.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 20/03/2009
 */

#include "client_serveur.h"

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
    FileAttenteClients listeAttenteClient;

    /* initialisation de la liste d'attente */
    initialisationListeAttenteClient(listeAttenteClient);

    /* création du thread pour écouter les demandes clients */
    pthread_create(variableThread, NULL, (void*) threadDialogueClient, (void*) listeAttenteClient);




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
