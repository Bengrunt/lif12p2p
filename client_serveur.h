/**
 * @file client_serveur.h
 * @project lif12p2p
 * @author R�mi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since 16/03/2009
 * @version 09/04/2009
 */

#ifndef CLIENT_SERVEUR_H
#define CLIENT_SERVEUR_H

#include "socket.h"
#include "sys/stat.h"

/******************************************
* Structures de donn�es
******************************************/

/**
* Gestion de la file d'attente des clients c�t� serveur.
*/

/**
* @note structure stockant les informations n�c�ssaires pour satisfaire la requete d'un client.
* @param socketClient : num�ro de la socket sur laquelle a �t� �tablie la connexion avec le client.
* @param numeroBloc : num�ro du bloc demand�.
* @param nomFichier : nom du fichier demand�.
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
* @note structure pour donner toutes les donn�es n�cessaire � la cr�ation d'un message
* @param idFichier : ID du fichier en cours de traitement
* @param nomFichier : nom du fichier en cours de traitement
* @param numTotalBloc : nombre total de bloc dans le fichier
* @param numeroBloc : num�ro du bloc en cours de traitement
*/
typedef struct StructureDisponibiliteBloc
{
    unsigned int idFichier;
    char* nomFichier;
    int numTotalBloc;
    int numeroBloc;
}StructureDisponibiliteBloc;

/**
* Gestion de la file d'attente des telechargements c�t� client.
*/

/**
* @note structure stockant les informations n�c�ssaires pour le t�l�chargement d'un bloc.
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
* @note file d'attente des t�l�chargements (FIFO).
* @param nbTelechargements : nombre de t�l�chargements dans la file.
* @param premierTelechargement : pointeur sur structure Telechargement correspondant au premier t�l�chargement de la file.
* @param dernierTelechargement : pointeur sur structure Telechargement correspondant au dernier t�l�chargement de la file.
* @param mutexListeAttenteClient : mutex sur la file d'attente des t�l�chargements
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
* 			0 - Pas trait�
* 			1 - Trait�
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
* @note liste des fichiers en cours de t�l�chargement.
* @param nbFichiers : nombre de fichiers.
* @param listeFichiers : liste chain�e de fichiers.
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
* Fonctions et proc�dures
***********************************/
/**
* fonction et proc�dure communes
*/

/**
* @note proc�dure cr�ant le message appropri� en chaine de cartact�re
* @param code : entier correspondant au code du message � cr�er
* @param structure : pointeur sur la structure avec les "bonnes" donn�es
* @param message : chaine de caract�re contenant le message cr��
* @return retourne 0 si le message est bien cr��, 1 sinon
*/
int creationMessage(int code, void* structure, char* message);

/**
* @note proc�dure � lancer dans un thread pour g�rer toute la lecture clavier du programe
*/
void threadLectureClavier();

/**
* @note proc�dure qui r�cup�re la ligne tap� au clavier (boucle s'il y a une ligne vide)
* @param message : chaine de caract�re lu
*/
void lireLigne(char* message);

/**
* Application cot� serveur
*/

/**
* @note Application cot� serveur g�rant l'emmission des fichiers.
*/
void applicationServeur();

/**
* @note fonction qui signale les fichiers disponibles � l'annuaire
* @param nomFichier : chaine de caract�re contenant le nom du fichier � mettre sur le r�seau
*/
void signalisationFichierAnnuaire(char* nomFichier);

/**
* @note proc�dure � ex�cuter dans un thread pour �couter les requ�tes clientes
*/
void threadDialogueClient();

/**
* @note proc�dure d'initialisation de la liste d'attente des clients
*/
void initialisationListeAttenteClient();

/**
* @note proc�dure dialoguant avec le client
* @param socketDialogue : socket de dialogue avec le client
*/
void dialogueClient(Socket socketDialogue);

/**
* @note proc�dure qui analyse le message re�u du client (rempli la liste d'attente si besoin)
* @param socketDialogue : socket de dialogue avec le client
* @param buff : chaine de caract�re contenant le message re�u du client
*/
void traitementMessageBloc(Socket socketDialogue, char* buff);

/**
* @note proc�dure qui analyse le message re�u du client (demandant une d�connexion du client)
* @param socketDialogue : socket de dialogue avec le client
*/
void traitementMessageArret(Socket socketDialogue);

/**
* @note proc�dure qui r�pond au message d'erreur
* @param socketDialogue : socket de dialogue avec le client
*/
void traitementMessageErreur(Socket socketDialogue);

/**
* @note proc�dure qui lance les threads d'emmission des blocs
*/
void threadEmmission();

/**
* @note Fonction � ex�cuter dans un thread pour envoyer des blocs la fonction boucle tant qu'il y a des blocs en liste d'attente
*/
void threadEnvoiMessage();

/**
* @note cherche et envoie les donn�es � transmettre
* @param valeur : entier correspondant � la charge : +/- 1
*/
void signalisationChargeServeur(int valeur);

/**
* @note cherche et envoie les donn�es � transmettre
* @param client : pointeur sur la structure contenant les informations sur le bloc � envoyer
*/
void envoiMessage(Client* client);

/**
* @note proc�dure qui signale l'arret du serveur a l'annuaire, et l'arrete
*/
void arretServeur();

/**
* Application cot� client
*/


/**
* @note Application cot� serveur g�rant le t�l�chargement des fichiers.
*/
void applicationClient();

/**
* @note proc�dure d'initialisation de la liste d'attente des telechargements
*/
void initialisationListeAttenteTelechargement();

/**
* @note met le fichier dans la liste des fichiers, et demande � l'annuaire
* @param nomFichier : nom du fichier � t�l�charger
*/
void demandeFichier(char* nomFichier);

/**
* @note proc�dure qui analyse la r�ponse positive de l'annuaire
* @param buff : chaine de caract�re � traiter
* @return la fonction retourne le nombre de bloc total du fichier
*/
void traitementMessagePositif(char* buff);

/**
* @note proc�dure qui analyse la r�ponse n�gative de l'annuaire
* @param buff : chaine de caract�re � traiter
*/
void traitementMessageNegatif(char* buff);

/**
* @note proc�dure qui lance en boucle des threads pour t�l�chargerdes blocs
*/
void threadTelechargement();

/**
* @note proc�dure � ex�cuter dans un thread pour t�l�charger un bloc la focntion boucle tant qu'il y a des blocs � t�l�charger
*/
void threadRecuperationBloc();

/**
* @note proc�dure qui demande et t�l�charge le bloc aupres du serveur
* @param telechargementATraiter : pointeur sur le bloc � t�l�charger
*/
void telechargementBloc(Telechargement* telechargementATraiter);

/**
* @note proc�dure qui analyse la reception d'un bloc
* @param socketDialogue : socket sur laquelle le message est re�u
* @param buff : chaine de caract�re � traiter
*/
void traitementMessageReceptionBloc(Socket socketDialogue, char* buff);

/**
* @note proc�dure qui analyse le fait de ne pas avoir trouv� le bloc
* @param telechargementATraiter : pointeur sur le t�l�chargement en cours
* @return retourne 0 si l'annuaire a renvoy� un autre serveur pour r�cup�rer le bloc, 1 sinon
*/
int traitementMessageBlocIntrouvable(Telechargement* telechargementATraiter);

/**
* @note proc�dure qui test si le fichier est complet (et le finalise le cas �ch�ant)
* @param pointeurFichier : pointeur sur le fichier en cours de traitement
*/
void finalisationFichier(Fichier* pointeurFichier);

/**
* @note proc�dure qui signale l'arret du client � l'annuaire, et l'arrete
*/
void arretClient();

#endif
/***************************
 * Fin du fichier
 **************************/


