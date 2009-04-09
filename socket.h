/**
 * @file socket.h
 * @project lif12p2p
 * @author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since 19/03/2009
 * @version 9/04/2009
 */

#ifndef SOCKET_H
#define SOCKET_H

#include "errno.h"
#include "error.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "unistd.h"
#include "pthread.h"
/* Une partie nécessaire pour utiliser les sockets sous linux et windows */

/*#if defined (WIN32)
    #include "winsock2.h"
#elif defined (linux)*/
    #include "sys/types.h"
    #include "sys/socket.h"
    #include "netinet/in.h"
    #include "arpa/inet.h"
    #include "netdb.h"
/*#endif
*/

/**
* Definition des types.
*/
typedef int Socket;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;


/**
* @note procédure de création d'une socket.
* @return la socket créée est retournée.
*/
Socket creationSocket();


/**
* @note procédure de définition de nom d'une socket.
* @param s : socket que l'on va lier à un numéro de port.
* @param port : numéro de port auquel on va lier la socket s.
*/
void definitionNomSocket(Socket s, int port);


/**
* @note procédure d'acceptation d'une connexion.
* @param s : socket sur laquelle on accepte la connexion.
* @return renvoie la socket sur laquelle la connexion est acceptée.
*/
Socket acceptationConnexion(Socket s);


/**
* @note procédure de demande de connexion à une socket.
* @param la socket passée en parametre essai de se connecter à un serveur distant.
* @param nomServeur : le nom du serveur.
* @param port : le numero de port du serveur.
* @return renvoie 0 si tout se passe bien, 1 sinon.
*/
int demandeConnexionSocket(Socket s, char* nomServeur, int port);


/**
* @note procedure de capture de message sur une socket.
* @param s : socket d'écoute.
* @param buff : chaine de caractere stockant un message capturé.
*/
void ecouteSocket(Socket s, char* buff, int taille_buff);


/**
* @note fonction d'envoi de message sur une socket.
* @param s : socket sur laquelle on envoie le message.
* @param buff : message que l'on veut envoyer.
* @return retourne 0 si tout se passe bien, 1 sinon.
*/
int ecritureSocket(Socket s, char* buff, int taille_buff);


/**
* @note procédure de fermeture d'une socket.
* @param s : socket que l'on ferme.
*/
void clotureSocket(Socket s);


#endif
/*******************
* Fin de fichier
*******************/
