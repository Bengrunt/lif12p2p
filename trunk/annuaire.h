/*********************************************************************
 * \file annuaire.h
 * \author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * \since 16/03/2009
 * \version 19/04/2009
 * \brief Projet de transfert de fichier P2P centralisé par annuaire.
 ********************************************************************/

#ifndef ANNUAIRE_H
#define ANNUAIRE_H


/****************************
 * Fichiers d'en-tête inclus
 ****************************/

#include "socket.h"


/************************
 * Structures de données
 ************************/

/**
 * Gestion de la base de données des fichiers.
 */

/**
 * \struct Serveur annuaire.h
 * \brief Structure stockant un pointeur vers le référencement du serveur possédant un bloc dans la BDD des serveurs.
 * \param infos Pointeur vers le serveur dans BddServeur.
 */
typedef struct Serveur
{
	struct InfoServeurs* infos;
}Serveur;



/**
 * \struct Bloc annuaire.h
 * \brief Structure stockant le tableau de pointeurs sur les serveurs possédant le bloc.
 * \param nbServeurs Nombre de serveurs répertoriés dans le tableau.
 * \param capaTabServeurs Capacité maximum du tableau tabServeurs.
 * \param tabServeurs Tableau dynamique de pointeurs sur structures Serveur.
 * \warning tabServeurs n'est pas forcément rempli de manière contigüe et encore moins de manière triée.
 *          Les éléments sont insérés et supprimés de façon à minimiser la taille du tableau.
 */
typedef struct Bloc
{
	unsigned int nbServeurs;
	unsigned int capaTabServeurs;
	Serveur** tabServeurs;
}Bloc;



/**
 * \struct Fichier annuaire.h
 * \brief Structure stockant les informations sur les fichiers.
 * \param idFichier Identificateur de fichier.
 * \param nomFichier Nom du fichier.
 * \param nbBlocs Nombre de blocs du fichier.
 * \param capaTabBlocs Capacité max de tabBlocs.
 * \param tabBlocs Tableau de pointeurs sur structures Bloc contenant les informations sur chaque bloc.
 * \warning tabBloc est particulier car il ne peut (et ne doit) pas être modifié entre le moment de sa création et le moment de sa destruction. De ce fait ce n'est pas un tableau dynamique.
 */
typedef struct Fichier
{
    unsigned int idFichier;
	char * nomFichier;
	unsigned int nbBlocs;
	Bloc** tabBlocs;
}Fichier;



/**
 * \struct BddFichiers annuaire.h
 * \brief Structure simulant une base de données sous forme d'un tableau dynamique des fichiers référencés par l'annuaire.
 * \param nbFichiers Nombre de fichiers  référencés.
 * \param capaTabFichiers Capacité maximum de tabFichiers.
 * \param tabFichiers Tableau dynamique de pointeurs sur structures Fichier.
 * \warning tabFichiers n'est pas forcément rempli de manière contigüe et encore moins de manière triée.
 *          Les éléments sont insérés et supprimés de façon à minimiser la taille du tableau.
 * \param verrou_bddfich_w Mutex de la BddFichiers en écriture.
 * \param verrou_bddfich_r Mutex de la BddFichiers en lecture.
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
 * \struct InfoServeurs annuaire.h
 * \brief Structure stockant les informations sur les serveurs.
 * \param idServeur Identificateur de serveur.
 * \param numPort Numéro de port du serveur.
 * \param adresseServeur Adresse du serveur (adresse IP ou hostname)
 */
typedef struct InfoServeurs
{
    unsigned int idServeur;
	int numPort;
	char* adresseServeur;
}InfoServeurs;



/**
 * \struct BddServeurs annuaire.h
 * \brief Structure simulant une base de données sous forme d'un tableau dynamique des serveurs référencés par l'annuaire.
 * \param nbServeurs Nombre de serveurs référencés dans le tableau.
 * \param capaTabServeurs Capacité maximum de tabServeurs.
 * \param tabInfoServeurs Tableau de pointeurs sur structures InfoServeurs.
 * \warning tabInfoServeurs n'est pas forcément rempli de manière contigüe et encore moins de manière triée.
 *           Les éléments sont insérés et supprimés de façon à minimiser la taille du tableau.
 * \param verrou_bddserv_w : mutex de la BddServeurs en écriture.
 * \param verrou_bddserv_r : mutex de la BddServeurs en lecture.
 */
typedef struct BddServeurs
{
	unsigned int nbInfoServeurs;
	unsigned int capaTabInfoServeurs;
	InfoServeurs** tabInfoServeurs;
	pthread_mutex_t verrou_bddserv_w;
	pthread_mutex_t verrou_bddserv_r;
}BddServeurs;


/**************************
 * Fonctions et procédures
 **************************/

/**
 * \fn int initialisationAnnuaire( )
 * \brief Fonction d'initialisation de l'annuaire.
 * \warning Modifie le contenu des variables globales serveurs et fichiers.
 * \return Renvoie 0 si tout se passe bien, -1 en cas de problème.
 */
int initialisationAnnuaire( );


/**
 * \fn Socket initialiseSocketEcouteAnnuaire( int portAnnuaire )
 * \brief Fonction d'initialisation de la socket d'écoute de l'annuaire.
 * \param [in] portAnnuaire Numéro de port sur lequel on crée la socket d'écoute.
 * \return Renvoie la socket créée ou -1 en cas d'échec.
 */
Socket initialiseSocketEcouteAnnuaire( int portAnnuaire );


/**
 * \fn void lectureClavier( )
 * \brief Procedure de lecture des entrées clavier de l'utilisateur.
 */
void lectureClavier( );


/**
 * \fn int traiteMessage( Socket arg )
 * \brief Fonction globale de traitement de message reçu.
 * \param [in] arg Socket sur laquelle le message est arrivé.
 * \return Renvoie 0 si tout se passe bien ou -1 en cas de problème.
 */
int traiteMessage( Socket arg );


/**
 * \fn void traiteDemandeFichierClient( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (31) "Demande de fichier d'un client".
 * \param [in] s La socket sur laquelle la demande de fichier client a été émise.
 * \param [in] mess Le message contenant la demande de fichier client à traiter.
 */
void traiteDemandeFichierClient( Socket s, char* mess );


/**
 * \fn void traiteDemandeBlocClient( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (32) "Demande de bloc d'un client".
 * \param [in] s La socket sur laquelle la demande de bloc client a été émise.
 * \param [in] mess Le message contenant la demande de bloc client à traiter.
 */
void traiteDemandeBlocClient(Socket s, char* mess);


/**
 * \fn void traiteFinCommunicationClientClient( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (34) "Fin de communication".
 * \param [in] s La socket sur laquelle le message de fin de communication client a été émis.
 * \param [in] mess Le message de fin de communication client à traiter.
 */
void traiteFinCommunicationClient( Socket s, char* mess );


/**
 * \fn void traiteBlocDisponibleServeur( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (51) "Nouveau bloc disponible sur serveur".
 * \param [in] s La socket sur laquelle le message de nouveau bloc disponible sur serveur a été émis.
 * \param [in] mess Le message de nouveau bloc disponible serveur à traiter.
 */
void traiteBlocDisponibleServeur( char* mess );


/**
 * \fn void traiteArretServeur( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (52) "Arrêt de serveur".
 * \param [in] s La socket sur laquelle le message d'arrêt de serveur a été émis.
 * \param [in] mess Le message d'arrêt serveur à traiter.
 */
void traiteArretServeur( char* mess );


/**
 * \fn void traiteDemandeIdServeur( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (54) "Demande d'ID serveur".
 * \param [in] s La socket sur laquelle le message de demande d'ID serveur a été émis.
 * \param [in] mess Le message de demande d'ID serveur à traiter.
 * \warning Incrémente le compteur de génération d'idServeur.
 */
void traiteDemandeIdServeur( Socket s, char* mess );


/**
 * \fn void traiteDemandeIdFichier( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (55) "Demande d'ID fichier".
 * \param [in] s La socket sur laquelle le message de demande d'ID fichier a été émis.
 * \param [in] mess Le message de demande d'ID fichier à traiter.
 * \warning Incrémente le compteur de génération d'idFichier.
 */
void traiteDemandeIdFichier( Socket s, char* mess );


/**
 * \fn void traiteMessageErr( Socket s, char* mess )
 * \brief Procédure de traitement de message adressé au mauvais destinataire.
 * \param [in] s La socket sur laquelle le message inattendu a été émis.
 * \param [in] mess Le message en question.
 */
void traiteMessageErr( Socket s );


/**
 * \fn void fermetureAnnuaire( )
 * \brief Procédure de fermeture de l'annuaire.
 * \warning Modifie le contenu des variables globales serveurs et fichiers.
 */
void fermetureAnnuaire();


#endif

/***************************
 * Fin du fichier
 **************************/

