/**
 * @file: client_serveur.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 01/04/2009
 */

#include "client_serveur.h"
#define NBTHREAD 10
#define TAILLE_BUFF 100

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

/**
* main
*/

int main()
{
    char adresseAnnuaire[100];
    int portAnnuaire;
    char resultat;
    pthread_t variableThread;

    /* demande de la socket de l'annuaire */
    do
    {
        printf("Quelle est l'adresse de l'annuaire utilisé ?\n");
        scanf("%s", adresseAnnuaire);
        printf("Quel est le port de l'annuaire utilisé ?\n");
        scanf("%d", &portAnnuaire);

        /* tentative de connexion à l'annuaire */
        /* création de la socket */
        socketAnnuaire = creationSocket();
        /* connexion a l'annuaire */
        if (demandeConnexionSocket(socketAnnuaire) == 1)
        {
            printf("La connexion a l'annuaire a échouer ! Voulez-vous vous connecter sur un autre annuaire ? (O/N)\n");
            scanf("%c", &resultat);
            if (resultat == 'N' || resultat == 'n')
            {
                printf("Au revoir\n");
                exit(1);
            }
        }
        else
        {
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

/**
* code des fonctions et procédures coté serveur
*/

void applicationServeur()
{
    /* déclatation des variables */
    pthread_t variableThread;

    finThreadServeur = 0;
    nbThreadServeurLance = 1;

    /* initialisation de la liste d'attente */
    initialisationListeAttenteClient(listeAttenteClient);

    /* signalisation des fichiers disponibles à l'annuaire*/
    signalisationFichierAnnuaire(socketAnnuaire);

    /* création du thread pour écouter les demandes clients */
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadDialogueClient, NULL);

    /* création des threads pour envoyer des blocs */
    pthread_create(&variableThread, NULL, (void*(*)(void*))threadEmmission, NULL);

    /* lecture du clavier en attente du message "fin serveur" */
    while (!finThreadServeur)
    {

        char buff[TAILLE_BUFF];

        fgets(buff, TAILLE_BUFF, stdin);
        /* Le dernier carractère est un retour chariot */
        buff[strlen(buff)-1] = '\0';

        if (strcmp(buff, "fin serveur") == 0)
        {
            /* si le texte "fin serveur" est entré : on arrete le serveur */
            finThreadServeur = 1;
        }
    }

    /* arrêt du serveur */
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
    Socket socketEcouteServeur;
    Socket socketDemandeConnexion;
    pthread_t variableThread;

    /* crée une socket et affectation de la socket */
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
    char* buff;
    int code;
    int finDialogue;        /* variable indiquant  si on doit sortir de la boucle de discution
                                0- on continue a écouter
                                1- on srot de la boucle */

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

    /* fermeture de la socket */
    /* soit par ce que le serveur va s'arreter */
    if (finThreadServeur == 1)
    {
        struct hostent *hp;
        char* message;
        char* numPortServeur;

        message = malloc(100* sizeof(char));
        numPortServeur = malloc (10* sizeof(char));

        /* récupération du nom et du port du serveur */
        hp = gethostbyname("localhost");
        sprintf(numPortServeur, "%d", portServeur);

        /* création du message à envoyer au client */
        message = "13 arret ";
        strcat(message, hp->h_addr_list[0]);
        strcat(message, " ");
        strcat(message, numPortServeur);

        /* envoi du message */
        ecritureSocket(socketDialogue, message);

        /* libération de l'espace mémoire */
        free(message);
        free(numPortServeur);
        free(hp);
    }
    /* soit parce que le client le demande */
    clotureSocket(socketDialogue);
    free(buff);
}

void traitementMessageBloc(Socket socketDialogue, char* buff)
{
    /* analyse du message et ajout en liste d'attente */
    int code;
    char* mot;
    Client* clientAAjouter;

    /* initialisation des variables */
    mot = malloc(20 * sizeof(char));
    clientAAjouter = malloc(sizeof(Client));
    clientAAjouter->nomFichier = malloc(100 * sizeof(char));
    /* récupération du nom du fichier, et du numéro de bloc */
    if (sscanf(buff, "%d %s %s %d", &code, mot, clientAAjouter->nomFichier, &(clientAAjouter->numeroBloc)) == 4)
    {
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
    free(mot);
}

void traitementMessageArret(Socket socketDialogue, char* buff)
{
    /* suppression du client de la liste d'attente */
    Client* tempClient;

    /*******  bloquage du mutex pour éviter la lecture / écriture de la liste d'attente  *********/
    pthread_mutex_lock(&(listeAttenteClient.mutexListeAttenteServeur));
    tempClient = listeAttenteClient.premierClient;

    /* cas s'il faut supprimer le premier client de la liste d'attente */
    while (tempClient != NULL && tempClient->socketClient == socketDialogue)
    {
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
        tempClient = listeAttenteClient.premierClient;
    }
    /** le client pointé par "tempClient" n'est pas à supprimer, test sur son suivant */
    /* parcours du reste de la liste d'attente s'il reste des clients en liste d'attente */
    if (tempClient != NULL)
    {
        while (tempClient->clientSuivant != NULL)
        {
            /* test si le client est à supprimer */
            if ((tempClient->clientSuivant)->socketClient == socketDialogue)
            {
                Client* tempClientSupprimer;
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
            tempClient = tempClient->clientSuivant;
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
    pthread_t variableThread;

    /* boucler tant que l'arret du serveur n'a pas été demandée */
    while(!finThreadServeur)
    {
        /* s'il n'y a pas déjà trop de thread lancé */
        if (nbThreadServeurLance < NBTHREAD)
        {
            /* incrémentation du nombre de thread et création du thread */
            nbThreadServeurLance++;
            pthread_create(&variableThread, NULL, (void*(*)(void*))threadEnvoiMessage, NULL);
        }
    }
}

void threadEnvoiMessage()
{
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
    char* message;
    char* messageValeur;

    /* initialisation des variables */
    message = malloc(50* sizeof(char));
    messageValeur = malloc(5* sizeof(char));

    /* création de la chaine à envoyer */
    sprintf(messageValeur, "%d", valeur);
    message = "10 charge ";
    strcat(message, messageValeur);

    /* envoi du message */
    /** Ajouter un code de message pour la charge du serveur vesr l'annuaire */
    ecritureSocket(socketAnnuaire, message);

    /* libération de l'espace mémoire */
    free(message);
    free(messageValeur);
}

void envoiMessage(Client* client)
{
    char* buff;
    FILE* fichierALire;

    buff = malloc(65536 * sizeof(char));

    /* ouverture du fichier */
    fichierALire = fopen(client->nomFichier, "r");
    /* récupération de la chaine appropriée */
    fseek(fichierALire, (client->numeroBloc) * 65536, SEEK_SET);

    /** Utliser la fonction "read" pour récupérer un bloc dans un fichier */
  /*  fscanf(fichierALire, "%s", buff);  */
    /* écriture des données sur la socket */
    ecritureSocket(client->socketClient, buff);

    /* fermeture du fichier */
    fclose(fichierALire);
    free(buff);
}

void arretServeur()
{
    /* variable temporaire */
    struct hostent *hp;
    char* message;
    char* numPortServeur;
    Client* tempClient;

    /* iniitalisation des variables */
    message = malloc(100* sizeof(char));
    numPortServeur = malloc(10* sizeof(char));

    /* récupération du nom et du port du serveur */
    hp = gethostbyname("localhost");
    sprintf(numPortServeur, "%d", portServeur);

    /* création du message à envoyer à l'annuaire */
    message = "9 arret ";
    strcat(message, hp->h_addr_list[0]);
    strcat(message, " ");
    strcat(message, numPortServeur);

    /* envoi du message à l'annuaire */
    ecritureSocket(socketAnnuaire, message);

    free(message);
    free(numPortServeur);

    /* vidage de la liste d'attente */
    tempClient = listeAttenteClient.premierClient;
    while (tempClient != NULL)
    {
        /* affectation du novueau premier élément de la liste */
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
    pthread_t variableThread;

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
    /* arrêt de tous les threads lancés */
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
    /* boucle qui demande à l'utilisateur des fichiers à télécharger */
    while (!finThreadClient)
    {
        char buff[TAILLE_BUFF];
        printf("Veuillez entrer un nom de fichier a telecharger, ");
        printf("vous pouvez aussi rentrer  \"fin client\" ou \"fin serveur\" ");
        printf("pour arreter le client ou le serveur.\n ");

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
    int finDialogue;
    char* message;
    int code;
    int nbBlocTotal;
    int compteur;

    /* initialisation */
    finDialogue = 0;
    compteur = 0;
    message = malloc(200* sizeof(char));
    /* demande à l'annuaire */
    ecritureSocket(socketAnnuaire, nomFichier);

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
                /* message d'arret du client */
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
    free(message);


}

int traitementMessagePositif(char* buff)
{
    /* variable */
    int code;
    char* mot;
    Telechargement* blocAAjouter;
    Fichier* fichierAAjouter;
    int nbTotalBloc;

    /* initialisation des variables */
    blocAAjouter = malloc(sizeof(Telechargement));
    blocAAjouter->nomFichier = malloc(100* sizeof(char));

    /* récupération des champs du message */
    if (sscanf(buff, "%d %s %s %d %d %s %d", &code, mot, blocAAjouter->nomFichier, &nbTotalBloc
                   , &(blocAAjouter->numeroBloc), blocAAjouter->adresseServeur, &(blocAAjouter->numPortServeur)) == 7);
    {
        /* test si le fichier est déja dans la liste des fichiers */
            /** verrou mutex sur la liste des fichiers (lecture) */
        while (1)
        {


        }
        /* si le fichier n'existe pas : ajout */
        if(1)
        {
            fichierAAjouter = malloc(sizeof(Fichier));
            /* allocation de la structure */
            /** verrou mutex sur la liste des fichiers (écriture) */
        }
        /** libération des mutex (écriture et lecture) */


        /* ajout du bloc du fichier dans la liste d'attente */
        /** verrou mutex liste d'attente (écriture) */

        /** libération du mutex liste d'attente (écriture) */
    }




}

void traitementMessageNegatif(char* buff)
{
    int code;
    char* mot;
    char* nomFichier;

    /* initialisation */
    mot = malloc(10* sizeof(char));
    nomFichier = malloc(40* sizeof(char));

    /* affichage de l'échec de la recherche du fichier */
    if(sscanf(buff, "%d %s %s", &code, mot, nomFichier) == 3)
    {
        printf("Le fichier suivant n'a pas été trouvé : %s\n", nomFichier);
    }
    free(mot);
    free(nomFichier);
}

void threadTelechargement()
{
    pthread_t variableThread;

    /* boucler tant que l'arret du serveur n'a pas été demandée */
    while(!finThreadClient)
    {
        /* s'il n'y a pas déjà trop de thread lancé */
        if (nbThreadClientLance < NBTHREAD)
        {
            /* incrémentation du nombre de thread et création du thread */
            nbThreadClientLance++;
            pthread_create(&variableThread, NULL, (void*(*)(void*))threadRecuperationBloc, NULL);
        }
    }
}

void threadRecuperationBloc()
{

}

/*Telechargement* rechercheBloc()
{

}*/

void arretClient()
{
     /* variable temporaire */
    struct hostent *hp;
    char* message;
    Telechargement* tempBloc;

    /* iniitalisation des variables */
    message = malloc(100* sizeof(char));

    /* récupération du nom et du port du serveur */
    hp = gethostbyname("localhost");

    /* création du message à envoyer à l'annuaire */
    message = "05 arret ";
    strcat(message, hp->h_addr_list[0]);

    /* envoi du message à l'annuaire */
    ecritureSocket(socketAnnuaire, message);

    free(message);

    /* vidage de la liste d'attente */
    tempBloc = listeAttenteTelechargement.premierTelechargement;
    while (tempBloc != NULL)
    {
        /* affectation du novueau premier élément de la liste */
    	listeAttenteTelechargement.premierTelechargement = tempBloc->telechargementSuivant;
    	/* libération de l'espace mémoire occupé */
    	free(tempBloc->nomFichier);
    	free(tempBloc->adresseServeur);
    	free(tempBloc);
        /* re-affectation du pointeur temporaire */
    	tempBloc = listeAttenteTelechargement.premierTelechargement;
    }


}


/*****************
* Fin de Fichier
*****************/

