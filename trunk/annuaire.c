/**
 * @file annuaire.c
 * @project lif12p2p
 * @author Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since 20/03/2009
 * @version 9/04/2009
 */

#include "annuaire.h"

/**
* Variables Globales
*/

BddServeurs* serveurs; /* pointeur sur la liste des serveurs connus */
BddFichiers* fichiers; /* pointeur sur la liste des fichiers connus */

unsigned int generateurIdServeur; /* compteur de génération des idServeur */
unsigned int generateurIdFichier; /* compteur de génération des idFichier */

/**
* Fonctions et procédures du module
*/

/**
* @note procédure d'initialisation de l'annuaire :  les listes de clients, de serveurs, de fichiers.
* @brief modifie le contenu des variables globales serveurs et fichiers.
* @return renvoie 0 si tout se passe bien, -1 sinon.
*/
int initialisationAnnuaire()
{
    /* Creation des bases de données de serveurs */
    serveurs = malloc(sizeof(BddServeurs));
    if (serveurs == NULL)
    {
        perror("Echec de l'allocation en mémoire de la base de donnée des serveurs.\n");
        return -1;
    }
    serveurs->nbServeurs = 0;
    serveurs->capaTabServeurs = 1;
    serveurs->tabServeurs = malloc(serveurs->capaTabServeurs*sizeof(InfoServeurs*));
    pthread_mutex_init(&serveurs->verrou_bddserv_r, NULL);
    pthread_mutex_init(&serveurs->verrou_bddserv_w, NULL);

    /* Creation des bases de données de fichiers */
    fichiers = malloc(sizeof(BddFichiers));
    if (fichiers == NULL)
    {
        perror("Echec de l'allocation en mémoire de la base de donnée des fichiers.\n");
        return -1;
    }
    fichiers->nbFichiers = 0;
    fichiers->listeFichiers = NULL;
    pthread_mutex_init(&fichiers->verrou_bddfich_r, NULL);
    pthread_mutex_init(&fichiers->verrou_bddfich_w, NULL);

    /* Initialisation des générateurs d'identificateurs */
    generateurIdServeur = 0;
    generateurIdFichier = 0;
    return 0;
}


/**
* @note fonction d'initialisation de la socket d'écoute de l'annuaire.
* @param portAnnuaire : numéro de port sur lequel on crée la socket d'écoute.
* @return renvoie la socket créée.
*/
Socket initialiseSocketEcouteAnnuaire(int portAnnuaire)
{
    /* On déclare la socket pour l'annuaire */
    Socket socketEcouteAnnuaire;

    /* On la crée */
    socketEcouteAnnuaire = creationSocket();

    /* On lie la socket au numero de port portAnnuaire */
    definitionNomSocket(socketEcouteAnnuaire,portAnnuaire);

    return socketEcouteAnnuaire;
}


/**
* @note fonction globale de traitement d'un message reçu.
* @param arg : socket sur laquelle le message est arrivé.
* @return renvoie 0 si tout se passe bien, -1 sinon.
*/
int traiteMessage(Socket arg)
{
    /* variables */
    char* buff; /* Message lu sur la socket */

    int type_message; /* type de message */
    int fin_thread; /* booléen qui détermine si le thread doit se finir ou pas */

    /* Initialisation des booléens */
    fin_thread = 0; /* booleen qui décide de l'arret du thread */

    /* Allocation mémoire des chaines de caracteres */
    buff = malloc(200*sizeof(char));

    /* boucle de traitement des messages */
    while (!fin_thread)
    {
        /* on ecoute sur la socket arg */
        ecouteSocket(arg, buff, 200);

        /* on analyse le type du message reçu et on agit en conséquence */
        if (sscanf ( buff, "%d", &type_message) != 1 )
        {
            fprintf(stderr, "Message ignoré, impossible de l'analyser.\n Contenu du message: %s \n", buff);
        }
        else
        {
            printf("Message reçu.\n");
            /* On lance l'action correspondant au type de message. */
            switch (type_message)
            {
            case 31: /* Demande d'un fichier */
                traiteDemandeFichierClient(arg, buff);
                break;
            case 32: /* Demande d'un bloc */
                traiteDemandeBlocClient(arg, buff);
                break;
            case 33: /* Arret d'échange d'un client */
                traiteArretClient(arg, buff);
                fin_thread = 1;
                break;
            case 51: /* Disponibilité d'un bloc */
                traiteBlocDisponibleServeur(arg, buff);
                break;
            case 52: /* Arret d'un serveur */
                traiteArretServeur(arg, buff);
                fin_thread = 1;
                break;
            case 54: /* Demande IDServeur */
                traiteDemandeIdServeur(arg, buff);
                break;
            case 55: /* Demande IDFichier */
                traiteDemandeIdFichier(arg, buff);
                break;
            case 71: /* Indiquation que l'on a envoyé des messages au mauvais destinataire sur la socket donc fermeture */
                fin_thread = 1;
                break;
            default: /* Un message géré par le réseau a bien été reçu mais inadapté donc la connexion
                            doit être terminé car ce ne sont pas les bons interlocuteurs. */
                traiteMessageErr(arg, buff);
                fin_thread = 1;
            }
        }
    }

    /* Fermeture de la socket arg */
    clotureSocket(arg);

    return 0;

}


/**
* @note traitement d'un message de type demande de fichier d'un client.
* @param s : la socket sur laquelle la demande de fichier client a été émise.
* @param mess : la demande de fichier client a traiter.
*/
void traiteDemandeFichierClient(Socket s, char* mess)
{
    /* Variables */
    int fichTrouve; /* Booléen indiquant si le fichier a été trouvé dans la BDD des fichiers */
    int type_message; /* type du message : ici 31 */
    int i,j; /* itérateur */

    char* buff; /* tampon pour écriture du message */
    char* var_nomDeFichier; /* nom du fichier demandé */
    char* str_idFichier; /* transformation de l'identificateur du fichier en chaine de caractères */
    char* str_nbBlocs; /* transformation du nombre de blocs du fichier en chaine de caractères */
    char* str_numBloc; /* transformation du numéro du bloc du fichier en chaine de caractères */
    char* str_idServeur; /* transformation de l'identificateur de serveur en chaine de caractères */
    char* str_numPort; /* transformation du numéro de port du serveur en chaine de caractères */

    Fichier* ptBddFichiers; /* pointeur de travail pour se déplacer dans la BDD des fichiers */
    Serveur* ptListeServeurs; /* pointeur de travail pour se déplacer dans la liste des serveurs d'un bloc */

    /* On initialise le booleen de réussite :p */
    fichTrouve = 0; /* On a pas encore trouvé le fichier */

    /* Allocations mémoire des chaines de caractère */
    var_nomDeFichier = malloc(100*sizeof(char));
    str_idFichier = malloc(10*sizeof(char));
    str_idServeur = malloc(10*sizeof(char));
    str_nbBlocs = malloc(10*sizeof(char));
    str_numBloc = malloc(10*sizeof(char));
    str_numPort = malloc(10*sizeof(char));

    /* On récupère le contenu du message */
    /* Doit être de la forme "31 nomDeFichier" */
    if (sscanf(mess, "%d %s", &type_message, var_nomDeFichier ) < 2)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
    }

    /* On vérrouille en ecriture les BDD des fichiers et serveurs avant la lecture */
    pthread_mutex_lock(&(fichiers->verrou_bddfich_w));
    pthread_mutex_lock(&(serveurs->verrou_bddserv_w));

    /* On recherche ans la BDD des fichiers si on possède celui qui est demandé */
    ptBddFichiers = fichiers->listeFichiers;

    while ( ptBddFichiers != NULL)
    {
        /******** Le fichier demandé est trouvé ********/
        if ( strcmp( fichiers->listeFichiers->nomFichier, var_nomDeFichier ) == 0 )
        {
            /* Pour chaque bloc constituant du fichier on envoie un message indiquant où le télécharger au client */
            buff = malloc(200*sizeof(char));

            for (i = ptBddFichiers->nbBlocs;i>0;i--)
            {

                /* On envoie pour chaque bloc un message indiquant au client où télécharger chaque bloc */
                /* Message de la forme : "11 idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur" */
                strcpy(buff, "11 ");
                sprintf(str_idFichier, "%u", ptBddFichiers->idFichier);
                strcat(buff, str_idFichier);
                strcat(buff, " ");
                strcat(buff, ptBddFichiers->nomFichier);
                strcat(buff, " ");
                sprintf(str_nbBlocs, "%d", ptBddFichiers->nbBlocs);
                strcat(buff, str_nbBlocs);
                strcat(buff, " ");
                sprintf(str_numBloc, "%d", i);
                strcat(buff, str_numBloc);
                strcat(buff, " ");

                ptListeServeurs = ptBddFichiers->tabBlocs[i].listeServeurs;

                /* On a des serveurs proposant ce bloc */
                if (ptListeServeurs != NULL)
                {
                    /* On fait un rand parmi les serveurs pour répartir leur charge en attributan les requetes aléatoirement */
                    for (j = rand()%ptBddFichiers->tabBlocs[i].nbServeursDansListe;j>=0;j--)
                    {
                        ptListeServeurs = ptListeServeurs->serveurSuivant;
                    }

                    sprintf(str_idServeur, "%u", serveurs->tabServeurs[ptListeServeurs->numServeur]->idServeur);
                    strcat(buff, str_idServeur);
                    strcat(buff, " ");
                    strcat(buff, serveurs->tabServeurs[ptListeServeurs->numServeur]->adresseServeur);
                    strcat(buff, " ");
                    sprintf(str_numPort, "%d", serveurs->tabServeurs[ptListeServeurs->numServeur]->numPort);
                    strcat(buff, str_numPort);

                }
                /* On a pas de serveur proposant ce bloc, on envoie dans la réponse -1 -1 pour indiquer au client que des blocs sont manquants */
                else
                {
                    strcat(buff, " -1 -1");
                }

                /* Envoi du message sur la socket */
                ecritureSocket(s,buff,200);
            }
            /* Libération de la chaine après utilisation */
            free(buff);
            /* On a bien trouvé le fichier et envoyé le message */
            fichTrouve = 1;
        }
        /******** Le fichier n'a pas encore été trouvé, continue la recherche ********/
        else
        {
            ptBddFichiers = ptBddFichiers->fichierSuivant;
        }
    }
    /******** Le fichier n'est pas dans la BDD des fichiers, on envoie une réponse défavorable au client *********/
    if (!fichTrouve)
    {
        /* On envoie un message de réponse défavorable au client */
        /* Message de la forme: "02 erreur nomDeFichier" */
        buff = malloc(200*sizeof(char));

        strcpy(buff, "12 ");
        strcat(buff, var_nomDeFichier);

        /* Envoi du message sur la socket */
        ecritureSocket(s,buff,200);

        /* Libération de la chaine après utilisation */
        free(buff);
    }

    /* Libérations mémoire de chaines de caractère */
    free(var_nomDeFichier);
    free(str_idFichier);
    free(str_idServeur);
    free(str_nbBlocs);
    free(str_numBloc);
    free(str_numPort);

    /* On dévérouille en écriture les BDD des fichiers et serveurs après la lecture */
    pthread_mutex_unlock(&(fichiers->verrou_bddfich_w));
    pthread_mutex_unlock(&(serveurs->verrou_bddserv_w));
}


/**
* @note traitement d'un message de type demande de bloc d'un client.
* @param s : la socket sur laquelle la demande de bloc client a été émise.
* @param mess : la demande de bloc client a traiter.
*/
void traiteDemandeBlocClient(Socket s, char* mess)
{
    /* Variables */
    int type_message; /* type du message : ici 32 */
    unsigned int var_idFichier; /* identificateur du fichier */
    int var_numBloc; /* numero du bloc demandé */
    int j; /* Itérateur */
    int fichTrouve; /* Booléen indiquant si le fichier a été trouvé dans la BDD des fichiers */

    char* buff; /* tampon pour écriture du message */
    char* var_nomDeFichier; /* nom du fichier auquel appartient le bloc */
    char* str_idFichier; /* transformation de l'identificateur du fichier en chaine de caractères */
    char* str_nbBlocs; /* transformation du nombre de blocs du fichier en chaine de caractères */
    char* str_numBloc; /* transformation du numéro du bloc du fichier en chaine de caractères */
    char* str_idServeur; /* transformation de l'identificateur de serveur en chaine de caractères */
    char* str_numPort; /* transformation du numéro de port du serveur en chaine de caractères */

    Fichier* ptBddFichiers; /* pointeur de travail pour se déplacer dans la BDD des fichiers */
    Serveur* ptListeServeurs; /* pointeur de travail pour se déplacer dans la liste des serveurs d'un bloc */

    /* Initialisation des booléens */
    fichTrouve = 0;

    /* Allocations mémoire des chaines de caractère */
    var_nomDeFichier = malloc(100*sizeof(char));
    str_idFichier = malloc(10*sizeof(char));
    str_idServeur = malloc(10*sizeof(char));
    str_nbBlocs = malloc(10*sizeof(char));
    str_numBloc = malloc(10*sizeof(char));
    str_numPort = malloc(10*sizeof(char));

    /* On récupère le contenu du message */
    /* Doit être de la forme "32 idFichier nomDeFichier numeroDeBloc" */
    if (sscanf(mess, "%d %u %s %d", &type_message, &var_idFichier, var_nomDeFichier, &var_numBloc ) < 4)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
        exit(1);
    }

    /* On vérrouille en ecriture les BDD des fichiers et serveurs avant la lecture */
    pthread_mutex_lock(&(fichiers->verrou_bddfich_w));
    pthread_mutex_lock(&(serveurs->verrou_bddserv_w));

    /* On recherche ans la BDD des fichiers si on possède celui qui est demandé */
    ptBddFichiers = fichiers->listeFichiers;

    while ( ptBddFichiers != NULL)
    {
        /******** Le fichier demandé est trouvé ********/
        if ( strcmp( fichiers->listeFichiers->nomFichier, var_nomDeFichier ) == 0 && var_idFichier == fichiers->listeFichiers->idFichier)
        {
            /* On envoie un message indiquant où télécharger le bloc demandé au client */
            buff = malloc(200*sizeof(char));
            /* On envoie pour chaque bloc un message indiquant au client où télécharger chaque bloc */
            /* Message de la forme : "11 bloc idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur" */
            strcpy(buff, "11 ");
            sprintf(str_idFichier, "%u", ptBddFichiers->idFichier);
            strcat(buff, str_idFichier);
            strcat(buff, " ");
            strcat(buff, ptBddFichiers->nomFichier);
            strcat(buff, " ");
            sprintf(str_nbBlocs, "%d", ptBddFichiers->nbBlocs);
            strcat(buff, str_nbBlocs);
            strcat(buff, " ");
            sprintf(str_numBloc, "%d", var_numBloc);
            strcat(buff, str_numBloc);
            strcat(buff, " ");

            ptListeServeurs = ptBddFichiers->tabBlocs[var_numBloc].listeServeurs;

            /* On a des serveurs proposant ce bloc */
            if (ptListeServeurs != NULL)
            {
                /* On fait un rand parmi les serveurs pour répartir leur charge en attribuant les requetes aléatoirement */
                for (j = rand()%ptBddFichiers->tabBlocs[var_numBloc].nbServeursDansListe;j>=0;j--)
                {
                    ptListeServeurs = ptListeServeurs->serveurSuivant;
                }

                sprintf(str_idServeur, "%u", serveurs->tabServeurs[ptListeServeurs->numServeur]->idServeur);
                strcat(buff, str_idServeur);
                strcat(buff, " ");
                strcat(buff, serveurs->tabServeurs[ptListeServeurs->numServeur]->adresseServeur);
                strcat(buff, " ");
                sprintf(str_numPort, "%d", serveurs->tabServeurs[ptListeServeurs->numServeur]->numPort);
                strcat(buff, str_numPort);

            }
            /* On a pas de serveur proposant ce bloc, on envoie dans la réponse -1 -1 pour indiquer au client que des blocs sont manquants */
            else
            {
                strcat(buff, " -1 -1");
            }

            /* Envoi du message sur la socket */
            ecritureSocket(s,buff,200);
            /* Libération de la chaine après utilisation */
            free(buff);
            /* On a bien trouvé le fichier et envoyé le message */
            fichTrouve = 1;
        }
        /******** Le fichier n'a pas encore été trouvé, continue la recherche ********/
        else
        {
            ptBddFichiers = ptBddFichiers->fichierSuivant;
        }
    }
    /******** Le fichier n'est pas dans la BDD des fichiers, on envoie une réponse défavorable au client *********/
    if (!fichTrouve)
    {
        /* On envoie un message de réponse défavorable au client */
        /* Message de la forme: "12 erreur nomDeFichier" */
        buff = malloc(200*sizeof(char));

        strcpy(buff, "12 ");
        strcat(buff, var_nomDeFichier);

        /* Envoi du message sur la socket */
        ecritureSocket(s,buff,200);

        /* Libération de la chaine après utilisation */
        free(buff);
    }

    /* Libérations mémoire de chaines de caractère */
    free(var_nomDeFichier);
    free(str_idFichier);
    free(str_idServeur);
    free(str_nbBlocs);
    free(str_numBloc);
    free(str_numPort);

    /* On dévérouille en écriture les BDD des fichiers et serveurs après la lecture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}


/**
* @note traitement d'un message de type arret d'échange client.
* @param s : la socket sur laquelle le message d'arret client a été émis.
* @param mess : le message d'arret client a traiter.
*/
void traiteArretClient(Socket s, char* mess)
{

}


/**
* @note traitement d'un message de type nouveau bloc disponible sur serveur.
* @param s : la socket sur laquelle le message de nouveau bloc disponible sur serveur a été émis.
* @param mess : le message de nouveau bloc disponible a traiter.
*/
void traiteBlocDisponibleServeur(Socket s, char* mess)
{
    /* Variables */
    int type_message; /* Type du message : ici 51 */
    unsigned int var_idFichier; /* identificateur du fichier */
    int var_nbBlocs; /*  nombre de blocs du fichier */
    int var_numBloc; /* numero de bloc */
    unsigned int var_idServeur; /* identificateur du serveur */
    int var_portServeur; /* port du serveur */

    int fichTrouve; /* booléen */
    int servTrouve; /* booléen */

    char* var_nomDeFichier; /* nom du fichier */
    char* var_adresseServeur; /* adresse du serveur */

    Fichier* ptListeFichiers; /* pointeur de travail sur une liste de fichiers */
    Serveur* ptListeServeurs; /* pointeur de travail sur une liste de serveurs */

    /* Initialisation des booléens */
    fichTrouve = 0;
    servTrouve = 0;

    /* Allocations mémoire des chaines de caractère */
    var_nomDeFichier = malloc(100*sizeof(char));
    var_adresseServeur = malloc(40*sizeof(char));

    /* On récupère le contenu du message */
    /* Doit être de la forme "51 idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur" */
    if (sscanf(mess, "%d %u %s %d %d %u %s %d", &type_message, &var_idFichier, var_nomDeFichier, &var_nbBlocs, &var_numBloc, &var_idServeur, var_adresseServeur, &var_portServeur ) < 6)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
    }

    /* On vérouille les BDD des fichiers et serveurs en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock(&fichiers->verrou_bddfich_r);
    pthread_mutex_lock(&fichiers->verrou_bddfich_w);
    pthread_mutex_lock(&serveurs->verrou_bddserv_r);
    pthread_mutex_lock(&serveurs->verrou_bddserv_w);

    /* Mise à jour de la BDD des fichiers avec les nouvelles informations */
    ptListeFichiers = fichiers->listeFichiers;

    /* On parcourre la liste des fichiers pour trouver celui auquel on ajoute une nouvelle reference de bloc */
    while (ptListeFichiers != NULL && fichTrouve) /* Tant qu'on est pas au bout de la liste et qu'on a pas trouvé le fichier */
    {
        if ( strcmp(ptListeFichiers->nomFichier, var_nomDeFichier) == 0 && ptListeFichiers->idFichier == var_idFichier )
        {
            fichTrouve = 1;
            ptListeServeurs = ptListeFichiers->tabBlocs[var_numBloc].listeServeurs;

            while (ptListeServeurs != NULL && servTrouve) /* Tant qu'on arrive pas au bout de la liste des serveurs ou qu'on trouve le serveur déjà référencé */
            {
                if () /* Cas où la liste des serveurs est vide */
                {

                }
                else if (serveurs->tabServeurs[ptListeServeurs->numServeur]->idServeur == var_idServeur) /* Si on trouve le serveur pas besoin de le rajouter */
                {
                    servTrouve = 1;
                }
                else if ()
                {

                }
                else  /* Sinon on continue */
                {
                    ptListeServeurs = ptListeServeurs->serveurSuivant;
                }
            }
        }
        else if () /* Cas général */
        {

        }
        else /* Sinon on continue */
        {
            ptListeFichiers = ptListeFichiers->fichierSuivant;
        }
    }

    /* Libérations mémoire de chaines de caractère */
    free(var_nomDeFichier);
    free(var_adresseServeur);

    /* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_r);
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_r);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}


/**
* @note traitement d'un message de type arrêt de serveur.
* @param s : la socket sur laquelle le message d'arret de serveur a été emis.
* @param mess : le message d'arret serveur a traiter.
*/
void traiteArretServeur(Socket s, char* mess)
{
    /* Variables */
    int type_message; /* type du message reçu : ici 52 */
    unsigned int var_idServeur; /* identificateur du serveur */
    int var_portServeur; /* port du serveur */

    int i,j; /* itérateur */

    int sourceExiste; /* booléen */

    char* var_adresseServeur; /* adresse du serveur */

    Fichier* ptListeFichiers; /* pointeur de travail sur la liste des fichiers de la BDD */
    Fichier* ptRetListeFichiers; /* pointeur de travail sur la liste des fichiers de la BDD */

    Serveur* ptListeServeurs; /* pointeur de travail sur la liste des serveurs de la BDD */
    Serveur* ptRetListeServeurs; /* pointeur de travail sur la liste des serveurs de la BDD */

    Serveur* ptTempServeur; /* pointeur de travail sur un Serveur */
    Fichier* ptTempFichier; /* pointeur de travail sur un Fichier */

    /* Initialisation des booléens */
    sourceExiste = 0;

    /* Allocations mémoire des chaines de caractère */
    var_adresseServeur = malloc(40*sizeof(char));

    /* On récupère le contenu du message */
    /* Doit être de la forme "52 idServeur adresseServeur portServeur" */
    if (sscanf(mess, "%d %u %s %d", &type_message, &var_idServeur, var_adresseServeur, &var_portServeur) < 4)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
    }

    /* On vérouille la BDD des fichiers en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock(&fichiers->verrou_bddfich_r);
    pthread_mutex_lock(&fichiers->verrou_bddfich_w);
    pthread_mutex_lock(&serveurs->verrou_bddserv_r);
    pthread_mutex_lock(&serveurs->verrou_bddserv_w);

    /* On supprime de la BDD des fichiers les informations relatives au serveur qui s'arrete */
    ptListeFichiers = fichiers->listeFichiers;
    ptRetListeFichiers = ptListeFichiers;
    ptTempFichier = ptListeFichiers;

    while (ptListeFichiers != NULL) /* Tant que l'on a pas parcouru toute la liste des fichiers */
    {
        for (i = ptListeFichiers->nbBlocs;i>0;i--) /* Pour chaque bloc du fichier */
        {
            ptListeServeurs = ptListeFichiers->tabBlocs[i].listeServeurs;
            ptRetListeServeurs = NULL;

            while (ptListeServeurs != NULL) /* Tant que l'on a pas parcouru toute la liste des serveurs */
            {
                if (serveurs->tabServeurs[ptListeServeurs->numServeur]->idServeur == var_idServeur) /* Si on trouve le serveur à supprimer */
                {
                    if ( ptListeFichiers->tabBlocs[i].nbServeursDansListe == 1) /* Si il est le seul de la liste */
                    {
                        ptTempServeur = ptListeServeurs;
                        free(ptTempServeur);

                        ptListeFichiers->tabBlocs[i].listeServeurs = NULL;
                        ptListeFichiers->tabBlocs[i].nbServeursDansListe = 0;
                        break;
                    }
                    else /* Sinon il restera au moins un serveur dans la liste donc inutile de mettre NULL */
                    {
                        ptTempServeur = ptListeServeurs;
                        ptRetListeServeurs->serveurSuivant = ptListeServeurs->serveurSuivant;
                        ptListeServeurs = ptListeServeurs->serveurSuivant;
                        free(ptTempServeur);
                        break;
                    }
                }
                else /* Sinon on n'a pas encore trouvé le serveur à supprimer donc on continue à parcourir la liste des serveurs */
                {
                    ptRetListeServeurs = ptListeServeurs;
                    ptListeServeurs = ptListeServeurs->serveurSuivant;
                }
            }
        }

        for (i = ptListeFichiers->nbBlocs;i>0;i--) /* On vérifie qu'il reste encore au moins un bloc disponible pour le fichier */
        {
            if (ptListeFichiers->tabBlocs[i].listeServeurs != NULL)
            {
                sourceExiste = 1; /* On a trouvé encore au moins un bloc pour le fichier */
                break;
            }
        }

        if (!sourceExiste) /* Auquel cas on le supprime de la BDD des fichiers */
        {
            ptTempFichier = ptListeFichiers;
            ptRetListeFichiers->fichierSuivant = ptListeFichiers->fichierSuivant;
            ptListeFichiers = ptListeFichiers->fichierSuivant;
            free(ptTempFichier);
        }
        else /* Sinon on passe au fichier suivant */
        {
            ptRetListeFichiers = ptListeFichiers;
            ptListeFichiers = ptListeFichiers->fichierSuivant;
        }
    }

    /* On peut maintenant supprimer de la BDD des serveurs le serveur qui s'arrete */
    for (i = serveurs->nbServeurs;i>0;i--)
    {
        if (serveurs->tabServeurs[i]->idServeur == var_idServeur) /* Si on trouve le serveur on le supprime */
        {
            free(serveurs->tabServeurs[i]);
            for (j = i;j<serveurs->nbServeurs;j++)
            {
                serveurs->tabServeurs[j] = serveurs->tabServeurs[j+1];
            }
            serveurs->nbServeurs--;
            break;
        }
    }

    /* Libérations mémoire de chaines de caractère */
    free(var_adresseServeur);

    /* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_r);
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_r);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}


/**
* @note traitement d'un message de type demande d'ID serveur.
* @param s : la socket sur laquelle le message de demande d'ID serveur a été émis.
* @param mess  : le message de demande d'ID serveur à traiter.
* @warning incrémente le generateur d'idServeur.
*/
void traiteDemandeIdServeur(Socket s, char* mess)
{
    /* Variables */
    int type_message; /* type du message reçu : ici 54 */
    int var_portServeur; /* port du serveur */

    int i; /* itérateur */

    char* buff; /* tampon pour écriture du message */
    char* var_adresseServeur; /* adresse du serveur */
    char* str_portServeur; /* transformation du port du serveur en chaine de caractères */
    char* str_idServeur; /* transformation de l'identificateur du serveur en chaine de caractères */
    InfoServeurs** temp; /* pointeur de travail sur un tableau de pointeurs sur InfoServeurs */

    /* Allocations mémoire des chaines de caractère */
    buff = malloc(200*sizeof(char));
    var_adresseServeur = malloc(40*sizeof(char));
    str_portServeur = malloc(10*sizeof(char));
    str_idServeur = malloc(10*sizeof(char));

    /* On récupère le contenu du message */
    /* Doit être de la forme "54 adresseServeur portServeur" */
    if (sscanf(mess, "%d %s %d", &type_message, var_adresseServeur, &var_portServeur) < 3)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
    }

    /* On vérouille la BDD des fichiers en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock(&fichiers->verrou_bddfich_r);
    pthread_mutex_lock(&fichiers->verrou_bddfich_w);
    pthread_mutex_lock(&serveurs->verrou_bddserv_r);
    pthread_mutex_lock(&serveurs->verrou_bddserv_w);

    /* On référence le nouveau serveur dans la BDD des serveurs */
    if (serveurs->capaTabServeurs <= serveurs->nbServeurs) /* Si il n y a pas assez de place dans la tableau dynamique, on l'agrandit */
    {
        temp = serveurs->tabServeurs;
        serveurs->tabServeurs = malloc(2*serveurs->capaTabServeurs*sizeof(InfoServeurs*));

        for (i = serveurs->nbServeurs;i>0;i--)
        {
            serveurs->tabServeurs[i] = temp[i];
        }
    }

    /* On génère un nouvel idServeur */
    generateurIdServeur++;

    /* Création du nouveau serveur dans la BDD des serveurs */
    serveurs->tabServeurs[serveurs->nbServeurs] = malloc(sizeof(InfoServeurs));
    serveurs->tabServeurs[serveurs->nbServeurs]->adresseServeur = var_adresseServeur;
    serveurs->tabServeurs[serveurs->nbServeurs]->numPort = var_portServeur;
    serveurs->tabServeurs[serveurs->nbServeurs]->idServeur = generateurIdServeur;
    serveurs->nbServeurs++;

    /* On envoie un message au serveur pour lui donner son idServeur qui sera utilisé */
    /* Message de la forme : "21 adresseServeur portServeur idServeur" */
    strcpy(buff, "21 ");
    strcat(buff, var_adresseServeur);
    strcat(buff, " ");
    sprintf(str_portServeur, "%d", var_portServeur);
    strcat(buff, str_portServeur);
    strcat(buff, " ");
    sprintf(str_idServeur, "%u", generateurIdServeur);
    strcat(buff, str_idServeur);

    /* Envoi du message sur la socket */
    ecritureSocket(s,buff,200);

    /* Libérations mémoire de chaines de caractère */
    free(buff);
    free(var_adresseServeur);
    free(str_portServeur);
    free(str_idServeur);

    /* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_r);
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_r);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}


/**
* @note traitement d'un message de type demande d'ID fichier.
* @param s : la socket sur laquelle le message de demande d'ID fichier a été émis.
* @param mess  : le message de demande d'ID fichier à traiter.
* @warning incrémente le generateur d'idFichier.
*/
void traiteDemandeIdFichier(Socket s, char* mess)
{
    /* Variables */
    int type_message; /* type du message reçu : ici 55 */

    char* buff; /* tampon pour écriture du message */
    char* var_nomFichier; /* nom du fichier */
    char* str_idFichier; /* transformation de l'identificateur du serveur en chaine de caractères */

    Fichier* ptListeFichiers; /* pointeur de travail sur la liste des fichiers de la BDD */
    Fichier* ptRetListeFichiers; /* pointeur de travail sur la liste des fichiers de la BDD */

    /* Allocations mémoire des chaines de caractère */
    buff = malloc(200*sizeof(char));
    var_nomFichier = malloc(40*sizeof(char));
    str_idFichier = malloc(10*sizeof(char));

    /* On récupère le contenu du message */
    /* Doit être de la forme "55 nomFichier" */
    if (sscanf(mess, "%d %s", &type_message, var_nomFichier) < 2)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
    }

    /* On vérouille la BDD des fichiers en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock(&fichiers->verrou_bddfich_r);
    pthread_mutex_lock(&fichiers->verrou_bddfich_w);
    pthread_mutex_lock(&serveurs->verrou_bddserv_r);
    pthread_mutex_lock(&serveurs->verrou_bddserv_w);

    /* On référence le nouveau fichier dans la BDD des fichiers */
    /* On génère un nouvel idFichier */
    generateurIdFichier++;

    /* Création du nouveau fichier dans la BDD des fichiers */
    ptListeFichiers = fichiers->listeFichiers;
    ptRetListeFichiers = ptListeFichiers;

    /* On parcourre la liste des fichiers jusqu'au bout pour ajouter la nouvelle référence */
    while(ptListeFichiers != NULL)
    {
        ptRetListeFichiers = ptListeFichiers;
        ptListeFichiers = ptListeFichiers->fichierSuivant;
    }

    ptRetListeFichiers->fichierSuivant = malloc(sizeof(Fichier));
    ptRetListeFichiers->fichierSuivant->idFichier = generateurIdFichier;
    strcpy(ptRetListeFichiers->fichierSuivant->nomFichier, var_nomFichier);
    ptRetListeFichiers->fichierSuivant->nbBlocs = 0;
    ptRetListeFichiers->fichierSuivant->tabBlocs = NULL;
    ptRetListeFichiers->fichierSuivant->capaTabBlocs = 0;
    ptRetListeFichiers->fichierSuivant->fichierSuivant = NULL;


    /* On envoie un message au serveur pour lui donner son idServeur qui sera utilisé */
    /* Message de la forme : "22 nomFichier idFichier" */
    strcpy(buff, "22 ");
    strcat(buff, var_nomFichier);
    strcat(buff, " ");
    sprintf(str_idFichier, "%u", generateurIdFichier);
    strcat(buff, str_idFichier);

    /* Envoi du message sur la socket */
    ecritureSocket(s,buff,200);

    /* Libérations mémoire de chaines de caractère */
    free(buff);
    free(var_nomFichier);
    free(str_idFichier);

    /* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_r);
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_r);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}


/**
* @note traitement d'un message adressé au mauvais destinataire.
* @param s : la socket sur laquelle le message inattendu a été émis.
* @param mess : le message en question.
*/
void traiteMessageErr(Socket s, char* mess)
{
    ecritureSocket(s, "71 mauvais destinataire", 200);
}


/**
* @note procédure de fermeture de l'annuaire de façon propre.
* @brief modifie le contenu des variables globales serveurs et fichiers.
*/
void fermetureAnnuaire()
{
    /* Variables locales */
    int i,j;
    Fichier* temp_fichier;
    Serveur* temp_serveur;

    /* Destruction des listes de serveurs */
    for (j = serveurs->nbServeurs;j>0;j--)
    {
        free(serveurs->tabServeurs[j]->adresseServeur);
        free(serveurs->tabServeurs[j]);
    }
    free(serveurs->tabServeurs);

    pthread_mutex_destroy(&serveurs->verrou_bddserv_r);
    pthread_mutex_destroy(&serveurs->verrou_bddserv_w);

    free(serveurs);

    /* Destruction des listes de fichiers */
    while (fichiers->listeFichiers != NULL)
    {
        for (i = fichiers->listeFichiers->nbBlocs;i>0;i--)
        {
            while ((fichiers->listeFichiers->tabBlocs[i]).listeServeurs != NULL)
            {
                temp_serveur = (fichiers->listeFichiers->tabBlocs[i]).listeServeurs;
                (fichiers->listeFichiers->tabBlocs[i]).listeServeurs = temp_serveur->serveurSuivant;
                free(temp_serveur);
            }
        }
        free(fichiers->listeFichiers->tabBlocs);
        free(fichiers->listeFichiers->nomFichier);

        temp_fichier = fichiers->listeFichiers;
        fichiers->listeFichiers = temp_fichier->fichierSuivant;
        free(temp_fichier);
    }

    pthread_mutex_destroy(&fichiers->verrou_bddfich_r);
    pthread_mutex_destroy(&fichiers->verrou_bddfich_w);

    free(fichiers);
}


int main(void)
{
    /* Variables */
    int portEcouteAnnuaire; /* port d'écoute de l'annuaire */
    int nbthreads; /* nombres de threads clients gérés actuellement */

    Socket socketEcouteAnnuaire; /* Socket d'écoute de l'annuaire */
    Socket socketDemandeConnexion; /* Socket ouverte par un client */

    int quitter; /* Booléen pour savoir si on quitte ou pas */

    pthread_t th_client; /* Thread pour gerer les connexions clients */


    /* Initialisation de la graine pour utiliser le rand() */
    srand(time(NULL));

    /* Initialisation de l'annuaire */
    printf("Bienvenue sur le programme lif12p2p.\n");
    printf("Initialisation des bases de données de l'annuaire...\n");
    if (initialisationAnnuaire() < 0)
    {
        perror("Problème lors de l'initialisation des bases de données de l'annuaire. \n Fermeture du programme.\n");
        exit(1);
    }
    printf("Bases de données de l'annuaire initialisées.\n");

    /* Initialisation de la socket d'écoute de l'annuaire */
    printf("Sur quel port souhaitez vous lancer le serveur annuaire?\n");
    scanf("%d", &portEcouteAnnuaire);
    printf("Vous avez choisi le port numero %d.\n", portEcouteAnnuaire);

    printf("Initialisation de la socket d'écoute de l'annuaire...\n");
    socketEcouteAnnuaire = initialiseSocketEcouteAnnuaire(portEcouteAnnuaire);
    printf("Socket d'écoute de l'annuaire initialisée.\n");

    /* Menu */
    quitter = 0;
    nbthreads = 0;
    while (!quitter)
    {
        socketDemandeConnexion = acceptationConnexion(socketEcouteAnnuaire);

        if (pthread_create(&th_client, NULL, (void* (*)(void*))traiteMessage, (void*)socketDemandeConnexion ) == 0)
        {
            nbthreads++;
            printf("Il y a actuellement %d thread(s) de discussion ouvert(s).\n", nbthreads);
        }
        else
        {
            perror("Un thread de traitement de message n'a pas pu être créé. Message ignoré.\n");
        }
    }

    /* Fermeture de l'annuaire */
    printf("Destruction des bases de données de l'annuaire...\n");
    fermetureAnnuaire();
    printf("Destruction des bases de données de l'annuaire terminée.\n");

    return 0;
}

/*****************
* Fin de Fichier
*****************/

