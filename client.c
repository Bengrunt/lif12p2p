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
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1


#define NOM_SERVEUR "b710l089"
#define PORT_SERVEUR 8888
#define TAILLE_BUFF 100

int
main (int argc, char *argv[])
{
#if defined (WIN32)
  // Ceci est du code spécifique à windows
  WSADATA WSAData;
  int erreur = WSAStartup(MAKEWORD(2,0), &WSAData);
  if (erreur) {
    perror("probleme avec l'initialisation des sockets windows");
    exit(1);
  }
#endif

  SOCKET s; // Le socket à créer
  int sock_err; // Une variable pour stocker les erreurs
  SOCKADDR_IN sin; // La structure de description d'adresse
                   // Elle dépend du réseau que nous allons utilisé
                   // SOCKADDR_IN = Internet IPV4
                   // SOCKADDR_IN6 = Internet IPV6
                   // SOCKADDR_atalk = apple talk

  struct hostent *hp; // Pour obtenir l'adresse du serveur à partir de son nom

    // ######################################################
    // (1) Creation du socket (1)
    // Tout d'abord, il faut créer un socket,
    // ######################################################
  s = socket(AF_INET, SOCK_STREAM, 0); // AF_INET pour IPv4
                                       // SOCK_STREAM signifie TCP,
                                       // 0 il n'y a pas de protocole à choisir
  if (s == INVALID_SOCKET)
    {
      perror("Erreur à la creation du socket");
      exit(1);
    }


  // ######################################################
  // (3) Préparation de la connexion
  // ensuite il faut retrouver l'adresse IP de l'hôte (ordinateur) hébergeant
  // le serveur à l'aide de la fonction gethostbyname()
  // ######################################################
  hp = gethostbyname(NOM_SERVEUR); // Pour le test c'est "localhost"

  // Definition de l'adresse du serveur
  sin.sin_family = AF_INET;
  sin.sin_port = htons(PORT_SERVEUR); // htons sert à transformer l'entier en entier 16bits
  memcpy(&sin.sin_addr, hp->h_addr, hp->h_length); // On copie le résultat de gethostbyname
                                                   // au bon endroit
  // Si on connait l'addresse IP On peut aussi utiliser
  // sin.sin_addr.s_addr = inet_addr("127.0.0.1");

  // ######################################################
  // (4) Demande de connexion
  //  enfin, établir la connexion par la fonction connect()
  // ######################################################

  if(connect(s, (SOCKADDR *)&sin, sizeof(sin)) <0)
    {
      perror("connect");
      exit(1);
    }
  printf("Connection à %s sur le port %d\n", inet_ntoa (sin.sin_addr), htons(sin.sin_port));
  printf("envoie de données sur le socket %d\n ",s);


  // ######################################################
  // (5) Dialogue
  // Une fois la connexion établie, le serveur et le client peuvent s'échanger des messages
  // envoyés par write() ou send() et lus par read()ou recv() selon un protocole établi.
  // Ici nous utiliseront le protocole suivant : le client écrit et le serveur lit.
  // ######################################################
  while (1) {
    char buff[TAILLE_BUFF];
    fgets(buff, TAILLE_BUFF, stdin);
    // Le dernier carractère est un retour chariot
    buff[strlen(buff)-1] = '\0';

    if (strcmp(buff, "fin")==0) {
	// Quand l'utilisateur tape fin on sort
	break;
      }
    sock_err = send(s, (char*)buff, (strlen(buff)+1)*sizeof(char), MSG_NOSIGNAL);
    // s le socket sur laquel on ecrit
    // buff le message écrit
    // (strlen(buff)+1)*sizeof(char) la longueur du mesage
    // MSG_NOSIGNAL lorsque la connexion est brisée, send renvoie une erreur et
    //              ne génère pas de signaux (que je n'ai pas envie de traiter)
    if (sock_err < 0) {
      perror("erreur dans le send");
      break;
    }
  }


  // ######################################################
  // (6) Fermeture de la connexion
  // ######################################################
  if (close(s)< 0) {
    perror("Problème à la fermeture du socket");
  }
#if defined (WIN32)
  // Ceci est du code spécifique à windows
  WSACleanup();
#endif

  fprintf(stdout, "bye\n");
  return 0;
}


