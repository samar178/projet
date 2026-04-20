#include "ennemi.h"
#include <stdio.h>

void initEnnemi(Ennemi* e, SDL_Renderer* renderer, int x_depart) {
    char path[100];
    for(int i = 0; i < 6; i++) {
        sprintf(path, "ennemi/ressources/marche_%d.png", i + 1);
        e->anim_marche[i] = IMG_LoadTexture(renderer, path);
    }
    for(int i = 0; i < 5; i++) {
        sprintf(path, "ennemi/ressources/frappe_%d.png", i + 1);
        e->anim_frappe[i] = IMG_LoadTexture(renderer, path);
        sprintf(path, "ennemi/ressources/mort_%d.png", i + 1);
        e->anim_mort[i] = IMG_LoadTexture(renderer, path);
    }

    e->y_sol = 650; 
    e->pos = (SDL_Rect){x_depart, e->y_sol - 200, 150, 200};
    
    e->posMin = x_depart - 400;
    e->posMax = x_depart + 400;
    e->vitesse = 4;
    e->direction = 1; 
    e->frame = 0; e->timer = 0;
    e->etat = E_PATROUILLE;
    e->actif = 0; e->mort_terminee = 0;
}

void updateEnnemi(Ennemi* e) {
    if (e->etat == E_MORT) {
        e->timer++;
        if (e->timer > 10) {
            if (e->frame < 4) {
                e->frame++;
            } else {
                e->mort_terminee = 1; // Déclencheur du déverrouillage
            }
            e->timer = 0;
        }
        return;
    }

    if (e->direction == 0) {
        e->pos.x += e->vitesse;
        if (e->pos.x >= e->posMax) e->direction = 1;
    } else {
        e->pos.x -= e->vitesse;
        if (e->pos.x <= e->posMin) e->direction = 0;
    }

    e->timer++;
    if (e->timer > 8) {
        e->frame = (e->frame + 1) % 6;
        e->timer = 0;
    }
}

void afficherEnnemi(Ennemi e, SDL_Renderer* renderer, SDL_Rect camera) {
    if (e.mort_terminee) return;
    SDL_Rect pos_rel = e.pos;
    pos_rel.x -= camera.x;
    if (pos_rel.x < -200 || pos_rel.x > 1920) return;

    SDL_Texture* t = e.anim_marche[e.frame % 6];
    if (e.etat == E_MORT) t = e.anim_mort[e.frame % 5];
    else if (e.etat == E_ATTAQUE) t = e.anim_frappe[e.frame % 5];

    SDL_RendererFlip flip = (e.direction == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, t, NULL, &pos_rel, 0, NULL, flip);
}

void libererEnnemi(Ennemi* e) {
    for(int i=0; i<6; i++) if(e->anim_marche[i]) SDL_DestroyTexture(e->anim_marche[i]);
    for(int i=0; i<5; i++) {
        if(e->anim_frappe[i]) SDL_DestroyTexture(e->anim_frappe[i]);
        if(e->anim_mort[i]) SDL_DestroyTexture(e->anim_mort[i]);
    }
}
