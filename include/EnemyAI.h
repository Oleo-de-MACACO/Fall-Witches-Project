#ifndef ENEMY_AI_H
#define ENEMY_AI_H

// Forward-declare structs to be used as pointers, avoiding circular includes.
// This tells the compiler "these types exist" without needing their full definition here.
struct MapCharacter;
struct Player;
struct WorldSection;

// Enum para os diferentes estados da IA do inimigo.
// Esta é a ÚNICA e oficial definição deste enum.
typedef enum {
    AI_STATE_IDLE,      // Ocioso, vagando aleatoriamente
    AI_STATE_CHASING,   // Perseguindo o jogador
    AI_STATE_COOLDOWN   // Pausa breve após perder o jogador de vista
} EnemyAIState;

/**
 * @brief Inicializa o estado da IA para um determinado personagem.
 * @param character O personagem inimigo a ser inicializado.
 */
void EnemyAI_Init(struct MapCharacter* character);

/**
 * @brief Atualiza a lógica da IA para um inimigo.
 * @param enemy O inimigo a ser atualizado.
 * @param player O jogador que a IA deve rastrear.
 * @param activeSection A seção do mundo atual, para verificação de colisões.
 */
void EnemyAI_Update(struct MapCharacter* enemy, struct Player* player, const struct WorldSection* activeSection);

#endif // ENEMY_AI_H
