#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdbool.h>

#define MAX_SPEAKER_NAME_LENGTH 32
#define MAX_DIALOGUE_LINE_LENGTH 256

/**
 * @brief Representa uma única linha de diálogo com um falante e seu texto.
 */
typedef struct {
    char speaker[MAX_SPEAKER_NAME_LENGTH];
    char text[MAX_DIALOGUE_LINE_LENGTH];
} DialogueLine;

/**
 * @brief Representa uma sequência de diálogo completa, identificada por um ID.
 */
typedef struct {
    int id;
    DialogueLine* lines; // Array dinâmico de linhas de diálogo
    int lineCount;
} Dialogue;

// --- Funções do Módulo de Diálogo ---

/**
 * @brief Carrega todos os diálogos de um arquivo de texto para a memória.
 * @param filePath O caminho para o arquivo de diálogos (ex: "Dialogues/dialogues.txt").
 */
void Dialogue_LoadAll(const char* filePath);

/**
 * @brief Libera toda a memória alocada para os diálogos carregados.
 */
void Dialogue_UnloadAll(void);

/**
 * @brief Inicia uma sequência de diálogo com base no seu ID.
 * Torna o sistema de diálogo ativo e prepara a primeira linha para ser exibida.
 * @param dialogueId O ID do diálogo a ser iniciado.
 */
void Dialogue_StartById(int dialogueId);

/**
 * @brief Atualiza o estado do diálogo ativo.
 * Lida com o input do jogador para avançar o texto e as linhas.
 * Esta função deve ser chamada a cada frame se Dialogue_IsActive() for verdadeiro.
 */
void Dialogue_Update(void);

/**
 * @brief Desenha a UI do diálogo na tela com um estilo JRPG.
 * Esta função deve ser chamada a cada frame se Dialogue_IsActive() for verdadeiro.
 */
void Dialogue_Draw(void);

/**
 * @brief Verifica se uma sequência de diálogo está atualmente em andamento.
 * @return true se um diálogo está ativo, false caso contrário.
 */
bool Dialogue_IsActive(void);

#endif // DIALOGUE_H