#include "../include/Singleplayer.h"
#include "../include/Game.h"      // Para move_character e constantes globais como virtualScreenHeight, gameSectionHeightMultiplier
#include "raylib.h"               // Para KEY_A, etc.
#include <stddef.h>
// WorldLoading.h é incluído por Game.h se Singleplayer.h o inclui, ou diretamente se necessário

// As variáveis globais virtualScreenHeight e gameSectionHeightMultiplier
// são acessadas através de 'extern' em Game.h

void SinglePlayer_HandleMovement(Player *player, const WorldSection* activeSection) {
    if (!player) return; // Adicionada verificação de nulo para activeSection se ele puder ser nulo
    if (!activeSection || !activeSection->isLoaded) {
        TraceLog(LOG_WARNING, "SinglePlayer_HandleMovement: Secao ativa invalida ou nao carregada. Usando dimensoes de fallback para move_character.");
        // Fallback para dimensões baseadas em multiplicadores se a seção não estiver carregada
        // Isso evita crash mas o movimento pode não respeitar os limites da seção desejada
        // As globais gameSectionWidthMultiplier e virtualScreenWidth são de Game.h (via main.c)
        move_character(player, KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT, NULL); // Passa NULL se não há seção
                                                                                    // move_character precisará de um fallback para limites se activeSection for NULL
        return;
    }
    move_character(player, KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT, activeSection);
}