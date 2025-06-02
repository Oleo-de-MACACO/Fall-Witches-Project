#ifndef CHARACTER_CREATION_H
#define CHARACTER_CREATION_H

#include "raylib.h"
#include "Game.h"    // Para GameState, Player, Music, MAX_PLAYERS
// Classes.h é incluído por Game.h, então Player, Classe, MAX_PLAYER_NAME_LENGTH estão disponíveis
#include <stdbool.h> // Para o tipo bool

// --- Declarações das Funções da Tela de Criação de Personagem ---

/**
 * @brief Inicializa as variáveis para a tela de criação de personagem.
 * Reseta os nomes, classes e o estado de edição para cada jogador.
 */
void InitializeCharacterCreation(void);

/**
 * @brief Atualiza a lógica da tela de criação de personagem.
 * Lida com a entrada do usuário para nomear personagens, selecionar classes e confirmar.
 * @param currentScreen_ptr Ponteiro para o estado atual do jogo (GameState), para permitir alterá-lo.
 * @param players Array de estruturas Player, para inicializar os jogadores criados.
 * @param mapX Ponteiro para a coordenada X do mapa (para resetar em novo jogo).
 * @param mapY Ponteiro para a coordenada Y do mapa (para resetar em novo jogo).
 * @param playlist Array de músicas (para iniciar/parar música).
 * @param currentMusicIndex Índice da música atual na playlist.
 * @param currentVolume Volume atual da música.
 * @param musicIsPlaying_ptr Ponteiro para a flag que indica se a música está tocando.
 */
void UpdateCharacterCreationScreen(GameState *currentScreen_ptr, Player players[], int *mapX, int *mapY, Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsPlaying_ptr);

/**
 * @brief Desenha a tela de criação de personagem.
 * Exibe as instruções, campos de nome, opções de classe e atributos de pré-visualização.
 * @param players Array de estruturas Player (atualmente não usado diretamente para desenho nesta função, mas mantido para consistência ou uso futuro).
 */
void DrawCharacterCreationScreen(Player players[]);

#endif // CHARACTER_CREATION_H