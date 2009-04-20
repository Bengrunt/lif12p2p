/*********************************************************************
 * \file annuaire.h
 * \author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * \since 16/03/2009
 * \version 19/04/2009
 * \brief Projet de transfert de fichier P2P centralisé par annuaire.
 ********************************************************************/

/****************************
 * Fichiers d'en-tête inclus
 ****************************/

#include "annuaire.h"

/***********************************
 * Variables Globales et constantes
 ***********************************/

/**
 * \def TAILLE_BUFF
 * \brief Taille de buffer standard.
 */
#define TAILLE_BUFF         200

/**
 * \def TAILLE_BUFF_LAR
 * \brief Taille de buffer large.
 */
#define TAILLE_BUFF_LAR     TAILLE_BUFF/2

/**
 * \def TAILLE_BUFF_MED
 * \brief Taille de buffer medium.
 */
#define TAILLE_BUFF_MED     TAILLE_BUFF/5

/**
 * \def TAILLE_BUFF_SM
 * \brief Taille de buffer small.
 */
#define TAILLE_BUFF_SM      TAILLE_BUFF/10

/**
 * \def TAILLE_BUFF_VSM
 * \brief Taille de buffer very small.
 */
#define TAILLE_BUFF_VSM     TAILLE_BUFF/20

/**
 * \def TABDYN_AUGM_VAL
 * \brief Taux d'augmentation de la taille des tableaux dynamiques.
 */
#define TABDYN_AUGM_VAL     20

/**
 * \var BddServeurs* serveurs
 * \brief Pointeur sur la base de données référençant les serveurs connus.
 */
BddServeurs* serveurs;

/**
 * \var BddFichiers* fichiers
 * \brief Pointeur sur la base de données référençant les fichiers connus.
 */
BddFichiers* fichiers;

/**
 * \var unsigned int generateurIdServeur
 * \brief Compteur de génération des idServeur.
 */
unsigned int generateurIdServeur;

/**
 * \var unsigned int generateurIdFichier
 * \brief Compteur de génération des idFichier.
 */
unsigned int generateurIdFichier;

/**
 * \var unsigned int global_nbthreads
 * \brief Compteur du nombre de threads gérés par l'annuaire.
 */
unsigned int global_nbthreads;

/**
 * \var int global_exit
 * \brief Booléen de sortie du programme.
 */
int global_exit;


/************************************
 * Fonctions et procédures du module
 ************************************/

/**
 * \fn int initialisationAnnuaire( )
 * \brief Fonction d'initialisation de l'annuaire.
 * \warning Modifie le contenu des variables globales serveurs et fichiers.
 * \return Renvoie 0 si tout se passe bien, -1 en cas de problème.
 */
int initialisationAnnuaire( )
{
    /* Variables */
    unsigned int i,j; /* Itérateurs */

    /* Creation des bases de données de serveurs et initialisation aux valeurs par défaut */
    serveurs = malloc( sizeof( BddServeurs ) );
    if ( serveurs == NULL )
    {
        perror( "< ERREUR ANNUAIRE > Echec de l'allocation en mémoire de la base de donnée des serveurs.\n" );
        return -1;
    }
    serveurs->nbInfoServeurs = 0;
    serveurs->capaTabInfoServeurs = 1;
    serveurs->tabInfoServeurs = malloc( serveurs->capaTabInfoServeurs * sizeof( InfoServeurs* ) );
    for ( i = 0 ; i < serveurs->capaTabInfoServeurs ; i++ )
    {
    	serveurs->tabInfoServeurs[i] = NULL;
    }
    pthread_mutex_init( &serveurs->verrou_bddserv_r, NULL );
    pthread_mutex_init( &serveurs->verrou_bddserv_w, NULL );

    /* Creation des bases de données de fichiers et initialisation aux valeurs par défaut */
    fichiers = malloc( sizeof( BddFichiers ) );
    if ( fichiers == NULL )
    {
        perror( "< ERREUR ANNUAIRE > Echec de l'allocation en mémoire de la base de donnée des fichiers.\n" );
        return -1;
    }
    fichiers->nbFichiers = 0;
    fichiers->capaTabFichiers = 1;
    fichiers->tabFichiers = malloc( fichiers->capaTabFichiers * sizeof( Fichier* ) );
    for ( j = 0 ; j < fichiers->capaTabFichiers ; j++ )
    {
        fichiers->tabFichiers[j] = NULL;
    }
    pthread_mutex_init( &fichiers->verrou_bddfich_r, NULL );
    pthread_mutex_init( &fichiers->verrou_bddfich_w, NULL );

    /* Initialisation des générateurs d'identificateurs */
    generateurIdServeur = 0;
    generateurIdFichier = 0;

    /* Initialisation des variables globales d'utilisation de l'annuaire */
    global_nbthreads = 0;
    global_exit = 0;

    return 0;
}


/**
 * \fn Socket initialiseSocketEcouteAnnuaire( int portAnnuaire )
 * \brief Fonction d'initialisation de la socket d'écoute de l'annuaire.
 * \param [in] portAnnuaire Numéro de port sur lequel on crée la socket d'écoute.
 * \return Renvoie la socket créée ou -1 en cas d'échec.
 */
Socket initialiseSocketEcouteAnnuaire( int portAnnuaire )
{
    /* On déclare la socket pour l'annuaire */
    Socket socketEcouteAnnuaire;

    /* On la crée */
    socketEcouteAnnuaire = creationSocket( );
    if ( socketEcouteAnnuaire < 0 )
    {
        perror( "< ERREUR ANNUAIRE > Problème lors de la création de la socket d'écoute de l'annuaire." );
        return -1;
    }

    /* On lie la socket au numero de port portAnnuaire */
    definitionNomSocket( socketEcouteAnnuaire, portAnnuaire );

    printf( "La socket d'écoute de l'annuaire a été créée sur le port %d.\n", portAnnuaire );
    return socketEcouteAnnuaire;
}


/**
 * \fn void lectureClavier( )
 * \brief Procedure de lecture des entrées clavier de l'utilisateur.
 */
void lectureClavier(  )
{
    /* Variables */
    char* buff;

    /* Allocation mémoire de la chaine de caractères */
    buff = malloc( TAILLE_BUFF * sizeof( char ) );

    /* Boucle infinie de lecture clavier */
    do
    {
        printf( "=======================================================================\n" );
        printf( "Pour arrêter l'annuaire, tapez à tout moment \"quit\" dans le terminal.\n" );
        printf( "=======================================================================\n" );

        scanf( "%s", buff );

    } while( strcmp( buff, "quit" ) != 0 );

    global_exit = 1;

    /* Libération mémoire de la chaine de caractères */
    free( buff );

}

/**
 * \fn int traiteMessage( Socket arg )
 * \brief Fonction globale de traitement de message reçu.
 * \param [in] arg Socket sur laquelle le message est arrivé.
 * \return Renvoie 0 si tout se passe bien ou -1 en cas de problème.
 */
int traiteMessage( Socket arg )
{
    /* Variables */
    char* buff; /* Message lu sur la socket */

    int type_message; /* Type de message */
    int fin_thread; /* Booleen qui décide de l'arret du thread */

    /* Initialisation des booléens */
    fin_thread = 0;

    /* Allocation mémoire des chaines de caractères */
    buff = malloc( TAILLE_BUFF * sizeof( char ) );

    /* Boucle de traitement des messages */
    while ( !fin_thread )
    {
        /* On écoute sur la socket arg */
        ecouteSocket( arg, buff, TAILLE_BUFF );

        if ( strcmp( buff, "") != 0 ) /* Si on lit quelquechose sur la socket */
        {

            /* On analyse le type du message reçu et on agit en conséquence */
            if ( sscanf ( buff, "%d", &type_message ) != 1 ) /* Si il est corrompu on l'ignore */
            {
                fprintf( stderr, "< ERREUR ANNUAIRE > Message ignoré, impossible de l'analyser.\n Contenu du message: < %s > \n", buff );

                /* Libération des chaines de caractères */
                free( buff );

                /* Fermeture de la socket arg */
                clotureSocket( arg );

                return -1;
            }
            else /* Sinon on lance la procédure correspondant au type de message reçu. */
            {
                switch ( type_message )
                {
                    case 31: /* Demande d'un fichier */
                        traiteDemandeFichierClient( arg, buff );
                        break;
                    case 32: /* Demande d'un bloc */
                        traiteDemandeBlocClient( arg, buff );
                        break;
                    case 33: /* Fin de communication avec une entité */
                        traiteFinCommunicationClient( arg, buff );
                        fin_thread = 1;
                        break;
                    case 51: /* Disponibilité d'un bloc */
                        traiteBlocDisponibleServeur( buff );
                        break;
                    case 52: /* Arret d'un serveur */
                        traiteArretServeur( buff );
                        fin_thread = 1;
                        break;
                    case 53: /* Indication de charge serveur (PAS IMPLEMENTE) */
                        /* traiteChargeServeur( arg, buff ); */
                        break;
                    case 54: /* Demande IDServeur */
                        traiteDemandeIdServeur( arg, buff );
                        break;
                    case 55: /* Demande IDFichier */
                        traiteDemandeIdFichier( arg, buff );
                        break;
                    case 71: /* Indiquation que l'on a envoyé des messages au mauvais destinataire sur la socket donc fermeture */
                        fin_thread = 1;
                        break;
                    default: /* Un message géré par le réseau a bien été reçu mais inadapté donc la connexion doit être terminé car ce ne sont pas les bons interlocuteurs. */
                        traiteMessageErr( arg );
                        fin_thread = 1;
                }
            }
        }
    }

    /* Libération des chaines de caractères */
    free( buff );

    /* Fermeture de la socket arg */
    clotureSocket( arg );

    global_nbthreads--;
    printf( "Il y a actuellement %u thread(s) de discussion ouvert(s).\n", global_nbthreads );

    return 0;
}


/**
 * \fn void traiteDemandeFichierClient( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (31) "Demande de fichier d'un client".
 * \param [in] s La socket sur laquelle la demande de fichier client a été émise.
 * \param [in] mess Le message contenant la demande de fichier client à traiter.
 */
void traiteDemandeFichierClient( Socket s, char* mess )
{
    /* Variables */
    int fichTrouve; /* Booléen indiquant si le fichier a été trouvé dans la BDD des fichiers */
    int type_message; /* type du message : ici 31 */
    unsigned int random; /* Valeur aléatoire */
    unsigned int i,j,k,l; /* itérateurs */

    char* buff; /* tampon pour écriture du message */
    char* var_nomDeFichier; /* nom du fichier demandé */
    char* str_idFichier; /* transformation de l'identificateur du fichier en chaine de caractères */
    char* str_nbBlocs; /* transformation du nombre de blocs du fichier en chaine de caractères */
    char* str_numBloc; /* transformation du numéro du bloc du fichier en chaine de caractères */
    char* str_idServeur; /* transformation de l'identificateur de serveur en chaine de caractères */
    char* str_numPort; /* transformation du numéro de port du serveur en chaine de caractères */

    /* On initialise le booleen de réussite :p */
    fichTrouve = 0; /* On a pas encore trouvé le fichier */

    /* Allocations mémoire des chaines de caractère */
    var_nomDeFichier = malloc( TAILLE_BUFF_LAR * sizeof( char ) );
    str_idFichier = malloc( TAILLE_BUFF_VSM * sizeof( char ) );
    str_idServeur = malloc( TAILLE_BUFF_VSM * sizeof( char ) );
    str_nbBlocs = malloc(TAILLE_BUFF_VSM * sizeof( char ) );
    str_numBloc = malloc(TAILLE_BUFF_VSM * sizeof( char ) );
    str_numPort = malloc(TAILLE_BUFF_VSM * sizeof( char ) );

    /* On récupère le contenu du message */
    /* Doit être de la forme "31 nomDeFichier" */
    if ( sscanf( mess, "%d %s", &type_message, var_nomDeFichier ) < 2 )
    {
        fprintf( stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess );
    }

    /* On vérrouille en ecriture les BDD des fichiers et serveurs avant la lecture */
    pthread_mutex_lock( &(fichiers->verrou_bddfich_w) );
    pthread_mutex_lock( &(serveurs->verrou_bddserv_w) );

    /* On recherche dans la BDD des fichiers si on possède celui qui est demandé */
    for ( i = 0 ; i < fichiers->capaTabFichiers ; i++ ) /* On parcoure le tableau des fichiers */
    {
        if( fichiers->tabFichiers[i] != NULL ) /* Si il y a un pointeur dans la case du tableau on peut lancer le traitement */
        {
            /******** Le fichier demandé est trouvé ********/
            if ( strcmp( fichiers->tabFichiers[i]->nomFichier, var_nomDeFichier ) == 0 )
            {
                /* Pour chaque bloc constituant du fichier on envoie un message indiquant où le télécharger au client */
                buff = malloc( TAILLE_BUFF * sizeof( char ) );

                for ( j = 0 ; j < fichiers->tabFichiers[i]->nbBlocs ; j++ )
                {
                    if ( fichiers->tabFichiers[i]->tabBlocs[j] != NULL ) /* Si il y a un pointeur dans la case du tableau on peut lancer le traitement */
                    {
                        /* On envoie pour chaque bloc un message indiquant au client où télécharger chaque bloc */
                        /* Message de la forme : "11 idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur" */
                        strcpy( buff, "11 " );
                        sprintf( str_idFichier, "%u", fichiers->tabFichiers[i]->idFichier );
                        strcat( buff, str_idFichier );
                        strcat( buff, " " );
                        strcat( buff, fichiers->tabFichiers[i]->nomFichier );
                        strcat( buff, " " );
                        sprintf( str_nbBlocs, "%d", fichiers->tabFichiers[i]->nbBlocs );
                        strcat( buff, str_nbBlocs );
                        strcat( buff, " " );
                        sprintf( str_numBloc, "%d", j );
                        strcat( buff, str_numBloc );
                        strcat( buff, " " );

                        /* On a des serveurs proposant ce bloc */
                        if ( fichiers->tabFichiers[i]->tabBlocs[j]->nbServeurs > 0 )
                        {
                            /* On fait un rand parmi les serveurs pour répartir leur charge en attribuant les requetes aléatoirement */
                            random = ( unsigned int ) rand( ) % fichiers->tabFichiers[i]->tabBlocs[j]->nbServeurs; /* numero du serveur sur qui la requete sera envoyée par le client */
                            k = 0;
                            l = 1;

                            while ( l <= random )
                            {
                                if( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k] != NULL ) /* On saute les cases du tableau qui sont vides */
                                {
                                    l++; /* compteur de cases remplies */
                                }

                                k++; /* itérateur de déplacement dans le tableau */
                            }

                            sprintf( str_idServeur, "%u", fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k]->infos->idServeur );
                            strcat( buff, str_idServeur );
                            strcat( buff, " " );
                            strcat( buff, fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k]->infos->adresseServeur );
                            strcat( buff, " " );
                            sprintf( str_numPort, "%d", fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k]->infos->numPort );
                            strcat( buff, str_numPort );

                        }
                        /* On a pas de serveur proposant ce bloc, on envoie dans la réponse -1 -1 pour indiquer au client que des blocs sont manquants */
                        else
                        {
                            strcat( buff, " -1 -1" );
                        }

                        /* Envoi du message sur la socket */
                        ecritureSocket( s, buff, TAILLE_BUFF );
                    }
                }
                /* Libération de la chaine après utilisation */
                free( buff );
                /* On a bien trouvé le fichier et envoyé les messages */
                fichTrouve = 1;
            }
        }
    }

    /* On dévérouille en écriture les BDD des fichiers et serveurs après la lecture */
    pthread_mutex_unlock( &(fichiers->verrou_bddfich_w) );
    pthread_mutex_unlock( &(serveurs->verrou_bddserv_w) );

    /* Si on a trouvé le fichier et qu'on a fini de répondre on l'indique au client  */
    if( fichTrouve )
    {
        /* On envoie un message de fin de transmission au client */
        /* Message de la forme: "13 fin" */
        buff = malloc( TAILLE_BUFF * sizeof( char ) );

        strcpy( buff, "13 fin" );

        /* Envoi du message sur la socket */
        ecritureSocket( s, buff, TAILLE_BUFF );

        /* Libération de la chaine après utilisation */
        free( buff );
    }

    /******** Le fichier n'est pas dans la BDD des fichiers, on envoie une réponse défavorable au client *********/
    if ( !fichTrouve )
    {
        /* On envoie un message de réponse défavorable au client */
        /* Message de la forme: "02 erreur nomDeFichier" */
        buff = malloc( TAILLE_BUFF * sizeof( char ) );

        strcpy( buff, "12 " );
        strcat( buff, var_nomDeFichier );

        /* Envoi du message sur la socket */
        ecritureSocket( s, buff, TAILLE_BUFF );

        /* Libération de la chaine après utilisation */
        free( buff );

        /* On envoie un message de fin de transmission au client */
        /* Message de la forme: "13 fin" */
        buff = malloc( TAILLE_BUFF * sizeof( char ) );

        strcpy( buff, "13 fin" );

        /* Envoi du message sur la socket */
        ecritureSocket( s, buff, TAILLE_BUFF );

        /* Libération de la chaine après utilisation */
        free( buff );
    }

    /* Libérations mémoire de chaines de caractère */
    free( var_nomDeFichier );
    free( str_idFichier );
    free( str_idServeur );
    free( str_nbBlocs );
    free( str_numBloc );
    free( str_numPort );
}


/**
 * \fn void traiteDemandeBlocClient( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (32) "Demande de bloc d'un client".
 * \param [in] s La socket sur laquelle la demande de bloc client a été émise.
 * \param [in] mess Le message contenant la demande de bloc client à traiter.
 */
void traiteDemandeBlocClient( Socket s, char* mess )
{
    /* Variables */
    int type_message; /* type du message : ici 32 */
    unsigned int var_idFichier; /* identificateur du fichier */
    unsigned int var_numBloc; /* numero du bloc demandé */
    unsigned int random; /* Valeur aléatoire */
    unsigned int i,j,k; /* Itérateurs */


    int fichTrouve; /* Booléen indiquant si le fichier a été trouvé dans la BDD des fichiers */

    char* buff; /* tampon pour écriture du message */
    char* var_nomDeFichier; /* nom du fichier auquel appartient le bloc */
    char* str_idFichier; /* transformation de l'identificateur du fichier en chaine de caractères */
    char* str_nbBlocs; /* transformation du nombre de blocs du fichier en chaine de caractères */
    char* str_numBloc; /* transformation du numéro du bloc du fichier en chaine de caractères */
    char* str_idServeur; /* transformation de l'identificateur de serveur en chaine de caractères */
    char* str_numPort; /* transformation du numéro de port du serveur en chaine de caractères */

    /* Initialisation des booléens */
    fichTrouve = 0;

    /* Allocations mémoire des chaines de caractère */
    var_nomDeFichier = malloc( TAILLE_BUFF_LAR * sizeof( char ) );
    str_idFichier = malloc( TAILLE_BUFF_VSM * sizeof( char ) );
    str_idServeur = malloc( TAILLE_BUFF_VSM * sizeof( char ) );
    str_nbBlocs = malloc( TAILLE_BUFF_VSM * sizeof( char ) );
    str_numBloc = malloc( TAILLE_BUFF_VSM * sizeof( char ) );
    str_numPort = malloc( TAILLE_BUFF_VSM * sizeof( char ) );

    /* On récupère le contenu du message */
    /* Doit être de la forme "32 idFichier nomDeFichier numeroDeBloc" */
    if ( sscanf( mess, "%d %u %s %u", &type_message, &var_idFichier, var_nomDeFichier, &var_numBloc ) < 4 )
    {
        fprintf( stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess );
        exit( 1 );
    }

    /* On vérrouille en ecriture les BDD des fichiers et serveurs avant la lecture */
    pthread_mutex_lock( &(fichiers->verrou_bddfich_w) );
    pthread_mutex_lock( &(serveurs->verrou_bddserv_w) );

    /* On recherche ans la BDD des fichiers si on possède celui qui est demandé */
    for ( i = 0 ; i < fichiers->capaTabFichiers ; i++ )
    {
        if ( fichiers->tabFichiers[i] != NULL ) /* Si il y a un pointeur dans la case du tableau on peut lancer le traitement */
        {
            /******** Le fichier demandé est trouvé ********/
            if ( strcmp( fichiers->tabFichiers[i]->nomFichier, var_nomDeFichier ) == 0 && var_idFichier == fichiers->tabFichiers[i]->idFichier )
            {
                /* On envoie un message indiquant où télécharger le bloc demandé au client */
                buff = malloc( TAILLE_BUFF * sizeof( char ) );

                /* Message de la forme : "11 bloc idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur" */
                strcpy( buff, "11 " );
                sprintf( str_idFichier, "%u", fichiers->tabFichiers[i]->idFichier );
                strcat( buff, str_idFichier );
                strcat( buff, " " );
                strcat( buff, fichiers->tabFichiers[i]->nomFichier );
                strcat( buff, " " );
                sprintf( str_nbBlocs, "%u", fichiers->tabFichiers[i]->nbBlocs );
                strcat( buff, str_nbBlocs );
                strcat( buff, " " );
                sprintf( str_numBloc, "%u", var_numBloc );
                strcat( buff, str_numBloc );
                strcat( buff, " " );

                /* Si on a des serveurs proposant ce bloc */
                if ( fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->nbServeurs > 0 )
                {
                    /* On fait un rand parmi les serveurs pour répartir leur charge en attribuant les requetes aléatoirement */
                    random = ( unsigned int ) rand( ) % fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->nbServeurs; /* numero du serveur sur qui la requete sera envoyée par le client */
                    j = 0;
                    k = 1;

                    while ( k <= random )
                    {
                        if( fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[j] != NULL ) /* On saute les cases du tableau qui sont vides */
                        {
                            k++; /* compteur de cases remplies */
                        }

                        j++; /* itérateur de déplacement dans le tableau */
                    }

                    sprintf( str_idServeur, "%u", fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[j]->infos->idServeur );
                    strcat( buff, str_idServeur );
                    strcat( buff, " " );
                    strcat( buff, fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[j]->infos->adresseServeur );
                    strcat( buff, " " );
                    sprintf( str_numPort, "%d", fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[j]->infos->numPort );
                    strcat( buff, str_numPort );

                }
                /* On a pas de serveur proposant ce bloc, on envoie dans la réponse -1 -1 pour indiquer au client que des blocs sont manquants */
                else
                {
                    strcat( buff, " -1 -1" );
                }

                /* Envoi du message sur la socket */
                ecritureSocket( s, buff, TAILLE_BUFF );
                /* Libération de la chaine après utilisation */
                free( buff );
                /* On a bien trouvé le fichier et envoyé le message */
                fichTrouve = 1;
            }
        }
    }

    /* On dévérouille en écriture les BDD des fichiers et serveurs après la lecture */
    pthread_mutex_unlock( &fichiers->verrou_bddfich_w );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_w );

    /* Si on a trouvé le fichier et qu'on a fini de répondre on l'indique au client  */
    if( fichTrouve )
    {
        /* On envoie un message de fin de transmission au client */
        /* Message de la forme: "13 fin" */
        buff = malloc( TAILLE_BUFF * sizeof( char ) );

        strcpy( buff, "13 fin" );

        /* Envoi du message sur la socket */
        ecritureSocket( s, buff, TAILLE_BUFF );

        /* Libération de la chaine après utilisation */
        free( buff );
    }

    /* Le fichier n'est pas dans la BDD des fichiers, on envoie une réponse défavorable au client *********/
    if ( !fichTrouve )
    {
        /* On envoie un message de réponse défavorable au client */
        /* Message de la forme: "12 erreur nomDeFichier" */
        buff = malloc( TAILLE_BUFF * sizeof( char ) );

        strcpy( buff, "12 " );
        strcat( buff, var_nomDeFichier );

        /* Envoi du message sur la socket */
        ecritureSocket( s, buff, TAILLE_BUFF );

        /* Libération de la chaine après utilisation */
        free( buff );

        /* On envoie un message de fin de transmission au client */
        /* Message de la forme: "13 fin" */
        buff = malloc( TAILLE_BUFF * sizeof( char ) );

        strcpy( buff, "13 fin" );

        /* Envoi du message sur la socket */
        ecritureSocket( s, buff, TAILLE_BUFF );

        /* Libération de la chaine après utilisation */
        free( buff );
    }

    /* Libérations mémoire de chaines de caractère */
    free( var_nomDeFichier );
    free( str_idFichier );
    free( str_idServeur );
    free( str_nbBlocs );
    free( str_numBloc );
    free( str_numPort );
}


/**
 * \fn void traiteFinCommunicationClientClient( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (33) "Fin de communication".
 * \param [in] s La socket sur laquelle le message de fin de communication client a été émis.
 * \param [in] mess Le message de fin de communication client à traiter.
 */
void traiteFinCommunicationClient( Socket s, char* mess )
{
    /* On traite l'arret du serveur si il n'a pas déjà été fait */
    traiteArretServeur( mess );

    /* Fermeture de la socket ouverte avec le client */
    clotureSocket(s);
}


/**
 * \fn void traiteBlocDisponibleServeur( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (51) "Nouveau bloc disponible sur serveur".
 * \param [in] s La socket sur laquelle le message de nouveau bloc disponible sur serveur a été émis.
 * \param [in] mess Le message de nouveau bloc disponible serveur à traiter.
 */
void traiteBlocDisponibleServeur( char* mess )
{
    /* Variables */
    int type_message; /* Type du message : ici 51 */
    unsigned int var_idFichier; /* identificateur du fichier */
    unsigned int var_nbBlocs; /*  nombre de blocs du fichier */
    unsigned int var_numBloc; /* numero de bloc */
    unsigned int var_idServeur; /* identificateur du serveur */
    unsigned int i,j,k,l,m; /* Itérateurs */

    int servTrouve; /* Booléen */

    Serveur** tempTabServeurs; /* pointeur temporaire pour l'extension de tabServeurs */

    /* Initialisation des booléens */
    servTrouve = 0;

    /* On récupère le contenu du message */
    /* Doit être de la forme "51 idFichier nombreTotalDeBloc numeroDeBloc idServeur" */
    if ( sscanf( mess, "%d %u %u %u %u", &type_message, &var_idFichier, &var_nbBlocs, &var_numBloc, &var_idServeur ) < 5 )
    {
        fprintf( stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess );
    }

    /* On vérouille les BDD des fichiers et serveurs en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock( &fichiers->verrou_bddfich_r );
    pthread_mutex_lock( &fichiers->verrou_bddfich_w );
    pthread_mutex_lock( &serveurs->verrou_bddserv_r );
    pthread_mutex_lock( &serveurs->verrou_bddserv_w );

    /* Mise à jour des BDD des fichiers et des serveurs avec les nouvelles informations */

    /* On parcoure le tableau des fichiers pour trouver celui auquel on doit ajouter une nouvelle reference de serveur ayant un de ses blocs */
    i = 0;
    while (i < fichiers->capaTabFichiers && fichiers->tabFichiers[i]->idFichier != var_idFichier) /* Tant qu'on a pas trouvé le fichier */
    {
        i++;
    }

    if ( fichiers->tabFichiers[i]->idFichier != var_idFichier ) /* /!\ LA BASE DE DONNEES DES FICHIERS EST CORROMPUE */
    {
        fprintf( stderr, "BASE DE DONNEES DES FICHIERS CORROMPUE!\n ARRET DU SERVEUR ANNUAIRE...\n");
        exit( 1 );
    }

    if ( fichiers->tabFichiers[i]->tabBlocs != NULL ) /* Si le fichier est déjà référencé au niveau de ses blocs */
    {
        if ( fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->nbServeurs > 0 ) /* Il y a déjà au moins une source pour le bloc ajouté */
        {
            /* On cherche si le serveur est déjà référencé comme source pour ce bloc */
            for ( j = 0 ; j < fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->capaTabServeurs ; j++ )
            {
                if ( fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[j] != NULL )
                {
                    if ( fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[j]->infos->idServeur == var_idServeur )
                    {
                        servTrouve = 1; /* On a trouvé le serveur, pas besoin de le rajouter une fois de plus */
                        break;
                    }
                }
            }
        }

        if( !servTrouve ) /* Il faut référencer le serveur de ce nouveau bloc car on ne l'a pas trouvé */
        {
            for ( k = 0 ; k < serveurs->capaTabInfoServeurs ; k++ ) /* On cherche où est référencé ce serveur dans la BDD des serveurs */
            {
                if ( serveurs->tabInfoServeurs[k] != NULL )
                {
                    if ( serveurs->tabInfoServeurs[k]->idServeur == var_idServeur )
                        break;
                }
            }

            if( fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->nbServeurs == 0 ) /* Cas où il y a aucun autre serveur référencé pour ce bloc */
            {
                fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[0] = malloc( sizeof( Serveur ) );
                fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[0]->infos = serveurs->tabInfoServeurs[k];
                fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->nbServeurs++;
            }
            else /* Cas où il y a déjà d'autres serveurs référencés */
            {
                if ( fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->nbServeurs == fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->capaTabServeurs ) /* Si tabServeurs est plein on l'agrandit */
                {
                    tempTabServeurs = fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs;
                    fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs = malloc( ( TABDYN_AUGM_VAL + fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->capaTabServeurs ) * sizeof( Serveur* ) );

                    for( l = 0 ; l < fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->capaTabServeurs ; l++ )
                    {
                        fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[l] = tempTabServeurs[l];
                    }

                    fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->capaTabServeurs += TABDYN_AUGM_VAL;

                    for( m = l ; m < fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->capaTabServeurs ; m++ )
                    {
                        fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[m] = NULL;
                    }

                    free( tempTabServeurs );
                }
                else /* Sinon on recherche une case vide dans tabServeurs */
                {
                    l = 0;
                    while ( l < fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->capaTabServeurs )
                    {
                        if ( fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[l] == NULL ) /* Si on trouve une case vide on y référence le serveur */
                        {
                            break;
                        }

                        l++;
                    }
                }

                /* on on ajoute la nouvelle référence dans la case vide trouvée */
                fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[l] = malloc( sizeof( Serveur ) );
                fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[l]->infos = serveurs->tabInfoServeurs[k];
                fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->nbServeurs++;
            }
        }
    }
    else /* C'est le premier référencement de bloc pour ce fichier il faut donc mettre en place le tableau */
    {
        fichiers->tabFichiers[i]->nbBlocs = var_nbBlocs;
        fichiers->tabFichiers[i]->tabBlocs = malloc( var_nbBlocs * sizeof( Bloc* ) );

        for ( j = 0 ; j < var_nbBlocs ; j++ ) /* Pour chaque bloc du fichier on réserve l'emplacement mémoire pour les informations qui arriveront plus tard */
        {
            fichiers->tabFichiers[i]->tabBlocs[j] = malloc( sizeof( Bloc ) );
            fichiers->tabFichiers[i]->tabBlocs[j]->nbServeurs = 0;
            fichiers->tabFichiers[i]->tabBlocs[j]->capaTabServeurs = 1;
            fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs = malloc( fichiers->tabFichiers[i]->tabBlocs[j]->capaTabServeurs * sizeof( Serveur* ) );
            fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[0] = NULL;
        }

        for ( k = 0 ; k < serveurs->capaTabInfoServeurs ; k++ ) /* On cherche où est référencé ce serveur dans la BDD des serveurs */
        {
            if ( serveurs->tabInfoServeurs[k]->idServeur == var_idServeur )
                break;
        }

        fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[0] = malloc( sizeof( Serveur ) );
        fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[0]->infos = malloc( sizeof( InfoServeurs* ) );
        fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->tabServeurs[0]->infos = serveurs->tabInfoServeurs[k];
        fichiers->tabFichiers[i]->tabBlocs[var_numBloc]->nbServeurs++;

    }

    /* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock( &fichiers->verrou_bddfich_r );
    pthread_mutex_unlock( &fichiers->verrou_bddfich_w );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_r );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_w );
}


/**
 * \fn void traiteArretServeur( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (52) "Arrêt de serveur".
 * \param [in] s La socket sur laquelle le message d'arrêt de serveur a été émis.
 * \param [in] mess Le message d'arrêt serveur à traiter.
 */
void traiteArretServeur( char* mess )
{
    /* Variables */
    int type_message; /* type du message reçu : ici 52 */
    unsigned int var_idServeur; /* identificateur du serveur */
    unsigned int i,j,k; /* Itérateurs */

    int sourceExiste; /* booléen */

    /* Initialisation des booléens */
    sourceExiste = 0;


    /* On récupère le contenu du message */
    /* Doit être de la forme "52 idServeur" */
    if ( sscanf( mess, "%d %u", &type_message, &var_idServeur ) < 2 )
    {
        fprintf( stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess );
    }

    /* On vérouille la BDD des fichiers en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock( &fichiers->verrou_bddfich_r );
    pthread_mutex_lock( &fichiers->verrou_bddfich_w );
    pthread_mutex_lock( &serveurs->verrou_bddserv_r );
    pthread_mutex_lock( &serveurs->verrou_bddserv_w );

    /* On supprime de la BDD des fichiers les informations relatives au serveur qui s'arrete */
    for ( i = 0 ; i < fichiers->capaTabFichiers ; i++ ) /* Pour chaque fichier de la BDD des fichiers */
    {
        if ( fichiers->tabFichiers[i] != NULL ) /* Si la case n'est pas vide on lance le traitement */
        {
            for ( j = 0 ; j < fichiers->tabFichiers[i]->nbBlocs ; j++ ) /* Pour chaque bloc du fichier */
            {
                for ( k = 0 ; k < fichiers->tabFichiers[i]->tabBlocs[j]->nbServeurs ; k++ ) /* Pour chaque serveur du bloc */
                {
                    if ( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k] != NULL ) /* Si la case n'est pas vide on lance le traitement */
                    {
                        if ( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k]->infos->idServeur == var_idServeur ) /* Si on trouve le serveur on le supprime */
                        {
                            free( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k]->infos );
                            free( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k] );
                            fichiers->tabFichiers[i]->tabBlocs[j]->nbServeurs--;
                        }
                    }
                }

                if ( fichiers->tabFichiers[i]->tabBlocs[j]->nbServeurs == 0 ) /* Si ce bloc n'a plus de serveurs référencés on détruit tabServeurs */
                {
                    free( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs );
                    free( fichiers->tabFichiers[i]->tabBlocs[j] );
                    fichiers->tabFichiers[i]->tabBlocs[j] = NULL;
                }
            }

            for ( j = 0 ; j < fichiers->tabFichiers[i]->nbBlocs ; j++ ) /* Si ce fichier n'a plus de blocs référencés on détruit tabBlocs puis on supprime le fichier de la BDD des fichiers*/
            {
                if ( fichiers->tabFichiers[i]->tabBlocs[j] != NULL ) /* Si on trouve une seule source pour un seul bloc on doit garder le fichier référencé */
                {
                    sourceExiste = 1;
                    break;
                }
            }

            if ( !sourceExiste ) /* Si il n y a pas de raison de conserver le fichier on le supprime */
            {
                free( fichiers->tabFichiers[i]->tabBlocs );
                free( fichiers->tabFichiers[i]->nomFichier );
                free( fichiers->tabFichiers[i] );
                fichiers->tabFichiers[i] = NULL;
                fichiers->nbFichiers--;
            }
        }
    }

    /* On peut maintenant supprimer de la BDD des serveurs le serveur qui s'arrete */
    for ( i = 0 ; i < serveurs->nbInfoServeurs ; i++ )
    {
        if ( serveurs->tabInfoServeurs[i] != NULL ) /* Si la case n'est pas vide on lance le traitement */
        {
            if ( serveurs->tabInfoServeurs[i]->idServeur == var_idServeur ) /* Si on trouve le serveur on le supprime */
            {
                free( serveurs->tabInfoServeurs[i]->adresseServeur );
                free( serveurs->tabInfoServeurs[i] );
                serveurs->tabInfoServeurs[i] = NULL;
                serveurs->nbInfoServeurs--;
                break;
            }
        }
    }

    /* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock( &fichiers->verrou_bddfich_r );
    pthread_mutex_unlock( &fichiers->verrou_bddfich_w );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_r );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_w );
}


/**
 * \fn void traiteDemandeIdServeur( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (54) "Demande d'ID serveur".
 * \param [in] s La socket sur laquelle le message de demande d'ID serveur a été émis.
 * \param [in] mess Le message de demande d'ID serveur à traiter.
 * \warning Incrémente le compteur de génération d'idServeur.
 */
void traiteDemandeIdServeur( Socket s, char* mess )
{
    /* Variables */
    int type_message; /* type du message reçu : ici 54 */
    int var_portServeur; /* port du serveur */

    unsigned int i,j; /* Itérateurs */

    char* buff; /* tampon pour écriture du message */
    char* var_adresseServeur; /* adresse du serveur */
    char* str_portServeur; /* transformation du port du serveur en chaine de caractères */
    char* str_idServeur; /* transformation de l'identificateur du serveur en chaine de caractères */
    InfoServeurs** tempTabInfoServeurs; /* pointeur de travail sur un tableau de pointeurs sur InfoServeurs */

    /* Allocations mémoire des chaines de caractère */
    buff = malloc( TAILLE_BUFF * sizeof( char ) );
    var_adresseServeur = malloc( TAILLE_BUFF_MED * sizeof( char ) );
    str_portServeur = malloc( TAILLE_BUFF_VSM * sizeof( char ) );
    str_idServeur = malloc( TAILLE_BUFF_VSM * sizeof( char ) );

    /* On récupère le contenu du message */
    /* Doit être de la forme "54 adresseServeur portServeur" */
    if ( sscanf( mess, "%d %s %d", &type_message, var_adresseServeur, &var_portServeur ) < 3 )
    {
        fprintf( stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess );
    }

    /* On vérouille la BDD des fichiers en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock( &fichiers->verrou_bddfich_r );
    pthread_mutex_lock( &fichiers->verrou_bddfich_w );
    pthread_mutex_lock( &serveurs->verrou_bddserv_r );
    pthread_mutex_lock( &serveurs->verrou_bddserv_w );

    /* On référence le nouveau serveur dans la BDD des serveurs */
    if ( serveurs->capaTabInfoServeurs == serveurs->nbInfoServeurs ) /* Si il n y a pas assez de place dans la tableau dynamique, on l'agrandit */
    {
        tempTabInfoServeurs = serveurs->tabInfoServeurs;
        serveurs->tabInfoServeurs = malloc( ( TABDYN_AUGM_VAL + serveurs->capaTabInfoServeurs ) * sizeof( InfoServeurs* ) );

        for ( i = 0 ; i < serveurs->capaTabInfoServeurs ; i++ )
        {
            serveurs->tabInfoServeurs[i] = tempTabInfoServeurs[i];
        }

        serveurs->capaTabInfoServeurs += TABDYN_AUGM_VAL;

        for ( i = i ; i < serveurs->capaTabInfoServeurs ; i++ )
        {
            serveurs->tabInfoServeurs[i] = NULL;
        }

        free( tempTabInfoServeurs );
    }

    /* On génère un nouvel idServeur */
    generateurIdServeur++;

    /* Création du nouveau serveur dans la BDD des serveurs */
    for ( j = 0 ; j < serveurs->capaTabInfoServeurs ; j++ )
    {
        if ( serveurs->tabInfoServeurs[j] == NULL )
            break;
    }

    serveurs->tabInfoServeurs[j] = malloc( sizeof( InfoServeurs ) );
    serveurs->tabInfoServeurs[j]->adresseServeur = malloc( TAILLE_BUFF_LAR * sizeof( char ) );
    strcpy( serveurs->tabInfoServeurs[j]->adresseServeur, var_adresseServeur );
    serveurs->tabInfoServeurs[j]->numPort = var_portServeur;
    serveurs->tabInfoServeurs[j]->idServeur = generateurIdServeur;
    serveurs->nbInfoServeurs++;

    /* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock( &fichiers->verrou_bddfich_r );
    pthread_mutex_unlock( &fichiers->verrou_bddfich_w );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_r );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_w );

    /* On envoie un message au serveur pour lui donner son idServeur qui sera utilisé */
    /* Message de la forme : "21 adresseServeur portServeur idServeur" */
    strcpy( buff, "21 " );
    strcat( buff, var_adresseServeur );
    strcat( buff, " " );
    sprintf( str_portServeur, "%d", var_portServeur );
    strcat( buff, str_portServeur );
    strcat( buff, " " );
    sprintf( str_idServeur, "%u", generateurIdServeur );
    strcat( buff, str_idServeur );

    /* Envoi du message sur la socket */
    ecritureSocket( s, buff, TAILLE_BUFF );

    /* Libérations mémoire de chaines de caractère */
    free( buff );
    free( var_adresseServeur );
    free( str_portServeur );
    free( str_idServeur );
}


/**
 * \fn void traiteDemandeIdFichier( Socket s, char* mess )
 * \brief Procédure de traitement de message de type (55) "Demande d'ID fichier".
 * \param [in] s La socket sur laquelle le message de demande d'ID fichier a été émis.
 * \param [in] mess Le message de demande d'ID fichier à traiter.
 * \warning Incrémente le compteur de génération d'idFichier.
 */
void traiteDemandeIdFichier( Socket s, char* mess )
{
    /* Variables */
    int type_message; /* type du message reçu : ici 55 */
    unsigned int i,j; /* Iterateur */

    char* buff; /* tampon pour écriture du message */
    char* var_nomFichier; /* nom du fichier */
    char* str_idFichier; /* transformation de l'identificateur du serveur en chaine de caractères */

    Fichier** tempTabFichiers; /* Pointeur de travail sur tabFichiers */

    /* Allocations mémoire des chaines de caractère */
    buff = malloc( TAILLE_BUFF * sizeof( char ) );
    var_nomFichier = malloc( TAILLE_BUFF_MED * sizeof( char ) );
    str_idFichier = malloc( TAILLE_BUFF_VSM * sizeof( char ) );

    /* On récupère le contenu du message */
    /* Doit être de la forme "55 nomFichier" */
    if ( sscanf( mess, "%d %s", &type_message, var_nomFichier ) < 2 )
    {
        fprintf( stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess );
    }

    /* On vérouille la BDD des fichiers en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock( &fichiers->verrou_bddfich_r );
    pthread_mutex_lock( &fichiers->verrou_bddfich_w );
    pthread_mutex_lock( &serveurs->verrou_bddserv_r );
    pthread_mutex_lock( &serveurs->verrou_bddserv_w );

    /* On référence le nouveau fichier dans la BDD des fichiers */
    /* On génère un nouvel idFichier */
    generateurIdFichier++;

    /* Création du nouveau fichier dans la BDD des fichiers */
    /* On parcoure la liste pour trouver une case vide dans le tableau des fichiers */
    for ( i = 0 ; i < fichiers->capaTabFichiers ; i++ )
    {
        if ( fichiers->tabFichiers[i] == NULL )
            break;
    }

    if ( i == fichiers->capaTabFichiers ) /* Si on a pas trouvé de place on aggrandit le tableau */
    {
        tempTabFichiers = fichiers->tabFichiers;
        fichiers->tabFichiers = malloc( ( fichiers->capaTabFichiers + TABDYN_AUGM_VAL ) * sizeof( Fichier* ) );

        for( j = 0 ; j < fichiers->capaTabFichiers ; j++ )
        {
            fichiers->tabFichiers[j] = tempTabFichiers[j];
        }

        fichiers->capaTabFichiers += TABDYN_AUGM_VAL;

        for( j = j ; j < fichiers->capaTabFichiers ; j++ )
        {
            fichiers->tabFichiers[j] = NULL;
        }

        free( tempTabFichiers );
    }

    fichiers->tabFichiers[i] = malloc( sizeof( Fichier ) );
    fichiers->tabFichiers[i]->idFichier = generateurIdFichier;
    fichiers->tabFichiers[i]->nomFichier = malloc( TAILLE_BUFF_LAR * sizeof( char ) );
    strcpy( fichiers->tabFichiers[i]->nomFichier, var_nomFichier );
    fichiers->tabFichiers[i]->tabBlocs = NULL;

    /* On envoie un message au serveur pour lui donner son idServeur qui sera utilisé */
    /* Message de la forme : "22 nomFichier idFichier" */
    strcpy( buff, "22 " );
    strcat( buff, var_nomFichier );
    strcat( buff, " " );
    sprintf( str_idFichier, "%u", generateurIdFichier );
    strcat( buff, str_idFichier );

    /* Envoi du message sur la socket */
    ecritureSocket( s, buff, TAILLE_BUFF );

    /* Libérations mémoire de chaines de caractère */
    free( buff );
    free( var_nomFichier );
    free( str_idFichier );

    /* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock( &fichiers->verrou_bddfich_r );
    pthread_mutex_unlock( &fichiers->verrou_bddfich_w );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_r );
    pthread_mutex_unlock( &serveurs->verrou_bddserv_w );
}


/**
 * \fn void traiteMessageErr( Socket s, char* mess )
 * \brief Procédure de traitement de message adressé au mauvais destinataire.
 * \param [in] s La socket sur laquelle le message inattendu a été émis.
 * \param [in] mess Le message en question.
 */
void traiteMessageErr( Socket s )
{
    /* Envoi d'un message d'avertissement */
    ecritureSocket( s, "71 mauvais destinataire", TAILLE_BUFF );

    /* Fermeture de la socket qui n'a plus lieu d'être */
    clotureSocket( s );
}


/**
 * \fn void fermetureAnnuaire( )
 * \brief Procédure de fermeture de l'annuaire.
 * \warning Modifie le contenu des variables globales serveurs et fichiers.
 */
void fermetureAnnuaire( )
{
    /* Variables */
    unsigned int i,j,k; /* Itérateurs */

    /* Destruction des listes de serveurs */
    for ( i = 0 ; i < serveurs->nbInfoServeurs ; i++ )
    {
        free( serveurs->tabInfoServeurs[i]->adresseServeur );
        free( serveurs->tabInfoServeurs[i] );
    }
    free( serveurs->tabInfoServeurs );

    pthread_mutex_destroy( &serveurs->verrou_bddserv_r );
    pthread_mutex_destroy( &serveurs->verrou_bddserv_w );

    free( serveurs );

    /* Destruction des listes de fichiers */
    for ( i = 0 ; i < fichiers->capaTabFichiers ; i++ ) /* Pour chaque fichier */
    {
        if ( fichiers->tabFichiers[i] != NULL ) /* Si il y a un fichier référencé */
        {
            for ( j = 0 ; j < fichiers->tabFichiers[i]->nbBlocs ; j++ ) /* Pour chaque bloc */
            {
                for ( k = 0 ; k < fichiers->tabFichiers[i]->tabBlocs[j]->capaTabServeurs ; k++ ) /* Pour chaque case de tabServeurs */
                {
                    if ( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k] != NULL ) /* Si il y a un serveur on le libère */
                    {
                        free( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k]->infos );
                        free( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs[k] );
                    }
                }

                free( fichiers->tabFichiers[i]->tabBlocs[j]->tabServeurs );
            }

            free( fichiers->tabFichiers[i]->nomFichier );
            free( fichiers->tabFichiers[i]->tabBlocs );
        }
    }

    free( fichiers->tabFichiers );

    pthread_mutex_destroy( &fichiers->verrou_bddfich_r );
    pthread_mutex_destroy( &fichiers->verrou_bddfich_w );

    free( fichiers );
}


int main( void )
{
    /* Variables */
    int portEcouteAnnuaire; /* port d'écoute de l'annuaire */

    Socket socketEcouteAnnuaire; /* Socket d'écoute de l'annuaire */
    Socket socketDemandeConnexion; /* Socket ouverte par un client */

    pthread_t th_client; /* Thread pour gerer les connexions clients */
    pthread_t th_io; /* Thread de lecture clavier */

    /* Initialisation de la graine pour utiliser le rand() */
    srand( time( NULL ) );

    /* Initialisation de l'annuaire */
    printf( "Bienvenue sur le programme lif12p2p.\n" );
    printf( "Initialisation des bases de données de l'annuaire...\n" );
    if ( initialisationAnnuaire( ) < 0 )
    {
        perror( "Problème lors de l'initialisation des bases de données de l'annuaire. \n Fermeture du programme.\n" );
        exit( 1 );
    }
    printf( "Bases de données de l'annuaire initialisées.\n" );

    /* Initialisation de la socket d'écoute de l'annuaire */
    printf( "Sur quel port souhaitez vous lancer le serveur annuaire?\n" );
    scanf( "%d", &portEcouteAnnuaire );
    printf( "Vous avez choisi le port numero %d.\n", portEcouteAnnuaire );

    printf( "Initialisation de la socket d'écoute de l'annuaire...\n" );
    socketEcouteAnnuaire = initialiseSocketEcouteAnnuaire( portEcouteAnnuaire );
    printf( "Socket d'écoute de l'annuaire initialisée.\n" );

    pthread_create( &th_io, NULL, ( void* ( * )( void* ) ) lectureClavier, NULL );

    /* Menu */
    while ( !global_exit )
    {
        socketDemandeConnexion = acceptationConnexion( socketEcouteAnnuaire );
        if ( socketDemandeConnexion > 0 ) /* Si on a une connexion sur la socket */
        {
            if ( pthread_create( &th_client, NULL, ( void* ( * )( void* ) ) traiteMessage, ( void* ) socketDemandeConnexion ) == 0 )
            {
                global_nbthreads++;
                printf( "Il y a actuellement %d thread(s) de discussion ouvert(s).\n", global_nbthreads );
            }
            else
            {
                perror( "Un thread de traitement de message n'a pas pu être créé. Message ignoré.\n" );
            }
        }
    }

    /* Fermeture de l'annuaire */
    printf( "Destruction des bases de données de l'annuaire...\n" );
    fermetureAnnuaire( );
    printf( "Destruction des bases de données de l'annuaire terminée.\n" );
    printf( "Fermeture du programme.\n" );

    return 0;
}

/*****************
* Fin de Fichier
*****************/

