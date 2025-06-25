#include "../include/Dialogue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // Para isspace
#include "raylib.h" // Para desenho, input e structs

// --- Armazenamento Global de Diálogos e Estado Interno ---
static Dialogue* s_allDialogues = NULL;
static int s_dialogueCount = 0;
static int s_dialogueCapacity = 0;

static bool s_isDialogueActive = false;
static const Dialogue* s_currentDialogue = NULL;
static int s_currentLineIndex = 0;

// --- Estado do Efeito de Digitação ---
static int s_visibleCharacters = 0;   // Quantos caracteres da linha atual são visíveis
static float s_typingTimer = 0.0f;    // Timer para controlar a velocidade de digitação
static const float CHARS_PER_MINUTE = 500.0f; // Caracteres por minuto
static const float TIME_PER_CHAR = 60.0f / CHARS_PER_MINUTE; // Tempo para cada caractere aparecer

// --- Constantes da UI de Diálogo ---
#define DIALOGUE_BOX_HEIGHT 140
#define DIALOGUE_BOX_PADDING 10
#define SPEAKER_TEXT_Y_OFFSET 15
#define DIALOGUE_TEXT_Y_OFFSET 45
#define TEXT_FONT_SIZE 20
#define PROMPT_FONT_SIZE 15

// --- Funções Auxiliares Internas ---
static void AddDialogueToList(Dialogue dialogue) {
    if (s_dialogueCount >= s_dialogueCapacity) {
        // *** CORRIGIDO: Usava s_eventCapacity por engano ***
        s_dialogueCapacity = (s_dialogueCapacity == 0) ? 16 : s_dialogueCapacity * 2;
        s_allDialogues = (Dialogue*)RL_REALLOC(s_allDialogues, (size_t)s_dialogueCapacity * sizeof(Dialogue));
        if (!s_allDialogues) { TraceLog(LOG_FATAL, "Dialogue_LoadAll: Falha ao alocar memoria para dialogos!"); return; }
    }
    s_allDialogues[s_dialogueCount++] = dialogue;
}
static void AddLineToDialogue(Dialogue* dialogue, DialogueLine line) {
    if (!dialogue) return;
    DialogueLine* newLines = (DialogueLine*)RL_REALLOC(dialogue->lines, (size_t)(dialogue->lineCount + 1) * sizeof(DialogueLine));
    if (!newLines) { TraceLog(LOG_ERROR, "Falha ao realocar memoria para linhas de dialogo."); return; }
    dialogue->lines = newLines;
    dialogue->lines[dialogue->lineCount] = line;
    dialogue->lineCount++;
}
static char* TrimWhitespace(char* str) {
    char *end; while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str; end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0'; return str;
}


// --- Funções Públicas ---
void Dialogue_LoadAll(const char* filePath) {
    if (s_allDialogues) { Dialogue_UnloadAll(); }

    FILE* file = fopen(filePath, "rt");
    if (!file) {
        TraceLog(LOG_ERROR, "Dialogue_LoadAll: Falha ao abrir o arquivo de dialogos: %s", filePath);
        return;
    }
    char lineBuffer[1024];
    // *** CORRIGIDO: Inicialização explícita para evitar warnings ***
    Dialogue currentDialogue = {.id = -1, .lines = NULL, .lineCount = 0};

    while (fgets(lineBuffer, sizeof(lineBuffer), file)) {
        char* trimmedLine = TrimWhitespace(lineBuffer);
        if (strlen(trimmedLine) == 0 || (trimmedLine[0] == '/' && trimmedLine[1] == '/')) continue;

        if (strncmp(trimmedLine, "---", 3) == 0) {
            if (currentDialogue.id != -1) { AddDialogueToList(currentDialogue); }
            currentDialogue = (Dialogue){.id = -1, .lines = NULL, .lineCount = 0};
            continue;
        }
        if (trimmedLine[0] == '#') {
            if (currentDialogue.id != -1) { AddDialogueToList(currentDialogue); } // Inicia um novo diálogo
            currentDialogue = (Dialogue){.id = 0, .lines = NULL, .lineCount = 0};
            sscanf(trimmedLine + 1, "%d", &currentDialogue.id);
            continue;
        }
        if (currentDialogue.id != -1) {
            char* speaker = strtok(trimmedLine, ":"); char* text = strtok(NULL, "\n");
            if (speaker && text) {
                // *** CORRIGIDO: Inicialização explícita para evitar warnings ***
                DialogueLine dialogueLine = {.speaker = {0}, .text = {0}};
                strncpy(dialogueLine.speaker, TrimWhitespace(speaker), MAX_SPEAKER_NAME_LENGTH - 1);
                strncpy(dialogueLine.text, TrimWhitespace(text), MAX_DIALOGUE_LINE_LENGTH - 1);
                AddLineToDialogue(&currentDialogue, dialogueLine);
            }
        }
    }
    if (currentDialogue.id != -1) { AddDialogueToList(currentDialogue); }
    fclose(file);
    TraceLog(LOG_INFO, "%d dialogos carregados de '%s'", s_dialogueCount, filePath);
}

void Dialogue_UnloadAll(void) {
    if (s_allDialogues) {
        for (int i = 0; i < s_dialogueCount; i++) {
            if (s_allDialogues[i].lines) { RL_FREE(s_allDialogues[i].lines); }
        }
        RL_FREE(s_allDialogues); s_allDialogues = NULL;
    }
    s_dialogueCount = 0; s_dialogueCapacity = 0;
}

void Dialogue_StartById(int dialogueId) {
    for (int i = 0; i < s_dialogueCount; i++) {
        if (s_allDialogues[i].id == dialogueId) {
            s_currentDialogue = &s_allDialogues[i];
            s_currentLineIndex = 0;
            s_visibleCharacters = 0;
            s_typingTimer = 0.0f;
            s_isDialogueActive = true;
            TraceLog(LOG_INFO, "Iniciando dialogo ID: %d", dialogueId);
            return;
        }
    }
    TraceLog(LOG_WARNING, "Dialogue_StartById: Dialogo com ID %d nao encontrado.", dialogueId);
}

void Dialogue_Update(void) {
    if (!s_isDialogueActive || !s_currentDialogue) return;

    const char* currentText = s_currentDialogue->lines[s_currentLineIndex].text;
    int totalChars = (int)strlen(currentText); // *** CORRIGIDO: Cast para int para evitar warning de comparação de sinais ***

    // Efeito de digitação
    if (s_visibleCharacters < totalChars) {
        s_typingTimer += GetFrameTime();
        if (s_typingTimer >= TIME_PER_CHAR) {
            s_typingTimer = 0;
            s_visibleCharacters++;
        }
    }

    // Input do jogador para avançar
    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (s_visibleCharacters < totalChars) {
            // Se o texto ainda está sendo "digitado", revela tudo de uma vez
            s_visibleCharacters = totalChars;
        } else {
            // Se o texto já foi todo revelado, avança para a próxima linha
            s_currentLineIndex++;
            if (s_currentLineIndex >= s_currentDialogue->lineCount) {
                // Fim do diálogo
                s_isDialogueActive = false;
                s_currentDialogue = NULL;
                s_currentLineIndex = 0;
            } else {
                // Prepara para a próxima linha
                s_visibleCharacters = 0;
                s_typingTimer = 0.0f;
            }
        }
    }
}

void Dialogue_Draw(void) {
    if (!s_isDialogueActive || !s_currentDialogue) return;

    DialogueLine* currentLine = &s_currentDialogue->lines[s_currentLineIndex];
    const char* fullText = currentLine->text;

    // Garante que não tentamos mostrar mais caracteres do que existem
    if (s_visibleCharacters > (int)strlen(fullText)) { // *** CORRIGIDO: Cast para int ***
        s_visibleCharacters = (int)strlen(fullText);
    }
    const char* visibleText = TextSubtext(fullText, 0, s_visibleCharacters);

    // *** CORRIGIDO: Casts explícitos para (float) para evitar warnings de narrowing ***
    Rectangle dialogueBoxRect = {
        (float)DIALOGUE_BOX_PADDING,
        (float)(GetScreenHeight() - DIALOGUE_BOX_HEIGHT - DIALOGUE_BOX_PADDING),
        (float)(GetScreenWidth() - (DIALOGUE_BOX_PADDING * 2)),
        (float)DIALOGUE_BOX_HEIGHT
    };

    // Desenha a caixa de diálogo com estilo JRPG
    DrawRectangleRec(dialogueBoxRect, Fade((Color){ 0, 20, 40, 255 }, 0.85f)); // Fundo azul escuro semi-transparente
    // *** CORRIGIDO: Chamada para DrawRectangleRoundedLinesEx com espessura da linha ***
    DrawRectangleRoundedLinesEx(dialogueBoxRect, 0.1f, 8, 3.0f, Fade(SKYBLUE, 0.7f)); // Borda azulada

    // Desenha a caixa do nome do falante (se houver nome)
    if (strlen(currentLine->speaker) > 0) {
        int nameBoxWidth = MeasureText(currentLine->speaker, TEXT_FONT_SIZE) + 20;
        Rectangle nameBoxRect = { dialogueBoxRect.x + 15, dialogueBoxRect.y - (float)TEXT_FONT_SIZE - 10, (float)nameBoxWidth, (float)TEXT_FONT_SIZE + 10 };
        DrawRectangleRec(nameBoxRect, Fade((Color){ 0, 40, 70, 255 }, 0.85f));
        DrawRectangleLinesEx(nameBoxRect, 2, Fade(SKYBLUE, 0.7f));
        DrawText(currentLine->speaker, (int)nameBoxRect.x + 10, (int)nameBoxRect.y + 7, TEXT_FONT_SIZE, WHITE);
    }

    // Desenha o texto da linha atual (com efeito de digitação)
    DrawText(visibleText,
             (int)dialogueBoxRect.x + SPEAKER_TEXT_Y_OFFSET,
             (int)dialogueBoxRect.y + DIALOGUE_TEXT_Y_OFFSET,
             TEXT_FONT_SIZE, WHITE);

    // Desenha o prompt piscante para avançar, se todo o texto já foi revelado
    if (s_visibleCharacters >= (int)strlen(fullText)) { // *** CORRIGIDO: Cast para int ***
        if (((int)(GetTime() * 2.0f)) % 2 == 0) { // Pisca a cada meio segundo
            Vector2 trianglePoints[3] = {
                { dialogueBoxRect.x + dialogueBoxRect.width - 25, dialogueBoxRect.y + dialogueBoxRect.height - 25 },
                { dialogueBoxRect.x + dialogueBoxRect.width - 15, dialogueBoxRect.y + dialogueBoxRect.height - 20 },
                { dialogueBoxRect.x + dialogueBoxRect.width - 25, dialogueBoxRect.y + dialogueBoxRect.height - 15 }
            };
            DrawTriangle(trianglePoints[0], trianglePoints[1], trianglePoints[2], WHITE);
        }
    }
}

bool Dialogue_IsActive(void) {
    return s_isDialogueActive;
}