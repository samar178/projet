#ifndef JOUEUR_H
#define JOUEUR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Définition des états du joueur
typedef enum {
    IDLE, MARCHE, RECULE, SPRINT, SPRINT_G,
    SAUT, ACCROUPI, FRAPPE, FRAPPE_G, MORT
} EtatJoueur;

typedef struct {
    SDL_Rect pos;
    SDL_Texture* texture_idle;
    SDL_Texture* texture_accroupi;
    
    // Tableaux d'animations
    SDL_Texture* anim_marche[6];
    SDL_Texture* anim_recule[6];
    SDL_Texture* anim_sprint[5];
    SDL_Texture* anim_sprint_g[5];
    SDL_Texture* anim_saut[5];
    SDL_Texture* anim_frappe[5];
    SDL_Texture* anim_frappe_g[5];
    SDL_Texture* anim_mort[5];

    // Paramètres physiques
    int y_sol;
    float vel_y;
    int nb_sauts;
    
    // Logique interne
    EtatJoueur etat;
    int direction; // 0: droite, 1: gauche
    int frame;
    int timer;
    
    // Statistiques de jeu
    int vie;
    float sante;      
    float endurance;  
    int invulnerable;
    int mort_terminee; 
    
    int score; // NOUVEAU : Variable pour stocker le score total
    
} Joueur;

void initJoueur(Joueur* j, SDL_Renderer* renderer);
void gestionEntrees(Joueur* j, const Uint8* keys, SDL_Event* e);
void updatePhysique(Joueur* j);
void afficherJoueur(Joueur j, SDL_Renderer* renderer);
void libererJoueur(Joueur* j);

#endif
