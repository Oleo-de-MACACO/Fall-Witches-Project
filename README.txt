Fall Witches

Fall Witches é um jogo de RPG baseado em turnos (clássico RPG de mesa), onde dois heróis, rivais, vindos de dois reinos diferentes, onde humanos e monstros dominam o mundo, se encontram no meio de uma floresta densa, em busca de uma jóia rara que pode mudar o rumo de seus reinados. Porém, uma figura misteriosa invade o caminho e rouba o artefato, sumindo como um vulto, diante de seus olhos.
Agora, mais do que nunca, você e seu rival embarcam nessa grande jornada e precisam aceitar que você e seu rival terão que trabalhar juntos para conseguir recuperar a preciosa jóia que os dois reinos estão tão obcecados.
Bom, isto é apenas o começo. Mas o que será que os dois heróis enfrentarão, durante a jornada?


# --------------------------------------
# PRÉ REQUISITOS E CONFIGURAÇÃO 
# --------------------------------------
Do jogo, vai ter programas com os executáveis já prontos para rodar e jogar, mas, caso desejam pegar e compilar o projeto todo, aqui, sugiro que façam os seguintes passos:

************
**Windows:**
************

 1° passo:
  - Baixe e instale o Raylib, por meio do link "https://www.raylib.com/"

  - Baixe e instale o Code::Blocks, por meio do link "https://sourceforge.net/projects/codeblocks/files/Binaries/25.03/Windows/codeblocks-25.03-setup.exe"

 2° passo:
  - Com o Raylib instalado, pegue o caminho da pasta para "C:\raylib\w64devkit\bin" (sem aspas) e coloque na sua variável de ambiente do sistema, para que o compilador funcione.
  - E com o Code::blocks instalado, faça o seguinte:
     1. Dentro do programa, vá em Settings > Compiler, e depois, vá para "Toolchain executables". Vai aparecer um caminho de onde o programa irá fazer as compilações, e caso esteja em "C:\raylib\w64devkit\bin", não precisa fazer nada. Mas, caso contrário, se não estiver definido esse caminho, vá no botão "..." ao lado e procure pelo caminho acima.
     2. Após isso, abre o projeto no local de onde você baixou e deixou.
     3. Com o projeto aberto, tente compilar e executar. 
     3.1. Caso não esteja compilando com sucesso, verifique se todos os arquivos foram chamados para dentro do programa e tente recompilar o projeto.
     3.2. Caso o problema esteja relacionado ao "raymath.h" ou semelhante, verifique se há algum conflito e veja se o projeto está configurado corretamente em "Build Options > Linker directories", antes de recompilar o projeto.
     3.3. Caso a compilação e a execução do jogo derem certos, é só pegar e jogar.

***********************************
**Linux (para Ubuntu e derivados)**
***********************************

Para a instalação dos programas e a compilação do jogo, deve fazer os seguintes passos:

  1° passo:
    - Use o terminal do seu sistema Linux (pode abrir acessando o menu ou apertando as teclas Ctrl+Alt+T)
    - Digite "sudo apt update && sudo apt upgrade" (para atualizar os repositórios).
    - Depois de atualizado, digite "sudo apt install gcc make cmake git codeblocks libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev (Vai pedir sua senha para proceder a instalação e perguntar se que quer instalar ou não. Aperte Entre para proceder).
    - Após isso, digite os comandos, um de cada vez: 
"git clone https://github.com/raysan5/raylib
cd raylib
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
sudo make install
sudo cp /usr/local/lib/libraylib.so.420 /usr/lib/ 
Depois que conseguir o arquivo, já pode remover o diretório que foi clonado, caso queira liberar espaço de seu PC: cd ../.. && rm -rf raylib
    
  2° passo:
    - Depois de configurado, abra o projeto no Codeblocks e verifique se a configuração para a compilação do jogo está certa.
    - Caso for compilar e executar e o jogo estiver funcionando perfeitamente, perfeito! Você já tem o jogo em mãos e é só pegar e jogar.
    - Porém, se a compilação não der certo, verifique se há arquivos faltando no projeto ou se, quando tiver aberto o projeto na IDE, não conseguir reconhecer os arquivos restantes, ainda no projeto, vá em File > Add files e adicione os arquivos do jogo novamente.
    - Ou, se o problema na compilação for por causa de, por exemplo, "raymath.h", verifique se há conflito entre outros arquivos. Depois, tente recompilar o projeto.


# ---------------------------------------
# JOGABILIDADE
# ---------------------------------------

Os comandos básicos:

Mover: 
  (P1):
    W - para cima
    A - para a esquerda
    S - para baixo
    D - para a direita
  (P2):
    ↑ - para cima
   ← - para a esquerda
    ↓ - para baixo
   → - para a direita

P - Pause
E - Inventário

# Para música
PgUp - Música anterior
PgDn - Música posterior
" + " - Aumentar volume
" - " - Diminuir volume
" ; " - Pausar música
