#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<SDL.h>
#include<string.h>
#include<SDL_ttf.h>
#define WINDOW_LARGEUR 500
#define WINDOW_HAUTEUR 500
#define largeur 100
#define hauteur 120
#define FACTEUR_BLOCS 15
#define MAX_BLOCS 15


     //Déclaration des fonctions
    void SDL_ExitWithErrors(const char *message);
    void ecrireBlocs(SDL_Renderer *renderer,SDL_Window *fenetre,char* buffer,
     int isoccupied,SDL_Rect rectangle);
    void dessinerBlocs(SDL_Renderer *renderer,int isoccupied,SDL_Rect rectangle);
   
   
    int main (int argc,char **argv){
    
   //Initialisation de SDL 
    SDL_Init(SDL_INIT_VIDEO);
    //Systeme de gestion d'erreur
    if(SDL_Init(SDL_INIT_VIDEO)!=0)
    SDL_ExitWithErrors("Initialisation SDL");
    //Initialisation de TTF
    TTF_Init();
    //Systeme de gestion d'erreur
    if(TTF_Init()!=0){
       TTF_Quit();
    SDL_ExitWithErrors("Initialisation TTF");}
    
    // Création de la fenetre
    SDL_Window *fenetre=NULL;
    fenetre= SDL_CreateWindow("L'état de la memoire secondaire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,WINDOW_LARGEUR ,WINDOW_HAUTEUR,SDL_WINDOW_SHOWN );
    //Systeme de gestion d'erreur 
     if( fenetre==NULL){
     SDL_DestroyWindow(fenetre);
      TTF_Quit();
    SDL_ExitWithErrors("Création fenetre");}
    // Création du rendu
    SDL_Renderer *renderer=NULL;
    renderer = SDL_CreateRenderer(fenetre,-1, SDL_RENDERER_SOFTWARE);
    //Systeme de gestion d'erreur 
     if( renderer==NULL)
    { SDL_DestroyWindow(fenetre);
     TTF_Quit();
    SDL_ExitWithErrors("Création rendu"); }
    
           
    //Creation d'un rectangle
   SDL_Rect rectangle;
   // Position initiale du rectangle
    rectangle.x=0;
    rectangle.y=0;
    //Dimensions du rectangle
     rectangle.w=largeur;
     rectangle.h=hauteur;
    
      char buffer[300]; BLOC buffer1;
     rewind(ms);
     //Récupérer les informations de la ms et dessiner l'état de la ms
     if(!fread(&table_allocation,sizeof(BLOC),1,ms))
     {printf("Erreur lors de la lecture de la memoire secondaire");}
     else{ 
     for(int j=1;j<FACTEUR_BLOCS;j++){ 
       
        if(table_allocation.enregs[j].id==1){ //Le bloc est occupé!
            fseek(ms,j*sizeof(BLOC),SEEK_SET);
            if(! fread(&buffer1,sizeof(BLOC),1,ms))
           {printf("error");}
            snprintf(buffer,sizeof(buffer),"%s:%d enrg",buffer1.metadata.fichier_nom,buffer1.enregs_utilises); //Copier les informations à écrire sur le rectangle dans un buffer
            ecrireBlocs(renderer,fenetre, buffer,table_allocation.enregs[j].id,rectangle); //Dessiner et écrire sur le rectangle
        }else{
            sprintf(buffer,"Libre"); //Le bloc est libre!
             ecrireBlocs(renderer,fenetre, buffer,table_allocation.enregs[j].id,rectangle);
        } //Préparation de la position du prochain rectangle
     rectangle.x= rectangle.x+ rectangle.w;
     if( rectangle.x==WINDOW_LARGEUR) //Sauter la ligne si on arrive à la largeur de la fenetre
     {    rectangle.x=0;
          rectangle.y= rectangle.y+ rectangle.h;}
     }

     }

      SDL_RenderPresent(renderer); //Afficher le résultat sur la fenetre
   
     //Gérer la fermeture de la fenetre
    SDL_Event evenement; 
        while(1){
        SDL_PollEvent(&evenement);
        if(evenement.type== SDL_QUIT){break;};}
         
       //Nettoyer l'écran
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        //Libérer les ressources
      SDL_DestroyRenderer(renderer); 
      SDL_DestroyWindow(fenetre); 
      TTF_Quit();
      SDL_Quit();
    return 0;}
    //Fonction de gestion d'erreurs
    void SDL_ExitWithErrors(const char *message)
    { SDL_Log("ERREUR: %s, %s\n", message, SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);}
    
     //Fonction pour écrire dans les blocs(les rectangles)

     void ecrireBlocs(SDL_Renderer *renderer,SDL_Window *fenetre,char* buffer,
     int isoccupied,SDL_Rect rectangle)
     { 
        //Création de la police 
    TTF_Font *font=NULL;
    font=TTF_OpenFont("src/police.ttf",10);
       //Systeme de gestion d'erreur 
     if( font==NULL)
    {  SDL_DestroyRenderer(renderer); 
    SDL_DestroyWindow(fenetre);
     TTF_Quit();
    SDL_ExitWithErrors("Création de la police"); }

     //Choisir la couleur du texte
    SDL_Color fontNoir={0,0,0}; //NOIR
     //Création de la zone texte
       //Création d'une surface pour le texte
     SDL_Surface *text=NULL;
     text=TTF_RenderText_Blended(font,buffer,fontNoir);
    if( text==NULL)
    {  SDL_DestroyRenderer(renderer); 
    SDL_DestroyWindow(fenetre);
     TTF_Quit();
    SDL_ExitWithErrors("Création rendu"); }
     //Convertir la surface en texture 
    SDL_Texture *textTexture=NULL;
    textTexture=SDL_CreateTextureFromSurface(renderer,text);
     //Systeme de gestion d'erreur 
     if( textTexture==NULL)
    {  SDL_DestroyRenderer(renderer); 
    SDL_DestroyWindow(fenetre);
     TTF_Quit();
    SDL_ExitWithErrors("Création de la texture"); }
   
    //Appeller la fonction déssiner blocs
    dessinerBlocs(renderer, isoccupied, rectangle);
    //Définir la position du texte
    SDL_Rect textPosition={ rectangle.x + (rectangle.w - text->w) / 2,rectangle.y + (rectangle.h - text->h) / 2,text->w,text->h};
    //Déssiner le texte
    SDL_RenderCopy(renderer,textTexture,NULL,&textPosition);
   //Libérer de la surface et la texture
   SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(text);
    }
   //Fonction pour déssiner les blocs
   void dessinerBlocs(SDL_Renderer *renderer,int isoccupied,SDL_Rect rectangle){
   
    if(isoccupied==0)//Le bloc est libre!
   { 
   if(SDL_SetRenderDrawColor(renderer,0,200,0,255)!=0) //Définir une couleur pour le rectangle:VERT
      SDL_ExitWithErrors("Impossible de changer la couleur pour le rendu");
      if(SDL_RenderFillRect(renderer,&rectangle)!=0)//Colorier le rectangle
      SDL_ExitWithErrors("Impossible de colorier le rectangle");
      
      if(SDL_SetRenderDrawColor(renderer,0,0,0,255)!=0)//Définir une couleur pour le contour:NOIR
      SDL_ExitWithErrors("Impossible de changer la couleur pour le rendu");
      if(SDL_RenderDrawRect(renderer,&rectangle)!=0)//Dessiner le contour du rectangle
      SDL_ExitWithErrors("Impossible de dessiner le contour du rectangle");
      }
    else {//Le bloc est occupé!
      
     if(SDL_SetRenderDrawColor(renderer,200,0,0,0)!=0)//Définir une couleur pour le rectangle:ROUGE
     SDL_ExitWithErrors("Impossible de changer la couleur pour le rendu");
     if(SDL_RenderFillRect(renderer,&rectangle)!=0)
     SDL_ExitWithErrors("Impossible de dessiner le rectangle");
     
     if(SDL_SetRenderDrawColor(renderer,0,0,0,255)!=0)
      SDL_ExitWithErrors("Impossible de changer la couleur pour le rendu");
      if(SDL_RenderDrawRect(renderer,&rectangle)!=0)
      SDL_ExitWithErrors("Impossible de dessiner le contour du  rectangle"); } 
     

   } 