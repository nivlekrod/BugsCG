# Instruções desenvolvimento do projeto

1. Contexto
Ao longo da disciplina foi desenvolvido, em sala de aula, um jogo no estilo Doom-like, utilizando OpenGL em C/C++ como base para o estudo prático dos principais conceitos de Computação Gráfica.
Este trabalho final tem como objetivo consolidar todos os conteúdos vistos na disciplina por meio do desenvolvimento de um novo jogo autoral, tomando como base o código do jogo construído durante o semestre.
Cada grupo deverá utilizar o projeto-base fornecido (motor e estrutura do Doom-like) como ponto de partida, realizando as adaptações necessárias para criar um jogo próprio, com identidade visual, mecânicas e proposta diferenciadas.

2. Objetivo do trabalho
Desenvolver um jogo 3D interativo utilizando OpenGL, demonstrando, de forma integrada, a aplicação prática dos conceitos de Computação Gráfica estudados na disciplina, por meio da evolução do código-base apresentado em aula.
O trabalho também tem como objetivo estimular a organização de código, a tomada de decisões arquiteturais e o desenvolvimento de um pequeno projeto de jogo completo.
O jogo final não pode ser apenas uma cópia do projeto base. Ele deve apresentar identidade própria (tema, mapa, mecânica, objetivos e estética).

3. Base obrigatória do projeto
O desenvolvimento deve ser realizado obrigatoriamente a partir do código do jogo estilo Doom desenvolvido ao longo do semestre, sendo vedado o uso de engines prontas (Unity, Godot, Unreal, etc.).
É permitido refatorar, reorganizar e estender o código fornecido.
Trabalhos que apenas adaptem superficialmente o projeto (troca de mapa, troca de textura, pequenas mudanças visuais) não serão aceitos.

4. Requisitos gerais do jogo
O jogo desenvolvido deverá:
·    ser executável em ambiente desktop;
·    possuir jogabilidade completa (início, progresso e finalização);
·    possuir identidade própria (tema, ambientação, narrativa ou proposta de jogo);
·    manter a estrutura de renderização em OpenGL utilizada no projeto-base.

5. Requisito obrigatório de fases
O jogo deverá possuir, obrigatoriamente, pelo menos 3 (três) fases jogáveis distintas.
Cada fase deve:
·    possuir seu próprio mapa ou configuração de cenário;
·    permitir jogabilidade completa (início, progresso e finalização);
·    estar integrada ao fluxo do jogo (transição entre fases);
·    apresentar, no mínimo, alguma diferença perceptível em relação às demais, como:
o    layout do cenário,
o    distribuição de inimigos e itens,
o    iluminação ou ambientação,
o    desafios propostos ao jogador.
A progressão entre fases deve ser controlada pelo próprio jogo, por exemplo, ao atingir um objetivo, coletar um item específico ou derrotar um inimigo.

6. Conteúdos de Computação Gráfica que devem estar presentes
O projeto final deve, obrigatoriamente, demonstrar a utilização prática de todos os principais conteúdos trabalhados na disciplina, conforme listados a seguir.
O jogo deverá conter, no mínimo:
Sistema de renderização 3D baseado em raycasting
·    Uso do raycasting para renderização das paredes e do cenário.
·    Correta projeção em tela e cálculo de distâncias.
Renderização de sprites no mundo
·    Renderização correta de entidades (inimigos, itens, objetos, etc.).
·    Ordenação por profundidade (correção de sobreposição).
·    Uso de sprites animados.
Sistema de câmera em primeira ou terceira pessoa
·    Controle de rotação horizontal (yaw).
·    Controle de inclinação vertical (pitch), quando aplicável.
·    Atualização correta da direção de visão.
Iluminação e efeitos visuais
·    Uso de pelo menos um modelo de iluminação visto em aula (ex.: luz direcional, luz ambiente, iluminação interna, etc.).
·    Aplicação coerente no cenário e/ou nos objetos.
·    Uso de shaders, quando aplicável no projeto.
Interface gráfica (HUD)
O jogo deve possuir, no mínimo:
·    HUD com informações do jogador (vida, munição, ou equivalente).
·    Elementos gráficos sobrepostos corretamente ao mundo 3D.
·    Atualização dinâmica conforme o estado do jogo.
Sistema de menus
·    Menu inicial.
·    Menu de pausa.
·    Tela de fim de jogo (game over ou vitória).
·    Uso adequado de projeção 2D para interface.
Estados do jogo
O projeto deve possuir, no mínimo:
·    Estado de menu inicial.
·    Estado jogando.
·    Estado pausado.
·    Estado de fim de jogo.
A lógica do jogo deve estar corretamente separada de acordo com esses estados.
Sistema de áudio
·    Reprodução de som ambiente.
·    Sons de interação (tiro, dano, item, ou equivalente).
·    Atualização da posição do ouvinte conforme a câmera.
Sistema de entidades
·    Estrutura de entidades (inimigos, itens, objetos ou personagens).
·    Atualização, renderização e remoção de entidades.
·    Interação entre jogador e entidades.
Sistema de partículas ou efeitos visuais
·    Implementação de pelo menos um efeito visual dinâmico (ex.: fogo, explosão, fumaça, impacto, etc.).

7. Requisitos de implementação
·    O código deve ser organizado e legível.
·    O projeto deve compilar sem erros.
·    O jogo não pode apresentar falhas que impeçam a jogabilidade básica.
·    A estrutura do projeto deve deixar claro onde cada funcionalidade foi implementada.

8. Entrega
A entrega deverá conter:
·    o código-fonte completo do projeto;
·    instruções de compilação e execução (README);
·    todos os arquivos de recursos utilizados (texturas, sons e shaders).
O projeto deverá ser entregue por meio de repositório (Git ou plataforma indicada pelo professor).

9. Apresentação
Cada grupo deverá apresentar o jogo, demonstrando:
·    as três fases em funcionamento;
·    os principais recursos gráficos utilizados;
·    a aplicação dos conteúdos da disciplina;
·    as principais decisões de implementação adotadas.

10. Critérios de avaliação
O trabalho será avaliado considerando:
·    atendimento aos requisitos técnicos;
·    correta aplicação dos conteúdos de Computação Gráfica;
·    funcionamento geral do jogo;
·    qualidade da organização do código;
·    clareza na apresentação;
·    criatividade e proposta do jogo.

11. Observação final
Este trabalho representa a consolidação prática dos conceitos de Computação Gráfica estudados ao longo do semestre.
Projetos que não utilizarem o código-base desenvolvido em aula ou que não apresentarem os conteúdos mínimos exigidos não serão considerados para avaliação.