/**
 * @file: client_serveur.h
 * @project: lif12p2p
 * @author: R�mi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 16/03/2009
 * @version: 08/04/2009
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
	int idFichier;
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

typedef struct StructureDisponibiliteBloc
{
    char* nomFichier;
    int numTotalBloc;
    int numeroBloc;
    int idServeur;
}StructureDisponibiliteBloc;

/**
* Gestion de la file d'attente des telechargements c�t� client.
*/

typedef struct Telechargement
{
	int numeroBloc;
	int idFichier;
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
    pthread_mutex_t mutexFichierEcriture;
    int longueurDernierBloc;
    int tailleFichier;
	int nbBlocs;
	int idFichier;
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
* fonction et proc�dure communes
*/

int creationMessage(int code, void* structure, char* message);
/**
* @note: proc�dure cr�ant le message appropri� en chaine de cartact�re
* @param: code : entier correspondant au code du message � cr�er
* @param: structure : pointeur sur la structure avec les "bonnes" donn�es
* @param: message : chaine de caract�re contenant le message cr��
* @param: retourne 0 si le message est bien cr��, 1 sinon
*/

void threadLectureClavier();
/**
* @note: proc�dure � lancer dans un thread pour g�rer toute la lecture clavier du programe
* @param:
*/

void lireLigne(char* message);
/**
* @note: proc�dure qui r�cup�re la ligne tap� au clavier (boucle s'il y a une ligne vide)
* @param: message : chaine de caract�re lu
*/

/**
* Application cot� serveur
*/

void applicationServeur();
/**
* @note: Application cot� serveur g�rant l'emmission des fichiers.
* @param:
*/

void signalisationFichierAnnuaire(char* nomFichier);
/**
* @note: fonction qui signale les fichiers disponibles � l'annuaire
* @param: nomFichier : chaine de caract�re contenant le nom du fichier � mettre sur le r�seau
*/

void threadDialogueClient();
/**
* @note: proc�dure � ex�cuter dans un thread pour �couter les requ�tes clientes
* @param:
*/

void initialisationListeAttenteClient();
/**
* @note: proc�dure d'initialisation de la liste d'attente des clients
* @param: listeAttente : structure "liste d'attente" � initialiser
*/


void dialogueClient(Socket socketDialogue);
/**
* @note: proc�dure dialoguant avec le client
* @param: socketDialogue : socket de dialogue avec le client
*/

void traitementMessageBloc(Socket socketDialogue, char* buff);
/**
* @note: proc�dure qui analyse le message re�u du client (rempli la liste d'attente si besoin)
* @param: socketDialogue : socket de dialogue avec le client
* @param: buff : chaine de caract�re contenant le message re�u du client
*/

void traitementMessageArret(Socket socketDialogue, char* buff);
/**
* @note: proc�dure qui analyse le message re�u du client (demandant une d�connexion du client)
* @param: socketDialogue : socket de dialogue avec le client
* @param: buff : chaine de caract�re contenant le message re�u du client
*/

void traitementMessageErreur(Socket socketDialogue);
/**
* @note: proc�dure qui r�pond au message d'erreur
* @param: socketDialogue : socket de dialogue avec le client
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
* @param: client : pointeur sur la structure contenant les informations sur le bloc � envoyer
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

void initialisationListeAttenteTelechargement();
/**
* @note: proc�dure d'initialisation de la liste d'attente des telechargements
* @param: listeAttente : structure "liste d'attente" � initialiser
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

void telechargementBloc(Telechargement* telechargementATraiter);
/**
* @note: proc�dure qui demande et t�l�charge le bloc aupres du serveur
* @param: telechargementATraiter : pointeur sur le bloc � t�l�charger
*/

void traitementMessageReceptionBloc(Socket socketDialogue, char* buff);
/**
* @note: proc�dure qui analyse la reception d'un bloc
* @param: buff : chaine de caract�re � traiter
*/

int traitementMessageBlocIntrouvable(Telechargement* telechargementATraiter);
/**
* @note: proc�dure qui analyse le fait de ne pas avoir trouv� le bloc
* @param: buff : chaine de caract�re � traiter
* @param: retourne 0 si l'annuaire a renvoy� un autre serveur pour r�cup�rer le bloc, 1 sinon
*/

void finalisationFichier(Fichier* pointeurFichier);
/**
* @note: proc�dure qui test si le fichier est complet (et le finalise le cas �ch�ant)
* @param:
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

