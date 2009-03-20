/**
 * @file: client_serveur.h
 * @project: lif12p2p
 * @author: R�mi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 16/03/2009
 * @version: 20/03/2009
 */

#ifndef CLIENT_SERVEUR_H
#define CLIENT_SERVEUR_H

#include "socket.h"

/*#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>*/

/* Une partie n�cessaire pour utiliser les sockets sous linux et windows*/
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
* Gestion de la file d'attente des clients c�t� serveur.
*/
typedef struct Client
{
	Socket socketClient;
	int numeroBloc;
	char* nomFichier;
	struct Client* clientSuivant;
}Client;
/**
* @note: structure stockant les informations n�c�ssaires pour satisfaire la requete d'un client.
* @param: socketClient : num�ro de la socket sur laquelle a �t� �tablie la connexion avec le client.
* @param: numeroBloc : num�ro du bloc demand�.
* @param: nomFichier : nom du fichier demand�.
* @param: clientSuivant : pointeur sur le client suivant dans la file.
*/


typedef struct FileAttenteClients
{
	int nbClients;
	struct Client* premierClient;
	struct Client* dernierClient;
}FileAttenteClients;
/**
* @note: file d'attente des clients (FIFO).
* @param: nbClient : nombre de clients en attentes.
* @param: premierClient : pointeur sur structure Client correspondant au premier client de la file.
* @param: dernierClient : pointeur sur structure Client correspondant au dernier client de la file.
*/

/**
* Gestion de la file d'attente des telechargements c�t� client.
*/

typedef struct Telechargement
{
	int numeroBloc;
	char* nomFichier;
	char* adresseServeur;
	int numPortServeur;
}Telechargement;
/**
* @note: structure stockant les informations n�c�ssaires pour le t�l�chargement d'un bloc.
* @param:
* @param:
*/

typedef struct FileAttenteTelechargements
{
	int nbTelechargements;
	struct Telechargement* premierTelechargement;
	struct Telechargement* dernierTelechargement;
}FileAttenteTelechargements;
/**
* @note: file d'attente des t�l�chargements (FIFO).
* @param: nbTelechargements : nombre de t�l�chargements dans la file.
* @param: premierTelechargement : pointeur sur structure Telechargement correspondant au premier t�l�chargement de la file.
* @param: dernierTelechargement : pointeur sur structure Telechargement correspondant au dernier t�l�chargement de la file.
*/

typedef struct Fichier
{
	int nbBlocs;
	char* nomFichier;
	int* statutBlocs;
	struct Fichier* fichierSuivant;
}Fichier;
/**
* @note: structure stockant les informations sur les fichiers en cours de traitement.
* @param: nbBlocs : nombre de blocs du fichier.
* @param: nomFichier : nom du fichier.
* @param: statutBlocs : status du traitement des blocs:
* 			0 - Pas trait�
* 			1 - Trait�
* @param: fichierSuivant : pointeur sur le fichier suivant dans la liste.
*/

typedef struct ListeFichiers
{
	int nbFichiers;
	Fichier* listeFichiers;
}ListeFichiers;
/**
* @note: liste des fichiers en cours de t�l�chargement.
* @param: nbFichiers : nombre de fichiers.
* @param: listeFichiers : liste chain�e de fichiers.
*/



/***********************************
* Fonctions et proc�dures
***********************************/



#endif
/***************************
 * Fin du fichier
 **************************/


