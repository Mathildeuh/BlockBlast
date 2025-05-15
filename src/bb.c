#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/bb.h"
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <locale.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_BLOCK   "\033[1;34m"
#define COLOR_SCORE   "\033[1;32m"
#define COLOR_FRAME   "\033[1;37m"
#define COLOR_OK     "\033[1;32m"
#define COLOR_NO     "\033[1;31m"
#define COLOR_YELLOW  "\033[1;33m"
#define LANG_MAX 64
#define MSG_MAX 64
#define MSG_LEN 128

// Quelques formes de blocs de type Tetris (max 5x5)
static const int formes[][BLOCK_MAX_SIZE][BLOCK_MAX_SIZE] = {
    // ##### (ligne de 5)
    {{1,1,1,1,1}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // #\n#\n#\n#\n# (colonne de 5)
    {{1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}},
    // ###\n###\n### (carré 3x3 plein)
    {{1,1,1,0,0}, {1,1,1,0,0}, {1,1,1,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // #\n### (L inversé)
    {{1,0,0,0,0}, {1,1,1,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // ##\n## (carré 2x2)
    {{1,1,0,0,0}, {1,1,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // ##\n## (carré 2x2 bis)
    {{1,1,0,0,0}, {1,1,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    //     #\n### (L à droite)
    {{0,0,1,0,0}, {1,1,1,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // ###\n# (L à gauche)
    {{1,1,1,0,0}, {1,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // ###\n### (rectangle 3x2)
    {{1,1,1,0,0}, {1,1,1,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // #### (ligne de 4)
    {{1,1,1,1,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // #\n#\n#\n# (colonne de 4)
    {{1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {0,0,0,0,0}},
    // # (bloc 1x1)
    {{1,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // ##\n ## (S)
    {{1,1,0,0,0}, {0,1,1,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    //  ##\n## (Z)
    {{0,1,1,0,0}, {1,1,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // #\n##\n  # (S inversé)
    {{1,0,0,0,0}, {1,1,0,0,0}, {0,1,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    //  #\n##\n# (Z inversé)
    {{0,1,0,0,0}, {1,1,0,0,0}, {1,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // S : ##\n ##
    {{1,1,0,0,0}, {0,1,1,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // Z :  ##\n##
    {{0,1,1,0,0}, {1,1,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // S inversé :  ##\n##
    {{0,1,1,0,0}, {1,1,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    // Z inversé : ##\n ##
    {{1,1,0,0,0}, {0,1,1,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},

    {{1,0,0,0,0}, {1,1,0,0,0}, {1,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}},
    { {1,1,0,0,0}, {1,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0} },
    { {1,1,1,0,0}, {1,0,0,0,0}, {1,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0} },
};
#define NB_FORMES (sizeof(formes)/sizeof(formes[0]))

// Ajout d'une fonction pour comparer deux formes
int forme_egale(const int a[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE], const int b[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE]) {
    for (int y = 0; y < BLOCK_MAX_SIZE; ++y)
        for (int x = 0; x < BLOCK_MAX_SIZE; ++x)
            if (a[y][x] != b[y][x]) return 0;
    return 1;
}

// Rotation d'une forme de 90° (dans out)
void rotation_90(const int in[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE], int out[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE]) {
    for (int y = 0; y < BLOCK_MAX_SIZE; ++y)
        for (int x = 0; x < BLOCK_MAX_SIZE; ++x)
            out[x][BLOCK_MAX_SIZE-1-y] = in[y][x];
}

// Génère toutes les formes (avec rotations uniques)
static int toutes_formes[128][BLOCK_MAX_SIZE][BLOCK_MAX_SIZE];
static int nb_toutes_formes = 0;

// Décale la forme pour qu'elle soit en haut à gauche (compactage)
void compacter_forme(int shape[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE]) {
    int min_x = BLOCK_MAX_SIZE, min_y = BLOCK_MAX_SIZE;
    for (int y = 0; y < BLOCK_MAX_SIZE; ++y)
        for (int x = 0; x < BLOCK_MAX_SIZE; ++x)
            if (shape[y][x]) {
                if (x < min_x) min_x = x;
                if (y < min_y) min_y = y;
            }
    if (min_x == 0 && min_y == 0) return; // déjà compacté
    int tmp[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE] = {0};
    for (int y = 0; y < BLOCK_MAX_SIZE; ++y)
        for (int x = 0; x < BLOCK_MAX_SIZE; ++x)
            if (shape[y][x])
                tmp[y - min_y][x - min_x] = 1;
    memcpy(shape, tmp, sizeof(tmp));
}

void initialiser_formes_rotations(void) {
    nb_toutes_formes = 0;
    for (size_t f = 0; f < NB_FORMES; ++f) {
        int base[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE];
        memcpy(base, formes[f], sizeof(base));
        for (int rot = 0; rot < 4; ++rot) {
            compacter_forme(base);
            // Vérifier si déjà présente
            int deja = 0;
            for (int i = 0; i < nb_toutes_formes; ++i) {
                if (forme_egale(toutes_formes[i], base)) { deja = 1; break; }
            }
            if (!deja) {
                memcpy(toutes_formes[nb_toutes_formes++], base, sizeof(base));
            }
            // Tourner pour la prochaine rotation
            int tmp[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE];
            rotation_90(base, tmp);
            memcpy(base, tmp, sizeof(base));
        }
    }
}

void generer_blocs(Block blocs[NB_BLOCKS]) {
    static int init = 0;
    if (!init) { srand(time(NULL)); initialiser_formes_rotations(); init = 1; }
    for (int i = 0; i < NB_BLOCKS; ++i) {
        int trouve = 0;
        while (!trouve) {
            int f = rand() % nb_toutes_formes;
            blocs[i].size = 5;
            for (int y = 0; y < BLOCK_MAX_SIZE; ++y)
                for (int x = 0; x < BLOCK_MAX_SIZE; ++x)
                    blocs[i].shape[y][x] = toutes_formes[f][y][x];
            // Vérifier que le bloc est plaçable sur une grille vide
            Grid grille_vide = {0};
            trouve = 0;
            for (int gy = 0; gy <= GRID_SIZE - BLOCK_MAX_SIZE; ++gy) {
                for (int gx = 0; gx <= GRID_SIZE - BLOCK_MAX_SIZE; ++gx) {
                    if (peut_placer_bloc(&grille_vide, &blocs[i], gx, gy)) {
                        trouve = 1;
                        break;
                    }
                }
                if (trouve) break;
            }
        }
    }
}

int get_terminal_width(void) {
    struct winsize w;
    if (ioctl(1, TIOCGWINSZ, &w) == 0) return w.ws_col;
    return 80;
}

void print_centered(const char *str, int width) {
    int len = (int)strlen(str);
    int pad = (width - len) / 2;
    if (pad > 0) for (int i = 0; i < pad; ++i) putchar(' ');
    printf("%s", str);
}

void afficher_grille(const Grid *grille, int score) {
    char buf[128];
    snprintf(buf, sizeof(buf), COLOR_SCORE "Score : %d" COLOR_RESET "\n\n", score);
    printf("%s", buf);
    char ligne[256];
    // Ligne du haut
    strcpy(ligne, "   +");
    for (int x = 0; x < GRID_SIZE; ++x) strcat(ligne, "--");
    strcat(ligne, "+");
    printf("%s\n", ligne);
    for (int y = 0; y < GRID_SIZE; ++y) {
        sprintf(ligne, COLOR_FRAME "%2d |" COLOR_RESET, y+1);
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (grille->cells[y][x])
                strcat(ligne, COLOR_BLOCK "# " COLOR_RESET);
            else
                strcat(ligne, ". ");
        }
        strcat(ligne, COLOR_FRAME "|" COLOR_RESET);
        printf("%s\n", ligne);
    }
    // Ligne du bas
    strcpy(ligne, "   +");
    for (int x = 0; x < GRID_SIZE; ++x) strcat(ligne, "--");
    strcat(ligne, "+");
    printf("%s\n", ligne);
    strcpy(ligne, "    ");
    for (int x = 0; x < GRID_SIZE; ++x) {
        char tmp[8];
        sprintf(tmp, "%d ", x+1);
        strcat(ligne, tmp);
    }
    printf("%s\n", ligne);
}

void afficher_blocs(const Block blocs[NB_BLOCKS]) {
    printf("\nBlocs disponibles :\n");
    for (int b = 0; b < NB_BLOCKS; ++b) {
        printf(COLOR_FRAME "Bloc %d :\n" COLOR_RESET, b+1);
        for (int y = 0; y < BLOCK_MAX_SIZE; ++y) {
            int ligne_vide = 1;
            for (int x = 0; x < BLOCK_MAX_SIZE; ++x)
                if (blocs[b].shape[y][x]) ligne_vide = 0;
            if (ligne_vide) continue;
            printf("   ");
            for (int x = 0; x < BLOCK_MAX_SIZE; ++x) {
                if (blocs[b].shape[y][x])
                    printf(COLOR_BLOCK "# " COLOR_RESET);
                else
                    printf("  ");
            }
            printf("\n");
        }
        printf("\n");
    }
}

// Vérifie si le bloc peut être placé à la position (x, y) sans le placer
enum { BLOC_OK = 1, BLOC_NON = 0 };
int peut_placer_bloc(const Grid *grille, const Block *bloc, int x, int y) {
    for (int by = 0; by < BLOCK_MAX_SIZE; ++by) {
        for (int bx = 0; bx < BLOCK_MAX_SIZE; ++bx) {
            if (bloc->shape[by][bx]) {
                int gy = y + by;
                int gx = x + bx;
                if (gy < 0 || gy >= GRID_SIZE || gx < 0 || gx >= GRID_SIZE)
                    return BLOC_NON;
                if (grille->cells[gy][gx])
                    return BLOC_NON;
            }
        }
    }
    return BLOC_OK;
}

// Affiche la grille avec le bloc fantôme à la position (bx, by)
void afficher_grille_avec_bloc(const Grid *grille, const Block *bloc, int bx, int by, int peut_placer) {
    char buf[128];
    snprintf(buf, sizeof(buf), COLOR_SCORE "Score : ?" COLOR_RESET "\n\n");
    printf("%s", buf);
    char ligne[256];
    // Prévoir les lignes qui seront pleines si on place le bloc
    int lignes_pleines[GRID_SIZE] = {0};
    int colonnes_pleines[GRID_SIZE] = {0};
    if (peut_placer) {
        Grid grille_test = *grille;
        placer_bloc(&grille_test, bloc, bx, by);
        // Détection des lignes pleines
        for (int i = 0; i < GRID_SIZE; ++i) {
            int pleine_ligne = 1, pleine_col = 1;
            for (int j = 0; j < GRID_SIZE; ++j) {
                if (!grille_test.cells[i][j]) pleine_ligne = 0;
                if (!grille_test.cells[j][i]) pleine_col = 0;
            }
            lignes_pleines[i] = pleine_ligne;
            colonnes_pleines[i] = pleine_col;
        }
    }
    // Ligne du haut
    strcpy(ligne, "   +");
    for (int x = 0; x < GRID_SIZE; ++x) strcat(ligne, "--");
    strcat(ligne, "+");
    printf("%s\n", ligne);
    for (int y = 0; y < GRID_SIZE; ++y) {
        // Affichage du numéro de ligne
        if (lignes_pleines[y])
            sprintf(ligne, COLOR_YELLOW "%2d |" COLOR_RESET, y+1);
        else
            sprintf(ligne, COLOR_FRAME "%2d |" COLOR_RESET, y+1);
        for (int x = 0; x < GRID_SIZE; ++x) {
            int affiche = 0;
            for (int byb = 0; byb < BLOCK_MAX_SIZE; ++byb)
                for (int bxb = 0; bxb < BLOCK_MAX_SIZE; ++bxb)
                    if (bloc->shape[byb][bxb] && y == by + byb && x == bx + bxb)
                        affiche = 1;
            if (lignes_pleines[y]) {
                if (affiche) {
                    if (peut_placer)
                        strcat(ligne, COLOR_YELLOW COLOR_OK "# " COLOR_RESET);
                    else
                        strcat(ligne, COLOR_YELLOW COLOR_NO "# " COLOR_RESET);
                } else if (grille->cells[y][x]) {
                    strcat(ligne, COLOR_YELLOW COLOR_BLOCK "# " COLOR_RESET);
                } else {
                    strcat(ligne, COLOR_YELLOW ". " COLOR_RESET);
                }
            } else {
                if (affiche) {
                    if (peut_placer)
                        strcat(ligne, COLOR_OK "# " COLOR_RESET);
                    else
                        strcat(ligne, COLOR_NO "# " COLOR_RESET);
                } else if (grille->cells[y][x]) {
                    strcat(ligne, COLOR_BLOCK "# " COLOR_RESET);
                } else {
                    strcat(ligne, ". ");
                }
            }
        }
        strcat(ligne, COLOR_FRAME "|" COLOR_RESET);
        printf("%s\n", ligne);
    }
    // Ligne du bas
    strcpy(ligne, "   +");
    for (int x = 0; x < GRID_SIZE; ++x) strcat(ligne, "--");
    strcat(ligne, "+");
    printf("%s\n", ligne);
    strcpy(ligne, "    ");
    for (int x = 0; x < GRID_SIZE; ++x) {
        if (colonnes_pleines[x])
            sprintf(ligne + strlen(ligne), COLOR_YELLOW "%d " COLOR_RESET, x+1);
        else
            sprintf(ligne + strlen(ligne), "%d ", x+1);
    }
    printf("%s\n", ligne);
}

// Lecture d'une touche (flèches, espace, etc.) en mode non-canonique
int lire_touche(void) {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    if (ch == 27) { // séquence d'échappement
        getchar(); // skip '['
        switch(getchar()) {
            case 'A': ch = 'U'; break; // Up
            case 'B': ch = 'D'; break; // Down
            case 'C': ch = 'R'; break; // Right
            case 'D': ch = 'L'; break; // Left
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// Casse les lignes et colonnes pleines, retourne le nombre de lignes/colonnes cassées
int casser_lignes_colonnes(Grid *grille) {
    int score = 0;
    int lignes[GRID_SIZE] = {0};
    int colonnes[GRID_SIZE] = {0};
    // Détection
    for (int i = 0; i < GRID_SIZE; ++i) {
        int pleine_ligne = 1, pleine_col = 1;
        for (int j = 0; j < GRID_SIZE; ++j) {
            if (!grille->cells[i][j]) pleine_ligne = 0;
            if (!grille->cells[j][i]) pleine_col = 0;
        }
        lignes[i] = pleine_ligne;
        colonnes[i] = pleine_col;
    }
    // Suppression
    for (int i = 0; i < GRID_SIZE; ++i) {
        if (lignes[i]) {
            for (int j = 0; j < GRID_SIZE; ++j)
                grille->cells[i][j] = 0;
            score++;
        }
        if (colonnes[i]) {
            for (int j = 0; j < GRID_SIZE; ++j)
                grille->cells[j][i] = 0;
            score++;
        }
    }
    return score;
}

void sauvegarder_score(int score, const char* pseudo) {
    FILE *f = fopen("score.txt", "a");
    if (f) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char date[32];
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(f, "%d;%s;%s\n", score, pseudo, date);
        fclose(f);
    }
}

int lire_meilleur_score(void) {
    FILE *f = fopen("score.txt", "r");
    int score, best = 0;
    if (!f) return 0;
    while (fscanf(f, "%d", &score) == 1) {
        if (score > best) best = score;
    }
    fclose(f);
    return best;
}

void afficher_scores(void) {
    FILE *f = fopen("score.txt", "r");
    int score, i = 1;
    char pseudo[32], date[32];
    if (!f) {
        printf("Aucun score enregistré.\n");
        return;
    }
    printf("\n--- Historique des scores ---\n");
    while (fscanf(f, "%d;%31[^;];%31[^\n]\n", &score, pseudo, date) == 3) {
        printf("Partie %d : %d pts par %s le %s\n", i++, score, pseudo, date);
    }
    fclose(f);
    printf("---------------------------\n");
}

// Place effectivement le bloc sur la grille si possible, retourne 1 si OK, 0 sinon
int placer_bloc(Grid *grille, const Block *bloc, int x, int y) {
    if (!peut_placer_bloc(grille, bloc, x, y))
        return 0;
    for (int by = 0; by < BLOCK_MAX_SIZE; ++by) {
        for (int bx = 0; bx < BLOCK_MAX_SIZE; ++bx) {
            if (bloc->shape[by][bx]) {
                grille->cells[y + by][x + bx] = 1;
            }
        }
    }
    return 1;
}

static char current_lang[LANG_MAX] = "fr-FR";
static struct {
    char key[MSG_MAX][32];
    char val[MSG_MAX][MSG_LEN];
    int count;
} lang_msgs = {0};

void charger_langue(const char* code) {
    snprintf(current_lang, sizeof(current_lang), "%s", code);
    lang_msgs.count = 0;
    char path[128];
    snprintf(path, sizeof(path), "lang/%s.lang", code);
    FILE* f = fopen(path, "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char* key = line;
        char* val = eq+1;
        // Enlever retour à la ligne
        char* nl = strchr(val, '\n');
        if (nl) *nl = 0;
        strncpy(lang_msgs.key[lang_msgs.count], key, 31);
        lang_msgs.key[lang_msgs.count][31] = '\0';
        snprintf(lang_msgs.val[lang_msgs.count], MSG_LEN, "%s", val);
        lang_msgs.count++;
    }
    fclose(f);
}

const char* MSG(const char* key) {
    for (int i = 0; i < lang_msgs.count; ++i)
        if (strcmp(lang_msgs.key[i], key) == 0)
            return lang_msgs.val[i];
    return key;
} 