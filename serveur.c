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
#endif
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

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

  SOCKET s; // La socket à créer
  int sock_err; // Une variable pour stocker les erreurs
  SOCKADDR_IN sin; // La structure de description d'adresse
                   // Elle dépend du réseau que nous allons utilisé
                   // SOCKADDR_IN = Internet IPV4
                   // SOCKADDR_IN6 = Internet IPV6
                   // SOCKADDR_atalk = apple talk

  // ######################################################
  // (1) Creation du socket (1)
  // celui-ci commence par créer un socket de connexion (que nous appellerons s)
  // ######################################################
  s = socket(AF_INET, SOCK_STREAM, 0); // AF_INET pour IPv4
                                       // SOCK_STREAM signifie TCP,
                                       // 0 il n'y a pas de protocole à choisir
  if (s == INVALID_SOCKET)
    {
      perror("Erreur à la creation du socket");
      exit(1);
    }
  printf ("Le socket %d est maintenant ouverte en mode TCP/IP\n", s);

  // ######################################################
  // (2.1) Définition d'un nom externe
  // Pour être atteind, le socket doit avoir un nom (couple adresse/port)
  // ######################################################
  sin.sin_family         = AF_INET;              // IPv4
  sin.sin_port           = htons (PORT_SERVEUR);       // Le port d'écoute
  sin.sin_addr.s_addr    = htonl (INADDR_ANY);   // Si la machine a plusieurs adresses
                                                 // on les écoute toutes
                                                 // Voir le code du client pour d'autre forme
  sock_err = bind (s, (SOCKADDR *) &sin, sizeof(sin));
  if (sock_err < 0) {
    perror("bind");
    close(s);
    exit(1);
  }

  printf ("Le socket %d est maintenant en attente sur le port %u\n", s, PORT_SERVEUR);

  // ######################################################
  // (2.2) Attente de connexion
  //
  // ######################################################
  sock_err = listen (s, 5);
  if (sock_err < 0) {
    perror("listen");
    close(s);
    exit(1);
  }

  // listen ne bloque pas, à partir de la 5 demande de conexion peuvent arriver
  // sans que le serveur les accept ou les rejette

  // ######################################################
  // (4) Acceptation d'une connexion
  // Le serveur accepte l'une des demande arrivée depuis le listen ou
  // attend s'il n'y en a pas
  // ######################################################
  SOCKET t;
  SOCKADDR_IN tadr;
  size_t recsize = sizeof(tadr);

  t = accept (s, (SOCKADDR *) &tadr, &recsize); //s : la socket d'attente
                                                //tadr : la structure ou on va stocker les info sur le client
                                                //recsize : donnée = la taille de tadr (pour éviter le dépassement)
                                                //          resultat = la taille de ce qui est réellement mis dans tadr

  printf("Connection de %s sur le port %d\n", inet_ntoa (tadr.sin_addr), htons(tadr.sin_port));

  // ######################################################
  // (5) Dialogue
  // Une fois la connexion établie, le serveur et le client peuvent s'échanger des messages
  // envoyés par write() ou send() et lus par read()ou recv() selon un protocole établi.
  // Ici nous utiliseront le protocole suivant : le client écrit et le serveur lit.
  // ######################################################
  while(1) {
    char buff[TAILLE_BUFF];
    size_t nbo;

    nbo = recv(t, buff, TAILLE_BUFF, 0);
    if (nbo < 0) {
      perror("erreur à la réception");
    }
    if (nbo == 0) {
      // C'est fini
      break;
    }
    printf("recu : %s\n", buff);

  }

  // ######################################################
  // (6) Fermeture de la connexion
  // ######################################################
  if (close(s)< 0) {
    perror("Problème à la fermeture du socket d'attente");
  }
  if (close(t)< 0) {
    perror("Problème à la fermeture du socket de discution");
  }
#if defined (WIN32)
  // Ceci est du code spécifique à windows
  WSACleanup();
#endif

  fprintf(stdout, "bye\n");

  return 0;
}


