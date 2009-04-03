/**
 * @file: annuaire.h
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 16/03/2009
 * @version: 03/04/2009
 */

#ifndef ANNUAIRE_H
#define ANNUAIRE_H
#include "socket.h"

/*#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>*/

/* Une partie nécessaire pour utiliser les sockets sous linux et windows */
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
* Structures de données
******************************************/

/**
* Gestion de la base de données des fichiers.
*/

typedef struct Serveur
{
	int numServeur;
	struct Serveur* serveurSuivant;
}Serveur;
/**
* @note: Structure "cellule" stockant le numéro du serveur possédant un bloc.
* @param: numServeur : numéro du serveur dans BddServeur.
* @param: serveurSuivant : pointeur sur le serveur suivant dans la liste des serveurs.
*/

typedef struct Bloc
{
	struct Serveur* listeServeurs;
	int nbServeursDansListe;
}Bloc;
/**
* @note: structure stockant la liste des serveurs possédant le bloc.
* @param: listeServeurs : liste chainée de structures Serveur.
* @param: nbServeursDansListe : nombre de serveurs répétoriés dans la liste.
*/

typedef struct Fichier
{
    int idFichier;
	char * nomFichier;
	int nbBlocs;
	int capaTabBlocs;
	Bloc* tabBlocs;
	struct Fichier* fichierSuivant;
}Fichier;
/**
* @note: structure "cellule" stockant les informations sur les fichiers.
* @param: idFichier : identificateur.
* @param: nomFichier : nom du fichier.
* @param: nbBlocs : nombre de blocs du fichier.
* @param: capaTabBlocs : capacité max de tabBlocs.
* @param: tabBlocs : tableau de structures Bloc contenant les informations sur chaque bloc.
* @param: fichierSuivant : pointeur sur le fichier suivant.
*/

typedef struct BddFichiers
{
	int nbFichiers;
	Fichier* listeFichiers;
	pthread_mutex_t verrou_bddfich_w;
	pthread_mutex_t verrou_bddfich_r;
}BddFichiers;
/**
* @note: gère la liste des fichiers référencés par l'annuaire.
* @param: nbFichiers : nombre de fichiers  référencés.
* @param: listeFichiers : liste chainée de structures Fichier.
* @param: verrou_bddfich_w : mutex de la BddFichiers en écriture.
* @param: verrou_bddfich_r : mutex de la BddServeurs en lecture.
*/



/**
* Gestion de la base de donnée des serveurs.
*/
typedef struct InfoServeurs
{
    int idServeur;
	int numPort;
	char* adresseServeur;
}InfoServeurs;
/**
* @note: structure stockant les informations sur les serveurs.
* @param: idServeur : identificateur.
* @param: numPort : numéro de port du serveur.
* @param: adresseServeur : adresse du serveur (IP ou hostname)
*/


typedef struct BddServeurs
{
	int nbServeurs;
	int capaTabServeurs;
	InfoServeurs** tabServeurs;
	pthread_mutex_t verrou_bddserv_w;
	pthread_mutex_t verrou_bddserv_r;
}BddServeurs;
/**
* @note: gère la liste des serveurs en contact avec l'annuaire.
* @param: nbServeurs : nombre de serveurs référencés dans la liste.
* @param: capaTabServeurs : capacité max de tabServeurs.
* @param: tabServeurs : tableau de pointeurs sur des structures de type InfoServeurs stockant les informations des serveurs référencés.
* @param: verrou_bddserv_w : mutex de la BddServeurs en écriture.
* @param: verrou_bddserv_r : mutex de la BddServeurs en lecture.
*/



/**
* Gestion de la base de donnée des clients. (FACULTATIF POUR L'INSTANT)
*/
typedef struct BddClients
{
    int none;
    /* PAS ENCORE UTILISE */
}BddClients;
/**
* @note: gère la liste des clients en contact avec l'annuaire.
*/




/***********************************
* Fonctions et procédures
***********************************/

int initialisationAnnuaire(BddServeurs * serveurs, BddFichiers * fichiers);
/**
* @note: procédure d'initialisation de l'annuaire :  les listes de clients, de serveurs, de fichiers.
* @param: serveurs : pointeur sur la base de données des serveurs.
* @param: fichiers : pointeur sur la base de données des fichiers.
* @return: renvoie 0 si tout se passe bien, -1 sinon.
*/

Socket initialiseSocketEcouteAnnuaire(int portAnnuaire);
/**
* @note: fonction d'initialisation de la socket d'écoute de l'annuaire.
* @param: portAnnuaire : numéro de port sur lequel on crée la socket d'écoute.
* @return: renvoie la socket créée.
*/

int traiteMessage(Socket arg);
/**
* @note: fonction globale de traitement d'un message reçu.
* @param: arg : socket sur laquelle le message est arrivé.
* @return: renvoir 0 si tout se passe bien, -1 sinon.
*/

void traiteDemandeFichierClient(Socket s, char* mess);
/**
* @note: traitement d'un message de type demande de fichier d'un client.
* @param: s : la socket sur laquelle la demande de fichier client a été émise.
* @param: mess : la demande de fichier client a traiter.
*/


void traiteDemandeBlocClient(Socket s, char* mess);
/**
* @note: traitement d'un message de type demande de bloc d'un client.
* @param: s : la socket sur laquelle la demande de bloc client a été émise.
* @param: mess : la demande de bloc client a traiter.
*/


void traiteArretClient(Socket s, char* mess);
/**
* @note: traitement d'un message de type arret d'échange client.
* @param: s : la socket sur laquelle le message d'arret client a été émis.
* @param: mess : le message d'arret client a traiter.
*/

void traiteBlocDisponibleServeur(Socket s, char* mess);
/**
* @note: traitement d'un message de type nouveau bloc disponible sur serveur.
* @param: s : la socket sur laquelle le message de nouveau bloc disponible sur serveur a été émis.
* @param: mess : le message de nouveau bloc disponible a traiter.
*/


void traiteArretServeur(Socket s, char* mess);
/**
* @note: traitement d'un message de type arrêt de serveur.
* @param: s : la socket sur laquelle le message d'arret de serveur a été emis.
* @param: mess : le message d'arret serveur a traiter.
*/


void traiteMessageErr(Socket s, char* mess);
/**
* @note: traitement d'un message adressé au mauvais destinataire.
* @param: s : la socket sur laquelle le message inattendu a été émis.
* @param: mess : le message en question.
*/


void fermetureAnnuaire(BddServeurs * serveurs, BddFichiers * fichiers);
/**
* @note: procédure de fermeture de l'annuaire de façon propre.
* @param: serveurs : pointeur sur la base de données des serveurs.
* @param: fichiers : pointeur sur la base de données des fichiers.
*/


#endif
/***************************
 * Fin du fichier
 **************************/

