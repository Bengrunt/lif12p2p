/**
 * @file: client_serveur.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 05/04/2009
 */

#include "client_serveur.h"
#define NBTHREAD 10
#define TAILLE_BUFF 100
#define TAILLE_BLOC 65536

/**
* Variables globales
*/

FileAttenteTelechargements listeAttenteTelechargement;
FileAttenteClients listeAttenteClient;
int finThreadServeur;
int finThreadClient;
Socket socketAnnuaire;
int portServeur;
int nbThreadServeurLance;
int nbThreadClientLance;
ListeFichiers listeFichier;
int arretApplication;

/**
* main et fonctions communes
*/

int main()
{
    /* variables */
    char* adresseAnnuaire;      /* stocke l'adresse de l'annuaire à utiliser */
    int portAnnuaire;           /* stocke le port de l'annuaire à utiliser */
    char* resultat;             /* stocke la frappe au clavier si la connexion a échoué */
    pthread_t variableThread;   /* variable pour lancer les threads "serveur" et "client" */

    /* initalisation */
    finThreadClient = 0;
    finThreadServeur = 0;
    arretApplication = 0;
    resultat = malloc(2* sizeof(char));
    adresseAnnuaire = malloc(100* sizeof(char));

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
    }while (strcmp(resultat,"O") == 0 || strcmp(resultat,"o") == 0);

    /* demande du port du serveur */
    printf("Sur quel port voulez-vous lancer le serveur ?\n");
    scanf("%d", &portServeur);

    /* La connexion à l'annuaire est établie : lancement de 2 threads : serveur et client */
    pthread_create(&variableThread, NULL, (void*(*)(void*))applicationServeur, NULL);
    pthread_create(&variableThread, NULL, (void*(*)(void*))applicationClient, NULL);
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadLectureClavier, NULL);

    /* libération de l'espace mémoire */
    free(adresseAnnuaire);
    /* boucle tant que les applications client et serveur ne sont pas terminées */
    while (1)
    {
    	if(arretApplication == 2)
    	{
    	    break;
    	}
    }

    return 0;
}

int creationMessage(int code, void* structure, char* message)
{
    /* variables */
    struct hostent *hp;         /* structure récupérant le nom du client */
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
    	    /* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            strcpy(message,"33 ");
            strcat(message, hp->h_addr_list[0]);
            /* libération de l'espace mémoire */
            free(hp);
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
            /* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            strcpy(message, "42 ");
            strcat(message, hp->h_addr_list[0]);
            /* libération de l'espace mémoire */
            free(hp);
            break;
    	case 51 :
    	/* message de disponibilité d'un bloc à l'annuaire */
    	/* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            strcpy(message,"51 ");
/*            sprintf(tempChaine, "%d", ((StructureDisponibiliteBloc*) structure)->idFichier);
            strcat(message, tempChaine);
            strcat(message, " "); */
            strcat(message, ((StructureDisponibiliteBloc*) structure)->nomFichier);
            strcat(message, " ");
            sprintf(tempChaine, "%d", ((StructureDisponibiliteBloc*) structure)->numTotalBloc);
            strcat(message, tempChaine);
            strcat(message, " ");
            sprintf(tempChaine, "%d", ((StructureDisponibiliteBloc*) structure)->numeroBloc);
            strcat(message, tempChaine);
            strcat(message, " ");
            sprintf(tempChaine, "%d", ((StructureDisponibiliteBloc*) structure)->idServeur);
            strcat(message, tempChaine);
            strcat(message, " ");
            strcat(message, hp->h_addr_list[0]);
            strcat(message, " ");
            sprintf(tempChaine, "%d", portServeur);
            strcat(message, tempChaine);
            /* libération de l'espace mémoire */
            free(hp);
    		break;
    	case 52 :
    	/* message d'arret du serveur à l'annuaire */
            /* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            strcpy(message,"52 ");
            strcat(message, hp->h_addr_list[0]);
            /* libération de l'espace mémoire */
            free(hp);
            break;
    	case 53 :
    	/* message de charge du serveur vers l'annuaire */
            strcpy(message,"53 ");
            sprintf(tempChaine, "%d", *((int*) structure));
            strcat(message, tempChaine);
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
            strcat(message, ((Client*) structure)->nomFichier);
            strcat(message, " ");
            sprintf(tempChaine, "%d", ((Client*) structure)->numeroBloc);
            strcat(message, tempChaine);
    		break;
    	case 63 :
    	/* message de déconnexion du serveur vers les clients */
            /* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            strcpy(message,"63 ");
/*            sprintf(tempChaine, "%d", *((int*) structure));
            strcat(message, tempChaine);
            strcat(message, " ");  */
            strcat(message, hp->h_addr_list[0]);
            strcat(message, " ");
            sprintf(tempChaine, "%d", portServeur);
            strcat(message, tempChaine);
            /* libération de l'espace mémoire */
            free(hp);
    		break;
    	default:
    	/* code non reconnu */
            return 1;
    		break;
    }
    free(tempChaine);
    return 0;
}

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
        printf("3- Arreter le serveur\n");
        printf("4- Arreter le client\n");
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
    }while ((finThreadClient == 0) && (finThreadServeur == 0));
}

void lireLigne(char* message)
{
    do
    {
        /* lecture d'une ligne au clavier */
        fgets(message, TAILLE_BUFF, stdin);
        /* Le dernier carractère est un retour chariot */
        message[strlen(message)-1] = '\0';
    /* boucle tant que la la ligne lu est vide */
    }while (strcmp(message,"") == 0);
}

/**
* code des fonctions et procédures coté serveur
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
    while(1)
    {
        /* on boucle tant qu'il y a au moins un thread lancé */
        if (nbThreadServeurLance != 0)
        {
            break;
        }

    }
    /* suppression de la liste d'attente */
    arretServeur();

}

void signalisationFichierAnnuaire(char* nomFichier)
{
    /* variables */
    FILE* fichierADecouper;
    int nbBloc;
    char* message;
    char* lectureFichier;

    /* initialisation */
    message = malloc(200* sizeof(char));
    lectureFichier = malloc((TAILLE_BLOC)* sizeof(char));
    nbBloc = -1;

    /* ouverture du fichier */
    if ((fichierADecouper = fopen(nomFichier, "r")) == NULL)
    {
        /* le fichier recherché n'a pas été trouvé */
        printf("Le fichier suivant n'a pas été trouvé : %s\n", nomFichier);
    }
    else
    {
        /* récupération du nombre de bloc */
        while(!feof(fichierADecouper))
        {
            fread((void*)lectureFichier, sizeof(char), TAILLE_BLOC, fichierADecouper);
            nbBloc++;
        }
        /* libération de l'espace mémoire */
        fclose(fichierADecouper);
        /* test si le fichier fait 0 octet */
        if (nbBloc == -1)
        {
            /* le fichier fait 0 octet */
            printf("Le fichier fait 0 octet, il ne peut pas etre partagé.\n");
        }
        else
        {
            StructureDisponibiliteBloc structurePourEnvoi;
            structurePourEnvoi.nomFichier = nomFichier;
            structurePourEnvoi.numTotalBloc = nbBloc + 1;
/*            structurePourEnvoi.idServeur  */
            /* envoi d'un message pour chaque bloc lu */
            for (nbBloc = nbBloc; nbBloc > -1; nbBloc--)
            {
                structurePourEnvoi.numeroBloc = nbBloc;
                /* création du message */
                creationMessage(51, (void*) &structurePourEnvoi, message);
                /* envoi des données à l'annuaire */
                ecritureSocket(socketAnnuaire, message);
            }
        }
    }
    free(message);
    free(lectureFichier);
}

void initialisationListeAttenteClient()
{
    /* mise a 0 du nombre de client, et mise a NULL des deux pointeurs
       et initialisation du mutex */
    listeAttenteClient.nbClients = 0;
    listeAttenteClient.premierClient = NULL;
    listeAttenteClient.dernierClient = NULL;
    pthread_mutex_init(&(listeAttenteClient.mutexListeAttenteServeur), NULL);

}

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

        /* création d'un thread pour dialoguer avec chaque client */
        pthread_create(&variableThread, NULL, (void*(*)(void*))dialogueClient, (void*)socketDemandeConnexion);
    }
    /* On sort de la boucle quand on recoit le signal "fin thread" : décrémentation du nombre de thread */
    nbThreadServeurLance--;
}

void dialogueClient(Socket socketDialogue)
{
    /* variables */
    char* buff;             /* chaine de caractère stockant le contenu du message reçu */
    int code;               /* entier correspondant au code du message reçu */
    int finDialogue;        /* variable indiquant si on doit sortir de la boucle de discution
                                0- on continue a écouter
                                1- on sort de la boucle */

    /* initialisation des variables */
    buff = malloc(100 * sizeof(char));
    finDialogue = 0;

    while (!finThreadServeur && !finDialogue)
    {
        /* récupération du prochain message sur la socket */
        ecouteSocket(socketDialogue, buff);

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
                traitementMessageArret(socketDialogue, buff);
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

    /** fermeture de la socket */
    /* soit par ce que le serveur va s'arreter */
    if (finThreadServeur == 1)
    {   /* il faut alors envoyer un message à tous les clients */
        /* variables */
        char* message;          /* chaine de caractère pour créer le message à envoyer */

        /* initialisation */
        message = malloc(100* sizeof(char));
        /* création du message */
        creationMessage(63, NULL, message);
        /* envoi du message */
        ecritureSocket(socketDialogue, message);

        /* libération de l'espace mémoire */
        free(message);
    }
    /* soit parce que le client le demande */
    clotureSocket(socketDialogue);
    free(buff);
}

void traitementMessageBloc(Socket socketDialogue, char* buff)
{   /** analyse du message et ajout en liste d'attente */
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

void traitementMessageArret(Socket socketDialogue, char* buff)
{   /** suppression du client de la liste d'attente */
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

void traitementMessageErreur(Socket socketDialogue)
{
    /* envoi du message d'erreur */
    ecritureSocket(socketDialogue, "14 erreur mauvais destinataire");
}

void threadEmmission()
{
    /* variables */
    pthread_t variableThread;       /* variable pour lancer les threads d'envoi de bloc */

    /* boucler tant que l'arret du serveur n'a pas été demandée */
    while(!finThreadServeur)
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

void signalisationChargeServeur(int valeur)
{
    /* variables */
    char* message;          /* chaine de caractère pour le message à envoyer */

    /* initialisation des variables */
    message = malloc(50* sizeof(char));
    /* création de la chaine à envoyer */
    creationMessage(53, (void*) &valeur, message);
    /* envoi du message */
    ecritureSocket(socketAnnuaire, message);
    /* libération de l'espace mémoire */
    free(message);
}

void envoiMessage(Client* client)
{
    /* variables */
    char* buff;             /* chaine de caractère pour le contenu du bloc */
    char* message;          /* chaine de caractère pour le message à envoyer */
    FILE* fichierALire;     /* fichier dans lequel il faut lire les données */

    /* initialisation */
    buff = malloc(TAILLE_BLOC * sizeof(char));
    message = malloc((TAILLE_BLOC + 200) * sizeof(char)); /* prévision de 200 caractère en plus du contenu du bloc */

    /* ouverture du fichier */
    if( (fichierALire = fopen(client->nomFichier, "r")) != NULL)
    {/* le fichier a été trouvé */
        /* récupération de la chaine appropriée */
        fseek(fichierALire, (client->numeroBloc) * TAILLE_BLOC, SEEK_SET);
        if (fread((void*) buff, sizeof(char), TAILLE_BLOC, fichierALire) < 0)
        {
            printf("Erreur de lecture du fichier!!! \n");
            exit(1);
        }
        /* fermeture du fichier */
        fclose(fichierALire);
        /* création du message à envoyer */
        creationMessage(61, (void*) client, message);
        strcat(message, buff);
    }
    else
    {/* le fichier n'a pas été trouvé */
        /* création du message à envoyer */
        creationMessage(62, (void*) client, message);
    }
    /* écriture des données sur la socket */
    ecritureSocket(client->socketClient, message);
    /* libération de l'espace mémoire */
    free(buff);
    free(message);
}

void arretServeur()
{
    /* variable */
    char* message;
    Client* tempClient;

    /* iniitalisation des variables */
    message = malloc(100* sizeof(char));

    /* création du message à envoyer à l'annuaire */
    creationMessage(52, NULL, message);
    /* envoi du message à l'annuaire */
    ecritureSocket(socketAnnuaire, message);

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

/**
* code des fonctions et procédures coté client
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
    while(1)
    {
        /* on boucle tant qu'il y a au moins un thread lancé */
        if (nbThreadClientLance != 0)
        {
            break;
        }
    }
    /* suppression de la liste d'attente */
    arretClient();
}

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

void demandeFichier(char* nomFichier)
{
    /* variables */
    int finDialogue;    /* booléen indiquant l'arrêt (ou non) du dialogue */
    char* messageEnvoi; /* chaine de caractère stockant le message à envoyer */
    char* message;      /* chaine de caractère stockant le message reçu */
    int code;           /* entier stockant le code du message */
    int nbBlocTotal;    /* entier stockant le nombre total de message (1 par bloc) attendu */
    int compteur;       /* compteur sur le nombre de message reçu */

    /* initialisation */
    finDialogue = 0;
    compteur = 0;
    message = malloc(200* sizeof(char));
    messageEnvoi = malloc(200* sizeof(char));
    /* demande à l'annuaire */
    creationMessage(31, (void*) nomFichier, messageEnvoi);
    ecritureSocket(socketAnnuaire, messageEnvoi);
    printf("le message suivant a ete envoye a l'annuaire : %s \n", messageEnvoi);
    /* traitement de la réponse de l'annuaire */
    while (!finDialogue)
    {
    	ecouteSocket(socketAnnuaire, message);

    	if (sscanf(message, "%d", &code) == 1)
        {
            /* analyse du code :
               1 - réponse positive de l'annuaire
               2 - réponse négative de l'annuaire
               autre - message non destiné au client */
            switch (code)
            {
            case 1:
                /* réponse positive de l'annuaire */
                compteur++;
                nbBlocTotal = traitementMessagePositif(message);
                if (compteur == nbBlocTotal)
                {
                    finDialogue = 1;
                }
                break;
            case 2:
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

int traitementMessagePositif(char* buff)
{
    /* variables */
    int code;                       /* entier correspondant au code du message */
    Telechargement* blocAAjouter;   /* structure Telechargement à rajouter à la fin de la liste d'attente */
    Fichier* fichierAAjouter;       /* structure Fichier à rajouter à la fin de la liste (si besoin) */
    int nbTotalBloc;                /* entier stockant le nombre total de bloc du fichier */
    Fichier* tempFichier;           /* structure Fichier temporaire pour la recherche */
    int i;                          /* compteur pour incrémenter une boucle */

    /* initialisation des variables */
    blocAAjouter = malloc(sizeof(Telechargement));
    blocAAjouter->nomFichier = malloc(100* sizeof(char));
    blocAAjouter->adresseServeur = malloc(100* sizeof(char));

    /* récupération des champs du message */
    if (sscanf(buff, "%d %s %d %d %s %d", &code, blocAAjouter->nomFichier, &nbTotalBloc
                   , &(blocAAjouter->numeroBloc), blocAAjouter->adresseServeur, &(blocAAjouter->numPortServeur)) == 6);
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
        if(tempFichier == NULL)
        {
            /* allocation de la structure */
            fichierAAjouter = malloc(sizeof(Fichier));
            fichierAAjouter->nomFichier = malloc(100* sizeof(char));
            fichierAAjouter->statutBlocs = malloc(nbTotalBloc* sizeof(int));
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

void traitementMessageNegatif(char* buff)
{
    /* variables */
    int code;           /* entier correspondant au code du message */
    char* nomFichier;   /* chaine de caractère correspondant au fichier demandé */

    /* initialisation */
    nomFichier = malloc(40* sizeof(char));

    /* affichage de l'échec de la recherche du fichier */
    if(sscanf(buff, "%d %s", &code, nomFichier) == 3)
    {
        printf("Le fichier suivant n'a pas été trouvé : %s\n", nomFichier);
    }
    /* libération de l'espace mémoire */
    free(nomFichier);
}

void threadTelechargement()
{
    /* variables */
    pthread_t variableThread;   /* variable pour lancer les threads de téléchargement */

    /* boucler tant que l'arret du serveur n'a pas été demandée */
    while(!finThreadClient)
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
}

void telechargementBloc(Telechargement* telechargementATraiter)
{
    Socket socketDialogue;  /* socket de dialogue avec le serveur */
    char* message;          /* chaine de caractère contenant le message destiné au serveur */
    char* buff;             /* chaine de caractère récupérant la réponse du serveur */
    int code;               /* entier correspondant au code du message reçu */
    int finDialogue;        /* booléen indiquant l'arret du dialogue */

    /* initialisation */
    message = malloc(100* sizeof(char));
    buff = malloc((TAILLE_BLOC + 200)* sizeof(char));
    do
    {
        /* création d'une socket */
        socketDialogue = creationSocket();
        /* connexion à la socket du serveur */
        demandeConnexionSocket(socketDialogue, telechargementATraiter->adresseServeur, telechargementATraiter->numPortServeur);
        /* création du message à envoyer */
        creationMessage(41, (void*) telechargementATraiter, message);
        /* envoi de la demande de fichier */
        ecritureSocket(socketDialogue, message);
        /* récupération de la réponse du serveur */
        ecouteSocket(socketDialogue, buff);
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
    }while((!finDialogue) && (!finThreadClient));

    /* libération de l'espace mémoire */
    free(telechargementATraiter->adresseServeur);
    free(telechargementATraiter->nomFichier);
    free(telechargementATraiter);
    free(message);
    free(buff);
}

void traitementMessageReceptionBloc(Socket socketDialogue, char* buff)
{
    /* variables */
    FILE* fichierAEcrire;
    int code;
    int idFichier;
    char* nomFichier;
    int numeroBloc;
    char* contenuBloc;

    /* initialisation */
    nomFichier = malloc(100* sizeof(char));
    contenuBloc = malloc(TAILLE_BLOC* sizeof(char));

    /* récupération de la partie de fichier */
    sscanf(buff, "%d %d %s %d", &code, &idFichier, nomFichier, &numeroBloc);
    ecouteSocket(socketDialogue, contenuBloc);
    /* recherche du fichier dans la liste */
    while (1)
    {

    }

        /** blocage du mutex en écriture sur le fichier*/
        pthread_mutex_lock(&());
        /* ouverture du fichier */
        fopen();
        /* avancement dans le fichier */
        fseek();
        /* écriture des données */
        fwrite();
        /* fermeture du fichier */
        fclose();
        /** libération du mutex */
        pthread_mutex_unlock(&());
        /* mise à jour de la liste des fichiers */

        /* test si c'est le dernier bloc */

        /* finalisation fichier : oui / non */

    /* libération de l'espace mémoire */
    free(nomFichier);
}

int traitementMessageBlocIntrouvable(Telechargement* telechargementATraiter)
{
    /* variables */
    char* buff;                 /* chaine de caractère pour le message reçu */
    char* message;              /* chaine de caractère pour le message à envoyer */
    Telechargement* donneeRecu; /* structure pour stocker les données recu */
    int code;                   /* entier pour récupérer le code du message reçu */
    int nbTotalBloc;            /* entier pour récupérer le nombre total de bloc */
    int idServeur;              /* entier pour récupérer l'idServeur */

    /* initialisation */
    message = malloc(100 * sizeof(char));
    buff = malloc(100 * sizeof(char));
    /* création du message */
    creationMessage(32, (void*) telechargementATraiter, message);
    /* demande un nouveau serveur à l'anuaire */
    ecritureSocket(socketAnnuaire, message);
    /* récupération du message de l'annuaire */
    ecouteSocket(socketAnnuaire, buff);
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
            if(sscanf(buff, "%d %d %s %d %d %d %s %d", &code, &(donneeRecu->idFichier), donneeRecu->nomFichier,
                &nbTotalBloc, &(donneeRecu->numeroBloc), &idServeur, donneeRecu->adresseServeur, &(donneeRecu->numPortServeur)) == 8)
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

void arretClient()
{
    /* variables */
    char* message;              /* chaine de caractère pour le message à envoyer */
    Telechargement* tempBloc;   /* pointeur temporaire pour supprimer la liste d'attente */
    Fichier* tempFichier;       /* pointeur temporaire pour supprimer la liste de fichiers */

    /** envoi du message d'arret à l'annuaire */
    /* iniitalisation des variables */
    message = malloc(100* sizeof(char));
    /* création du message */
    if (creationMessage(33, NULL, message) == 1)
    {
        printf("Erreur inconnu !!!\n");
        exit(1);
    }
    /* envoi du message à l'annuaire */
    ecritureSocket(socketAnnuaire, message);
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

