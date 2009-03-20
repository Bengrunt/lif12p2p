/**
 * @file: client_serveur.h
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
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

/* Une partie nécessaire pour utiliser les sockets sous linux et windows*/
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
* Gestion de la file d'attente des clients côté serveur.
*/
typedef struct Client
{
	Socket socketClient;
	int numeroBloc;
	char* nomFichier;
	struct Client* clientSuivant;
}Client;
/**
* @note: structure stockant les informations nécéssaires pour satisfaire la requete d'un client.
* @param: socketClient : numéro de la socket sur laquelle a été établie la connexion avec le client.
* @param: numeroBloc : numéro du bloc demandé.
* @param: nomFichier : nom du fichier demandé.
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
* Gestion de la file d'attente des telechargements côté client.
*/

typedef struct Telechargement
{
	int numeroBloc;
	char* nomFichier;
	char* adresseServeur;
	int numPortServeur;
}Telechargement;
/**
* @note: structure stockant les informations nécéssaires pour le téléchargement d'un bloc.
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
* @note: file d'attente des téléchargements (FIFO).
* @param: nbTelechargements : nombre de téléchargements dans la file.
* @param: premierTelechargement : pointeur sur structure Telechargement correspondant au premier téléchargement de la file.
* @param: dernierTelechargement : pointeur sur structure Telechargement correspondant au dernier téléchargement de la file.
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
* 			0 - Pas traité
* 			1 - Traité
* @param: fichierSuivant : pointeur sur le fichier suivant dans la liste.
*/

typedef struct ListeFichiers
{
	int nbFichiers;
	Fichier* listeFichiers;
}ListeFichiers;
/**
* @note: liste des fichiers en cours de téléchargement.
* @param: nbFichiers : nombre de fichiers.
* @param: listeFichiers : liste chainée de fichiers.
*/



/***********************************
* Fonctions et procédures
***********************************/



#endif
/***************************
 * Fin du fichier
 **************************/


