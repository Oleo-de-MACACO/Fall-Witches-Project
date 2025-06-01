#ifndef INVENTORY_H     // Diretiva de pré-processador para evitar inclusões múltiplas.
#define INVENTORY_H     // Define INVENTORY_H para marcar que este cabeçalho foi incluído.

#include "raylib.h"     // Inclui o cabeçalho da Raylib, usado para tipos básicos e possivelmente desenho.
#include "../include/Classes.h" // Inclui Classes.h para ter acesso às estruturas Player e InventoryItem.
#include "../include/Game.h"    // Inclui Game.h para o enum GameState e outras definições de jogo, como Music.

// Enumeração para as diferentes abas do painel de inventário/status do jogador.
typedef enum {
    TAB_INVENTORY = 0,      // Aba para mostrar o inventário de itens consumíveis/gerais.
    TAB_EQUIPMENT,          // Aba para mostrar os itens atualmente equipados pelo jogador.
    TAB_STATUS,             // Aba para mostrar os status e atributos detalhados do jogador (nível, HP, MP, ataque, etc.).
    MAX_INVENTORY_TABS      // Número total de abas. Usado para contagem, limites e navegação cíclica entre abas.
} InventoryTabType;         // Nome do tipo da enumeração para as abas do inventário.

// --- Protótipos de Função (implementadas em Inventory.c) ---

/**
 * @brief Atualiza a lógica da tela de inventário.
 * Lida com a entrada do usuário para fechar a tela de inventário (voltando para GAMESTATE_PLAYING)
 * e para navegar entre as diferentes abas (Inventário, Equipamento, Status) para cada jogador.
 * Também gerencia o estado da música (pausa ao entrar, resume ao sair se estava tocando).
 * @param currentScreen_ptr Ponteiro para o estado atual do jogo (GameState), para permitir alterá-lo.
 * @param players Array de estruturas Player, pois a navegação de abas é individual.
 * @param musicIsPlaying_ptr Ponteiro para a flag que indica se a música está tocando (para pausar/retomar).
 * @param playlist Array de músicas (necessário para pausar/retomar o stream correto).
 * @param currentMusicIndex_ptr Ponteiro para o índice da música atual (necessário para pausar/retomar).
 */
void UpdateInventoryScreen(GameState *currentScreen_ptr, Player players[], int *musicIsPlaying_ptr, Music playlist[], int *currentMusicIndex_ptr);

/**
 * @brief Desenha a tela de inventário.
 * Primeiro, desenha a tela de jogo (GAMESTATE_PLAYING) como fundo escurecido.
 * Depois, sobrepõe os painéis de inventário/status para os jogadores.
 * Cada painel mostra HP/MP, as abas navegáveis, e o conteúdo da aba selecionada (itens, equipamentos ou status).
 * @param players_arr Array de estruturas Player contendo os dados a serem exibidos.
 * @param background_players_arr Array de jogadores para desenhar o fundo (tela de jogo).
 * @param background_musicVolume Volume da música para o fundo.
 * @param background_currentMusicIndex Índice da música para o fundo.
 * @param background_musicIsPlaying Estado da música para o fundo.
 * @param background_mapX Coordenada X do mapa para o fundo.
 * @param background_mapY Coordenada Y do mapa para o fundo.
 */
void DrawInventoryScreen(Player players_arr[], Player background_players_arr[], float background_musicVolume, int background_currentMusicIndex, int background_musicIsPlaying, int background_mapX, int background_mapY);

/**
 * @brief Adiciona um item ao inventário de um jogador específico.
 * Se o item já existir no inventário, tenta empilhar (aumentar a quantidade).
 * Se não existir e houver espaço, adiciona o item a um slot vazio.
 * @param player Ponteiro para o jogador que receberá o item.
 * @param itemName Nome do item a ser adicionado.
 * @param quantity Quantidade do item a ser adicionada (deve ser > 0).
 * @return true se o item foi adicionado ou empilhado com sucesso, false caso contrário (ex: inventário cheio).
 */
bool AddItemToInventory(Player *player, const char *itemName, int quantity);

/**
 * @brief Remove uma certa quantidade de um item do inventário de um jogador.
 * @param player Ponteiro para o jogador de cujo inventário o item será removido.
 * @param itemName Nome do item a ser removido.
 * @param quantity Quantidade do item a ser removida (deve ser > 0).
 * @return true se a quantidade especificada do item foi removida com sucesso.
 * false se o item não foi encontrado ou a quantidade a remover é maior que a existente.
 */
bool RemoveItemFromInventory(Player *player, const char *itemName, int quantity);

#endif // INVENTORY_H // Fim da diretiva de pré-processador #ifndef.