#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include "background/background.h"
#include "joueur/joueur.h"
#include "ennemi/ennemi.h"
#include "minimap/minimap.h"

// Fonction de collision (Hitbox)
int testCollision(SDL_Rect a, SDL_Rect b) {
    if (a.x + a.w <= b.x || a.x >= b.x + b.w || a.y + a.h <= b.y || a.y >= b.y + b.h) return 0;
    return 1;
}

int main(int argc, char* argv[]) {
    // --- INITIALISATION ---
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return 1;
    if (TTF_Init() == -1) return 1;

    SDL_Window* window = SDL_CreateWindow("7SINS - Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // --- VARIABLES ---
    Background bg; 
    Joueur j; 
    Ennemi e20; 
    Minimap mn;

    const int SEUIL_VERROUILLAGE = 7680; // 9600 - 1920

    initBackground(&bg, renderer, 1); 
    initJoueur(&j, renderer); 
    initEnnemi(&e20, renderer, SEUIL_VERROUILLAGE + 960); 
    initMinimap(&mn, renderer);

    int run = 1; 
    SDL_Event event;
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    while (run) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) run = 0;
            
            // CONTRAINTE : Touche K pour tuer l'ennemi (Debug)
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_k) {
                e20.etat = E_MORT;
                e20.frame = 0;
            }
            gestionEntrees(&j, keys, &event);
        }

        // Détermination de la vitesse (Marche ou Sprint)
        int v = (j.etat == SPRINT || j.etat == SPRINT_G) ? 14 : 8;
        int absX_joueur = j.pos.x + bg.camera_pos.x;

        // --- LOGIQUE DE MOUVEMENT ET VERROUILLAGE ---
        if (keys[SDL_SCANCODE_RIGHT]) {
            bg.direction = 0;
            // SI on atteint le seuil ET que l'ennemi est vivant
            if (bg.camera_pos.x >= SEUIL_VERROUILLAGE && !e20.mort_terminee) {
                bg.camera_pos.x = SEUIL_VERROUILLAGE; // VERROUILLAGE STRICT
                if (j.pos.x < 1800) j.pos.x += v;
            } 
            // SINON Scrolling si le joueur dépasse le milieu
            else if (j.pos.x >= 960 && bg.camera_pos.x < SEUIL_VERROUILLAGE) {
                scrolling(&bg, v);
            } 
            // SINON Marche simple à l'écran
            else if (j.pos.x < 1850) {
                j.pos.x += v;
            }
        }
        
        if (keys[SDL_SCANCODE_LEFT]) {
            bg.direction = 1;
            if (j.pos.x <= 960 && bg.camera_pos.x > 0) {
                // Bloquer le retour en arrière si on est dans la zone de combat
                int limite_g = (bg.camera_pos.x >= SEUIL_VERROUILLAGE && !e20.mort_terminee) ? SEUIL_VERROUILLAGE : 0;
                if (bg.camera_pos.x > limite_g) {
                    scrolling(&bg, v);
                } else if (j.pos.x > 50) {
                    j.pos.x -= v;
                }
            } else if (j.pos.x > 50) {
                j.pos.x -= v;
            }
        }

        updatePhysique(&j);

        // --- GESTION ENNEMI ET COLLISIONS PHYSIQUES ---
        if (bg.camera_pos.x >= SEUIL_VERROUILLAGE - 1920) e20.actif = 1;

        if (e20.actif && !e20.mort_terminee) {
            updateEnnemi(&e20);
            SDL_Rect j_abs = {absX_joueur, j.pos.y, j.pos.w, j.pos.h};
            
            if (testCollision(j_abs, e20.pos)) {
                if (j.etat == FRAPPE || j.etat == FRAPPE_G) {
                    if (e20.etat != E_MORT) { e20.etat = E_MORT; e20.frame = 0; }
                } else {
                    // CONTRAINTE : Répulsion physique (le joueur ne traverse pas l'ennemi)
                    if (j.pos.x < (e20.pos.x - bg.camera_pos.x)) j.pos.x -= 20;
                    else j.pos.x += 20;
                }
            }
        }

        // Mise à jour de la minimap
        updateMinimap(&mn, absX_joueur, e20.pos.x);

        // --- TRANSITION NIVEAU ---
        if (bg.niveau_actuel == 1 && absX_joueur >= 9500 && e20.mort_terminee) {
            libererBackground(&bg);
            initBackground(&bg, renderer, 2);
            j.pos.x = 100; 
            bg.camera_pos.x = 0;
            // On réinitialise l'ennemi pour le niveau suivant
            initEnnemi(&e20, renderer, SEUIL_VERROUILLAGE + 960); 
        }

        // --- RENDU ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        afficherBackground(bg, renderer);
        updateEtAfficherDecors(&bg, renderer);
        
        if (e20.actif) {
            afficherEnnemi(e20, renderer, bg.camera_pos);
        }

        afficherJoueur(j, renderer);
        afficherBarreNiveau(&bg, renderer);
        afficherTemps(&bg, renderer);
        
        // Minimap
        afficherMinimap(mn, renderer, bg.niveau_actuel, !e20.mort_terminee);

        SDL_RenderPresent(renderer);
    }

    // --- CLEANUP ---
    Liberer(&mn);
    libererJoueur(&j); libererBackground(&bg); libererEnnemi(&e20);
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit();

    return 0;
}
