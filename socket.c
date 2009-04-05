/**
 * @file: socket.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 19/03/2009
 * @version: 26/03/2009
 */

#include "socket.h"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define NOM_SERVEUR "localhost"
#define PORT_SERVEUR 8888
#define TAILLE_BUFF 100

Socket creationSocket()
{
    #if defined (WIN32)
    /* Ceci est du code spécifique à windows */
        WSADATA WSAData;
        int erreur = WSAStartup(MAKEWORD(2,0), &WSAData);
        if (erreur)
        {
            perror("probleme avec l'initialisation des sockets windows");
            exit(1);
        }
    #endif

    Socket s; /* La socket à créer */

    /* ######################################################
    // (1) Creation du socket (1)
    // celui-ci commence par créer un socket de connexion (que nous appellerons s)
    // #####################################################*/
    s = socket(AF_INET, SOCK_STREAM, 0); /* AF_INET pour IPv4 */
                                       /* SOCK_STREAM signifie TCP,*/
                                       /* 0 il n'y a pas de protocole à choisir */
    if (s == INVALID_SOCKET)
    {
        perror("Erreur à la creation du socket");
        exit(1);
    }
    printf ("Le socket %d est maintenant ouverte en mode TCP/IP\n", s);

    return s;
}

void definitionNomSocket(Socket s, int port)
{
    int sock_err; /* Une variable pour stocker les erreurs */
    SOCKADDR_IN sin; /* La structure de description d'adresse */
                   /* Elle dépend du réseau que nous allons utilisé */
                   /* SOCKADDR_IN = Internet IPV4 */
                   /* SOCKADDR_IN6 = Internet IPV6 */
                   /* SOCKADDR_atalk = apple talk */

    /* ######################################################
    // (2.1) Définition d'un nom externe
    // Pour être atteind, le socket doit avoir un nom (couple adresse/port)
    // #####################################################*/
    sin.sin_family         = AF_INET;               /* IPv4 */
    sin.sin_port           = htons (port);          /* Le port d'écoute */
    sin.sin_addr.s_addr    = htonl (INADDR_ANY);    /* Si la machine a plusieurs adresses */
                                                    /* on les écoute toutes */
                                                    /* Voir le code du client pour d'autre forme */
    sock_err = bind (s, (SOCKADDR *) &sin, sizeof(sin));
    if (sock_err < 0)
    {
        perror("bind");
        close(s);
        exit(1);
    }

    printf ("Le socket %d est maintenant en attente sur le port %u\n", s, port);

    /* ######################################################
    // (2.2) Attente de connexion
    //
    // #####################################################*/
    sock_err = listen (s, 5);
    if (sock_err < 0)
    {
        perror("listen");
        close(s);
        exit(1);
    }
    /* listen ne bloque pas, à partir de la 5 demande de conexion peuvent arriver */
    /* sans que le serveur les accepte ou les rejette */
}

Socket acceptationConnexion(Socket s)
{
    /* ######################################################
    // (4) Acceptation d'une connexion
    // Le serveur accepte l'une des demande arrivée depuis le listen ou
    // attend s'il n'y en a pas
    // #####################################################*/
    Socket t;
    SOCKADDR_IN tadr;
    size_t recsize = sizeof(tadr);

    t = accept (s, (SOCKADDR *) &tadr, &recsize);   /* s : la socket d'attente */
                                                    /* tadr : la structure ou on va stocker les info sur le client */
                                                    /* recsize : donnée = la taille de tadr (pour éviter le dépassement) */
                                                    /*          resultat = la taille de ce qui est réellement mis dans tadr */

    printf("Connection de %s sur le port %d\n", inet_ntoa (tadr.sin_addr), htons(tadr.sin_port));
    return t;
}

int demandeConnexionSocket(Socket s, char* nomServeur, int port)
{
    struct hostent *hp;    /* Pour obtenir l'adresse du serveur à partir de son nom */
    SOCKADDR_IN sin;       /* La structure de description d'adresse */
                           /* Elle dépend du réseau que nous allons utilisé */
                           /* SOCKADDR_IN = Internet IPV4 */
                           /* SOCKADDR_IN6 = Internet IPV6 */
                           /* SOCKADDR_atalk = apple talk */

    /* ######################################################
    // (3) Préparation de la connexion
    // ensuite il faut retrouver l'adresse IP de l'hôte (ordinateur) hébergeant
    // le serveur à l'aide de la fonction gethostbyname()
    // #####################################################*/
    hp = gethostbyname(nomServeur); /* Pour le test c'est "localhost" */

    /* Definition de l'adresse du serveur */
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port); /* htons sert à transformer l'entier en entier 16bits */
    memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length); /* On copie le résultat de gethostbyname */
                                                   /* au bon endroit */
    /* Si on connait l'addresse IP On peut aussi utiliser */
    /* sin.sin_addr.s_addr = inet_addr("127.0.0.1"); */

    /* ######################################################
    // (4) Demande de connexion
    //  enfin, établir la connexion par la fonction connect()
    // #####################################################*/

    if(connect(s, (SOCKADDR *)&sin, sizeof(sin)) <0)
    {
        perror("connect");
        return 1;
    }
    printf("Connection à %s sur le port %d\n", inet_ntoa (sin.sin_addr), htons(sin.sin_port));
    printf("envoie de données sur le socket %d\n ",s);
    return 0;
}

void ecouteSocket(Socket s, char* buff)
{
    if( recv(s, buff, TAILLE_BUFF, 0) < 0)
    {
        perror("erreur à la réception");
    }
}

int ecritureSocket(Socket s, char* buff)
{
    int sock_err; /* Une variable pour stocker les erreurs */

    sock_err = send(s, (char*)buff, (strlen(buff)+1)*sizeof(char), MSG_NOSIGNAL);
        /* s le socket sur laquel on ecrit */
        /* buff le message écrit */
        /* (strlen(buff)+1)*sizeof(char) la longueur du mesage */
        /* MSG_NOSIGNAL lorsque la connexion est brisée, send renvoie une erreur et */
        /*              ne génère pas de signaux (que je n'ai pas envie de traiter) */
    if (sock_err < 0)
    {
        perror("erreur dans le send");
        return 1;
    }
    return 0;
}

void clotureSocket(Socket s)
{
    /* ######################################################
    // (6) Fermeture de la connexion
    // #####################################################*/
    if (close(s)< 0)
    {
        perror("Problème à la fermeture du socket d'attente");
    }
    #if defined (WIN32)
        /* Ceci est du code spécifique à windows */
        WSACleanup();
    #endif
}

/*****************
* Fin de Fichier
*****************/
