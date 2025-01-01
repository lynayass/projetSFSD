
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_BLOCS 15
#define FACTEUR_BLOCS 15  // Facteur de blocage égal au nombre d'enregistrements par bloc

// Structure d'un enregistrement
typedef struct ENREG {
    int id;            // Indique si le bloc est occupé (1) ou libre (0)
    char nom[50];      // Contient le numéro du bloc (nom du bloc)
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
    int deplacement;
} POSITION;


// Fonction pour lire la table d'allocation depuis le premier bloc
void lireTableAllocation(FILE *ms, BLOC *buffer) {
    rewind(ms);
    fread(buffer, sizeof(BLOC), 1, ms);
}

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

