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
} TextureMaps; TextureMaps Texturearea[44]; // aumenta para mais texturas
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

void pixel(int x, int y, int c) {
  int rgb[3];
  
  if (c==0) {
    rgb[0]=255; rgb[1]=255; rgb[2]=0; // Amarelo
  }
  if (c==1) {
    rgb[0]=160; rgb[1]=160; rgb[2]=0; // Amarelo escuro
  }
  if (c==2) {
    rgb[0]=0; rgb[1]=255; rgb[2]=0; // Verde
  }
  if (c==3) {
    rgb[0]=0; rgb[1]=160; rgb[2]=0; // Verde escuro
  }
  if (c==4) {
    rgb[0]=0; rgb[1]=255; rgb[2]=255; // Ciano
  }
  if (c==5) {
    rgb[0]=0; rgb[1]=160; rgb[2]=160; // Ciano escuro
  }
  if (c==6) {
    rgb[0]=160; rgb[1]=100; rgb[2]=0; // Marrom
  }
  if (c==7) {
    rgb[0]=110; rgb[1]=50; rgb[2]=0; // Marrom escuro
  }
  if (c==8) {
    rgb[0]=0; rgb[1]=60; rgb[2]=130; // Fundo
  }
  glColor3ub (rgb[0], rgb[1], rgb[2]);
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
    P.x += dx; P.y  -= dy;
  }
  if (K.sl==1) {
    P.x -= dx; P.y  += dy;
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
  int x, y;

  for (y=0; y < SH; y++) {
    for (x=0; x < SW; x++) {
      pixel (x, y, 8);
    }
  }
}

void clipBehindPlayer(int *x1, int *y1, int *z1, int x2, int y2, int z2) {
  float da = *y1; // distancia do plano -> ponto a
  float db = y2; // distancia do plano -> ponto b
  float d = da - db; if (d==0) {d=1;}
  float s = da / (da - db); // fator de interceção (entre 0 e 1)
  *x1 = *x1 + s * (x2 - (*x1)); 
  *y1 = *y1 + s * (y2 - (*y1)); if (*y1 == 0) {*y1=1;} // evita divisão por zero
  *z1 = *z1 + s * (z2 - (*z1));
}

void drawWall(int x1, int x2, int b1, int b2, int t1, int t2, int s, int w, int frontBack) {
  int x, y;

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
    //
    // desenha frente da parede
    if (frontBack==0) {
      if (S[s].surface==1){S[s].surf[x]=y1;}
      if (S[s].surface==2){S[s].surf[x]=y2;}
      for (y = y1; y < y2; y++) {
        pixel(x, y, 0); // parde normal
      }
    }

    // desenha a tras da parede
    if (frontBack==1) {
      if (S[s].surface==1){S[s].surf[x];}
      if (S[s].surface==2){S[s].surf[x];}
      for (y = y1; y < y2; y++) {
        pixel(x, y, 2); // superficie
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
  int x, s, w, frontBack, cycles, loop, wx[4], wy[4], wz[4]; float CS = M.cos[P.a], SN = M.sin[P.a];

  // ordem dos setores por distancia
  for (s=0; s < numSect-1; s++) {
    for (w=0; w < numSect-s-1; w++) {
      if (S[w].d < S[w+1].d) {
        sectors st = S[w]; S[w]=S[w+1]; S[w+1]=st;
      }
    }
  }

  // Desenha setores 
  for (s=0; s<numSect; s++) {
    S[s].d=0; // limpa distancia
    if (P.z<S[s].z1) {
      S[s].surface=1; cycles=2; for (x=0; x<SW; x++) {S[s].surf[x]=SH;}; // superficie inferior
    } else if (P.z>S[s].z2) {
      S[s].surface=2; cycles=2; for (x=0; x<SW; x++) {S[s].surf[x]=0;}; // superficie superior
    } else {
      S[s].surface = 0; cycles=1;// sem superficie
    }
    for (frontBack=0; frontBack < cycles; frontBack++) {

      for (w=S[s].ws; w<S[s].we; w++) {
        // Deslocamento de dois pontos inferiores ao jogador
        int x1 = W[w].x1 - P.x, y1 = W[w].y1 - P.y;
        int x2 = W[w].x2 - P.x, y2 = W[w].y2 - P.y;

        // troca para a superficie
        if (frontBack == 1) {
          int swp = x1; x1 = x2; x2 = swp; swp = y1; y1 = y2; y2 = swp;
        }

        // Posição x no Mundo
        wx[0] = x1*CS - y1*SN;
        wx[1] = x2*CS - y2*SN;
        wx[2] = wx[0];
        wx[3] = wx[1];
        // Posição y no Mundo (profundade n altura)
        wy[0] = y1*CS + x1*SN;
        wy[1] = y2*CS + x2*SN;
        wy[2] = wy[0];
        wy[3] = wy[1];
        S[s].d += dist(0,0,(wx[0]+wx[1])/2, (wy[0]+wy[1])/2); // armazena a distancia desta parede
                                                              // Altura z no mundo
        wz[0] = S[s].z1 - P.z + ((P.l*wy[0])/32.0);
        wz[1] = S[s].z1 - P.z + ((P.l*wy[1])/32.0);
        wz[0] = S[s].z1 - P.z + ((P.l*wy[0])/32.0);
        wz[1] = S[s].z1 - P.z + ((P.l*wy[1])/32.0);


        // não desenha atrás do jogador
        if (wy[0]<1 && wy[1]<1)
          continue;
        // se ponto 1 atrás do jogador, corta
        if (wy[0]<1) {
          clipBehindPlayer(&wx[0], &wy[0], &wz[0], wx[1], wy[1], wz[1]); // linha de baixo
          clipBehindPlayer(&wx[2], &wy[2], &wz[2], wx[3], wy[3], wz[3]); // linha de cima
        }

        // se ponto 2 atrás do jogador, corta
        if (wy[1]<1) {
          clipBehindPlayer(&wx[1], &wy[1], &wz[1], wx[0], wy[0], wz[0]); // linha de baixo
          clipBehindPlayer(&wx[3], &wy[3], &wz[3], wx[2], wy[2], wz[2]); // linha de cima
        }

        // x e y da tela (centro)
        wx[0] = wx[0]*200/wy[0]+SW2; wy[0] = wz[0]*200/wy[0]+SH2;
        wx[1] = wx[1]*200/wy[1]+SW2; wy[1] = wz[1]*200/wy[1]+SH2;
        wx[2] = wx[2]*200/wy[2]+SW2; wy[2] = wz[2]*200/wy[2]+SH2;
        wx[3] = wx[3]*200/wy[3]+SW2; wy[3] = wz[3]*200/wy[3]+SH2;
        // desenha os pontos
        // if (wx[0] > 0 && wx[0] < SW && wy[0] > 0 && wy[0] < SH)
        //   pixel(wx[0], wy[0], 0);
        // if (wx[1] > 0 && wx[1] < SW && wy[1] > 0 && wy[1] < SH)
        //   pixel(wx[1], wy[1], 0);
        drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], s, w, frontBack);
      }
      S[s].d /= (S[s].we - S[s].ws); // Encontra a distancia média do setor
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
