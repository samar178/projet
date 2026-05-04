#include "joueur.h"
#include <stdio.h>

void initJoueur(Joueur* j, SDL_Renderer* renderer) {
    char path[100];
    
    // --- Chargement des Textures ---
    j->texture_idle = IMG_LoadTexture(renderer, "joueur/ressources/debout_1.png");
    j->texture_accroupi = IMG_LoadTexture(renderer, "joueur/ressources/accroupi.png");

    for(int i = 0; i < 6; i++) {
        sprintf(path, "joueur/ressources/marche_%d.png", i + 1);
        j->anim_marche[i] = IMG_LoadTexture(renderer, path);
        sprintf(path, "joueur/ressources/recule_%d.png", i + 1);
        j->anim_recule[i] = IMG_LoadTexture(renderer, path);
    }

    for(int i = 0; i < 5; i++) {
        sprintf(path, "joueur/ressources/sprint_%d.png", i + 1); 
        j->anim_sprint[i] = IMG_LoadTexture(renderer, path);
        sprintf(path, "joueur/ressources/sprint_g_%d.png", i + 1); 
        j->anim_sprint_g[i] = IMG_LoadTexture(renderer, path);
        sprintf(path, "joueur/ressources/saut_%d.png", i + 1); 
        j->anim_saut[i] = IMG_LoadTexture(renderer, path);
        sprintf(path, "joueur/ressources/frappe_%d.png", i + 1); 
        j->anim_frappe[i] = IMG_LoadTexture(renderer, path);
        sprintf(path, "joueur/ressources/frappe_g_%d.png", i + 1); 
        j->anim_frappe_g[i] = IMG_LoadTexture(renderer, path);
        
        sprintf(path, "joueur/ressources/mort_%d.png", i + 1); 
        j->anim_mort[i] = IMG_LoadTexture(renderer, path);
    }

    // --- Statistiques et Physique ---
    j->y_sol = 700; 
    j->pos = (SDL_Rect){100, 700 - 200, 150, 200}; 
    j->vie = 3;
    j->sante = 100.0f;
    j->endurance = 100.0f;
    j->invulnerable = 0;
    j->mort_terminee = 0;
    j->score = 0; // NOUVEAU : Initialisation du score
    
    j->etat = IDLE;
    j->direction = 0; 
    j->frame = 0;
    j->timer = 0;
    j->vel_y = 0;
    j->nb_sauts = 0;
}

void gestionEntrees(Joueur* j, const Uint8* keys, SDL_Event* e) {
    if (j->etat == MORT) return;

    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        if (e->key.keysym.scancode == SDL_SCANCODE_X && j->etat != FRAPPE && j->etat != FRAPPE_G) {
            j->etat = (j->direction == 1) ? FRAPPE_G : FRAPPE;
            j->frame = 0; j->timer = 0;
            return;
        }
        if (e->key.keysym.scancode == SDL_SCANCODE_SPACE && j->nb_sauts < 2) {
            j->vel_y = -22; 
            j->nb_sauts++;
            j->etat = SAUT;
            j->frame = 0;
        }
    }

    if (j->etat == FRAPPE || j->etat == FRAPPE_G) return;

    if (keys[SDL_SCANCODE_LCTRL]) {
        j->etat = ACCROUPI;
    } 
    else if (keys[SDL_SCANCODE_RIGHT]) {
        j->direction = 0;
        j->etat = (keys[SDL_SCANCODE_LSHIFT]) ? SPRINT : MARCHE;
    } 
    else if (keys[SDL_SCANCODE_LEFT]) {
        j->direction = 1;
        j->etat = (keys[SDL_SCANCODE_LSHIFT]) ? SPRINT_G : MARCHE; 
    } 
    else if (j->etat != SAUT) {
        j->etat = IDLE;
    }
}

void updatePhysique(Joueur* j) {
    if (j->etat == MORT) {
        j->timer++;
        if (j->timer > 12) {
            j->timer = 0;
            if (j->frame < 4) j->frame++;
            else j->mort_terminee = 1;
        }
        return; 
    }

    j->pos.h = (j->etat == ACCROUPI) ? 100 : 200;

    j->vel_y += 1.2; 
    j->y_sol += (int)j->vel_y;

    if (j->y_sol >= 700) {
        j->y_sol = 700;
        j->vel_y = 0;
        j->nb_sauts = 0;
        if (j->etat == SAUT) j->etat = IDLE;
    }

    j->pos.y = j->y_sol - j->pos.h;

    if (j->invulnerable > 0) j->invulnerable--;

    j->timer++;
    int vitesseAnim = (j->etat == SPRINT || j->etat == SPRINT_G) ? 5 : 8;
    
    if (j->timer > vitesseAnim) {
        j->timer = 0;
        j->frame++;
        
        if ((j->etat == FRAPPE || j->etat == FRAPPE_G) && j->frame >= 5) {
            j->etat = IDLE;
            j->frame = 0;
        }
    }
}

void afficherJoueur(Joueur j, SDL_Renderer* renderer) {
    if (j.invulnerable > 0 && (SDL_GetTicks() / 100) % 2 == 0) return;

    SDL_Texture* t = j.texture_idle;
    SDL_RendererFlip flip = (j.direction == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    switch(j.etat) {
        case MARCHE:   t = j.anim_marche[j.frame % 6]; break;
        case RECULE:   t = j.anim_recule[j.frame % 6]; break;
        case SPRINT:   t = j.anim_sprint[j.frame % 5]; break;
        case SPRINT_G: t = j.anim_sprint_g[j.frame % 5]; flip = SDL_FLIP_NONE; break;
        case SAUT:     t = j.anim_saut[j.frame % 5];   break;
        case ACCROUPI: t = j.texture_accroupi; break;
        case FRAPPE:   t = j.anim_frappe[j.frame % 5]; flip = SDL_FLIP_NONE; break;
        case FRAPPE_G: t = j.anim_frappe_g[j.frame % 5]; flip = SDL_FLIP_NONE; break;
        case MORT:     t = j.anim_mort[j.frame % 5]; break;
        default:       t = j.texture_idle; break;
    }

    if (t) SDL_RenderCopyEx(renderer, t, NULL, &j.pos, 0, NULL, flip);
}

void libererJoueur(Joueur* j) {
    SDL_DestroyTexture(j->texture_idle);
    SDL_DestroyTexture(j->texture_accroupi);
    for(int i = 0; i < 6; i++) {
        SDL_DestroyTexture(j->anim_marche[i]);
        SDL_DestroyTexture(j->anim_recule[i]);
    }
    for(int i = 0; i < 5; i++) {
        SDL_DestroyTexture(j->anim_sprint[i]);
        SDL_DestroyTexture(j->anim_sprint_g[i]);
        SDL_DestroyTexture(j->anim_saut[i]);
        SDL_DestroyTexture(j->anim_frappe[i]);
        SDL_DestroyTexture(j->anim_frappe_g[i]);
        SDL_DestroyTexture(j->anim_mort[i]);
    }
}
