#include "ennemi.h"
#include <stdio.h>
#include <stdlib.h> // AJOUT : Nécessaire pour la fonction abs()
#include "../background/background.h"

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

    e->y_sol = 700; 
    e->pos = (SDL_Rect){x_depart, e->y_sol - 200, 150, 200};
    e->sante = 100.0f;
    e->endurance = 100.0f; 
    e->posMin = x_depart - 400;
    e->posMax = x_depart + 400;
    e->vitesse = 4; // Tu peux l'augmenter si tu veux qu'il coure vers toi !
    e->direction = 1; 
    e->frame = 0; e->timer = 0;
    e->etat = E_PATROUILLE;
    e->actif = 0; 
    e->mort_terminee = 0;
}

void updateEnnemi(Ennemi* e, SDL_Rect player_hitbox) {
    if (e->etat == E_MORT) {
        e->timer++;
        if (e->timer > 10) {
            if (e->frame < 4) e->frame++;
            else e->mort_terminee = 1;
            e->timer = 0;
        }
        return;
    }

    // 1. Sauvegarde la position avant mouvement pour le blocage
    int old_ex = e->pos.x;

    // --- NOUVEAU : SYSTÈME DE VISION ET POURSUITE ---
    int centre_ennemi = e->pos.x + (e->pos.w / 2);
    int centre_joueur = player_hitbox.x + (player_hitbox.w / 2);
    int distance = abs(centre_ennemi - centre_joueur);
    
    int portee_vision = 600; // L'ennemi détecte le joueur à 600 pixels

    // 2. Logique de mouvement
    if (distance < portee_vision && e->etat != E_ATTAQUE) {
        // Le joueur est repéré ! L'ennemi se tourne vers lui et le poursuit
        if (centre_joueur < centre_ennemi) {
            e->direction = 1; // Le joueur est à gauche, tourne à gauche
            e->pos.x -= e->vitesse;
        } else {
            e->direction = 0; // Le joueur est à droite, tourne à droite
            e->pos.x += e->vitesse;
        }
    } else if (e->etat == E_PATROUILLE) {
        // S'il ne voit pas le joueur, patrouille normale entre posMin et posMax
        if (e->direction == 0) {
            e->pos.x += e->vitesse;
            if (e->pos.x >= e->posMax) e->direction = 1;
        } else {
            e->pos.x -= e->vitesse;
            if (e->pos.x <= e->posMin) e->direction = 0;
        }
    }

    // 3. COLLISION ET BLOCAGE MUTUEL
    // Si l'ennemi touche le joueur, il revient à sa position précédente et attaque
    if (collisionTrigonometrique(e->pos, player_hitbox)) {
        e->pos.x = old_ex; 
        e->etat = E_ATTAQUE;
    } else {
        e->etat = E_PATROUILLE;
    }

    // Gestion de l'animation
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
