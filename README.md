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
