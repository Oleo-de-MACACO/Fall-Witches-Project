#ifndef CLASS_SETTINGS_H
#define CLASS_SETTINGS_H

#include "Classes.h" // Para o enum Classe

#define MAX_MOVES_PER_CLASS 8
#define MAX_MOVE_NAME_LENGTH 50

// Enum para o tipo de movimento
typedef enum {
    MOVE_TYPE_PHYSICAL,
    MOVE_TYPE_MAGICAL,
    MOVE_TYPE_STATUS
} MoveType;

// Estrutura para um único movimento/habilidade
typedef struct {
    char name[MAX_MOVE_NAME_LENGTH];
    MoveType type;
    float power_multiplier; // Ex: 1.0 para ataque normal, 1.5 para forte
    int mana_cost;
} Move;

// Estrutura para o conjunto de movimentos de uma classe
typedef struct {
    Classe class_id;
    Move moves[MAX_MOVES_PER_CLASS];
    int move_count;
} ClassMoveset;

/**
 * @brief Carrega todos os conjuntos de movimentos do arquivo de configurações.
 * @param filePath O caminho para o arquivo Settings.txt.
 */
void ClassSettings_LoadAll(const char* filePath);

/**
 * @brief Obtém o conjunto de movimentos para uma classe específica.
 * @param player_class O enum da classe do jogador.
 * @return Um ponteiro constante para o conjunto de movimentos da classe, ou NULL se não encontrado.
 */
const ClassMoveset* ClassSettings_GetMovesForClass(Classe player_class);

#endif // CLASS_SETTINGS_H
