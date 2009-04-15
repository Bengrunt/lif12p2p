/**
 * @file client_serveur.h
 * @project lif12p2p
 * @author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since 16/03/2009
 * @version 09/04/2009
 */

#ifndef CLIENT_SERVEUR_H
#define CLIENT_SERVEUR_H

#include "socket.h"
#include "sys/stat.h"

/******************************************
* Structures de données
******************************************/

/**
* Gestion de la file d'attente des clients côté serveur.
*/

/**
* @note structure stockant les informations nécéssaires pour satisfaire la requete d'un client.
* @param socketClient : numéro de la socket sur laquelle a été établie la connexion avec le client.
* @param numeroBloc : numéro du bloc demandé.
* @param nomFichier : nom du fichier demandé.
* @param clientSuivant : pointeur sur le client suivant dans la file.
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
* @note file d'attente des clients (FIFO).
* @param nbClient : nombre de clients en attentes.
* @param premierClient : pointeur sur structure Client correspondant au premier client de la file.
* @param dernierClient : pointeur sur structure Client correspondant au dernier client de la file.
*/
typedef struct FileAttenteClients
{
    int nbClients;
    pthread_mutex_t mutexListeAttenteServeur;
    struct Client* premierClient;
    struct Client* dernierClient;
}FileAttenteClients;

/**
* @note structure pour donner toutes les données nécessaire à la création d'un message
* @param idFichier : ID du fichier en cours de traitement
* @param nomFichier : nom du fichier en cours de traitement
* @param numTotalBloc : nombre total de bloc dans le fichier
* @param numeroBloc : numéro du bloc en cours de traitement
*/
typedef struct StructureDisponibiliteBloc
{
    unsigned int idFichier;
    char* nomFichier;
    int numTotalBloc;
    int numeroBloc;
}StructureDisponibiliteBloc;

/**
* Gestion de la file d'attente des telechargements côté client.
*/

/**
* @note structure stockant les informations nécéssaires pour le téléchargement d'un bloc.
* @param
* @param
*/
typedef struct Telechargement
{
    unsigned int numeroBloc;
    unsigned int idFichier;
    char* nomFichier;
    char* adresseServeur;
    int numPortServeur;
    struct Telechargement* telechargementSuivant;
}Telechargement;

/**
* @note file d'attente des téléchargements (FIFO).
* @param nbTelechargements : nombre de téléchargements dans la file.
* @param premierTelechargement : pointeur sur structure Telechargement correspondant au premier téléchargement de la file.
* @param dernierTelechargement : pointeur sur structure Telechargement correspondant au dernier téléchargement de la file.
* @param mutexListeAttenteClient : mutex sur la file d'attente des téléchargements
*/
typedef struct FileAttenteTelechargements
{
    pthread_mutex_t mutexListeAttenteClient;
    int nbTelechargements;
    struct Telechargement* premierTelechargement;
    struct Telechargement* dernierTelechargement;
}FileAttenteTelechargements;

/**
* @note structure stockant les informations sur les fichiers en cours de traitement.
* @param nbBlocs : nombre de blocs du fichier.
* @param nomFichier : nom du fichier.
* @param statutBlocs : tableau des status du traitement des blocs:
* 			0 - Pas traité
* 			1 - Traité
* @param fichierSuivant : pointeur sur le fichier suivant dans la liste.
* @param mutexFichierEcriture : mutex sur le fichier
*/
typedef struct Fichier
{
    pthread_mutex_t mutexFichierEcriture;
    int longueurDernierBloc;
    int tailleFichier;
    unsigned int nbBlocs;
    unsigned int idFichier;
    char* nomFichier;
    int* statutBlocs;
    struct Fichier* fichierSuivant;
}Fichier;

/**
* @note liste des fichiers en cours de téléchargement.
* @param nbFichiers : nombre de fichiers.
* @param listeFichiers : liste chainée de fichiers.
* @param mutexListeFichierEcriture, mutexListeFichierLecture : mutex sur la liste des fichiers
*/
typedef struct ListeFichiers
{
    pthread_mutex_t mutexListeFichierEcriture;
    pthread_mutex_t mutexListeFichierLecture;
    int nbFichiers;
    Fichier* listeFichiers;
}ListeFichiers;



/***********************************
* Fonctions et procédures
***********************************/
/**
* fonction et procédure communes
*/

/**
* @note procédure créant le message approprié en chaine de cartactère
* @param code : entier correspondant au code du message à créer
* @param structure : pointeur sur la structure avec les "bonnes" données
* @param message : chaine de caractère contenant le message créé
* @return retourne 0 si le message est bien créé, 1 sinon
*/
int creationMessage(int code, void* structure, char* message);

/**
* @note procédure à lancer dans un thread pour gérer toute la lecture clavier du programe
*/
void threadLectureClavier();

/**
* @note procédure qui récupère la ligne tapé au clavier (boucle s'il y a une ligne vide)
* @param message : chaine de caractère lu
*/
void lireLigne(char* message);

/**
* Application coté serveur
*/

/**
* @note Application coté serveur gérant l'emmission des fichiers.
*/
void applicationServeur();

/**
* @note fonction qui signale les fichiers disponibles à l'annuaire
* @param nomFichier : chaine de caractère contenant le nom du fichier à mettre sur le réseau
*/
void signalisationFichierAnnuaire(char* nomFichier);

/**
* @note procédure à exécuter dans un thread pour écouter les requètes clientes
*/
void threadDialogueClient();

/**
* @note procédure d'initialisation de la liste d'attente des clients
*/
void initialisationListeAttenteClient();

/**
* @note procédure dialoguant avec le client
* @param socketDialogue : socket de dialogue avec le client
*/
void dialogueClient(Socket socketDialogue);

/**
* @note procédure qui analyse le message reçu du client (rempli la liste d'attente si besoin)
* @param socketDialogue : socket de dialogue avec le client
* @param buff : chaine de caractère contenant le message reçu du client
*/
void traitementMessageBloc(Socket socketDialogue, char* buff);

/**
* @note procédure qui analyse le message reçu du client (demandant une déconnexion du client)
* @param socketDialogue : socket de dialogue avec le client
*/
void traitementMessageArret(Socket socketDialogue);

/**
* @note procédure qui répond au message d'erreur
* @param socketDialogue : socket de dialogue avec le client
*/
void traitementMessageErreur(Socket socketDialogue);

/**
* @note procédure qui lance les threads d'emmission des blocs
*/
void threadEmmission();

/**
* @note Fonction à exécuter dans un thread pour envoyer des blocs la fonction boucle tant qu'il y a des blocs en liste d'attente
*/
void threadEnvoiMessage();

/**
* @note cherche et envoie les données à transmettre
* @param valeur : entier correspondant à la charge : +/- 1
*/
void signalisationChargeServeur(int valeur);

/**
* @note cherche et envoie les données à transmettre
* @param client : pointeur sur la structure contenant les informations sur le bloc à envoyer
*/
void envoiMessage(Client* client);

/**
* @note procédure qui signale l'arret du serveur a l'annuaire, et l'arrete
*/
void arretServeur();

/**
* Application coté client
*/


/**
* @note Application coté serveur gérant le téléchargement des fichiers.
*/
void applicationClient();

/**
* @note procédure d'initialisation de la liste d'attente des telechargements
*/
void initialisationListeAttenteTelechargement();

/**
* @note met le fichier dans la liste des fichiers, et demande à l'annuaire
* @param nomFichier : nom du fichier à télécharger
*/
void demandeFichier(char* nomFichier);

/**
* @note procédure qui analyse la réponse positive de l'annuaire
* @param buff : chaine de caractère à traiter
* @return la fonction retourne le nombre de bloc total du fichier
*/
void traitementMessagePositif(char* buff);

/**
* @note procédure qui analyse la réponse négative de l'annuaire
* @param buff : chaine de caractère à traiter
*/
void traitementMessageNegatif(char* buff);

/**
* @note procédure qui lance en boucle des threads pour téléchargerdes blocs
*/
void threadTelechargement();

/**
* @note procédure à exécuter dans un thread pour télécharger un bloc la focntion boucle tant qu'il y a des blocs à télécharger
*/
void threadRecuperationBloc();

/**
* @note procédure qui demande et télécharge le bloc aupres du serveur
* @param telechargementATraiter : pointeur sur le bloc à télécharger
*/
void telechargementBloc(Telechargement* telechargementATraiter);

/**
* @note procédure qui analyse la reception d'un bloc
* @param socketDialogue : socket sur laquelle le message est reçu
* @param buff : chaine de caractère à traiter
*/
void traitementMessageReceptionBloc(Socket socketDialogue, char* buff);

/**
* @note procédure qui analyse le fait de ne pas avoir trouvé le bloc
* @param telechargementATraiter : pointeur sur le téléchargement en cours
* @return retourne 0 si l'annuaire a renvoyé un autre serveur pour récupérer le bloc, 1 sinon
*/
int traitementMessageBlocIntrouvable(Telechargement* telechargementATraiter);

/**
* @note procédure qui test si le fichier est complet (et le finalise le cas échéant)
* @param pointeurFichier : pointeur sur le fichier en cours de traitement
*/
void finalisationFichier(Fichier* pointeurFichier);

/**
* @note procédure qui signale l'arret du client à l'annuaire, et l'arrete
*/
void arretClient();

#endif
/***************************
 * Fin du fichier
 **************************/


