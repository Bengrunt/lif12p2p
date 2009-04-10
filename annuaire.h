/**
 * @file annuaire.h
 * @project lif12p2p
 * @author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since 16/03/2009
 * @version 10/04/2009
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

/**
* @note Structure stockant un pointeur vers le référencement du serveur possédant un bloc dans la BDD des serveurs.
* @param infos : pointeur vers le serveur dans BddServeur.
*/
typedef struct Serveur
{
	struct InfoServeurs* infos;
}Serveur;



/**
* @note Structure stockant le tableau de pointeurs sur les serveurs possédant le bloc.
* @param nbServeurs : nombre de serveurs répétoriés dans le tableau.
* @param capaTabServeurs : capacité maximum du tableau tabServeurs.
* @param tabServeurs : tableau dynamique de pointeurs sur structures Serveur.
* @warning tabServeurs n'est pas forcément rempli de manière contigue et encore moins triée. Les éléments sont insérés et supprimés de façon à minimiser la taille du tableau.
*/
typedef struct Bloc
{
	unsigned int nbServeurs;
	unsigned int capaTabServeurs;
	Serveur** tabServeurs;
}Bloc;



/**
* @note structure stockant les informations sur les fichiers.
* @param idFichier : identificateur de fichier.
* @param nomFichier : nom du fichier.
* @param nbBlocs : nombre de blocs du fichier.
* @param capaTabBlocs : capacité max de tabBlocs.
* @param tabBlocs : tableau de pointeurs sur structures Bloc contenant les informations sur chaque bloc.
* @warning tabBloc est particulier car il ne peut (et ne doit) pas être modifié entre le moment de sa création et le moment de sa destruction. De ce fait ce n'est pas un tableau dynamique.
*/
typedef struct Fichier
{
    unsigned int idFichier;
	char * nomFichier;
	unsigned int nbBlocs;
	Bloc** tabBlocs;
}Fichier;



/**
* @note tableau dynamique des fichiers référencés par l'annuaire.
* @param nbFichiers : nombre de fichiers  référencés.
* @param capaTabFichiers : capacité maximum de tabFichiers.
* @param tabFichiers : tableau dynamique de pointeurs sur structures Fichier.
* @warning tabFichiers n'est pas forcément rempli de manière contigue et encore moins triée. Les éléments sont insérés et supprimés de façon à minimiser la taille du tableau.
* @param verrou_bddfich_w : mutex de la BddFichiers en écriture.
* @param verrou_bddfich_r : mutex de la BddServeurs en lecture.
*/
typedef struct BddFichiers
{
	unsigned int nbFichiers;
	unsigned int capaTabFichiers;
	Fichier** tabFichiers;
	pthread_mutex_t verrou_bddfich_w;
	pthread_mutex_t verrou_bddfich_r;
}BddFichiers;




/**
* Gestion de la base de donnée des serveurs.
*/

/**
* @note structure stockant les informations sur les serveurs.
* @param idServeur : identificateur.
* @param numPort : numéro de port du serveur.
* @param adresseServeur : adresse du serveur (IP ou hostname)
*/
typedef struct InfoServeurs
{
    unsigned int idServeur;
	int numPort;
	char* adresseServeur;
}InfoServeurs;



/**
* @note tableau dynamique des serveurs référencés par l'annuaire.
* @param nbServeurs : nombre de serveurs référencés dans le tableau.
* @param capaTabServeurs : capacité maximum de tabServeurs.
* @param tabInfoServeurs : tableau de pointeurs sur structures InfoServeurs.
* @warning tabInfoServeurs n'est pas forcément rempli de manière contigue et encore moins triée. Les éléments sont insérés et supprimés de façon à minimiser la taille du tableau.
* @param verrou_bddserv_w : mutex de la BddServeurs en écriture.
* @param verrou_bddserv_r : mutex de la BddServeurs en lecture.
*/
typedef struct BddServeurs
{
	unsigned int nbInfoServeurs;
	unsigned int capaTabInfoServeurs;
	InfoServeurs** tabInfoServeurs;
	pthread_mutex_t verrou_bddserv_w;
	pthread_mutex_t verrou_bddserv_r;
}BddServeurs;




/**
* Gestion de la base de donnée des clients. (FACULTATIF POUR L'INSTANT)
*/

/**
* @note gère la liste des clients en contact avec l'annuaire.
*/
typedef struct BddClients
{
    int none;
    /* PAS ENCORE UTILISE */
}BddClients;





/***********************************
* Fonctions et procédures
***********************************/

/**
* @note procédure d'initialisation de l'annuaire :  les listes de clients, de serveurs, de fichiers.
* @brief modifie le contenu des variables globales serveurs et fichiers.
* @return renvoie 0 si tout se passe bien, -1 sinon.
*/
int initialisationAnnuaire();


/**
* @note fonction d'initialisation de la socket d'écoute de l'annuaire.
* @param portAnnuaire : numéro de port sur lequel on crée la socket d'écoute.
* @return renvoie la socket créée.
*/
Socket initialiseSocketEcouteAnnuaire(int portAnnuaire);


/**
* @note fonction globale de traitement d'un message reçu.
* @param arg : socket sur laquelle le message est arrivé.
* @return renvoie 0 si tout se passe bien, -1 sinon.
*/
int traiteMessage(Socket arg);


/**
* @note traitement d'un message de type demande de fichier d'un client.
* @param s : la socket sur laquelle la demande de fichier client a été émise.
* @param mess : la demande de fichier client a traiter.
*/
void traiteDemandeFichierClient(Socket s, char* mess);


/**
* @note traitement d'un message de type demande de bloc d'un client.
* @param s : la socket sur laquelle la demande de bloc client a été émise.
* @param mess : la demande de bloc client a traiter.
*/
void traiteDemandeBlocClient(Socket s, char* mess);


/**
* @note traitement d'un message de type arret d'échange client.
* @param s : la socket sur laquelle le message d'arret client a été émis.
* @param mess : le message d'arret client a traiter.
*/
void traiteArretClient(Socket s, char* mess);


/**
* @note traitement d'un message de type nouveau bloc disponible sur serveur.
* @param s : la socket sur laquelle le message de nouveau bloc disponible sur serveur a été émis.
* @param mess : le message de nouveau bloc disponible a traiter.
*/
void traiteBlocDisponibleServeur(Socket s, char* mess);


/**
* @note traitement d'un message de type arrêt de serveur.
* @param s : la socket sur laquelle le message d'arret de serveur a été emis.
* @param mess : le message d'arret serveur a traiter.
*/
void traiteArretServeur(Socket s, char* mess);


/**
* @note traitement d'un message de type demande d'ID serveur.
* @param s : la socket sur laquelle le message de demande d'ID serveur a été émis.
* @param mess  : le message de demande d'ID serveur à traiter.
* @warning incrémente le generateur d'idServeur.
*/
void traiteDemandeIdServeur(Socket s, char* mess);


/**
* @note traitement d'un message de type demande d'ID fichier.
* @param s : la socket sur laquelle le message de demande d'ID fichier a été émis.
* @param mess  : le message de demande d'ID fichier à traiter.
* @warning incrémente le generateur d'idFichier.
*/
void traiteDemandeIdFichier(Socket s, char* mess);


/**
* @note traitement d'un message adressé au mauvais destinataire.
* @param s : la socket sur laquelle le message inattendu a été émis.
* @param mess : le message en question.
*/
void traiteMessageErr(Socket s, char* mess);


/**
* @note procédure de fermeture de l'annuaire de façon propre.
* @brief modifie le contenu des variables globales serveurs et fichiers.
*/
void fermetureAnnuaire();


#endif
/***************************
 * Fin du fichier
 **************************/

