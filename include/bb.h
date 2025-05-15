#ifndef BB_H
#define BB_H

#define GRID_SIZE 8
#define BLOCK_MAX_SIZE 5
#define NB_BLOCKS 3

// Structure pour un bloc (type Tetris)
typedef struct {
    int shape[BLOCK_MAX_SIZE][BLOCK_MAX_SIZE]; // 1 = carré présent, 0 = vide
    int size; // taille réelle du bloc (ex: 3 pour un L 3x3)
} Block;

// Structure pour la grille
typedef struct {
    int cells[GRID_SIZE][GRID_SIZE]; // 1 = occupé, 0 = vide
} Grid;

// Prototypes
void afficher_grille(const Grid *grille, int score);
void afficher_blocs(const Block blocs[NB_BLOCKS]);
void generer_blocs(Block blocs[NB_BLOCKS]);
int placer_bloc(Grid *grille, const Block *bloc, int x, int y);
int casser_lignes_colonnes(Grid *grille);
void sauvegarder_score(int score, const char* pseudo);
int lire_meilleur_score(void);
void afficher_scores(void);
int peut_placer_bloc(const Grid *grille, const Block *bloc, int x, int y);
void afficher_grille_avec_bloc(const Grid *grille, const Block *bloc, int bx, int by, int peut_placer);
int lire_touche(void);
int get_terminal_width(void);
void print_centered(const char *str, int width);
void charger_langue(const char* code);
const char* MSG(const char* key);

#endif // BB_H
