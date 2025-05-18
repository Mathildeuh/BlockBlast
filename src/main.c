#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/bb.h"

// Fonction utilitaire pour afficher les blocs disponibles centrés
void afficher_blocs_centres(const Block blocs[NB_BLOCKS], const int blocs_utilises[NB_BLOCKS], int selection) {
    const int cadre_largeur = 11;
    // Ligne du haut de chaque bloc
    for (int b = 0; b < NB_BLOCKS; ++b) {
        if (!blocs_utilises[b]) {
            printf("╭");
            for (int i = 0; i < cadre_largeur; ++i) printf("─");
            printf("╮ ");
        }
    }
    printf("\n");
    // Calcul des paddings verticaux pour chaque bloc
    int min_y[NB_BLOCKS], max_y[NB_BLOCKS];
    for (int b = 0; b < NB_BLOCKS; ++b) {
        min_y[b] = BLOCK_MAX_SIZE;
        max_y[b] = -1;
        if (!blocs_utilises[b]) {
            for (int by = 0; by < BLOCK_MAX_SIZE; ++by)
                for (int bx = 0; bx < BLOCK_MAX_SIZE; ++bx)
                    if (blocs[b].shape[by][bx]) {
                        if (by < min_y[b]) min_y[b] = by;
                        if (by > max_y[b]) max_y[b] = by;
                    }
        }
    }
    int bloc_height[NB_BLOCKS], pad_top[NB_BLOCKS], pad_bottom[NB_BLOCKS];
    for (int b = 0; b < NB_BLOCKS; ++b) {
        if (!blocs_utilises[b] && max_y[b] >= min_y[b]) {
            bloc_height[b] = max_y[b] - min_y[b] + 1;
            pad_top[b] = (BLOCK_MAX_SIZE - bloc_height[b]) / 2;
            pad_bottom[b] = BLOCK_MAX_SIZE - bloc_height[b] - pad_top[b];
        } else {
            bloc_height[b] = 0;
            pad_top[b] = BLOCK_MAX_SIZE;
            pad_bottom[b] = 0;
        }
    }
    // Affichage des lignes du cadre (toujours BLOCK_MAX_SIZE lignes)
    for (int y = 0; y < BLOCK_MAX_SIZE; ++y) {
        for (int b = 0; b < NB_BLOCKS; ++b) {
            if (!blocs_utilises[b]) {
                printf("│");
                if (b == selection) printf("\033[47m");
                if (y < pad_top[b] || y >= BLOCK_MAX_SIZE - pad_bottom[b]) {
                    for (int i = 0; i < cadre_largeur; ++i) printf(" ");
                } else {
                    int min_x = BLOCK_MAX_SIZE, max_x = -1;
                    for (int by = 0; by < BLOCK_MAX_SIZE; ++by)
                        for (int bx = 0; bx < BLOCK_MAX_SIZE; ++bx)
                            if (blocs[b].shape[by][bx]) {
                                if (bx < min_x) min_x = bx;
                                if (bx > max_x) max_x = bx;
                            }
                    int bloc_width = (max_x >= min_x) ? (max_x - min_x + 1) : 0;
                    int pad_left = (BLOCK_MAX_SIZE - bloc_width) / 2;
                    int pad_right = BLOCK_MAX_SIZE - bloc_width - pad_left;
                    int bloc_y = min_y[b] + (y - pad_top[b]);
                    int printed = 0;
                    for (int x = 0; x < BLOCK_MAX_SIZE; ++x) {
                        if (x < pad_left || x >= BLOCK_MAX_SIZE - pad_right) {
                            printf("  ");
                            printed += 2;
                        } else {
                            int bloc_x = min_x + (x - pad_left);
                            if (blocs[b].shape[bloc_y][bloc_x]) {
                                if (b == selection)
                                    printf("\033[30m▣ \033[0m\033[47m");
                                else
                                    printf("▣ ");
                            } else {
                                printf("  ");
                            }
                            printed += 2;
                        }
                    }
                    while (printed < cadre_largeur) { printf(" "); ++printed; }
                }
                if (b == selection) printf("\033[0m");
                printf("│ ");
            }
        }
        printf("\n");
    }
    // Ligne du bas de chaque bloc
    for (int b = 0; b < NB_BLOCKS; ++b) {
        if (!blocs_utilises[b]) {
            printf("╰");
            for (int i = 0; i < cadre_largeur; ++i) printf("─");
            printf("╯ ");
        }
    }
    printf("\n");
}

// Fonction utilitaire pour centrer verticalement
void print_vertical_padding(int content_height) {
    int termh = 24;
#ifdef TIOCGWINSZ
    struct winsize w;
    if (ioctl(1, TIOCGWINSZ, &w) == 0) termh = w.ws_row;
#endif
    int pad = (termh - content_height) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; ++i) putchar('\n');
}

int main(void)
{
    charger_langue("fr-FR");
    while (1) {
        Grid grille = {0};
        Block blocs[NB_BLOCKS];
        int score = 0;
        int blocs_utilises[NB_BLOCKS] = {0};
        int nb_blocs_restants = NB_BLOCKS;
        int px, py;
        int content_height = 0;

        generer_blocs(blocs);

        while (1) {
            system("clear"); // ou "cls" sous Windows
            printf(MSG("MSG_SCORE"), score); printf("\n\n");
            afficher_grille(&grille, score);
            afficher_blocs_centres(blocs, blocs_utilises, -1);
            printf("%s ", MSG("MSG_BLOCKS_LEFT"));
            for (int i = 0; i < NB_BLOCKS; ++i) printf("%d ", blocs_utilises[i] ? 0 : 1);
            printf("\n");
            // Vérifier si au moins un placement est possible
            int placement_possible = 0;
            for (int b = 0; b < NB_BLOCKS; ++b) {
                if (blocs_utilises[b]) continue;
                for (int y = 0; y < GRID_SIZE; ++y) {
                    for (int x = 0; x < GRID_SIZE; ++x) {
                        if (placer_bloc(&grille, &blocs[b], x, y)) {
                            // Annuler le placement test
                            for (int by = 0; by < BLOCK_MAX_SIZE; ++by)
                                for (int bx = 0; bx < BLOCK_MAX_SIZE; ++bx)
                                    if (blocs[b].shape[by][bx])
                                        grille.cells[y+by][x+bx] = 0;
                            placement_possible = 1;
                        }
                    }
                }
            }
            if (!placement_possible) {
                printf("\n%s\n", MSG("MSG_GAME_OVER"));
                printf(MSG("MSG_FINAL_SCORE"), score); printf("\n");
                char pseudo[32];
                printf("%s", MSG("MSG_ENTER_PSEUDO"));
                fgets(pseudo, sizeof(pseudo), stdin);
                size_t len = strlen(pseudo);
                if (len > 0 && pseudo[len-1] == '\n') pseudo[len-1] = '\0';
                sauvegarder_score(score, pseudo);
                printf("%s\n", MSG("MSG_SAVED"));
                printf("%s", MSG("MSG_PRESS_ENTER"));
                getchar();
                break;
            }
            // Sélection du bloc avec les flèches
            int selection = 0;
            for (int i = 0; i < NB_BLOCKS; ++i) {
                if (!blocs_utilises[i]) { selection = i; break; }
            }
            while (1) {
                system("clear");
                afficher_grille(&grille, score);
                afficher_blocs_centres(blocs, blocs_utilises, selection);
                printf("%s\n", MSG("MSG_SELECT"));
                int touche = lire_touche();
                if (touche == 'M' || touche == 'm') {
                    // Menu de langue
                    printf("\n1. Français\n2. English\nChoisissez la langue : ");
                    int choix = getchar();
                    while (getchar() != '\n');
                    if (choix == '1') charger_langue("fr-FR");
                    else if (choix == '2') charger_langue("en-US");
                    continue;
                }
                if (touche == 'L') {
                    do { selection = (selection - 1 + NB_BLOCKS) % NB_BLOCKS; } while (blocs_utilises[selection]);
                } else if (touche == 'R') {
                    do { selection = (selection + 1) % NB_BLOCKS; } while (blocs_utilises[selection]);
                } else if (touche == ' ' || touche == '\n') {
                    break;
                } else if (touche == 'q' || touche == 'Q') {
                    selection = -1;
                    break;
                } else if (touche == 'S' || touche == 's') {
                    afficher_scores();
                    printf("Appuyez sur Entrée pour continuer...");
                    getchar();
                }
                content_height = 2 + GRID_SIZE + 4 + BLOCK_MAX_SIZE + 2;
                print_vertical_padding(content_height);
            }
            if (selection < 0) continue;
            int choix = selection;
            int largeur = 0, hauteur = 0;
            for (int by = 0; by < BLOCK_MAX_SIZE; ++by)
                for (int bx = 0; bx < BLOCK_MAX_SIZE; ++bx)
                    if (blocs[choix].shape[by][bx]) {
                        if (bx + 1 > largeur) largeur = bx + 1;
                        if (by + 1 > hauteur) hauteur = by + 1;
                    }
            px = (GRID_SIZE - largeur) / 2;
            py = (GRID_SIZE - hauteur) / 2;
            int valider = 0;
            while (!valider) {
                system("clear");
                afficher_grille_avec_bloc(&grille, &blocs[choix], px, py, peut_placer_bloc(&grille, &blocs[choix], px, py));
                afficher_blocs_centres(blocs, blocs_utilises, -1);
                printf("%s\n", MSG("MSG_PLACE"));
                int touche = lire_touche();
                if (touche == 'L' && px > 0) px--;
                else if (touche == 'R' && px < GRID_SIZE - largeur) px++;
                else if (touche == 'U' && py > 0) py--;
                else if (touche == 'D' && py < GRID_SIZE - hauteur) py++;
                else if (touche == ' ' || touche == '\n') {
                    if (peut_placer_bloc(&grille, &blocs[choix], px, py)) valider = 1;
                }
                else if (touche == 'q' || touche == 'Q') break;
                content_height = 2 + GRID_SIZE + 4 + BLOCK_MAX_SIZE + 2;
                print_vertical_padding(content_height);
            }
            if (!valider) continue;
            placer_bloc(&grille, &blocs[choix], px, py);
            blocs_utilises[choix] = 1;
            nb_blocs_restants--;
            score += casser_lignes_colonnes(&grille);
            if (nb_blocs_restants == 0) {
                generer_blocs(blocs);
                for (int i = 0; i < NB_BLOCKS; ++i) blocs_utilises[i] = 0;
                nb_blocs_restants = NB_BLOCKS;
            }
        }
    }
    return 0;
}