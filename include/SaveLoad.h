#ifndef SAVELOAD_H
#define SAVELOAD_H

#include "../include/Classes.h" // Para a struct Player
#include <stdbool.h>           // Para o tipo bool

#define MAX_SAVE_SLOTS 30
#define SAVE_DIRECTORY "Saves" // Nome do diretório de saves

/**
 * @brief Garante que o diretório de saves exista. Cria se não existir.
 * @return true se o diretório existe ou foi criado com sucesso, false caso contrário.
 */
bool EnsureSavesDirectoryExists(void);

/**
 * @brief Verifica se um arquivo de save existe para um determinado slot.
 * @param slot_number O número do slot (0 a MAX_SAVE_SLOTS-1).
 * @return true se o arquivo de save do slot existe, false caso contrário.
 */
bool DoesSaveSlotExist(int slot_number);

/**
 * @brief Gera o nome do arquivo para um determinado slot de save.
 * @param buffer Buffer de string para armazenar o nome do arquivo gerado.
 * @param buffer_size Tamanho do buffer.
 * @param slot_number O número do slot.
 */
void GetSaveFileName(char *buffer, int buffer_size, int slot_number);

/**
 * @brief Salva o estado atual do jogo no slot especificado.
 * @param players Array de jogadores.
 * @param num_players Número de jogadores.
 * @param slot_number O número do slot para salvar (0 a MAX_SAVE_SLOTS-1).
 * @param currentMapX A coordenada X atual do mapa global.
 * @param currentMapY A coordenada Y atual do mapa global.
 * @return true se o jogo foi salvo com sucesso, false caso contrário.
 */
bool SaveGame(Player players[], int num_players, int slot_number, int currentMapX, int currentMapY);

/**
 * @brief Carrega o estado do jogo do slot especificado.
 * @param players Array de jogadores para preencher com dados carregados.
 * @param num_players_in_game Número de jogadores esperado no jogo atual.
 * @param slot_number O número do slot para carregar (0 a MAX_SAVE_SLOTS-1).
 * @param loadedMapX Ponteiro para armazenar a coordenada X do mapa carregado.
 * @param loadedMapY Ponteiro para armazenar a coordenada Y do mapa carregado.
 * @return true se o jogo foi carregado com sucesso, false caso contrário.
 */
bool LoadGame(Player players[], int num_players_in_game, int slot_number, int *loadedMapX, int *loadedMapY);

#endif // SAVELOAD_H