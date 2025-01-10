# projetSFSD
Projet TP : Simulateur Simplifié d’un Système de Gestion de  Fichiers (SGF)
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

