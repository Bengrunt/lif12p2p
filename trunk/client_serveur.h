/**
 * @file: client_serveur.h
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 16/03/2009
 * @version: 05/04/2009
 */

#ifndef CLIENT_SERVEUR_H
#define CLIENT_SERVEUR_H

#include "socket.h"

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
	pthread_mutex_t mutexListeAttenteServeur;
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
	struct Telechargement* telechargementSuivant;
}Telechargement;
/**
* @note: structure stockant les informations nécéssaires pour le téléchargement d'un bloc.
* @param:
* @param:
*/

typedef struct FileAttenteTelechargements
{
    pthread_mutex_t mutexListeAttenteClient;
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
* @param: statutBlocs : tableau des status du traitement des blocs:
* 			0 - Pas traité
* 			1 - Traité
* @param: fichierSuivant : pointeur sur le fichier suivant dans la liste.
*/

typedef struct ListeFichiers
{
    pthread_mutex_t mutexListeFichierEcriture;
    pthread_mutex_t mutexListeFichierLecture;
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

/**
* Application coté serveur
*/

void applicationServeur();
/**
* @note: Application coté serveur gérant l'emmission des fichiers.
* @param:
*/

void signalisationFichierAnnuaire(Socket socketAnnuaire);
/**
* @note: fonction qui signale les fichiers disponibles à l'annuaire
* @param:
*/

void threadDialogueClient();
/**
* @note: procédure à exécuter dans un thread pour écouter les requètes clientes
* @param:
*/

void initialisationListeAttenteClient(FileAttenteClients listeAttente);
/**
* @note: procédure d'initialisation de la liste d'attente des clients
* @param: listeAttente : liste d'attente à initialiser
*/


void dialogueClient(Socket socketDialogue);
/**
* @note: procédure dialoguant avec le client
* @param:
*/

void traitementMessageBloc(Socket socketDialogue, char* buff);
/**
* @note: procédure qui analyse le message reçu du client
* @note: rempli la liste d'attente si besoin
* @param:
*/

void traitementMessageArret(Socket socketDialogue, char* buff);
/**
* @note: procédure qui analyse le message reçu du client
* @note: demandant une déconnexion du client
* @param:
*/

void traitementMessageErreur(Socket socketDialogue);
/**
* @note: procédure qui répond au message d'erreur
* @param:
*/

void threadEmmission();
/**
* @note: procédure qui lance les threads d'emmission des blocs
* @param:
*/

void threadEnvoiMessage();
/**
* @note: Fonction à exécuter dans un thread pour envoyer des blocs
* @note: la fonction boucle tant qu'il y a des blocs en liste d'attente
* @param:
*/

void signalisationChargeServeur(int valeur);
/**
* @note: cherche et envoie les données à transmettre
* @param: les données sont écrites sur la socket passée en parametre
*/

void envoiMessage(Client* client);
/**
* @note: cherche et envoie les données à transmettre
* @param: les données sont écrites sur la socket passée en parametre (dans la structure client)
*/

void arretServeur();
/**
* @note: procédure qui signale l'arret du serveur a l'annuaire, et l'arrete
* @param:
*/

/**
* Application coté client
*/


void applicationClient();
/**
* @note: Application coté serveur gérant le téléchargement des fichiers.
* @param:
*/

void threadDemandeFichier();
/**
* @note: demande à l'utilisateur un fichier à télécharger, et traite les messages d'arrêts
* @param:
*/

void demandeFichier(char* nomFichier);
/**
* @note: met le fichier dans la liste des fichiers, et demande à l'annuaire
* @param: nomFichier : nom du fichier à télécharger
*/

int traitementMessagePositif(char* buff);
/**
* @note: procédure qui analyse la réponse positive de l'annuaire
* @param: buff : chaine de caractère à traiter
* @param: la fonction retourne le nombre de bloc total du fichier
*/

void traitementMessageNegatif(char* buff);
/**
* @note: procédure qui analyse la réponse négative de l'annuaire
* @param: buff : chaine de caractère à traiter
*/

void threadTelechargement();
/**
* @note: procédure qui lance en boucle des threads pour téléchargerdes blocs
* @param:
*/

void threadRecuperationBloc();
/**
* @note: procédure à exécuter dans un thread pour télécharger un bloc
* @note: la focntion boucle tant qu'il y a des blocs à télécharger
* @param:
*/

Telechargement* rechercheBloc();
/**
* @note: fonction qui retourne le bloc suivant à télécharger
* @param:
*/

void telechargementBloc(Telechargement* );
/**
* @note: procédure qui demande et télécharge le bloc aupres du serveur
* @param: Telechargement* : pointeur sur le bloc à télécharger
*/

void arretClient();
/**
* @note: procédure qui signale l'arret du client à l'annuaire, et l'arrete
* @param:
*/

#endif
/***************************
 * Fin du fichier
 **************************/


