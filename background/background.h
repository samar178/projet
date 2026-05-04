#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX_OBSTACLES 40

typedef enum { AVION_OBS, VOITURE, BARRIERE } TypeObstacle;

typedef struct {
    TypeObstacle type;
    SDL_Rect pos;
    int actif;
} Obstacle;

typedef struct {
    float x, y;
    float vitesse;
    SDL_Texture* tex;
} ObjetDecor;

typedef struct {
    SDL_Texture* image;      // Était manquant ou mal nommé
    SDL_Rect camera_pos;
    int direction;           // Était manquant
    int niveau_actuel;
    
    ObjetDecor avion;
    ObjetDecor virevolant;
    
    Obstacle tab_obstacles[MAX_OBSTACLES];
    int nb_obstacles;
    SDL_Texture* tex_obs_avion;
    SDL_Texture* tex_obs_voiture;
    SDL_Texture* tex_obs_barriere;
    
    TTF_Font* font;
    Uint32 startTime;
    Uint32 tempsEcoule;
    SDL_Rect tempsPos;
    SDL_Texture* tempsTex;
} Background;

// Vérifie que tous ces prototypes sont présents pour éviter les "implicit declaration"
void initBackground(Background* b, SDL_Renderer* renderer, int niveau);
void initObstacles(Background* b, SDL_Renderer* renderer, int niveau_actuel);
void scrolling(Background* b, int dx);
void updateEtAfficherDecors(Background* b, SDL_Renderer* renderer);
void afficherObstacles(Background b, SDL_Renderer* renderer);
void afficherBackground(Background b, SDL_Renderer* renderer);
void afficherBarreNiveau(Background* b, SDL_Renderer* renderer);
void afficherTemps(Background* b, SDL_Renderer* renderer);
void libererBackground(Background* b);
int collisionTrigonometrique(SDL_Rect a, SDL_Rect b);

#endif
