/**
 * @file: annuaire.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 03/04/2009
 */

#include "annuaire.h"

/**
* Variables Globales
*/

BddServeurs* serveurs; /* pointeur sur la liste des serveurs connus */
BddFichiers* fichiers; /* pointeur sur la liste des fichiers connus */

/**
* Fonctions et procédures du module
*/

int initialisationAnnuaire(BddServeurs * serveurs, BddFichiers * fichiers)
/**
* @note: procédure d'initialisation de l'annuaire :  les listes de clients, de serveurs, de fichiers.
* @param: serveurs : pointeur sur la base de données des serveurs.
* @param: fichiers : pointeur sur la base de données des fichiers.
* @return: renvoie 0 si tout se passe bien, -1 sinon.
*/
{
    /* Creation des listes de serveurs */
    serveurs = malloc(sizeof(BddServeurs));
    if (serveurs == NULL)
    {
        perror("Echec de l'allocation en mémoire de la base de donnée des serveurs.\n");
        return -1;
    }
    serveurs->nbServeurs=0;
    serveurs->capaTabServeurs=1;
    serveurs->tabServeurs=malloc(serveurs->capaTabServeurs*sizeof(InfoServeurs*));
    pthread_mutex_init(&serveurs->verrou_bddserv_r, NULL);
    pthread_mutex_init(&serveurs->verrou_bddserv_w, NULL);

    /* Creation des listes de fichiers */
    fichiers = malloc(sizeof(BddFichiers));
    if (fichiers == NULL)
    {
        perror("Echec de l'allocation en mémoire de la base de donnée des fichiers.\n");
        return -1;
    }
    fichiers->nbFichiers=0;
    fichiers->listeFichiers=NULL;
    pthread_mutex_init(&fichiers->verrou_bddfich_r, NULL);
    pthread_mutex_init(&fichiers->verrou_bddfich_w, NULL);

    return 0;
}


Socket initialiseSocketEcouteAnnuaire(int portAnnuaire)
/**
* @note: fonction d'initialisation de la socket d'écoute de l'annuaire.
* @param: portAnnuaire : numéro de port sur lequel on crée la socket d'écoute.
* @return: renvoie la socket créée.
*/
{
    /* On déclare la socket pour l'annuaire */
    Socket socketEcouteAnnuaire;

    /* On la crée */
    socketEcouteAnnuaire=creationSocket();

    /* On lie la socket au numero de port portAnnuaire */
    definitionNomSocket(socketEcouteAnnuaire,portAnnuaire);

    return socketEcouteAnnuaire;
}


int traiteMessage(Socket arg)
/**
* @note: fonction globale de traitement d'un message reçu.
* @param: arg : socket sur laquelle le message est arrivé.
* @return: renvoir 0 si tout se passe bien, -1 sinon.
*/
{
    /* variables */
    char* buff; /* Message lu sur la socket */
    int type_message; /* type de message */
    int fin_thread; /* booléen qui détermine si le thread doit se finir ou pas */

    /* boucle de traitement des messages */
    buff=malloc(100*sizeof(char)); /* */

    fin_thread=1; /* booleen qui décide de l'arret du thread */

    while (!fin_thread)
    {
        /* on ecoute sur la socket arg */
        ecouteSocket(arg, buff);

        /* on analyse le type du message reçu et on agit en conséquence */
        if (sscanf ( buff, "%d", &type_message) != 1 )
        {
            fprintf(stderr, "Message ignoré, impossible de l'analyser.\n Contenu du message: %s \n", buff);
        }
        else
        {
            /* On lance l'action correspondant au type de message. */
            switch (type_message)
            {
            case 3: /* Demande d'un fichier */
                traiteDemandeFichierClient(arg, buff);
                break;
            case 4: /* Demande d'un bloc */
                traiteDemandeBlocClient(arg, buff);
                break;
            case 5: /* Arret d'échange d'un client */
                traiteArretClient(arg, buff);
                fin_thread=0;
                break;
            case 8: /* Disponibilité d'un bloc */
                traiteBlocDisponibleServeur(arg, buff);
                break;
            case 9: /* Arret d'un serveur */
                traiteArretServeur(arg, buff);
                fin_thread=0;
                break;
            case 13: /* Indiquation que l'on a envoyé des messages au mauvais destinataire sur la socket donc fermeture */
                fin_thread=0;
                break;
            default: /* Un message géré par le réseau a bien été reçu mais inadapté donc la connexion
                            doit être terminé car ce ne sont pas les bons interlocuteurs. */
                traiteMessageErr(arg, buff);
                fin_thread=0;
            }
        }
    }

    /* Fermeture de la socket arg */
    clotureSocket(arg);

    return 0;

}


void traiteDemandeFichierClient(Socket s, char* mess)
/**
* @note: traitement d'un message de type demande de fichier d'un client.
* @param: s : la socket sur laquelle la demande de fichier client a été émise.
* @param: mess : la demande de fichier client a traiter.
*/
{
/* Variables */
    int fichTrouve; /* Booléen indiquant si le fichier a été trouvé dans la BDD des fichiers */
    int type_message; /* type du message : ici 3 */
    int i,j; /* itérateur */

    char* buff; /* tampon pour écriture du message */
    char* var_fichier; /* fichier forcément */
    char* var_nomDeFichier; /* nom du fichier demandé */
    char* str_idFichier; /* transformation de l'identificateur du fichier en chaine de caractères */
    char* str_nbBlocs; /* transformation du nombre de blocs du fichier en chaine de caractères */
    char* str_numBloc; /* transformation du numéro du bloc du fichier en chaine de caractères */
    char* str_idServeur; /* transformation de l'identificateur de serveur en chaine de caractères */
    char* str_numPort; /* transformation du numéro de port du serveur en chaine de caractères */

    Fichier* ptBddFichiers; /* pointeur de travail pour se déplacer dans la BDD des fichiers */
    Serveur* ptListeServeurs; /* pointeur de travail pour se déplacer dans la liste des serveurs d'un bloc */

/* On initialise le booleen de réussite :p */
    fichTrouve=1; /* On a pas encore trouvé le fichier */

/* On récupère le contenu du message */
/* Doit être de la forme "3 fichier nomDeFichier" */
    if (sscanf(mess, "%d %s %s", &type_message, var_fichier, var_nomDeFichier ) < 3)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
    }

/* On vérrouille en ecriture les BDD des fichiers et serveurs avant la lecture */
    pthread_mutex_lock(&fichiers->verrou_bddfich_w);
    pthread_mutex_lock(&serveurs->verrou_bddserv_w);

/* On recherche ans la BDD des fichiers si on possède celui qui est demandé */
    ptBddFichiers=fichiers->listeFichiers;

    while( ptBddFichiers != NULL)
    {
/******** Le fichier demandé est trouvé ********/
        if( strcmp( fichiers->listeFichiers->nomFichier, var_nomDeFichier ) == 0 )
        {
/* Pour chaque bloc constituant du fichier on envoie un message indiquant où le télécharger au client */
            buff=malloc(200*sizeof(char));

            for(i=ptBddFichiers->nbBlocs;i>0;i--)
            {

/* On envoie pour chaque bloc un message indiquant au client où télécharger chaque bloc */
/* Message de la forme : "1 bloc idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur" */
                strcpy(buff, "1 bloc ");
                sprintf(str_idFichier, "%d", ptBddFichiers->idFichier);
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

                ptListeServeurs=ptBddFichiers->tabBlocs[i].listeServeurs;

/* On a des serveurs proposant ce bloc */
                if(ptListeServeurs != NULL)
                {
/* On fait un rand parmi les serveurs pour répartir leur charge en attributan les requetes aléatoirement */
                    for(j=rand()%ptBddFichiers->tabBlocs[i].nbServeursDansListe;j>=0;j--)
                    {
                        ptListeServeurs=ptListeServeurs->serveurSuivant;
                    }

                    sprintf(str_idServeur, "%d", serveurs->tabServeurs[ptListeServeurs->numServeur]->idServeur);
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
                ecritureSocket(s,buff);
            }
/* Libération de la chaine après utilisation */
            free(buff);
/* On a bien trouvé le fichier et envoyé le message */
            fichTrouve=0;
        }
/******** Le fichier n'a pas encore été trouvé, continue la recherche ********/
        else
        {
            ptBddFichiers = ptBddFichiers->fichierSuivant;
        }
    }
/******** Le fichier n'est pas dans la BDD des fichiers, on envoie une réponse défavorable au client *********/
    if(!fichTrouve)
    {
/* On envoie un message de réponse défavorable au client */
/* Message de la forme: "02 erreur nomDeFichier" */
        buff=malloc(200*sizeof(char));

        strcpy(buff, "2 erreur ");
        strcat(buff, var_nomDeFichier);

/* Envoi du message sur la socket */
        ecritureSocket(s,buff);

/* Libération de la chaine après utilisation */
        free(buff);
    }

/* On dévérouille en écriture les BDD des fichiers et serveurs après la lecture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}


void traiteDemandeBlocClient(Socket s, char* mess)
/**
* @note: traitement d'un message de type demande de bloc d'un client.
* @param: s : la socket sur laquelle la demande de bloc client a été émise.
* @param: mess : la demande de bloc client a traiter.
*/
{
/* Variables */
    int type_message; /* type du message : ici 4 */
    int var_idFichier; /* identificateur du fichier */
    int var_numBloc; /* numero du bloc demandé */
    int j; /* Itérateur */
    int fichTrouve; /* Booléen indiquant si le fichier a été trouvé dans la BDD des fichiers */

    char* buff; /* tampon pour écriture du message */
    char* var_bloc; /* bloc */
    char* var_nomDeFichier; /* nom du fichier auquel appartient le bloc */
    char* str_idFichier; /* transformation de l'identificateur du fichier en chaine de caractères */
    char* str_nbBlocs; /* transformation du nombre de blocs du fichier en chaine de caractères */
    char* str_numBloc; /* transformation du numéro du bloc du fichier en chaine de caractères */
    char* str_idServeur; /* transformation de l'identificateur de serveur en chaine de caractères */
    char* str_numPort; /* transformation du numéro de port du serveur en chaine de caractères */

    Fichier* ptBddFichiers; /* pointeur de travail pour se déplacer dans la BDD des fichiers */
    Serveur* ptListeServeurs; /* pointeur de travail pour se déplacer dans la liste des serveurs d'un bloc */

/* On récupère le contenu du message */
/* Doit être de la forme "4 bloc idFichier nomDeFichier numeroDeBloc" */
    if (sscanf(mess, "%d %s %d %s %d", &type_message, var_bloc, &var_idFichier, var_nomDeFichier, &var_numBloc ) < 5)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
        exit(1);
    }

/* On vérrouille en ecriture les BDD des fichiers et serveurs avant la lecture */
    pthread_mutex_lock(&fichiers->verrou_bddfich_w);
    pthread_mutex_lock(&serveurs->verrou_bddserv_w);

/* On recherche ans la BDD des fichiers si on possède celui qui est demandé */
    ptBddFichiers=fichiers->listeFichiers;

    while( ptBddFichiers != NULL)
    {
/******** Le fichier demandé est trouvé ********/
        if( strcmp( fichiers->listeFichiers->nomFichier, var_nomDeFichier ) == 0 && var_idFichier == fichiers->listeFichiers->idFichier)
        {
/* On envoie un message indiquant où télécharger le bloc demandé au client */
            buff=malloc(200*sizeof(char));
/* On envoie pour chaque bloc un message indiquant au client où télécharger chaque bloc */
/* Message de la forme : "1 bloc idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur" */
            strcpy(buff, "1 bloc ");
            sprintf(str_idFichier, "%d", ptBddFichiers->idFichier);
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

            ptListeServeurs=ptBddFichiers->tabBlocs[var_numBloc].listeServeurs;

/* On a des serveurs proposant ce bloc */
            if(ptListeServeurs != NULL)
            {
/* On fait un rand parmi les serveurs pour répartir leur charge en attribuant les requetes aléatoirement */
                for(j=rand()%ptBddFichiers->tabBlocs[var_numBloc].nbServeursDansListe;j>=0;j--)
                {
                    ptListeServeurs=ptListeServeurs->serveurSuivant;
                }

                sprintf(str_idServeur, "%d", serveurs->tabServeurs[ptListeServeurs->numServeur]->idServeur);
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
            ecritureSocket(s,buff);
/* Libération de la chaine après utilisation */
            free(buff);
/* On a bien trouvé le fichier et envoyé le message */
            fichTrouve=0;
        }
/******** Le fichier n'a pas encore été trouvé, continue la recherche ********/
        else
        {
            ptBddFichiers = ptBddFichiers->fichierSuivant;
        }
    }
/******** Le fichier n'est pas dans la BDD des fichiers, on envoie une réponse défavorable au client *********/
    if(!fichTrouve)
    {
/* On envoie un message de réponse défavorable au client */
/* Message de la forme: "02 erreur nomDeFichier" */
        buff=malloc(200*sizeof(char));

        strcpy(buff, "2 erreur ");
        strcat(buff, var_nomDeFichier);

/* Envoi du message sur la socket */
        ecritureSocket(s,buff);

/* Libération de la chaine après utilisation */
        free(buff);
    }

/* On dévérouille en écriture les BDD des fichiers et serveurs après la lecture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}


void traiteArretClient(Socket s, char* mess)
/**
* @note: traitement d'un message de type arret d'échange client.
* @param: s : la socket sur laquelle le message d'arret client a été émis.
* @param: mess : le message d'arret client a traiter.
*/
{

}


void traiteBlocDisponibleServeur(Socket s, char* mess)
/**
* @note: traitement d'un message de type nouveau bloc disponible sur serveur.
* @param: s : la socket sur laquelle le message de nouveau bloc disponible sur serveur a été émis.
* @param: mess : le message de nouveau bloc disponible a traiter.
*/
{
/* Variables */
    int type_message; /* Type du message : ici 8 */
    int var_idFichier; /* identificateur du fichier */
    int var_nbBlocs; /*  nombre de blocs du fichier */
    int var_numBloc; /* numero de bloc */
    int var_idServeur; /* identificateur du serveur */
    int var_portServeur; /* port du serveur */
    int i; /* Itérateur */

    int fichTrouve; /* booléen */
    int servTrouve; /* booléen */

    char* var_bloc; /* bloc */
    char* var_nomDeFichier; /* nom du fichier */
    char* var_adresseServeur; /* adresse du serveur */

    Fichier* ptListeFichiers; /* pointeur de travail sur une liste de fichiers */
    Serveur* ptListeServeurs; /* pointeur de travail sur une liste de serveurs */
    InfoServeurs** temp; /* pointeur de travail sur un tableau de pointeurs sur InfoServeurs */

/* Initialisation des booléens */
    fichTrouve = 1;
    servTrouve = 1;

/* On récupère le contenu du message */
/* Doit être de la forme "8 bloc idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur" */
    if (sscanf(mess, "%d %s %d %s %d %d %d %s %d", &type_message, var_bloc, &var_idFichier, var_nomDeFichier, &var_nbBlocs, &var_numBloc, &var_idServeur, var_adresseServeur, &var_portServeur ) < 7)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
    }

/* On vérouille les BDD des fichiers et serveurs en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock(&fichiers->verrou_bddfich_r);
    pthread_mutex_lock(&fichiers->verrou_bddfich_w);
    pthread_mutex_lock(&serveurs->verrou_bddserv_r);
    pthread_mutex_lock(&serveurs->verrou_bddserv_w);

/* Mise à jour de la BDD des fichiers avec les nouvelles informations */
    ptListeFichiers=fichiers->listeFichiers;

/* On parcourre la liste des fichiers du début et on ajoute le nouveau fichier à la fin si on ne l'a pas trouvé dans la liste */
    while(ptListeFichiers != NULL && fichTrouve) /* Tant qu'on est pas au bout de la liste ou qu'on a pas trouvé le fichier */
    {
        if( strcmp(ptListeFichiers->nomFichier, var_nomDeFichier) == 0 && ptListeFichiers->idFichier == var_idFichier) /* Si on trouve le fichier déjà référencé */
        {
            fichTrouve = 0;
            ptListeServeurs=ptListeFichiers->tabBlocs[var_numBloc].listeServeurs;

            while(ptListeServeurs != NULL && servTrouve) /* Tant qu'on arrive pas au bout de la liste des serveurs ou qu'on trouve le serveur déjà référencé */
            {
                if(serveurs->tabServeurs[ptListeServeurs->numServeur]->idServeur == var_idServeur) /* Si on trouve le serveur pas besoin de le rajouter */
                {
                    servTrouve = 0;
                }
                else  /* Sinon on continue */
                {
                    ptListeServeurs=ptListeServeurs->serveurSuivant;
                }
            }

            if(!servTrouve) /* Si on a pas trouvé le serveur on le rajoute en fin de liste et on crée son entité dans la BDD des serveurs */
            {
                ptListeServeurs=malloc(sizeof(Serveur));
                ptListeServeurs->serveurSuivant=NULL;

                /* Création de son entité dans la BDD des serveurs */
                if(serveurs->capaTabServeurs <= serveurs->nbServeurs) /* Si il n y a pas assez de place dans la tableau dynamique, on l'agrandit */
                {
                    temp=serveurs->tabServeurs;
                    serveurs->tabServeurs=malloc(2*serveurs->capaTabServeurs*sizeof(InfoServeurs*));

                    for(i=serveurs->nbServeurs;i>0;i--)
                    {
                        serveurs->tabServeurs[i]=temp[i];
                    }
                }

                serveurs->tabServeurs[serveurs->nbServeurs]=malloc(sizeof(InfoServeurs*));
                serveurs->tabServeurs[serveurs->nbServeurs]->adresseServeur = var_adresseServeur;
                serveurs->tabServeurs[serveurs->nbServeurs]->idServeur = var_idServeur;
                serveurs->tabServeurs[serveurs->nbServeurs]->numPort = var_portServeur;

                serveurs->nbServeurs++;

            }
        }
        else /* Sinon on continue */
        {
            ptListeFichiers=ptListeFichiers->fichierSuivant;
        }
    }
    if(!fichTrouve) /* Si on ne trouve pas le fichier dans la BDD on le référence à la fin */
    {
        ptListeFichiers=malloc(sizeof(Fichier));
        ptListeFichiers->nomFichier=var_nomDeFichier;
        ptListeFichiers->tabBlocs=malloc(sizeof(Serveur));
        ptListeFichiers->nbBlocs++;
        fichiers->nbFichiers++;
    }


/* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_r);
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_r);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}

void traiteArretServeur(Socket s, char* mess)
/**
* @note: traitement d'un message de type arrêt de serveur.
* @param: s : la socket sur laquelle le message d'arret de serveur a été emis.
* @param: mess : le message d'arret serveur a traiter.
*/
{
/* Variables */
    int type_message; /* type du message reçu : ici 9 */
    int var_idServeur; /* identificateur du serveur */
    int var_portServeur; /* port du serveur */

    int i; /* itérateur */
    int compt_nbServ; /* compteur de travail */

    int sourceExiste; /* booléen */

    char* var_arret; /* "arret" */
    char* var_adresseServeur; /* adresse du serveur */

    Fichier* ptListeFichiers; /* pointeur de travail sur la liste des fichiers de la BDD */
    Serveur* ptListeServeurs; /* pointeur de travail sur la liste des serveurs de la BDD */
    Serveur* ptTempServeur; /* pointeur de travail sur un Serveur */
    Fichier* ptTempFichier; /* pointeur de travail sur un Fichier */

/* Initialisation des booléens */
    sourceExiste = 0;

/* On récupère le contenu du message */
/* Doit être de la forme "9 arret idServeur adresseServeur portServeur" */
    if (sscanf(mess, "%d %s %d %s %d", &type_message, var_arret, &var_idServeur, var_adresseServeur, &var_portServeur) < 5)
    {
        fprintf(stderr, "Message invalide, impossible de l'utiliser.\n Contenu du message: %s \n", mess);
    }

/* On vérouille la BDD des fichiers en lecture et en écriture avant l'écriture des nouvelles données */
    pthread_mutex_lock(&fichiers->verrou_bddfich_r);
    pthread_mutex_lock(&fichiers->verrou_bddfich_w);
    pthread_mutex_lock(&serveurs->verrou_bddserv_r);
    pthread_mutex_lock(&serveurs->verrou_bddserv_w);

/* On supprime de la BDD des fichiers les informations relatives au serveur qui s'arrete */
    ptListeFichiers=fichiers->listeFichiers;
    ptTempFichier=ptListeFichiers;

    while(ptListeFichiers != NULL) /* Tant que l'on a pas parcouru toute la liste des fichiers */
    {
        for(i=ptListeFichiers->nbBlocs;i>0;i--) /* Pour chaque bloc du fichier */
        {
            ptListeServeurs=ptListeFichiers->tabBlocs[i].listeServeurs;
            compt_nbServ=0;

            while(serveurs->tabServeurs[ptListeServeurs->numServeur]->idServeur != var_idServeur || ptListeServeurs !=NULL) /* Tant que l'on a pas parcouru toute la liste des serveurs ou trouvé le serveur */
            {
                compt_nbServ++;
                if( compt_nbServ == 1 && ptListeServeurs->serveurSuivant == NULL ) /* Si le serveur que l'on doit enlever est le seul de la liste */
                {
                    ptListeFichiers->tabBlocs[i].listeServeurs = NULL;
                }
                else /* Il restera au moins un serveur dans la liste donc inutile de mettre NULL */
                {
                    ptTempServeur = ptListeServeurs; /* /!\ CHECK THAT ! */
                    ptListeServeurs = ptTempServeur->serveurSuivant; /* /!\ CHECK THAT ! */
                    free(ptTempServeur); /* /!\ CHECK THAT ! */
                }

                ptListeServeurs=ptListeServeurs->serveurSuivant;
            }
        }

        for(i=ptListeFichiers->nbBlocs;i>0;i--) /* On vérifie qu'il reste encore au moins un bloc disponible pour le fichier */
        {
            if(ptListeFichiers->tabBlocs[i].listeServeurs == NULL)
            {
                sourceExiste = 1; /* Plus aucun bloc n'est disponible pour le fichier */
                break;
            }
        }

        if(!sourceExiste) /* Auquel cas on le supprime de la BDD des fichiers */
        {
            ptTempFichier = ptListeFichiers; /* /!\ CHECK THAT ! */
            ptListeFichiers = ptTempFichier->fichierSuivant; /* /!\ CHECK THAT ! */
            free(ptTempFichier);  /* /!\ CHECK THAT ! */
        }

        ptListeFichiers=ptListeFichiers->fichierSuivant;
    }

/* On peut maintenant supprimer de la BDD des serveurs le serveur qui s'arrete */
    for(i=serveurs->nbServeurs;i>0;i--)
    {

    }

/* On dévérouille la BDD des fichiers en lecture et en écriture */
    pthread_mutex_unlock(&fichiers->verrou_bddfich_r);
    pthread_mutex_unlock(&fichiers->verrou_bddfich_w);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_r);
    pthread_mutex_unlock(&serveurs->verrou_bddserv_w);
}


void traiteMessageErr(Socket s, char* mess)
/**
* @note: traitement d'un message adressé au mauvais destinataire.
* @param: s : la socket sur laquelle le message inattendu a été émis.
* @param: mess : le message en question.
*/
{
    ecritureSocket(s, "13 erreur mauvais destinataire");
}


void fermetureAnnuaire(BddServeurs * serveurs, BddFichiers * fichiers)
/**
* @note: procédure de fermeture de l'annuaire de façon propre.
* @param: serveurs : pointeur sur la base de données des serveurs.
* @param: fichiers : pointeur sur la base de données des fichiers.
*/
{
/* Variables locales */
    int i,j;
    Fichier* temp_fichier;
    Serveur* temp_serveur;

/* Destruction des listes de serveurs */
    for (j=serveurs->nbServeurs;j>0;j--)
    {
        free(serveurs->tabServeurs[j]->adresseServeur);
        free(serveurs->tabServeurs[j]);
    }
    free(serveurs->tabServeurs);

    pthread_mutex_destroy(&serveurs->verrou_bddserv_r);
    pthread_mutex_destroy(&serveurs->verrou_bddserv_w);

    free(serveurs);

/* Destruction des listes de fichiers */
    while (fichiers->listeFichiers!=NULL)
    {
        for (i=fichiers->listeFichiers->nbBlocs;i>0;i--)
        {
            while ((fichiers->listeFichiers->tabBlocs[i]).listeServeurs!=NULL)
            {
                temp_serveur=(fichiers->listeFichiers->tabBlocs[i]).listeServeurs;
                (fichiers->listeFichiers->tabBlocs[i]).listeServeurs=temp_serveur->serveurSuivant;
                free(temp_serveur);
            }
        }
        free(fichiers->listeFichiers->tabBlocs);
        free(fichiers->listeFichiers->nomFichier);

        temp_fichier=fichiers->listeFichiers;
        fichiers->listeFichiers=temp_fichier->fichierSuivant;
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
    int numport_ok; /* booleen pour l'entrée du port annuaire */

    char numport_key; /* confirmation de réponse pour le port annuaire */

    pthread_t* th_client; /* tableau de threads pour gerer les connexions clients */


/* Initialisation de la graine pour utiliser le rand() */
    srand(time(NULL));

/* Initialisation de l'annuaire */
    printf("Bienvenue sur le programme lif12p2p.\n");
    printf("Initialisation des bases de données de l'annuaire.\n");
    if (initialisationAnnuaire(serveurs, fichiers)<0)
    {
        perror("Problème lors de l'initialisation des bases de données de l'annuaire. \n Fermeture du programme.\n");
        exit(1);
    }

/* Initialisation de la socket d'écoute de l'annuaire */

    do
    {
        /* NE FONCTIONNE PAS : A VERIFIER */
        numport_ok=1;

        printf("Sur quel port souhaitez vous lancer le serveur annuaire?\n");
        scanf("%d",&portEcouteAnnuaire);
        printf("Vous avez choisi le port numero %d.\n",portEcouteAnnuaire);
        printf("Si ce choix vous convient appuyez sur 'Y' sinon appuyez sur n'importe quelle touche pour changer.\n");
        scanf("%c",&numport_key);
        printf("%c",numport_key);
        if (numport_key=='y' || numport_key=='Y')
            numport_ok=0;

    }
    while (numport_ok);

    socketEcouteAnnuaire=initialiseSocketEcouteAnnuaire(portEcouteAnnuaire);

/* Menu */
    quitter=1;
    nbthreads=0;
    while (quitter)
    {
        socketDemandeConnexion=acceptationConnexion(socketEcouteAnnuaire);

        pthread_create(&th_client[nbthreads+1], NULL, (void* (*)(void*))traiteMessage , (void*)socketDemandeConnexion );
        nbthreads++;
    }

/* Fermeture de l'annuaire */
    fermetureAnnuaire(serveurs, fichiers);


    return 0;
}

/*****************
* Fin de Fichier
*****************/
