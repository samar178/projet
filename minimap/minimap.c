#include "minimap.h"

void initMinimap(Minimap* m, SDL_Renderer* renderer) {
    // Chargement des ressources spécifiées
    m->backgroundTextures[0] = IMG_LoadTexture(renderer, "minimap/ressources/minimap_bg1.png");
    m->backgroundTextures[1] = IMG_LoadTexture(renderer, "minimap/ressources/minimap_bg2.png");
    m->playerTexture = IMG_LoadTexture(renderer, "minimap/ressources/minimap_j.png");
    m->ennemiTexture = IMG_LoadTexture(renderer, "minimap/ressources/minimap_e.png");

    // Position fixe du HUD sur l'écran
    m->minimapPosition = (SDL_Rect){1400, 30, 480, 54};

    // Taille des icônes (12x21)
    m->playerPosition.w = 12; m->playerPosition.h = 21;
    m->ennemiPosition.w = 12; m->ennemiPosition.h = 21;
}

void updateMinimap(Minimap* m, int absX_joueur, int absX_ennemi) {
    // Ratio : 480px de minimap / 9600px de niveau total = 0.05
    float ratio = 480.0f / 9600.0f;

    // Mise à jour de l'icône joueur
    m->playerPosition.x = m->minimapPosition.x + (int)(absX_joueur * ratio);
    m->playerPosition.y = m->minimapPosition.y + 28; // Centrage vertical approximatif

    // Mise à jour de l'icône ennemi
    m->ennemiPosition.x = m->minimapPosition.x + (int)(absX_ennemi * ratio);
    m->ennemiPosition.y = m->minimapPosition.y + 28;
}

void afficherMinimap(Minimap m, SDL_Renderer* renderer, int niveau_actuel, int ennemiVivant) {
    // Choix du fond selon le niveau actif (1 ou 2)
    int idx = (niveau_actuel == 1) ? 0 : 1;
    
    if (m.backgroundTextures[idx])
        SDL_RenderCopy(renderer, m.backgroundTextures[idx], NULL, &m.minimapPosition);
    
    // Affichage de l'ennemi s'il est actif/vivant
    if (ennemiVivant && m.ennemiTexture)
        SDL_RenderCopy(renderer, m.ennemiTexture, NULL, &m.ennemiPosition);
    
    // Affichage du joueur
    if (m.playerTexture)
        SDL_RenderCopy(renderer, m.playerTexture, NULL, &m.playerPosition);
}

void Liberer(Minimap* m) {
    for(int i = 0; i < 2; i++) {
        if (m->backgroundTextures[i]) SDL_DestroyTexture(m->backgroundTextures[i]);
    }
    if (m->playerTexture) SDL_DestroyTexture(m->playerTexture);
    if (m->ennemiTexture) SDL_DestroyTexture(m->ennemiTexture);
}
