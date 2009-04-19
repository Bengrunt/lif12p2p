/*********************************************************************************************
 * \file socket.h
 * \author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * \since 19/03/2009
 * \version 11/04/2009
 * \brief Projet créé dans le cadre de l'UE Lif12 de 3ème année de licence d'informatique.
 *          Module de gestion des sockets. Basé sur les sources données par F. Rico.
 ********************************************************************************************/


#ifndef SOCKET_H
#define SOCKET_H


/****************************
 * Fichiers d'en-tête inclus
 ****************************/


#include "errno.h"
#include "error.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "unistd.h"
#include "pthread.h"
#include "fcntl.h"

/* Une partie nécéssaire pour utiliser les sockets sous linux et windows */
/* #if defined (WIN32)
    #include "winsock2.h"
#elif defined (linux) */
    #include "sys/types.h"
    #include "sys/socket.h"
    #include "netinet/in.h"
    #include "arpa/inet.h"
    #include "netdb.h"
/* #endif */


/************************
 * Définition des types.
 ************************/


/**
 * \typedef int Socket
 */
typedef int Socket;

/**
 * \typedef struct sockaddr_in SOCKADDR_IN
 */
typedef struct sockaddr_in SOCKADDR_IN;

/**
 * \typedef sockaddr SOCKADDR
 */
typedef struct sockaddr SOCKADDR;



/************************************
 * Fonctions et procédures du module
 ************************************/


/**
 * \fn Socket creationSocket( )
 * \brief Procédure de création d'une socket.
 * \return La socket créée est retournée. -1 en cas d'erreur.
 */
Socket creationSocket( );


/**
 * \fn void definitionNomSocket( Socket s, int port )
 * \brief Procédure de définition de nom de socket.
 * \param [in] s Socket que l'on va lier à son numéro de port.
 * \param [in] port Numéro de port auquel on va lier la socket.
 */
void definitionNomSocket( Socket s, int port );


/**
 * \fn Socket acceptationConnexion( Socket s )
 * \brief Fonction d'acceptation d'une connexion sur une socket.
 * \param [in] s Socket sur laquelle on veut accepter la connexion.
 * \return Renvoie la socket sur laquelle la connexion est acceptée, qui vaut -1 si l'acceptation de connexion échoue.
 */
Socket acceptationConnexion( Socket s );


/**
 * \fn int demandeConnexionSocket( Socket s, char* nomServeur, int port )
 * \brief Fonction de demande de connexion sur une socket.
 * \param [in] s La socket avec laquelle on essaie de se connecter à un serveur distant.
 * \param [in] nomServeur Le nom du serveur (ou son adresse IP).
 * \param [in] port Le numéro de port du serveur.
 * \return Renvoie 0 si tout se passe bien, -1 en cas d'échec de la connexion.
 */
int demandeConnexionSocket( Socket s, char* nomServeur, int port );


/**
 * \fn void ecouteSocket( Socket s, char* buff, int taille_buff )
 * \brief Fonction de capture de message entrant sur une socket.
 * \param [in] s La socket sur laquelle on écoute les messages entrants.
 * \param [out] buff La chaine de caractères stockant le message capturé.
 * \param [in] taille_buff La taille de la chaine de caractères stockant le message capturé.
 * \return Renvoie 0 si tout se passe bien, -1 en cas d'échec de capture du message.
 */
int ecouteSocket( Socket s, char* buff, int taille_buff );


/**
 * \fn int ecritureSocket( Socket s, char* buff, int taille_buff )
 * \brief Fonction d'envoi de message sur une socket.
 * \param [in] s La socket sur laquelle on envoie le message.
 * \param [in] buff Le message que l'on veut envoyer.
 * \param [in] taille_buff La taille de la chaine de caractères stockant le message à envoyer.
 * \return Retourne 0 si tout se passe bien, -1 en cas d'échec d'envoi du message.
 */
int ecritureSocket( Socket s, char* buff, int taille_buff );


/**
 * \fn void clotureSocket( Socket s )
 * \brief Procédure de fermeture d'une socket.
 * \param [in] s La socket que l'on ferme.
 */
void clotureSocket(Socket s);


#endif

/*******************
 * Fin de fichier
 *******************/
