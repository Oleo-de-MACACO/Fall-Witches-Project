#ifndef SETTINGS_H
#define SETTINGS_H

#include "raylib.h"
#include "Game.h" // Para GameState

/**
 * @brief Inicializa a tela de configurações.
 * Deve ser chamada ao entrar na tela de configurações para definir estados iniciais,
 * e para registrar de qual tela o menu de configurações foi acessado, para retorno.
 * @param previousScreen A tela da qual as configurações foram abertas.
 */
void InitializeSettingsScreen(GameState previousScreen);

/**
 * @brief Atualiza a lógica da tela de configurações.
 * Lida com a entrada do usuário para interagir com os elementos da UI (sliders, botões)
 * e para sair da tela de configurações.
 * @param currentScreen_ptr Ponteiro para o estado atual do jogo, para permitir alterá-lo.
 */
void UpdateSettingsScreen(GameState *currentScreen_ptr);

/**
 * @brief Desenha a tela de configurações.
 * Renderiza todos os elementos da UI, como títulos de seção, sliders de volume,
 * placeholders de keybinding e botões de navegação.
 */
void DrawSettingsScreen(void);

#endif // SETTINGS_H