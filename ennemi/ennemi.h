#ifndef ENNEMI_H
#define ENNEMI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef enum { E_PATROUILLE, E_ATTAQUE, E_MORT } EtatEnnemi;

typedef struct {
    SDL_Texture* anim_marche[6];
    SDL_Texture* anim_frappe[5];
    SDL_Texture* anim_mort[5];
    
    SDL_Rect pos;      
    int frame;         
    int timer;         
    int direction;     
    float sante;
    float endurance;
    int posMin, posMax; 
    int vitesse;
    EtatEnnemi etat;
    int actif; 
    int mort_terminee;
    int y_sol;
} Ennemi;

void initEnnemi(Ennemi* e, SDL_Renderer* renderer, int x_depart);
void updateEnnemi(Ennemi* e);
void afficherEnnemi(Ennemi e, SDL_Renderer* renderer, SDL_Rect camera);
void libererEnnemi(Ennemi* e);

#endif
