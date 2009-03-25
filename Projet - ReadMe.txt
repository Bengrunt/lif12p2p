AUDUON R�mi													derni�re mise � jour : 18 mars 2009
BONNET-JAQUEMET Thibault												version 2.01
GUILLON Benjamin

					LIF 12 : Projet d'application peer-to-peer : GROUPE 8

1] Analyse du projet
    a) Annuaire
    b) Client/serveur
2] Structure de programmation
    a) Format des messages qui transitent sur le r�seau
    b) Annuaire
    c) Client/serveur
3] Programmation syst�me
    a) Socket
    b) Thread
    c) Processus


*********************************************************************************************

1] Analyse du projet

D�coupage du projet en 2 applications : une pour l'annuaire, et une pour le c�t� client/serveur.
a) Annuaire 
	Centre n�vralgique du r�seau
	_ Attend les demandes de fichiers des clients
		-> r�ponse favorable : dit au client o� et quoi t�l�charger
		-> r�ponse d�favorable : rej�te la demande (le fichier demand� n'est pas sur le r�seau)
	_ Traite les informations donn�es par les serveurs
		-> Quels fichiers sont disponibles sur le r�seau, et o�
		-> Quelle est la charge des serveurs

b) Client / serveur
	_ Client
		-> Requete l'annuaire pour les recherches de fichier
		-> Requete les serveurs pour t�l�charger des blocs de fichier
	_ Serveur
		-> R�pond aux requetes des clients
		-> Envoie des informations sur les fichiers a l'annuaire et sur sa charge
		-> G�re le d�coupage des fichiers

*********************************************************************************************

2] Structure de programmation
a) Format des messages qui transitent sur le r�seau
	-> de l'annuaire vers le client
		* R�ponse favorable (pour chaque bloc) : 	01 bloc nomDeFichier nombreTotalDeBloc numeroDeBloc adresseServeur portServeur
		* R�ponse d�favorable : 					02 erreur nomDeFichier
	-> de l'annuaire vers le serveur
		* aucun message
	-> du client vers l'annuaire
		* Demande d'un fichier : 					03 fichier nomDeFichier
		* Demande d'un bloc : 					04 bloc nomDeFichier numeroDeBloc
		* Indication de fin de requ�te client :		05 arret adresseClient portClient 
	-> du client vers le serveur
		* Demande d'un bloc : 					06 bloc nomDeFichier numeroDeBloc
	-> du serveur vers l'annuaire
		* Disponibilit� pour chaque bloc :			07 bloc nomDeFichier nombreTotalDeBloc numeroDeBloc adresseServeur portServeur
		* Indication d'arr�t du serveur :			08 arret adresseServeur portServeur
	-> du serveur vers le client
		* Envoi d'un bloc :						09 bloc nomDeFichier numeroDeBloc contenuDuBloc
		* Bloc introuvable :						10 erreur nomDeFichier numeroDeBloc

b) Annuaire
	_ Structure des donn�es utilis�es :
		-> Stockage de la liste des fichiers :
			* Les fichiers sont d�coup�s en "Bloc"
			* On stocke, pour chaque bloc, la liste des serveurs sur lesquelles il est disponible (sous forme de liste chain�e)
			* Tous les blocs d'un fichier sont stock� dans un tableau de "Bloc"
			* Les fichiers sont stock�s dans une liste (chain�e) de "Fichier"
		-> Stockage de la liste des clients :
		-> Stockage de la liste des serveurs : 
			* Pour chaque serveur, on stocke les donn�es lui correspondant dans une structure � 2 champs : Adresse, et Port
			* L'application garde en m�moire un tableau (dynamique) de pointeur sur "Serveur"

	_ D�roulement du code :
		-> Initialisation de l'annuaire pour la liste des fichiers, des serveurs et des clients (� partir d'un fichier, ou mise � z�ro)
		-> Initialisation de la socket d'�coute de l'annuaire
		-> �coute et analyse des messages re�us
		-> traitement et r�ponse aux messages
		-> il faut que 

c) Client/serveur 
principe de fonctionnement :
	L'application client/serveur est d�coup� en 2 parties ; une partie cliente, et une partie serveur :
		_ La partie serveur g�re la mise � disposition de fichier sur le r�seau (en le notifiant � l'annuaire). 
			Ensuite, la partie serveur s'occupe de r�pondre au demande client pour les fichiers mis � disposition. 
		_ La partie cliente g�re la demande de fichier � l'annuaire, et les demande ensuite au serveur appropri�.

	_ Partie serveur :
		-> Structure des donn�es utilis�es :
			* Liste d'attente des clients qui demandent un transfert :
				- On stocke pour chaque client les donn�es suivantes : la socket cr��e lors de sa connexion, le nom du fichier et le num�ro de bloc demand�s
				- Les diff�rents clients sont stock�s dans une file d'attente
	
	_ Partie client :
		-> Structure des donn�es utilis�es :
			* Liste d'attente des blocs en attente de t�l�chargement :
				- On stocke pour chaque bloc � t�l�charger les donn�es suivantes : l'adresse et le port du serveur, ainsi que le nom du fichier et le num�ro du bloc � demander
				- Les diff�rents blocs sont stock�s dans une file d'attente
			* Liste des fichiers demand�s :
				- On stocke pour chaque fichier, demand� � l'annuaire, le nom du fichier, le nombre total de bloc
				- De plus, pour chaque bloc, on stocke le statut du t�l�chargement : "Pas trait�" / "Trait�"
				- Les diff�rents fichiers sont stock�s dans une liste chain�e
				
	_ D�roulement du code :
	
	
	


*********************************************************************************************

3] Programmation syst�me
a) Socket


b) Thread


c) Processus







