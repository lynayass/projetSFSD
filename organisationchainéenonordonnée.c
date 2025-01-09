#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 

#define MAX_BLOCS 15
#define FACTEUR_BLOCS 15

// Structure d'un enregistrement
typedef struct ENREG ENREG;
struct ENREG {
    int id;            // Identifiant
    char nom[50];      // Nom
};

// Structure des métadonnées d'un fichier
typedef struct METADATA {
    char fichier_nom[50];     // Nom du fichier
    int  taille_blocs;        // Nombre de blocs utilisés dans le fichier
    int  taille_enregs;       // Nombre d'enregistrements
    int  adresse_premier;     // Adresse du premier bloc
    char org_globale[30];     // Organisation globale ('contigu', 'chaîné')
    char org_interne[30];     // Organisation interne ('ORDONNE', 'NONORDONNE')
} METADATA;

// Structure d'un bloc
typedef struct BLOC {
    ENREG enregs[FACTEUR_BLOCS]; // Enregistrements dans le bloc
    int enregs_utilises;         // Nombre d'enregistrements utilisés
    METADATA metadata;           // Métadonnées du fichier
    int suivant;                 // Adresse logique du bloc suivant (pour chaîné)
} BLOC;

typedef struct POSITION POSITION;
struct POSITION {
   int bloc;            
    int deplacement;   
};


//fonction pour trouver l'adresse du dernier bloc
int trouveradressedernierbloc(FILE *ms, int adresse_premier_bloc,int taille_fichier ){
	
	fseek(ms, (adresse_premier_bloc-1) * sizeof(BLOC), SEEK_SET);
    BLOC buffer;
	int j=adresse_premier_bloc;
    int i = 0, k = -1;
    while (i < taille_fichier && j != -1) { //verifier pour ne pas dÃ©passer taille du fichier en blocs
        fseek(ms, (j-1) * sizeof(BLOC), SEEK_SET);
		fread(&buffer, sizeof(BLOC), 1, ms);
		k=j;
		j = buffer.suivant;
		i++;
	}
	return k;
}
 

// Fonction pour calculer le nombre de blocs libres dans un fichier contenant une table d'allocation
int nombredeblocsLibres(FILE *ms) { // f est le fichier qui contient la table d'allocation
    rewind(ms);
    BLOC buffer;
    int nombre_de_blocs_libres = 0;
    
    fread(&buffer, sizeof(BLOC), 1, ms);
    for (int i = 1; i < MAX_BLOCS; i++) {
        if (buffer.enregs[i].id == 0) {  // Bloc libre
            nombre_de_blocs_libres++;
        }
    }

    return nombre_de_blocs_libres;  // Retourne le nombre de blocs libres (remarque: le max de blocs libres qu'on peut avoir est(MAX_BLOCS-1)
}

// Fonction pour allouer un nouveau bloc
int allouerNouveauBloc(FILE *ms) {
    rewind(ms); // Positionner au début pour lire le bloc 0

    BLOC buffer;
    BLOC table_allocation; // Pour stocker la table d'allocation (bloc 0)
    BLOC nouveau_bloc;     // Pour manipuler le nouveau bloc
    
    // Lire le bloc 0 (table d'allocation) dans le buffer
    fread(&buffer, sizeof(BLOC), 1, ms);
    table_allocation = buffer; // Copier les données du buffer dans la table d'allocation

    // Parcourir la table d'allocation pour trouver un bloc libre
    for (int i = 1; i < MAX_BLOCS; i++) { // Commencer à 1 (bloc 0 est réservé pour la table)
        if (table_allocation.enregs[i].id == 0) { // Vérifie si le bloc est libre
            table_allocation.enregs[i].id = 1; // Marque ce bloc comme occupé

            // Mise à jour de la table d'allocation (bloc 0)
            buffer = table_allocation;
            rewind(ms); // Retourner au début pour réécrire le bloc 0
            fwrite(&buffer, sizeof(BLOC), 1, ms); 

            // Lire le bloc alloué pour le modifier
            fseek(ms, (i-1) * sizeof(BLOC), SEEK_SET); // Positionner au bloc `i`
            fread(&buffer, sizeof(BLOC), 1, ms); 
          
            nouveau_bloc = buffer; // Charger le bloc à partir du buffer
            nouveau_bloc.suivant = -1;             // Aucun bloc suivant
            nouveau_bloc.enregs_utilises = 0;      // Pas d'enregistrements utilisés initialement
            
            // Réécrire le bloc modifié
            fseek(ms, (i-1) * sizeof(BLOC), SEEK_SET);
            buffer = nouveau_bloc;
            fwrite(&buffer, sizeof(BLOC), 1, ms);

            printf("Nouveau bloc alloué à l'adresse logique : %d\n", i);
            return i; // Retourner l'indice logique du bloc alloué
        }
    }

    // Si aucun bloc libre n'a été trouvé
    printf("Erreur : Aucun bloc libre disponible.\n");
    return -1;
}



//Fonction pour chercher le premier bloc libre dans la table d'allocation et le marquer comme occupé
int chercherBlocLibre(int *tablleAllocation ){
	int k=1;
    for(k=1; k < MAX_BLOCS; k++){
		if(tablleAllocation[k] == 0){  // Si un bloc est libre (valeur 0),
			tablleAllocation[k] = 1 ;  //marquer comme occupé
            return k+1;  //  retourner l'indice de ce bloc
		}
	}
    return -1;
}


// Fonction pour générer une chaîne de caractères aléatoire
void rand_string(char* str, int num){
	// Remplir la chaîne avec des lettres aléatoires entre 'a' et 'z'
	for(int i = 0;i < num; i++){
		str[i] = rand()  % ('z' - 'a' +1)+ 'a';
	}
	str[num] = '\0';   // // Ajouter un terminateur null à la fin de la chaîne
}


// Fonction pour charger un fichier dans une organisation chaînée non ordonnée

void chargerChaineNonOrdonne(FILE *ms, METADATA metadata) {
    printf("Chargement du fichier '%s' dans une organisation Chaînée Non ORDONNE.\n", metadata.fichier_nom);
    BLOC buffer1;
    BLOC buffer2;
    int tablleAllocation[MAX_BLOCS] = {0};  // Initialiser tablleAllocation avec des zéros
    int j = 0, position = 0, emplacementBlocPresedent,i;

    rewind(ms);   // Réinitialiser le curseur du fichier pour lire depuis le début
    if (fread(&buffer1, sizeof(buffer1), 1, ms) != 1) {  // Lecture de la table d'allocation
        perror("Erreur : Impossible de lire le fichier.\n");
        return;
    }
    
    for(int k = 0; k < MAX_BLOCS; k++) { // Copier la table d'allocation
        tablleAllocation[k] = buffer1.enregs[k].id;
    } 

    srand(time(0));
    // Pour chaque bloc
    for( i = 0; i < metadata.taille_blocs; i++) {
        emplacementBlocPresedent = position;  //sauvegarder l'adresse du bloc précédent dans variable emplacementBlocPresedent
        memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1

        // Remplir le bloc courant avec des enregistrements alétoire 
        while(j < metadata.taille_enregs && buffer1.enregs_utilises < FACTEUR_BLOCS) {
            buffer1.enregs[buffer1.enregs_utilises].id = rand(); // génerer ID aléatoire
            rand_string(buffer1.enregs[buffer1.enregs_utilises].nom, 49);  // Générer un nom aléatoire
            buffer1.enregs_utilises++;  // Augmenter le compteur des enregistrements utilisés
            j++; // Augmenter le compteur des enregistrements total dans le fichier 
        }

        // Trouver un bloc libre
        position = chercherBlocLibre(tablleAllocation); ////retourner l'indice de bloc libre et marquer comme occupé 
        if(position == -1){
            printf("No free blocks available\n");
            return;
        }
        strcpy(buffer1.metadata.fichier_nom, metadata.fichier_nom); //remplire le nom du fichier dans buffer1
        if(i == 0) {// Premier bloc
            metadata.adresse_premier = position;    // sauvegarder l'adresse du premier bloc de fichier dans variable metadata 
            memcpy(&buffer1.metadata, &metadata, sizeof(METADATA)); // Copier les métadonnées de fichier dans buffer1
        } else {
            // Mettre à jour le pointeur du bloc précédent
            buffer2.suivant = position;  //remplire la position de bloc actuel dans le bloc précédent
            fseek(ms, (emplacementBlocPresedent-1) * sizeof(BLOC), SEEK_SET);
            if (fwrite(&buffer2, sizeof(BLOC), 1, ms) != 1) {  //ecrire le bloc précédent dans le fichier
                printf("Erreur d'écriture du bloc précédent\n");
                return;
            }
            fseek(ms, (emplacementBlocPresedent-1) * sizeof(BLOC), SEEK_SET);
            fread(&buffer2, sizeof(BLOC), 1, ms);
        }

        // Sauvegarder le bloc courant pour le prochain tour
        memcpy(&buffer2, &buffer1, sizeof(BLOC));
    }

    // Écrire le dernier bloc
    buffer2.suivant = -1;  // Terminer la chaîne de blocs en mettant le champ 'suivant' à -1  
    fseek(ms, (position-1) * sizeof(BLOC), SEEK_SET);
    if (fwrite(&buffer2, sizeof(BLOC), 1, ms) != 1) {  //ecrire le dernier bloc dans le fichier 
        printf("Erreur d'écriture du dernier bloc\n");
        return;
    }

    // Mettre à jour la table d'allocation
    rewind(ms);
    for(int k = 0; k < MAX_BLOCS; k++) {
        buffer1.enregs[k].id = tablleAllocation[k];
    }
    if (fwrite(&buffer1, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur de mise à jour de la table d'allocation\n");
    }

    printf("Chargement terminé avec succès\n");
}


// Fonction d'insertion dans une organisation chaînée et non ordonnée
void insertionChaineeNonOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg) { 
    
	printf("Insertion dans une organisation Chaînée non NONORDONNE.\n");
	
	rewind(ms);
    BLOC table_allocation, buffer;
    int nombre_de_blocs_libres = 0;
    bool fichier_trouve = false;
    int adresse_premier = -1, taille_fichier = 0, adresse_dernier_bloc,taille_enregs;
     
	 // Lecture de la table d'allocation
    fread(&buffer, sizeof(BLOC), 1, ms);
    table_allocation = buffer;
    
    int i=1; //compteur de blocs parcourus
    //deplacer le curseur vers le premier bloc (bloc num 1)
	fseek(ms, 0 * sizeof(BLOC), SEEK_SET);
    
	// Recherche le fichier en comparant son nom avec les non des autres fichiers dans les BLOCs occupés
    while (i<MAX_BLOCS && fichier_trouve == false){
	
        if (table_allocation.enregs[i].id == 1){ //blocs occupé
		    
			fseek(ms, (i-1) * sizeof(BLOC), SEEK_SET);
			fread(&buffer, sizeof(BLOC), 1, ms);
	        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0 && buffer.metadata.adresse_premier==i) { //comparer le nom du fichier avec les fichiers existants dans la mémoire secondaire 
            fichier_trouve = true;
            adresse_premier = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            taille_enregs =buffer.metadata.taille_enregs;
            break;
        }
    }
      i++;
  }
    if (fichier_trouve == false ) {
        printf("Erreur : fichier non trouve.\n"); //le fichier n'existe pas
        return;
    }

	adresse_dernier_bloc = trouveradressedernierbloc(ms,adresse_premier,taille_fichier ); //l'adresse du dernier bloc
    // Lecture du dernier bloc
    fseek(ms, (adresse_dernier_bloc-1) * sizeof(BLOC), SEEK_SET);
    fread(&buffer, sizeof(BLOC), 1, ms);

    // Vérification de l'espace dans le dernier bloc
    if (buffer.enregs_utilises < FACTEUR_BLOCS) {
    	buffer.enregs_utilises++; // incrémenter le nombre d'enregistrements utilsés
        buffer.enregs[buffer.enregs_utilises] = nouvel_enreg; // Inserer le nouvel enregistrement
        fseek(ms, (adresse_dernier_bloc-1) * sizeof(BLOC), SEEK_SET);
        fwrite(&buffer, sizeof(BLOC), 1, ms);
        printf("Enregistrement insére avec succes.\n");
    } else {
    	//dernier bloc plein donc Allocation nécessaire
		//vérifier si il y'a assez d'espace dans la mémoire secondaire pour allouer un nouveau bloc
    	int nombre_de_blocs_Libres = nombredeblocsLibres(ms);
    	if (nombre_de_blocs_Libres == 0){ // il y'a pas de bloc libre
		int choix;
		printf("espace insuffisant pour une insertion, voulez vous proceder a un compactage pour liberer un peu d espace ?\n ");
        printf("1. Oui\n");
        printf("2. Non\n");
        
        do {
        printf("Veuillez entrer 1 ou 2 : ");
        if (scanf("%d", &choix) != 1) { 
        // Si scanf échoue (entrée non numérique), vider le buffer
        printf("Entrée invalide. ");
        while (getchar() != '\n'); // Nettoyer le buffer
        choix = 0; // Réinitialiser choix à une valeur non valide
        }
       } while (choix != 1 && choix != 2); // Répéter tant que l'entrée n'est pas valide

		if (choix == 2){ // Non 
        	printf("echec de l insertion: Pas despace disponible. \n");//pas d'espace
			return;
		}
		
		if (choix == 1){ // oui
		compactageMemoire(ms);// Effectuer le compactage
		 printf("Compactage en cours...\n");
		 // Vérifier l'espace libre après le compactage
        	nombre_de_blocs_Libres = nombredeblocsLibres(ms);
        	if (nombre_de_blocs_Libres == 0){ // il y'a pas de bloc libre
        	printf("echec insertion: memoire secondaire pleine ,espace insuffisant\n");
			return;
			}
		}
	   
	   }
	   // cas ou il y'a un bloc libre aprés ou sans compactage
	   
       	//modifier le pointeur de l'ancien dernier bloc 
		int j= allouerNouveauBloc(ms); //adresse du bloc alloué
        fseek(ms, (adresse_dernier_bloc-1) * sizeof(BLOC), SEEK_SET);
		fread(&buffer, sizeof(BLOC), 1, ms);
		buffer.suivant=j; //chainer le dernier bloc du fichier avec le nouveau bloc alloué qui devient à son tour le dernier bloc
		fwrite(&buffer, sizeof(BLOC), 1, ms); 
		
		
		//remplir le bloc alloué et le mettre à jour dans la ms 
        fseek(ms, (j-1) * sizeof(BLOC), SEEK_SET); //accéder au nouveau dernier bloc 
		buffer.enregs[0] = nouvel_enreg;  // Inserer l'enregistrement au debut du nouveau bloc allouer
        buffer.enregs_utilises = 1;  // incrémenter le nombre d'enregistrements utilsés à 1
		strcpy(buffer.metadata.fichier_nom, nom_fichier); //remplire la matdonnée du nom
		fwrite(&buffer, sizeof(BLOC), 1, ms); // le mettre à jour dans la mémoire secondaire 
        
		//mettre à jour la table d'allocation 
        table_allocation.enregs[j].id == 1;
        buffer = table_allocation;
        rewind(ms);//se deplacer au tout debut du fichier de la mémoire secondaire 
        fwrite(&buffer, sizeof(BLOC), 1, ms); //modifier la table d'allocation dans la mémoire secondaire 
	    
	    // mettre à jour les metadonnée stockées dans le premire bloc  
			 fseek(ms, (adresse_premier-1)*sizeof(BLOC), SEEK_SET);
			 fread(&buffer, sizeof(BLOC), 1, ms);
			 buffer.metadata.taille_enregs++;
			 buffer.metadata.taille_blocs++;
			 fseek(ms, (adresse_premier-1)*sizeof(BLOC), SEEK_SET);
			 fwrite(&buffer, sizeof(BLOC), 1, ms);  
	 
     
}
}


// Fonction de recherche dans une organisation chaînée et non ordonnée
POSITION rechercherChaineeNonOrdonnee(FILE *ms, const char *nom_fichier, int id) {
    
    printf("Recherche dans une organisation Chaînée et NON ORDONNE pour l'ID %d.\n", id);
     
	rewind(ms);
    BLOC table_allocation,buffer;
    int nombre_de_blocs_libres = 0;
    bool fichier_trouve = false;
    int adresse_premier = -1, taille_fichier = 0, indice,taille_enregs;
    POSITION p;
    p.bloc = -1; //initialisation
    p.deplacement= -1;
    
	// Lecture de la table d'allocation
    fread(&buffer, sizeof(BLOC), 1, ms);
    table_allocation = buffer;
    
    int i=1; //compteur de blocs parcourus
    
    //deplacer le curseur vers le premier bloc (bloc num 1)
	fseek(ms, 0 * sizeof(BLOC), SEEK_SET);
    
	// Recherche le fichier en comparant son nom avec les non des autres fichiers dans les blocs occupés
    while (i<MAX_BLOCS && fichier_trouve == false){
	
        if (table_allocation.enregs[i].id == 1){ //blocs occupé
		    
			fseek(ms, (i-1) * sizeof(BLOC), SEEK_SET);
		    fread(&buffer, sizeof(BLOC), 1, ms);
	        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0 && buffer.metadata.adresse_premier==i) { //comparer le nom du fichier avec les fichiers existants dans la mémoire secondaire 
            fichier_trouve = true;
            adresse_premier = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            taille_enregs =buffer.metadata.taille_enregs;
            break;
        }
    }
      i++;
  }
    if (fichier_trouve == false ) {
        printf("Erreur : fichier non trouve.\n"); //le fichier n'existe pas
        return p;
    }
      if (buffer.enregs_utilises==0){//le fichier ne contient aucun enregistrment 
      	 printf("le fichier est vide.\n");
      	 return p;
	  }
     
    // Parcours des blocs pour chercher l'enregistrement
    indice = adresse_premier;
	int j=0;
    for (int i = 0; i < taille_fichier; i++) { //verifier pour ne pas dépasser taille du fichier en blocs
        int k=0;
		fseek(ms, (indice-1) * sizeof(BLOC), SEEK_SET);//accéder au bloc num indice
		fread(&buffer, sizeof(BLOC), 1, ms);
        while(j < taille_enregs && k < buffer.enregs_utilises) {//verifier pour ne pas dépasser taille du fichier en enregistrement 
        	
            if (buffer.enregs[k].id == id) {    	  
                printf("L enregistrement se trouve dans le bloc : %d, de decalage : %d \n",indice, k);
                p.bloc = indice ;
                p.deplacement=k;
                return p;
            }
        j++; k++;
		}
        indice = buffer.suivant; //adresse du bloc suivant
	}

    printf("L enregistrement n existe pas.\n");
    return p; // p =-1
}
    

// Suppression logique : marquer comme supprimé dans une organisation chaînée et non ordonnée
void suppressionChaineeNonOrdonneeLogique(FILE *ms, const char *nom_fichier, int id) {
     
	printf("Suppression logique de l'enregistrement avec ID %d dans une organisation Chaînée et NONORDONNE.\n", id);
	
	rewind(ms);
    BLOC table_allocation,buffer;
    int nombre_de_blocs_libres = 0;
    bool fichier_trouve = false;
    int adresse_premier = -1, taille_fichier = 0, taille_enregs;
    
	// Lecture de la table d'allocation
    fread(&buffer, sizeof(BLOC), 1, ms);
    table_allocation = buffer;
    
    int i=1; //compteur de blocs parcourus
    
    //deplacer le curseur vers le premier bloc (bloc num 1)
	fseek(ms, 0 * sizeof(BLOC), SEEK_SET);
    
	// Recherche le fichier en comparant son nom avec les non des autres fichiers dans les blocs occupés
    while (i<MAX_BLOCS && fichier_trouve == false){
	
        if (table_allocation.enregs[i].id == 1){ //blocs occupé
		    
			fseek(ms, (i-1) * sizeof(BLOC), SEEK_SET);
		    fread(&buffer, sizeof(BLOC), 1, ms);
	        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0 && buffer.metadata.adresse_premier==i) { //comparer le nom du fichier avec les fichiers existants dans la mémoire secondaire 
            fichier_trouve = true;
            adresse_premier = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            taille_enregs =buffer.metadata.taille_enregs;
            break;
        }
    }
      i++;
  }
    if (fichier_trouve == false ) {
        printf("Erreur : fichier non trouve.\n"); //le fichier n'existe pas
        return;
    }

    // Parcours des blocs pour chercher l'enregistrement
    fseek(ms, (adresse_premier-1) * sizeof(BLOC), SEEK_SET);
    int indice = adresse_premier;
	int j=0;
    for (int i = 0; i < taille_fichier; i++) { //verifier pour ne pas dépasser taille du fichier en blocs
        int k=0;
		fseek(ms, (indice-1) * sizeof(BLOC), SEEK_SET);
		fread(&buffer, sizeof(BLOC), 1, ms);
        while(j < taille_enregs && k < buffer.enregs_utilises && indice != -1) {//verifier pour ne pas dépasser taille du fichier en enregistrement 
        	
            if (buffer.enregs[k].id == id) {
            	buffer.enregs[k].id = -1;
            	
                printf("L enregistrement avec ID %d a etait suprrime logiquement avec succés. \n",id);
                return;
            }
            
        j++; k++;
		}
    int indice = buffer.suivant; //adresse du bloc suivant
	}

    printf("Enregistrement avec ID %d introuvable dans le fichier '%s'.\n", id, nom_fichier);
}


// Suppression physique : réorganiser les blocs pour libérer l'espace dans une organisation chaînée et non ordonnée
void suppressionChaineeNonOrdonneePhysique(FILE *ms, const char *nom_fichier, int id) {
    
	rewind(ms);
    BLOC table_allocation,buffer;
    int nombre_de_blocs_libres = 0;
    bool fichier_trouve = false;
    int adresse_premier = -1, taille_fichier = 0, adresse_dernier_bloc,taille_enregs,adresse_avant_dernier_bloc;
    
	// Lecture de la table d'allocation
    fread(&buffer, sizeof(BLOC), 1, ms);
    table_allocation = buffer;
    
    int i=1; //compteur de blocs parcourus
    
    //deplacer le curseur vers le premier bloc (bloc num 1)
	fseek(ms, 0 * sizeof(BLOC), SEEK_SET);
    
	// Recherche le fichier en comparant son nom avec les non des autres fichiers dans les blocs occupés
    while (i<MAX_BLOCS && fichier_trouve == false){
	
        if (table_allocation.enregs[i].id == 1){ //blocs occupé
		    
			fseek(ms, (i-1) * sizeof(BLOC), SEEK_SET);
		    fread(&buffer, sizeof(BLOC), 1, ms);
	        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0 && buffer.metadata.adresse_premier==i) { //comparer le nom du fichier avec les fichiers existants dans la mémoire secondaire 
            fichier_trouve = true;
            adresse_premier = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            taille_enregs =buffer.metadata.taille_enregs;
            break;
        }
    }
      i++;
  }
    if (fichier_trouve == false ) {
        printf("Erreur : fichier non trouve.\n"); //le fichier n'existe pas
        return;
    }
    adresse_dernier_bloc = trouveradressedernierbloc(ms,adresse_premier, taille_fichier); //l'adresse du dernier bloc
            
	//trouver le dernière enregistrement du fichier 
    fseek(ms, (adresse_dernier_bloc-1) * sizeof(BLOC), SEEK_SET);
    fread(&buffer, sizeof(BLOC), 1, ms);
	
	ENREG dernier_enregistrement = buffer.enregs[buffer.enregs_utilises]; 

     
    // chercher l'enregistrement
            	POSITION p = rechercherChaineeNonOrdonnee(ms, nom_fichier,id);
                
				if (p.bloc != -1) { //l'enregistrement à supprimer existe 
            	   fseek(ms, (p.bloc-1) * sizeof(BLOC), SEEK_SET); // accéder au bloc ou se trouve l'enregistrement a suprimer physiquement du fichier
				   fread(&buffer, sizeof(BLOC), 1, ms); 
				   buffer.enregs[p.deplacement] = dernier_enregistrement;//ecraser l'enregistrement à supprimer par le dernier enregistrement du fichier
            	   fseek(ms, (p.bloc-1) * sizeof(BLOC), SEEK_SET);
				   fwrite(&buffer, sizeof(BLOC), 1, ms); 
				   
                if (taille_enregs==(taille_fichier-1)*FACTEUR_BLOCS+1){// le dernier bloc ne contenait qu'un seul enregistrement donc il deviendra vide aprés suppresion
            		table_allocation.enregs[adresse_dernier_bloc].id == 0; //mettre à jour la table d'alloction (marquer le bloc comme libre)
            		
					fseek(ms, (adresse_dernier_bloc-1) * sizeof(BLOC), SEEK_SET);
                    fread(&buffer, sizeof(BLOC), 1, ms);
                    buffer.enregs_utilises=0;
					strcpy(buffer.metadata.fichier_nom, " "); //initiliser les metadonnées du dernier bloc
                    fseek(ms, (adresse_dernier_bloc-1) * sizeof(BLOC), SEEK_SET);
                    fwrite(&buffer, sizeof(BLOC), 1, ms); 
                    
					adresse_avant_dernier_bloc=trouveradressedernierbloc(ms,adresse_premier, taille_fichier-1); //l'adresse de l'avant dernier bloc
            	
				if 	(adresse_avant_dernier_bloc != -1){ // cas: le fichir contient plus d'un blos  
			
			    //mettre à jour les metadonnés dans le premier bloc 
				 fseek(ms, (adresse_premier-1)*sizeof(BLOC), SEEK_SET);
				 fread(&buffer, sizeof(BLOC), 1, ms);
				 buffer.metadata.taille_enregs--;
				 buffer.metadata.taille_blocs--;
				fseek(ms, (adresse_premier-1)*sizeof(BLOC), SEEK_SET);
				 fwrite(&buffer, sizeof(BLOC), 1, ms);  
				 
				}else{//le fichier ne contient qu'un seul bloc et un seul enregistrement (il ne sera pas supprimé mais devien vide)
					fseek(ms, (adresse_premier-1)*sizeof(BLOC), SEEK_SET);//accéder au seul bloc du fichier
					buffer.metadata.taille_enregs--;//mettre à jour les metadonné
				    taille_fichier++;//le fichier contient encore un bloc mais il est vide
					fwrite(&buffer, sizeof(BLOC), 1, ms);  
				    return;  
				}
				}
                //cas ou aucun bloc n'a etait suprimmé 
                 fseek(ms, -(long)(sizeof(BLOC)), SEEK_CUR);
				 fwrite(&buffer, sizeof(BLOC), 1, ms); //mettre à jour les information du dernier bloc
				 
				 //mettre à jour les metadonnés dans le premier bloc 
				 fseek(ms, (adresse_premier-1)*sizeof(BLOC), SEEK_SET);
				 fread(&buffer, sizeof(BLOC), 1, ms);
				 buffer.metadata.taille_enregs--;
				 fseek(ms, -(long)(sizeof(BLOC)), SEEK_CUR);
				 fwrite(&buffer, sizeof(BLOC), 1, ms);  
				printf("L enregistrement avec ID %d a etait suprrime physiquement avec succés : \n",id);
				 return;
				}
		
       printf("Enregistrement avec ID %d introuvable dans le fichier '%s' .\n", id ,nom_fichier);
       return;
}


// Fonction pour défragmenter un fichier dans une organisation chaînée et non ordonnée
void defragmentationChaineeNonOrdonnee(FILE *ms, const char *nom_fichier) {
     // Réorganiser les enregistrements pour réduire les espaces vides dans la chaîne
	printf("Défragmentation dans une organisation Chaînée et NONORDONNE.\n");
   
    rewind(ms);
    BLOC table_allocation,buffer;
    int nombre_de_blocs_libres = 0,adresse_premier,taille_fichier,taille_enregs;
    bool fichier_trouve = false;
    
	// Lecture de la table d'allocation
    fread(&buffer, sizeof(BLOC), 1, ms);
    table_allocation = buffer;
    
    int i=1; //compteur de blocs parcourus
    
    //deplacer le curseur vers le premier bloc (bloc num 1)
	fseek(ms, 0 * sizeof(BLOC), SEEK_SET);
    
	// Recherche le fichier en comparant son nom avec les non des autres fichiers dans les blocs occupés
    while (i<MAX_BLOCS && fichier_trouve == false){
	
        if (table_allocation.enregs[i].id == 1){ //blocs occupé
		    
			fseek(ms, (i-1) * sizeof(BLOC), SEEK_SET);
		    fread(&buffer, sizeof(BLOC), 1, ms);
	        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0 && buffer.metadata.adresse_premier==i) { //comparer le nom du fichier avec les fichiers existants dans la mémoire secondaire 
            fichier_trouve = true;
            adresse_premier = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            taille_enregs =buffer.metadata.taille_enregs;
            break;
        }
    }
      i++;
  }
    if (fichier_trouve == false ) {
        printf("Erreur : fichier non trouve.\n"); //le fichier n'existe pas
        return;
    }
     
      
    int adresse_suivante = buffer.suivant , nombre_apparition =0;
    
    int indice = adresse_premier;                                  //initialisation de l'indice et le nombre d'enregistrements supprimés logiquement dans le fichier
    // Parcours des blocs pour chercher l'enregistrement
    int j=0;                                                       //initialisation du compteur d'enregistrements du fichier
    for (int i = 0; i < taille_fichier; i++) {                     //verifier pour ne pas dépasser taille du fichier en blocs
        int k =0;                                                  //initialisation du compteur d'enregistrements de chaque bloc 
		fseek(ms, (indice-1) * sizeof(BLOC), SEEK_SET); 
		fread(&buffer, sizeof(BLOC), 1, ms);                       // lire le bloc à l'adresse indice
        while(j < taille_enregs && k < buffer.enregs_utilises) {   //verifier pour ne pas dépasser taille du fichier en enregistrement 
        	
            if (buffer.enregs[k].id == -1) {
            	nombre_apparition++;                               //calculer le nombre d'enregistrements supprimés logiquement dans le fichier
            }
        j++; k++;
		}
        indice = buffer.suivant;                                   //adresse du bloc suivant
	}
	if (nombre_apparition ==0){ // aucun  enregistrement n'a était supprimé logiquement
	printf("pas de Enregistrements supprimes logiquement dans ce fichier.\n");
    return;	
	}
    i=0;
  while (i<nombre_apparition){
       suppressionChaineeNonOrdonneePhysique(ms, nom_fichier, -1);
  	i++;
	     
  }
}
