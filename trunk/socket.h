
#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Une partie n�cessaire pour utiliser les sockets sous linux et windows

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
* @note: un "serveur" doit ex�cuter creationSocket, definitionSocket puis acceptationSocket
* @note: un "client" doit ex�cuter creationSocket puis demande de connexion
* @note: pour communiquer, les deux utilisent ecouteSocket et ecritureSocket
* @note: enfin, toute socket doit etre ferm�e avec fermetureSocket
*/

Socket creationSocket();
/**
* @note: proc�dure de cr�ation d'une socket
* @param: la socket cr��e est retourn�e comme valeur de retour
* @param:
*
*/

void definitionNomSocket(Socket s); /* on pourra rajouter le port en parametre */
/**
* @note: proc�dure de d�finition de nom d'une socket
* @param: la socket pass�e en parametre est affect�e � un port
* @param: et mise en mode �coute
*
*/

Socket acceptationConnexion(Socket s);
/**
* @note: proc�dure d'acceptation d'une connexion
* @param: la socket cr��e lors de l'acceptation de la connexion est
* @param: renvoy�e comme valeur de retour
*
*/

void demandeConnexionSocket(Socket s); /* On pourra passer le nom et le port du serveur en parametre */
/**
* @note: proc�dure de demande de connexion
* @param: la socket pass�e en parametre essai de se connecter �
* @param: une socket distante (nom d'hote et num�ro de port en variable globale)
*
*/

void ecouteSocket(Socket s);
/**
* @note: proc�dure qui �coute une socket
* @param: tout message arrivant sur la socket pass�e en parametre est
* @param: affich� � l'�cran
*
*/

void ecritureSocket(Socket s); /* ici, le message est lu au clavier, on pourra le passer en parametre */
/**
* @note: proc�dure d'envoi de message
* @param: tout message tap� au clavier (suivi de la touche "entr�e" est
* @param: envoy� � travers la socket pass�e en parametre
*
*/

void clotureSocket(Socket s);
/**
* @note: proc�dure de fermeture d'une socket
* @param:
*
*/

