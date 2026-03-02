# BugsCGame - The Backrooms Escape

## 📖 1. História do Jogo (Lore)
Você é um especialista em cibersegurança que foi digitalizado e arrastado para as profundezas da rede — um labirinto de servidores corrompidos, escuro e claustrofóbico. Uma horda de **Bugs** monstruosos e implacáveis está atacando a internet de dentro para fora, espalhando o caos. O ambiente é um breu quase total, iluminado apenas por algumas lâmpadas de dados que piscam em curto-circuito e pelo brilho incandescente da "Lixeira do Sistema" — um poço de lava digital localizado no centro do mapa.

Seu trabalho é letal e urgente: **matar os bugs e fugir deste pesadelo**. As anomalias patrulham os corredores sombrios prontas para destruir qualquer intruso. Sua única chance de escapar e restaurar a ordem na rede é alcançar a saída: um portal de segurança que atualmente encontra-se inativo. 

Para ativar o portal, você deve explorar a escuridão munido apenas de sua lanterna tática e uma escopeta. Sua missão principal é rastrear e coletar todos os **notebooks** infectados espalhados pelo labirinto e levá-los até o **poço de lava** central para queimá-los, erradicando a raiz da corrupção local. Somente após destruir os notebooks e sobreviver aos ataques dos bugs, o portal se abrirá, permitindo que você avance pelas **3 fases (mapas)** do jogo. Seu objetivo final é chegar ao **Núcleo da Internet**, a área central que está sendo massivamente corrompida pelos bugs, para purificar o sistema de uma vez por todas e finalmente escapar.

---

## 🚀 2. Instruções de Compilação e Execução

O projeto foi desenvolvido em **C++** utilizando **OpenGL** (pipeline fixo + GLSL 1.20) para a renderização, **GLUT** para o sistema de janelas e entrada, e **GLEW** para carregamento das extensões de shaders. Todo o sistema de construção é automatizado via `Makefile`.

### 🐧 Dependências e Compilação no Linux

Certifique-se de ter as seguintes bibliotecas instaladas em seu sistema:

```bash
# Exemplo para distribuições baseadas em Debian/Ubuntu
sudo apt update
sudo apt install g++ make freeglut3-dev libglew-dev libglu1-mesa-dev libopenal-dev
```

**Para compilar e executar o jogo com um único comando:**
```bash
make run
```
*Observação: O comando `make run` compila todos os arquivos para a pasta `build/`, copia todos os `assets`, `maps` e `shaders` garantindo que o executável seja portátil, e em seguida executa o jogo.*

### 🪟 Dependências e Compilação no Windows (MSYS2 / MinGW)

Se estiver usando o MSYS2, instale as dependências com o `pacman`:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-freeglut mingw-w64-x86_64-glew mingw-w64-x86_64-openal
```

**Para compilar e rodar:**
```bash
mingw32-make run
# ou simplesmente "make run"
```

---

## 📚 3. Biblioteca de Ativos (Assets Library)

Os recursos utilizados no jogo estão organizados de maneira a proporcionar uma experiência imersiva e aterrorizante, focada na escuridão e tensão:

- **Texturas Padrão e de Ambiente (`assets/`):** Elementos visuais que constroem o labirinto escuro, como paredes e carpetes sombrios, além de telas de menu, game over e vitória, e gráficos do HUD (vida, arma e coleta de notebooks).
- **Sprites e Inimigos (`assets/enemies/` e `assets/`):** Utiliza-se a técnica de billboarding para as entidades. O jogo possui sprites detalhados da sua arma com animações de recarga, tiro e inatividade, e o sprite crucial do notebook. Os inimigos (Bugs) possuem múltiplas variações e estados de animação (modo patrulha, recebendo dano e modo de fúria/rage).
- **Áudio (`assets/audio/`):** Efeitos sonoros espaciais exportados em formato WAV Mono para o funcionamento correto do OpenAL com áudio 3D. Inclui sons de ambiente tenso, luzes piscando, respiração do jogador, disparos, grunhidos de dor, gritos dos monstros, passos e os efeitos do poço de lava central.
- **Shaders (`shaders/`):** Programas desenvolvidos em GLSL (Vertex e Fragment Shaders) fundamentais para a atmosfera do jogo. Incluem a **lanterna dinâmica** essencial para a exploração na escuridão, animação e brilho do poço de lava, além de efeitos circulares animados para os portais de fase.

---

## 🗂️ 4. Todos os Arquivos de Recursos Utilizados

Abaixo está a lista detalhada de todos os arquivos de recurso incluídos no projeto.

### 🖼️ Texturas e Sprites (Imagens `.png`)
- **Imagens base e interface:** 
  `016.png`, `060.png`, `066.png`, `081_on.png`, `081.png`, `082.png`, `088.png`, `091.png`, `100.png`, `115.png`, `150.png`, `179.png`, `181.png`, `185.png`, `190.png`, `540.png`, `565.png`, `7AgU5CT.png`, `Va4wUMQ.png`, `Y1m2NAI.png`, `damage.png`, `gamezerado.png`, `hd.png`, `heal.png`, `menu_bg.png`, `notebook_wPc.png`, `portal.png`, `telaWin.png`
- **Ambiente Backrooms:** 
  `backrooms-carpet-diffuse.png`, `backrooms-ceiling-light-diffuse.png`, `backrooms-ceiling-light-emission.png`, `backrooms-ceiling-tile-diffuse.png`, `backrooms-wall-diffuse.png`
- **Armas e Mãos:** 
  `gun_default.png`, `gun_fire1.png`, `gun_fire2.png`, `gun_reload1.png`, `gun_reload2.png`, `Shotgun.png`
- **Inimigos (Variações Normais):** 
  `enemy.png`, `enemy(1).png`, `enemy2.png`, `enemy2(1).png`, `enemy3.png`, `enemy3(1).png`, `enemy4.png`, `enemy5.png`
- **Inimigos (Modo Fúria/Rage):** 
  `enemyRage.png`, `enemyRage(1).png`, `enemyRage2.png`, `enemyRage2(1).png`, `enemyRage3.png`, `enemyRage3(1).png`, `enemyRage4.png`, `enemyRage5.png`
- **Inimigos (Recebendo Dano / Fúria):** 
  `enemyRageDamage.png`, `enemyRageDamage(1).png`, `enemyRageDamage2.png`, `enemyRageDamage2(1).png`, `enemyRageDamage3.png`, `enemyRageDamage3(1).png`, `enemyRageDamage4.png`, `enemyRageDamage5.png`

### 🔊 Arquivos de Áudio (Sons `.wav`)
- `ambient_mono.wav`
- `breath_mono.wav`
- `click_reload_mono.wav`
- `collect_mono.wav`
- `enemy_mono.wav`
- `enemy_scream_mono.wav`
- `fire.wav`
- `grunt_mono.wav`
- `hurt_mono.wav`
- `kill_mono.wav`
- `lava_mono.wav`
- `light_flicker.wav`
- `music.wav`
- `reload_mono.wav`
- `shot_mono.wav`
- `step_mono.wav`

### ✨ Arquivos de Shaders (`.vert` e `.frag`)
- `blood.frag`, `blood.vert` *(Efeito de distorção de paredes de sangue)*
- `fire.frag`, `fire.vert` *(Efeito de paredes incandescentes/fogo)*
- `flashlight.frag`, `flashlight.vert` *(Iluminação dinâmica da lanterna do jogador no escuro)*
- `lava.frag`, `lava.vert` *(Animação e iluminação do poço de lava central)*
- `melt.frag`, `melt.vert` *(Efeito de derretimento do ambiente)*
- `portal.frag`, `portal.vert` *(Efeito especial pulsante do portal de saída)*
