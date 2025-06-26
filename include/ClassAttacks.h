#ifndef CLASS_ATTACKS_H
#define CLASS_ATTACKS_H

#include "Classes.h" // Para o enum Classe

#define MAX_ATTACKS_PER_CLASS 10
#define MAX_ATTACK_NAME_LENGTH 50

// Enum para o tipo de ataque (físico, mágico ou de status)
typedef enum {
    ATTACK_TYPE_PHYSICAL,
    ATTACK_TYPE_MAGICAL,
    ATTACK_TYPE_STATUS
} AttackType;

// Estrutura para um único ataque/habilidade
typedef struct {
    char name[MAX_ATTACK_NAME_LENGTH];
    AttackType type;
    float power_multiplier; // Ex: 1.0 para ataque normal, 1.5 para forte
    int mana_cost;
} Attack;

// Estrutura para o conjunto de ataques de uma classe
typedef struct {
    Classe class_id;
    Attack attacks[MAX_ATTACKS_PER_CLASS];
    int attack_count;
} ClassAttackSet;

/**
 * @brief Carrega todos os conjuntos de ataques do arquivo de configurações.
 * @param filePath O caminho para o arquivo ClassAttacks.txt.
 */
void ClassAttacks_LoadAll(const char* filePath);

/**
 * @brief Libera a memória alocada para os conjuntos de ataques.
 */
void ClassAttacks_UnloadAll(void);

/**
 * @brief Obtém o conjunto de ataques para uma classe específica.
 * @param player_class O enum da classe do jogador.
 * @return Um ponteiro constante para o conjunto de ataques da classe, ou NULL se não encontrado.
 */
const ClassAttackSet* ClassAttacks_GetAttackSetForClass(Classe player_class);

#endif // CLASS_ATTACKS_H