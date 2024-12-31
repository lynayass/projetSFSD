#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_BLOCS 15
#define FACTEUR_BLOCS 15  // Facteur de blocage �gal au nombre d'enregistrements par bloc

// Structure d'un enregistrement
typedef struct ENREG {
    int id;            // Indique si le bloc est occup� (1) ou libre (0)
    char nom[50];      // Contient le num�ro du bloc (nom du bloc)
} ENREG;

// Structure des m�tadonn�es d'un fichier
typedef struct METADATA {
    char fichier_nom[50];     // Nom du fichier
    int taille_blocs;         // Nombre de blocs utilis�s dans le fichier
    int taille_enregs;        // Nombre d'enregistrements
    int adresse_premier;      // Adresse du premier bloc
    char org_globale[30];     // Organisation globale ('contigu', 'cha�n�')
    char org_interne[30];     // Organisation interne ('ORDONNE', 'NONORDONNE')
} METADATA;

// Structure d'un bloc
typedef struct BLOC {
    ENREG enregs[FACTEUR_BLOCS]; // Enregistrements dans le bloc
    int enregs_utilises;         // Nombre d'enregistrements utilis�s
    METADATA metadata;           // M�tadonn�es du fichier
    int suivant;                 // Adresse logique du bloc suivant (pour cha�n�)
} BLOC;

typedef struct POSITION POSITION;
struct POSITION {
    int bloc;
    int deplacement;
};

// Fonction pour lire la table d'allocation depuis le premier bloc
void lireTableAllocation(FILE *ms, BLOC *buffer) {
    rewind(ms);
    fread(buffer, sizeof(BLOC), 1, ms);
}

// Fonction pour mettre � jour les m�tadonn�es dans le bloc de m�moire secondaire
void mettreAJourMetadonnees(FILE *ms, const METADATA *metadata, int blocIndex) {
    if (ms == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier de m�moire secondaire.\n");
        return;
    }

    // Calculer la position du bloc dans le fichier
    long position = blocIndex * sizeof(BLOC);

    // D�placer le pointeur du fichier � la position du bloc
    fseek(ms, position, SEEK_SET);

    // Lire le bloc existant
    BLOC buffer;
    fread(&buffer, sizeof(BLOC), 1, ms);

    // Mettre � jour les m�tadonn�es dans le bloc
    buffer.metadata = *metadata;

    // Revenir � la position du bloc et �crire les m�tadonn�es mises � jour
    fseek(ms, position, SEEK_SET);
    if (fwrite(&buffer, sizeof(BLOC), 1, ms) != 1) {
        printf("Erreur : Impossible de mettre � jour les m�tadonn�es dans le fichier.\n");
    } else {
        printf("Les m�tadonn�es ont �t� mises � jour avec succ�s dans le bloc %d.\n", blocIndex);
    }
}

void chargerContiguOrdonne(FILE *ms, METADATA *metadata) {
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

    // Initialiser le g�n�rateur de nombres al�atoires
    srand(time(NULL));

    // Marquer les blocs comme occup�s
    lireTableAllocation(ms, &table_allocation);
    for (int i = 0 ; i < nombre_blocs; i++) {
        table_allocation.enregs[debut_bloc + i].id = 1; // Marquer le bloc comme occup�
        sprintf(table_allocation.enregs[debut_bloc + i].nom, "Bloc_%d", debut_bloc + i);
    }
    rewind(ms);
    fwrite(&table_allocation, sizeof(BLOC), 1, ms);

    // Allouer les blocs et les remplir avec des valeurs al�atoires
    for (int i = 0; i < nombre_blocs; i++) {
        int current_bloc = debut_bloc + i;

        // Lire le bloc � allouer
        fseek(ms, current_bloc * sizeof(BLOC), SEEK_SET);
        fread(&bloc_alloue, sizeof(BLOC), 1, ms);

        // Remplir le bloc avec des valeurs al�atoires
        bloc_alloue.enregs_utilises = 0;
        for (int j = 0; j < FACTEUR_BLOCS && enreg_restants > 0; j++) {
            bloc_alloue.enregs[j].id = rand() % 1000; // Valeur al�atoire entre 0 et 999
            sprintf(bloc_alloue.enregs[j].nom, "Enreg_%d", rand() % 1000);
            enreg_restants--;
            bloc_alloue.enregs_utilises++;
        }

        // Si c'est le premier bloc du fichier cr��, mettre � jour les m�tadonn�es
        if (i == 0) {
            bloc_alloue.metadata = *metadata;
        } else {
            // Initialiser les m�tadonn�es des autres blocs
            memset(&bloc_alloue.metadata, 0, sizeof(METADATA));
        }

        // �crire le bloc allou� dans le fichier
        fseek(ms, current_bloc * sizeof(BLOC), SEEK_SET);
        fwrite(&bloc_alloue, sizeof(BLOC), 1, ms);
    }

    printf("Chargement des blocs contigus ordonn�s effectu� avec succ�s.\n");
}

POSITION rechercherContigueOrdonnee(FILE *ms, const char *nom_fichier, int id) {
    POSITION pos;
    pos.bloc = -1;  // Valeur par d�faut indiquant que l'enregistrement n'a pas �t� trouv�
    pos.deplacement = -1;

    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return pos;  // V�rifier si le pointeur du fichier est NULL
    }

    BLOC buffer;
    int adresse_premier_bloc = -1;
    int taille_fichier = 0;
    int fichier_trouve = 0;

    // Recherche du fichier dans le fichier de stockage
    fseek(ms, 0, SEEK_SET);  // Positionner au d�but du fichier
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        // Comparer le nom du fichier actuel avec le nom recherch�
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            fichier_trouve = 1;
            adresse_premier_bloc = buffer.metadata.adresse_premier;
            taille_fichier = buffer.metadata.taille_blocs;
            break;  // Fichier trouv�, sortir de la boucle
        }
    }

    // V�rifier si le fichier a �t� trouv�
    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouv�.\n", nom_fichier);
        return pos;  // Fichier non trouv�
    }

    // V�rifier la validit� des m�tadonn�es du fichier
    if (taille_fichier == 0 || adresse_premier_bloc == -1) {
        printf("Erreur : Le fichier '%s' est vide ou corrompu.\n", nom_fichier);
        return pos;
    }

    // Recherche dans les blocs contigus du fichier
    fseek(ms, adresse_premier_bloc * sizeof(BLOC), SEEK_SET);  // Aller � l'adresse du premier bloc
    for (int i = 0; i < taille_fichier; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : Lecture du bloc %d a �chou�.\n", i);
            return pos;  // Erreur de lecture du bloc
        }

        // Parcours des enregistrements du bloc pour rechercher l'ID
        for (int j = 0; j < buffer.enregs_utilises; j++) {
            if (buffer.enregs[j].id == id) {  // Si l'ID correspond
                pos.bloc = adresse_premier_bloc + i;
                pos.deplacement = j;
                return pos;  // Retourner la position de l'enregistrement
            }
            if (buffer.enregs[j].id > id) { // Si l'ID est sup�rieur, on peut arr�ter la recherche
                break; // Sortir de la boucle, car les enregistrements sont ordonn�s
            }
        }
    }

    // Si l'enregistrement n'est pas trouv�
    printf("Enregistrement non trouv�.\n");
    return pos;  // Enregistrement non trouv�
}

void insertionContigueOrdonnee(FILE *ms, const char *nom_fichier, ENREG nouvel_enreg) {
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;  // V�rifier si le pointeur du fichier est NULL
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

    // V�rifier si le fichier a �t� trouv�
    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouv�.\n", nom_fichier);
        return;  // Fichier non trouv�
    }

    // V�rifier si l'ID est unique en appelant rechercherContigueOrdonnee
    if (rechercherContigueOrdonnee(ms, nom_fichier, nouvel_enreg.id).bloc != -1) {
        printf("Erreur : L'ID %d existe d�j�.\n", nouvel_enreg.id);
        return;  // ID trouv�, donc non unique
    }

    // R�initialiser la position pour l'insertion
    fseek(ms, adresse_premier_bloc * sizeof(BLOC), SEEK_SET);
    for (int i = 0; i < taille_fichier; i++) {
        if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            printf("Erreur : Lecture du bloc %d a �chou�.\n", i);
            return;  // Erreur de lecture du bloc
        }

        // Chercher un emplacement libre dans le bloc
        if (buffer.enregs_utilises < FACTEUR_BLOCS) {
            // Ins�rer le nouvel enregistrement � la bonne position pour maintenir l'ordre
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
            buffer.enregs[j] = nouvel_enreg;  // Ins�rer le nouvel enregistrement
            buffer.enregs_utilises++;
            fseek(ms, -sizeof(BLOC), SEEK_CUR);  // Revenir en arri�re pour r��crire le bloc
            fwrite(&buffer, sizeof(BLOC), 1, ms);  // �crire le bloc mis � jour
            printf("Enregistrement ins�r� avec succ�s : Bloc = %d, Index = %d\n", i, j);

            // Mettre � jour les m�tadonn�es du premier bloc du fichier
            if (i == 0) {
                buffer.metadata.taille_enregs++;
                fseek(ms, adresse_premier_bloc * sizeof(BLOC), SEEK_SET);
                fwrite(&buffer, sizeof(BLOC), 1, ms);
            }

            return;  // Enregistrement ins�r� avec succ�s
        }
    }

    // Si aucun espace n'est disponible pour allouer un nouveau bloc
    int bloc_alloue = -1;
    for (int i = 1; i < MAX_BLOCS; i++) {
        if (table_allocation.enregs[i].id == 0) { // Si le bloc est libre
            table_alloue = i;
            break;
        }
    }

    if (bloc_alloue == -1) {
        int choix;
        printf ("Pas de blocs disponibles pour allouer un nouveau bloc. Choisissez une option :\n");
        printf("1. Compactage de la m�moire\n");
        printf("2. Annuler l'insertion\n");
        scanf("%d", &choix);

        if (choix == 1) {
            compactageMemoire(ms);
            for (int i = 1; i < MAX_BLOCS; i++) {
                if (table_allocation.enregs[i].id == 0) { // Si le bloc est libre apr�s compactage
                    table_alloue = i;
                    break;
                }
            }
            if (table_alloue == -1) {
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
    nouveau_bloc.enregs[0] = nouvel_enreg;  // Ins�rer le nouvel enregistrement
    nouveau_bloc.enregs_utilises = 1;
    nouveau_bloc.metadata = buffer.metadata;

    // �crire le nouveau bloc dans la m�moire secondaire
    fseek(ms, bloc_alloue * sizeof(BLOC), SEEK_SET);
    fwrite(&nouveau_bloc, sizeof(BLOC), 1, ms);

    // Mettre � jour la table d'allocation
    rewind(ms);
    fwrite(&table_allocation, sizeof(BLOC), 1, ms);

    printf("Enregistrement ins�r� avec succ�s dans un nouveau bloc : Bloc = %d, Index = 0\n", bloc_alloue);

    // Mettre � jour les m�tadonn�es du premier bloc du fichier
    fseek(ms, adresse_premier_bloc * sizeof(BLOC), SEEK_SET);
    fread(&buffer, sizeof(BLOC), 1, ms);
    buffer.metadata.taille_enregs++;
    fseek(ms, adresse_premier_bloc * sizeof(BLOC), SEEK_SET);
    fwrite(&buffer, sizeof(BLOC), 1, ms);
}

void suppressionContigueOrdonneeLogique(FILE *ms, METADATA *meta, const char *nom_fichier, int id) {
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
    printf("Enregistrement avec ID %d marqu� comme supprim� dans le fichier '%s'.\n", id, nom_fichier);
}

void suppressionContigueOrdonneePhysique(FILE *ms, METADATA *meta, const char *nom_fichier, int id) {
    // V�rifier si le pointeur du fichier est NULL
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    // Appeler la fonction de recherche pour trouver la position de l'enregistrement
    POSITION pos = gererRecherche(ms, meta, nom_fichier, id);

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

    // V�rifier si le bloc contient des fichiers critiques ou s'il est le premier bloc
    bool isCriticalBlock = (strcmp(buffer.metadata.fichier_nom, "table_allocation") == 0);

    // Si le bloc est maintenant vide et ne contient pas de fichiers critiques, mettre � jour la table d'allocation
    if (buffer.enregs_utilises == 0 && pos.bloc != 0 && !isCriticalBlock) {
        fseek(ms, 0, SEEK_SET); // Retourner au d�but pour lire la table d'allocation
        BLOC table_allocation;
        fread(&table_allocation, sizeof(BLOC), 1, ms);
        table_allocation.enregs[pos.bloc].id = 0; // Marquer le bloc comme libre
        fseek(ms, 0, SEEK_SET); // Retourner au d�but pour �crire la table d'allocation mise � jour
        fwrite(&table_allocation, sizeof(BLOC), 1, ms);
    } else if (pos.bloc == 0) {
        printf("Bloc %d non lib�r� car il s'agit du premier bloc de fichier.\n", pos.bloc);
    } else {
        printf("Bloc %d non lib�r� car il contient un fichier critique.\n", pos.bloc);
    }

    // Mettre � jour les m�tadonn�es
    meta->taille_enregs--;

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

// Fonction pour mettre � jour la table d'allocation
void miseAJourTableAllocation(FILE *ms, int indice, int est_libre) {
    BLOC table_allocation;

    // Lire la table d'allocation
    rewind(ms);
    fread(&table_allocation, sizeof(BLOC), 1, ms);

    // Mettre � jour la table d'allocation
    table_allocation.enregs[indice].id = est_libre ? 0 : 1;

    // �crire la table d'allocation mise � jour
    rewind(ms);
    fwrite(&table_allocation, sizeof(BLOC), 1, ms);
}

void defragmentationContigueOrdonnee(FILE *ms, const char *nom_fichier) {
    // V�rifier si le pointeur du fichier est NULL
    if (ms == NULL) {
        printf("Erreur : Le fichier de stockage est invalide.\n");
        return;
    }

    BLOC buffer;
    BLOC temp_buffer; // Buffer temporaire pour stocker les enregistrements valides
    int fichier_trouve = 0;
    int new_adresse_bloc = 0;
    int enregs_count = 0;

    // Recherche du fichier dans le fichier de stockage
    fseek(ms, sizeof(BLOC), SEEK_SET);  // Positionner au d�but apr�s la table d'allocation
    while (fread(&buffer, sizeof(BLOC), 1, ms) == 1) {
        // Comparer le nom du fichier actuel avec le nom recherch�
        if (strcmp(buffer.metadata.fichier_nom, nom_fichier) == 0) {
            fichier_trouve = 1;
            break; // Fichier trouv�, sortir de la boucle
        }
    }

    // V�rifier si le fichier a �t� trouv�
    if (!fichier_trouve) {
        printf("Erreur : fichier '%s' non trouv�.\n", nom_fichier);
        return;
    }

    // Initialiser le buffer temporaire
    temp_buffer.enregs_utilises = 0;
    temp_buffer.metadata = buffer.metadata;

    // Parcourir les blocs et d�placer les enregistrements valides
    fseek(ms, buffer.metadata.adresse_premier * sizeof(BLOC), SEEK_SET);
    for (int i = 0; i < buffer.metadata.taille_blocs; i++) {
 if (fread(&buffer, sizeof(BLOC), 1, ms) != 1) {
            // V�rifier si la lecture du bloc a �chou�
            printf("Erreur : Lecture du bloc %d a �chou�.\n", i);
            return;
        }

        // Parcourir les enregistrements du bloc
        for (int j = 0; j < buffer.enregs_utilises; j++) {
            if (buffer.enregs[j].id != -1) { // V�rifier si l'enregistrement n'est pas marqu� comme supprim�
                // Copier l'enregistrement valide dans le buffer temporaire
                temp_buffer.enregs[temp_buffer.enregs_utilises++] = buffer.enregs[j];
                enregs_count++;

                // Si le buffer temporaire est plein, �crire dans le fichier de stockage
                if (temp_buffer.enregs_utilises == FACTEUR_BLOCS) {
                    fseek(ms, new_adresse_bloc * sizeof(BLOC), SEEK_SET);
                    fwrite(&temp_buffer, sizeof(BLOC), 1, ms);
                    // Mettre � jour la table d'allocation
                    miseAJourTableAllocation(ms, new_adresse_bloc, 0);
                    new_adresse_bloc++;
                    temp_buffer.enregs_utilises = 0;  // R�initialiser le buffer temporaire
                }
            }
        }

        // Si le bloc actuel devient vide, mettre � jour la table d'allocation
        if (buffer.enregs_utilises == 0) {
            miseAJourTableAllocation(ms, i, 1);
        }
    }

    // �crire le reste des enregistrements du buffer temporaire s'il n'est pas vide
    if (temp_buffer.enregs_utilises > 0) {
        fseek(ms, new_adresse_bloc * sizeof(BLOC), SEEK_SET);
        fwrite(&temp_buffer, sizeof(BLOC), 1, ms);
        // Mettre � jour la table d'allocation
        miseAJourTableAllocation(ms, new_adresse_bloc, 0);
        new_adresse_bloc++;
    }

    // Mise � jour des m�tadonn�es
    buffer.metadata.taille_blocs = (enregs_count + FACTEUR_BLOCS - 1) / FACTEUR_BLOCS;
    buffer.metadata.taille_enregs = enregs_count;
    fseek(ms, buffer.metadata.adresse_premier * sizeof(BLOC), SEEK_SET);
    fwrite(&buffer, sizeof(BLOC), 1, ms);

    printf("D�fragmentation termin�e pour le fichier '%s'.\n", nom_fichier);
}
