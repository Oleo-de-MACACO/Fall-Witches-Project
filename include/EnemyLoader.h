#ifndef ENEMY_LOADER_H
#define ENEMY_LOADER_H

#include "Classes.h" // For Classe enum

#define MAX_ENEMY_TYPES 50
#define MAX_ENEMY_NAME_LENGTH MAX_CHAR_NAME_LENGTH

// Estrutura para armazenar os stats base de um tipo de inimigo
typedef struct {
    char name[MAX_ENEMY_NAME_LENGTH];
    Classe specialty; // Define o "tipo" ou "classe" do inimigo
    int hp;
    int mp;
    int attack;
    int defense;
    int magic_attack;
    int magic_defense;
} EnemyData;

/**
 * @brief Carrega os dados de todos os inimigos do arquivo de configuração.
 * @param filePath O caminho para o arquivo Enemies.txt.
 */
void EnemyLoader_LoadAll(const char* filePath);

/**
 * @brief Obtém os stats de um inimigo pelo seu nome.
 * @param enemyName O nome do inimigo (ex: "Goblin").
 * @return Um ponteiro constante para os dados do inimigo, ou NULL se não encontrado.
 */
const EnemyData* EnemyLoader_GetByName(const char* enemyName);

/**
 * @brief Libera a memória alocada pelo carregador de inimigos.
 */
void EnemyLoader_UnloadAll(void);

#endif // ENEMY_LOADER_H
