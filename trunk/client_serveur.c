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

/**
* main et fonctions communes
*/

int main()
{
    /* variables */
    char adresseAnnuaire[100];  /* stocke l'adresse de l'annuaire à utiliser */
    int portAnnuaire;           /* stocke le port de l'annuaire à utiliser */
    char resultat;              /* stocke la frappe au clavier si la connexion a échoué */
    pthread_t variableThread;   /* variable pour lancer les threads "serveur" et "client" */

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
            scanf("%c", &resultat);
            if (resultat == 'N' || resultat == 'n')
            {
                /* si l'utilisateur ne veut pas se connecter à un autre annuaire */
                printf("Au revoir\n");
                exit(1);
            }
        }
        else
        {
            /* si la connexion à réussi, initialisation de la variable "resultat" */
            resultat = 'N';
        }
    }while (resultat == 'O' || resultat == 'o');

    /* demande du port du serveur */
    printf("Sur quel port voulez-vous lancer le serveur ?\n");
    scanf("%d", &portServeur);

    /* La connexion à l'annuaire est établie : lancement de 2 threads : serveur et client */
    pthread_create(&variableThread, NULL, (void*(*)(void*))applicationServeur, NULL);
    pthread_create(&variableThread, NULL, (void*(*)(void*))applicationClient, NULL);

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
    	case 3 :
    	/* message de demande de fichier à l'annuaire */
            message = "3 fichier ";
            strcat(message, (char*) structure);
    		break;
    	case 4 :
    	/* message de demande de bloc à l'annuaire */
            message = "4 bloc ";
            sprintf(tempChaine, "%d", ((Telechargement*) structure)->idFichier);
            strcat(message, tempChaine);
            strcat(message, " ");
            strcat(message, ((Telechargement*) structure)->nomFichier);
            strcat(message, " ");
            sprintf(tempChaine, "%d", ((Telechargement*) structure)->numeroBloc);
            strcat(message, tempChaine);
    		break;
    	case 5 :
    	/* message d'arret du client à l'annuaire */
    	    /* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            message = "5 arret ";
            strcat(message, hp->h_addr_list[0]);
            /* libération de l'espace mémoire */
            free(hp);
    		break;
    	case 6 :
    	/* message de demande de bloc à un serveur */
            message = "6 bloc ";
            sprintf(tempChaine, "%d", ((Telechargement*) structure)->idFichier);
            strcat(message, tempChaine);
            strcat(message, " ");
            strcat(message, ((Telechargement*) structure)->nomFichier);
            strcat(message, " ");
            sprintf(tempChaine, "%d", ((Telechargement*) structure)->numeroBloc);
            strcat(message, tempChaine);
    		break;
    	case 7 :
    	/* message d'arret du client aux serveurs */
            /* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            message = "7 arret ";
            strcat(message, hp->h_addr_list[0]);
            /* libération de l'espace mémoire */
            free(hp);
            break;
    	case 8 :
    	/* message de disponibilité d'un bloc à l'annuaire */
    	/* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            message = "8 bloc ";
            sprintf(tempChaine, "%d", ((StructureDisponibiliteBloc*) structure)->idFichier);
            strcat(message, tempChaine);
            strcat(message, " ");
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
    	case 9 :
    	/* message d'arret du serveur à l'annuaire */
            /* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            message = "9 arret ";
            strcat(message, hp->h_addr_list[0]);
            /* libération de l'espace mémoire */
            free(hp);
            break;
    	case 10 :
    	/* message de charge du serveur vers l'annuaire */
            message = "10 charge ";
            sprintf(tempChaine, "%d", *((int*) structure));
            strcat(message, tempChaine);
    		break;
    	case 11:
    	/* envoi d'un bloc d'un serveur à un client */
            message = "11 bloc ";
            sprintf(tempChaine, "%d", ((Client*) structure)->idFichier);
            strcat(message, tempChaine);
            strcat(message, " ");
            strcat(message, ((Client*) structure)->nomFichier);
            strcat(message, " ");
            sprintf(tempChaine, "%d", ((Client*) structure)->numeroBloc);
            strcat(message, tempChaine);
    		break;
    	case 12:
    	/* envoi d'un bloc d'un serveur à un client */
            message = "12 erreur ";
            sprintf(tempChaine, "%d", ((Client*) structure)->idFichier);
            strcat(message, tempChaine);
            strcat(message, " ");
            strcat(message, ((Client*) structure)->nomFichier);
            strcat(message, " ");
            sprintf(tempChaine, "%d", ((Client*) structure)->numeroBloc);
            strcat(message, tempChaine);
    		break;
    	case 13:
    	/* envoi d'un bloc d'un serveur à un client */
            /* récupération du nom et du port du serveur */
            hp = gethostbyname("localhost");
            /* création du message */
            message = "13 arret ";
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

/**
* code des fonctions et procédures coté serveur
*/

void applicationServeur()
{
    /* déclatation des variables */
    pthread_t variableThread;       /* variable pour lancer les threads : un d'écoute, et un d'emmision */

    /* initialisation des variables globales du coté serveur */
    finThreadServeur = 0;
    nbThreadServeurLance = 1;

    /* initialisation de la liste d'attente */
    initialisationListeAttenteClient(listeAttenteClient);

    /* signalisation des fichiers disponibles à l'annuaire*/
    signalisationFichierAnnuaire(socketAnnuaire);

    /* création du thread pour écouter les demandes clients */
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadDialogueClient, NULL);

    /* création du thread gérant les envoi de blocs */
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadEmmission, NULL);

    /* lecture du clavier en attente du message "fin serveur" */
    while (!finThreadServeur)
    {
        char buff[TAILLE_BUFF];     /* variable pour récupérer le message tapé par l'utilisateur */

        fgets(buff, TAILLE_BUFF, stdin);
        /* Le dernier carractère est un retour chariot */
        buff[strlen(buff)-1] = '\0';

        if (strcmp(buff, "fin serveur") == 0)
        {
            /* si le texte "fin serveur" est entré : on arrete le serveur */
            finThreadServeur = 1;
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

/** ********************* A faire ***************************** */
void signalisationFichierAnnuaire(Socket socketAnnuaire)
{

}

void initialisationListeAttenteClient(FileAttenteClients listeAttente)
{
    /* mise a 0 du nombre de client, et mise a NULL des deux pointeurs */
    listeAttente.nbClients = 0;
    listeAttente.premierClient = NULL;
    listeAttente.dernierClient = NULL;
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
        creationMessage(13, NULL, message);
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
    char* mot;                  /* chaine de caractère correspondant au mot clé */
    Client* clientAAjouter;     /* structure "Client" temporaire à ajouter en fin de liste d'attente */

    /* initialisation des variables */
    mot = malloc(20 * sizeof(char));
    clientAAjouter = malloc(sizeof(Client));
    clientAAjouter->nomFichier = malloc(100 * sizeof(char));

    /* récupération du nom du fichier, et du numéro de bloc à partir du message reçu */
    if (sscanf(buff, "%d %s %s %d", &code, mot, clientAAjouter->nomFichier, &(clientAAjouter->numeroBloc)) == 4)
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
    /* libération des variables */
    free(mot);
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
    creationMessage(10, (void*) &valeur, message);
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
    if( (fichierALire = fopen(client->nomFichier, "r")) == 0)
    {/* le fichier a été trouvé */
        /* récupération de la chaine appropriée */
        fseek(fichierALire, (client->numeroBloc) * TAILLE_BLOC, SEEK_SET);
        if (fread((void*) buff, sizeof(char), TAILLE_BLOC, fichierALire) <0)
        {
            printf("Erreur de lecture du fichier!!! \n");
            exit(1);
        }
        /* fermeture du fichier */
        fclose(fichierALire);
        /* création du message à envoyer */
        creationMessage(11, (void*) client, message);
        strcat(message, buff);
    }
    else
    {/* le fichier n'a pas été trouvé */
        /* création du message à envoyer */
        creationMessage(12, (void*) client, message);
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
    creationMessage(9, NULL, message);
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

    /* lancement du thread de demande de fichier à l'annuaire */
    nbThreadClientLance++;
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadDemandeFichier, NULL);

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

void threadDemandeFichier()
{
    /* boucle qui demande à l'utilisateur des fichiers à télécharger
        tant que l'arret du client n'a pas été demandé */
    while (!finThreadClient)
    {
        /* variables */
        char buff[TAILLE_BUFF];

        /* affichage du texte */
        printf("Veuillez entrer un nom de fichier a telecharger, ");
        printf("vous pouvez aussi rentrer  \"fin client\" ou \"fin serveur\" ");
        printf("pour arreter le client ou le serveur.\n ");
        /* lecture de la frappe clavier */
        fgets(buff, TAILLE_BUFF, stdin);
        /* Le dernier carractère est un retour chariot */
        buff[strlen(buff)-1] = '\0';

        if (strcmp(buff, "fin client") == 0)
        {
            /* si le texte "fin client" est entré : on arrete le client */
            finThreadClient = 1;
        }
        else if (strcmp(buff, "fin serveur") == 0)
        {
            /* si le texte "fin serveur" est entré : on arrete le serveur */
            finThreadServeur = 1;
        }
        else
        {
            /* traitement de la demande de téléchargement d'un fichier */
            demandeFichier(buff);
        }
        /* boucle jusqu'à ce que le texte "fin client" est entré */
    }
    /* décrémentation du nombre de thread */
    nbThreadClientLance--;
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
    creationMessage(3, (void*) nomFichier, messageEnvoi);
    ecritureSocket(socketAnnuaire, messageEnvoi);

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
            perror("Le message suivant non reconnu a été reçu : ");
            perror(message);
        }
    }
    /* libération de l'espace mémoire */
    free(message);
}

int traitementMessagePositif(char* buff)
{
    /* variables */
    int code;                       /* entier correspondant au code du message */
    char* mot;                      /* chaine de caractère correspondant au mot clé du message */
    Telechargement* blocAAjouter;   /* structure Telechargement à rajouter à la fin de la liste d'attente */
    Fichier* fichierAAjouter;       /* structure Fichier à rajouter à la fin de la liste (si besoin) */
    int nbTotalBloc;                /* entier stockant le nombre total de bloc du fichier */
    Fichier* tempFichier;           /* structure Fichier temporaire pour la recherche */
    int i;                          /* compteur pour incrémenter une boucle */

    /* initialisation des variables */
    blocAAjouter = malloc(sizeof(Telechargement));
    blocAAjouter->nomFichier = malloc(100* sizeof(char));
    blocAAjouter->adresseServeur = malloc(100* sizeof(char));
    mot = malloc(20* sizeof(char));

    /* récupération des champs du message */
    if (sscanf(buff, "%d %s %s %d %d %s %d", &code, mot, blocAAjouter->nomFichier, &nbTotalBloc
                   , &(blocAAjouter->numeroBloc), blocAAjouter->adresseServeur, &(blocAAjouter->numPortServeur)) == 7);
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
    /* libération de l'espace mémoire */
    free(mot);
    return nbTotalBloc;
}

void traitementMessageNegatif(char* buff)
{
    /* variables */
    int code;           /* entier correspondant au code du message */
    char* mot;          /* chaine de caractère correspondant au mot clé du message */
    char* nomFichier;   /* chaine de caractère correspondant au fichier demandé */

    /* initialisation */
    mot = malloc(10* sizeof(char));
    nomFichier = malloc(40* sizeof(char));

    /* affichage de l'échec de la recherche du fichier */
    if(sscanf(buff, "%d %s %s", &code, mot, nomFichier) == 3)
    {
        printf("Le fichier suivant n'a pas été trouvé : %s\n", nomFichier);
    }
    /* libération de l'espace mémoire */
    free(mot);
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
        creationMessage(6, (void*) telechargementATraiter, message);
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
                traitementMessageReceptionBloc(buff);
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

void traitementMessageReceptionBloc(char* buff)
{

}

int traitementMessageBlocIntrouvable(Telechargement* telechargementATraiter)
{
    /* variables */
    char* buff;                 /* chaine de caractère pour le message reçu */
    char* message;              /* chaine de caractère pour le message à envoyer */
    Telechargement* donneeRecu; /* structure pour stocker les données recu */
    int code;                   /* entier pour récupérer le code du message reçu */
    char* mot;                  /* chaine de caractère pour le mot clé */
    int nbTotalBloc;            /* entier pour récupérer le nombre total de bloc */
    int idServeur;              /* entier pour récupérer l'idServeur */

    /* initialisation */
    message = malloc(100 * sizeof(char));
    buff = malloc(100 * sizeof(char));
    mot = malloc(100 * sizeof(char));
    /* création du message */
    creationMessage(4, (void*) telechargementATraiter, message);
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
            if(sscanf(buff, "%d %s %d %s %d %d %d %s %d", &code, mot, &(donneeRecu->idFichier), donneeRecu->nomFichier,
                &nbTotalBloc, &(donneeRecu->numeroBloc), &idServeur, donneeRecu->adresseServeur, &(donneeRecu->numPortServeur)) == 9)
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
                    free(mot);
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
    free(mot);
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
    if (creationMessage(5, NULL, message) == 1)
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
}

/*****************
* Fin de Fichier
*****************/

