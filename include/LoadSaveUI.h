#ifndef LOAD_SAVE_UI_H
#define LOAD_SAVE_UI_H

#include "raylib.h" // Para Vector2, Rectangle, Color, etc.
#include "Game.h"   // Para GameState, Player, Music

/**
 * @brief Atualiza a lógica da tela de Save/Load.
 * Lida com a seleção de slots, confirmações de sobrescrita, salvamento/carregamento do jogo,
 * e navegação para outras telas (como CharacterCreation ou de volta à tela anterior).
 * Utiliza funções getter/setter de Menu.c para acessar/modificar o contexto de Save/Load.
 * @param currentScreen_ptr Ponteiro para o estado atual do jogo, para permitir alterá-lo.
 * @param players Array de jogadores, para passar para SaveGame/LoadGame ou CharacterCreation.
 * @param playlist Array de músicas, para reiniciar música ao carregar.
 * @param currentMusicIndex Índice da música atual.
 * @param currentVolume Volume atual da música.
 * @param musicIsCurrentlyPlaying_ptr Ponteiro para o estado da música.
 * @param currentMapX_ptr Ponteiro para a coordenada X do mapa global.
 * @param currentMapY_ptr Ponteiro para a coordenada Y do mapa global.
 * @param virtualMousePos Posição do mouse na tela virtual.
 */
void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsCurrentlyPlaying_ptr, int *currentMapX_ptr, int *currentMapY_ptr, Vector2 virtualMousePos);

/**
 * @brief Desenha a tela de Save/Load.
 * Exibe a lista de slots de save, a caixa de diálogo de confirmação (se aplicável),
 * e o fundo apropriado (como a tela de pausa ou um fundo simples).
 * Utiliza funções getter de Menu.c para determinar o contexto de desenho.
 * @param players Array de jogadores, para passar para DrawPauseScreen (fundo).
 * @param playlist Array de músicas (para DrawPauseScreen).
 * @param currentMusicIndex Índice da música atual (para DrawPauseScreen).
 * @param musicIsPlaying Estado da música (para DrawPauseScreen).
 * @param musicVolume Volume da música (para DrawPauseScreen).
 * @param mapX Coordenada X do mapa (para DrawPauseScreen).
 * @param mapY Coordenada Y do mapa (para DrawPauseScreen).
 */
void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, int musicIsPlaying, float musicVolume, int mapX, int mapY);

#endif // LOAD_SAVE_UI_H