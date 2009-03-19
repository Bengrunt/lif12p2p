
#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Une partie nécessaire pour utiliser les sockets sous linux et windows

#if defined (WIN32)
    #include <winsock2.h>
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#endif

typedef int Socket;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

/**
* @note: un "serveur" doit exécuter creationSocket, definitionSocket puis acceptationSocket
* @note: un "client" doit exécuter creationSocket puis demande de connexion
* @note: pour communiquer, les deux utilisent ecouteSocket et ecritureSocket
* @note: enfin, toute socket doit etre fermée avec fermetureSocket
*/

Socket creationSocket();
/**
* @note: procédure de création d'une socket
* @param: la socket créée est retournée comme valeur de retour
* @param:
*
*/

void definitionNomSocket(Socket s); /* on pourra rajouter le port en parametre */
/**
* @note: procédure de définition de nom d'une socket
* @param: la socket passée en parametre est affectée à un port
* @param: et mise en mode écoute
*
*/

Socket acceptationConnexion(Socket s);
/**
* @note: procédure d'acceptation d'une connexion
* @param: la socket créée lors de l'acceptation de la connexion est
* @param: renvoyée comme valeur de retour
*
*/

void demandeConnexionSocket(Socket s); /* On pourra passer le nom et le port du serveur en parametre */
/**
* @note: procédure de demande de connexion
* @param: la socket passée en parametre essai de se connecter à
* @param: une socket distante (nom d'hote et numéro de port en variable globale)
*
*/

void ecouteSocket(Socket s);
/**
* @note: procédure qui écoute une socket
* @param: tout message arrivant sur la socket passée en parametre est
* @param: affiché à l'écran
*
*/

void ecritureSocket(Socket s); /* ici, le message est lu au clavier, on pourra le passer en parametre */
/**
* @note: procédure d'envoi de message
* @param: tout message tapé au clavier (suivi de la touche "entrée" est
* @param: envoyé à travers la socket passée en parametre
*
*/

void clotureSocket(Socket s);
/**
* @note: procédure de fermeture d'une socket
* @param:
*
*/

