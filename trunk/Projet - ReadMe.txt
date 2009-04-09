AUDUON Rémi													dernière mise à jour : 05 avril 2009
BONNET-JAQUEMET Thibault												version 2.01
GUILLON Benjamin

					LIF 12 : Projet d'application peer-to-peer : GROUPE 8

1] Analyse du projet
    a) Annuaire
    b) Client/serveur
2] Structure de programmation
    a) Format des messages qui transitent sur le réseau
    b) Annuaire
    c) Client/serveur
3] Programmation système
    a) Socket
    b) Thread
    c) Processus


*********************************************************************************************

1] Analyse du projet

Découpage du projet en 2 applications : une pour l'annuaire, et une pour le côté client/serveur.
a) Annuaire 
	Centre névralgique du réseau
	_ Attend les demandes de fichiers des clients
		-> réponse favorable : dit au client où et quoi télécharger
		-> réponse défavorable : rejète la demande (le fichier demandé n'est pas sur le réseau)
	_ Traite les informations données par les serveurs
		-> Quels fichiers sont disponibles sur le réseau, et où
		-> Quelle est la charge des serveurs

b) Client / serveur
	_ Client
		-> Requete l'annuaire pour les recherches de fichier
		-> Requete les serveurs pour télécharger des blocs de fichier
	_ Serveur
		-> Répond aux requetes des clients
		-> Envoie des informations sur les fichiers a l'annuaire et sur sa charge
		-> Gère le découpage des fichiers

*********************************************************************************************

2] Structure de programmation
a) Format des messages qui transitent sur le réseau
	-> de l'annuaire vers le client
		* Réponse favorable (pour chaque bloc) : 	11 idFichier nomDeFichier nombreTotalDeBloc numeroDeBloc idServeur adresseServeur portServeur
		* Réponse défavorable : 					12 nomDeFichier
		* message de fin de transmission :			13 fin communication
	-> de l'annuaire vers le serveur
		* réponse avec un ID serveur				21 adresseServeur portServeur idServeur
		* réponse avec un ID fichier				22 nomFichier idFichier
	-> du client vers l'annuaire
		* Demande d'un fichier : 					31 nomDeFichier
		* Demande d'un bloc : 					32 idFichier nomDeFichier numeroDeBloc
		* Indication de fin de requête client :		33 adresseClient
	-> du client vers le serveur
		* Demande d'un bloc : 					41 idFichier nomDeFichier numeroDeBloc
		* Déconnexion du client :					42 adresseClient
	-> du serveur vers l'annuaire
		* Disponibilité pour chaque bloc :			51 idFichier nombreTotalDeBloc numeroDeBloc idServeur
		* Indication d'arrêt du serveur :			52 idServeur
		* Indication de charge serveur :			53 (-) 1
		* demande ID serveur					54 adresseServeur portServeur
		* demande ID fichier					55 nomFichier
	-> du serveur vers le client
		* Envoi d'un bloc :						61 idFichier numeroDeBloc tailleBloc
											contenuDuBloc
		* Bloc introuvable :						62 idFichier numeroDeBloc
		* Déconnexion du serveur :				63 idServeur adresseServeur portServeur
	-> entre n'importe quelle source et destinataire
		* Réponse à un message inconnu :			71 mauvais destinataire


b) Annuaire
	_ Structure des données utilisées :
		-> Stockage de la liste des fichiers :
			* Les fichiers sont découpés en "Bloc"
			* On stocke, pour chaque bloc, la liste des serveurs sur lesquelles il est disponible (sous forme de liste chainée)
			* Tous les blocs d'un fichier sont stocké dans un tableau de "Bloc"
			* Les fichiers sont stockés dans une liste (chainée) de "Fichier"
		-> Stockage de la liste des clients :
		-> Stockage de la liste des serveurs : 
			* Pour chaque serveur, on stocke les données lui correspondant dans une structure à 2 champs : Adresse, et Port
			* L'application garde en mémoire un tableau (dynamique) de pointeur sur "Serveur"

	_ Déroulement du code :
		-> Initialisation de l'annuaire pour la liste des fichiers, des serveurs et des clients (à partir d'un fichier, ou mise à zéro)
		-> Initialisation de la socket d'écoute de l'annuaire
		-> écoute et analyse des messages reçus
		-> traitement et réponse aux messages
		-> il faut que 

c) Client/serveur 
principe de fonctionnement :
	L'application client/serveur est découpé en 2 parties ; une partie cliente, et une partie serveur :
		_ La partie serveur gère la mise à disposition de fichier sur le réseau (en le notifiant à l'annuaire). 
			Ensuite, la partie serveur s'occupe de répondre au demande client pour les fichiers mis à disposition. 
		_ La partie cliente gère la demande de fichier à l'annuaire, et les demande ensuite au serveur approprié.

	_ Partie serveur :
		-> Structure des données utilisées :
			* Liste d'attente des clients qui demandent un transfert :
				- On stocke pour chaque client les données suivantes : la socket créée lors de sa connexion, le nom du fichier et le numéro de bloc demandés
				- Les différents clients sont stockés dans une file d'attente
	
	_ Partie client :
		-> Structure des données utilisées :
			* Liste d'attente des blocs en attente de téléchargement :
				- On stocke pour chaque bloc à télécharger les données suivantes : l'adresse et le port du serveur, ainsi que le nom du fichier et le numéro du bloc à demander
				- Les différents blocs sont stockés dans une file d'attente
			* Liste des fichiers demandés :
				- On stocke pour chaque fichier, demandé à l'annuaire, le nom du fichier, le nombre total de bloc
				- De plus, pour chaque bloc, on stocke le statut du téléchargement : "Pas traité" / "Traité"
				- Les différents fichiers sont stockés dans une liste chainée
				
	_ Déroulement du code :
	
	
	


*********************************************************************************************

3] Programmation système
a) Socket


b) Thread


c) Processus







