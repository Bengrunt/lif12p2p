/**
 * @file: client_serveur.h
 * @project: lif12p2p
 * @author: R�mi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 16/03/2009
 * @version: 05/04/2009
 */

#ifndef CLIENT_SERVEUR_H
#define CLIENT_SERVEUR_H

#include "socket.h"

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
* Gestion de la file d'attente des telechargements c�t� client.
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
* @note: structure stockant les informations n�c�ssaires pour le t�l�chargement d'un bloc.
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
* @param: statutBlocs : tableau des status du traitement des blocs:
* 			0 - Pas trait�
* 			1 - Trait�
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
* @note: liste des fichiers en cours de t�l�chargement.
* @param: nbFichiers : nombre de fichiers.
* @param: listeFichiers : liste chain�e de fichiers.
*/



/***********************************
* Fonctions et proc�dures
***********************************/

/**
* Application cot� serveur
*/

void applicationServeur();
/**
* @note: Application cot� serveur g�rant l'emmission des fichiers.
* @param:
*/

void signalisationFichierAnnuaire(Socket socketAnnuaire);
/**
* @note: fonction qui signale les fichiers disponibles � l'annuaire
* @param:
*/

void threadDialogueClient();
/**
* @note: proc�dure � ex�cuter dans un thread pour �couter les requ�tes clientes
* @param:
*/

void initialisationListeAttenteClient(FileAttenteClients listeAttente);
/**
* @note: proc�dure d'initialisation de la liste d'attente des clients
* @param: listeAttente : liste d'attente � initialiser
*/


void dialogueClient(Socket socketDialogue);
/**
* @note: proc�dure dialoguant avec le client
* @param:
*/

void traitementMessageBloc(Socket socketDialogue, char* buff);
/**
* @note: proc�dure qui analyse le message re�u du client
* @note: rempli la liste d'attente si besoin
* @param:
*/

void traitementMessageArret(Socket socketDialogue, char* buff);
/**
* @note: proc�dure qui analyse le message re�u du client
* @note: demandant une d�connexion du client
* @param:
*/

void traitementMessageErreur(Socket socketDialogue);
/**
* @note: proc�dure qui r�pond au message d'erreur
* @param:
*/

void threadEmmission();
/**
* @note: proc�dure qui lance les threads d'emmission des blocs
* @param:
*/

void threadEnvoiMessage();
/**
* @note: Fonction � ex�cuter dans un thread pour envoyer des blocs
* @note: la fonction boucle tant qu'il y a des blocs en liste d'attente
* @param:
*/

void signalisationChargeServeur(int valeur);
/**
* @note: cherche et envoie les donn�es � transmettre
* @param: les donn�es sont �crites sur la socket pass�e en parametre
*/

void envoiMessage(Client* client);
/**
* @note: cherche et envoie les donn�es � transmettre
* @param: les donn�es sont �crites sur la socket pass�e en parametre (dans la structure client)
*/

void arretServeur();
/**
* @note: proc�dure qui signale l'arret du serveur a l'annuaire, et l'arrete
* @param:
*/

/**
* Application cot� client
*/


void applicationClient();
/**
* @note: Application cot� serveur g�rant le t�l�chargement des fichiers.
* @param:
*/

void threadDemandeFichier();
/**
* @note: demande � l'utilisateur un fichier � t�l�charger, et traite les messages d'arr�ts
* @param:
*/

void demandeFichier(char* nomFichier);
/**
* @note: met le fichier dans la liste des fichiers, et demande � l'annuaire
* @param: nomFichier : nom du fichier � t�l�charger
*/

int traitementMessagePositif(char* buff);
/**
* @note: proc�dure qui analyse la r�ponse positive de l'annuaire
* @param: buff : chaine de caract�re � traiter
* @param: la fonction retourne le nombre de bloc total du fichier
*/

void traitementMessageNegatif(char* buff);
/**
* @note: proc�dure qui analyse la r�ponse n�gative de l'annuaire
* @param: buff : chaine de caract�re � traiter
*/

void threadTelechargement();
/**
* @note: proc�dure qui lance en boucle des threads pour t�l�chargerdes blocs
* @param:
*/

void threadRecuperationBloc();
/**
* @note: proc�dure � ex�cuter dans un thread pour t�l�charger un bloc
* @note: la focntion boucle tant qu'il y a des blocs � t�l�charger
* @param:
*/

Telechargement* rechercheBloc();
/**
* @note: fonction qui retourne le bloc suivant � t�l�charger
* @param:
*/

void telechargementBloc(Telechargement* );
/**
* @note: proc�dure qui demande et t�l�charge le bloc aupres du serveur
* @param: Telechargement* : pointeur sur le bloc � t�l�charger
*/

void arretClient();
/**
* @note: proc�dure qui signale l'arret du client � l'annuaire, et l'arrete
* @param:
*/

#endif
/***************************
 * Fin du fichier
 **************************/


