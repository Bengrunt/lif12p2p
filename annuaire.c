/**
 * @file: annuaire.c
 * @project: lif12p2p
 * @author: Rémi AUDUON, Thibault BONNET-JACQUEMET, Benjamin GUILLON
 * @since: 20/03/2009
 * @version: 24/03/2009
 */

#include "annuaire.h"

/**
* Fonctions et procédures du module
*/

int initialisationAnnuaire(BddServeurs * serveurs, BddFichiers * fichiers)
/**
* @note: procédure d'initialisation de l'annuaire :  les listes de clients, de serveurs, de fichiers.
* @param: serveurs : pointeur sur la base de données des serveurs.
* @param: fichiers : pointeur sur la base de données des fichiers.
*/
{
/* Creation des listes de serveurs */
    serveurs = malloc(sizeof(BddServeurs));
    if(serveurs == NULL)
    {
        perror("Echec de l'allocation en mémoire de la base de donnée des serveurs.\n");
        return -1;
    }
/* Creation des listes de fichiers */
    fichiers = malloc(sizeof(BddFichiers));
    if(serveurs == NULL)
    {
        perror("Echec de l'allocation en mémoire de la base de donnée des fichiers.\n");
        return -1;
    }

    return 0;
}


Socket initialiseSocketEcouteAnnuaire(int portAnnuaire)
/**
* @note: fonction d'initialisation de la socket d'écoute de l'annuaire.
* @param: portAnnuaire : numéro de port sur lequel on crée la socket d'écoute.
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
    for(j=serveurs->nbServeurs;j>0;j--)
    {
        free(serveurs->tabServeurs[j]->adresseServeur);
        free(serveurs->tabServeurs[j]);
    }
    free(serveurs->tabServeurs);
    free(serveurs);
/* Destruction des listes de fichiers */
    while(fichiers->listeFichiers!=NULL)
    {
        for(i=fichiers->listeFichiers->nbBlocs;i>0;i--)
        {
            while((fichiers->listeFichiers->tabBlocs[i]).listeServeurs!=NULL)
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
    free(fichiers);
}


int main(void)
{
/* Variables */
    int portEcouteAnnuaire; /* port d'écoute de l'annuaire */
    int quitter; /* Booléen pour savoir si on quitte ou pas */
    BddServeurs* serveurs; /* pointeur sur la liste des serveurs connus */
    BddFichiers* fichiers; /* pointeur sur la liste des fichiers connus */
    int numport_ok; /* booleen pour l'entrée du port annuaire */
    char numport_key; /* confirmation de réponse pour le port annuaire */

/* Initialisation de l'annuaire */
    printf("Bienvenue sur le programme lif12p2p.\n");
    printf("Initialisation des bases de données de l'annuaire.\n");
    if(initialisationAnnuaire(serveurs, fichiers)<0)
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
        if(numport_key=='y' || numport_key=='Y')
            numport_ok=0;

    }while(numport_ok);

    initialiseSocketEcouteAnnuaire(portEcouteAnnuaire);

/* Menu */
    quitter=1;
    while(quitter)
    {

    }

/* Fermeture de l'annuaire */
    fermetureAnnuaire(serveurs, fichiers);


    return 0;
}

/*****************
* Fin de Fichier
*****************/

