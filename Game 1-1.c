#include<stdio.h>
#include<stdlib.h>
#include<conio2.h>
#include<time.h>
#include<windows.h>
#define JUMPRANGE 6
#define EXTJUMPRANGE 3
#define SLPTIME 80
#define SLPSPACE 120
#define MAXPLATNUM 8
#define MAXSPEED 3
#define MESSAGETIME 1
// TODO (Artur#1#): Trocar como os caracteres especiais sao printados: trocar o código ascii pelo caractere em si.
// TODO (Artur#1#): Implementar a funcionalidade de mostrar mensagens na ultima linha. ...
//Sugestão: usar um campo de mensagem na estrutura PLAYER


typedef struct
{
    int x;
    int y;
} COORDENATE; //define um tipo para coordenadas

typedef struct
{
    int color; //cor da plataforma
    int direction; // 0...2 para cada uma das direções
    int size; // número de caracteres que ocupa
    int speed; //1...3, implementada pela função move_plat
    COORDENATE position; //coordenada da plaaforma na tela
} PLATFORM; //define um tipo para as plataformas

typedef struct
{
    char name[51]; //nome do jogador
    int ch; // ASCII do caracter que representa o jogador
    int color; // cor que o caracter vai ser plotado
    int lives; //assume valor -1 seo jogador está morto ou 0 se ele está vivo
    int jump; //representa quantas casas ainda devem ser puladas
    int extra_jump; // vale 1 se o jogador puder usar o pulo extra ou 0 caso ele não possa
    int points; //contabiliza o total de pontos do jogador
    int died;   //vale 1 se o jogador tiver morrido no último movimento
    char messages[50]; //string que armazenara as mensagens mostradas ao jogador
    int m_shown;    //vale 0, ou 1 caso a mensagem ja tiver sido mostrada nos últimos 3 segundos
    COORDENATE position; //coordenada do jogador na tela
} PLAYER; //define um tipo para o jogador

int read_arrows(int input) //le uma tecla do teclado e devolve o codigo da direção, ou 4 no caso da barra de espaço
{
    /*A função getch() devolve -32 ou 224 para QUALQUER seta, nesse caso o segundo getch() devolverá um código ASCII diferente,
     o que possibilita a diferenciação entre as setas*/
    input=getch();
    if(input == -32 || input == 224)
        input = getch();
    switch(input)
    {
    case 80:
        return(3); //DOWN
    case 32:
        return(4); //SPACE BAR)
    case 75:
        return(2); //LEFT
    case 77:
        return(1); //RIGHT
    default:
        return(-1);
    }
}

int check_position(COORDENATE coord) //retorna true se a coordenada de entrada não está dentro da interface
{
    if(coord.x<=1 || coord.x>=80 || coord.y<=1 || coord.y>=25) //   está errado mas da maneira correta nao funciona também
        return(1);
    else
        return(0);
}

void draw_plat(PLATFORM plat) //desenha a plataforma de acordo com a estrutura de entrada
{
    textbackground(plat.color);
    int i, x = plat.position.x, y = plat.position.y, tam = plat.size;
    for(i=0; i<tam; i++)
    {
        if(x+i<81 && x+i>=1)
            putchxy( x+i, y, ' ');
    }
    textbackground(0);
}

void draw_platforms(PLATFORM plat[], int n) //desenha multiplas ras de acordo com o arranjo de entrada
{
    int i;
    for(i=0; i<n; i++)
    {
        draw_plat(plat[i]);
    }
}

void erase_plat(PLATFORM plat) //apaga a plataforma de entrada e posiciona o cursor uma posição a frente dela
{
    int i, x = plat.position.x, y = plat.position.y;
    gotoxy(x, y);   //posiciona o cursor na primeira casa da plataforma
    for(i=0; i < plat.size ; i++)
    {
        if(x+i<81 && x+i>=1)
            putchxy( x+i, y, ' ');//sobreescreve todas as casas da plataforma com espaços em branco
    }
}

int get_platcenter(PLATFORM plat) //devolve metade do tamanho da plataforma, arredondado para baixo caso este seja ímpar
{
    int platcenter;
    if(plat.size%2==0)
        platcenter = plat.size/2 - 1;
    else
        platcenter = plat.size/2;
    return(platcenter);
}

void start_plat(PLATFORM *plat)
{
    plat->color = 6;
    plat->size = rand() % 15 + 3;
    plat->speed = rand() % MAXSPEED;
}

void start_platforms(PLATFORM platforms[], int n )
{
    int i;
    for(i=0; i<n; i++)
        start_plat(&platforms[i]);
}

void generate_plat(PLATFORM *plat) //gera uma plataforma aleatória de acordo com os padrões de aparecimento exigidos
{
    int gen;
    gen = rand() % 10 + 1;
    if(gen<=2)    // 20% aparecem na parte de baixo da telaem uma coluna aleatória e vão para cima.
    {
        plat->direction = 0;
        plat->position.x = rand() % 80 + 1;
        plat->position.y = rand() % 10 + 14;
    }
    if(gen>2 && gen<=6)    // 40% aparecem no lado direito da tela em uma linha aleatória e vão para a esquerda.
    {
        plat->direction = 2;
        plat->position.x = rand() % 40 + 39;
        plat->position.y = rand() % 22 + 3;
    }
    if(gen>6)    //// 40%(as demais) aparecem no lado esquerdo da tela em uma linha aleatória e vão para a direita.
    {
        plat->direction = 1;
        plat->position.x = rand() % 40;
        plat->position.y = rand() % 22 + 3;
    }
}

void generate_platforms(PLATFORM platforms[], int n )
{
    int i;
    for(i=0; i<n; i++)
        generate_plat(&platforms[i]);
}

void restart_plat(PLATFORM *plat) //reinicializa uma plataforma, conservando seu tamanho, cor e velocidade.
{
    COORDENATE plat_rightside = plat->position;
    plat_rightside.x += plat->size;
    if(check_position(plat->position) && check_position(plat_rightside))
        generate_plat(plat);
}

PLATFORM move_plat(PLATFORM plat) //retorna a plataforma deslocada uma casa na direção especificada em sua estrutura
{
    int i;
    PLATFORM temp=plat; //variavel local que é uma cópia de plat, será alterada de acordo com a direção de movimento
    erase_plat(temp); //apaga a plataforma
    switch(temp.direction) //muda a coordenada da plataforma(na variável local temp) de acordo com a direção de movimento
    {
    case 0:
        temp.position.y--;  //UP
        break;
    case 2:
        temp.position.x--;  //LEFT
        break;
    case 1:
        temp.position.x++;  //RIGHT
        break;
    default:
        break;
    }
    restart_plat(&temp);
    draw_plat(temp); //desenha a plataforma na nova posição
    return(temp);
}

void move_platforms (PLATFORM platforms[], int n, int cont) //move n plataformas j casas na direção indicada na estrutura, j depende da velocidade da plataforma
{
    int i, j;
    for(i=0; i<n; i++)
        if(cont % MAXSPEED <= platforms[i].speed)
            platforms[i]=move_plat(platforms[i]);
}

void draw_player(PLAYER player) //desenha o jogador de acordo com a estrutura de entrada
{
    textcolor(player.color);    //muda a cor do texto para a cor do jogador
    putchxy(player.position.x, player.position.y, player.ch);   //imprime o caractere do jogador na coordenada de sua estrutura
    textcolor(15);  //troca a cor do texto de volta para branco
}

PLAYER move_player(PLAYER player, int direction)    //retorna o jogador deslocado uma casa na direção de entrada
{
    PLAYER temp=player; //variavel local que é uma cópia de player, será alterada de acordo com a direção de movimento
    erase_player(temp); //apaga o caractere do jogador
    switch(direction)   //altera temp de acordo com a direção do movimento
    {
    case 0:
        temp.position.y--; //UP
        break;
    case 3:
        temp.position.y++; //DOWN
        break;
    case 2:
        temp.position.x--; //LEFT
        break;
    case 1:
        temp.position.x++; //RIGHT
        break;
    default:
        break;
    }
    if(check_position(player.position))//verifica se o jogador está em uma posição inváida
    {
        temp.lives--;  //muda o valor de temp.alive para indicar que o jogador morreu
        temp.died = 1;
    }
    else
    {
        draw_player(temp);  //como continua vivo, o jogador deve ser desenhado em sua nova posição
    }
    return(temp);
}

int erase_player(PLAYER player)    //apaga o jogador e posiciona o cursor uma posição a frente dele
{
    gotoxy(player.position.x, player.position.y);
    printf(" ");
}

void restart_player(PLAYER *player)   //reinicializa o jogador no centro de uma plataforma e indica para o programa que ele está vivo e que não está pulando
{
    if(player->died)
    {
        player->position.x = 40;
        player->position.y = 13;
        draw_player(*player);
        player->m_shown = 0;
        strcpy(player->messages, "You died! Lost a life...\t\t\t\t\t\t       ");
    }
}

void start_player(PLAYER *player, PLATFORM platforms[], int n)   //inicializa o jogador no centro de uma plataforma e indica para o programa que ele está vivo e que não está pulando
{
    player->position.x = platforms[n].position.x + get_platcenter(platforms[n]); //altera a coordenada x do jogador para o centro da plataforma
    player->position.y = platforms[n].position.y - 1;   //altera a coordenada y do jogador para uma linha acima da plataforma
    player->ch = 2;
    player->lives = 3;    //indica para o programa que o jogador tem 3 vidas
    player->jump = 0;    //indica para o programa que o jogador não tem casas para pular(não está pulando)
    player->color = 15;
    player->died = 0;
    player->points = 0;
    player->m_shown = 0;
    strcpy(player->messages, "message");
    draw_player(*player);   //desenha o jogador na sua nova posição
}

PLAYER jump_player(PLAYER player)   //retorna o jogador delocado uma casa para cima, caso ele esteja pulando(ainda), do contrário retorna o mesmo jogador de entrada
{
    PLAYER temp=player; //variável local que é uma cópia de player, será alterada se o jogador estiver pulando
    int para_cima=0;    //constante de valor correspondente a direção para cima, presente apenas para clareza do programa
    if(temp.jump>0) //true se o jogador ainda tiver casas para pular(estiver pulando)
    {
        temp.jump--;    //indica para o programa que o jogador percorreu uma casa em seu pulo, ou seja, que ele tem menos uma casa para pular
        temp=move_player(temp, para_cima);  //desloca o jogador uma casa para cima
    }
    return(temp);
}

int on_plat(PLAYER player, PLATFORM plat)  //retorna true se o jogador está em cima de uma plataforma
{
    int yplat = plat.position.y;
    int xplat = plat.position.x;
    int yplayer = player.position.y;
    int xplayer = player.position.x;
    if(yplayer==yplat-1 && xplayer>=xplat && xplat+plat.size>xplayer)
        return(1);
    else
        return(0);
}

int on_platforms(PLAYER player, PLATFORM platforms[], int n) //retorna o indice da plataforma na qual o jogador está em cima, caso ele nao esteja em cima de nenhuma plataforma retorna -1
{
    int i=0, falling=-1;
    while(!on_plat(player, platforms[i])&& falling) //o laço continua até encontrar a plataforma na qual o jogador está em cima ou até chegar no final do vetor plataforms
    {
        if(i==n+1)
        {
            i=falling;
            falling=0;
        }
        else
            i++;
    }
    return(i);
}

PLAYER fall_player(PLAYER player, PLATFORM platforms[], int n)   //retorna o jogador deslocado uma casa para baixo caso ele não esteja nem cima de uma plataforma nem pulando
{
    int para_baixo=3;   //constante de valor correspondente a direção para baixo, presente apenas para a clareza do programa
    PLAYER temp=player; // //variável local que é uma cópia de player, será alterada se o jogador não estiver em uma plataforma
    if(on_platforms(temp, platforms, n)==-1 && temp.jump<=0 && !temp.died)    //verifica se o jogador não está nem cima de uma plataforma nem pulando
    {
        temp=move_player(temp, para_baixo); //desloca o jogador uma casa para baixo
    }
    return(temp);
}

void show_message(PLAYER *player, clock_t *start, clock_t end)
{
    double tempo = ((double) (end - *start)) / CLOCKS_PER_SEC;
    if(strcmp(player->messages, "message") && (int)tempo < MESSAGETIME)
    {
        gotoxy(2, 25);
        textbackground(RED);
        printf(" %s", player->messages);
        if(!player->m_shown)
            *start = clock();
        player->m_shown = 1;
    }
    else
    {
        strcpy(player->messages, "message");
        if(!player->m_shown)
            *start = end;
    }
    textbackground(BLACK);
}

int draw_interface(PLAYER player) // remover as barras e apagar a linha 386 muda o padrao de movimento do fogo
{
    int i, k=0;

    gotoxy(2, 25);
    if(!strcmp(player.messages, "message"))
        for(i=1; i<79; i++)
        {
            textbackground(BLACK);
            textcolor((rand() % 2)*2 + 12);
            putchar(30);
        }
    textcolor(15);

    gotoxy(1,1);
    textbackground(RED);
    printf("\tKeep the face from the FIRE!\tLives: ");
    for(i=0; i<player.lives; i++)
        putchar(3);
    printf("\tPoints: %d\tPower:  ", player.points);
    textbackground(0);
}

int gameloop(PLAYER *player, PLATFORM platforms[], int n)   //laço do jogo
{
    int the_plat, input, cont=0;
    double tempo=0;
    clock_t start, end, m_start;
    while(player->lives>0)
    {
        start = clock();
        Sleep(SLPTIME);
        restart_player(player);
        show_message(player, &m_start, end);
        *player=fall_player(*player, platforms, n);
        *player=jump_player(*player);
        if(kbhit())
        {
            if(player->died)
            {
                player->extra_jump = 1;
                player->died = 0;
            }
            input=read_arrows(input);
            if(input==4)
            {
                if(on_plat(*player, platforms[the_plat]))
                    player->jump=JUMPRANGE;
                else if(player->extra_jump == 1)
                {
                    player->jump=EXTJUMPRANGE;
                    player->extra_jump = 0;
                }
            }
            if(input != 4)
                *player=move_player(*player, input);
        }
        the_plat=on_platforms(*player, platforms, n); //retorna -1 caso o jogador nao esteja em cima de nenhuma plataforma ou o indice da plataforma na qual ele está em cima
        if(the_plat!=-1 && (cont % MAXSPEED <= platforms[the_plat].speed) && player->died != 1)
        {
            *player=move_player(*player, platforms[the_plat].direction);
            player->extra_jump = 1;
        }
        end = clock();
        if(!player->died)
        {
            tempo += ((double) (end - start)) / CLOCKS_PER_SEC;
            player->points = (int) tempo;
        }
        move_platforms(platforms, n, cont);
        cont++;
        draw_interface(*player);
    }
}

int get_platforms(PLATFORM platforms[], int *n)
{
    int i;
    FILE *arq;

    if((arq = fopen("plataformas.txt", "r")) == 0)
    {
        printf("Erro na abertura do arquivo de plataformas!\n");
        return(0);
    }

    fscanf(arq, "%d\n", n);

    for(i=0; i<*n; i++)
    {
        fscanf(arq, "%d %d\n", &platforms[i].size, &platforms[i].speed);
    }

    fclose(arq);

    return(1);
}

int main()
{
    srand(time(NULL));

    int n, i, k=MAXPLATNUM;
    FILE *arq;
    PLAYER player;
    PLATFORM plat[k];

    start_platforms(plat, k);
    generate_platforms(plat, k);
    draw_platforms(plat, k);

    start_player(&player, plat, 0);

    getch();

    gameloop(&player, plat, k);

    return(0);
}
