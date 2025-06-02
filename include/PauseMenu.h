#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include "raylib.h" // Para Vector2 e outras definições da Raylib
#include "Game.h"   // Para GameState, Player, Music

/**
 * @brief Atualiza a lógica do menu de pausa.
 * Lida com a entrada do usuário para interagir com os botões de pausa (Continuar, Salvar, Carregar, etc.).
 * Solicita transições de estado para outros menus (como Save/Load, Main Menu) através de funções em Menu.c.
 * @param currentScreen_ptr Ponteiro para o estado atual do jogo (GameState), para permitir alterá-lo para GAMESTATE_PLAYING.
 * @param players Array de jogadores (atualmente não usado diretamente na lógica de update, mas mantido por consistência).
 * @param playlist Array de músicas da playlist do jogo.
 * @param currentMusicIndex Índice da música atual na playlist.
 * @param isPlaying_beforePause Estado da música (tocando/pausada) antes de entrar no menu de pausa.
 * @param musicIsCurrentlyPlaying_ptr Ponteiro para a flag que indica se a música está tocando atualmente.
 * @param virtualMousePos Posição do mouse na tela virtual para interações com botões.
 */
void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int isPlaying_beforePause, int *musicIsCurrentlyPlaying_ptr, Vector2 virtualMousePos);

/**
 * @brief Desenha os elementos da interface do usuário (UI) do menu de pausa.
 * Isso inclui o filtro escurecido, o texto "PAUSADO" e os botões.
 * Esta função é projetada para ser chamada quando nenhuma transformação de câmera está ativa,
 * desenhando os elementos em coordenadas fixas da tela virtual.
 */
void DrawPauseMenuElements(void);

/**
 * @brief Desenha a tela de pausa completa, incluindo o fundo do jogo e a UI de pausa.
 * Esta função é tipicamente chamada por outras telas (como Save/Load) que querem usar
 * a aparência da tela de pausa como seu próprio fundo.
 * @param players_arr Array de jogadores para desenhar o fundo do jogo.
 * @param currentVolume Volume atual da música para o fundo do jogo.
 * @param currentMusicIndex Índice da música atual para o fundo do jogo.
 * @param isPlaying_when_game_paused Estado da música para o fundo do jogo.
 * @param currentMapX Coordenada X do mapa atual para o fundo do jogo.
 * @param currentMapY Coordenada Y do mapa atual para o fundo do jogo.
 */
void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying_when_game_paused, int currentMapX, int currentMapY);

#endif // PAUSE_MENU_H