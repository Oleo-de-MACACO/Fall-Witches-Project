#include "../include/Menu.h"
#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/SaveLoad.h"
#include "../include/Inventory.h"
#include <stdio.h>
#include <string.h>
#include "raylib.h"

// --- Variáveis Globais Externas ---
// Dimensões da tela virtual (declaradas em main.c)
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;

// Flag global para solicitar saída do jogo (declarada em main.c)
extern bool g_request_exit;

// === Configurações e Variáveis Estáticas para Menus ===
// Botões do Menu Principal
#define NUM_MAIN_MENU_BUTTONS 5
static MenuButton mainMenuButtons[NUM_MAIN_MENU_BUTTONS];
static bool mainMenuButtonsInitialized = false;

// Botões do Menu de Modo de Jogador
#define NUM_PLAYER_MODE_MENU_BUTTONS 2
static MenuButton playerModeMenuButtons[NUM_PLAYER_MODE_MENU_BUTTONS];
static bool playerModeMenuButtonsInitialized = false;

// Botões do Menu de Pausa
#define NUM_PAUSE_MENU_BUTTONS 5
static MenuButton pauseMenuButtons[NUM_PAUSE_MENU_BUTTONS];
static bool pauseMenuButtonsInitialized = false;

// Configurações para a Tela de Save/Load Slots
#define SLOTS_PER_ROW_SAVE_LOAD 6 // Não usado atualmente no layout de lista vertical
#define SLOT_PADDING_SAVE_LOAD 10 // Espaçamento entre slots
#define SLOT_HEIGHT_SAVE_LOAD 70  // Altura de cada slot
static MenuButton saveLoadSlotButtons[MAX_SAVE_SLOTS]; // Array de botões para os slots
static bool saveLoadSlotsInitialized = false; // Flag de inicialização dos slots

// Variáveis de estado para a tela de Save/Load
static float saveLoadScrollOffset = 0.0f; // Deslocamento da barra de rolagem
static float saveLoadTotalContentHeight = 0.0f; // Altura total do conteúdo dos slots
static Rectangle saveLoadSlotsViewArea = {0}; // Área visível para os slots (para rolagem)

static bool s_is_in_save_mode = true;      // true = salvando, false = carregando
static bool s_is_new_game_flow = false;    // true se o menu de save/load foi acessado a partir do fluxo de "Novo Jogo"
static int s_save_load_menu_sub_state = 0; // 0 = selecionando slot, 1 = confirmando sobrescrita
static int s_selected_slot_for_action = -1; // Slot atualmente selecionado para uma ação
static GameState s_previous_screen_before_save_load; // Tela anterior ao menu de save/load

// Botões de confirmação para sobrescrever save
static MenuButton s_confirmationButtons[2]; // [0] = Sim, [1] = Não
static bool s_confirmationButtonsInitialized = false;

// Variáveis de estado para a Tela de Criação de Personagem
static int creation_current_player_idx = 0; // Índice do jogador sendo configurado (não usado diretamente, derivado de creation_step)
static int creation_step = 0; // Passo atual na criação (0-1: P1 nome/classe, 2-3: P2 nome/classe, 4: confirmação)
static char creation_player_names[MAX_PLAYERS][MAX_PLAYER_NAME_LENGTH]; // Nomes dos jogadores
static int creation_name_letter_counts[MAX_PLAYERS] = {0}; // Contagem de letras nos nomes
static Classe creation_player_classes[MAX_PLAYERS]; // Classes dos jogadores
static int creation_class_selection_idx[MAX_PLAYERS] = {0}; // Índice da classe selecionada na lista
static bool creation_name_edit_mode[MAX_PLAYERS] = {false}; // true se o nome do jogador está sendo editado

// Classes disponíveis para seleção e seus nomes de exibição
const Classe AVAILABLE_CLASSES[] = {GUERREIRO, MAGO, ARQUEIRO, BARBARO, LADINO, CLERIGO};
const int NUM_AVAILABLE_CLASSES = sizeof(AVAILABLE_CLASSES) / sizeof(AVAILABLE_CLASSES[0]);
const char* CLASS_DISPLAY_NAMES[] = {"Guerreiro", "Mago", "Arqueiro", "Bárbaro", "Ladino", "Clérigo"};

// Cores padrão para os botões
#define COLOR_BUTTON_ACTIVE MAROON     // Cor base do botão ativo
#define COLOR_BUTTON_HOVER ORANGE      // Cor do botão quando o mouse está sobre ele
#define COLOR_BUTTON_DISABLED GRAY     // Cor do botão desabilitado
#define COLOR_BUTTON_TEXT WHITE        // Cor do texto do botão


// --- Funções do Menu Principal ---
// Inicializa os botões do menu principal, usando virtualScreenWidth/Height para layout.
void InitializeMainMenuButtons() {
    float buttonWidth = 280; float buttonHeight = 50; float spacingY = 20; float spacingX = 20;
    // Calcula a largura de uma linha com dois botões
    float twoButtonRowWidth = 2 * buttonWidth + spacingX;
    // Calcula a altura total do bloco de botões (3 linhas de botões)
    float totalButtonBlockHeight = 3 * buttonHeight + 2 * spacingY;
    // Estimativa da altura do título para posicionar os botões abaixo
    float titleHeightEstimate = virtualScreenHeight / 7.0f + 70;
    float startY = titleHeightEstimate + 40; // Posição Y inicial dos botões

    // Ajusta startY se os botões excederem a altura da tela virtual
    if (startY + totalButtonBlockHeight > virtualScreenHeight - 30) {
        startY = (virtualScreenHeight - totalButtonBlockHeight) / 2.0f;
        if (startY < titleHeightEstimate) startY = titleHeightEstimate;
    }
    // Garante que os botões não comecem muito alto
    if (startY < virtualScreenHeight * 0.3f) startY = virtualScreenHeight * 0.3f;

    // Calcula a posição X inicial para centralizar os botões
    float startX_twoButtons = (virtualScreenWidth - twoButtonRowWidth) / 2.0f;
    float startX_oneButton = (virtualScreenWidth - buttonWidth) / 2.0f;

    // Define as propriedades de cada botão
    mainMenuButtons[0] = (MenuButton){{startX_twoButtons, startY, buttonWidth, buttonHeight}, "Um Jogador (em breve)", COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, LIGHTGRAY, false, false, BUTTON_ACTION_NONE };
    mainMenuButtons[1] = (MenuButton){{startX_twoButtons + buttonWidth + spacingX, startY, buttonWidth, buttonHeight}, "Dois Jogadores", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_GOTO_PLAYER_MODE_MENU };
    float currentY_main = startY + buttonHeight + spacingY;
    mainMenuButtons[2] = (MenuButton){{startX_oneButton, currentY_main, buttonWidth, buttonHeight}, "Multijogador (em breve)", COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, LIGHTGRAY, false, false, BUTTON_ACTION_NONE };
    currentY_main += buttonHeight + spacingY;
    mainMenuButtons[3] = (MenuButton){{startX_twoButtons, currentY_main, buttonWidth, buttonHeight}, "Opções", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_SETTINGS };
    mainMenuButtons[4] = (MenuButton){{startX_twoButtons + buttonWidth + spacingX, currentY_main, buttonWidth, buttonHeight}, "Sair do Jogo", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_QUIT_GAME };
    mainMenuButtonsInitialized = true; // Marca os botões como inicializados
}

// Atualiza a lógica da tela de menu principal.
// Recebe virtualMousePos para interações com os botões.
void UpdateMenuScreen(GameState *currentScreen_ptr, Vector2 virtualMousePos) {
    if (!mainMenuButtonsInitialized) InitializeMainMenuButtons(); // Inicializa os botões se ainda não foram

    // Itera sobre os botões do menu principal
    for (int i = 0; i < NUM_MAIN_MENU_BUTTONS; i++) {
        mainMenuButtons[i].is_hovered = false; // Reseta o estado hover
        // Verifica se o botão está ativo e se o mouse (virtual) está sobre ele
        if (mainMenuButtons[i].is_active && CheckCollisionPointRec(virtualMousePos, mainMenuButtons[i].rect)) {
            mainMenuButtons[i].is_hovered = true; // Marca como hover
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { // Se o botão esquerdo do mouse foi pressionado
                // Executa a ação associada ao botão
                switch (mainMenuButtons[i].action) {
                    case BUTTON_ACTION_GOTO_PLAYER_MODE_MENU:
                        *currentScreen_ptr = GAMESTATE_PLAYER_MODE_MENU; // Muda para o menu de modo de jogador
                        playerModeMenuButtonsInitialized = false; // Reseta flag de inicialização do próximo menu
                        s_is_new_game_flow = false; // Garante que não está no fluxo de novo jogo
                        TraceLog(LOG_INFO, "[Menu Principal] -> Menu de Modo de Jogador");
                        break;
                    case BUTTON_ACTION_SETTINGS:
                        TraceLog(LOG_INFO, "[Menu Principal] Opções (Placeholder)"); // Ação de placeholder
                        break;
                    case BUTTON_ACTION_QUIT_GAME:
                        g_request_exit = true; // Solicita o fechamento do jogo
                        TraceLog(LOG_INFO, "[Menu Principal] Sair do Jogo");
                        break;
                    default: break;
                }
            }
        }
    }
 }

// Desenha a tela de menu principal.
// Usa virtualScreenWidth/Height para layout.
void DrawMenuScreen(void) {
    if (!mainMenuButtonsInitialized) InitializeMainMenuButtons(); // Garante que os botões estão inicializados

    ClearBackground(DARKGRAY); // Limpa a tela virtual com cinza escuro
    // Desenha o título do jogo centralizado na tela virtual
    DrawText("FALL WITCHES", virtualScreenWidth/2 - MeasureText("FALL WITCHES", 70)/2, virtualScreenHeight/7, 70, WHITE);

    // Desenha cada botão do menu
    for (int i = 0; i < NUM_MAIN_MENU_BUTTONS; i++) {
        // Define a cor do botão baseada no seu estado (ativo, hover, desabilitado)
        Color btnC = !mainMenuButtons[i].is_active ? mainMenuButtons[i].disabled_color : (mainMenuButtons[i].is_hovered ? mainMenuButtons[i].hover_color : mainMenuButtons[i].base_color);
        Color txtC = !mainMenuButtons[i].is_active ? DARKGRAY : mainMenuButtons[i].text_color; // Cor do texto
        DrawRectangleRec(mainMenuButtons[i].rect, btnC); // Desenha o retângulo do botão
        DrawRectangleLinesEx(mainMenuButtons[i].rect, 2, Fade(BLACK,0.4f)); // Desenha a borda do botão
        // Desenha o texto do botão centralizado
        int tw = MeasureText(mainMenuButtons[i].text,20);
        DrawText(mainMenuButtons[i].text, mainMenuButtons[i].rect.x+(mainMenuButtons[i].rect.width-tw)/2, mainMenuButtons[i].rect.y+(mainMenuButtons[i].rect.height-20)/2, 20, txtC);
    }
    // Desenha uma instrução na parte inferior da tela virtual
    DrawText("Use o mouse para selecionar.", 10, virtualScreenHeight-20, 10, LIGHTGRAY);
}

// --- Funções da Tela de Introdução ---
// Atualiza a lógica da tela de introdução.
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr) {
    (*introFrames_ptr)++; // Incrementa o contador de frames
    // Se o contador exceder o limite, ou uma tecla/mouse for pressionado, avança para o menu principal
    if (*introFrames_ptr > 180 || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON) ) {
        *currentScreen_ptr = GAMESTATE_MENU; // Muda para o menu principal
        *introFrames_ptr = 0; // Zera o contador
        mainMenuButtonsInitialized = false; // Reseta flag de inicialização do menu principal
    }
 }

// Desenha a tela de introdução.
// Usa virtualScreenWidth/Height para layout.
void DrawIntroScreen(void) {
    ClearBackground(BLACK); // Limpa a tela virtual com preto
    // Desenha textos informativos centralizados na tela virtual
    DrawText("TELA DE INTRODUÇÃO", virtualScreenWidth/2 - MeasureText("TELA DE INTRODUÇÃO", 30)/2, virtualScreenHeight/2 - 40, 30, WHITE);
    DrawText("Fall Witches Engine v0.0.7", virtualScreenWidth/2 - MeasureText("Fall Witches Engine v0.0.6", 20)/2, virtualScreenHeight/2 + 10, 20, LIGHTGRAY);
    DrawText("Pressione ENTER, ESC ou clique para continuar...", virtualScreenWidth/2 - MeasureText("Pressione ENTER, ESC ou clique para continuar...", 10)/2, virtualScreenHeight - 30, 10, GRAY);
}


// --- Funções do Menu de Modo de Jogador (Novo Jogo / Carregar) ---
// Inicializa os botões do menu de modo de jogador, usando virtualScreenWidth/Height para layout.
void InitializePlayerModeMenuButtons() {
    float buttonWidth = 250;
    float buttonHeight = 50;
    float spacingY = 30;
    // Calcula a altura total ocupada pelos botões e espaçamentos
    float totalHeight = NUM_PLAYER_MODE_MENU_BUTTONS * buttonHeight + (NUM_PLAYER_MODE_MENU_BUTTONS - 1) * spacingY;
    // Calcula a posição Y inicial para centralizar verticalmente
    float startY = (virtualScreenHeight - totalHeight) / 2.0f;
    // Calcula a posição X inicial para centralizar horizontalmente
    float startX = (virtualScreenWidth - buttonWidth) / 2.0f;

    // Define as propriedades dos botões "Novo Jogo" e "Carregar Jogo"
    playerModeMenuButtons[0] = (MenuButton){
        {startX, startY, buttonWidth, buttonHeight},
        "Novo Jogo",
        MAROON, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT,
        true, false, BUTTON_ACTION_NEW_GAME_SETUP_SLOT
    };
    playerModeMenuButtons[1] = (MenuButton){
        {startX, startY + buttonHeight + spacingY, buttonWidth, buttonHeight},
        "Carregar Jogo",
        MAROON, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT,
        true, false, BUTTON_ACTION_LOAD_GAME
    };
    playerModeMenuButtonsInitialized = true; // Marca como inicializados
}

// Atualiza a lógica do menu de modo de jogador.
// Recebe virtualMousePos para interações.
void UpdatePlayerModeMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr, Vector2 virtualMousePos) {
    // Parâmetros de música não são usados diretamente aqui, mas mantidos para consistência ou uso futuro
    (void)playlist; (void)currentMusicIndex; (void)currentVolume; (void)isPlaying_ptr;

    if (!playerModeMenuButtonsInitialized) InitializePlayerModeMenuButtons(); // Inicializa se necessário

    // Itera sobre os botões do menu
    for (int i = 0; i < NUM_PLAYER_MODE_MENU_BUTTONS; i++) {
        playerModeMenuButtons[i].is_hovered = false; // Reseta hover
        // Verifica colisão com o mouse virtual
        if (playerModeMenuButtons[i].is_active && CheckCollisionPointRec(virtualMousePos, playerModeMenuButtons[i].rect)) {
            playerModeMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { // Se clicado
                // Executa a ação do botão
                switch (playerModeMenuButtons[i].action) {
                    case BUTTON_ACTION_NEW_GAME_SETUP_SLOT: // Novo Jogo
                        s_is_in_save_mode = true;    // Indica que estamos "salvando" um novo jogo (escolhendo slot)
                        s_is_new_game_flow = true;   // Estamos no fluxo de novo jogo
                        s_save_load_menu_sub_state = 0; // Estado inicial da tela de slots
                        s_previous_screen_before_save_load = GAMESTATE_PLAYER_MODE_MENU; // Tela anterior
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU; // Vai para a tela de seleção de slot
                        saveLoadSlotsInitialized = false; // Reseta flags do próximo menu
                        s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f;
                        TraceLog(LOG_INFO, "[Menu Modo Jogador] Novo Jogo -> Selecionar Slot");
                        break;
                    case BUTTON_ACTION_LOAD_GAME: // Carregar Jogo
                        s_is_in_save_mode = false;   // Indica que estamos carregando
                        s_is_new_game_flow = false;
                        s_save_load_menu_sub_state = 0;
                        s_previous_screen_before_save_load = GAMESTATE_PLAYER_MODE_MENU;
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU; // Vai para a tela de seleção de slot
                        saveLoadSlotsInitialized = false;
                        s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f;
                        TraceLog(LOG_INFO, "[Menu Modo Jogador] Carregar Jogo -> Tela de Slots");
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // Se ESC for pressionado, volta para o menu principal
    if (IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = GAMESTATE_MENU;
        mainMenuButtonsInitialized = false; // Reseta flag do menu principal
        s_is_new_game_flow = false; // Sai do fluxo de novo jogo
    }
}

// Desenha o menu de modo de jogador.
// Usa virtualScreenWidth/Height para layout.
void DrawPlayerModeMenuScreen(void) {
    if (!playerModeMenuButtonsInitialized) InitializePlayerModeMenuButtons(); // Garante inicialização

    ClearBackground(DARKGRAY); // Limpa a tela virtual
    // Desenha o título da tela
    DrawText("DOIS JOGADORES", virtualScreenWidth / 2 - MeasureText("DOIS JOGADORES", 40) / 2, virtualScreenHeight / 4, 40, WHITE);

    // Desenha os botões
    for (int i = 0; i < NUM_PLAYER_MODE_MENU_BUTTONS; i++) {
        Color btnC = !playerModeMenuButtons[i].is_active ? playerModeMenuButtons[i].disabled_color : (playerModeMenuButtons[i].is_hovered ? playerModeMenuButtons[i].hover_color : playerModeMenuButtons[i].base_color);
        Color txtC = !playerModeMenuButtons[i].is_active ? DARKGRAY : playerModeMenuButtons[i].text_color;
        DrawRectangleRec(playerModeMenuButtons[i].rect, btnC);
        DrawRectangleLinesEx(playerModeMenuButtons[i].rect, 2, Fade(BLACK, 0.4f));
        int tw = MeasureText(playerModeMenuButtons[i].text, 20);
        DrawText(playerModeMenuButtons[i].text, playerModeMenuButtons[i].rect.x + (playerModeMenuButtons[i].rect.width - tw) / 2, playerModeMenuButtons[i].rect.y + (playerModeMenuButtons[i].rect.height - 20) / 2, 20, txtC);
    }
    // Instrução na parte inferior
    DrawText("ESC para Voltar ao Menu Principal", 10, virtualScreenHeight - 20, 10, LIGHTGRAY);
}

// --- Tela de Criação de Personagem ---
// Inicializa as variáveis para a tela de criação de personagem.
void InitializeCharacterCreation(void) {
    creation_current_player_idx = 0; // Reseta o índice do jogador (embora derivado de creation_step)
    creation_step = 0;               // Reseta para o primeiro passo (nome do P1)

    for (int i = 0; i < MAX_PLAYERS; i++) {
        strcpy(creation_player_names[i], ""); // Limpa nomes
        creation_name_letter_counts[i] = 0;   // Zera contagem de letras
        creation_player_classes[i] = GUERREIRO; // Classe padrão
        creation_class_selection_idx[i] = 0;    // Índice de seleção de classe padrão
        creation_name_edit_mode[i] = false;     // Modo de edição de nome desativado
    }
    if (MAX_PLAYERS > 0) creation_name_edit_mode[0] = true; // Ativa edição de nome para P1
    TraceLog(LOG_INFO, "Tela de Criação de Personagem Inicializada.");
}

// Atualiza a lógica da tela de criação de personagem.
// Posições dos jogadores (em PrepareNewGameSession) já usam virtualScreenHeight.
void UpdateCharacterCreationScreen(GameState *currentScreen_ptr, Player players[], int *mapX, int *mapY, Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsPlaying_ptr) {
    // Índice do jogador sendo configurado (0 para P1, 1 para P2)
    // Derivado do passo atual: passos 0 e 1 são para P1, passos 2 e 3 para P2.
    int player_idx_configuring = creation_step / 2;

    if (creation_step % 2 == 0) { // Passo de entrada de nome (passos 0, 2)
        creation_name_edit_mode[player_idx_configuring] = true; // Ativa modo de edição de nome
        SetMouseCursor(MOUSE_CURSOR_IBEAM); // Muda cursor para indicar texto

        int key = GetCharPressed(); // Obtém o caractere pressionado (unicode)
        while (key > 0) { // Processa todos os caracteres na fila
            // Adiciona o caractere ao nome se for válido e houver espaço
            if ((key >= 32) && (key <= 125) && (creation_name_letter_counts[player_idx_configuring] < MAX_PLAYER_NAME_LENGTH - 1)) {
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = (char)key;
                creation_name_letter_counts[player_idx_configuring]++;
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = '\0'; // Terminador nulo
            }
            key = GetCharPressed(); // Próximo caractere
        }
        // Tecla Backspace para apagar caractere
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (creation_name_letter_counts[player_idx_configuring] > 0) {
                creation_name_letter_counts[player_idx_configuring]--;
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = '\0';
            }
        }
        // Tecla Enter para confirmar o nome
        if (IsKeyPressed(KEY_ENTER)) {
            if (creation_name_letter_counts[player_idx_configuring] > 0) { // Nome não pode ser vazio
                creation_name_edit_mode[player_idx_configuring] = false; // Desativa modo de edição
                SetMouseCursor(MOUSE_CURSOR_DEFAULT); // Restaura cursor padrão
                creation_step++; // Avança para o passo de seleção de classe
                TraceLog(LOG_INFO, "Jogador %d Nome: '%s' confirmado.", player_idx_configuring + 1, creation_player_names[player_idx_configuring]);
            } else {
                TraceLog(LOG_INFO, "Nome do Jogador %d não pode ser vazio.", player_idx_configuring + 1);
                // Adicionar feedback visual aqui seria bom (ex: caixa de nome pisca)
            }
        }
    } else { // Passo de seleção de classe (passos 1, 3)
        SetMouseCursor(MOUSE_CURSOR_DEFAULT); // Garante cursor padrão
        creation_name_edit_mode[player_idx_configuring] = false; // Garante que não está editando nome

        // Navegação na lista de classes com setas CIMA/BAIXO
        if (IsKeyPressed(KEY_UP)) {
            creation_class_selection_idx[player_idx_configuring]--;
            if (creation_class_selection_idx[player_idx_configuring] < 0) creation_class_selection_idx[player_idx_configuring] = NUM_AVAILABLE_CLASSES - 1;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            creation_class_selection_idx[player_idx_configuring]++;
            if (creation_class_selection_idx[player_idx_configuring] >= NUM_AVAILABLE_CLASSES) creation_class_selection_idx[player_idx_configuring] = 0;
        }
        // Atualiza a classe do jogador baseada no índice selecionado
        creation_player_classes[player_idx_configuring] = AVAILABLE_CLASSES[creation_class_selection_idx[player_idx_configuring]];

        // Tecla Enter para confirmar a classe
        if (IsKeyPressed(KEY_ENTER)) {
            TraceLog(LOG_INFO, "Jogador %d Classe: %s confirmada.", player_idx_configuring + 1, CLASS_DISPLAY_NAMES[creation_class_selection_idx[player_idx_configuring]]);
            if (player_idx_configuring < MAX_PLAYERS - 1) { // Se ainda há jogadores para configurar
                creation_step++; // Avança para o próximo passo (nome do próximo jogador)
                creation_name_edit_mode[player_idx_configuring + 1] = true; // Ativa edição de nome para o próximo jogador
            } else { // Todos os jogadores configurados
                creation_step++; // Avança para o passo de confirmação final
            }
        }
    }

    // Passo de confirmação final (depois de configurar todos os jogadores)
    int final_confirmation_step = MAX_PLAYERS * 2;
    if (creation_step == final_confirmation_step) {
        if (IsKeyPressed(KEY_ENTER)) { // Se Enter for pressionado para confirmar
            // Inicializa cada jogador com o nome e classe escolhidos
            for (int i = 0; i < MAX_PLAYERS; i++) {
                init_player(&players[i], creation_player_names[i], creation_player_classes[i]);
                // Posições iniciais são definidas em PrepareNewGameSession,
                // que já usa virtualScreenWidth/Height.
            }

            *mapX = 0; // Reseta mapa para a origem
            *mapY = 0;

            // Inicia/reinicia a música da playlist principal
            if (playlist[currentMusicIndex].stream.buffer != NULL) {
                if(IsMusicStreamPlaying(playlist[currentMusicIndex])) StopMusicStream(playlist[currentMusicIndex]);
                PlayMusicStream(playlist[currentMusicIndex]);
                SetMusicVolume(playlist[currentMusicIndex], currentVolume);
                if(musicIsPlaying_ptr) *musicIsPlaying_ptr = 1;
            } else { if(musicIsPlaying_ptr) *musicIsPlaying_ptr = 0; }

            *currentScreen_ptr = GAMESTATE_PLAYING; // Muda para a tela de jogo
            TraceLog(LOG_INFO, "[Criação Personagem Completa] -> Iniciando Jogo!");
        }
    }

    // Tecla ESC para voltar um passo ou sair da criação de personagem
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (creation_step > 0) { // Se não for o primeiro passo
            creation_name_edit_mode[player_idx_configuring] = false; // Desativa edição ao voltar
            creation_step--; // Volta um passo
            // Se voltou para um passo de nome, reativa o modo de edição
            int new_player_idx_configuring = creation_step / 2;
            if (creation_step % 2 == 0) {
                 creation_name_edit_mode[new_player_idx_configuring] = true;
            }
        } else { // Se estiver no primeiro passo, volta para o menu de modo de jogador
            *currentScreen_ptr = GAMESTATE_PLAYER_MODE_MENU;
            playerModeMenuButtonsInitialized = false; // Reseta flag de inicialização
            TraceLog(LOG_INFO, "[Criação Personagem] -> Menu Modo Jogador (ESC no primeiro passo)");
        }
    }
}

// Desenha a tela de criação de personagem.
// Usa virtualScreenWidth/Height para layout.
void DrawCharacterCreationScreen(Player players[], int currentPlayerFocus) {
    (void)players; // Parâmetro players não é usado diretamente para desenho nesta versão
    (void)currentPlayerFocus; // Parâmetro currentPlayerFocus não é usado

    ClearBackground(DARKBLUE); // Limpa a tela virtual com azul escuro

    // Índice do jogador sendo configurado atualmente para desenho
    int player_idx_drawing = creation_step / 2;

    // Título da tela
    DrawText("CRIAÇÃO DE PERSONAGEM", virtualScreenWidth/2 - MeasureText("CRIAÇÃO DE PERSONAGEM", 30)/2, 30, 30, WHITE);

    // Coordenadas e dimensões da caixa de conteúdo principal
    int boxX = 50;
    int boxY = 80;
    int boxWidth = virtualScreenWidth - 100; // Largura da caixa

    // Se ainda estiver nos passos de configuração de nome/classe
    if (creation_step < MAX_PLAYERS * 2) {
        DrawText(TextFormat("CONFIGURANDO JOGADOR %d", player_idx_drawing + 1), boxX + 20, boxY + 20, 20, RAYWHITE);

        if (creation_step % 2 == 0) { // Passo de entrada de nome
            DrawText("Digite o Nome:", boxX + 20, boxY + 60, 20, LIGHTGRAY);
            // Caixa de texto para o nome
            DrawRectangleLines(boxX + 20, boxY + 90, boxWidth - 40, 40, WHITE);
            DrawText(creation_player_names[player_idx_drawing], boxX + 25, boxY + 100, 20, YELLOW);
            // Cursor piscante se estiver editando nome
            if (creation_name_edit_mode[player_idx_drawing]) {
                if (((int)(GetTime()*2.0f)) % 2 == 0) { // Pisca a cada meio segundo
                    DrawText("_", boxX + 25 + MeasureText(creation_player_names[player_idx_drawing], 20), boxY + 100, 20, YELLOW);
                }
            }
            DrawText("Pressione ENTER para confirmar o nome.", boxX + 20, boxY + 140, 10, GRAY);
        }
        else { // Passo de seleção de classe
            DrawText(TextFormat("Nome: %s", creation_player_names[player_idx_drawing]), boxX + 20, boxY + 60, 20, YELLOW);
            DrawText("Escolha a Classe:", boxX + 20, boxY + 90, 20, LIGHTGRAY);

            // Lista de classes para seleção
            int classOptionY = boxY + 120;
            for (int i = 0; i < NUM_AVAILABLE_CLASSES; i++) {
                Color textColor = (i == creation_class_selection_idx[player_idx_drawing]) ? ORANGE : WHITE; // Destaca selecionada
                DrawText(TextFormat("%s %s", (i == creation_class_selection_idx[player_idx_drawing]) ? "->" : "  ", CLASS_DISPLAY_NAMES[i]),
                         boxX + 40, classOptionY + i * 25, 20, textColor);
            }
            DrawText("Use SETAS CIMA/BAIXO e ENTER para selecionar.", boxX + 20, classOptionY + NUM_AVAILABLE_CLASSES * 25 + 20, 10, GRAY);

            // Preview dos atributos base da classe selecionada
            Player previewPlayer; // Jogador temporário para obter atributos
            init_player(&previewPlayer, "", AVAILABLE_CLASSES[creation_class_selection_idx[player_idx_drawing]]);
            int previewX = boxX + (boxWidth / 2) + 20; // Posição X para o preview
            int previewY = boxY + 90; // Posição Y para o preview
            DrawText("Atributos Base:", previewX, previewY, 10, RAYWHITE); previewY += 15;
            DrawText(TextFormat("HP:%d MP:%d ATK:%d DEF:%d", previewPlayer.max_vida, previewPlayer.max_mana, previewPlayer.ataque, previewPlayer.defesa), previewX, previewY, 10, LIGHTGRAY); previewY += 15;
            DrawText(TextFormat("FOR:%d PER:%d RES:%d", previewPlayer.forca, previewPlayer.percepcao, previewPlayer.resistencia), previewX, previewY, 10, LIGHTGRAY); previewY += 15;
            DrawText(TextFormat("CAR:%d INT:%d AGI:%d SOR:%d", previewPlayer.carisma, previewPlayer.inteligencia, previewPlayer.agilidade, previewPlayer.sorte), previewX, previewY, 10, LIGHTGRAY);
        }
    } else { // Passo de confirmação final
        DrawText("CONFIRMAR EQUIPE?", virtualScreenWidth/2 - MeasureText("CONFIRMAR EQUIPE?", 20)/2, boxY + 20, 20, YELLOW);
        int summaryY = boxY + 60; // Posição Y inicial para o resumo da equipe
        // Exibe o resumo de cada jogador configurado
        for(int i=0; i < MAX_PLAYERS; i++) {
            DrawText(TextFormat("Jogador %d: %s (%s)", i+1, creation_player_names[i], CLASS_DISPLAY_NAMES[creation_class_selection_idx[i]]),
                     boxX + 20, summaryY, 20, WHITE);
            summaryY += 30; // Próxima linha
        }
        DrawText("Pressione ENTER para Iniciar o Jogo", virtualScreenWidth/2 - MeasureText("Pressione ENTER para Iniciar o Jogo", 20)/2, virtualScreenHeight - 80, 20, LIME);
    }
    // Instrução para voltar/cancelar na parte inferior
    DrawText("Pressione ESC para Voltar/Cancelar", 20, virtualScreenHeight - 30, 10, GRAY);
}


// --- Funções do Menu de Pausa ---
// Inicializa os botões do menu de pausa, usando virtualScreenWidth/Height para layout.
void InitializePauseMenuButtons() {
    float btnContinueWidth = 300; float btnContinueHeight = 60; // Botão "Continuar" maior
    float btnWidth = 220; float btnHeight = 45; // Outros botões
    float spacingY = 15; float spacingX = 20; // Espaçamentos
    float titlePauseFontSize = 50;
    // Posição Y do título "PAUSADO"
    float titlePauseY = virtualScreenHeight / 2.0f - 120;
    // Posição Y inicial para os botões, abaixo do título
    float startY = titlePauseY + titlePauseFontSize + 20;

    // Botão "Continuar"
    pauseMenuButtons[0] = (MenuButton){{(virtualScreenWidth-btnContinueWidth)/2.0f, startY, btnContinueWidth, btnContinueHeight}, "Continuar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_RESUME_GAME};
    float currentY = startY + btnContinueHeight + spacingY; // Atualiza Y para a próxima linha de botões

    // Linha com dois botões ("Salvar", "Carregar")
    float twoButtonRowWidthPause = 2 * btnWidth + spacingX;
    float startX_twoButtonsPause = (virtualScreenWidth - twoButtonRowWidthPause) / 2.0f; // X para centralizar
    pauseMenuButtons[1] = (MenuButton){{startX_twoButtonsPause, currentY, btnWidth, btnHeight}, "Salvar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_SAVE_GAME};
    pauseMenuButtons[2] = (MenuButton){{startX_twoButtonsPause + btnWidth + spacingX, currentY, btnWidth, btnHeight}, "Carregar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_LOAD_GAME};
    currentY += btnHeight + spacingY; // Próxima linha

    // Linha com dois botões ("Opções", "Sair para Menu")
    pauseMenuButtons[3] = (MenuButton){{startX_twoButtonsPause, currentY, btnWidth, btnHeight}, "Opções", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_PAUSE_SETTINGS};
    pauseMenuButtons[4] = (MenuButton){{startX_twoButtonsPause + btnWidth + spacingX, currentY, btnWidth, btnHeight}, "Sair para Menu", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_GOTO_MAIN_MENU};
    pauseMenuButtonsInitialized = true; // Marca como inicializados
}

// Atualiza a lógica do menu de pausa.
// Recebe virtualMousePos para interações.
void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int isPlaying_beforePause, int *musicIsCurrentlyPlaying_ptr, Vector2 virtualMousePos) {
    (void)players; // Parâmetro players não usado diretamente aqui

    if (!pauseMenuButtonsInitialized) InitializePauseMenuButtons(); // Inicializa se necessário

    // Itera sobre os botões de pausa
    for (int i = 0; i < NUM_PAUSE_MENU_BUTTONS; i++) {
        pauseMenuButtons[i].is_hovered = false; // Reseta hover
        // Verifica colisão com mouse virtual
        if (pauseMenuButtons[i].is_active && CheckCollisionPointRec(virtualMousePos, pauseMenuButtons[i].rect)) {
            pauseMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { // Se clicado
                // Executa a ação do botão
                switch (pauseMenuButtons[i].action) {
                    case BUTTON_ACTION_RESUME_GAME: // Continuar jogo
                        *currentScreen_ptr = GAMESTATE_PLAYING; // Volta para a tela de jogo
                        // Retoma a música se estava tocando antes de pausar
                        if(isPlaying_beforePause && playlist[currentMusicIndex].stream.buffer!=NULL) {
                            ResumeMusicStream(playlist[currentMusicIndex]);
                            if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 1;
                        }
                        break;
                    case BUTTON_ACTION_SAVE_GAME: // Salvar jogo
                        s_is_in_save_mode = true;
                        s_is_new_game_flow = false;
                        s_save_load_menu_sub_state = 0;
                        s_previous_screen_before_save_load = GAMESTATE_PAUSE; // Tela anterior é Pausa
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU; // Vai para tela de slots
                        saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f;
                        TraceLog(LOG_INFO, "[Menu Pausa] Salvar Jogo -> Tela de Slots");
                        break;
                    case BUTTON_ACTION_LOAD_GAME: // Carregar jogo
                        s_is_in_save_mode = false;
                        s_is_new_game_flow = false;
                        s_save_load_menu_sub_state = 0;
                        s_previous_screen_before_save_load = GAMESTATE_PAUSE;
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU; // Vai para tela de slots
                        saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f;
                        TraceLog(LOG_INFO, "[Menu Pausa] Carregar Jogo -> Tela de Slots");
                        break;
                    case BUTTON_ACTION_PAUSE_SETTINGS: // Opções (Placeholder)
                        TraceLog(LOG_INFO, "[Menu Pausa] Opções (Placeholder)");
                        break;
                    case BUTTON_ACTION_GOTO_MAIN_MENU: // Sair para o Menu Principal
                        *currentScreen_ptr = GAMESTATE_MENU; // Volta para o menu principal
                        if(playlist[currentMusicIndex].stream.buffer!=NULL) StopMusicStream(playlist[currentMusicIndex]); // Para a música atual
                        if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 0;
                        mainMenuButtonsInitialized = false; // Reseta flag do menu principal
                        s_is_new_game_flow = false;
                        break;
                    default: break;
                }
            }
        }
    }
    // Se ESC ou P for pressionado, volta para o jogo
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
        *currentScreen_ptr = GAMESTATE_PLAYING;
        // Retoma a música se estava tocando
        if(isPlaying_beforePause && playlist[currentMusicIndex].stream.buffer!=NULL) {
            ResumeMusicStream(playlist[currentMusicIndex]);
            if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 1;
        }
    }
 }

// Desenha a tela de pausa.
// Usa virtualScreenWidth/Height para layout.
void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying_when_game_paused, int currentMapX, int currentMapY) {
    if (!pauseMenuButtonsInitialized) InitializePauseMenuButtons(); // Garante inicialização

    // 1. Desenha a tela de jogo por baixo (como estava quando pausou)
    // Esta função já desenha usando coordenadas virtuais se chamada dentro do BeginTextureMode.
    DrawPlayingScreen(players_arr, currentVolume, currentMusicIndex, isPlaying_when_game_paused, currentMapX, currentMapY);

    // 2. Sobrepõe um filtro escuro para dar efeito de pausa
    DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.85f));

    // 3. Desenha o título "PAUSADO"
    DrawText("PAUSADO", virtualScreenWidth/2 - MeasureText("PAUSADO", 50)/2, virtualScreenHeight/2.0f - 120, 50, GRAY);

    // 4. Desenha os botões do menu de pausa
    for (int i = 0; i < NUM_PAUSE_MENU_BUTTONS; i++) {
        Color btnC = !pauseMenuButtons[i].is_active ? pauseMenuButtons[i].disabled_color : (pauseMenuButtons[i].is_hovered ? pauseMenuButtons[i].hover_color : pauseMenuButtons[i].base_color);
        Color txtC = !pauseMenuButtons[i].is_active ? DARKGRAY : pauseMenuButtons[i].text_color;
        DrawRectangleRec(pauseMenuButtons[i].rect, btnC);
        DrawRectangleLinesEx(pauseMenuButtons[i].rect, 2, Fade(BLACK,0.4f));
        int tw = MeasureText(pauseMenuButtons[i].text,20);
        DrawText(pauseMenuButtons[i].text, pauseMenuButtons[i].rect.x+(pauseMenuButtons[i].rect.width-tw)/2, pauseMenuButtons[i].rect.y+(pauseMenuButtons[i].rect.height-20)/2, 20, txtC);
    }
}

// --- Funções para a Tela de Slots de Save/Load ---
// Inicializa os botões/layout da tela de save/load, usando virtualScreenWidth/Height.
void InitializeSaveLoadSlotsMenuButtons() {
    float panelPadding = 20; // Espaçamento nas bordas do painel de slots
    float topOffsetForTitle = 70; // Espaço no topo para o título da tela
    float bottomOffsetForEsc = 30; // Espaço na base para instruções (ESC)

    // Define a área visível para os slots (para a barra de rolagem)
    saveLoadSlotsViewArea = (Rectangle){
        panelPadding,
        topOffsetForTitle,
        (float)virtualScreenWidth - 2 * panelPadding, // Largura da área de slots
        (float)virtualScreenHeight - topOffsetForTitle - bottomOffsetForEsc // Altura da área de slots
    };
    // Largura de cada slot (ocupa toda a largura da área de visualização menos um preenchimento)
    float slotWidth = saveLoadSlotsViewArea.width - 2 * SLOT_PADDING_SAVE_LOAD;

    saveLoadTotalContentHeight = 0; // Zera a altura total do conteúdo para recálculo
    // Inicializa cada botão de slot
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        saveLoadSlotButtons[i].rect.x = SLOT_PADDING_SAVE_LOAD; // Posição X relativa à saveLoadSlotsViewArea
        // Posição Y é acumulada, formando uma lista vertical
        saveLoadSlotButtons[i].rect.y = i * (SLOT_HEIGHT_SAVE_LOAD + SLOT_PADDING_SAVE_LOAD);
        saveLoadSlotButtons[i].rect.width = slotWidth;
        saveLoadSlotButtons[i].rect.height = SLOT_HEIGHT_SAVE_LOAD;
        saveLoadSlotButtons[i].text = ""; // Texto será definido dinamicamente (Slot X - Ocupado/Vazio)
        saveLoadSlotButtons[i].base_color = DARKGRAY; // Cor base
        saveLoadSlotButtons[i].hover_color = GRAY;    // Cor ao passar o mouse
        saveLoadSlotButtons[i].disabled_color = DARKGRAY; // Cor desabilitado (não usado ativamente para slots individuais)
        saveLoadSlotButtons[i].text_color = WHITE;    // Cor do texto
        saveLoadSlotButtons[i].is_active = true;      // Todos os slots são clicáveis inicialmente
        saveLoadSlotButtons[i].is_hovered = false;    // Estado hover inicial
        saveLoadSlotButtons[i].action = BUTTON_ACTION_NONE; // Ação é tratada no update

        // Acumula a altura total do conteúdo para a barra de rolagem
        if (i < MAX_SAVE_SLOTS -1) {
            saveLoadTotalContentHeight += SLOT_HEIGHT_SAVE_LOAD + SLOT_PADDING_SAVE_LOAD;
        } else {
            saveLoadTotalContentHeight += SLOT_HEIGHT_SAVE_LOAD; // Último slot não tem padding abaixo
        }
    }
    saveLoadSlotsInitialized = true; // Marca como inicializado
}

// Inicializa os botões de confirmação (Sim/Não) para sobrescrever save.
// Usa virtualScreenWidth/Height para centralizar a caixa de diálogo.
void InitializeSaveLoadConfirmationButtons() {
    float btnWidth = 120; float btnHeight = 40; // Dimensões dos botões Sim/Não
    // Largura e altura da caixa de diálogo de confirmação
    float boxWidth = s_is_new_game_flow ? 550 : 450; // Caixa maior se for fluxo de novo jogo (mais texto)
    float boxHeight = s_is_new_game_flow ? 180 : 150;

    // Posição X e Y para centralizar a caixa na tela virtual
    float boxX = (virtualScreenWidth - boxWidth) / 2.0f;
    float boxY = (virtualScreenHeight - boxHeight) / 2.0f;
    float spacing = 20; // Espaçamento entre os botões Sim e Não
    float totalBtnWidth = 2 * btnWidth + spacing; // Largura total ocupada pelos dois botões e espaçamento
    // Posição Y dos botões dentro da caixa de diálogo (na parte inferior)
    float buttonY = boxY + boxHeight - btnHeight - 20;

    // Botão "Sim"
    s_confirmationButtons[0] = (MenuButton){
        {boxX + (boxWidth - totalBtnWidth)/2.0f, buttonY, btnWidth, btnHeight}, // Posição e dimensões
        "Sim", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, GRAY, WHITE, true, false, BUTTON_ACTION_NONE };
    // Botão "Não"
    s_confirmationButtons[1] = (MenuButton){
        {boxX + (boxWidth - totalBtnWidth)/2.0f + btnWidth + spacing, buttonY, btnWidth, btnHeight},
        "Não", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, GRAY, WHITE, true, false, BUTTON_ACTION_NONE };
    s_confirmationButtonsInitialized = true; // Marca como inicializados
 }

// Atualiza a lógica da tela de save/load.
// Recebe virtualMousePos para interações.
// Adicionada a lógica para transicionar para GAMESTATE_CHARACTER_CREATION.
void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsCurrentlyPlaying_ptr, int *currentMapX_ptr, int *currentMapY_ptr, Vector2 virtualMousePos) {
    if (!saveLoadSlotsInitialized) InitializeSaveLoadSlotsMenuButtons(); // Garante inicialização dos slots

    float wheelMove = GetMouseWheelMove(); // Obtém movimento da roda do mouse para rolagem

    // Sub-estado 0: Selecionando um slot
    if (s_save_load_menu_sub_state == 0) {
        // Lógica da barra de rolagem com a roda do mouse
        // Verifica se o mouse (real) está sobre a área dos slots para permitir rolagem
        // Nota: GetMousePosition() é usado aqui porque a rolagem é uma interação com a janela, não com elementos virtuais.
        // No entanto, a área de colisão saveLoadSlotsViewArea é definida em coordenadas virtuais.
        // Para ser mais preciso, a checagem de colisão aqui deveria usar coordenadas da janela real,
        // ou a rolagem só funcionar quando o mouse virtual está dentro da área.
        // Por simplicidade, a colisão da área de rolagem pode ser considerada com a janela inteira
        // ou uma área maior se não houver outros elementos roláveis.
        // Aqui, vamos assumir que a rolagem é permitida se o mouse estiver sobre a área visível dos slots na tela.
        // Uma forma de fazer isso é converter saveLoadSlotsViewArea para coordenadas de tela e checar GetMousePosition().
        // Ou, simplificando, rolar se wheelMove != 0 e a tela está ativa.

        if (wheelMove != 0) { // Se a roda do mouse foi movida
            saveLoadScrollOffset += wheelMove * SLOT_HEIGHT_SAVE_LOAD; // Ajusta o deslocamento
            float maxScroll = 0.0f; // Deslocamento máximo permitido
            // Se o conteúdo total é maior que a área visível, há rolagem
            if (saveLoadTotalContentHeight > saveLoadSlotsViewArea.height) {
                maxScroll = saveLoadTotalContentHeight - saveLoadSlotsViewArea.height;
            }
            // Limita o deslocamento para não rolar além do conteúdo
            if (saveLoadScrollOffset > 0) saveLoadScrollOffset = 0; // Não rolar para cima além do início
            if (maxScroll > 0 && saveLoadScrollOffset < -maxScroll) saveLoadScrollOffset = -maxScroll; // Não rolar para baixo além do fim
            else if (maxScroll <=0) saveLoadScrollOffset = 0; // Sem rolagem se conteúdo cabe
        }
        // Tecla ESC para voltar para a tela anterior
        if (IsKeyPressed(KEY_ESCAPE)) {
            *currentScreen_ptr = s_previous_screen_before_save_load; // Restaura tela anterior
            // Reseta flags de inicialização da tela anterior, se aplicável
            if (s_previous_screen_before_save_load == GAMESTATE_PLAYER_MODE_MENU) playerModeMenuButtonsInitialized = false;
            else if (s_previous_screen_before_save_load == GAMESTATE_PAUSE) pauseMenuButtonsInitialized = false;
            s_is_new_game_flow = false; // Sai do fluxo de novo jogo
            // Reseta flags e estado desta tela
            saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f;
            return; // Sai da função de atualização
        }

        // Verifica interação com cada slot
        for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
            // Calcula o retângulo do slot na tela virtual, considerando o deslocamento da rolagem
            Rectangle onScreenSlotRect = {
                saveLoadSlotsViewArea.x + saveLoadSlotButtons[i].rect.x,
                saveLoadSlotsViewArea.y + saveLoadSlotButtons[i].rect.y + saveLoadScrollOffset,
                saveLoadSlotButtons[i].rect.width,
                saveLoadSlotButtons[i].rect.height
            };
            saveLoadSlotButtons[i].is_hovered = false; // Reseta hover

            // Verifica se o slot está visível na área de rolagem e se o mouse virtual está sobre ele
            if (CheckCollisionRecs(onScreenSlotRect, saveLoadSlotsViewArea)) { // Garante que só interage com slots visíveis
                 if (CheckCollisionPointRec(virtualMousePos, onScreenSlotRect)) {
                    saveLoadSlotButtons[i].is_hovered = true; // Marca hover
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { // Se clicado
                        s_selected_slot_for_action = i; // Armazena o slot selecionado

                        if (s_is_in_save_mode) { // Se estamos salvando
                            if (DoesSaveSlotExist(s_selected_slot_for_action)) { // Se o slot já existe
                                s_save_load_menu_sub_state = 1; // Vai para o sub-estado de confirmação de sobrescrita
                                s_confirmationButtonsInitialized = false; // Reseta flag dos botões de confirmação
                            } else { // Se o slot está vazio
                                if (s_is_new_game_flow) { // Se está no fluxo de "Novo Jogo"
                                    // Prepara a sessão para um novo jogo (reseta mapa, posições iniciais dos jogadores)
                                    // PrepareNewGameSession já usa virtualScreenHeight/Width.
                                    PrepareNewGameSession(players, currentMapX_ptr, currentMapY_ptr);
                                    InitializeCharacterCreation(); // Prepara a tela de criação de personagem
                                    *currentScreen_ptr = GAMESTATE_CHARACTER_CREATION; // Muda para a tela de criação
                                } else { // Se está salvando um jogo em progresso
                                    if (SaveGame(players, MAX_PLAYERS, s_selected_slot_for_action, *currentMapX_ptr, *currentMapY_ptr)) { /* Sucesso */ }
                                    *currentScreen_ptr = s_previous_screen_before_save_load; // Volta para tela anterior
                                }
                            }
                        } else { // Se estamos carregando
                            if (DoesSaveSlotExist(s_selected_slot_for_action)) { // Se o slot existe
                                if (LoadGame(players, MAX_PLAYERS, s_selected_slot_for_action, currentMapX_ptr, currentMapY_ptr)) { // Tenta carregar
                                    *currentScreen_ptr = GAMESTATE_PLAYING; // Vai para a tela de jogo
                                    // Reinicia a música
                                    if (playlist[currentMusicIndex].stream.buffer != NULL) {
                                         if(IsMusicStreamPlaying(playlist[currentMusicIndex])) StopMusicStream(playlist[currentMusicIndex]);
                                         PlayMusicStream(playlist[currentMusicIndex]);
                                         SetMusicVolume(playlist[currentMusicIndex], currentVolume);
                                         if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 1;
                                    } else { if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 0; }
                                } else { /* Falha ao carregar, exibir mensagem? */ }
                            } else { /* Slot vazio, não pode carregar, exibir mensagem? */ }
                        }
                        // Se mudou de tela ou sub-estado, reseta flags para forçar reinicialização do layout na próxima vez
                        if (*currentScreen_ptr != GAMESTATE_SAVE_LOAD_MENU || s_save_load_menu_sub_state == 1) {
                           saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f;
                        }
                        // Se voltou para a tela anterior, reseta flags dela
                        if (*currentScreen_ptr == s_previous_screen_before_save_load) {
                             if (s_previous_screen_before_save_load == GAMESTATE_PLAYER_MODE_MENU) playerModeMenuButtonsInitialized = false;
                             else if (s_previous_screen_before_save_load == GAMESTATE_PAUSE) pauseMenuButtonsInitialized = false;
                        }
                        return; // Sai da função de atualização após uma ação
                    }
                }
            }
        }
    } else if (s_save_load_menu_sub_state == 1) { // Sub-estado 1: Confirmando sobrescrita/novo jogo em slot ocupado
        if (!s_confirmationButtonsInitialized) InitializeSaveLoadConfirmationButtons(); // Garante inicialização dos botões Sim/Não

        // Tecla ESC para cancelar a confirmação e voltar para a seleção de slot
        if (IsKeyPressed(KEY_ESCAPE)) {
            s_save_load_menu_sub_state = 0; // Volta para o sub-estado de seleção
            s_selected_slot_for_action = -1; // Reseta slot selecionado
            saveLoadSlotsInitialized = false; // Força reinicialização dos slots (layout)
            return;
        }
        // Verifica interação com os botões Sim/Não
        for (int i = 0; i < 2; i++) { // 0 = Sim, 1 = Não
            s_confirmationButtons[i].is_hovered = false; // Reseta hover
            if (CheckCollisionPointRec(virtualMousePos, s_confirmationButtons[i].rect)) { // Colisão com mouse virtual
                s_confirmationButtons[i].is_hovered = true;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { // Se clicado
                    if (i == 0) { // Botão "Sim" foi clicado
                        if (s_is_new_game_flow) { // Se no fluxo de "Novo Jogo" e confirmou sobrescrever
                            PrepareNewGameSession(players, currentMapX_ptr, currentMapY_ptr);
                            InitializeCharacterCreation();
                            *currentScreen_ptr = GAMESTATE_CHARACTER_CREATION; // Vai para criação de personagem
                        } else { // Se salvando jogo em progresso e confirmou sobrescrever
                            if (SaveGame(players, MAX_PLAYERS, s_selected_slot_for_action, *currentMapX_ptr, *currentMapY_ptr)) { /* Sucesso */ }
                            *currentScreen_ptr = s_previous_screen_before_save_load; // Volta para tela anterior
                        }
                    } else { // Botão "Não" foi clicado
                         s_save_load_menu_sub_state = 0; // Volta para seleção de slot
                    }

                    // Se mudou de tela ou sub-estado, reseta flags
                    if (*currentScreen_ptr != GAMESTATE_SAVE_LOAD_MENU || s_save_load_menu_sub_state == 0){
                        saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f;
                    }
                    s_selected_slot_for_action = -1; // Reseta slot selecionado
                    // Se voltou para a tela anterior, reseta flags dela
                    if(*currentScreen_ptr == s_previous_screen_before_save_load) {
                        if (s_previous_screen_before_save_load == GAMESTATE_PLAYER_MODE_MENU) playerModeMenuButtonsInitialized = false;
                        else if (s_previous_screen_before_save_load == GAMESTATE_PAUSE) pauseMenuButtonsInitialized = false;
                        // Garante que s_is_new_game_flow seja resetado se não for para a criação de personagem
                        if (!s_is_new_game_flow && (*currentScreen_ptr != GAMESTATE_CHARACTER_CREATION)) s_is_new_game_flow = false;
                    }
                    return; // Sai da função de atualização
                }
            }
        }
    }
}


// Desenha a tela de save/load.
// Usa virtualScreenWidth/Height para layout.
void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, int musicIsPlaying, float musicVolume, int mapX, int mapY) {
    (void)playlist; // Parâmetro playlist não usado diretamente aqui

    // 1. Desenha a tela de fundo apropriada
    // Se veio da tela de pausa, desenha a tela de pausa (que inclui o jogo)
    if (s_previous_screen_before_save_load == GAMESTATE_PAUSE) {
        DrawPauseScreen(players, musicVolume, currentMusicIndex, musicIsPlaying, mapX, mapY);
    }
    // Se veio do menu de modo de jogador, desenha um fundo escuro simples
    else if (s_previous_screen_before_save_load == GAMESTATE_PLAYER_MODE_MENU) {
        ClearBackground(DARKGRAY);
        DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.85f)); // Escurece
    }
    // Caso contrário (fallback, não deveria acontecer com a lógica atual)
    else {
        ClearBackground(DARKGRAY);
        DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.85f));
    }
    // Sobrepõe um filtro escuro adicional para destacar o menu de save/load
    DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.6f));

    // Garante que os layouts dos botões estão inicializados
    if (!saveLoadSlotsInitialized) InitializeSaveLoadSlotsMenuButtons();
    if (s_save_load_menu_sub_state == 1 && !s_confirmationButtonsInitialized) {
        InitializeSaveLoadConfirmationButtons();
    }

    // Título da tela (varia se é novo jogo, salvar ou carregar)
    char title[128];
    if (s_is_new_game_flow) {
        sprintf(title, "SELECIONAR SLOT PARA NOVO JOGO");
    } else {
        sprintf(title, "ESCOLHA UM SLOT PARA %s", s_is_in_save_mode ? "SALVAR" : "CARREGAR");
    }
    DrawText(title, (virtualScreenWidth - MeasureText(title, 30)) / 2, 30, 30, WHITE);

    // Se no sub-estado de seleção de slot
    if (s_save_load_menu_sub_state == 0) {
        // Ativa o modo Scissor para restringir o desenho à área de rolagem dos slots
        BeginScissorMode((int)saveLoadSlotsViewArea.x, (int)saveLoadSlotsViewArea.y, (int)saveLoadSlotsViewArea.width, (int)saveLoadSlotsViewArea.height);
            // Desenha cada slot
            for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                // Calcula a posição Y do slot na tela, considerando o deslocamento da rolagem
                float onScreenSlotY = saveLoadSlotsViewArea.y + saveLoadSlotButtons[i].rect.y + saveLoadScrollOffset;
                // Retângulo do slot na tela (para desenho e colisão)
                Rectangle onScreenSlotRect = {
                    saveLoadSlotsViewArea.x + saveLoadSlotButtons[i].rect.x,
                    onScreenSlotY,
                    saveLoadSlotButtons[i].rect.width,
                    saveLoadSlotButtons[i].rect.height
                };
                // Só desenha o slot se ele estiver (parcialmente) visível na área de Scissor
                if ((onScreenSlotRect.y + onScreenSlotRect.height > saveLoadSlotsViewArea.y) && (onScreenSlotRect.y < saveLoadSlotsViewArea.y + saveLoadSlotsViewArea.height)) {
                    Color slotColor = saveLoadSlotButtons[i].is_hovered ? LIGHTGRAY : DARKGRAY;
                    // Se estiver carregando e o slot estiver vazio, escurece para indicar que não pode ser selecionado (embora ainda clicável para feedback)
                    if (!s_is_in_save_mode && !DoesSaveSlotExist(i)) {
                        slotColor = Fade(DARKGRAY, 0.5f);
                    }
                    DrawRectangleRec(onScreenSlotRect, slotColor); // Desenha o fundo do slot
                    DrawRectangleLinesEx(onScreenSlotRect, 2, GRAY); // Desenha a borda do slot

                    // Texto do slot (Número - Ocupado/Vazio)
                    char slotText[64];
                    if (DoesSaveSlotExist(i)) {
                        sprintf(slotText, "Slot %d - Ocupado", i + 1);
                        DrawText(slotText, (int)(onScreenSlotRect.x + 10), (int)(onScreenSlotRect.y + (onScreenSlotRect.height - 20)/2), 20, WHITE);
                    } else {
                        sprintf(slotText, "Slot %d - Vazio", i + 1);
                        DrawText(slotText, (int)(onScreenSlotRect.x + 10), (int)(onScreenSlotRect.y + (onScreenSlotRect.height - 20)/2), 20, LIGHTGRAY);
                    }
                }
            }
        EndScissorMode(); // Termina o modo Scissor

        // Desenha a barra de rolagem se necessário
        float maxScroll = 0.0f;
        if (saveLoadTotalContentHeight > saveLoadSlotsViewArea.height) {
            maxScroll = saveLoadTotalContentHeight - saveLoadSlotsViewArea.height;
        }
        if (maxScroll > 0) { // Só desenha a barra se houver conteúdo para rolar
            // Área da barra de rolagem (à direita da área dos slots)
            Rectangle scrollBarArea = { saveLoadSlotsViewArea.x + saveLoadSlotsViewArea.width + 2, saveLoadSlotsViewArea.y, 8, saveLoadSlotsViewArea.height };
            DrawRectangleRec(scrollBarArea, LIGHTGRAY); // Fundo da barra de rolagem
            // "Polegar" da barra de rolagem (indicador da posição atual)
            float thumbHeight = (saveLoadSlotsViewArea.height / saveLoadTotalContentHeight) * scrollBarArea.height;
            if (thumbHeight < 20) thumbHeight = 20; // Altura mínima do polegar
            float thumbY = scrollBarArea.y;
            if (maxScroll > 0) thumbY += (-saveLoadScrollOffset / maxScroll) * (scrollBarArea.height - thumbHeight);
            DrawRectangle((int)scrollBarArea.x, (int)thumbY, (int)scrollBarArea.width, (int)thumbHeight, DARKGRAY);
        }
        // Instruções na parte inferior da tela
        DrawText("ESC para Voltar | Roda do Mouse para Scroll", 10, virtualScreenHeight - 20, 10, WHITE);

    } else if (s_save_load_menu_sub_state == 1) { // Se no sub-estado de confirmação
        // Dimensões da caixa de diálogo de confirmação
        float boxWidth = s_is_new_game_flow ? 550 : 450;
        float boxHeight = s_is_new_game_flow ? 180 : 150;
        Rectangle confBoxRect = {(virtualScreenWidth - boxWidth)/2.0f, (virtualScreenHeight - boxHeight)/2.0f, boxWidth, boxHeight};
        DrawRectangleRec(confBoxRect, Fade(BLACK, 0.95f)); // Fundo da caixa (quase opaco)
        DrawRectangleLinesEx(confBoxRect, 2, WHITE); // Borda da caixa

        float textY = confBoxRect.y + 20; // Posição Y inicial para o texto dentro da caixa
        float textPadding = 15; // Preenchimento lateral para o texto

        // Texto de confirmação (varia se é novo jogo ou sobrescrever)
        if (s_is_new_game_flow) {
            char line1[128], line2[128], line3[128];
            sprintf(line1, "Slot %d contém dados salvos.", s_selected_slot_for_action + 1);
            sprintf(line2, "Iniciar novo jogo aqui?");
            sprintf(line3, "(O jogo anterior será perdido se não salvo em outro slot)");

            DrawText(line1, (int)(confBoxRect.x + (boxWidth - MeasureText(line1, 20)) / 2.0f), (int)textY, 20, WHITE);
            textY += 25;
            DrawText(line2, (int)(confBoxRect.x + (boxWidth - MeasureText(line2, 20)) / 2.0f), (int)textY, 20, WHITE);
            textY += 25 + 5;
            DrawText(line3, (int)(confBoxRect.x + (boxWidth - MeasureText(line3, 10)) / 2.0f), (int)textY, 10, LIGHTGRAY);
        } else { // Texto para sobrescrever save existente
            char confText[128];
            sprintf(confText, "Você quer sobrescrever o Save %d?", s_selected_slot_for_action + 1);
            DrawText(confText, (int)(confBoxRect.x + (boxWidth - MeasureText(confText, 20)) / 2.0f), (int)(confBoxRect.y + (boxHeight - s_confirmationButtons[0].rect.height - 20 - 20 - 10) / 2.0f), 20, WHITE);
        }

        // Desenha os botões Sim/Não
        for (int i = 0; i < 2; i++) {
            Color btnColor = s_confirmationButtons[i].is_hovered ? ORANGE : MAROON;
            DrawRectangleRec(s_confirmationButtons[i].rect, btnColor);
            DrawText(s_confirmationButtons[i].text,
                     (int)(s_confirmationButtons[i].rect.x + (s_confirmationButtons[i].rect.width - MeasureText(s_confirmationButtons[i].text, 20)) / 2.0f),
                     (int)(s_confirmationButtons[i].rect.y + (s_confirmationButtons[i].rect.height - 20) / 2.0f), 20, WHITE);
        }
        // Instrução para cancelar dentro da caixa de diálogo
        DrawText("ESC para Cancelar", (int)(confBoxRect.x + textPadding), (int)(confBoxRect.y + confBoxRect.height - 20), 10, GRAY);
    }
}