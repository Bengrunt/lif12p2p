/**
 * @file: annuaire.h
 * @project: lif12p2p
 * @author: R�mi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 16/03/2009
 * @version: 20/03/2009
 */

#ifndef ANNUAIRE_H
#define ANNUAIRE_H
#include "socket.h"

/*#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>*/

/* Une partie n�cessaire pour utiliser les sockets sous linux et windows */
/*#if defined (WIN32)
    #include <winsock2.h>
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#endif
*/
/******************************************
* Structures de donn�es
******************************************/

/**
* Gestion de la base de donn�es des fichiers.
*/

typedef struct Serveur
{
	int numServeur;
	struct Serveur* serveurSuivant;
}Serveur;
/**
* @note: Structure "cellule" stockant le num�ro du serveur poss�dant un bloc.
* @param: numServeur : num�ro du serveur dans BddServeur.
* @param: serveurSuivant : pointeur sur le serveur suivant dans la liste des serveurs.
*/

typedef struct Bloc
{
	struct Serveur* listeServeurs;
}Bloc;
/**
* @note: structure stockant la liste des serveurs poss�dant le bloc.
* @param: listeServeurs : liste chain�e de structures Serveur.
*/

typedef struct Fichier
{
	char * nomFichier;
	int nbBlocs;
	Bloc* tabBlocs;
	struct Fichier* fichierSuivant;
}Fichier;
/**
* @note: structure "cellule" stockant les informations sur les fichiers.
* @param: nomFichier : nom du fichier.
* @param: nbBlocs : nombre de blocs du fichier.
* @param: tabBlocs : tableau de structures Bloc contenant les informations sur chaque bloc.
* @param: fichierSuivant : pointeur sur le fichier suivant.
*/

typedef struct BddFichiers
{
	int nbFichiers;
	Fichier* listeFichiers;
}BddFichiers;
/**
* @note: g�re la liste des fichiers r�f�renc�s par l'annuaire.
* @param: nbFichiers : nombre de fichiers  r�f�renc�s.
* @param: listeFichiers : liste chain�e de structures Fichier.
*/



/**
* Gestion de la base de donn�e des serveurs.
*/
typedef struct InfoServeurs
{
	int numPort;
	char* adresseServeur;
}InfoServeurs;
/**
* @note: structure stockant les informations sur les serveurs.
* @param: numPort : num�ro de port du serveur.
* @param: adresseServeur : adresse du serveur (IP ou hostname)
*/


typedef struct BddServeurs
{
	int nbServeurs;
	InfoServeurs** tabServeurs;
}BddServeurs;
/**
* @note: g�re la liste des serveurs en contact avec l'annuaire.
* @param: nbServeurs : nombre de serveurs r�f�renc�s dans la liste.
* @param: tabServeurs : tableau de pointeurs sur des structures de type InfoServeurs stockant les informations des serveurs r�f�renc�s.
*/



/**
* Gestion de la base de donn�e des clients. (FACULTATIF POUR L'INSTANT)
*/
typedef struct BddClients
{
    /* PAS ENCORE UTILISE */
}BddClients;
/**
* @note: g�re la liste des clients en contact avec l'annuaire.
*/




/***********************************
* Fonctions et proc�dures
***********************************/

void initialisationAnnuaire();
/**
* @note: proc�dure d'initialisation de l'annuaire :  les listes de clients, de serveurs, de fichiers.
* @param:
* @param:
*
*/

Socket initialiseSocketEcouteAnnuaire();
/**
* @note: fonction d'initialisation de la socket d'�coute de l'annuaire.
* @param:
* @param:
*
*/

void traiteDemandeFichierClient();
/**
* @note: traitement d'un message de type demande de fichier d'un client.
* @param:
*/


void traiteDemandeBlocClient();
/**
* @note: traitement d'un message de type demande de bloc d'un client.
* @param:
*/


void traiteBlocDisponibleServeur();
/**
* @note: traitement d'un message de type nouveau bloc.
* @param:
*/


void traiteArretServeur();
/**
* @note: traitement d'un message de type arr�t de serveur.
* @param:
*/


void analyseMessage();
/**
* @note: analyse un message re�u par l'annuaire et lance le traitement ad�quat.
* @param:
*/

void fermetureAnnuaire();
/**
* @note: proc�dure de fermeture de l'annuaire de fa�on propre.
* @param:
*/


#endif
/***************************
 * Fin du fichier
 **************************/

