/**
 * @file client_serveur.c
 * @project lif12p2p
 * @author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since 20/03/2009
 * @version 09/04/2009
 */

#include "client_serveur.h"

#define NBTHREAD 10
#define TAILLE_BUFF 200
#define TAILLE_BLOC 65536

/*********************
* Variables globales *
**********************/

FileAttenteTelechargements listeAttenteTelechargement;  /* liste d'attente des téléchargements du coté client */
FileAttenteClients listeAttenteClient;                  /* liste d'attente des clients du coté serveur */
int finThreadServeur;                                   /* variable indiquant que l'arret du serveur est demandé */
int finThreadClient;                                    /* variable indiquant que l'arret du client est demandé */
Socket socketAnnuaire;                                  /* socket de dialogue avec l'annuaire */
int portServeur;                                        /* entier contenant le port du serveur */
char* adresseServeur;
int nbThreadServeurLance;                               /* entier comptant le nombre de threads lancés coté serveur */
int nbThreadClientLance;                                /* entier comptant le nombre de threads lancés coté client */
ListeFichiers listeFichier;                             /* liste des fichiers en cours de téléchargement du coté client */
int arretApplication;                                   /* indique le nombre de "partie" arrétée */
unsigned int idServeur;                                 /* entier contenantant l'IDServeur (coté serveur) */

/******************************
* main et fonctions communes *
*******************************/

int main()
{
    /* variables */
    char* adresseAnnuaire;      /* stocke l'adresse de l'annuaire à utiliser */
    int portAnnuaire;           /* stocke le port de l'annuaire à utiliser */
    char* resultat;             /* stocke la frappe au clavier si la connexion a échouée */
    pthread_t variableThread;   /* variable pour lancer les threads "serveur" et "client" */
    char* message;              /* chaine de caractère pour le message à envoyer à l'annuaire */
    char* buff;                 /* chaine de caractère pour écouter la réponse de l'annuaire */
    char* tempNomServeur;       /* variable temporaire pour récupérer la réponse de l'annuaire */
    int tempCode;               /* variable temporaire pour récupérer la réponse de l'annuaire */
    int tempPortServ;           /* variable temporaire pour récupérer la réponse de l'annuaire */

    /* initalisation */
    finThreadClient = 0;
    finThreadServeur = 0;
    arretApplication = 0;
    resultat = malloc(2* sizeof(char));
    adresseAnnuaire = malloc(100* sizeof(char));
    message = malloc(TAILLE_BUFF* sizeof(char));
    buff = malloc(TAILLE_BUFF* sizeof(char));
    tempNomServeur = malloc(100* sizeof(char));
    adresseServeur = malloc(100* sizeof(char));
    idServeur = 0;

    /* demande de la socket de l'annuaire */
    do
    {
        printf("Quelle est l'adresse de l'annuaire utilisé ?\n");
        scanf("%s", adresseAnnuaire);
        printf("Quel est le port de l'annuaire utilisé ?\n");
        scanf("%d", &portAnnuaire);

        /** tentative de connexion à l'annuaire */
        /* création de la socket */
        socketAnnuaire = creationSocket();
        /* connexion à l'annuaire */
        if (demandeConnexionSocket(socketAnnuaire, adresseAnnuaire, portAnnuaire) == 1)
        {
            /* si la connexion échoue, demande à l'utilisateur s'il veut recommencer avec un autre annuaire */
            printf("La connexion a l'annuaire a échouer ! Voulez-vous vous connecter sur un autre annuaire ? (O/N)\n");
            scanf("%s", resultat);
            if ( strcmp(resultat,"N") == 0 || strcmp(resultat,"n") == 0)
            {
                /* si l'utilisateur ne veut pas se connecter à un autre annuaire */
                printf("Au revoir\n");
                exit(1);
            }
        }
        else
        {
            /* si la connexion à réussi, initialisation de la variable "resultat" */
            strcpy(resultat,"N");
        }
    }
    while (strcmp(resultat,"O") == 0 || strcmp(resultat,"o") == 0);

    /* demande de l'adresse et du port du serveur */
    printf("Quel est le nom du serveur utiliser ?\n");
    scanf("%s", adresseServeur);
    printf("Sur quel port voulez-vous lancer le serveur ?\n");
    scanf("%d", &portServeur);

    /* demande d'un ID serveur a l'annuaire */
    creationMessage(54, NULL, message);
    ecritureSocket(socketAnnuaire, message, TAILLE_BUFF);
    /* ecoute de la réponse de l'annuaire */
    ecouteSocket(socketAnnuaire, buff, TAILLE_BUFF);
    /* analyse de la réponse de l'annuaire */
    printf("plop\n");
    if (sscanf(buff, "%d %s %d %u", &tempCode, tempNomServeur, &tempPortServ, &idServeur) < 4)
    {
        /* le message reçu ne comporte pas assez de champ */
        printf("Probleme à la création d'un IDServeur!! (erreur lecture socket)\n");
        exit(1);
    }
    if (tempCode != 21)
    {
        /* le message reçu n'a pas le bon code */
        printf("Probleme à la création d'un IDServeur!! (code erroné)\n");
        exit(1);
    }
    if (idServeur == 0)
    {
        /* le message reçu n'a pas d'IDServeur */
        printf("Probleme à la création d'un IDServeur!! (ID nom reconnu)\n");
        exit(1);
    }

    /* La connexion à l'annuaire est établie : lancement de 3 threads : serveur, client et un pour la lecture clavier */
    pthread_create(&variableThread, NULL, (void*(*)(void*))applicationServeur, NULL);
    pthread_create(&variableThread, NULL, (void*(*)(void*))applicationClient, NULL);
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadLectureClavier, NULL);

    /* libération de l'espace mémoire */
    free(adresseAnnuaire);
    /* boucle tant que les applications client et serveur ne sont pas terminées */
    while (1)
    {
        if (arretApplication == 3)
        {
            break;
        }
    }
    printf("Applisation arrété\n");
    return 0;
}

/**
* @note procédure créant le message approprié en chaine de cartactère
* @param code : entier correspondant au code du message à créer
* @param structure : pointeur sur la structure avec les "bonnes" données
* @param message : chaine de caractère contenant le message créé
* @return retourne 0 si le message est bien créé, 1 sinon
*/
int creationMessage(int code, void* structure, char* message)
{
    /* variables */
    char* tempChaine;           /* chaine de caractère temporaire pour passer les entiers en chaine */

    /* initialisation */
    tempChaine = malloc(100* sizeof(char));

    switch (code)
    {
    case 31 :
        /* message de demande de fichier à l'annuaire */
        strcpy(message,"31 ");
        strcat(message, (char*) structure);
        break;
    case 32 :
        /* message de demande de bloc à l'annuaire */
        strcpy(message,"32 ");
        sprintf(tempChaine, "%d", ((Telechargement*) structure)->idFichier);
        strcat(message, tempChaine);
        strcat(message, " ");
        strcat(message, ((Telechargement*) structure)->nomFichier);
        strcat(message, " ");
        sprintf(tempChaine, "%d", ((Telechargement*) structure)->numeroBloc);
        strcat(message, tempChaine);
        break;
    case 33 :
        /* message d'arret du client à l'annuaire */
        strcpy(message,"33 ");
        strcat(message, adresseServeur);
        break;
    case 41 :
        /* message de demande de bloc à un serveur */
        strcpy(message,"41 ");
        sprintf(tempChaine, "%d", ((Telechargement*) structure)->idFichier);
        strcat(message, tempChaine);
        strcat(message, " ");
        strcat(message, ((Telechargement*) structure)->nomFichier);
        strcat(message, " ");
        sprintf(tempChaine, "%d", ((Telechargement*) structure)->numeroBloc);
        strcat(message, tempChaine);
        break;
    case 42 :
        /* message d'arret du client aux serveurs */
        strcpy(message, "42 ");
        strcat(message, adresseServeur);
        break;
    case 51 :
        /* message de disponibilité d'un bloc à l'annuaire */
        strcpy(message,"51 ");
        sprintf(tempChaine, "%d", ((StructureDisponibiliteBloc*) structure)->idFichier);
        strcat(message, tempChaine);
        strcat(message, " ");
        sprintf(tempChaine, "%d", ((StructureDisponibiliteBloc*) structure)->numTotalBloc);
        strcat(message, tempChaine);
        strcat(message, " ");
        sprintf(tempChaine, "%d", ((StructureDisponibiliteBloc*) structure)->numeroBloc);
        strcat(message, tempChaine);
        strcat(message, " ");
        sprintf(tempChaine, "%u", idServeur);
        strcat(message, tempChaine);
        break;
    case 52 :
        /* message d'arret du serveur à l'annuaire */
        strcpy(message,"52 ");
        sprintf(tempChaine, "%u", idServeur);
        strcat(message, tempChaine);
        break;
    case 53 :
        /* message de charge du serveur vers l'annuaire */
        strcpy(message,"53 ");
        sprintf(tempChaine, "%d", *((int*) structure));
        strcat(message, tempChaine);
        break;
    case 54 :
        /* message de demande d'IDServeur */
        strcpy(message,"54 ");
        strcat(message, adresseServeur);
        strcat(message, " ");
        sprintf(tempChaine, "%d", portServeur);
        strcat(message, tempChaine);
        break;
    case 55 :
        /* message de demande d'IDFichier */
        strcpy(message,"55 ");
        strcat(message, (char*) structure);
        break;
    case 61 :
        /* envoi d'un bloc d'un serveur à un client */
        strcpy(message,"61 ");
        sprintf(tempChaine, "%d", ((Client*) structure)->idFichier);
        strcat(message, tempChaine);
        strcat(message, " ");
        strcat(message, ((Client*) structure)->nomFichier);
        strcat(message, " ");
        sprintf(tempChaine, "%d", ((Client*) structure)->numeroBloc);
        strcat(message, tempChaine);
        break;
    case 62 :
        /* envoi d'un bloc d'un serveur à un client */
        strcpy(message,"62 ");
        sprintf(tempChaine, "%d", ((Client*) structure)->idFichier);
        strcat(message, tempChaine);
        strcat(message, " ");
        sprintf(tempChaine, "%d", ((Client*) structure)->numeroBloc);
        strcat(message, tempChaine);
        break;
    case 63 :
        /* message de déconnexion du serveur vers les clients */
        strcpy(message,"63 ");
        sprintf(tempChaine, "%u", idServeur);
        strcat(message, tempChaine);
        strcat(message, " ");
        strcat(message, adresseServeur);
        strcat(message, " ");
        sprintf(tempChaine, "%d", portServeur);
        strcat(message, tempChaine);
        break;
    default:
        /* code non reconnu */
        return 1;
        break;
    }
    free(tempChaine);
    return 0;
}

/**
* @note procédure à lancer dans un thread pour gérer toute la lecture clavier du programe
*/
void threadLectureClavier()
{
    /* variables */
    char buff[TAILLE_BUFF];     /* chaine de caractère pour la lecture clavier */
    int code;                   /* entier correspondant au code du menu */

    do
    {
        /* affichage du texte */
        printf("\n\nMenu :\n");
        printf("1- Mettre un fichier sur le reseau\n");
        printf("2- recuperer un fichier sur le reseau\n");
        if (!finThreadServeur)
        {
            printf("3- Arreter le serveur\n");
        }
        if (!finThreadClient)
        {
            printf("4- Arreter le client\n");
        }
        printf("5- Quitter\n");
        scanf("%d", &code);
        switch (code)
        {
        case 1 :
            /* demande du fichier à partager */
            printf("Rentrez un nom de fichier : \n");
            printf("Il doit etre dans le dossier d'execution de l'application.\n");
            lireLigne(buff);
            signalisationFichierAnnuaire(buff);
            break;
        case 2 :
            /* demande du fichier a télécharger */
            printf("Rentrez un nom de fichier : \n");
            lireLigne(buff);
            demandeFichier(buff);
            break;
        case 3 :
            finThreadServeur = 1;
            break;
        case 4 :
            finThreadClient = 1;
            break;
        case 5 :
            finThreadClient = 1;
            finThreadServeur = 1;
            break;
        default:
            printf("Code inconnu !!!\n");
            break;
        }
    }
    while ((finThreadClient == 0) || (finThreadServeur == 0));
    arretApplication++;
}

/**
* @note procédure qui récupère la ligne tapé au clavier (boucle s'il y a une ligne vide)
* @param message : chaine de caractère lu
*/
void lireLigne(char* message)
{
    do
    {
        /* lecture d'une ligne au clavier */
        fgets(message, TAILLE_BUFF, stdin);
        /* Le dernier carractère est un retour chariot */
        message[strlen(message)-1] = '\0';
        /* boucle tant que la la ligne lu est vide */
    }
    while (strcmp(message,"") == 0);
}

/*************************************************
* code des fonctions et procédures coté serveur *
**************************************************/

/**
* @note Application coté serveur gérant l'emmission des fichiers.
*/
void applicationServeur()
{
    /* déclatation des variables */
    pthread_t variableThread;       /* variable pour lancer les threads : un d'écoute, et un d'emmision */

    /* initialisation des variables globales du coté serveur */
    nbThreadServeurLance = 1;

    /* initialisation de la liste d'attente */
    initialisationListeAttenteClient(&listeAttenteClient);

    /* création du thread pour écouter les demandes clients */
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadDialogueClient, NULL);

    /* création du thread gérant les envoi de blocs */
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadEmmission, NULL);

    /* attente du changement de la variable "finThreadServeur" */
    while (1)
    {
        if (finThreadServeur == 1)
        {
            break;
        }
    }

    /** arrêt du serveur */
    /* arrêt de tous les threads lancés */
    while (1)
    {
        /* on boucle tant qu'il y a au moins un thread lancé */
        if (nbThreadServeurLance == 0)
        {
            break;
        }

    }
    /* suppression de la liste d'attente */
    arretServeur();
}

/**
* @note fonction qui signale les fichiers disponibles à l'annuaire
* @param nomFichier : chaine de caractère contenant le nom du fichier à mettre sur le réseau
*/
void signalisationFichierAnnuaire(char* nomFichier)
{
    /* variables */
    FILE* fichierADecouper; /* descripteur de fichier vers le fichier a découper */
    unsigned int nbBloc;    /* entier comptant le nombre de bloc */
    char* message;          /* chaine de caractère contenant le message vers l'annuaire */
    char* lectureFichier;   /* chaine de caractère pour la lecture fichier */
    char* buff;             /* chaine de caractère pour la réception d'un message */
    unsigned int idFichier; /* entier récupérant l'IDFichier */
    int code;               /* entier correspondant au code du message reçu */
    char* nomFich;          /* chaine de caractère correspondant au nom du fichier reçu dans le message */
    char* cheminFichier;    /* chemin d'acces au fichier */

    /* initialisation */
    message = malloc(TAILLE_BUFF* sizeof(char));
    buff = malloc(TAILLE_BUFF* sizeof(char));
    lectureFichier = malloc((TAILLE_BLOC)* sizeof(char));
    nomFich = malloc(100* sizeof(char));
    cheminFichier = malloc(100* sizeof(char));
    nbBloc = 0;

    /* demande d'un IDFichier */
    creationMessage(55, (void*) nomFichier, message);
    ecritureSocket(socketAnnuaire, message, TAILLE_BUFF);
    ecouteSocket(socketAnnuaire, buff, TAILLE_BUFF);
    sscanf(buff, "%d %s %u", &code, nomFich, &idFichier);

    /* récupération du chemin du fichier */
    strcpy(cheminFichier, "partage/");
    strcat(cheminFichier, nomFichier);
    printf("%s", cheminFichier);
    /* ouverture du fichier */
    if ((fichierADecouper = fopen(cheminFichier, "r")) == NULL)
    {
        /* le fichier recherché n'a pas été trouvé */
        printf("Le fichier suivant n'a pas été trouvé : %s\n", nomFichier);
    }
    else
    {
        /* récupération du nombre de bloc */
        while (!feof(fichierADecouper))
        {
            fread((void*)lectureFichier, sizeof(char), TAILLE_BLOC, fichierADecouper);
            nbBloc++;
        }
        /* libération de l'espace mémoire */
        fclose(fichierADecouper);
        /* test si le fichier fait 0 octet */
        if (nbBloc == 0)
        {
            /* le fichier fait 0 octet */
            printf("Le fichier fait 0 octet, il ne peut pas etre partagé.\n");
        }
        else
        {
            /* initialisation de la structure */
            StructureDisponibiliteBloc structurePourEnvoi;
            structurePourEnvoi.nomFichier = nomFichier;
            structurePourEnvoi.numTotalBloc = nbBloc;
            structurePourEnvoi.idFichier = idFichier;
            /* envoi d'un message pour chaque bloc lu */
            for (nbBloc = nbBloc; nbBloc > 0; nbBloc--)
            {
                structurePourEnvoi.numeroBloc = nbBloc - 1;
                /* création du message */
                creationMessage(51, (void*) &structurePourEnvoi, message);
                /* envoi des données à l'annuaire */
                ecritureSocket(socketAnnuaire, message, TAILLE_BUFF);
            }
        }
    }
    /* libération de l'espace mémoire */
    free(message);
    free(buff);
    free(nomFich);
    free(lectureFichier);
}

/**
* @note procédure d'initialisation de la liste d'attente des clients
*/
void initialisationListeAttenteClient()
{
    /* mise a 0 du nombre de client, et mise a NULL des deux pointeurs
       et initialisation du mutex */
    listeAttenteClient.nbClients = 0;
    listeAttenteClient.premierClient = NULL;
    listeAttenteClient.dernierClient = NULL;
    pthread_mutex_init(&(listeAttenteClient.mutexListeAttenteServeur), NULL);

}

/**
* @note procédure à exécuter dans un thread pour écouter les requètes clientes
*/
void threadDialogueClient()
{
    /* variables */
    Socket socketEcouteServeur;         /* socket d'écoute en attente de connexion client */
    Socket socketDemandeConnexion;      /* socket de discution créée dès qu'un client se connecte */
    pthread_t variableThread;           /* variable pour lancer les threads de dialogue avec les clients */

    /* crée une socket et affectation de la socket (pour écouter les connexion client) */
    socketEcouteServeur = creationSocket();
    definitionNomSocket(socketEcouteServeur, portServeur);
    /* écoute les demandes de connexion */
    while (!finThreadServeur)
    {
        /* récupération des sockets de dialogue */
        socketDemandeConnexion = acceptationConnexion(socketEcouteServeur);

        /* test si la connexion a échoué */
        if (socketDemandeConnexion > 0)
        {
            /* création d'un thread pour dialoguer avec chaque client */
            pthread_create(&variableThread, NULL, (void*(*)(void*))dialogueClient, (void*)socketDemandeConnexion);
        }
        /* sinon, boucle tant qu'il faut */
    }
    /* On sort de la boucle quand on recoit le signal "fin thread" : décrémentation du nombre de thread */
    nbThreadServeurLance--;
}

/**
* @note procédure dialoguant avec le client
* @param socketDialogue : socket de dialogue avec le client
*/
void dialogueClient(Socket socketDialogue)
{
    /* variables */
    char* buff;             /* chaine de caractère stockant le contenu du message reçu */
    int code;               /* entier correspondant au code du message reçu */
    int finDialogue;        /* variable indiquant si on doit sortir de la boucle de discution
                                0- on continue a écouter
                                1- on sort de la boucle */

    /* initialisation des variables */
    buff = malloc(TAILLE_BUFF * sizeof(char));
    finDialogue = 0;

    while (!finThreadServeur && !finDialogue)
    {
        /* récupération du prochain message sur la socket */
        ecouteSocket(socketDialogue, buff, TAILLE_BUFF);
        if (strcmp(buff, "") != 0)
        {
            /* récupération du code en début de message */
            if (sscanf(buff, "%d", &code) == 1)
            {
                /* analyse du code :
                   6 - message d'un client demandant un bloc
                   7 - message d'arrêt d'un client
                   14- fermeture du thread
                   autre - message non destiné au serveur */
                switch (code)
                {
                case 6:
                    /* message de demande de bloc d'un client */
                    traitementMessageBloc(socketDialogue, buff);
                    break;
                case 7:
                    /* message d'arret du client */
                    traitementMessageArret(socketDialogue);
                    finDialogue = 1;
                    break;
                case 14:
                    /* le message que l'on vient d'envoyer est inconnu : fermeture du thread  */
                    finDialogue = 1;
                    break;
                default:
                    /* code inconnu */
                    traitementMessageErreur(socketDialogue);
                    finDialogue = 1;
                    break;
                }
            }
            else
            {
                /* le code au début du message reçu n'a pas été reconnu */
                /* message ignoré */
                perror("Le message suivant non reconnu a été reçu : ");
                perror(buff);
            }
        }
    }

    /** fermeture de la socket */
    /* soit par ce que le serveur va s'arreter */
    if (finThreadServeur == 1)
    {   /* il faut alors envoyer un message à tous les clients */
        /* variables */
        char* message;          /* chaine de caractère pour créer le message à envoyer */

        /* initialisation */
        message = malloc(TAILLE_BUFF* sizeof(char));
        /* création du message */
        creationMessage(63, NULL, message);
        /* envoi du message */
        ecritureSocket(socketDialogue, message, TAILLE_BUFF);

        /* libération de l'espace mémoire */
        free(message);
    }
    /* soit parce que le client le demande */
    clotureSocket(socketDialogue);
    free(buff);
}

/**
* @note procédure qui analyse le message reçu du client (rempli la liste d'attente si besoin)
* @param socketDialogue : socket de dialogue avec le client
* @param buff : chaine de caractère contenant le message reçu du client
*/
void traitementMessageBloc(Socket socketDialogue, char* buff)
{
    /* variables */
    int code;                   /* entier correspondant au code du message */
    Client* clientAAjouter;     /* structure "Client" temporaire à ajouter en fin de liste d'attente */

    /* initialisation des variables */
    clientAAjouter = malloc(sizeof(Client));
    clientAAjouter->nomFichier = malloc(100 * sizeof(char));

    /* récupération du nom du fichier, et du numéro de bloc à partir du message reçu */
    if (sscanf(buff, "%d %s %d", &code, clientAAjouter->nomFichier, &(clientAAjouter->numeroBloc)) == 3)
    {
        /* si la lecture s'est bien passée */
        clientAAjouter->socketClient = socketDialogue;
        clientAAjouter->clientSuivant = NULL;

        /*******  bloquage du mutex pour éviter la lecture / écriture de la liste d'attente  *********/
        pthread_mutex_lock(&(listeAttenteClient.mutexListeAttenteServeur));
        /* ajout en liste d'attente */
        if (listeAttenteClient.nbClients == 0)
        {
            /* cas où il n'y a aucun client dans la liste d'attente */
            listeAttenteClient.premierClient = clientAAjouter;
            listeAttenteClient.dernierClient = clientAAjouter;
            listeAttenteClient.nbClients++;
        }
        else
        {
            /* ajout du client à la suite du dernier */
            (listeAttenteClient.dernierClient)->clientSuivant = clientAAjouter;
            listeAttenteClient.nbClients++;
            listeAttenteClient.dernierClient = clientAAjouter;
        }
        /*******  libération du mutex  ********/
        pthread_mutex_unlock(&(listeAttenteClient.mutexListeAttenteServeur));
    }
    else
    {
        /* problème de lecture dans le message reçu */
        printf("Réception du message inconnu suivant : %s\n", buff);
    }
}

/**
* @note procédure qui analyse le message reçu du client (demandant une déconnexion du client)
* @param socketDialogue : socket de dialogue avec le client
*/
void traitementMessageArret(Socket socketDialogue)
{
    /* variables */
    Client* tempClient;         /* pointeur temporaire pour effectuer la suppression */

    /*******  bloquage du mutex pour éviter la lecture / écriture de la liste d'attente  *********/
    pthread_mutex_lock(&(listeAttenteClient.mutexListeAttenteServeur));
    /* initialisation du pointeur temporaire */
    tempClient = listeAttenteClient.premierClient;

    /* cas s'il faut supprimer le premier client de la liste d'attente */
    while (tempClient != NULL && tempClient->socketClient == socketDialogue)
    {/* ici, tempClient pointe sur le Client à supprimer */
        /* diminution du nombre de clients */
        listeAttenteClient.nbClients--;
        /* affectation du nouveau premier client */
        listeAttenteClient.premierClient = (listeAttenteClient.premierClient)->clientSuivant;
        /* cas où il n'y a plus de client */
        if (listeAttenteClient.nbClients == 0)
        {
            listeAttenteClient.dernierClient = NULL;
        }
        /* libération du client */
        free(tempClient->nomFichier);
        free(tempClient);
        /* reèinitialisation du pointeur temporaire pour le test suivant */
        tempClient = listeAttenteClient.premierClient;
    }
    /** le client pointé par "tempClient" n'est pas à supprimer, test sur son suivant */
    /* parcours du reste de la liste d'attente s'il reste des clients en liste d'attente */
    if (tempClient != NULL)
    {
        while (tempClient->clientSuivant != NULL)
        {/* ici, tempClient pointe sur le Client précédant celui à supprimer */
            /* test si le client est à supprimer */
            if ((tempClient->clientSuivant)->socketClient == socketDialogue)
            {
                /* variables */
                Client* tempClientSupprimer;        /* variable temporaire pointant sur le client à supprimer */

                /* mise en mémoire du client à supprimer */
                tempClientSupprimer = tempClient->clientSuivant;
                /* diminution du nombre de clients */
                listeAttenteClient.nbClients--;
                /* affectation du nouveau client qui suit */
                tempClient->clientSuivant = tempClientSupprimer->clientSuivant;
                /* cas où il s'agit du dernier client */
                if (tempClient->clientSuivant == listeAttenteClient.dernierClient)
                {
                    /* affectation du nouveau dernier client */
                    listeAttenteClient.dernierClient = tempClient;
                }
                /* libération du client */
                free(tempClientSupprimer->nomFichier);
                free(tempClientSupprimer);
            }
            else
            {
                /* avancement du client temporaire dans la liste */
                tempClient = tempClient->clientSuivant;
            }
        }
    }
    /* sortie de boucle car on est arrivé à la fin de la liste */
    /*******  libération du mutex  ********/
    pthread_mutex_unlock(&(listeAttenteClient.mutexListeAttenteServeur));
}

/**
* @note procédure qui répond au message d'erreur
* @param socketDialogue : socket de dialogue avec le client
*/
void traitementMessageErreur(Socket socketDialogue)
{
    /* variables */
    char* message;  /* chaine de caractère contenant le message à envoyer */
    /* initialisation */
    message = malloc(TAILLE_BUFF* sizeof(char));
    /* création du message */
    strcpy(message, "71 mauvais destinataire");
    /* envoi du message d'erreur */
    ecritureSocket(socketDialogue, message, TAILLE_BUFF);
    free(message);
}

/**
* @note procédure qui lance les threads d'emmission des blocs
*/
void threadEmmission()
{
    /* variables */
    pthread_t variableThread;       /* variable pour lancer les threads d'envoi de bloc */

    /* boucler tant que l'arret du serveur n'a pas été demandée */
    while (!finThreadServeur)
    {
        /* s'il n'y a pas déjà trop de thread lancé ET il y a un élément en liste d'attente*/
        if ((nbThreadServeurLance < NBTHREAD) && (listeAttenteClient.premierClient != NULL))
        {
            /* incrémentation du nombre de thread et création du thread */
            nbThreadServeurLance++;
            pthread_create(&variableThread, NULL, (void*(*)(void*))threadEnvoiMessage, NULL);
        }
    }
}

/**
* @note Fonction à exécuter dans un thread pour envoyer des blocs la fonction boucle tant qu'il y a des blocs en liste d'attente
*/
void threadEnvoiMessage()
{
    /* variables */
    Client* blocAEnvoyer;  /* pointeur de travail qui désignera le bloc à envoyer */

    /*******  bloquage du mutex pour éviter la lecture / écriture de la liste d'attente  *********/
    pthread_mutex_lock(&(listeAttenteClient.mutexListeAttenteServeur));
    /* sélection du prochain client en liste d'attente */
    blocAEnvoyer = listeAttenteClient.premierClient;

    while ((finThreadServeur == 0) && (blocAEnvoyer != NULL))
    {
        /* suppression de l'élément sélectionné */
        listeAttenteClient.premierClient = (listeAttenteClient.premierClient)->clientSuivant;
        listeAttenteClient.nbClients--;
        /* cas où il n'y a pas d'autre client en liste d'attente */
        if (listeAttenteClient.nbClients == 0)
        {
            /* changement du pointeur sur le dernier client */
            listeAttenteClient.dernierClient = NULL;
        }
        /*******  libération du mutex  ********/
        pthread_mutex_unlock(&(listeAttenteClient.mutexListeAttenteServeur));

        /* signalisation de la charge du serveur à l'annuaire */
        signalisationChargeServeur(1);
        /* envoi du bloc */
        envoiMessage(blocAEnvoyer);
        /* signalisation de la charge du serveur à l'annuaire */
        signalisationChargeServeur(-1);

        /* libération de l'espace mémoire */
        free(blocAEnvoyer->nomFichier);
        free(blocAEnvoyer);

        /* re-affectation du pointeur temporaire */
        /*******  bloquage du mutex pour éviter la lecture / écriture de la liste d'attente  *********/
        pthread_mutex_lock(&(listeAttenteClient.mutexListeAttenteServeur));
        /* sélection du prochain client en liste d'attente */
        blocAEnvoyer = listeAttenteClient.premierClient;
    }

    /* soit le pointeur "blocAEnvoyer" vaut "NULL", il n'y a donc plus aucun client en liste d'attente
       soit l'arret du serveur a été demandé */
    /*******  libération du mutex  ********/
    pthread_mutex_unlock(&(listeAttenteClient.mutexListeAttenteServeur));
    /* décrémentation du nombre de thread lancé */
    nbThreadServeurLance--;
}

/**
* @note cherche et envoie les données à transmettre
* @param valeur : entier correspondant à la charge : +/- 1
*/
void signalisationChargeServeur(int valeur)
{
    /* variables */
    char* message;          /* chaine de caractère pour le message à envoyer */

    /* initialisation des variables */
    message = malloc(TAILLE_BUFF* sizeof(char));
    /* création de la chaine à envoyer */
    creationMessage(53, (void*) &valeur, message);
    /* envoi du message */
    ecritureSocket(socketAnnuaire, message, TAILLE_BUFF);
    /* libération de l'espace mémoire */
    free(message);
}

/**
* @note cherche et envoie les données à transmettre
* @param client : pointeur sur la structure contenant les informations sur le bloc à envoyer
*/
void envoiMessage(Client* client)
{
    /* variables */
    char* buff;             /* chaine de caractère pour le contenu du bloc */
    char* message;          /* chaine de caractère pour le message à envoyer */
    FILE* fichierALire;     /* fichier dans lequel il faut lire les données */
    int tailleLu;           /* entier stockant la taille du bloc lu */
    char* strTailleLu;      /* chaine de caractère pour concaténer la taille lu */
    char* cheminFichier;    /*  */

    /* initialisation */
    buff = malloc(TAILLE_BLOC * sizeof(char));
    message = malloc(TAILLE_BUFF* sizeof(char));
    cheminFichier = malloc(100* sizeof(char));
    strTailleLu = malloc(10* sizeof(char));

    /* récupéation du chemin du fichier */
    strcpy(cheminFichier, "partage/");
    strcat(cheminFichier, client->nomFichier);
    /* ouverture du fichier */
    if ( (fichierALire = fopen(cheminFichier, "r")) != NULL)
    {/* le fichier a été trouvé */
        /* récupération de la chaine appropriée */
        fseek(fichierALire, (client->numeroBloc) * TAILLE_BLOC, SEEK_SET);
        tailleLu = fread((void*) buff, sizeof(char), TAILLE_BLOC, fichierALire);
        if (tailleLu < 0)
        {
            printf("Erreur de lecture du fichier!!! \n");
            exit(1);
        }
        /* fermeture du fichier */
        fclose(fichierALire);
        /* création du message à envoyer */
        creationMessage(61, (void*) client, message);
        strcat(message, " ");
        sprintf (strTailleLu, "%d",tailleLu);
        strcat(message, strTailleLu);
        /* écriture des données sur la socket */
        ecritureSocket(client->socketClient, message, TAILLE_BUFF);
        ecritureSocket(client->socketClient, buff, tailleLu);
    }
    else
    {/* le fichier n'a pas été trouvé */
        /* création du message à envoyer */
        creationMessage(62, (void*) client, message);
        /* écriture des données sur la socket */
        ecritureSocket(client->socketClient, message, TAILLE_BUFF);
    }
    /* libération de l'espace mémoire */
    free(buff);
    free(message);
}

/**
* @note procédure qui signale l'arret du serveur a l'annuaire, et l'arrete
*/
void arretServeur()
{
    /* variable */
    char* message;          /* chaine de caractère contenant le message à envoyer */
    Client* tempClient;     /* pointeur temporaire pour supprimer la liste des clients */

    /* iniitalisation des variables */
    message = malloc(TAILLE_BUFF* sizeof(char));

    /* création du message à envoyer à l'annuaire */
    creationMessage(52, NULL, message);
    /* envoi du message à l'annuaire */
    ecritureSocket(socketAnnuaire, message, TAILLE_BUFF);

    /* libération de l'espace mémoire */
    free(message);

    /* vidage de la liste d'attente */
    tempClient = listeAttenteClient.premierClient;
    while (tempClient != NULL)
    {
        /* affectation du nouveau premier élément de la liste */
        listeAttenteClient.premierClient = tempClient->clientSuivant;
        /* libération de l'espace mémoire occupé */
        free(tempClient->nomFichier);
        free(tempClient);
        /* re-affectation du pointeur temporaire */
        tempClient = listeAttenteClient.premierClient;
    }
    /* libération du mutex */
    pthread_mutex_destroy(&(listeAttenteClient.mutexListeAttenteServeur));

    arretApplication++;
}

/************************************************
* code des fonctions et procédures coté client *
*************************************************/

/**
* @note Application coté serveur gérant le téléchargement des fichiers.
*/
void applicationClient()
{
    /** séparation en 2 thread : - 1 pour faire des demandes de fichier à l'annuaire
                                 - 1 pour télécharger les parties en liste d'attente
    */
    /* variables */
    pthread_t variableThread;   /* variable pour lancer les threads */

    /* initialisation de la liste d'attente de téléchargement */
    initialisationListeAttenteTelechargement();
    /* lancement des threads pour télécharger des blocs */
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadTelechargement, NULL);

    /* Attente du changement de valeur de "finThreadClient" apres une lecture au clavier */
    while (1)
    {
        if (finThreadClient)
        {
            break;
        }
    }
    /* arrêt du client */
    /* attente de l'arrêt de tous les threads lancés */
    while (1)
    {
        /* on boucle tant qu'il y a au moins un thread lancé */
        if (nbThreadClientLance == 0)
        {
            break;
        }
    }
    /* suppression de la liste d'attente */
    arretClient();
}

/**
* @note procédure d'initialisation de la liste d'attente des telechargements
*/
void initialisationListeAttenteTelechargement()
{
    /* mise a 0 du nombre de client, et mise a NULL des deux pointeurs
       et initialisation des mutex */
    /* liste d'attente de téléchargement */
    listeAttenteTelechargement.nbTelechargements = 0;
    listeAttenteTelechargement.premierTelechargement = NULL;
    listeAttenteTelechargement.dernierTelechargement = NULL;
    pthread_mutex_init(&(listeAttenteTelechargement.mutexListeAttenteClient), NULL);
    /* liste des fichiers */
    listeFichier.nbFichiers = 0;
    listeFichier.listeFichiers = NULL;
    pthread_mutex_init(&(listeFichier.mutexListeFichierEcriture), NULL);
    pthread_mutex_init(&(listeFichier.mutexListeFichierLecture), NULL);

}

/**
* @note met le fichier dans la liste des fichiers, et demande à l'annuaire
* @param nomFichier : nom du fichier à télécharger
*/
void demandeFichier(char* nomFichier)
{
    /* variables */
    int finDialogue;    /* booléen indiquant l'arrêt (ou non) du dialogue */
    char* messageEnvoi; /* chaine de caractère stockant le message à envoyer */
    char* message;      /* chaine de caractère stockant le message reçu */
    int code;           /* entier stockant le code du message */
    unsigned int nbBlocTotal;    /* entier stockant le nombre total de message (1 par bloc) attendu */
    unsigned int compteur;       /* compteur sur le nombre de message reçu */

    /* initialisation */
    finDialogue = 0;
    compteur = 0;
    message = malloc(TAILLE_BUFF* sizeof(char));
    messageEnvoi = malloc(TAILLE_BUFF* sizeof(char));
    /* demande à l'annuaire */
    creationMessage(31, (void*) nomFichier, messageEnvoi);
    ecritureSocket(socketAnnuaire, messageEnvoi, TAILLE_BUFF);
    printf("le message suivant a ete envoye a l'annuaire : %s \n", messageEnvoi);
    /* traitement de la réponse de l'annuaire */
    while (!finDialogue)
    {
        ecouteSocket(socketAnnuaire, message, TAILLE_BUFF);

        if (sscanf(message, "%d", &code) == 1)
        {
            /* analyse du code :
               1 - réponse positive de l'annuaire
               2 - réponse négative de l'annuaire
               autre - message non destiné au client */
            switch (code)
            {
            case 11:
                /* réponse positive de l'annuaire */
                compteur++;
                nbBlocTotal = traitementMessagePositif(message);
                if (compteur == nbBlocTotal)
                {
                    finDialogue = 1;
                }
                break;
            case 12:
                /* réponse négative de l'annuaire */
                traitementMessageNegatif(message);
                finDialogue = 1;
                break;
            default:
                /* code inconnu : coupure du dialogue */
                finDialogue = 1;
                break;
            }
        }
        else
        {
            /* le code au début du message reçu n'a pas été reconnu */
            /* message ignoré */
            fprintf(stderr, "Le message suivant %s non reconnu a été reçu : %s\n",message, strerror(errno));
            pause();
            exit(1);
        }
    }
    /* libération de l'espace mémoire */
    free(message);
}

/**
* @note procédure qui analyse la réponse positive de l'annuaire
* @param buff : chaine de caractère à traiter
* @return la fonction retourne le nombre de bloc total du fichier
*/
int traitementMessagePositif(char* buff)
{
    /* variables */
    int code;                       /* entier correspondant au code du message */
    Telechargement* blocAAjouter;   /* structure Telechargement à rajouter à la fin de la liste d'attente */
    Fichier* fichierAAjouter;       /* structure Fichier à rajouter à la fin de la liste (si besoin) */
    unsigned int nbTotalBloc;       /* entier stockant le nombre total de bloc du fichier */
    Fichier* tempFichier;           /* structure Fichier temporaire pour la recherche */
    unsigned int i;                          /* compteur pour incrémenter une boucle */
    unsigned int tempIdServeur;     /* entier pour récupérer l'IDServeur */

    /* initialisation des variables */
    blocAAjouter = malloc(sizeof(Telechargement));
    blocAAjouter->nomFichier = malloc(100* sizeof(char));
    blocAAjouter->adresseServeur = malloc(100* sizeof(char));

    /* récupération des champs du message */
    if (sscanf(buff, "%d %u %s %u %u %u %s %d", &code, &(blocAAjouter->idFichier), blocAAjouter->nomFichier, &nbTotalBloc,
               &(blocAAjouter->numeroBloc), &tempIdServeur, blocAAjouter->adresseServeur, &(blocAAjouter->numPortServeur)) == 8)
    {
        /* recherche si le fichier est déja dans la liste des fichiers */
        /** verrou des mutex sur la liste des fichiers (lecture et écriture) */
        pthread_mutex_lock(&(listeFichier.mutexListeFichierLecture));
        pthread_mutex_lock(&(listeFichier.mutexListeFichierEcriture));
        /* initialisation en début de liste */
        tempFichier = listeFichier.listeFichiers;
        while ((tempFichier != NULL) && (strcmp(tempFichier->nomFichier, blocAAjouter->nomFichier) != 0))
        {
            /* tant que le fichier pointé n'est pas celui recherché, on avance dans la liste */
            tempFichier = tempFichier->fichierSuivant;
        }
        /* si le pointeur est égal à NULL, alors le fichier n'existe pas encore */
        if (tempFichier == NULL)
        {
            /* allocation de la structure */
            fichierAAjouter = malloc(sizeof(Fichier));
            fichierAAjouter->nomFichier = malloc(100* sizeof(char));
            fichierAAjouter->statutBlocs = malloc(nbTotalBloc* sizeof(int));
            pthread_mutex_init(&(fichierAAjouter->mutexFichierEcriture), NULL);
            /* initialisation des champs */
            for (i=0; i<nbTotalBloc; i++)
            {
                /* initialisation des status à 0 */
                fichierAAjouter->statutBlocs[i] = 0;
            }
            fichierAAjouter->nbBlocs = nbTotalBloc;
            strcpy(fichierAAjouter->nomFichier, blocAAjouter->nomFichier);
        }
        /** libération des mutex (écriture et lecture) */
        pthread_mutex_unlock(&(listeFichier.mutexListeFichierLecture));
        pthread_mutex_unlock(&(listeFichier.mutexListeFichierEcriture));

        /* ajout du bloc du fichier dans la liste d'attente */
        /** verrou mutex liste d'attente (écriture) */
        pthread_mutex_lock(&(listeAttenteTelechargement.mutexListeAttenteClient));
        /* incrémentation du nombre de téléchargement */
        listeAttenteTelechargement.nbTelechargements++;
        /* rajout du bloc à la suite du dernier bloc actuel */
        (listeAttenteTelechargement.dernierTelechargement)->telechargementSuivant = blocAAjouter;
        /* affectation du nouveau dernier bloc */
        listeAttenteTelechargement.dernierTelechargement = blocAAjouter;
        /** libération du mutex liste d'attente (écriture) */
        pthread_mutex_unlock(&(listeAttenteTelechargement.mutexListeAttenteClient));

    }
    return nbTotalBloc;
}

/**
* @note procédure qui analyse la réponse négative de l'annuaire
* @param buff : chaine de caractère à traiter
*/
void traitementMessageNegatif(char* buff)
{
    /* variables */
    int code;           /* entier correspondant au code du message */
    char* nomFichier;   /* chaine de caractère correspondant au fichier demandé */

    /* initialisation */
    nomFichier = malloc(40* sizeof(char));

    /* affichage de l'échec de la recherche du fichier */
    if (sscanf(buff, "%d %s", &code, nomFichier) == 2)
    {
        printf("Le fichier suivant n'a pas été trouvé : %s\n", nomFichier);
    }
    /* libération de l'espace mémoire */
    free(nomFichier);
}

/**
* @note procédure qui lance en boucle des threads pour téléchargerdes blocs
*/
void threadTelechargement()
{
    /* variables */
    pthread_t variableThread;   /* variable pour lancer les threads de téléchargement */

    /* boucler tant que l'arret du serveur n'a pas été demandée */
    while (!finThreadClient)
    {
        /* s'il n'y a pas déjà trop de thread lancé */
        if ((nbThreadClientLance < NBTHREAD) && (listeAttenteTelechargement.premierTelechargement != NULL))
        {
            /* incrémentation du nombre de thread et création du thread */
            nbThreadClientLance++;
            pthread_create(&variableThread, NULL, (void*(*)(void*))threadRecuperationBloc, NULL);
        }
    }
}

/**
* @note procédure à exécuter dans un thread pour télécharger un bloc la focntion boucle tant qu'il y a des blocs à télécharger
*/
void threadRecuperationBloc()
{
    /* variables */
    Telechargement* telechargementATraiter;     /* pointeur sur le téléchargement à récupérer */

    /** verrou mutex liste d'attente (écriture) */
    pthread_mutex_lock(&(listeAttenteTelechargement.mutexListeAttenteClient));
    /* récupération du premier élément en liste d'attente */
    telechargementATraiter = listeAttenteTelechargement.premierTelechargement;
    /* boucler tant qu'il y a un élément en liste d'attente */
    while ((telechargementATraiter != NULL) && (!finThreadClient))
    {
        /* suppression de l'élément de la liste d'attente */
        listeAttenteTelechargement.nbTelechargements--;
        listeAttenteTelechargement.premierTelechargement = telechargementATraiter->telechargementSuivant;
        /* cas où il n'y a plus aucun téléchargement en attente */
        if (listeAttenteTelechargement.nbTelechargements == 0)
        {
            listeAttenteTelechargement.dernierTelechargement = NULL;
        }
        /** libération du mutex liste d'attente (écriture) */
        pthread_mutex_unlock(&(listeAttenteTelechargement.mutexListeAttenteClient));
        /* téléchargement du bloc */
        telechargementBloc(telechargementATraiter);
        /* libération de l'espace mémoire */
        free(telechargementATraiter->adresseServeur);
        free(telechargementATraiter->nomFichier);
        free(telechargementATraiter);

        /** verrou mutex liste d'attente (écriture) */
        pthread_mutex_lock(&(listeAttenteTelechargement.mutexListeAttenteClient));
        /* re-affectation du pointeur temporaire pour le prochain téléchargement */
        telechargementATraiter = listeAttenteTelechargement.premierTelechargement;
    }

    /** libération du mutex liste d'attente (écriture) */
    pthread_mutex_unlock(&(listeAttenteTelechargement.mutexListeAttenteClient));
    nbThreadClientLance--;
}

/**
* @note procédure qui demande et télécharge le bloc aupres du serveur
* @param telechargementATraiter : pointeur sur le bloc à télécharger
*/
void telechargementBloc(Telechargement* telechargementATraiter)
{
    Socket socketDialogue;  /* socket de dialogue avec le serveur */
    char* message;          /* chaine de caractère contenant le message destiné au serveur */
    char* buff;             /* chaine de caractère récupérant la réponse du serveur */
    int code;               /* entier correspondant au code du message reçu */
    int finDialogue;        /* booléen indiquant l'arret du dialogue */

    /* initialisation */
    message = malloc(TAILLE_BUFF* sizeof(char));
    buff = malloc(TAILLE_BUFF* sizeof(char));
    do
    {
        /* création d'une socket */
        socketDialogue = creationSocket();
        /* connexion à la socket du serveur */
        demandeConnexionSocket(socketDialogue, telechargementATraiter->adresseServeur, telechargementATraiter->numPortServeur);
        /* création du message à envoyer */
        creationMessage(41, (void*) telechargementATraiter, message);
        /* envoi de la demande de fichier */
        ecritureSocket(socketDialogue, message, TAILLE_BUFF);
        /* récupération de la réponse du serveur */
        ecouteSocket(socketDialogue, buff, TAILLE_BUFF);
        /* traitement de la réponse du serveur */
        if (sscanf(buff, "%d", &code) == 1)
        {
            /* analyse du code :
               11 - reception du bloc
               12 - bloc introuvable
               autre - message non destiné au client */
            switch (code)
            {
            case 11:
                /* reception du bloc */
                traitementMessageReceptionBloc(socketDialogue, buff);
                break;
            case 12:
                /* bloc introuvable, nouvelle demande à l'annuaire */
                finDialogue = traitementMessageBlocIntrouvable(telechargementATraiter);
                break;
            default:
                /* code inconnu : coupure du dialogue */
                finDialogue = 1;
                break;
            }
        }
        else
        {
            /* le code au début du message reçu n'a pas été reconnu */
            /* message ignoré */
            perror("Le message suivant non reconnu a été reçu : ");
            perror(message);
        }
        /* fermeture de la socket */
        clotureSocket(socketDialogue);
        /* boucle tant que la fin du dialogue n'est pas demandé */
    }
    while ((!finDialogue) && (!finThreadClient));

    /* libération de l'espace mémoire */
    free(telechargementATraiter->adresseServeur);
    free(telechargementATraiter->nomFichier);
    free(telechargementATraiter);
    free(message);
    free(buff);
}

/**
* @note procédure qui analyse la reception d'un bloc
* @param socketDialogue : socket sur laquelle le message est reçu
* @param buff : chaine de caractère à traiter
*/
void traitementMessageReceptionBloc(Socket socketDialogue, char* buff)
{
    /* variables */
    FILE* fichierAEcrire;   /* fichier de destination pour le bloc */
    int code;               /* entier correspondant au code du message */
    unsigned int idFichier;          /* entier pour l'IDFichier */
    char* nomFichier;       /* chaine de caractère récupérant le nom du fichier */
    unsigned int numeroBloc;         /* entier corerspondant au numéro du bloc reçu */
    char* contenuBloc;      /* chaine de caractère avec le contenu du bloc */
    int tailleLu;           /* taille du bloc attendu dans le message suivant */
    Fichier* tempFichier;   /* pointeur pour parcourir la liste des fichiers */
    char* cheminFichier;    /*  */

    /* initialisation */
    nomFichier = malloc(TAILLE_BUFF* sizeof(char));
    contenuBloc = malloc(TAILLE_BLOC* sizeof(char));
    cheminFichier = malloc(100* sizeof(char));

    /* récupération de la partie de fichier */
    sscanf(buff, "%d %u %s %u %d", &code, &idFichier, nomFichier, &numeroBloc, &tailleLu);
    ecouteSocket(socketDialogue, contenuBloc, tailleLu);
    /* modification du nom de fichier en fichier ".temp" */
    strcat(nomFichier, ".temp");

    /** blocage du mutex en écriture sur la liste de fichier */
    pthread_mutex_lock(&(listeFichier.mutexListeFichierLecture));
    /* initialisation */
    tempFichier = listeFichier.listeFichiers;
    /* recherche du fichier dans la liste */
    while ((tempFichier != NULL) && (tempFichier->idFichier != idFichier))
    {
        tempFichier = tempFichier->fichierSuivant;
    }
    /** libération du mutex en écriture sur la liste de fichier */
    if (tempFichier == NULL)
    {
        /* le fichier n'est pas dans la liste des fichiers : erreur */
        printf("Erreur inconnu sur les fichiers!!! \n");
    }
    else
    {
        /* récupération du chemin du fichier */
        strcpy(cheminFichier, "reception\\");
        strcat(cheminFichier, nomFichier);
        /** blocage du mutex en écriture sur le fichier */
        pthread_mutex_lock(&(tempFichier->mutexFichierEcriture));
        /* ouverture du fichier */
        fichierAEcrire = fopen(cheminFichier, "w");
        /* avancement dans le fichier */
        fseek(fichierAEcrire, numeroBloc * TAILLE_BLOC, SEEK_SET);
        /* écriture des données */
        fwrite((void*) contenuBloc, sizeof(char), tailleLu, fichierAEcrire);
        /* fermeture du fichier */
        fclose(fichierAEcrire);
        /** libération du mutex en écriture sur le fichier */
        pthread_mutex_unlock(&(tempFichier->mutexFichierEcriture));

        /* mise à jour de la liste des fichiers */
        (tempFichier->statutBlocs)[numeroBloc] = 1;
        /* test si c'est le dernier bloc */
        if (tempFichier->nbBlocs == (numeroBloc + 1))
        {
            /* cas où on est en train d'écrire le dernier bloc */
            /* mise à jour de la taille total du fichier */
            tempFichier->tailleFichier = (tempFichier->nbBlocs -1) * TAILLE_BLOC + tailleLu;
        }
        /* finalisation fichier si besoin */
        finalisationFichier(tempFichier);
    }
    /* libération de l'espace mémoire */
    free(nomFichier);
}

/**
* @note procédure qui analyse le fait de ne pas avoir trouvé le bloc
* @param telechargementATraiter : pointeur sur le téléchargement en cours
* @return retourne 0 si l'annuaire a renvoyé un autre serveur pour récupérer le bloc, 1 sinon
*/
int traitementMessageBlocIntrouvable(Telechargement* telechargementATraiter)
{
    /* variables */
    char* buff;                 /* chaine de caractère pour le message reçu */
    char* message;              /* chaine de caractère pour le message à envoyer */
    Telechargement* donneeRecu; /* structure pour stocker les données recu */
    int code;                   /* entier pour récupérer le code du message reçu */
    unsigned int nbTotalBloc;   /* entier pour récupérer le nombre total de bloc */
    unsigned int tempIdServeur; /* entier pour récupérer l'idServeur */

    /* initialisation */
    message = malloc(TAILLE_BUFF * sizeof(char));
    buff = malloc(TAILLE_BUFF * sizeof(char));
    /* création du message */
    creationMessage(32, (void*) telechargementATraiter, message);
    /* demande un nouveau serveur à l'anuaire */
    ecritureSocket(socketAnnuaire, message, TAILLE_BUFF);
    /* récupération du message de l'annuaire */
    ecouteSocket(socketAnnuaire, buff, TAILLE_BUFF);
    /* traitement de la réponse de l'annuaire */

    if (sscanf(buff, "%d", &code) == 1)
    {
        if (code == 1)
        {
            /* initialisation */
            donneeRecu = malloc(sizeof(Telechargement));
            donneeRecu->adresseServeur = malloc(100* sizeof(char));
            donneeRecu->nomFichier = malloc(100* sizeof(char));
            /* comparaison des données du serveur avec l'ancien */
            if (sscanf(buff, "%d %u %s %u %u %u %s %d", &code, &(donneeRecu->idFichier), donneeRecu->nomFichier, &nbTotalBloc,
                       &(donneeRecu->numeroBloc), &tempIdServeur, donneeRecu->adresseServeur, &(donneeRecu->numPortServeur)) == 8)
            {
                /* si le serveur renvoyé par l'annuaire est différent du serveur actuel */
                if ((donneeRecu->adresseServeur != telechargementATraiter->adresseServeur)
                        || (donneeRecu->numPortServeur != telechargementATraiter->numPortServeur))
                {
                    /* libération de l'espace mémoire */
                    free(telechargementATraiter->adresseServeur);
                    free(telechargementATraiter->nomFichier);
                    free(telechargementATraiter);
                    /* modification des données avec le nouveau serveur */
                    telechargementATraiter = donneeRecu;
                    /* libération de l'espace mémoire */
                    free(message);
                    free(buff);
                    return 0;
                }
            }
        }
    }
    /* soit l'annuaire renvoi un message inconnu
       soit le l'annuaire ne trouve pas le bloc demandé
       soit les données du serveur sont les meme que celle déja dans la base de donnée */
    free(message);
    free(buff);
    return 1;
}

/**
* @note procédure qui test si le fichier est complet (et le finalise le cas échéant)
* @param pointeurFichier : pointeur sur le fichier en cours de traitement
*/
void finalisationFichier(Fichier* pointeurFichier)
{
    /* variables */
    unsigned int compteur;               /* entier pour parcourir les blocs */
    FILE* fichierDestination;   /* fichier de destination pour la recopie */
    FILE* fichierSource;        /* fichier source pour la recopie */
    char* nomFichierTemp;       /* chaine de caractère pour le nom de fichier temporaire */
    char* contenuBloc;          /* chaine de caractère contenant le bloc lu */
    int tailleDernierBloc;      /* entier contanant la taille du dernier bloc du fichier */
    Fichier* tempFichier;       /* pointeur sur fichier poru le parcours de la liste */
    char* cheminFichier;

    /* initialisation */
    compteur = 0;
    nomFichierTemp = malloc(100* sizeof(char));
    contenuBloc = malloc(TAILLE_BLOC* sizeof(char));
    cheminFichier = malloc(100* sizeof(char));

    /* parcours du tableau de statut */
    while (compteur < pointeurFichier->nbBlocs)
    {
        if ((pointeurFichier->statutBlocs)[compteur] == 0)
        {
            break;
        }
        compteur++;
    }
    /* test sur les conditions de sorties */
    if (compteur == (pointeurFichier->nbBlocs))
    {
        /* récupération du chemin du fichier */
        strcpy(cheminFichier, "reception\\");
        strcat(cheminFichier, pointeurFichier->nomFichier);
        /* création du fichier destination */
        fichierDestination = fopen(cheminFichier, "w");
        /* ouverture du fichier temporaire */
        strcpy(nomFichierTemp, cheminFichier);
        strcat(nomFichierTemp, ".temp");
        /* ouverture du fichier temporaire */
        fichierSource = fopen(nomFichierTemp, "r");

        /* recopie du fichier en supprimant les caractères inutiles en fin de fichier */
        for (compteur = 0; compteur < (pointeurFichier->nbBlocs - 1); compteur++)
        {
            fread(contenuBloc, sizeof(char), TAILLE_BLOC, fichierSource);
            fwrite(contenuBloc, sizeof(char), TAILLE_BLOC, fichierDestination);
        }
        /* calcul de la taille du dernier bloc */
        tailleDernierBloc = pointeurFichier->tailleFichier - ((pointeurFichier->nbBlocs -1) * TAILLE_BLOC);
        /* recopie du dernier bloc sans les "0" en trop */
        fread(contenuBloc, sizeof(char), tailleDernierBloc, fichierSource);
        fwrite(contenuBloc, sizeof(char), tailleDernierBloc, fichierDestination);
        /* fermeture des fichiers */
        fclose(fichierSource);
        fclose(fichierDestination);

        /* suppression de la liste des fichiers */
        /** verrou mutex sur la liste de fichier (lecture et ecriture) */
        pthread_mutex_lock(&(listeFichier.mutexListeFichierEcriture));
        pthread_mutex_lock(&(listeFichier.mutexListeFichierLecture));
        /* initialisation du pointeur */
        tempFichier = listeFichier.listeFichiers;
        /* test si le fichier recherché est le premier */
        if (tempFichier == pointeurFichier)
        {
            /* suppression du fichier de la liste */
            listeFichier.listeFichiers = tempFichier->fichierSuivant;
        }
        else
        {
            /* parcours de la liste pour rechercher le "précédent" */
            while (tempFichier->fichierSuivant != pointeurFichier)
            {
                /* passage au fichier suivant */
                tempFichier = tempFichier->fichierSuivant;
            }
            /* suppression du fichier de la liste */
            tempFichier->fichierSuivant = pointeurFichier->fichierSuivant;
        }
        /** liberation mutex sur la liste de fichier (lecture et ecriture) */
        pthread_mutex_unlock(&(listeFichier.mutexListeFichierEcriture));
        pthread_mutex_unlock(&(listeFichier.mutexListeFichierLecture));

        /* libération de l'espace mémoire */
        free(pointeurFichier->nomFichier);
        free(pointeurFichier->statutBlocs);
        pthread_mutex_destroy(&(pointeurFichier->mutexFichierEcriture));
        free(pointeurFichier);
    }
    /* libération de l'espace mémoire */
    free(nomFichierTemp);
    free(contenuBloc);
}

/**
* @note procédure qui signale l'arret du client à l'annuaire, et l'arrete
*/
void arretClient()
{
    /* variables */
    char* message;              /* chaine de caractère pour le message à envoyer */
    Telechargement* tempBloc;   /* pointeur temporaire pour supprimer la liste d'attente */
    Fichier* tempFichier;       /* pointeur temporaire pour supprimer la liste de fichiers */

    /** envoi du message d'arret à l'annuaire */
    /* iniitalisation des variables */
    message = malloc(TAILLE_BUFF* sizeof(char));
    /* création du message */
    if (creationMessage(33, NULL, message) == 1)
    {
        printf("Erreur inconnu !!!\n");
        exit(1);
    }
    /* envoi du message à l'annuaire */

    ecritureSocket(socketAnnuaire, message, TAILLE_BUFF);
    /* libération de l'espace mémoire */
    free(message);

    /** vidage de la liste d'attente */
    tempBloc = listeAttenteTelechargement.premierTelechargement;
    while (tempBloc != NULL)
    {
        /* affectation du nouveau premier élément de la liste */
        listeAttenteTelechargement.premierTelechargement = tempBloc->telechargementSuivant;
        /* libération de l'espace mémoire occupé */
        free(tempBloc->nomFichier);
        free(tempBloc->adresseServeur);
        free(tempBloc);
        /* re-affectation du pointeur temporaire */
        tempBloc = listeAttenteTelechargement.premierTelechargement;
    }
    /* libération des mutex */
    pthread_mutex_destroy(&(listeAttenteTelechargement.mutexListeAttenteClient));

    /** vidage de la liste des fichiers */
    tempFichier = listeFichier.listeFichiers;
    while (tempFichier != NULL)
    {
        /* affectation du nouveau premier fichier de la liste */
        listeFichier.listeFichiers = tempFichier->fichierSuivant;
        /* libération de l'espace mémoire occupé */
        free(tempFichier->nomFichier);
        free(tempFichier->statutBlocs);
        free(tempFichier);
        /* re-affectation du pointeur temporaire */
        tempFichier = listeFichier.listeFichiers;
    }
    /* libération des mutex */
    pthread_mutex_destroy(&(listeFichier.mutexListeFichierEcriture));
    pthread_mutex_destroy(&(listeFichier.mutexListeFichierLecture));

    arretApplication++;
}

/*****************
* Fin de Fichier
*****************/
