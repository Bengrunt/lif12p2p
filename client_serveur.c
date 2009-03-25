/**
 * @file: client_serveur.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 20/03/2009
 */

#include "client_serveur.h"
#define NBTHREAD 10
#define TAILLE_BUFF 100

FileAttenteClients listeAttenteClient;
int finThread;
Socket socketAnnuaire;
pthread_mutex_t mutexListeAttente;
int portServeur;

int main()
{
    int res;
    char adresseAnnuaire[100];
    int portAnnuaire;
    char resultat;

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
    }
    while (resultat == 'O' || resultat == 'o');

    /* La connexion à l'annuaire est établie : découpage de l'application en 2 processus : serveur et client*/
    if ( (res = fork()) < 0)
    {
        printf("Erreur dans le fork !!!!! \n");
        exit(1);
    }
    else if (res == 0)
    {
        applicationServeur();
    }
    else
    {
        applicationClient();
    }
    return 0;
}

/**
* code des fonctions et procédures coté serveur
*/

void applicationServeur()
{
    /* déclatation des variables */
    pthread_t variableThread;
    pthread_t tabThread[NBTHREAD];
    int i;

    finThread = 0;
    /* initialisation de la liste d'attente */
    initialisationListeAttenteClient(listeAttenteClient);

    /* signalisation des fichiers disponibles à l'annuaire*/
    signalisationFichierAnnuaire();

    /* création du thread pour écouter les demandes clients */
    pthread_create(&variableThread, NULL, threadDialogueClient, NULL);

    /* création des threads pour envoyer des blocs */
    for (i = 0; i<NBTHREAD; i++)
    {
        pthread_create(&tabThread[i], NULL, threadEnvoiMessage, NULL);
    }

    /* surveillance du clavier pour détecter s'il y a une frappe */
    while (1)
    {
        char buff[TAILLE_BUFF];
        fgets(buff, TAILLE_BUFF, stdin);
        /* Le dernier carractère est un retour chariot */
        buff[strlen(buff)-1] = '\0';

        if (strcmp(buff, "fin serveur") == 0)
        {
            /* si le texte "fin serveur" est entré : on arrete le serveur */
            break;
        }
    }

    /* arrêt du serveur */
    /* arrêt de tous les threads lancés */
    finThread = 1;
    for (i = 0; i<NBTHREAD; i++)
    {
        pthread_join(tabThread[i],NULL);
    }
    /* suppression de la liste d'attente */
    arretServeur();

}

void signalisationFichierAnnuaire()
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
    int quitter;
    int i;
    pthread_t* tabThread;
    tabThread = malloc(20*sizeof(pthread_t));

    /* demande du port du serveur */
    printf("Sur quel port voulez-vous lancer le serveur ?\n");
    scanf("%d", &portServeur);

    /* crée une socket et affectation de la socket */
    socketEcouteServeur = creationSocket();
    definitionNomSocket(socketEcouteServeur, portServeur);
    /* écoute les demandes de connexion */
    quitter = 1;
    i = 0;
    while (quitter)
    {
        /* récupération des sockets de dialogue */
    	socketDemandeConnexion = acceptationConnexion(socketEcouteServeur);

    	/* création d'un thread pour dialoguer avec chaque client */
    	pthread_create(&tabThread[i], NULL, dialogueClient, (void*)socketDemandeConnexion);
    	i++;

    	/* aggrandissement du tableau des threads */

    }

    /*  */

}

void dialogueClient()
{

}

void threadEnvoiMessage()
{
    /* il faut sortir de la fonction quand la variable globale "finThread" vaut 1 */
    while (finThread == 0)
    {
        Client* blocAEnvoyer;  /* pointeur de travail qui désignera le bloc à envoyer */



        /*******  bloqué le mutex pour éviter la lecture / écriture de la liste d'attente  *********/
        pthread_mutex_lock(&mutexListeAttente);
        /* sélection du prochain client en liste d'attente */
        blocAEnvoyer = listeAttenteClient.premierClient;


        if (blocAEnvoyer != NULL)
        {
            /* suppression de l'élément sélectionné */
            listeAttenteClient.premierClient = listeAttenteClient.premierClient->clientSuivant;

            /*******  libéré le mutex  ********/
            pthread_mutex_unlock(&mutexListeAttente);
            /* signalisation de la charge du serveur à l'annuaire */
            signalisationChargeServeur(1);
            /* envoi du bloc */
            envoiMessage(blocAEnvoyer);

            /* signalisation de la charge du serveur à l'annuaire */
            signalisationChargeServeur(-1);
        }
        else
        {
            /*******  libéré le mutex  ********/
            pthread_mutex_unlock(&mutexListeAttente);
        }
    }/* boucle tant que finThread vaut 0 */
}

void signalisationChargeServeur(int valeur)
{

}



void envoiMessage(Client* client)
{

}

void arretServeur()
{
    struct hostent *hp;
    char* message;
    char* numPortServeur;

    message = malloc(100* sizeof(char));

    /* récupération du nom et du port du serveur */
    hp = gethostbyname("localhost");
    sprintf(numPortServeur, "%d", portServeur);

    /* création du message à envoyer à l'annuaire */
    message = "7 arret ";
    strcat(message, hp->h_addr_list[0]);
    strcat(message, numPortServeur);

    /* envoi du message à l'annuaire */
    ecritureSocket(socketAnnuaire, message);
}

/**
* code des fonctions et procédures coté client
*/

void applicationClient()
{

}



/*****************
* Fin de Fichier
*****************/
