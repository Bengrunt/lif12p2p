/*********************************************************************************************
 * \file socket.c
 * \author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * \since 19/03/2009
 * \version 20/04/2009
 * \brief Projet créé dans le cadre de l'UE Lif12 de 3ème année de licence d'informatique.
 *          Module de gestion des sockets. Basé sur les sources données par F. Rico.
 ********************************************************************************************/


/****************************
 * Fichiers d'en-tête inclus
 ****************************/

#include "socket.h"


/***********************************
 * Variables globales et constantes
 ***********************************/

/**
 * \def INVALID_SOCKET
 */
#define INVALID_SOCKET  -1

/**
 * \def SOCKET_ERROR
 */
#define SOCKET_ERROR    -1


/************************************
 * Fonctions et procédures du module
 ************************************/

/**
 * \fn Socket creationSocket( )
 * \brief Procédure de création d'une socket.
 * \return La socket créée est retournée. -1 en cas d'erreur.
 */
Socket creationSocket( )
{
    #if defined (WIN32)
    /* Ceci est du code spécifique à windows */
        WSADATA WSAData;
        int erreur = WSAStartup( MAKEWORD( 2, 0 ), &WSAData );
        if ( erreur )
        {
            perror( "< ERREUR SOCKET > Problème avec l'initialisation des sockets windows" );
            exit( 1 );
        }
    #endif

    Socket s; /* La socket à créer */


    /* Creation de la socket */
    /* On commence par créer une socket de connexion (que nous appellerons s) */
    s = socket( AF_INET, SOCK_STREAM, 0 );
                /* AF_INET pour IPv4 */
                /* SOCK_STREAM signifie TCP,*/
                /* 0 il n'y a pas de protocole à choisir */

    if ( s == INVALID_SOCKET )
    {
        perror( "< ERREUR SOCKET > Problème à la creation de la socket" );
        exit( 1 );
    }
/*    printf ( "Socket %d >> La socket %d est maintenant ouverte en mode TCP/IP.\n", s, s ); */

    return s;
}


/**
 * \fn void definitionNomSocket( Socket s, int port )
 * \brief Procédure de définition de nom de socket.
 * \param [in] s Socket que l'on va lier à son numéro de port.
 * \param [in] port Numéro de port auquel on va lier la socket.
 */
void definitionNomSocket( Socket s, int port )
{
    int sock_err;       /* Une variable pour stocker les erreurs */
    SOCKADDR_IN sin;    /* La structure de description d'adresse */
                        /* Elle dépend du réseau que nous allons utiliser */
                        /* SOCKADDR_IN = Internet IPV4 */
                        /* SOCKADDR_IN6 = Internet IPV6 */
                        /* SOCKADDR_atalk = apple talk */


    /* Définition d'un nom externe */
    /* Pour être atteinte, la socket doit avoir un nom (couple (adresse, port)) */
    sin.sin_family         = AF_INET;               /* IPv4 */
    sin.sin_port           = htons ( port );          /* Le port d'écoute */
    sin.sin_addr.s_addr    = htonl ( INADDR_ANY );    /* Si la machine a plusieurs adresses on les écoute toutes */

    sock_err = bind ( s, ( SOCKADDR* ) &sin, sizeof( sin ) );
    if ( sock_err < 0 )
    {
        perror( "< ERREUR SOCKET > Problème lors du bind( )" );
        close( s );
        exit( 1 );
    }

    printf ( "Socket %d >> La socket %d est maintenant en attente sur le port %u.\n", s, s, port );

    /* Attente de connexion */
    sock_err = listen ( s, 5 );
    if ( sock_err < 0 )
    {
        perror( "< ERREUR SOCKET > Problème lors du listen( )" );
        close( s );
        exit( 1 );
    }
    /* listen ne bloque pas, à partir de là 5 demandes de conexion peuvent arriver sans que le serveur les accepte ou ne les rejette */
    /* changement de statut de la socket en non bloquante */
    fcntl(s, F_SETFL, O_NONBLOCK);
}

/**
 * \fn Socket acceptationConnexion( Socket s )
 * \brief Fonction d'acceptation d'une connexion sur une socket.
 * \param [in] s Socket sur laquelle on veut accepter la connexion.
 * \return Renvoie la socket sur laquelle la connexion est acceptée, qui vaut -1 si l'acceptation de connexion échoue.
 */
Socket acceptationConnexion( Socket s )
{
    /* Acceptation d'une connexion */
    /* Le serveur accepte l'une des demandes arrivées depuis le listen ou attend s'il n'y en a pas */
    Socket t;
    SOCKADDR_IN tadr;
    size_t recsize = sizeof( tadr );

    t = accept ( s, ( SOCKADDR* ) &tadr, &recsize );    /* s : la socket d'attente */
                                                        /* tadr : la structure où on va stocker les infos sur le client */
                                                        /* recsize : donnée: la taille de tadr (pour éviter les dépassements) */
                                                        /*           resultat: la taille de ce qui est réellement mis dans tadr */
    if (t > 0)
    {
        printf( "Socket %d >> Connection de %s sur le port %d.\n", s, inet_ntoa( tadr.sin_addr ), htons( tadr.sin_port ) );
    }

    return t;
}

/**
 * \fn int demandeConnexionSocket( Socket s, char* nomServeur, int port )
 * \brief Fonction de demande de connexion sur une socket.
 * \param [in] s La socket avec laquelle on essaie de se connecter à un serveur distant.
 * \param [in] nomServeur Le nom du serveur (ou son adresse IP).
 * \param [in] port Le numéro de port du serveur.
 * \return Renvoie 0 si tout se passe bien, -1 en cas d'échec de la connexion.
 */
int demandeConnexionSocket( Socket s, char* nomServeur, int port )
{
    struct hostent* hp;    /* Pour obtenir l'adresse du serveur à partir de son nom */
    SOCKADDR_IN sin;       /* La structure de description d'adresse */
                           /* Elle dépend du réseau que nous allons utilisé */
                           /* SOCKADDR_IN = Internet IPV4 */
                           /* SOCKADDR_IN6 = Internet IPV6 */
                           /* SOCKADDR_atalk = apple talk */

    /* Préparation de la connexion */
    /* Ensuite il faut retrouver l'adresse IP de l'hôte (ordinateur) hébergeant le serveur à l'aide de la fonction gethostbyname() */
    hp = gethostbyname( nomServeur );

    /* Definition de l'adresse du serveur */
    sin.sin_family = AF_INET;
    sin.sin_port = htons( port ); /* htons sert à transformer l'entier en entier 16bits */
    memcpy( &sin.sin_addr, hp->h_addr_list[0], hp->h_length );  /* On copie le résultat de gethostbyname au bon endroit */
                                                                /* Si on connait l'addresse IP On peut aussi utiliser "sin.sin_addr.s_addr = inet_addr("127.0.0.1");" */

    /* Demande de connexion */
    /* Enfin, établir la connexion par la fonction connect( ) */
    if( connect( s, ( SOCKADDR* ) &sin, sizeof( sin ) ) < 0 )
    {
        perror( "< ERREUR SOCKET > Problème lors du connect( )" );
        return -1;
    }
/*    printf( "Socket %d >> Connection à %s sur le port %d.\n", s, inet_ntoa( sin.sin_addr ), htons( sin.sin_port ) );
    printf( "Socket %d >> Envoi de données sur la socket %d.\n ", s, s ); */
    return 0;
}

/**
 * \fn void ecouteSocket( Socket s, char* buff, int taille_buff )
 * \brief Fonction de capture de message entrant sur une socket.
 * \param [in] s La socket sur laquelle on écoute les messages entrants.
 * \param [out] buff La chaine de caractères stockant le message capturé.
 * \param [in] taille_buff La taille de la chaine de caractères stockant le message capturé.
 * \return Renvoie 0 si tout se passe bien, -1 en cas d'échec de capture du message.
 */
int ecouteSocket( Socket s, char* buff, int taille_buff )
{
    int res;
    res = 0;
    while (res != taille_buff)
    {
        if( (res = res + recv( s, &(buff[res]), (taille_buff - res), 0 )) < 0 )
        {
            perror( "< ERREUR SOCKET > Problème lors du recv( )" );
            return -1;
        }
    }
    return 0;
}

/**
 * \fn int ecritureSocket( Socket s, char* buff, int taille_buff )
 * \brief Fonction d'envoi de message sur une socket.
 * \param [in] s La socket sur laquelle on envoie le message.
 * \param [in] buff Le message que l'on veut envoyer.
 * \param [in] taille_buff La taille de la chaine de caractères stockant le message à envoyer.
 * \return Retourne 0 si tout se passe bien, -1 en cas d'échec d'envoi du message.
 */
int ecritureSocket( Socket s, char* buff, int taille_buff )
{
    int sock_err; /* Une variable pour stocker les erreurs */

    sock_err = send( s, ( void* ) buff, taille_buff, MSG_NOSIGNAL );
            /* s la socket sur laquelle on ecrit */
            /* buff le message écrit */
            /* taille_buff la longueur du mesage */
            /* MSG_NOSIGNAL lorsque la connexion est brisée, send renvoie une erreur et ne génère pas de signaux (que je n'ai pas envie de traiter) */

    if ( sock_err < 0 )
    {
        perror( "< ERREUR SOCKET > Problème lors du send( )" );
        return -1;
    }
    return 0;
}

/**
 * \fn void clotureSocket( Socket s )
 * \brief Procédure de fermeture d'une socket.
 * \param [in] s La socket que l'on ferme.
 */
void clotureSocket( Socket s )
{
    /* Fermeture de la connexion */
    if ( close( s ) < 0 )
    {
        perror( "< ERREUR SOCKET > Problème à la fermeture de la socket" );
    }
    #if defined (WIN32)
        /* Ceci est du code spécifique à windows */
        WSACleanup( );
    #endif
}

/*****************
 * Fin de Fichier
 *****************/
