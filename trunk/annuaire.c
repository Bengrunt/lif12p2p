/**
 * @file: annuaire.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 20/03/2009
 */

#include "annuaire.h"

/**
* Déclaration des variables globales
*/

BddServeurs* vg_serveurs; /* pointeur sur la liste des serveurs connus */
BddFichiers* vg_fichiers; /* pointeur sur la liste des fichiers connus */

/**
* Fonctions et procédures du module
*/

void initialisationAnnuaire()
/**
* @note: procédure d'initialisation de l'annuaire :  les listes de clients, de serveurs, de fichiers.
*/
{
/* Creation des listes de serveurs */
    vg_serveurs = malloc(sizeof(BddServeurs));
/* Creation des listes de fichiers */
    vg_fichiers = malloc(sizeof(BddFichiers));
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


void fermetureAnnuaire()
/**
* @note: procédure de fermeture de l'annuaire de façon propre.
*/
{
/* Destruction des listes de serveurs */
    free(vg_serveurs);
/* Destruction des listes de fichiers */
    free(vg_fichiers);
}


int main()
{


    return 0;
}

 /*****************
* Fin de Fichier
*****************/

