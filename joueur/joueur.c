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
        
        // AJOUT : Chargement des images de mort
        sprintf(path, "mort_%d.png", i + 1);
        j->anim_mort[i] = IMG_LoadTexture(renderer, path);
    }

    // --- Statistiques et Physique ---
    j->y_sol = 750; 
    j->pos = (SDL_Rect){100, j->y_sol - 200, 150, 200};
    j->vie = 3;
    j->sante = 100.0f;      // Barre rouge
    j->endurance = 100.0f;  // Barre verte
    j->invulnerable = 0;
    j->mort_terminee = 0;
    
    j->etat = IDLE;
    j->direction = 0; 
    j->frame = 0;
    j->timer = 0;
    j->vel_y = 0;
    j->nb_sauts = 0;
}

void gestionEntrees(Joueur* j, const Uint8* keys, SDL_Event* e) {
    // Si le joueur est en train de mourir, on bloque tout
    if (j->etat == MORT) return;

    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        // Attaque (Touche X)
        if (e->key.keysym.scancode == SDL_SCANCODE_X && j->etat != FRAPPE && j->etat != FRAPPE_G) {
            j->etat = (j->direction == 1) ? FRAPPE_G : FRAPPE;
            j->frame = 0; j->timer = 0;
            return;
        }
        // Saut (Espace)
        if (e->key.keysym.scancode == SDL_SCANCODE_SPACE && j->nb_sauts < 2) {
            j->vel_y = -20; 
            j->nb_sauts++;
            j->etat = SAUT;
            j->frame = 0;
        }
    }

    // Si on est en train de frapper, on ne peut pas bouger
    if (j->etat == FRAPPE || j->etat == FRAPPE_G) return;

    if (keys[SDL_SCANCODE_LCTRL]) {
        j->etat = ACCROUPI;
    } 
    else if (keys[SDL_SCANCODE_RIGHT]) {
        j->direction = 0;
        j->etat = keys[SDL_SCANCODE_LSHIFT] ? SPRINT : MARCHE;
    } 
    else if (keys[SDL_SCANCODE_LEFT]) {
        j->direction = 1;
        j->etat = keys[SDL_SCANCODE_LSHIFT] ? SPRINT_G : MARCHE; 
    } 
    else {
        j->etat = IDLE;
    }
}

void updatePhysique(Joueur* j) {
    // 1. Gestion de la Mort
    if (j->etat == MORT) {
        j->timer++;
        if (j->timer > 12) { // Vitesse lente pour la mort
            j->timer = 0;
            if (j->frame < 4) j->frame++;
            else j->mort_terminee = 1;
        }
        return; 
    }

    // 2. Gravité et Saut
    j->y_sol += (int)j->vel_y;
    j->pos.h = (j->etat == ACCROUPI) ? 100 : 200;

    if (j->y_sol > 700) {
        j->y_sol = 700;
        j->vel_y = 0;
        j->nb_sauts = 0;
    } else {
        j->vel_y += 1; // Gravité
    }

    j->pos.y = j->y_sol - j->pos.h;

    // 3. Animation et Invulnérabilité
    if (j->invulnerable > 0) j->invulnerable--;

    j->timer++;
    if (j->timer > 8) {
        j->timer = 0;
        j->frame++;
        
        // Reset des animations d'attaque
        if ((j->etat == FRAPPE || j->etat == FRAPPE_G) && j->frame >= 5) {
            j->etat = IDLE;
            j->frame = 0;
        }
    }
}

void afficherJoueur(Joueur j, SDL_Renderer* renderer) {
    // Effet de clignotement si blessé
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
        case MORT:     t = j.anim_mort[j.frame % 5]; break; // Animation de mort
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
        SDL_DestroyTexture(j->anim_mort[i]); // Libération mort
    }
}
