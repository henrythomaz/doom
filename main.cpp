#include <GL/glut.h>
#include <cstdio>
#include <cmath>


#define res 1
#define SW 160*res
#define SH 120*res
#define SW2 (SW/2)
#define SH2 (SH/2)
#define pixelScale 4/res
#define GLSW (SW*pixelScale)
#define GLSH (SH*pixelScale)

//texturas
#include "textures/T_NUMBERS.h"
#include "textures/T_VIEW2D.h"
#include "textures/T_00.h"
#include "textures/T_01.h"
#include "textures/T_02.h"
#include "textures/T_03.h"
#include "textures/T_04.h"
#include "textures/T_05.h"
#include "textures/T_06.h"
#include "textures/T_07.h"
#include "textures/T_08.h"
#include "textures/T_09.h"
#include "textures/T_10.h"
#include "textures/T_11.h"
#include "textures/T_12.h"
#include "textures/T_13.h"
#include "textures/T_14.h"
#include "textures/T_15.h"
#include "textures/T_16.h"
#include "textures/T_17.h"
#include "textures/T_18.h"
#include "textures/T_19.h"
int numText = 19;
int numSect = 0;
int numWall = 0;

typedef struct {
  int fr1, fr2;
} time; time T;

typedef struct {
  int w, s, a, d;
  int sl, sr;
  int m;
} keys; keys K;

typedef struct {
  float cos[360];
  float sin[360];
} match; match M;

typedef struct {
  int x, y, z; // Posiçãod do jogador
  int a; // Angulo de rotação do jogador (esquerda/direita)
  int l; // Variável de olhar para cima e para baixo
} player; player P;

typedef struct {
  int x1, y1; // linha de baixo ponto 1
  int x2, y2; // linhas de cima ponto 2
  int c; // cor da parede
  int wt, u, v; // textura da parede e telha u/v 
  int shade; //sombra da parede.
} walls; walls W[256];

typedef struct {
  int ws, we; // Numbero de começo e fim da parede
  int z1, z2; // Altura do fundo e do topo
  // int x, y; // Posição central para o setor
  int d; // Adiciona uma distancia y para ordenar a direção
  int c1, c2; // Cor de fundo e de topo
  int st, ss; // superficie da textura, escala da superficie
  int surf[SW]; // Pontos de espera para superficies
  int surface; // é a superficie
} sectors; sectors S[128];

typedef struct {
  int w, h; // tamanho e altura da textura
  const unsigned char *name; // nome da textura
} TextureMaps; TextureMaps Textures[64]; // aumenta para mais texturas
                                            //
void load()
{
 FILE *fp = fopen("level.h","r");
 if(fp == NULL){ printf("Error opening level.h"); return;}
 int s,w;

 fscanf(fp,"%i",&numSect);   //numero de setores 
 for(s=0;s<numSect;s++)      //carrega todos os setores
 {
  fscanf(fp,"%i",&S[s].ws);  
  fscanf(fp,"%i",&S[s].we); 
  fscanf(fp,"%i",&S[s].z1);  
  fscanf(fp,"%i",&S[s].z2); 
  fscanf(fp,"%i",&S[s].st); 
  fscanf(fp,"%i",&S[s].ss);  
 }
 fscanf(fp,"%i",&numWall);   //numero de paredes
 for(s=0;s<numWall;s++)      //carrega todas as paredes
 {
  fscanf(fp,"%i",&W[s].x1);  
  fscanf(fp,"%i",&W[s].y1); 
  fscanf(fp,"%i",&W[s].x2);  
  fscanf(fp,"%i",&W[s].y2); 
  fscanf(fp,"%i",&W[s].wt);
  fscanf(fp,"%i",&W[s].u); 
  fscanf(fp,"%i",&W[s].v);  
  fscanf(fp,"%i",&W[s].shade);  
 }
 fscanf(fp,"%i %i %i %i %i",&P.x,&P.y,&P.z, &P.a,&P.l); //Posição do jogador, angulo, direção do olhar 
 fclose(fp); 
}

void drawPixel(int x, int y, int r, int g, int b) { // Desenha um pixel em x/y com rgb
  
  if (x < 0 || x >= SW || y < 0 || y >= SH) {
    return;
  }
  glColor3ub (r, g, b);
  glBegin(GL_POINTS);
  glVertex2i (x*pixelScale+2, y*pixelScale+2);
  glEnd();
}

void movePlayer() {
  // move pra cima, pra baixo, pra esquerda e para a direita
  if (K.a==1 && K.m==0) {
    P.a -= 4; 
    if (P.a < 0) 
      P.a += 360;
  }
  if (K.d==1 && K.m==0) {
    P.a += 4; 
    if (P.a > 359) 
      P.a -= 360;
  }

  int dx = M.sin[P.a]*10.0;
  int dy = M.cos[P.a]*10.0;

  if (K.w==1 && K.m==0) {
    P.x += dx; P.y  += dy;
  }
  if (K.s==1 && K.m==0) {
    P.x -= dx; P.y  -= dy;
  }

  // metralhar para a esquerda e para a direita
  if (K.sr==1) {
    P.x += dy; P.y  -= dx;
  }
  if (K.sl==1) {
    P.x -= dy; P.y  += dx;
  }


  // Mover para cima e para baixo, olhar para cima e para baixo
  if (K.a==1 && K.m==1) {
    P.l -= 1;
  }
  if (K.d==1 && K.m==1) {
    P.l += 1;
  }
  if (K.w==1 && K.m==1) {
    P.z-=4;
  }
  if (K.s==1 && K.m==1) {
    P.z+=4;
  }
}

void clearBackground() {
  // Céu (parte superior) e chão (parte inferior)
  for (int y = 0; y < SH; y++) {
    for (int x = 0; x < SW; x++) {
        drawPixel(x, y, 0, 60, 130);
    }
  }
}

void clipBehindPlayer(float *x1, float *y1, float *z1,
                      float x2, float y2, float z2)
{
    float da = *y1;
    float db = y2;

    if (fabs(db - da) < 0.001f) return;

    float t = (1.0f - da) / (db - da);

    *x1 = *x1 + t * (x2 - *x1);
    *y1 = 1.0f;
    *z1 = *z1 + t * (z2 - *z1);
}

void drawWall(int x1, int x2, int b1, int b2, int t1, int t2, int s, int w, int frontBack) {
  int x, y;

  // Textura da parede
  int wt = W[w].wt;

  // Textura horizontal da parede iniciando e valor do passo
  if(x1 > x2)
  {
    int t=x1;
    x1=x2;
    x2=t;

    t=b1;
    b1=b2;
    b2=t;

    t=t1;
    t1=t2;
    t2=t;
  }

  float ht = 0, ht_step = (float)Textures[wt].w/(float)(x2-x1);


  // mantem a direfença na distancia
  int dyb = b2 - b1; // distancia y do linha de baixo
  int dyt = t2 - t1; // distancia y da linha de cima
  int dx = x2 - x1; if (dx == 0) { dx=1; }; // distancia x
  int xs = x1; // mantem x1 inicial 

  // Recorta o x 
  if (x1 < 0) 
    x1 = 0; // corta esquerda
  if (x2 < 0) 
    x2 = 0; // corta esquerda
  if (x1 > SW) 
    x1 = SW; // corta direita
  if (x2 > SW) 
    x2 = SW; // corta direita

  // desenha x linhas verticais
  for (x = x1; x < x2; x++) {
    // O Y inicia e termina
    int y1 = dyb*(x - xs + 0.5)/dx+b1; // ponto y inferior
    int y2 = dyt*(x - xs + 0.5)/dx+t1; // ponto y inferior

    // Textura da parede vertical iniciando e valor do passo
    float vt = 0, vt_step=(float)Textures[wt].h / (float)(y2 - y1);
                                       
    // Recorta y
    if (y1 < 0)
      y1 = 0; // baixo
    if (y2 < 0)
      y2 = 0; // baixo
    if (y1 > SH)
      y1 = SH; // cima
    if (y2 > SH)
      y2 = SH; // cima
    
    // Superficie 

    // desenha frente da parede
    if (frontBack==0) {
      if (S[s].surface==1){S[s].surf[x]=y1;} // supeficie inferior salva a linha superior
      if (S[s].surface==2){S[s].surf[x]=y2;} // supercie superior salva a linha superior
      for (y = y1; y < y2; y++) {
        int pixel = ((int)(Textures[wt].h - vt - 1) * Textures[wt].w + (int)ht)*3;
        int r = Textures[wt].name[pixel+0];
        int g = Textures[wt].name[pixel+1];
        int b = Textures[wt].name[pixel+2];
        drawPixel(x,y,r,g,b);
        vt+=vt_step;
      }
      ht+=ht_step;
    }

    // desenha a tras da parede
    if (frontBack==1) {
      if (S[s].surface==1){y2=S[s].surf[x];}
      if (S[s].surface==2){y1=S[s].surf[x];}

    if(y1 > y2)
    {
        int t = y1;
        y1 = y2;
        y2 = t;
    }

      for (y = y1; y < y2; y++) {
        drawPixel(x, y, 255, 0, 0); // superficie
      }
    }

    // pixel(x, y1, 0); // baixo
    // pixel(x, y2, 0); // cima

  }
}

int dist(int x1, int y1, int x2, int y2) {
  int distance = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
  return distance;
}


void draw3D() {
    int s, w, frontBack, cycles, x;
    float CS = M.cos[P.a], SN = M.sin[P.a];
    
    // ============================================
// PASSO 1: Calcular distância de cada setor (usando a parede mais próxima)
// ============================================
for (s = 0; s < numSect; s++) {
    // Inicializa a distância mínima com um valor alto
    int sumDist = 0;
    int nWalls = 0;

    // Configura superfície (chão/teto) igual antes, mas sem interferir na distância
    if (P.z < S[s].z1) {
        S[s].surface = 1;
        cycles = 2;
        for (x = 0; x < SW; x++) { S[s].surf[x] = SH; }
    } else if (P.z > S[s].z2) {
        S[s].surface = 2;
        cycles = 2;
        for (x = 0; x < SW; x++) { S[s].surf[x] = 0; }
    } else {
        S[s].surface = 0;
        cycles = 1;
    }

    // Calcula distância apenas das paredes verticais (sem inverter ordem)
    for (w = S[s].ws; w < S[s].we; w++) {
        int x1 = W[w].x1 - P.x, y1 = W[w].y1 - P.y;
        int x2 = W[w].x2 - P.x, y2 = W[w].y2 - P.y;

        // Rotaciona para o espaço da câmera
        int wx1 = x1 * CS - y1 * SN;
        int wy1 = y1 * CS + x1 * SN;
        int wx2 = x2 * CS - y2 * SN;
        int wy2 = y2 * CS + x2 * SN;

        // Se ambos os pontos estão atrás do plano próximo, ignora
        if (wy1 < 1 && wy2 < 1) continue;

        // Aplica clipping para o cálculo da distância
        float cwx1 = wx1, cwy1 = wy1, cwx2 = wx2, cwy2 = wy2;
        float dummy;
        if (cwy1 < 1) clipBehindPlayer(&cwx1, &cwy1, &dummy, cwx2, cwy2, 0);
        if (cwy2 < 1) clipBehindPlayer(&cwx2, &cwy2, &dummy, cwx1, cwy1, 0);

        // Distância do ponto médio da parede até o jogador
        int midX = (cwx1 + cwx2) / 2;
        int midY = (cwy1 + cwy2) / 2;
        int dist = (int)sqrt(midX*midX + midY*midY);

        sumDist += dist;
        nWalls++;
    }

    if (nWalls > 0) {
        S[s].d = sumDist / nWalls;      // usa a parede mais próxima como referência
    } else {
        S[s].d = 999999;       // setor invisível
    }
}
    
    // ============================================
    // PASSO 2: Ordenar setores por distancia (mais longe primeiro)
    // ============================================
    for (s = 0; s < numSect - 1; s++) {
        for (w = 0; w < numSect - s - 1; w++) {
            if (S[w].d < S[w+1].d) {
                sectors st = S[w];
                S[w] = S[w+1];
                S[w+1] = st;
            }
        }
    }
    
    // ============================================
    // PASSO 3: Desenhar setores na ordem (longe → perto)
    // ============================================
    for (s = 0; s < numSect; s++) {
        // Pula setores muito distantes/invisiveis
        if (S[s].d > 8000) continue;
        
        // Determina ciclos baseado na superficie
        if (P.z < S[s].z1) {
            cycles = 2;
        } else if (P.z > S[s].z2) {
            cycles = 2;
        } else {
            cycles = 1;
        }
        
        // Desenha frente e tras (se aplicavel)
        for (int pass = 0; pass < cycles; pass++) {
            frontBack = (pass == 0) ? 0 : 1;
            
            for (w = S[s].ws; w < S[s].we; w++) {
                // Pontos da parede relativos ao jogador
                int x1 = W[w].x1 - P.x, y1 = W[w].y1 - P.y;
                int x2 = W[w].x2 - P.x, y2 = W[w].y2 - P.y;
                
                // Inverte ordem para desenhar o verso (superficie)
                if (frontBack == 1) {
                    int swp = x1; x1 = x2; x2 = swp;
                    swp = y1; y1 = y2; y2 = swp;
                }
                
                // Rotaciona para espaco da camera
float wx[4], wy[4], wz[4];
wx[0] = x1*CS - y1*SN;
wx[1] = x2*CS - y2*SN;
wx[2] = wx[0];   // copia x do chão para o teto
wx[3] = wx[1];

wy[0] = y1*CS + x1*SN;
wy[1] = y2*CS + x2*SN;
wy[2] = wy[0];   // copia profundidade do chão para o teto
wy[3] = wy[1];                

                // Altura Z (piso e teto)
                wz[0] = S[s].z1 - P.z + ((P.l * wy[0]) / 32.0);
                wz[1] = S[s].z1 - P.z + ((P.l * wy[1]) / 32.0);
                wz[2] = S[s].z2 - P.z + ((P.l * wy[0]) / 32.0);
                wz[3] = S[s].z2 - P.z + ((P.l * wy[1]) / 32.0);
                
                // Se os dois pontos estao atras, nao desenha
                if (wy[0] < 1 && wy[1] < 1)
                    continue;
                
                // Clipping para o ponto 1 (se atras do jogador)
                if (wy[0] < 1) {
                    clipBehindPlayer(&wx[0], &wy[0], &wz[0], wx[1], wy[1], wz[1]);
                    clipBehindPlayer(&wx[2], &wy[2], &wz[2], wx[3], wy[3], wz[3]);
                }
                
                // Clipping para o ponto 2 (se atras do jogador)
                if (wy[1] < 1) {
                    clipBehindPlayer(&wx[1], &wy[1], &wz[1], wx[0], wy[0], wz[0]);
                    clipBehindPlayer(&wx[3], &wy[3], &wz[3], wx[2], wy[2], wz[2]);
                }
                
                // Projeta para coordenadas de tela
                int scrX1 = (int)(wx[0] * 200.0f / wy[0] + SW2);
int scrY1 = (int)(wz[0] * 200.0f / wy[0] + SH2);

int scrX2 = (int)(wx[1] * 200.0f / wy[1] + SW2);
int scrY2 = (int)(wz[1] * 200.0f / wy[1] + SH2);

int scrX3 = (int)(wx[2] * 200.0f / wy[2] + SW2);
int scrY3 = (int)(wz[2] * 200.0f / wy[2] + SH2);

int scrX4 = (int)(wx[3] * 200.0f / wy[3] + SW2);
int scrY4 = (int)(wz[3] * 200.0f / wy[3] + SH2);                
                // Desenha a parede
                drawWall(scrX1, scrX2, scrY1, scrY2, scrY3, scrY4, s, w, frontBack);
            }
        }
    }
}

void testTextures() {
  int x, y, t;

  t = 4;

  for(y=0; y < Textures[t].h; y++) {
    for (x=0; x < Textures[t].w; x++) {
      int pixel = (Textures[t].h - y - 1)*3 * Textures[t].w + x*3;
      int r = Textures[t].name[pixel+0];
      int g = Textures[t].name[pixel+1];
      int b = Textures[t].name[pixel+2];
      drawPixel(x,y,r,g,b);
    }
  }
}

void display() {
  int x, y;

  if (T.fr1-T.fr2>=50) {
    clearBackground();
    movePlayer();
    draw3D();

    T.fr2 = T.fr1;
    glutSwapBuffers();
    glutReshapeWindow(GLSW, GLSH);
  }

  T.fr1=glutGet(GLUT_ELAPSED_TIME);
  glutPostRedisplay();
}

void KeysDown(unsigned char key, int x, int y) {
  if (key=='w') 
    K.w = 1;
  if (key=='s')
    K.s = 1;
  if (key=='a')
    K.a = 1;
  if (key=='d') 
    K.d = 1;
  if (key=='m')
    K.m = 1;
  if (key==',')
    K.sr = 1;
  if (key=='.')
    K.sl = 1;
  if (key==13)
    load(); // teclado enter carrega nível
}

void KeysUp(unsigned char key, int x, int y) {
  if (key=='w') 
    K.w = 0;
  if (key=='s')
    K.s = 0;
  if (key=='a')
    K.a = 0;
  if (key=='d') 
    K.d = 0;
  if (key=='m')
    K.m = 0;
  if (key==',')
    K.sr = 0;
  if (key=='.')
    K.sl = 0;
}

void init() {
  int x;

  // converte de radiano para graus (dividindo por 180 e multiplicando por pi)
  for (x=0; x<360; x++) {
    M.cos[x] = cos(x/180.0*M_PI);
    M.sin[x] = sin(x/180.0*M_PI);
  }

  // Inicializa o Jogador
  P.x = 70; P.y = -110; P.z = 20; P.a = 0; P.l = 0; // Variáveis iniciais do jogador
  
  // Define as texturas
  Textures[ 0].name=T_00; Textures[ 0].h=T_00_HEIGHT; Textures[ 0].w=T_00_WIDTH; 
  Textures[ 1].name=T_01; Textures[ 1].h=T_01_HEIGHT; Textures[ 1].w=T_01_WIDTH;
  Textures[ 2].name=T_02; Textures[ 2].h=T_02_HEIGHT; Textures[ 2].w=T_02_WIDTH;
  Textures[ 3].name=T_03; Textures[ 3].h=T_03_HEIGHT; Textures[ 3].w=T_03_WIDTH;
  Textures[ 4].name=T_04; Textures[ 4].h=T_04_HEIGHT; Textures[ 4].w=T_04_WIDTH;
  Textures[ 5].name=T_05; Textures[ 5].h=T_05_HEIGHT; Textures[ 5].w=T_05_WIDTH;
  Textures[ 6].name=T_06; Textures[ 6].h=T_06_HEIGHT; Textures[ 6].w=T_06_WIDTH;
  Textures[ 7].name=T_07; Textures[ 7].h=T_07_HEIGHT; Textures[ 7].w=T_07_WIDTH;
  Textures[ 8].name=T_08; Textures[ 8].h=T_08_HEIGHT; Textures[ 8].w=T_08_WIDTH;
  Textures[ 9].name=T_09; Textures[ 9].h=T_09_HEIGHT; Textures[ 9].w=T_09_WIDTH;
  Textures[10].name=T_10; Textures[10].h=T_10_HEIGHT; Textures[10].w=T_10_WIDTH;
  Textures[11].name=T_11; Textures[11].h=T_11_HEIGHT; Textures[11].w=T_11_WIDTH;
  Textures[12].name=T_12; Textures[12].h=T_12_HEIGHT; Textures[12].w=T_12_WIDTH;
  Textures[13].name=T_13; Textures[13].h=T_13_HEIGHT; Textures[13].w=T_13_WIDTH;
  Textures[14].name=T_14; Textures[14].h=T_14_HEIGHT; Textures[14].w=T_14_WIDTH;
  Textures[15].name=T_15; Textures[15].h=T_15_HEIGHT; Textures[15].w=T_15_WIDTH;
  Textures[16].name=T_16; Textures[16].h=T_16_HEIGHT; Textures[16].w=T_16_WIDTH;
  Textures[17].name=T_17; Textures[17].h=T_17_HEIGHT; Textures[17].w=T_17_WIDTH;
  Textures[18].name=T_18; Textures[18].h=T_18_HEIGHT; Textures[18].w=T_18_WIDTH;
  Textures[19].name=T_19; Textures[19].h=T_19_HEIGHT; Textures[19].w=T_19_WIDTH;

}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowPosition(GLSW/2, GLSH/2);
  glutInitWindowSize(GLSW, GLSH);
  glutCreateWindow("");
  glPointSize(pixelScale);
  gluOrtho2D(0, GLSW, 0, GLSH);
  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(KeysDown);
  glutKeyboardUpFunc(KeysUp);
  glutMainLoop();

  return 0; 
}
