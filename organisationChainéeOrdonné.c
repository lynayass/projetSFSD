#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define MAX_BLOCS 15
#define FACTEUR_BLOCS 15

// Structure d'un enregistrement
typedef struct ENREG ENREG;
struct ENREG {
    int id;            // Identifiant
    char nom[50];      // Nom
};

typedef struct POSITION POSITION;
struct POSITION{
    int bloc;            
    int deplacement;        
};

// Structure des métadonnées d'un fichier
typedef struct METADATA METADATA;
struct METADATA {
    char fichier_nom[50];     // Nom du fichier
    int taille_blocs;         // Nombre de blocs
    int taille_enregs;        // Nombre d'enregistrements
    int adresse_premier;      // Adresse du premier bloc
    char org_globale[30];     // Organisation globale ('contigu', 'chaîné', etc.)
    char org_interne[30];     // Organisation interne ('ORDONNE', 'NONORDONNE', etc.)
};

// Structure d'un bloc
typedef struct BLOC BLOC;
struct BLOC {
    ENREG enregs[FACTEUR_BLOCS]; // Enregistrements dans le bloc
    int enregs_utilises;         // Nombre d'enregistrements utilisés
    METADATA metadata;          // Métadonnées du fichier
    int suivant;       //  le bloc suivant (pour chaîné)
};


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

// Fonction pour comparer deux entiers (utilisée par qsort)
int comparer(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Fonction pour générer des nombres triés globalement
void genererNombresTries(int *tousLesNombres,int NB_TOTAL) {
    int i;
    for (i = 0; i < NB_TOTAL; i++) {
        tousLesNombres[i] = rand() ;  // Générer des nombres aléatoires 
    }
    qsort(tousLesNombres, NB_TOTAL, sizeof(int), comparer);  // Trier les nombres
}

// Fonction pour générer une chaîne de caractères aléatoire
void rand_string(char* str, int num){
	// Remplir la chaîne avec des lettres aléatoires entre 'a' et 'z'
	for(int i = 0;i < num; i++){
		str[i] = rand()  % ('z' - 'a' +1)+ 'a';
	}
	str[num] = '\0';   // // Ajouter un terminateur null à la fin de la chaîne
}


// Fonction pour charger un fichier dans une organisation chaînée et ordonnée
void chargerChaineeOrdonne(FILE *ms, METADATA metadata) {
    printf("Chargement du fichier '%s' dans une organisation Chaînée et ORDONNE.\n", metadata.fichier_nom);
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

    int* tousLesNombres = (int*)malloc(metadata.taille_enregs * sizeof(int)); // Allouer dynamiquement la mémoire pour `tousLesNombres` car sa taille est déterminée à l'exécution
    if (tousLesNombres == NULL) {
        perror("Erreur d'allocation mémoire\n");
        return;
    }

    srand(time(0));
    genererNombresTries(tousLesNombres, metadata.taille_enregs);  // générer des nombres triés et remplire le tableau 

    // Pour chaque bloc
    for( i = 0; i < metadata.taille_blocs; i++) {
        emplacementBlocPresedent = position;  //sauvegarder l'adresse du bloc précédent dans variable emplacementBlocPresedent
        memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1

        // Remplir le bloc courant avec des enregistrements alétoire 
        while(j < metadata.taille_enregs && buffer1.enregs_utilises < FACTEUR_BLOCS) {
            buffer1.enregs[buffer1.enregs_utilises].id = tousLesNombres[j]; // Assigner un ID aléatoire
            rand_string(buffer1.enregs[buffer1.enregs_utilises].nom, 49);  // Générer un nom aléatoire
            buffer1.enregs_utilises++;  // Augmenter le compteur des enregistrements utilisés
            j++; // Augmenter le compteur des enregistrements total dans le fichier 
        }

        // Trouver un bloc libre
        position = chercherBlocLibre(tablleAllocation); ////retourner l'indice de bloc libre et marquer comme occupé 
        if(position == -1){
            printf("No free blocks available\n");
            free(tousLesNombres);
            return;
        }
        strcpy(buffer1.metadata.fichier_nom, metadata.fichier_nom); //rempire le nom du fichier dans buffer1
        if(i == 0) {// Premier bloc
            metadata.adresse_premier = position;    // sauvegarder l'adresse du premier bloc de fichier dans variable metadata 
            memcpy(&buffer1.metadata, &metadata, sizeof(METADATA)); // Copier les métadonnées de fichier dans buffer1 
        } else {
            // Mettre à jour le pointeur du bloc précédent
            buffer2.suivant = position;  //remplire la position de bloc actuel dans le bloc précédent
            fseek(ms, (emplacementBlocPresedent-1) * sizeof(BLOC), SEEK_SET);
            if (fwrite(&buffer2, sizeof(BLOC), 1, ms) != 1) {  //ecrire le bloc précédent dans le fichier
                printf("Erreur d'écriture du bloc précédent\n");
                free(tousLesNombres);
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
    if (fwrite(&buffer2, sizeof(BLOC), 1, ms) != 1) {  //ecrire le deriier bloc dans le fichier 
        printf("Erreur d'écriture du dernier bloc\n");
        free(tousLesNombres);   // Libérer la mémoire allouée dynamiquement
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

    free(tousLesNombres);   // Libérer la mémoire allouée dynamiquement
    printf("Chargement terminé avec succès\n");
}


// Fonction pour gérer le décalage lors de l'insertion dans une organisation chaînée et ordonee
int decalage(FILE *ms ,const char *nom_fichier,int adresse_premier,int taille_blocs,POSITION position , BLOC buffer,int nombreBlocParcourir,ENREG x,int *tablleAllocation){
	ENREG y;
	BLOC buffer1;
	int i,j,allouerNouveuxBloc = 0,positionBloc = position.bloc,decalage = 1;
	i = 1; 
    
    //vérifier si il ya decalage ou pas (cas de l'enregistrement à inserer supéreur à tous l'enregistrements et dernier bloc est plein)
    if((nombreBlocParcourir - 1) == taille_blocs && position.deplacement == (buffer.enregs_utilises - 1) && buffer.suivant == -1){
        decalage = 0;
        position.deplacement == 0;
    }

    // fait le decalage et insérer le nouvel enregistrement
	while(i <= taille_blocs){
		
        if(decalage == 1){  //fait le decalage à l'intérieur bloc
            for(j = position.deplacement ; j<buffer.enregs_utilises; j++){  
		        memcpy(&y, &buffer.enregs[j], sizeof(x));
		        memcpy(&buffer.enregs[j], &x, sizeof(x));
		        memcpy(&x, &y, sizeof(x));
          	} 
        }

        //inserer le nouvel enregistrement x
       	if(buffer.enregs_utilises < FACTEUR_BLOCS){ //si le bloc n'est pas plein 
       		memcpy(&buffer.enregs[buffer.enregs_utilises], &x, sizeof(x));
       	    buffer.enregs_utilises++;
       	    fseek(ms, (positionBloc - 1) * sizeof(BLOC), SEEK_SET);
            fwrite(&buffer, sizeof(BLOC), 1, ms);
            break;
		}else{  //si le bloc est plein
			printf("si bloc est plein%d\n",x.id);
            if(buffer.suivant == -1){   // Si un bloc suivant n'existe pas
			    int nouveauBloc = chercherBlocLibre(tablleAllocation);   //allouer un nouveau bloc 
                if (nouveauBloc == -1) { //si l'espace n'est pas suffisant pour allouer un nouveau bloc
                    int choix;
                    printf("Pas de blocs disponibles pour allouer un nouveau bloc. Choisissez une option :\n");
                    printf("1. Compactage de la mémoire\n");
                    printf("2. Annuler l'insertion\n");
                    scanf("%d", &choix);

                    if (choix == 1) {
                        // compactageMemoire(ms); // Assurez-vous que cette fonction existe
                        nouveauBloc = chercherBlocLibre(tablleAllocation);
                        if (nouveauBloc == -1) {
                            printf("Erreur : Pas de blocs disponibles même après compactage.\n");
                            return -12;
                        }
                    } else {
                        printf("Insertion annulée.\n");
                        return -13;
                    }
                }
                // Écrire  le bloc complet actuel
                buffer.suivant = nouveauBloc; // Mettre à jour le pointeur suivant
                fseek(ms, (positionBloc- 1)*sizeof(BLOC), SEEK_SET);
                if (fwrite(&buffer, sizeof(BLOC), 1, ms) != 1) {  
                    printf("Erreur d'écriture du bloc\n");
                }
                //Initialiser et écrire le nouveau bloc
                memset(&buffer1, 0, sizeof(BLOC)); // initialiser buffer1
				buffer1.enregs_utilises = 1;
                buffer1.suivant = -1;
                memcpy(&buffer1.metadata.fichier_nom, &nom_fichier, sizeof(nom_fichier));  //inserer le nom du fichier
				memcpy(&buffer1.enregs[0], &x, sizeof(x));  //insérer l'enregistrement x
                fseek(ms, (nouveauBloc- 1)*sizeof(BLOC), SEEK_SET);
			    if (fwrite(&buffer1, sizeof(BLOC), 1, ms) != 1) {  //ecrire le nouveau bloc 
                    printf("Erreur d'écriture du bloc\n");
                }   
                taille_blocs ++;
                positionBloc = nouveauBloc; 
                break;
			}
             //Écrire le bloc actuel 
			fseek(ms, (positionBloc - 1) * sizeof(BLOC), SEEK_SET);
            fwrite(&buffer, sizeof(BLOC), 1, ms); 
            if(buffer.suivant == -1){
            	break;
			}     
            positionBloc = buffer.suivant;   //Passer au bloc suivant
            memset(&buffer, 0, sizeof(BLOC)); // Réinitialiser buffer
            fseek(ms, (positionBloc - 1) * sizeof(BLOC), SEEK_SET);
            if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) { //lire le bloc suivant
                printf("Erreur d'écriture du bloc\n");
            }  
           
		}
		position.deplacement = 0; 
		i++;
	}
    memset(&buffer, 0, sizeof(BLOC)); // Réinitialiser buffer
	fseek(ms, (adresse_premier- 1) * sizeof(BLOC), SEEK_SET);  //modifier le metadonnée dans premier bloc du fichier 
    fread(&buffer, sizeof(BLOC), 1, ms); 
    buffer.metadata.taille_blocs = taille_blocs;
    buffer.metadata.taille_enregs = buffer.metadata.taille_enregs +1;
    fseek(ms, (adresse_premier- 1) * sizeof(BLOC), SEEK_SET); 
    fwrite(&buffer, sizeof(BLOC), 1, ms); //ecrire dans premier bloc dun fichier
    return positionBloc;
}

// Fonction d'insertion dans une organisation chaînée et ordonnée
void insertionChaineeOrdonnee(FILE *ms, const char *nom_fichier,ENREG nouvel_enreg) {
    printf("Insertion dans une organisation Chaînée et ORDONNE.\n");
    BLOC buffer1,buffer2;
    ENREG x,y;
    int fichier_trouve = 0;  // Indique si le fichier a été trouvé
    int tablleAllocation[MAX_BLOCS ] = {0};   // Initialiser tablleAllocation avec des zéros
    POSITION position;  // Position de l'insertion
    position.deplacement = 0;
    position.bloc = 0;
    int taille_blocs = 0;         // Nombre de blocs de fichier
    int taille_enregs;        // Nombre d'enregistrements de fichier
    int adresse_premier;      // Adresse du premier bloc de fichier
    int blocSuivant = -1;
   	int j,c,k,i=1,positionTrouve = 0;

	rewind(ms);     // Réinitialiser le curseur du fichier pour lire depuis le début
    if (fread(&buffer1, sizeof(buffer1), 1, ms) != 1) {    //Lecture de la table d'allocation
        printf("Erreur : Impossible de lire le fichier.\n");
        return; }
    for(k=0; k<(MAX_BLOCS ); k++ ){
    	tablleAllocation[k] = buffer1.enregs[k].id;    //mettre la table d'allocation (buffer1) dans une variable tablleAllocation
	}

    for(k=0; k<(MAX_BLOCS ); k++ ){
    	printf("table allocation %d\n",tablleAllocation[k]);   
	}

	while (i<MAX_BLOCS && fichier_trouve == 0){ // Parcourir les blocs pour trouver le fichier correspondant
		
        if(tablleAllocation[i] == 1){  // Si le bloc est occupé
			
            memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1
            fseek(ms, (i) * sizeof(BLOC), SEEK_SET);
			fread(&buffer1, sizeof(buffer1), 1, ms);
	        
            if (strcmp(buffer1.metadata.fichier_nom, nom_fichier) == 0 && buffer1.metadata.adresse_premier == i+1) { //comparer le nom du fichier avec les fichiers existants dans la mémoire secondaire 
                fichier_trouve = 1;
                taille_blocs = buffer1.metadata.taille_blocs;         
                taille_enregs = buffer1.metadata.taille_enregs;       
                adresse_premier = buffer1.metadata.adresse_premier; //(car j'ai ecrit le métadonnée du fichier uniquement dans le premier bloc)
                blocSuivant = adresse_premier;
                j = 1;

                if(buffer1.enregs_utilises == 0 ){ //si le fichier est vide ,insertion a premier enregistrement
                	positionTrouve = 1;
				    position.deplacement = 0;
				    position.bloc = adresse_premier;
				    buffer1.enregs_utilises = 1;
				    buffer1.metadata.taille_enregs = 1;
				    memcpy(&buffer1.enregs[0], &nouvel_enreg, sizeof(ENREG));
				    fseek(ms, (position.bloc - 1) * sizeof(BLOC), SEEK_SET);
                    fwrite(&buffer1, sizeof(BLOC), 1, ms);
				    return;
			    }

                // Parcourir les blocs pour trouver la position de l'insertion
                while(j <= taille_blocs && positionTrouve == 0){
					c = 0;
					// Trouver la bonne position dans le bloc
				    while(c < buffer1.enregs_utilises){  
					    if(positionTrouve == 0 && nouvel_enreg.id < buffer1.enregs[c].id ){  
						    positionTrouve = 1;
						    position.deplacement = c;
							position.bloc = blocSuivant;
							break;
					    }
					    if(nouvel_enreg.id == buffer1.enregs[c].id ){
						    printf(" l’enregistrement existe  \n");
						    return;
					    }
					    c++;
				    }
                    /// Si nous n'avons pas trouvé la  position dans cette bloc 
                    if(buffer1.suivant == -1 ){  // Si est le dernier bloc, position a la fin 
                        if(positionTrouve == 0 && c== buffer1.enregs_utilises ){ 
                            positionTrouve = 1;
                            position.deplacement = buffer1.enregs_utilises ;
                            position.bloc = blocSuivant;
                            break;
                        }
                    }else{ // Passer au bloc suivant
                        blocSuivant = buffer1.suivant; //sauvgarder l'adresse de bloc suivant 
                        memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1
					    fseek(ms, (blocSuivant - 1)* sizeof(buffer1), SEEK_SET);
					    fread(&buffer1, sizeof(buffer1), 1, ms);  //lire le bloc suivant 
                    }
					j++;
                } 
            }
        }i++;                 
    }
    
    // Si une position a été trouvée, insérer l'enregistrement
    if(positionTrouve == 1){ 
        memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1
	    fseek(ms, (position.bloc - 1)* sizeof(BLOC), SEEK_SET);
	    fread(&buffer1, sizeof(buffer1), 1, ms);
        c = decalage(ms ,nom_fichier,adresse_premier,taille_blocs,position ,buffer1,j-,nouvel_enreg,tablleAllocation); // Appeler la fonction pour effectuer le décalage et insérer
		if(c > 0){
			printf("Insertion réussie.\n");
		}
	}

    // Si le fichier n'a pas été trouvé, afficher une erreur
    if(fichier_trouve == 0){
    	    printf("Erreur : fichier non trouve.\n"); //le fichier n'existe pas
            return;
	}
}


// Fonction de recherche dans une organisation chaînée et ordonnée
POSITION rechercherChaineeOrdonnee(FILE *ms, const char *nom_fichier, int id) {
	POSITION position;
	position.bloc = -2;   // Initialisation de la position avec des valeurs invalides pour indiquer "non trouvé"
	position.deplacement = -1;
    printf("Recherche dans une organisation Chaînée et ORDONNE pour l'ID %d.\n", id);
    BLOC buffer1;
    int k,a,nbBlocsVides = 0,taille_blocs;
    int tablleAllocation[MAX_BLOCS ] = {0};
    rewind(ms);  // Réinitialiser le pointeur de fichier au début
    if (fread(&buffer1, sizeof(buffer1), 1, ms) != 1) {   // Vérifier si le fichier peut être lu
        printf("Erreur : Impossible de lire le fichier.\n");
        return position; }  // Retourner une position invalide si la lecture échoue
    for(k=0; k<(MAX_BLOCS ); k++ ){
    	tablleAllocation[k] = buffer1.enregs[k].id;    //mettre la table d'allocation (buffer1) dans une variable tablleAllocation
	}
    int idTrouver = 0 ,fichierTrouve = 0;   //  indiquer si l'ID a été trouvé ,indiquer si le fichier a été trouvé
	k =0;
	 // Boucle principale pour parcourir les blocs chaînés
	 while(k<MAX_BLOCS && idTrouver == 0 && fichierTrouve ==0){
	    k++; // Incrémenter le compteur de boucle
    	if(tablleAllocation[k] == 1){    // Vérifier si le bloc actuel est alloué
    	    memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1
            fseek(ms,(k)*sizeof(buffer1),SEEK_SET);
    	    fread(&buffer1, sizeof(buffer1),1,ms);  //lire le contenu de bloc alloué 
    	    if(strcmp(buffer1.metadata.fichier_nom, nom_fichier ) == 0 && buffer1.metadata.adresse_premier == k+1){   //comparer le nom du fichier avec les fichiers existants dans la mémoire secondaire 
				fichierTrouve = 1; //fichier trouve 
                taille_blocs = buffer1.metadata.taille_blocs;   // Obtenir la taille des blocs à partir des métadonnées
    	        int bloc_courant = buffer1.metadata.adresse_premier; //obtenir l'adresse du premier bloc
                int nbEnregVides=0;
                nbBlocsVides = 0;

    	        if(buffer1.metadata.taille_blocs == 1 && buffer1.metadata.taille_enregs == 0){ //si le fichier est vide(suppresion physique)
                    nbBlocsVides = 1;
    	        	break;
				}

				do{  // Parcourir les blocs pour effectuer la recherche
					for( a=0 ; a<buffer1.enregs_utilises; a++){   
    	             	if(buffer1.enregs[a].id == -1){  // Vérifier si le bloc est vide (supprestion logique)
    	        	     	nbEnregVides++;   
				    	}
                        if(buffer1.enregs[a].id == id){ //Si l'ID est trouvé
                            idTrouver = 1;
                            position.deplacement = a;   // Enregistrer l'indice d'enregistrement dans le bloc
                            position.bloc = bloc_courant ;   // Enregistrer le numéro du bloc
                            printf("l'ID trouve : (numéro du bloc %d , déplacement %d) \n", bloc_courant , a);
                            return position;   // Retourner la position trouvée
                        }
					}
					if(a == nbEnregVides){
						nbBlocsVides++;  // Incrémenter le compteur pour les blocs vides
					}	    	         	
                    // Passer au bloc suivant
                    bloc_courant = buffer1.suivant; 
                    if (bloc_courant != -1){
                        memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1
                        fseek(ms,(bloc_courant-1)*sizeof(buffer1),SEEK_SET);
                        fread(&buffer1, sizeof(buffer1),1,ms);
                    }else{
                        break;
                    }

				}while(bloc_courant != -1);	
	        }  
    	}
    }

    if(fichierTrouve == 0){
        printf("Fichier '%s' non trouvé.\n", nom_fichier);
        return position;
    }

    if(taille_blocs == nbBlocsVides){  // Si tous les blocs du fichier sont vides
    	printf("le fichier %s est vide!",nom_fichier);
    	position.deplacement = -10;
    	return position;
	}

	// Si l'ID n'a pas été trouvé
    printf("l’enregistrement recherché n’existe pas dans le fichier %s . \n",nom_fichier );	
    position.deplacement = -3;
	return position;
} 


// Suppression logique : marquer comme supprimé dans une organisation chaînée et ordonnée
void suppressionChaineeOrdonneeLogique(FILE *ms, const char *nom_fichier, int id) {
    printf("Suppression logique de l'enregistrement avec ID %d dans une organisation Chaînée et ORDONNE.\n", id);
    POSITION position;  // Structure pour stocker la position de l'enregistrement à supprimer
    BLOC buffer;
    
	position = rechercherChaineeOrdonnee(ms,nom_fichier,id);  // Recherche de la position de l'enregistrement dans le fichier
   
    if(position.deplacement < 0){ //si l'ID n'a pas été trouvé ou le fichier n'existe pas ou le fichier  est vide
        return;
	}
	
    memset(&buffer, 0, sizeof(BLOC)); // initialiser buffer
	fseek(ms, (position.bloc - 1)* sizeof(BLOC), SEEK_SET);  // Positionnement au bloc contenant l'enregistrement à supprimer
	if (fread(&buffer, sizeof(buffer), 1, ms) != 1) {
        printf("Erreur : Impossible de lire le fichier.\n");
    return; }
    buffer.enregs[position.deplacement].id = -1;  // suppression logique de l'enregistrement en mettant son ID à -1
    
	fseek(ms, (position.bloc - 1)* sizeof(BLOC), SEEK_SET);   // Réécriture du bloc modifié dans le fichier
	if (fwrite(&buffer, sizeof(buffer), 1, ms) != 1) {
        printf("Erreur : Impossible d'ecrire le fichier.\n");
    return; }
    printf("L enregistrement avec ID %d a etait suprrime logiquement avec succés. \n",id);
}


// Suppression physique : réorganiser les blocs pour libérer l'espace dans une organisation chaînée et ordonnée
void suppressionChaineeOrdonneePhysique(FILE *ms, const char *nom_fichier, int id) {
    printf("Suppression physique de l'enregistrement avec ID %d dans une organisation Chaînée et ORDONNE.\n", id);
    
    POSITION position;
    BLOC buffer,buffer2,buffer1;  // Buffers pour lire les blocs du fichier
    
    position = rechercherChaineeOrdonnee(ms,nom_fichier,id);  // Recherche l'enregistrement à supprimer dans le fichier
    
    if(position.deplacement < 0){ //si l'ID n'a pas été trouvé ou le fichier n'existe pas ou le fichier  est vide
        return;
	}

    int tablleAllocation[MAX_BLOCS ] = {0};
    int k,fichierTrouve = 0,tailleBlocFinal,c,blocPresedent;

    rewind(ms);   // Rewind le fichier pour repositionner le curseur au début
    
    memset(&buffer, 0, sizeof(BLOC)); // initialiser buffer
    if (fread(&buffer, sizeof(buffer), 1, ms) != 1) {  // Lecture du premier bloc pour remplire la table d'allocation
        printf("Erreur : Impossible de lire le fichier.\n");
        return; 
    }

    for( k=0; k<(MAX_BLOCS ); k++ ){ // Remplissage du tableau d'allocation 
    	tablleAllocation[k] = buffer.enregs[k].id;
	}
    int i;
	int a = 0,adresse_premier_bloc,taille_blocs = 0;
	k = 0;
	while( k< MAX_BLOCS && fichierTrouve == 0){  // Boucle principale pour trouver la position du fichier qui contien l'enregistrement à supprimer
	    k++;
    	if(tablleAllocation[k] == 1){  // Si le bloc est alloué
            memset(&buffer, 0, sizeof(BLOC)); // Réinitialiser buffer
    	    fseek(ms,(k)*sizeof(buffer),SEEK_SET);  // Positionnement du curseur au début du bloc et lecture de son contenu
    	    fread(&buffer, sizeof(buffer),1,ms);
    	    if(strcmp(buffer.metadata.fichier_nom, nom_fichier ) == 0 && buffer.metadata.adresse_premier == k+1){ // Si le fichier correspond au nom spécifié, on procède à la suppression
				fichierTrouve = 1;
                taille_blocs = buffer.metadata.taille_blocs;
                tailleBlocFinal = taille_blocs;
    	        adresse_premier_bloc = buffer.metadata.adresse_premier;
                blocPresedent == buffer.metadata.adresse_premier;
                
                memset(&buffer, 0, sizeof(BLOC)); // Réinitialiser buffer
				fseek(ms,(position.bloc-1)*sizeof(buffer),SEEK_SET);  // Positionne le curseur au bloc contenant l'enregistrement à supprimer
    	        fread(&buffer, sizeof(buffer),1,ms);
    	        int i=1;
				do{	  
                                  
	                if(buffer.enregs_utilises == 1 && buffer.enregs[0].id == id){ // Si le bloc contient un seul enregistrement , on libère le bloc
	                	if(position.bloc == adresse_premier_bloc){
                            buffer.suivant = -1;
                            buffer.enregs_utilises = 0;
                            buffer.metadata.taille_enregs = 0;
                            fseek(ms,(adresse_premier_bloc-1)*sizeof(buffer),SEEK_SET);
                            fwrite(&buffer, sizeof(buffer),1,ms);
                            printf("le fichier %s se vide complètement apres la suppression physique !!",nom_fichier);
                            return;
                        }else{
                            tablleAllocation[position.bloc - 1] = 0;
                            strcpy(buffer.metadata.fichier_nom, " ");
                            tailleBlocFinal--;
                            fseek(ms,(blocPresedent -1)*sizeof(buffer),SEEK_SET);
                            fread(&buffer, sizeof(buffer),1,ms);
                            buffer.suivant = -1;
                            fseek(ms,(blocPresedent-1)*sizeof(buffer),SEEK_SET);
                            fwrite(&buffer, sizeof(buffer),1,ms);
                            break;
                        }
					}else{ // Décale les enregistrements pour supprimer celui spécifié
						while(position.deplacement < buffer.enregs_utilises ){
							memcpy(&buffer.enregs[position.deplacement], &buffer.enregs[position.deplacement + 1], sizeof(ENREG));
							position.deplacement++;
						}
						buffer.enregs_utilises--; // Diminue le nombre d'enregistrements utilisés
					}
                    blocPresedent = position.bloc;
					// Si le bloc a un suivant, on lit le bloc suivant et déplace ses enregistrements
					if (buffer.suivant != -1) {
                        memset(&buffer2, 0, sizeof(BLOC)); // Réinitialiser buffer2
                        fseek(ms, (buffer.suivant - 1) * sizeof(BLOC), SEEK_SET);
                        fread(&buffer2, sizeof(BLOC), 1, ms);
                        // le premier enregistrement de actul bloc dans le bloc présédent 
                        memcpy(&buffer.enregs[buffer.enregs_utilises -1], &buffer2.enregs[0], sizeof(ENREG));
                        buffer.enregs_utilises++; 
                        position.deplacement = 0;
                        id = buffer2.enregs[0].id;
					}else{
                        fseek(ms,(position.bloc-1)*sizeof(buffer),SEEK_SET); //
					    fwrite(&buffer, sizeof(buffer),1,ms);
                        break;
                    }
					fseek(ms,(position.bloc-1)*sizeof(buffer),SEEK_SET); //
					fwrite(&buffer, sizeof(buffer),1,ms);
					position.bloc = buffer.suivant;
					memcpy(&buffer, &buffer2, sizeof(BLOC));
					i++;	
				} while (i <= taille_blocs);  // Continue jusqu'à la fin de la chaîne
    	    }
        }
     }	

     if(position.deplacement >= 0){ //mettre à jour le méttadonnée de fichier 
     	memset(&buffer, 0, sizeof(BLOC)); // Réinitialiser buffer
        fseek(ms,(adresse_premier_bloc-1)*sizeof(buffer),SEEK_SET); 
     	fread(&buffer, sizeof(buffer),1,ms);
     	buffer.metadata.taille_blocs = tailleBlocFinal;
     	buffer.metadata.taille_enregs--;
     	fseek(ms,(adresse_premier_bloc-1)*sizeof(buffer),SEEK_SET);
		fwrite(&buffer, sizeof(buffer),1,ms);
        printf("L enregistrement avec ID  a etait suprrime physiquement avec succés. \n");
	 }
	
}

// Fonction pour défragmenter un fichier dans une organisation chaînée et ordonnée
void defragmentationChaineeOrdonnee(FILE *ms, const char *nom_fichier) {
    printf("Défragmentation dans une organisation Chaînée et ORDONNE.\n");
    POSITION position;
	position.bloc = -2;
	position.deplacement = -1;
    BLOC buffer1;
    int k,taille_blocs = 0,fichierTrouve = 0;
    int tablleAllocation[MAX_BLOCS ] = {0};
    
    rewind(ms);  // Repositionne le curseur au début du fichier
    if (fread(&buffer1, sizeof(buffer1), 1, ms) != 1) {    // Lecture du la table d'allocation
        printf("Erreur : Impossible de lire le fichier.\n");
        return; 
    }
    
    for(k=0; k<(MAX_BLOCS ); k++ ){
    	tablleAllocation[k] = buffer1.enregs[k].id;
	}

	k = 0;
	 while(k<MAX_BLOCS && fichierTrouve == 0){   // Boucle principale pour parcourir les blocs jusqu'à le fichier trouve ou la fin de MS
	    k++;
    	if(tablleAllocation[k] == 1){
            memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1
    	    fseek(ms,(k)*sizeof(buffer1),SEEK_SET);
    	    fread(&buffer1, sizeof(buffer1),1,ms);
    	    if(strcmp(buffer1.metadata.fichier_nom, nom_fichier ) == 0 && buffer1.metadata.adresse_premier == k+1){
    	        fichierTrouve = 1;
                taille_blocs = buffer1.metadata.taille_blocs;
                int c = 1; 
    	        do{
                    for( int i=0 ; i<buffer1.enregs_utilises; i++){   // Parcours les bloc du fichier jusqu'à derinier bloc
                        if(buffer1.enregs[i].id == -1){ //Si on trouve un enregistrement marqué comme supprimé logiquement,
                            suppressionChaineeOrdonneePhysique(ms,nom_fichier,-1); // Suppression physique de l'enregistrement
                        }
				    }
                    // Passe au bloc suivant dans le fichier
                    if(buffer1.suivant != -1){
                        fseek(ms,(buffer1.suivant-1)*sizeof(buffer1),SEEK_SET);
                        memset(&buffer1, 0, sizeof(BLOC)); // Réinitialiser buffer1
                        fread(&buffer1, sizeof(buffer1),1,ms);
                        c++;
                    }
                }while(c <= taille_blocs);
			}  
    	}
    }
    // Si le fichier non  trouvé
    if(fichierTrouve == 0){
       printf("Fichier '%s' non trouvé.\n", nom_fichier);
	}else{
        printf("Défragmentation terminée avec succès.\n");
    }
}

    
