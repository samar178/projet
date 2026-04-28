#include "background.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Fonction utilitaire interne pour définir les tailles
void configurerTailleObstacle(Obstacle* obs, TypeObstacle type) {
    obs->type = type;
    if (type == BARRIERE) {
        obs->pos.w = 120; obs->pos.h = 120;
    } else if (type == VOITURE) {
        obs->pos.w = 220; obs->pos.h = 110;
    } else { // AVION_OBS (boite.png)
        obs->pos.w = 100; obs->pos.h = 100;
    }
}

void initBackground(Background* b, SDL_Renderer* renderer, int niveau) {
    char chemin[100];
    sprintf(chemin, "background/ressources/back_%d.png", niveau);
    b->image = IMG_LoadTexture(renderer, chemin);
    b->niveau_actuel = niveau;
    b->camera_pos = (SDL_Rect){0, 0, 1920, 1080};
    b->direction = 0;

    b->avion.tex = IMG_LoadTexture(renderer, "background/ressources/avion.png");
    b->avion.x = -500; b->avion.y = 100; b->avion.vitesse = 4.0f;
    b->virevolant.tex = IMG_LoadTexture(renderer, "background/ressources/virevolant.png");
    b->virevolant.x = 2500; b->virevolant.y = 800; b->virevolant.vitesse = -3.0f;

    initObstacles(b, renderer, niveau);

    b->font = TTF_OpenFont("background/ressources/font.ttf", 40); 
    b->startTime = SDL_GetTicks();
    b->tempsPos = (SDL_Rect){850, 50, 200, 50};
    b->tempsTex = NULL;
}

void initObstacles(Background* bg, SDL_Renderer* renderer, int niveau_actuel) {
    srand(time(NULL)); 
    bg->nb_obstacles = 0;
    
    bg->tex_obs_avion = IMG_LoadTexture(renderer, "background/ressources/boite.png"); 
    bg->tex_obs_voiture = IMG_LoadTexture(renderer, "background/ressources/voiture.png");
    bg->tex_obs_barriere = IMG_LoadTexture(renderer, "background/ressources/barriere.png");

    int limite_x = 9600 - 1920;
    int x_actuel = 1500; 
    int i = 0;

    if (niveau_actuel == 2) {
        // --- 1. DÉBUT DU NIVEAU 2 : PREMIER DOUBLE FIXE ---
        configurerTailleObstacle(&bg->tab_obstacles[i], VOITURE);
        bg->tab_obstacles[i].pos.x = x_actuel;
        bg->tab_obstacles[i].pos.y = 700 - bg->tab_obstacles[i].pos.h + 5;
        bg->tab_obstacles[i].actif = 1; i++;
        configurerTailleObstacle(&bg->tab_obstacles[i], BARRIERE);
        bg->tab_obstacles[i].pos.x = x_actuel + 240;
        bg->tab_obstacles[i].pos.y = 700 - bg->tab_obstacles[i].pos.h + 5;
        bg->tab_obstacles[i].actif = 1; i++;
        
        x_actuel += 800; // Espacement réduit dès le départ

        // --- 2. MILIEU : PLUS DENSE ET PLUS ALÉATOIRE ---
        while(x_actuel < limite_x - 3200 && i < MAX_OBSTACLES - 6) {
            // Augmentation de la probabilité de doubles obstacles (60% au lieu de 40%)
            int est_double = (rand() % 100) < 60; 

            configurerTailleObstacle(&bg->tab_obstacles[i], rand() % 3);
            bg->tab_obstacles[i].pos.x = x_actuel;
            bg->tab_obstacles[i].pos.y = 700 - bg->tab_obstacles[i].pos.h + 5;
            bg->tab_obstacles[i].actif = 1;
            int l = bg->tab_obstacles[i].pos.w; i++;

            if (est_double) {
                configurerTailleObstacle(&bg->tab_obstacles[i], rand() % 3);
                bg->tab_obstacles[i].pos.x = x_actuel + l + 20;
                bg->tab_obstacles[i].pos.y = 700 - bg->tab_obstacles[i].pos.h + 5;
                bg->tab_obstacles[i].actif = 1;
                l = bg->tab_obstacles[i].pos.w; i++;
            }

            // Espacement réduit (entre 450 et 750 pixels au lieu de 800+)
            x_actuel += l + (rand() % 300 + 450); 
        }

        // --- 3. FIN DU NIVEAU 2 : LES DEUX TRIPLES (TRIANGLES) ---
        for(int t = 0; t < 2; t++) {
            x_actuel += 400;
            configurerTailleObstacle(&bg->tab_obstacles[i], VOITURE);
            bg->tab_obstacles[i].pos.x = x_actuel;
            bg->tab_obstacles[i].pos.y = 700 - bg->tab_obstacles[i].pos.h + 5;
            bg->tab_obstacles[i].actif = 1; i++;
            configurerTailleObstacle(&bg->tab_obstacles[i], BARRIERE);
            bg->tab_obstacles[i].pos.x = x_actuel + 240;
            bg->tab_obstacles[i].pos.y = 700 - bg->tab_obstacles[i].pos.h + 5;
            bg->tab_obstacles[i].actif = 1; i++;
            configurerTailleObstacle(&bg->tab_obstacles[i], AVION_OBS);
            bg->tab_obstacles[i].pos.x = x_actuel + 100;
            bg->tab_obstacles[i].pos.y = 700 - 120 - 100; 
            bg->tab_obstacles[i].actif = 1; i++;
            
            x_actuel += 1000; 
        }
        bg->nb_obstacles = i;

    } else {
        // --- NIVEAU 1 : RESTE IDENTIQUE ---
        while(x_actuel < limite_x - 2800 && i < MAX_OBSTACLES - 4) {
            configurerTailleObstacle(&bg->tab_obstacles[i], i % 3);
            bg->tab_obstacles[i].pos.x = x_actuel;
            bg->tab_obstacles[i].pos.y = 700 - bg->tab_obstacles[i].pos.h + 5;
            bg->tab_obstacles[i].actif = 1;
            x_actuel += 900; i++;
        }
        x_actuel += 600; 
        for(int j = 0; j < 4; j++) {
            configurerTailleObstacle(&bg->tab_obstacles[i], (j % 2 == 0) ? VOITURE : BARRIERE);
            bg->tab_obstacles[i].pos.x = x_actuel;
            bg->tab_obstacles[i].pos.y = 700 - bg->tab_obstacles[i].pos.h + 5;
            bg->tab_obstacles[i].actif = 1;
            x_actuel += (j == 1) ? bg->tab_obstacles[i].pos.w + 450 : bg->tab_obstacles[i].pos.w + 20; 
            i++;
        }
        bg->nb_obstacles = i;
    }
}

// Les fonctions de rendu (scrolling, afficherObstacles, etc.) restent les mêmes
void scrolling(Background* b, int dx) {
    if (b->direction == 0 && b->camera_pos.x < 9600 - 1920) b->camera_pos.x += dx;
    else if (b->direction == 1 && b->camera_pos.x > 0) b->camera_pos.x -= dx;
}

void updateEtAfficherDecors(Background* b, SDL_Renderer* renderer) {
    b->avion.x += b->avion.vitesse;
    if(b->avion.x > 10000) b->avion.x = -1000;
    SDL_Rect r_av = {(int)b->avion.x - b->camera_pos.x, (int)b->avion.y, 200, 100};
    SDL_RenderCopy(renderer, b->avion.tex, NULL, &r_av);
    b->virevolant.x += b->virevolant.vitesse;
    if(b->virevolant.x < -1000) b->virevolant.x = 10000;
    SDL_Rect r_vi = {(int)b->virevolant.x - b->camera_pos.x, (int)b->virevolant.y, 100, 100};
    SDL_RenderCopy(renderer, b->virevolant.tex, NULL, &r_vi);
}

void afficherObstacles(Background b, SDL_Renderer* renderer) {
    for (int i = 0; i < b.nb_obstacles; i++) {
        if (b.tab_obstacles[i].actif) {
            SDL_Rect r = b.tab_obstacles[i].pos;
            r.x -= b.camera_pos.x;
            SDL_Texture* tex = NULL;
            if (b.tab_obstacles[i].type == BARRIERE) tex = b.tex_obs_barriere;
            else if (b.tab_obstacles[i].type == VOITURE) tex = b.tex_obs_voiture;
            else tex = b.tex_obs_avion;
            if (tex) SDL_RenderCopy(renderer, tex, NULL, &r);
        }
    }
}

void afficherBackground(Background b, SDL_Renderer* renderer) { SDL_RenderCopy(renderer, b.image, &b.camera_pos, NULL); }
void afficherBarreNiveau(Background* b, SDL_Renderer* renderer) {
    float progression = (float)b->camera_pos.x / (9600 - 1920);
    SDL_Rect contour = {50, 50, 300, 30}, plein = {50, 50, (int)(progression * 300), 30};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); SDL_RenderDrawRect(renderer, &contour);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); SDL_RenderFillRect(renderer, &plein);
}
void afficherTemps(Background* b, SDL_Renderer* renderer) {
    Uint32 elapsed = (SDL_GetTicks() - b->startTime) / 1000;
    char total[20]; sprintf(total, "TIME: %02d:%02d", elapsed / 60, elapsed % 60);
    SDL_Color blanc = {255, 255, 255}; SDL_Surface* s = TTF_RenderText_Blended(b->font, total, blanc);
    if (b->tempsTex) SDL_DestroyTexture(b->tempsTex);
    b->tempsTex = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, b->tempsTex, NULL, &b->tempsPos); SDL_FreeSurface(s);
}
void libererBackground(Background* b) {
    if (b->image) SDL_DestroyTexture(b->image);
    if (b->avion.tex) SDL_DestroyTexture(b->avion.tex);
    if (b->virevolant.tex) SDL_DestroyTexture(b->virevolant.tex);
    if (b->tex_obs_avion) SDL_DestroyTexture(b->tex_obs_avion);
    if (b->tex_obs_voiture) SDL_DestroyTexture(b->tex_obs_voiture);
    if (b->tex_obs_barriere) SDL_DestroyTexture(b->tex_obs_barriere);
    if (b->tempsTex) SDL_DestroyTexture(b->tempsTex);
    if (b->font) TTF_CloseFont(b->font);
}
int collisionTrigonometrique(SDL_Rect a, SDL_Rect b) {
    // 1. Centres (inchangés)
    float x1 = a.x + a.w / 2.0f;
    float y1 = a.y + a.h / 2.0f;
    float x2 = b.x + b.w / 2.0f;
    float y2 = b.y + b.h / 2.0f;

    // 2. Utilisation du CERCLE INSCRIT avec une légère augmentation
    // On calcule le rayon de base (inscrit)
    float r1_base = (a.w < a.h) ? (a.w / 2.0f) : (a.h / 2.0f);
    float r2_base = (b.w < b.h) ? (b.w / 2.0f) : (b.h / 2.0f);

    // On augmente les rayons de 20% (multiplié par 1.2)
    // C'est ici que vous pouvez ajuster : 1.1 pour moins, 1.3 pour plus
    float r1 = r1_base * 1.2f;
    float r2 = r2_base * 1.2f;

    // 3. Calcul de la distance (Pythagore)
    float distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

    // Collision si la distance est inférieure à la somme des rayons augmentés
    return (distance <= (r1 + r2));
}
