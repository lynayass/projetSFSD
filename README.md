# Simulateur Simplifié de Système de Gestion des Fichiers

## Introduction
Ce projet a pour but de développer un **simulateur simplifié de système de gestion de fichiers (SGF)**. Un SGF est essentiel pour les systèmes d'exploitation, facilitant le stockage, l'organisation et la manipulation des fichiers sur une mémoire secondaire. Le simulateur vise à modéliser les principes fondamentaux de gestion des fichiers, incluant l'organisation de la mémoire, l'allocation de l'espace, la gestion des métadonnées et les opérations courantes sur les fichiers.

## Objectifs du Projet
- Assimiler les concepts de base des systèmes de gestion de fichiers.
- Manipuler des structures de données (tableaux, listes chaînées) dans le cadre de la gestion des fichiers.
- Mettre en œuvre des stratégies d'allocation de mémoire secondaire.
- Développer des algorithmes pour les opérations sur les fichiers.
- Renforcer les compétences en programmation en langage C.
- Rédiger un compte rendu de travaux pratiques.

## Détails du Système

### Mémoire Secondaire (MS)
#### Structure et Organisation
La mémoire secondaire est représentée par un disque virtuel divisé en blocs de taille fixe, numérotés de manière séquentielle. Une table d'allocation gère l'état de chaque bloc (libre ou occupé) et est stockée dans le premier champ du bloc 1, car le numéro du bloc est égal au numéro de l'enregistrement.

### Modes d'Organisation des Fichiers
- **Organisation Contiguë** : Les enregistrements d'un fichier sont stockés dans des blocs adjacents.
- **Organisation Chaînée** : Les blocs d'un fichier peuvent être dispersés avec des pointeurs vers le bloc suivant.

## Fonctionnalités
- **Initialisation du Disque** : Définir le nombre de blocs et leur taille. Tous les blocs sont initialement libres.
- **Table d'Allocation** : Mise à jour de l'état des blocs après chaque opération.
- **Compactage** : Réorganisation des fichiers pour éliminer les espaces vides.
- **Effacement** : Vider entièrement le disque.
- **Vérification de l'Espace** : Avant toute opération, vérifier la disponibilité des blocs. Proposer un compactage en cas de fragmentation excessive.

## Gestion des Fichiers de Données
Un fichier de données est une collection d'enregistrements stockés en mémoire selon une organisation définie par l'utilisateur. Chaque enregistrement a une taille fixe, un identifiant unique (ID) et plusieurs champs.

### Opérations sur les Fichiers
- **Création** : Demander les détails du fichier (nom, nombre d'enregistrements, modes d'organisation).
- **Chargement** : Allouer les blocs nécessaires.
- **Insertion** : Suivre les modes d'organisation lors de l'insertion.
- **Recherche par ID** : Retourner la position ou indiquer l'absence de l'enregistrement.
- **Suppression** :
  - Logique : Marquer comme supprimé.
  - Physique : Réorganiser pour libérer l'espace.
- **Défragmentation** : Réorganiser pour récupérer l'espace inutilisé après suppression.
- **Renommage** : Modifier le nom d'un fichier.
- **Suppression** : Retirer un fichier.

## Gestion des Métadonnées
Chaque fichier de données est associé à un fichier de métadonnées, stocké séparément, contenant des informations de gestion. Le nom du fichier est stocké dans chaque bloc du fichier, mais les autres informations de métadonnées sont uniquement stockées dans le premier bloc du fichier.

### Contenu des Métadonnées
- Nom du fichier
- Taille en blocs et en enregistrements (stockées uniquement dans le premier bloc)
- Adresse du premier bloc (stockée uniquement dans le premier bloc)
- Modes d'organisation (stockés uniquement dans le premier bloc)

## Interface Utilisateur
Le simulateur offrira un menu interactif avec les options suivantes :
- Initialiser la mémoire secondaire.
- Créer et charger un fichier.
- Afficher l'état de la mémoire (blocs libres/occupés).
- Afficher les métadonnées des fichiers.
- Rechercher un enregistrement par ID.
- Insérer un enregistrement.
- Supprimer un enregistrement (logique/physique).
- Défragmenter un fichier.
- Supprimer ou renommer un fichier.
- Compactage de la mémoire.
- Vider la mémoire.
- Quitter le programme.
# Installation de SDL2, SDL2_ttf et des Polices (TrueType)

Ce guide décrit les étapes pour installer **SDL2**, **SDL2_ttf**, et une police TrueType sur **Windows** et **Linux**. Vous apprendrez à configurer MinGW sur Windows et à utiliser des gestionnaires de paquets sur Linux pour compiler et lier vos programmes avec SDL2 et SDL2_ttf.

## Prérequis

- **Windows** : MinGW (compilateur GCC pour Windows)
- **Linux** : MinGW ou gestionnaires de paquets natifs (`libsdl2-dev`, `libsdl2-ttf-dev`)
- **Polices** : Police TrueType (ex. **DejaVuSans.ttf**)

---

## Installation sur **Windows** (avec MinGW)

### Étape 1 : Télécharger les bibliothèques
- Téléchargez **SDL2** MinGW depuis [SDL2 Downloads](https://www.libsdl.org/download-2.0.php) (choisissez `SDL2-devel-x.x.x-mingw.tar.gz`).
- Téléchargez **SDL2_ttf** MinGW depuis [SDL2_ttf Downloads](https://www.libsdl.org/projects/SDL_ttf/) (choisissez `SDL2_ttf-devel-x.x.x-mingw.tar.gz`).

### Étape 2 : Extraire les fichiers
- Décompressez les fichiers `.tar.gz` avec un outil comme **7-Zip** ou **WinRAR**.
- Placez les dossiers extraits dans des répertoires comme `C:\SDL2` et `C:\SDL2_ttf`.

### Étape 3 : Configurer MinGW
- **Inclure les répertoires** : Ajoutez les chemins vers les en-têtes (`include`) de SDL2 et SDL2_ttf dans MinGW.
  - Exemple : `-I C:/SDL2/include -I C:/SDL2_ttf/include`
  
- **Lier les bibliothèques** : Ajoutez les chemins vers les bibliothèques (`lib`) et liez les fichiers `.a` ou `.lib`.
  - Exemple : `-L C:/SDL2/lib -L C:/SDL2_ttf/lib -lSDL2 -lSDL2_ttf`
### Étape 4 :Télécharger et utiliser une police
Téléchargez une police comme DejaVuSans.ttf depuis Google Fonts.
Placez-la dans le répertoire de votre projet

### Étape 5 : Compiler le programme
Exécutez la commande suivante pour compiler et lier votre programme :

```bash
gcc -o mon_programme mon_programme.c -I C:/SDL2/include -I C:/SDL2_ttf/include -L C:/SDL2/lib -L C:/SDL2_ttf/lib -lSDL2 -lSDL2_ttf -mwindows

