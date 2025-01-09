#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_BLOCS 15
#define FACTEUR_BLOCS MAX_BLOCS  // Facteur de blocage égal au nombre de blocs

// Structure d'un enregistrement
typedef struct ENREG {
    int id;            // Indique si le bloc est occupé (1) ou libre (0)
    char nom[50];      // Contient le numéro du bloc
} ENREG;

// Structure des métadonnées d'un fichier
typedef struct METADATA {
    char fichier_nom[50];     // Nom du fichier
    int taille_blocs;         // Nombre de blocs utilisés dans le fichier
    int taille_enregs;        // Nombre d'enregistrements
    int adresse_premier;      // Adresse du premier bloc
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

typedef struct POSITION {
    int bloc;
    int deplacement;  // Assurez-vous que ce membre est défini correctement
} POSITION;

void afficherEtatTableAllocation(FILE* ms);
void afficherEtatMemoireSecondaire(FILE *ms);
void miseAJourTableAllocation(FILE *ms, int bloc, int etat);
void marquerBlocCommeOccupe(FILE* ms, int indiceBloc);
void marquerBlocCommeLibre(FILE *ms, int indiceBloc);
int verifierEspaceContigu(int blocsNecessaires, FILE *ms);
int verifierEspaceChainee(int blocsNecessaires, FILE *ms);
int fichierExiste(FILE *ms, const char *nom_fichier);
void creationEtChargementFichier(FILE *ms);
void chargerContiguOrdonne(FILE *ms, METADATA *metadata);
void chargerContiguNonOrdonne(FILE *ms, METADATA *metadata);
void chargerChaineOrdonne(FILE *ms, METADATA *metadata);
void chargerChaineNonOrdonne(FILE *ms, METADATA *metadata);
void mettreAJourMetadonnees(FILE *ms, const METADATA *metadata, int blocIndex);
void lireTableAllocation(FILE *ms, BLOC *buffer);

void lireTableAllocation(FILE *ms, BLOC *buffer);
void insertionContigueOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg);
void insertionContigueNonOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg);
void insertionChaineeOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg);
void insertionChaineeNonOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg);
int verifierIDUnique(FILE *ms, const char *nom_fichier, int id);
void gererInsertion(FILE *ms, METADATA* meta, const char *nom_fichier);
POSITION rechercherContigueOrdonnee(FILE *ms, const char *nom_fichier, int id);
POSITION rechercherContigueNonOrdonnee(FILE *ms, const char *nom_fichier, int id);
POSITION rechercherChaineeOrdonnee(FILE *ms, const char *nom_fichier, int id);
POSITION rechercherChaineeNonOrdonnee(FILE *ms, const char *nom_fichier, int id);
POSITION gererRecherche(FILE *ms, METADATA* meta, const char *nom_fichier, int id);


void suppressionContigueOrdonneeLogique(FILE *ms, const char *nom_fichier, int id);
void suppressionContigueOrdonneePhysique(FILE *ms, const char *nom_fichier, int id);
void suppressionContigueNonOrdonneeLogique(FILE *ms, const char *nom_fichier, int id);
void suppressionContigueNonOrdonneePhysique(FILE *ms, const char *nom_fichier, int id);
void suppressionChaineeOrdonneeLogique(FILE *ms, const char *nom_fichier, int id);
void suppressionChaineeOrdonneePhysique(FILE *ms, const char *nom_fichier, int id);
void suppressionChaineeNonOrdonneeLogique(FILE *ms, const char *nom_fichier, int id);
void suppressionChaineeNonOrdonneePhysique(FILE *ms, const char *nom_fichier, int id);

void defragmentationChaineeOrdonnee(FILE *ms, const char *nom_fichier);
void defragmentationChaineeNonOrdonnee(FILE *ms, const char *nom_fichier);
void defragmentationContigueNonOrdonnee(FILE *ms, const char *nom_fichier) ;

void defragmentationContigueOrdonnee(FILE *ms, const char *nom_fichier);
void gererSuppression(FILE *ms, const char *nom_fichier, int id, int type_suppression, const char *org_globale, const char *org_interne);
void compactageMemoire(FILE *ms);

void gererDefragmentation(FILE *ms, const char *nom_fichier, const char *org_globale, const char *org_interne);
void initialiserMemoireSecondaire(FILE* ms) ;
void afficherMemoire(FILE *ms, METADATA *meta, const char *nom_fichier);
void supprimerFichier(FILE *ms, const char *nom_du_fichier);

void defragmenterMemoire(FILE *ms, METADATA *meta);

void lireFichier(FILE *ms, METADATA *meta, const char *nom_fichier, int id);
void affichageBlocs(FILE *ms) ;
 int nombredeblocsLibres(FILE *f);
void afficherMetadonnees(FILE *ms);
void viderMemoireSecondaire(FILE *ms);
void initialiserMemoire(FILE *ms, METADATA *meta);

void renommerFichier(FILE *ms, const char *ancien_nom, const char *nouveau_nom) ;
void afficherBienvenue();
void afficherMenu();



int main() {
    FILE *ms;
    ms = fopen("memoire_secondaire.bin", "wb+");
    if (!ms) {
        perror("Échec de l'ouverture du fichier de mémoire secondaire");
        return EXIT_FAILURE;
    }
    METADATA meta;
    // initialiserMemoire(ms, &meta);

    // Afficher le message de bienvenue
    afficherBienvenue();
    initialiserMemoireSecondaire(ms);
    int choix;
    char nom_fichier[50];
    ENREG nouvel_enreg;
    int id;
    int type_suppression;

    do {
        // Afficher le menu
        afficherMenu();

        printf("Choisissez une option: ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                creationEtChargementFichier(ms);
                break;
            case 2:
              afficherMetadonnees(ms);
                break;
            case 3:
                printf("Entrez le nom du fichier: ");
                scanf("%s", nom_fichier);
                printf("Entrez l'ID de l'enregistrement: ");
                scanf("%d", &id);
                gererRecherche(ms, &meta, nom_fichier, id);
                break;
            case 4:
                printf("Entrez le nom du fichier: ");
                scanf("%s", nom_fichier);
                printf("Entrez l'ID du nouvel enregistrement: ");
                scanf("%d", &nouvel_enreg.id);
                printf("Entrez le nom du nouvel enregistrement: ");
                scanf("%s", nouvel_enreg.nom);
                gererInsertion(ms, &meta, nom_fichier);
                break;
            case 5:
                printf("Entrez le nom du fichier: ");
                scanf("%s", nom_fichier);
                printf("Entrez l'ID de l'enregistrement à supprimer: ");
                scanf("%d", &id);
                printf("Type de suppression (1 pour logique, 2 pour physique): ");
                scanf("%d", &type_suppression);
                gererSuppression(ms, nom_fichier, id, type_suppression, meta.org_globale, meta.org_interne);
                break;
            case 6:
                printf("Entrez le nom du fichier: ");
                scanf("%s", nom_fichier);
                printf("Entrez l'organisation globale : ");
                char org_globale[30];
                scanf("%s", org_globale);
                printf("Entrez l'organisation interne : ");
                char org_interne[30];
                scanf("%s", org_interne);
                gererDefragmentation(ms, nom_fichier, org_globale, org_interne);
                break;
            case 7:
                printf("Entrez le nom du fichier: ");
                scanf("%s", nom_fichier);
                supprimerFichier(ms, nom_fichier);
                break;
            case 8:
                printf("Entrez le nom du fichier à renommer: ");
                scanf("%s", nom_fichier);
                printf("Entrez le nouveau nom: ");
                char nouveau_nom[50];
                scanf("%s", nouveau_nom);
                renommerFichier(ms, nom_fichier, nouveau_nom);
                break;
            case 9:
                compactageMemoire(ms);
                break;
            case 10:
                viderMemoireSecondaire(ms);
                break;
            case 11:
                printf("+------------------------------------------------------------+\n");
                printf("|          Merci d'avoir utilisé le SGF. Au revoir !         |\n");
                printf("+------------------------------------------------------------+\n");
                break;
            default:
                printf("Option non valide. Veuillez réessayer.\n");
        }
    } while (choix != 11);

    fclose(ms);
    return 0;
}



void initialiserMemoireSecondaire(FILE* ms) {
    if (ms == NULL) {
        printf("Erreur : Impossible d'ouvrir 'memoiresecondaire.dat'.\n");
        return;
    }

    printf("Initialisation de la mémoire secondaire...\n");

    BLOC buffer;
    memset(&buffer, 0, sizeof(BLOC));  // Initialiser le buffer à zéro

    // Initialiser la table d'allocation dans le bloc 0
    buffer.enregs[0].id = 1; // Marquer le premier bloc comme occupé (table d'allocation)
    sprintf(buffer.enregs[0].nom, "Bloc%d", 0);

    for (int i = 1; i < FACTEUR_BLOCS; i++) {
        buffer.enregs[i].id = 0; // Marquer les blocs comme libres
        memset(buffer.enregs[i].nom, ' ', sizeof(buffer.enregs[i].nom) - 1); // Initialiser à des espaces
        buffer.enregs[i].nom[sizeof(buffer.enregs[i].nom) - 1] = '\0'; // Terminer la chaîne avec un NULL
    }

    // Initialiser les métadonnées avec des espaces pour le nom du fichier
    memset(buffer.metadata.fichier_nom, ' ', sizeof(buffer.metadata.fichier_nom) - 1);
    buffer.metadata.fichier_nom[sizeof(buffer.metadata.fichier_nom) - 1] = '\0'; // Terminer la chaîne avec un NULL

    buffer.metadata.taille_blocs = 1;  // Indiquer que ce bloc contient la table d'allocation
    buffer.metadata.taille_enregs = 0;
    buffer.metadata.adresse_premier = 0;
    strcpy(buffer.metadata.org_globale, "N/A");
    strcpy(buffer.metadata.org_interne, "N/A");

    buffer.enregs_utilises = FACTEUR_BLOCS;
    buffer.suivant = -1;  // Pas de bloc suivant

    // Écrire le buffer dans le fichier
    rewind(ms);  // S'assurer de commencer à écrire au début
    if (fwrite(&buffer, sizeof(buffer), 1, ms) != 1) {
        printf("Erreur : Impossible d'écrire dans 'memoiresecondaire.dat'.\n");
    } else {
        printf("Mémoire secondaire initialisée avec succès.\n");
    }
}






// Fonction pour afficher l'état de la table d'allocation
void afficherEtatTableAllocation(FILE* ms) {
    BLOC buffer;
    lireTableAllocation(ms, &buffer);

    printf("Table d'allocation des blocs :\n");
    for (int i = 0; i < FACTEUR_BLOCS; i++) {
        printf("Bloc %d : %s (Etat : %s)\n", i, buffer.enregs[i].nom, 
               (buffer.enregs[i].id == 0) ? "Libre" : "Occupé");
    }
}
void afficherMenu() {
    printf("\n+------------------------------------------------------------+\n");
    printf("|                           MENU                              |\n");
   printf("\n+------------------------------------------------------------+\n");
  
    printf("|   1. Créer et charger un fichier                            |\n");
    printf("|   2. Afficher les métadonnées des fichiers                  |\n");
    printf("|   3. Rechercher un enregistrement par ID                    |\n");
    printf("|   4. Insérer un nouvel enregistrement                       |\n");
    printf("|   5. Supprimer un enregistrement                            |\n");
    printf("|   6. Défragmenter un fichier                                |\n");
    printf("|   7. Supprimer un fichier                                   |\n");
    printf("|   8. Renommer un fichier                                    |\n");
    printf("|   9. Compactage de la mémoire secondaire                    |\n");
    printf("|   10. Vider la mémoire secondaire                           |\n");
    printf("|   11. Quitter                                               |\n");
    printf("+------------------------------------------------------------+\n");
}
void afficherBienvenue() {
    printf("+------------------------------------------------------------+\n");
    printf("|                                                            |\n");
    printf("|        BIENVENUE DANS LE Simulateur Simplifié d’un         |\n");
    printf("|            Système de Gestion de Fichiers (SGF)            |\n");
    printf("|                                                            |\n");
    printf("+------------------------------------------------------------+\n");
}


// Fonction pour afficher l'état de la mémoire secondaire
void afficherEtatMemoireSecondaire(FILE *ms) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de mémoire secondaire est invalide.\n");
        return;
    }

    printf("Affichage de l'état de la mémoire secondaire :\n");

    BLOC buffer;
    fseek(ms, 0, SEEK_SET);  // S'assurer de commencer à lire au début
    for (int i = 0; i < MAX_BLOCS; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", i);
            continue;
        }

        if (buffer.enregs_utilises == 0) {
            printf("Bloc %d: Libre\n", i);
        } else {
            printf("Bloc %d: Occupé par '%s', %d enregistrements\n", i, buffer.metadata.fichier_nom, buffer.enregs_utilises);
        }
    }

    int nbBlocsLibres = nombredeblocsLibres(ms);
    printf("Nombre de blocs libres : %d\n", nbBlocsLibres);
}


void miseAJourTableAllocation(FILE *ms, int bloc, int etat) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    BLOC table_allocation;

    // Lire la table d'allocation depuis le premier bloc
    rewind(ms);
    if (fread(&table_allocation, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Lecture de la table d'allocation a échoué.\n");
        return;
    }

    // Mettre à jour l'état du bloc spécifié
    table_allocation.enregs[bloc].id = etat;

    // Réécrire la table d'allocation mise à jour dans le fichier
    rewind(ms);
    if (fwrite(&table_allocation, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Écriture de la table d'allocation mise à jour a échoué.\n");
        return;
    }

    printf("Mise à jour de l'état du bloc %d à %d réussie.\n", bloc, etat);
}


// Fonction pour marquer un bloc comme occupé
void marquerBlocCommeOccupe(FILE* ms, int indiceBloc) {
    BLOC buffer;
    lireTableAllocation(ms, &buffer);  // Lire la table d'allocation

    if (indiceBloc >= 0 && indiceBloc < FACTEUR_BLOCS) {
        buffer.enregs[indiceBloc].id = 1;  // Marquer le bloc comme occupé
        printf("Le bloc %d est maintenant occupé.\n", indiceBloc);

        fseek(ms, 0, SEEK_SET);  // Revenir au début du fichier
        fwrite(&buffer, sizeof(BLOC), 1, ms);  // Sauvegarder les modifications
    } else {
        printf("Erreur : Indice de bloc invalide.\n");
    }
}

// Fonction pour marquer un bloc comme libre
void marquerBlocCommeLibre(FILE* ms, int indiceBloc) {
    BLOC buffer;
    lireTableAllocation(ms, &buffer);  // Lire la table d'allocation

    if (indiceBloc >= 0 && indiceBloc < FACTEUR_BLOCS) {
        buffer.enregs[indiceBloc].id = 0;  // Marquer le bloc comme libre
        printf("Le bloc %d est maintenant libre.\n", indiceBloc);

        fseek(ms, 0, SEEK_SET);  // Revenir au début du fichier
        fwrite(&buffer, sizeof(BLOC), 1, ms);  // Sauvegarder les modifications
    } else {
        printf("Erreur : Indice de bloc invalide.\n");
    }
}
// Fonction pour vérifier l'espace disponible pour l'organisation contiguë
int verifierEspaceContigu(int blocsNecessaires, FILE *ms) {
    BLOC buffer;
    lireTableAllocation(ms, &buffer);
    for (int i = 1; i <= MAX_BLOCS - blocsNecessaires; i++) {
        int contigusLibres = 1;
        for (int j = 0; j < blocsNecessaires; j++) {
            if (buffer.enregs[i + j].id == 1) { // Si le bloc est occupé
                contigusLibres = 0;
                break;
            }
        }
        if (contigusLibres) {
            return 1;  // Espace contigu disponible
        }
    }
    return 0;  // Pas d'espace contigu disponible
}


// Fonction pour vérifier s'il y a suffisamment d'espace pour l'organisation chaînée
int verifierEspaceChainee(int blocsNecessaires, FILE *ms) {
    BLOC buffer;
    lireTableAllocation(ms, &buffer);
    int blocsLibres = 0;

    // Parcourir la table d'allocation pour compter le nombre de blocs libres
    for (int i = 1; i < MAX_BLOCS; i++) {
        if (buffer.enregs[i].id == 0) { // Si le bloc est libre
            blocsLibres++;
        }
    }

    // Vérifier si le nombre de blocs libres est suffisant
    return blocsLibres >= blocsNecessaires ? 1 : 0;  // 1 si suffisamment d'espace est disponible, sinon 0
}


// Fonction pour vérifier si un fichier existe déjà
int fichierExiste(FILE *ms, const char *nom_fichier) {
    if (ms == NULL) {
        return 0; // Le fichier de stockage est invalide
    }

    BLOC buffer;
    fseek(ms, sizeof(BLOC), SEEK_SET); // Positionner après la table d'allocation

    // Parcourir tous les blocs pour vérifier les métadonnées
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            if (buffer.metadata.taille_enregs > 0) {
                return 1; // Le fichier existe et n'est pas vide
            } else {
                return 2; // Le fichier existe mais est vide
            }
        }
    }

    return 0; // Le fichier n'existe pas
}

void creationEtChargementFichier(FILE *ms) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    METADATA metadata;
    int nombre_enregistrements, nombre_blocs;

    // Demander le nom du fichier
    printf("Entrez le nom du fichier : ");
    scanf("%s", metadata.fichier_nom);

    // Vérifier si le fichier existe déjà
    int etat_fichier = fichierExiste(ms, metadata.fichier_nom);
    if (etat_fichier == 1) {
        printf("Erreur : Le fichier '%s' existe déjà et n'est pas vide.\n", metadata.fichier_nom);
        return;
    } else if (etat_fichier == 2) {
        printf("Erreur : Le fichier '%s' existe déjà et est vide.\n", metadata.fichier_nom);
    }

    // Demander le nombre d'enregistrements
    printf("Entrez le nombre d'enregistrements : ");
    scanf("%d", &nombre_enregistrements);

    // Calculer le nombre de blocs nécessaires
    nombre_blocs = (nombre_enregistrements + FACTEUR_BLOCS - 1) / FACTEUR_BLOCS;
    metadata.taille_blocs = nombre_blocs;
    metadata.taille_enregs = nombre_enregistrements;

    // Demander le mode d'organisation globale
    printf("Choisissez l'organisation globale :\n");
    printf("1. Contigu\n");
    printf("2. Chaîné\n");
    int choixGlobale;
    scanf("%d", &choixGlobale);

    // Demander le mode d'organisation interne
    printf("Choisissez l'organisation interne :\n");
    printf("1. ORDONNE\n");
    printf("2. NONORDONNE\n");
    int choixInterne;
    scanf("%d", &choixInterne);

    strcpy(metadata.org_interne, (choixInterne == 1) ? "ORDONNE" : "NONORDONNE");

    // Traiter les différents cas d'organisation
    int espaceSuffisant = 0;
    if (choixGlobale == 1) {
        strcpy(metadata.org_globale, "contigu");
        espaceSuffisant = verifierEspaceContigu(nombre_blocs, ms);
    } else if (choixGlobale == 2) {
        strcpy(metadata.org_globale, "chaîné");
        espaceSuffisant = verifierEspaceChainee(nombre_blocs, ms);
    } else {
        printf("Erreur : Organisation globale invalide.\n");
        return;
    }

    if (!espaceSuffisant) {
        int choix;
        printf("Pas assez d'espace pour créer le fichier. Choisissez une option :\n");
        printf("1. Compactage de la mémoire\n");
        printf("2. Annuler la création du fichier\n");
        scanf("%d", &choix);

        if (choix == 1) {
            compactageMemoire(ms);
            espaceSuffisant = (choixGlobale == 1) ? verifierEspaceContigu(nombre_blocs, ms) : verifierEspaceChainee(nombre_blocs, ms);
            if (!espaceSuffisant) {
                printf("Erreur : Pas assez d'espace même après le compactage.\n");
                return;
            }
        } else {
            printf("Création du fichier annulée.\n");
            return;
        }
    }

    if (choixGlobale == 1) {
        if (choixInterne == 1) {
            chargerContiguOrdonne(ms, &metadata);
        } else {
            chargerContiguNonOrdonne(ms, &metadata);
        }
    } else if (choixGlobale == 2) {
        if (choixInterne == 1) {
            chargerChaineOrdonne(ms, &metadata);
        } else {
            chargerChaineNonOrdonne(ms, &metadata);
        }
    }

    printf("Fichier '%s' créé avec succès.\n", metadata.fichier_nom);
}

void chargerContiguOrdonne(FILE *ms, METADATA *metadata) { // Impl�mentation de la fonction

 BLOC table_allocation;
    BLOC bloc_alloue;
    int enreg_restants = metadata->taille_enregs;
    int nombre_blocs = metadata->taille_blocs;

    // Lire la table d'allocation une seule fois
    lireTableAllocation(ms, &table_allocation);

    // Trouver un espace contigu libre
    int debut_bloc = -1;
    for (int i = 1; i <= MAX_BLOCS - nombre_blocs; i++) {
        int espace_libre = 1;
        for (int j = 0; j < nombre_blocs; j++) {
            if (table_allocation.enregs[i + j].id == 1) {
                espace_libre = 0;
                break;
            }
        }
        if (espace_libre) {
            debut_bloc = i;
            break;
        }
    }

    if (debut_bloc == -1) {
        printf("Erreur : Pas d'espace contigu disponible pour allouer %d blocs.\n", nombre_blocs);
        return;
    }

    // Marquer les blocs comme occup�s
    for (int i = 0; i < nombre_blocs; i++) {
        table_allocation.enregs[debut_bloc + i].id = 1;
        sprintf(table_allocation.enregs[debut_bloc + i].nom, "Bloc_%d", debut_bloc + i);
    }

    // �crire la table d'allocation mise � jour
    rewind(ms);
    fwrite(&table_allocation, sizeof(BLOC), 1, ms);

    // Allouer les blocs
    for (int i = 0; i < nombre_blocs; i++) {
        int current_bloc = debut_bloc + i;

        // Initialiser le bloc
        memset(&bloc_alloue, 0, sizeof(BLOC));

        // Remplir le bloc
        bloc_alloue.enregs_utilises = 0;
        int enregs_dans_bloc = (enreg_restants > FACTEUR_BLOCS) ? FACTEUR_BLOCS : enreg_restants;

        for (int j = 0; j < enregs_dans_bloc; j++) {
            bloc_alloue.enregs[j].id = rand() % 1000;
            sprintf(bloc_alloue.enregs[j].nom, "Enreg_%d", rand() % 1000);
            bloc_alloue.enregs_utilises++;
        }

        enreg_restants -= enregs_dans_bloc;

        // Mettre � jour les m�tadonn�es
        if (i == 0) {
            bloc_alloue.metadata = *metadata;
        } else {
            strcpy(bloc_alloue.metadata.fichier_nom, metadata->fichier_nom);
        }

        // �crire le bloc
        fseek(ms, current_bloc * sizeof(BLOC), SEEK_SET);
        if (fwrite(&bloc_alloue, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur d'�criture du bloc %d\n", current_bloc);
            return;
        }
    }

    printf("Chargement des blocs contigus ordonn�s effectu� avec succ�s.\n");
    fflush(stdout); // Pour s'assurer que le message est affich�

 }
  void chargerChaineOrdonne(FILE *ms, METADATA *metadata) { // Implémentation de la fonction
   } 
   void chargerChaineNonOrdonne(FILE *ms, METADATA *metadata) {}
// Fonction pour mettre à jour les métadonnées dans le bloc de mémoire secondaire

void mettreAJourMetadonnees(FILE *ms, const METADATA *metadata, int blocIndex) {
    if (ms == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier de mémoire secondaire.\n");
        return;
    }

    // Calculer la position du bloc dans le fichier
    long position = blocIndex * sizeof(BLOC);

    // Déplacer le pointeur du fichier à la position du bloc
    fseek(ms, position, SEEK_SET);

    // Lire le bloc existant
    BLOC buffer;
    fread(&buffer, sizeof(BLOC), 1, ms);

    // Mettre à jour les métadonnées dans le bloc
    buffer.metadata = *metadata;

    // Revenir à la position du bloc et écrire les métadonnées mises à jour
    fseek(ms, position, SEEK_SET);
    if (fwrite(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Impossible de mettre à jour les métadonnées dans le fichier.\n");
    } else {
        printf("Les métadonnées ont été mises à jour avec succès dans le bloc %d.\n", blocIndex);
    }
}





void lireTableAllocation(FILE *ms, BLOC *buffer) {
    rewind(ms); // Replacer le pointeur de fichier au début
    if (fread(buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Lecture de la table d'allocation a échoué.\n");
    }
}





void insertionContigueOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg) {
    printf("Insertion contigu� ordonn�e\n");
    printf("Nom du fichier : %s\n", nom_fichier);
    printf("Nouvel enregistrement : [ID: %d, Nom: %s, ]\n", nouvel_enreg.id, nouvel_enreg.nom);

     if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    BLOC table_allocation;
    BLOC buffer;
    int adresse_premier_bloc = -1;
    int taille_fichier = 0;
    int fichier_trouve = 0;

    // Lire la table d'allocation depuis le premier bloc
    rewind(ms);
    fread(&table_allocation, sizeof(BLOC), 1, ms);

    // Recherche du fichier dans le fichier de stockage
    fseek(ms, sizeof(BLOC), SEEK_SET);  // Positionner au d�but apr�s la table d'allocation
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            fichier_trouve = 1;
            adresse_premier_bloc = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            break;  // Fichier trouv�, sortir de la boucle
        }
    }

    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouv�.\n", nom_fichier);
        return;
    }

    // V�rifier si l'ID est unique
    POSITION pos = rechercherContigueOrdonnee(ms, nom_fichier, nouvel_enreg.id);
    if (pos.bloc != -1) {
        printf("Erreur : L'ID %d existe d�j�.\n", nouvel_enreg.id);
        return;  // ID trouv�, donc non unique
    }

    // R�initialiser la position pour l'insertion
    fseek(ms, adresse_premier_bloc * (long)sizeof(BLOC), SEEK_SET);
    for (int i = 0; i < taille_fichier; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : Lecture du bloc %d a �chou�.\n", i);
            return;  // Erreur de lecture du bloc
        }

        // Chercher un emplacement libre dans le bloc
        if (buffer.enregs_utilises < FACTEUR_BLOCS) {
            // Trouver la position d'insertion
            int j;
            for (j = 0; j < buffer.enregs_utilises; j++) {
                if (buffer.enregs[j].id > nouvel_enreg.id) {
                    break;  // Trouver la position d'insertion
                }
            }

            // D�caler les enregistrements pour faire de la place
            for (int k = buffer.enregs_utilises; k > j; k--) {
                buffer.enregs[k] = buffer.enregs[k - 1];
            }

            // Ins�rer le nouvel enregistrement
            buffer.enregs[j] = nouvel_enreg;
            buffer.enregs_utilises++;
            fseek(ms, -((long)sizeof(BLOC)), SEEK_CUR);  // Revenir en arri�re pour r��crire le bloc
            fwrite(&buffer, sizeof(BLOC), 1, ms);  // �crire le bloc mis � jour
            printf("Enregistrement ins�r� avec succ�s : Bloc = %d, Index = %d\n", i, j);

            // Mettre � jour les m�tadonn�es du premier bloc du fichier
            if (i == 0) {
                buffer.metadata.taille_enregs++;
                fseek(ms, adresse_premier_bloc * (long)sizeof(BLOC), SEEK_SET);
                fwrite(&buffer, sizeof(BLOC), 1, ms);
            }

            return;  // Enregistrement ins�r� avec succ�s
        }
    }

    // Si aucun espace n'est disponible pour allouer un nouveau bloc
    int bloc_alloue = -1;
    for (int i = 1; i < MAX_BLOCS; i++) {
        if (table_allocation.enregs[i].id == 0) { // Si le bloc est libre
            table_allocation.enregs[i].id = 1; // Marquer le bloc comme occup�
            bloc_alloue = i;
            break;
        }
    }

    if (bloc_alloue == -1) {
        int choix;
        printf("Pas de blocs disponibles pour allouer un nouveau bloc. Choisissez une option :\n");
        printf("1. Compactage de la m�moire\n");
        printf("2. Annuler l'insertion\n");
        scanf("%d", &choix);

        if (choix == 1) {
            compactageMemoire(ms);
            for (int i = 1; i < MAX_BLOCS; i++) {
                if (table_allocation.enregs[i].id == 0) { // Si le bloc est libre apr�s compactage
                    table_allocation.enregs[i].id = 1; // Marquer le bloc comme occup�
                    bloc_alloue = i;
                    break;
                }
            }
            if (bloc_alloue == -1) {
                printf("Erreur : Pas de blocs disponibles m�me apr�s compactage.\n");
                return;
            }
        } else {
            printf("Insertion annul�e.\n");
            return;
        }
    }

    // Initialiser le nouveau bloc
    BLOC nouveau_bloc = {0}; // R�initialiser tous les champs � 0
    nouveau_bloc.enregs[0] = nouvel_enreg;
    nouveau_bloc.enregs_utilises = 1;
    strcpy(nouveau_bloc.metadata.fichier_nom, nom_fichier);

    // �crire le nouveau bloc dans la m�moire secondaire
    fseek(ms, bloc_alloue * (long)sizeof(BLOC), SEEK_SET);
    fwrite(&nouveau_bloc, sizeof(BLOC), 1, ms);

    // Mettre � jour la table d'allocation
    rewind(ms);
    fwrite(&table_allocation, sizeof(BLOC), 1, ms);

    printf("Enregistrement ins�r� avec succ�s dans un nouveau bloc : Bloc = %d, Index = 0\n", bloc_alloue);

    // Mettre � jour les m�tadonn�es du premier bloc du fichier
    fseek(ms, adresse_premier_bloc * (long)sizeof(BLOC), SEEK_SET);
    fread(&buffer, sizeof(BLOC), 1, ms);
    buffer.metadata.taille_enregs++;
    fseek(ms, adresse_premier_bloc * (long)sizeof(BLOC), SEEK_SET);
    fwrite(&buffer, sizeof(BLOC), 1, ms);

    printf("Insertion termin�e dans l'ordre contigu.\n");
}


void insertionChaineeOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg) {
    printf("Insertion chaîne ordonnée\n");
    printf("Nom du fichier : %s\n", nom_fichier);
    
    
    // Logique d'insertion ici...
    
    printf("Insertion terminée dans l'ordre chaîne ordonnée.\n");
}

void insertionChaineeNonOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg) {
    printf("Insertion chaîne non ordonnée\n");
    printf("Nom du fichier : %s\n", nom_fichier);
  
    // Logique d'insertion ici...
    
    printf("Insertion terminée dans l'ordre chaîne non ordonnée.\n");
}


int verifierIDUnique(FILE *ms, const char *nom_fichier, int id) {
    BLOC buffer;
    int adresse_premier_bloc = -1;
    int taille_fichier = 0;
    int fichier_trouve = 0;

    // Recherche du fichier dans le fichier de stockage
    fseek(ms, 0, SEEK_SET);  // Positionner au début du fichier
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            fichier_trouve = 1;
            adresse_premier_bloc = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            break;
        }
    }

    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouvé.\n", nom_fichier);
        return 0;  // Fichier non trouvé
    }

    // Parcourir les blocs contigus pour vérifier l'unicité de l'ID
    fseek(ms, adresse_premier_bloc * sizeof(BLOC), SEEK_SET);
    for (int i = 0; i < taille_fichier; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : Lecture du bloc %d a échoué.\n", i);
            return 0;
        }

        for (int j = 0; j < buffer.enregs_utilises; j++) {
            if (buffer.enregs[j].id == id) {
                printf("Erreur : L'ID %d existe déjà.\n", id);
                return 0;  // ID trouvé, donc non unique
            }
        }
    }

    return 1;  // ID unique
}

void gererInsertion(FILE *ms, METADATA* meta, const char *nom_fichier) {
    ENREG nouvel_enreg;

    // Demander à l'utilisateur d'entrer l'ID et le nom du nouvel enregistrement
    printf("Entrez l'ID du nouvel enregistrement : ");
    scanf("%d", &nouvel_enreg.id);
    
    // Valider si l'ID est unique
    if (!verifierIDUnique(ms, nom_fichier, nouvel_enreg.id)) {
        printf("Insertion annulée : ID non unique.\n");
        return;  // Annuler l'insertion si l'ID n'est pas unique
    }

    printf("Entrez le nom du nouvel enregistrement : ");
    scanf("%s", nouvel_enreg.nom);

    // Vérification de l'organisation globale et interne et appel de la fonction d'insertion appropriée
    if (strcmp(meta->org_globale, "contigu") == 0) {
        if (strcmp(meta->org_interne, "ORDONNE") == 0) {
            insertionContigueOrdonnee(ms, nom_fichier, nouvel_enreg);
        } else if (strcmp(meta->org_interne, "NONORDONNE") == 0) {
            insertionContigueNonOrdonnee(ms, nom_fichier, nouvel_enreg);
        }
    } else if (strcmp(meta->org_globale, "chaîné") == 0) {
        if (strcmp(meta->org_interne, "ORDONNE") == 0) {
            insertionChaineeOrdonnee(ms, nom_fichier, nouvel_enreg);
        } else if (strcmp(meta->org_interne, "NONORDONNE") == 0) {
            insertionChaineeNonOrdonnee(ms, nom_fichier, nouvel_enreg);
        }
    } else {
        printf("Erreur : type d'organisation inconnu.\n");
    }
}

POSITION rechercherContigueOrdonnee(FILE *ms, const char *nom_fichier, int id) {
    POSITION pos = { -1, -1 };  // Valeur par défaut indiquant que l'enregistrement n'a pas été trouvé
    // Implémentation de la recherche...
    return pos;
}


POSITION rechercherChaineeOrdonnee(FILE *ms, const char *nom_fichier, int id) {
    POSITION pos = { -1, -1 };  // Valeur par défaut indiquant que l'enregistrement n'a pas été trouvé
    // Implémentation de la recherche...
    return pos;
}
POSITION rechercherChaineeNonOrdonnee(FILE *ms, const char *nom_fichier, int id) {
    POSITION pos = { -1, -1 };  // Valeur par défaut indiquant que l'enregistrement n'a pas été trouvé
    // Implémentation de la recherche...
    return pos;
}

POSITION gererRecherche(FILE *ms, METADATA* meta, const char *nom_fichier, int id) {
    POSITION pos;
    pos.bloc = -1;  // Valeur par défaut indiquant que l'enregistrement n'a pas été trouvé
    pos.deplacement = -1;

    // Vérification de l'organisation globale et interne et appel de la fonction de recherche appropriée
    if (strcmp(meta->org_globale, "contigu") == 0) {
        if (strcmp(meta->org_interne, "ORDONNE") == 0) {
            pos = rechercherContigueOrdonnee(ms, nom_fichier, id);
        } else if (strcmp(meta->org_interne, "NONORDONNE") == 0) {
            pos = rechercherContigueNonOrdonnee(ms, nom_fichier, id);
        }
    } else if (strcmp(meta->org_globale, "chaîné") == 0) {
        if (strcmp(meta->org_interne, "ORDONNE") == 0) {
            pos = rechercherChaineeOrdonnee(ms, nom_fichier, id);
        } else if (strcmp(meta->org_interne, "NONORDONNE") == 0) {
            pos = rechercherChaineeNonOrdonnee(ms, nom_fichier, id);
        }
    } else {
        printf("Erreur : type d'organisation inconnu.\n");
    }

    return pos;
}

// Suppression logique : marquer comme supprim� dans une organisation contigu� et ordonn�e
void suppressionContigueOrdonneeLogique(FILE *ms, const char *nom_fichier, int id) {
    // V�rifier si le pointeur du fichier est NULL
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    // Appeler la fonction de recherche pour trouver la position de l'enregistrement
    POSITION pos = rechercherContigueOrdonnee(ms, nom_fichier, id);

    // V�rifier si l'enregistrement a �t� trouv�
    if (pos.bloc == -1) {
        printf("Erreur : Enregistrement avec ID %d non trouv� dans le fichier '%s'.\n", id, nom_fichier);
        return;
    }

    // Se positionner au bloc trouv� � l'aide de fseek
    fseek(ms, pos.bloc * sizeof(BLOC), SEEK_SET);
    BLOC buffer;

    // Lire les donn�es du bloc trouv�
    if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Lecture du bloc %d a �chou�.\n", pos.bloc);
        return;
    }

    // Marquer l'enregistrement comme supprim� en mettant l'ID � -1
    buffer.enregs[pos.deplacement].id = -1;

    // Revenir � la position initiale pour r��crire le bloc avec les modifications
    fseek(ms, pos.bloc * sizeof(BLOC), SEEK_SET);

    // �crire le bloc mis � jour dans la m�moire secondaire
    if (fwrite(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : �criture du bloc %d a �chou�.\n", pos.bloc);
        return;
    }

    // Afficher un message de confirmation de la suppression logique
    printf("Suppression logique de l'enregistrement avec ID %d dans une organisation Contigu� et ORDONNE.\n", id);
    // Impl�menter la logique pour marquer l'enregistrement comme supprim� (par exemple, changer un flag ou une valeur sp�ciale)
}

// Suppression physique : r�organiser les blocs pour lib�rer l'espace dans une organisation contigu� et ordonn�e
void suppressionContigueOrdonneePhysique(FILE *ms, const char *nom_fichier, int id) {
     // V�rifier si le pointeur du fichier est NULL
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    // Appeler la fonction de recherche pour trouver la position de l'enregistrement
    POSITION pos = rechercherContigueOrdonnee(ms, nom_fichier, id);

    // V�rifier si l'enregistrement a �t� trouv�
    if (pos.bloc == -1) {
        printf("Erreur : Enregistrement avec ID %d non trouv� dans le fichier '%s'.\n", id, nom_fichier);
        return;
    }

    // Se positionner au bloc trouv� � l'aide de fseek
    fseek(ms, pos.bloc * sizeof(BLOC), SEEK_SET);
    BLOC buffer;

    // Lire les donn�es du bloc trouv�
    if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Lecture du bloc %d a �chou�.\n", pos.bloc);
        return;
    }

    // Supprimer l'enregistrement en d�pla�ant les autres pour combler le vide
    for (int k = pos.deplacement; k < buffer.enregs_utilises - 1; k++) {
        buffer.enregs[k] = buffer.enregs[k + 1];
    }
    buffer.enregs_utilises--;
    METADATA meta;
    meta.taille_enregs--;

    // V�rifier si le bloc contient des fichiers critiques ou s'il est le premier bloc
    bool isCriticalBlock = (strcmp(buffer.metadata.fichier_nom, "table_allocation") == 0);

    // Si le bloc est maintenant vide et ne contient pas de fichiers critiques, mettre � jour la table d'allocation
    if (buffer.enregs_utilises == 0 && pos.bloc != 0 && !isCriticalBlock) {
        // Lecture de la table d'allocation depuis le premier bloc
        fseek(ms, 0, SEEK_SET);
        BLOC table_allocation;
        fread(&table_allocation, sizeof(BLOC), 1, ms);

        // Marquer le bloc comme libre dans la table d'allocation
        table_allocation.enregs[pos.bloc].id = 0;

        // �crire la table d'allocation mise � jour
        fseek(ms, 0, SEEK_SET);
        fwrite(&table_allocation, sizeof(BLOC), 1, ms);

        // R�initialiser les m�tadonn�es du bloc
        memset(&buffer.metadata, 0, sizeof(METADATA));
        strcpy(buffer.metadata.fichier_nom, " ");
    }

    // Mettre � jour les m�tadonn�es du fichier
    meta.taille_enregs--;

    // Revenir en arri�re pour r��crire le bloc avec les modifications
    fseek(ms, pos.bloc * sizeof(BLOC), SEEK_SET);

    // �crire le bloc mis � jour dans la m�moire secondaire
    if (fwrite(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : �criture du bloc %d a �chou�.\n", pos.bloc);
        return;
    }

    // Afficher un message de confirmation de la suppression physique
    printf("Enregistrement avec ID %d supprim� physiquement dans le fichier '%s'.\n", id, nom_fichier);
}


// Suppression logique : marquer comme supprimé dans une organisation chaînée et ordonnée
void suppressionChaineeOrdonneeLogique(FILE *ms, const char *nom_fichier, int id) {
    printf("Suppression logique de l'enregistrement avec ID %d dans une organisation Chaînée et ORDONNE.\n", id);
    // Implémenter la logique pour marquer l'enregistrement comme supprimé (par exemple, changer un flag ou une valeur spéciale)
}

// Suppression physique : réorganiser les blocs pour libérer l'espace dans une organisation chaînée et ordonnée
void suppressionChaineeOrdonneePhysique(FILE *ms, const char *nom_fichier, int id) {
    printf("Suppression physique de l'enregistrement avec ID %d dans une organisation Chaînée et ORDONNE.\n", id);
    // Implémenter la logique pour supprimer physiquement l'enregistrement et ajuster les liens/pointeurs dans la chaîne
}

// Suppression logique : marquer comme supprimé dans une organisation chaînée et non ordonnée
void suppressionChaineeNonOrdonneeLogique(FILE *ms, const char *nom_fichier, int id) {
    printf("Suppression logique de l'enregistrement avec ID %d dans une organisation Chaînée et NONORDONNE.\n", id);
    // Implémenter la logique pour marquer l'enregistrement comme supprimé (par exemple, changer un flag ou une valeur spéciale)
}

// Suppression physique : réorganiser les blocs pour libérer l'espace dans une organisation chaînée et non ordonnée
void suppressionChaineeNonOrdonneePhysique(FILE *ms, const char *nom_fichier, int id) {
    printf("Suppression physique de l'enregistrement avec ID %d dans une organisation Chaînée et NONORDONNE.\n", id);
    // Implémenter la logique pour supprimer physiquement l'enregistrement et ajuster les liens/pointeurs dans la chaîne
}

void defragmentationContigueOrdonnee(FILE *ms, const char *nom_fichier) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    BLOC buffer, temp_buffer;
    int fichier_trouve = 0;
    int new_adresse_bloc = 0;
    int enregs_count = 0;
    int adresse_premier = -1;

    fseek(ms, sizeof(BLOC), SEEK_SET);
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            fichier_trouve = 1;
            adresse_premier = buffer.metadata.adresse_premier;
            break;
        }
    }

    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouv�.\n", nom_fichier);
        return;
    }

    temp_buffer.enregs_utilises = 0;
    temp_buffer.metadata = buffer.metadata;

    fseek(ms, adresse_premier * sizeof(BLOC), SEEK_SET);
    for (int i = 0; i < buffer.metadata.taille_blocs; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : Lecture du bloc %d a �chou�.\n", i);
            return;
        }

        for (int j = 0; j < buffer.enregs_utilises; j++) {
            if (buffer.enregs[j].id != -1) {
                temp_buffer.enregs[temp_buffer.enregs_utilises++] = buffer.enregs[j];
                enregs_count++;

                if (temp_buffer.enregs_utilises == FACTEUR_BLOCS) {
                    fseek(ms, new_adresse_bloc * sizeof(BLOC), SEEK_SET);
                    if (fwrite(&temp_buffer, sizeof(BLOC), 1, ms) != 1) {
                        printf("Erreur : �criture du bloc %d a �chou�.\n", new_adresse_bloc);
                        return;
                    }
                    new_adresse_bloc++;
                    temp_buffer.enregs_utilises = 0;
                }
            }
        }

        if (buffer.enregs_utilises == 0 && i != 0) {
            fseek(ms, 0, SEEK_SET);
            fread(&buffer, sizeof(BLOC), 1, ms);
            buffer.enregs[i].id = 0;
            fseek(ms, 0, SEEK_SET);
            fwrite(&buffer, sizeof(BLOC), 1, ms);

            strcpy(buffer.metadata.fichier_nom, " ");
            buffer.metadata.taille_blocs = 0;
            buffer.metadata.taille_enregs = 0;
            buffer.metadata.adresse_premier = 0;
            memset(buffer.metadata.org_globale, 0, sizeof(buffer.metadata.org_globale));
            memset(buffer.metadata.org_interne, 0, sizeof(buffer.metadata.org_interne));
            fseek(ms, i * sizeof(BLOC), SEEK_SET);
            fwrite(&buffer, sizeof(BLOC), 1, ms);
        }
    }

    if (temp_buffer.enregs_utilises > 0) {
        fseek(ms, new_adresse_bloc * sizeof(BLOC), SEEK_SET);
        if (fwrite(&temp_buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : �criture du dernier bloc a �chou�.\n");
            return;
        }
        fseek(ms, 0, SEEK_SET);
        fread(&buffer, sizeof(BLOC), 1, ms);
        buffer.enregs[new_adresse_bloc].id = 1;
        fseek(ms, 0, SEEK_SET);
        fwrite(&buffer, sizeof(BLOC), 1, ms);
        new_adresse_bloc++;
    }

    temp_buffer.metadata.taille_blocs = (enregs_count + FACTEUR_BLOCS - 1) / FACTEUR_BLOCS;
    temp_buffer.metadata.taille_enregs = enregs_count;
    fseek(ms, adresse_premier * sizeof(BLOC), SEEK_SET);
    fwrite(&temp_buffer, sizeof(BLOC), 1, ms);

    printf("D�fragmentation termin�e pour le fichier '%s'.\n", nom_fichier);
}

// Fonction principale pour gérer la suppression en fonction de l'organisation et du type de suppression
void gererSuppression(FILE *ms, const char *nom_fichier, int id, int type_suppression, const char *org_globale, const char *org_interne) {
    // Vérification des paramètres
    if (ms == NULL) {
        printf("Erreur : fichier non ouvert.\n");
        return;
    }
    
    // Logique de suppression en fonction du type d'organisation et du type de suppression
    if (strcmp(org_globale, "Contigue") == 0) {
        if (strcmp(org_interne, "Ordonnee") == 0) {
            // Organisation contiguë et ordonnée
            if (type_suppression == 1) {
                suppressionContigueOrdonneeLogique(ms, nom_fichier, id); // Suppression logique
            } else if (type_suppression == 2) {
                suppressionContigueOrdonneePhysique(ms, nom_fichier, id); // Suppression physique
            } else {
                printf("Erreur : type de suppression invalide pour l'organisation Contiguë et Ordonnée.\n");
            }
        } else if (strcmp(org_interne, "NonOrdonnee") == 0) {
            // Organisation contiguë et non ordonnée
            if (type_suppression == 1) {
                suppressionContigueNonOrdonneeLogique(ms, nom_fichier, id); // Suppression logique
            } else if (type_suppression == 2) {
                suppressionContigueNonOrdonneePhysique(ms, nom_fichier, id); // Suppression physique
            } else {
                printf("Erreur : type de suppression invalide pour l'organisation Contiguë et NonOrdonnée.\n");
            }
        } else {
            printf("Erreur : organisation interne non valide.\n");
        }
    } else if (strcmp(org_globale, "Chainee") == 0) {
        if (strcmp(org_interne, "Ordonnee") == 0) {
            // Organisation chaînée et ordonnée
            if (type_suppression == 1) {
                suppressionChaineeOrdonneeLogique(ms, nom_fichier, id); // Suppression logique
            } else if (type_suppression == 2) {
                suppressionChaineeOrdonneePhysique(ms, nom_fichier, id); // Suppression physique
            } else {
                printf("Erreur : type de suppression invalide pour l'organisation Chaînée et Ordonnée.\n");
            }
        } else if (strcmp(org_interne, "NonOrdonnee") == 0) {
            // Organisation chaînée et non ordonnée
            if (type_suppression == 1) {
                suppressionChaineeNonOrdonneeLogique(ms, nom_fichier, id); // Suppression logique
            } else if (type_suppression == 2) {
                suppressionChaineeNonOrdonneePhysique(ms, nom_fichier, id); // Suppression physique
            } else {
                printf("Erreur : type de suppression invalide pour l'organisation Chaînée et NonOrdonnée.\n");
            }
        } else {
            printf("Erreur : organisation interne non valide.\n");
        }
    } else {
        printf("Erreur : organisation globale non valide.\n");
    }
}


// Fonction pour défragmenter un fichier dans une organisation chaînée et ordonnée
void defragmentationChaineeOrdonnee(FILE *ms, const char *nom_fichier) {
    printf("Défragmentation dans une organisation Chaînée et ORDONNE.\n");
    // Logique pour réorganiser les pointeurs et compresser les blocs chaînés
    // Cela pourrait impliquer de déplacer des enregistrements entre les liens dans la chaîne
}

// Fonction pour défragmenter un fichier dans une organisation chaînée et non ordonnée
void defragmentationChaineeNonOrdonnee(FILE *ms, const char *nom_fichier) {
    printf("Défragmentation dans une organisation Chaînée et NONORDONNE.\n");
    // Logique pour réorganiser les pointeurs et compresser les blocs chaînés
    // Réorganiser les enregistrements pour réduire les espaces vides dans la chaîne
}


// Fonction principale pour gérer la défragmentation en fonction de l'organisation et du type
void gererDefragmentation(FILE *ms, const char *nom_fichier, const char *org_globale, const char *org_interne) {

    if (strcmp(org_globale, "contigu") == 0) {
        if (strcmp(org_interne, "ORDONNE") == 0) {
            defragmentationContigueOrdonnee(ms, nom_fichier);
        } else if (strcmp(org_interne, "NONORDONNE") == 0) {
            defragmentationContigueNonOrdonnee(ms, nom_fichier);
        }
    } else if (strcmp(org_globale, "chaîné") == 0) {
        if (strcmp(org_interne, "ORDONNE") == 0) {
            defragmentationChaineeOrdonnee(ms, nom_fichier);
        } else if (strcmp(org_interne, "NONORDONNE") == 0) {
            defragmentationChaineeNonOrdonnee(ms, nom_fichier);
        }
    } else {
        printf("Erreur : type d'organisation inconnu.\n");
    }
}


void compactageMemoire(FILE *ms) {
    if (ms == NULL) {
        printf("Erreur : Impossible d'ouvrir 'memoire_secondaire.dat'.\n");
        return;
    }

    printf("Début du compactage de la mémoire secondaire...\n");

    BLOC bufferLecture, bufferEcriture;
    lireTableAllocation(ms, &bufferLecture); // Lire la table d'allocation

    char fichiersTraites[MAX_BLOCS][50];
    int nombreFichiersTraites = 0;

    // Identifier et défragmenter chaque fichier
    for (int i = 0; i < MAX_BLOCS; i++) {
        if (bufferLecture.enregs[i].id == 1) { // Vérifier si le bloc est occupé
            int dejaTraite = 0;
            for (int j = 0; j < nombreFichiersTraites; j++) {
                if (strcmp(bufferLecture.enregs[i].nom, fichiersTraites[j]) == 0) {
                    dejaTraite = 1;
                    break;
                }
            }

            if (!dejaTraite) {
                const char *nom_fichier = bufferLecture.enregs[i].nom;
                const char *org_globale = bufferLecture.metadata.org_globale;
                const char *org_interne = bufferLecture.metadata.org_interne;

                gererDefragmentation(ms, nom_fichier, org_globale, org_interne); // Défragmenter le fichier

                strcpy(fichiersTraites[nombreFichiersTraites], nom_fichier);
                nombreFichiersTraites++;

                if (strcmp(org_globale, "contigu") == 0) {
                    i += bufferLecture.metadata.taille_blocs - 1; // Sauter les blocs suivants appartenant au même fichier
                } else if (strcmp(org_globale, "chaîné") == 0) {
                    // Déterminer l'adresse du dernier bloc pour les fichiers chaînés
                    int current_block = bufferLecture.metadata.adresse_premier;
                    int last_block = current_block;

                    while (current_block != -1) {
                        BLOC buffer;
                        fseek(ms, current_block * sizeof(BLOC), SEEK_SET);
                        fread(&buffer, sizeof(BLOC), 1, ms);
                        last_block = current_block;
                        current_block = buffer.suivant; // Passer au bloc suivant dans la chaîne
                    }

                    i = last_block; // Sauter jusqu'au dernier bloc
                }
            }
        }
    }

    int dernierBlocOccupe = 0;
    int dernierBlocLibre = MAX_BLOCS - 1;
    int premierBlocLibre = -1;

    // Identifier le dernier bloc occupé et le premier bloc libre
    for (int i = 0; i < MAX_BLOCS; i++) {
        if (bufferLecture.enregs[i].id == 1) { // Bloc occupé
            if (i > dernierBlocOccupe) {
                dernierBlocOccupe = i;
            }
        } else { // Bloc libre
            if (premierBlocLibre == -1) {
                premierBlocLibre = i;
            }
            if (i < dernierBlocLibre) {
                dernierBlocLibre = i;
            }
        }
    }

    memset(&bufferEcriture, 0, sizeof(BLOC));

    // Parcourir la mémoire pour les fichiers contigus et chaînés
    for (int i = 0; i <= dernierBlocOccupe; i++) {
        if (bufferLecture.enregs[i].id == 1) { // Bloc occupé
            fseek(ms, (i + 1) * sizeof(BLOC), SEEK_SET);
            fread(&bufferLecture, sizeof(BLOC), 1, ms);

            if (strcmp(bufferLecture.metadata.org_globale, "contigu") == 0) {
                // Déplacer les blocs contigus ensemble
                int adresse_initiale = bufferLecture.metadata.adresse_premier;
                int taille_blocs = bufferLecture.metadata.taille_blocs;

                for (int j = 0; j < taille_blocs; j++) {
                    fseek(ms, (adresse_initiale + j) * sizeof(BLOC), SEEK_SET);
                    fread(&bufferLecture, sizeof(BLOC), 1, ms);
                    fseek(ms, (premierBlocLibre + j) * sizeof(BLOC), SEEK_SET);
                    fwrite(&bufferLecture, sizeof(BLOC), 1, ms);

                    // Mettre à jour la table d'allocation
                    miseAJourTableAllocation(ms, premierBlocLibre + j, 0);
                }

                bufferLecture.metadata.adresse_premier = premierBlocLibre;
                premierBlocLibre += taille_blocs;
            } else if (strcmp(bufferLecture.metadata.org_globale, "chaîné") == 0) {
                // Déplacer les blocs chaînés et mettre à jour les pointeurs
                int bloc_actuel = bufferLecture.metadata.adresse_premier;

                while (bloc_actuel != -1) {
                    fseek(ms, bloc_actuel * sizeof(BLOC), SEEK_SET);
                    fread(&bufferLecture, sizeof(BLOC), 1, ms);

                    int prochain_bloc = bufferLecture.suivant;
                    bufferLecture.suivant = premierBlocLibre;

                    fseek(ms, premierBlocLibre * sizeof(BLOC), SEEK_SET);
                    fwrite(&bufferLecture, sizeof(BLOC), 1, ms);

                    miseAJourTableAllocation(ms, premierBlocLibre, 0);

                    bloc_actuel = prochain_bloc;
                    premierBlocLibre++;
                }
            }
        }
    }

    // Réécrire la table d'allocation mise à jour
    fseek(ms, 0, SEEK_SET);
    fwrite(&bufferEcriture, sizeof(BLOC), 1, ms);

    printf("Le compactage de la mémoire secondaire est terminé.\n");
}




void renommerFichier(FILE *ms, const char *ancien_nom, const char *nouveau_nom) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    // Vérification si le nouveau nom existe déjà
    int etat_fichier = fichierExiste(ms, nouveau_nom);
    if (etat_fichier == 1) {
        printf("Erreur : le nom '%s' est déjà utilisé.\n", nouveau_nom);
        return;
    }

    BLOC buffer;
    bool fichier_trouve = false;

    // Recherche du fichier à renommer
    fseek(ms, sizeof(BLOC), SEEK_SET); // Positionner après la table d'allocation
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        if (strcmp(buffer.metadata.fichier_nom, ancien_nom) == 0) {
            fichier_trouve = true;
            strcpy(buffer.metadata.fichier_nom, nouveau_nom); // Renommer le fichier
            fseek(ms, -(long)sizeof(BLOC), SEEK_CUR); // Se positionner sur le bloc actuel
            if (fwrite(&buffer, sizeof(BLOC), 1, ms) != 1) {
                printf("Erreur lors de l'écriture du fichier renommé.\n");
                return;
            }
            printf("Fichier '%s' renommé en '%s'.\n", ancien_nom, nouveau_nom);
            break;
        }
    }

    // Si le fichier n'a pas été trouvé
    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouvé.\n", ancien_nom);
    }
}

// Fonction pour supprimer un fichier
void supprimerFichier(FILE *ms, const char *nom_du_fichier) {
    BLOC buffer;
    bool fichier_trouve = false;
    int adresse_premier_bloc = -1, taille_fichier = 0;

    // Étape 1: Recherche du fichier à supprimer
    fseek(ms, 0, SEEK_SET);
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        if (strcmp(buffer.metadata.fichier_nom, nom_du_fichier) == 0) {
            fichier_trouve = true;
            adresse_premier_bloc = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            break;
        }
    }

    // Étape 2: Si le fichier n'a pas été trouvé
    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouvé.\n", nom_du_fichier); // Affichage de l'erreur
        return;
    }

    // Étape 3: Suppression physique du fichier (vidage des blocs)
    fseek(ms, adresse_premier_bloc * sizeof(BLOC), SEEK_SET); // Se positionner sur le premier bloc du fichier
    for (int i = 0; i < taille_fichier; i++) {
        memset(&buffer, 0, sizeof(BLOC)); // Vider le contenu du bloc
        fwrite(&buffer, sizeof(BLOC), 1, ms); // Écriture du bloc vide pour supprimer
    }

    // Étape 4: Affichage du succès
    printf("Fichier '%s' supprimé avec succès.\n", nom_du_fichier);
}
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

void viderMemoireSecondaire(FILE *ms) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    BLOC buffer;
    memset(&buffer, 0, sizeof(BLOC));  // Initialiser le buffer à zéro

    fseek(ms, 0, SEEK_SET);  // Positionner au début du fichier
    while (fwrite(&buffer, sizeof(BLOC), 1, ms) == 1);

    printf("Mémoire secondaire vidée.\n");
}

// Fonction pour afficher le contenu des blocs dans la mémoire secondaire
void affichageBlocs(FILE *ms) {
    BLOC buffer;
    int compteur_bloc = 0;

    // Revenir au début du fichier
    fseek(ms, 0, SEEK_SET);

    // Lire et afficher chaque bloc de mémoire
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        // Afficher le numéro du bloc et le nombre d'enregistrements utilisés
        printf("Bloc %d:\n", compteur_bloc++);
        printf("  Nombre d'enregistrements utilises: %d\n", buffer.enregs_utilises);

        // Afficher les enregistrements dans le bloc
        for (int i = 0; i < buffer.enregs_utilises; i++) {
            printf("    Enregistrement %d: ID=%d, Nom=%s\n", i + 1, buffer.enregs[i].id, buffer.enregs[i].nom);
        }
    }
}

 int nombredeblocsLibres(FILE *f) { // f fichier qui contient la table d'allocation
    rewind(f);

    int table_allocation[MAX_BLOCS];
    size_t nombre_elements_lus = fread(table_allocation, sizeof(int), MAX_BLOCS, f);

    if (nombre_elements_lus != MAX_BLOCS) {
        fprintf(stderr, "Erreur : lecture incomplÃ¨te de la table d'allocation.\n");
        return -1;
    }

    int nombre_de_blocs_libres = 0;
    for (int i = 0; i < MAX_BLOCS; i++) {
        if (table_allocation[i] == 0) {  // Bloc libre
            nombre_de_blocs_libres++;
        }
    }

    return nombre_de_blocs_libres;  // Retourne le nombre de blocs libres
}


// Fonction pour quitter le programme
void quitterProgramme(FILE *ms) {
    // Fermer le fichier de la mémoire secondaire (si ouvert)
    if (ms != NULL) {
        fclose(ms);
        printf("MS fermé avec succès.\n");
    }

    // Afficher un message de confirmation de sortie
    printf("Programme terminé. À bientôt !\n");

    // Quitter le programme
    exit(0);
}  


void afficherMetadonnees(FILE *ms) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    BLOC buffer;
    int fichier_trouve = 0;

    printf("+------------------------------------------------------------+\n");
    printf("| Nom du fichier                  | Taille (blocs) | Enregs |\n");
    printf("+------------------------------------------------------------+\n");

    // Parcourir les blocs de la mémoire secondaire pour trouver les fichiers
    fseek(ms, sizeof(BLOC), SEEK_SET);  // Positionner après la table d'allocation
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        // Vérifier si le bloc contient un fichier
        if (buffer.enregs_utilises > 0) {
            printf("| %-30s | %-15d | %-6d |\n", buffer.metadata.fichier_nom, buffer.metadata.taille_blocs, buffer.metadata.taille_enregs);
            fichier_trouve = 1;
        }
    }

    if (!fichier_trouve) {
        printf("| Aucune métadonnée de fichier trouvée.                      |\n");
    }

    printf("+------------------------------------------------------------+\n");
}

void chargerContiguNonOrdonne(FILE *ms, METADATA *metadata) {
    BLOC table_allocation;
    BLOC bloc_alloue;
    int enreg_restants = metadata->taille_enregs;
    int nombre_blocs = metadata->taille_blocs;

    // Trouver un espace contigu libre
    int debut_bloc = -1;
    for (int i = 1; i <= MAX_BLOCS - nombre_blocs; i++) {
        int espace_libre = 1;
        lireTableAllocation(ms, &table_allocation);
        for (int j = 0; j < nombre_blocs; j++) {
            if (table_allocation.enregs[i + j].id == 1) {
                espace_libre = 0;
                break;
            }
        }
        if (espace_libre) {
            debut_bloc = i;
            break;
        }
    }

    if (debut_bloc == -1) {
        printf("Erreur : Pas d'espace contigu disponible pour allouer %d blocs.\n", nombre_blocs);
        return;
    }

    // Initialiser le générateur de nombres aléatoires
    srand(time(NULL));

    // Marquer les blocs comme occupés
    lireTableAllocation(ms, &table_allocation);
    for (int i = 0; i < nombre_blocs; i++) {
        table_allocation.enregs[debut_bloc + i].id = 1; // Marquer le bloc comme occupé
        sprintf(table_allocation.enregs[debut_bloc + i].nom, "Bloc_%d", debut_bloc + i);
    }
    rewind(ms);
    fwrite(&table_allocation, sizeof(BLOC), 1, ms);

    // Allouer les blocs et les remplir avec des valeurs aléatoires
    for (int i = 0; i < nombre_blocs; i++) {
        int current_bloc = debut_bloc + i;

        // Lire le bloc à allouer
        fseek(ms, current_bloc * sizeof(BLOC), SEEK_SET);
        fread(&bloc_alloue, sizeof(BLOC), 1, ms);

        // Remplir le bloc avec des valeurs aléatoires
        bloc_alloue.enregs_utilises = 0;
        for (int j = 0; j < FACTEUR_BLOCS && enreg_restants > 0; j++) {
            bloc_alloue.enregs[j].id = rand() % 1000; // Valeur aléatoire entre 0 et 999
            sprintf(bloc_alloue.enregs[j].nom, "Enreg_%d", rand() % 1000);
            enreg_restants--;
            bloc_alloue.enregs_utilises++;
        }

        // Si c'est le premier bloc du fichier créé, mettre à jour les métadonnées
        if (i == 0) {
            bloc_alloue.metadata = *metadata;
        } else {
            // Mettre à jour seulement le nom du fichier pour les autres blocs
            strcpy(bloc_alloue.metadata.fichier_nom, metadata->fichier_nom);
            memset(&bloc_alloue.metadata + sizeof(bloc_alloue.metadata.fichier_nom), 0, sizeof(METADATA) - sizeof(bloc_alloue.metadata.fichier_nom));
        }

        // Écrire le bloc alloué dans le fichier
        fseek(ms, current_bloc * sizeof(BLOC), SEEK_SET);
        fwrite(&bloc_alloue, sizeof(BLOC), 1, ms);
    }

    printf("Chargement des blocs contigus non ordonnés effectué avec succès.\n");
}


POSITION rechercherContigueNonOrdonnee(FILE *ms, const char *nom_fichier, int id) {
    POSITION pos;
    pos.bloc = -1;  // Valeur par défaut indiquant que l'enregistrement n'a pas été trouvé
    pos.deplacement = -1;

    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return pos;  // Vérifier si le pointeur du fichier est NULL
    }

    BLOC buffer;
    int adresse_premier_bloc = -1;
    int taille_fichier = 0;
    int fichier_trouve = 0;

    // Recherche du fichier dans le fichier de stockage
    fseek(ms, 0, SEEK_SET);  // Positionner au début du fichier
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        // Comparer le nom du fichier actuel avec le nom recherché
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            fichier_trouve = 1;
            adresse_premier_bloc = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            break;  // Fichier trouvé, sortir de la boucle
        }
    }

    // Vérifier si le fichier a été trouvé
    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouvé.\n", nom_fichier);
        return pos;  // Fichier non trouvé
    }

    // Vérifier la validité des métadonnées du fichier
    if (taille_fichier == 0 || adresse_premier_bloc == -1) {
        printf("Erreur : Le fichier '%s' est vide ou corrompu.\n", nom_fichier);
        return pos;
    }

    // Recherche dans les blocs contigus du fichier
    fseek(ms, adresse_premier_bloc * sizeof(BLOC), SEEK_SET);  // Aller à l'adresse du premier bloc
    for (int i = 0; i < taille_fichier; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : Lecture du bloc %d a échoué.\n", i);
            return pos;  // Erreur de lecture du bloc
        }

        // Parcours des enregistrements du bloc pour rechercher l'ID
        for (int j = 0; j < buffer.enregs_utilises; j++) {
            if (buffer.enregs[j].id == id) {  // Si l'ID correspond
                pos.bloc = adresse_premier_bloc + i;
                pos.deplacement = j;
                return pos;  // Retourner la position de l'enregistrement
            }
        }
    }

    // Si l'enregistrement n'est pas trouvé
    printf("Enregistrement non trouvé.\n");
    return pos;  // Enregistrement non trouvé
}


// Fonction pour insérer un enregistrement de manière contiguë non ordonnée
void insertionContigueNonOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    BLOC table_allocation;
    BLOC buffer;
    int adresse_premier_bloc = -1;
    int taille_fichier = 0;
    int fichier_trouve = 0;

    // Lire la table d'allocation depuis le premier bloc
    rewind(ms);
    fread(&table_allocation, sizeof(BLOC), 1, ms);

    // Recherche du fichier dans le fichier de stockage
    fseek(ms, sizeof(BLOC), SEEK_SET);  // Positionner au début après la table d'allocation
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            fichier_trouve = 1;
            adresse_premier_bloc = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            break;  // Fichier trouvé, sortir de la boucle
        }
    }

    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouvé.\n", nom_fichier);
        return;
    }

    // Vérifier si l'ID est unique en appelant rechercherContigueNonOrdonnee
    POSITION pos = rechercherContigueNonOrdonnee(ms, nom_fichier, nouvel_enreg.id);
    if (pos.bloc != -1) {
        printf("Erreur : L'ID %d existe déjà.\n", nouvel_enreg.id);
        return;  // ID trouvé, donc non unique
    }

    // Réinitialiser la position pour l'insertion
    fseek(ms, adresse_premier_bloc * (long)sizeof(BLOC), SEEK_SET);
    for (int i = 0; i < taille_fichier; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : Lecture du bloc %d a échoué.\n", i);
            return;  // Erreur de lecture du bloc
        }

        // Chercher un emplacement libre dans le bloc
        if (buffer.enregs_utilises < FACTEUR_BLOCS) {
            buffer.enregs[buffer.enregs_utilises] = nouvel_enreg;  // Insérer le nouvel enregistrement
            buffer.enregs_utilises++;
            fseek(ms, -((long)sizeof(BLOC)), SEEK_CUR);  // Revenir en arrière pour réécrire le bloc
            fwrite(&buffer, sizeof(BLOC), 1, ms);  // Écrire le bloc mis à jour
            printf("Enregistrement inséré avec succès : Bloc = %d, Index = %d\n", i, buffer.enregs_utilises - 1);

            // Mettre à jour les métadonnées du premier bloc du fichier
            if (i == 0) {
                buffer.metadata.taille_enregs++;
                fseek(ms, adresse_premier_bloc * (long)sizeof(BLOC), SEEK_SET);
                fwrite(&buffer, sizeof(BLOC), 1, ms);
            }

            return;  // Enregistrement inséré avec succès
        }
    }

    // Si aucun espace n'est disponible pour allouer un nouveau bloc
    int bloc_alloue = -1;
    for (int i = 1; i < MAX_BLOCS; i++) {
        if (table_allocation.enregs[i].id == 0) { // Si le bloc est libre
            table_allocation.enregs[i].id = 1; // Marquer le bloc comme occupé
            bloc_alloue = i;
            break;
        }
    }

    if (bloc_alloue == -1) {
        int choix;
        printf("Pas de blocs disponibles pour allouer un nouveau bloc. Choisissez une option :\n");
        printf("1. Compactage de la mémoire\n");
        printf("2. Annuler l'insertion\n");
        scanf("%d", &choix);

        if (choix == 1) {
            compactageMemoire(ms);
            for (int i = 1; i < MAX_BLOCS; i++) {
                if (table_allocation.enregs[i].id == 0) { // Si le bloc est libre après compactage
                    table_allocation.enregs[i].id = 1; // Marquer le bloc comme occupé
                    bloc_alloue = i;
                    break;
                }
            }
            if (bloc_alloue == -1) {
                printf("Erreur : Pas de blocs disponibles même après compactage.\n");
                return;
            }
        } else {
            printf("Insertion annulée.\n");
            return;
        }
    }

    // Initialiser le nouveau bloc
    BLOC nouveau_bloc = {0}; // Réinitialiser tous les champs à 0
    nouveau_bloc.enregs[0] = nouvel_enreg;
    nouveau_bloc.enregs_utilises = 1;
    strcpy(nouveau_bloc.metadata.fichier_nom, nom_fichier);

    // Écrire le nouveau bloc dans la mémoire secondaire
    fseek(ms, bloc_alloue * (long)sizeof(BLOC), SEEK_SET);
    fwrite(&nouveau_bloc, sizeof(BLOC), 1, ms);

    // Mettre à jour la table d'allocation
    rewind(ms);
    fwrite(&table_allocation, sizeof(BLOC), 1, ms);

    printf("Enregistrement inséré avec succès dans un nouveau bloc : Bloc = %d, Index = 0\n", bloc_alloue);

    // Mettre à jour les métadonnées du premier bloc du fichier
    fseek(ms, adresse_premier_bloc * (long)sizeof(BLOC), SEEK_SET);
    fread(&buffer, sizeof(BLOC), 1, ms);
    buffer.metadata.taille_enregs++;
    fseek(ms, adresse_premier_bloc * (long)sizeof(BLOC), SEEK_SET);
    fwrite(&buffer, sizeof(BLOC), 1, ms);
}




void suppressionContigueNonOrdonneeLogique(FILE *ms, METADATA *meta, const char *nom_fichier, int id) {
    // Vérifier si le pointeur du fichier est NULL
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    // Appeler la fonction de recherche pour trouver la position de l'enregistrement
    POSITION pos = rechercherContigueNonOrdonnee(ms, nom_fichier, id);

    // Vérifier si l'enregistrement a été trouvé
    if (pos.bloc == -1) {
        printf("Erreur : Enregistrement avec ID %d non trouvé dans le fichier '%s'.\n", id, nom_fichier);
        return;
    }

    // Se positionner au bloc trouvé à l'aide de fseek
    fseek(ms, pos.bloc * sizeof(BLOC), SEEK_SET);
    BLOC buffer;

    // Lire les données du bloc trouvé
    if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Lecture du bloc %d a échoué.\n", pos.bloc);
        return;
    }

    // Marquer l'enregistrement comme supprimé en mettant l'ID à -1
    buffer.enregs[pos.deplacement].id = -1;

    // Revenir à la position initiale pour réécrire le bloc avec les modifications
    fseek(ms, pos.bloc * sizeof(BLOC), SEEK_SET);

    // Écrire le bloc mis à jour dans la mémoire secondaire
    if (fwrite(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Écriture du bloc %d a échoué.\n", pos.bloc);
        return;
    }

    // Afficher un message de confirmation de la suppression logique
    printf("Enregistrement avec ID %d marqué comme supprimé dans le fichier '%s'.\n", id, nom_fichier);
}

void suppressionContigueNonOrdonneePhysique(FILE *ms, METADATA *meta, const char *nom_fichier, int id) {
    // Vérifier si le pointeur du fichier est NULL
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    // Appeler la fonction de recherche pour trouver la position de l'enregistrement
    POSITION pos = gererRecherche(ms, meta, nom_fichier, id);

    // Vérifier si l'enregistrement a été trouvé
    if (pos.bloc == -1) {
        printf("Erreur : Enregistrement avec ID %d non trouvé dans le fichier '%s'.\n", id, nom_fichier);
        return;
    }

    // Se positionner au bloc trouvé à l'aide de fseek
    fseek(ms, pos.bloc * sizeof(BLOC), SEEK_SET);
    BLOC buffer;

    // Lire les données du bloc trouvé
    if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Lecture du bloc %d a échoué.\n", pos.bloc);
        return;
    }

    // Supprimer l'enregistrement en déplaçant les autres pour combler le vide
    for (int k = pos.deplacement; k < buffer.enregs_utilises - 1; k++) {
        buffer.enregs[k] = buffer.enregs[k + 1];
    }
    buffer.enregs_utilises--;

    // Vérifier si le bloc contient des fichiers critiques ou s'il est le premier bloc
    bool isCriticalBlock = (strcmp(buffer.metadata.fichier_nom, "table_allocation") == 0);

    // Si le bloc est maintenant vide et ne contient pas de fichiers critiques, mettre à jour la table d'allocation
    if (buffer.enregs_utilises == 0 && pos.bloc != 0 && !isCriticalBlock) {
        // Lecture de la table d'allocation depuis le premier bloc
        fseek(ms, 0, SEEK_SET);
        BLOC table_allocation;
        fread(&table_allocation, sizeof(BLOC), 1, ms);
        
        // Marquer le bloc comme libre dans la table d'allocation
        table_allocation.enregs[pos.bloc].id = 0;
        
        // Écrire la table d'allocation mise à jour
        fseek(ms, 0, SEEK_SET);
        fwrite(&table_allocation, sizeof(BLOC), 1, ms);

        // Réinitialiser les métadonnées du bloc
        memset(&buffer.metadata, 0, sizeof(METADATA));
        strcpy(buffer.metadata.fichier_nom, " ");
    }

    // Mettre à jour les métadonnées du fichier
    meta->taille_enregs--;

    // Revenir en arrière pour réécrire le bloc avec les modifications
    fseek(ms, pos.bloc * sizeof(BLOC), SEEK_SET);

    // Écrire le bloc mis à jour dans la mémoire secondaire
    if (fwrite(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Écriture du bloc %d a échoué.\n", pos.bloc);
        return;
    }

    // Afficher un message de confirmation de la suppression physique
    printf("Enregistrement avec ID %d supprimé physiquement dans le fichier '%s'.\n", id, nom_fichier);
}

void defragmentationContigueNonOrdonnee(FILE *ms, const char *nom_fichier) {
    // Vérifier si le pointeur du fichier est NULL
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    BLOC buffer, temp_buffer; // Buffer temporaire pour stocker les enregistrements valides
    int fichier_trouve = 0;
    int new_adresse_bloc = 0;
    int enregs_count = 0;
    int adresse_premier = -1;

    // Recherche du fichier dans le fichier de stockage
    fseek(ms, sizeof(BLOC), SEEK_SET); // Positionner au début après la table d'allocation
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        // Comparer le nom du fichier actuel avec le nom recherché
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            fichier_trouve = 1;
            adresse_premier = buffer.metadata.adresse_premier;
            break; // Fichier trouvé, sortir de la boucle
        }
    }

    // Vérifier si le fichier a été trouvé
    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouvé.\n", nom_fichier);
        return;
    }

    // Initialiser le buffer temporaire
    temp_buffer.enregs_utilises = 0;
    temp_buffer.metadata = buffer.metadata;

    // Parcourir les blocs et déplacer les enregistrements valides
    fseek(ms, adresse_premier * sizeof(BLOC), SEEK_SET);
    for (int i = 0; i < buffer.metadata.taille_blocs; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            // Vérifier si la lecture du bloc a échoué
            printf("Erreur : Lecture du bloc %d a échoué.\n", i);
            return;
        }

        // Parcourir les enregistrements du bloc
        for (int j = 0; j < buffer.enregs_utilises; j++) {
            if (buffer.enregs[j].id != -1) { // Vérifier si l'enregistrement n'est pas marqué comme supprimé
                // Copier l'enregistrement valide dans le buffer temporaire
                temp_buffer.enregs[temp_buffer.enregs_utilises++] = buffer.enregs[j];
                enregs_count++;

                // Si le buffer temporaire est plein, écrire dans le fichier de stockage
                if (temp_buffer.enregs_utilises == FACTEUR_BLOCS) {
                    fseek(ms, new_adresse_bloc * sizeof(BLOC), SEEK_SET);
                    fwrite(&temp_buffer, sizeof(BLOC), 1, ms);
                    // Mettre à jour la table d'allocation
                    fseek(ms, 0, SEEK_SET);
                    fread(&buffer, sizeof(BLOC), 1, ms);
                    buffer.enregs[new_adresse_bloc].id = 1; // Marquer le bloc comme occupé
                    fseek(ms, 0, SEEK_SET);
                    fwrite(&buffer, sizeof(BLOC), 1, ms);
                    new_adresse_bloc++;
                    temp_buffer.enregs_utilises = 0; // Réinitialiser le buffer temporaire
                }
            }
        }

        // Si le bloc actuel devient vide et n'est pas le premier bloc, mettre à jour la table d'allocation
        if (buffer.enregs_utilises == 0 && i != 0) {
            fseek(ms, 0, SEEK_SET);
            fread(&buffer, sizeof(BLOC), 1, ms);
            buffer.enregs[i].id = 0; // Marquer le bloc comme libre
            fseek(ms, 0, SEEK_SET);
            fwrite(&buffer, sizeof(BLOC), 1, ms);
            
            // Réinitialiser les métadonnées du bloc
            strcpy(buffer.metadata.fichier_nom, " ");
            buffer.metadata.taille_blocs = 0;
            buffer.metadata.taille_enregs = 0;
            buffer.metadata.adresse_premier = 0;
            memset(buffer.metadata.org_globale, 0, sizeof(buffer.metadata.org_globale));
            memset(buffer.metadata.org_interne, 0, sizeof(buffer.metadata.org_interne));
            fseek(ms, i * sizeof(BLOC), SEEK_SET);
            fwrite(&buffer, sizeof(BLOC), 1, ms);
        }
    }

    // Écrire le reste des enregistrements du buffer temporaire s'il n'est pas vide
    if (temp_buffer.enregs_utilises > 0) {
        fseek(ms, new_adresse_bloc * sizeof(BLOC), SEEK_SET);
        fwrite(&temp_buffer, sizeof(BLOC), 1, ms);
        // Mettre à jour la table d'allocation
        fseek(ms, 0, SEEK_SET);
        fread(&buffer, sizeof(BLOC), 1, ms);
        buffer.enregs[new_adresse_bloc].id = 1; // Marquer le bloc comme occupé
        fseek(ms, 0, SEEK_SET);
        fwrite(&buffer, sizeof(BLOC), 1, ms);
        new_adresse_bloc++;
    }

    // Mise à jour des métadonnées
    temp_buffer.metadata.taille_blocs = (enregs_count + FACTEUR_BLOCS - 1) / FACTEUR_BLOCS;
    temp_buffer.metadata.taille_enregs = enregs_count;
    fseek(ms, adresse_premier * sizeof(BLOC), SEEK_SET);
    fwrite(&temp_buffer, sizeof(BLOC), 1, ms);

    printf("Défragmentation terminée pour le fichier '%s'.\n", nom_fichier);
}

