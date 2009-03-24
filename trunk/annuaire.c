/**
 * @file: annuaire.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 20/03/2009
 */

#include "annuaire.h"

/**
* Fonctions et procédures du module
*/

void initialisationAnnuaire(BddServeurs * serveurs, BddFichiers * fichiers)
/**
* @note: procédure d'initialisation de l'annuaire :  les listes de clients, de serveurs, de fichiers.
*/
{
/* Creation des listes de serveurs */
    serveurs = malloc(sizeof(BddServeurs));
/* Creation des listes de fichiers */
    fichiers = malloc(sizeof(BddFichiers));
}


Socket initialiseSocketEcouteAnnuaire(int portAnnuaire)
/**
* @note: fonction d'initialisation de la socket d'écoute de l'annuaire.
* @param: portAnnuaire : numéro de port sur lequel on crée la socket d'écoute.
*/
{
    /* On déclare la socket pour l'annuaire */
    Socket socketEcouteAnnuaire;

    /* On la crée */
    socketEcouteAnnuaire=creationSocket();

    /* On lie la socket au numero de port portAnnuaire */
    definitionNomSocket(socketEcouteAnnuaire,portAnnuaire);

    return socketEcouteAnnuaire;
}


void fermetureAnnuaire(BddServeurs * serveurs, BddFichiers * fichiers)
/**
* @note: procédure de fermeture de l'annuaire de façon propre.
*/
{
/* Variables locales */
    int i,j;
    Fichier* temp_fichier;
    Serveur* temp_serveur;
/* Destruction des listes de serveurs */
    for(j=serveurs->nbServeurs;j>0;j--)
    {
        free(serveurs->tabServeurs[j]->adresseServeur);
        free(serveurs->tabServeurs[j]);
    }
    free(serveurs->tabServeurs);
    free(serveurs);
/* Destruction des listes de fichiers */
    while(fichiers->listeFichiers!=NULL)
    {
        for(i=fichiers->listeFichiers->nbBlocs;i>0;i--)
        {
            while((fichiers->listeFichiers->tabBlocs[i]).listeServeurs!=NULL)
            {
                temp_serveur=(fichiers->listeFichiers->tabBlocs[i]).listeServeurs;
                (fichiers->listeFichiers->tabBlocs[i]).listeServeurs=temp_serveur->serveurSuivant;
                free(temp_serveur);
            }
        }
        free(fichiers->listeFichiers->tabBlocs);
        free(fichiers->listeFichiers->nomFichier);

        temp_fichier=fichiers->listeFichiers;
        fichiers->listeFichiers=temp_fichier->fichierSuivant;
        free(temp_fichier);
    }
    free(fichiers);
}


int main()
{
    /* Initialisation de l'annuaire */
    BddServeurs* serveurs; /* pointeur sur la liste des serveurs connus */
    BddFichiers* fichiers; /* pointeur sur la liste des fichiers connus */
    initialisationAnnuaire(serveurs, fichiers);



    /* Fermeture de l'annuaire */
    fermetureAnnuaire(serveurs, fichiers);


    return 0;
}

 /*****************
* Fin de Fichier
*****************/

