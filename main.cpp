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
#define numSect 4
#define numWall 16

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
} walls; walls W[30];

typedef struct {
  int ws, we; // Numbero de começo e fim da parede
  int z1, z2; // Altura do fundo e do topo
  // int x, y; // Posição central para o setor
  int d; // Adiciona uma distancia y para ordenar a direção
  int c1, c2; // Cor de fundo e de topo
  int surf[SW]; // Pontos de espera para superficies
  int surface; // é a superficie
} sectors; sectors S[30];

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

void drawWall(int x1, int x2, int b1, int b2, int t1, int t2, int c, int s) {
  int x, y;

  // mantem a direfença na distancia
  int dyb = b2 - b1; // distancia y do linha de baixo
  int dyt = t2 - t1; // distancia y da linha de cima
  int dx = x2 - x1; if (dx == 0) { dx=1; }; // distancia x
  int xs = x1; // mantem x1 inicial 

  // Recorta o x 
  if (x1 < 1) 
    x1 = 1; // corta esquerda
  if (x2 < 1) 
    x2 = 1; // corta esquerda
  if (x1 > SW-1) 
    x1 = SW-1; // corta direita
  if (x2 > SW-1) 
    x2 = SW-1; // corta direita

  // desenha x linhas verticais
  for (x = x1; x < x2; x++) {
    // O Y inicia e termina
    int y1 = dyb*(x - xs + 0.5)/dx+b1; // ponto y inferior
    int y2 = dyt*(x - xs + 0.5)/dx+t1; // ponto y inferior
                                       
    // Recorta y
    if (y1 < 1)
      y1 = 1; // baixo
    if (y2 < 1)
      y2 = 1; // baixo
    if (y1 > SH-1)
      y1 = SH-1; // cima
    if (y2 > SH-1)
      y2 = SH-1; // cima
    
    // Superficie 
    if (S[s].surface == 1) {
      S[s].surf[x] = y1; continue; // Salva os pontos inferiores
    }
    if (S[s].surface == 2) {
      S[s].surf[x] = y2; continue; // Salva os pontos inferiores
    }
    if (S[s].surface == -1) {
      for (y=S[s].surf[x]; y<y1; y++) {
        pixel(x, y, S[s].c1); // baixo
      }; // Salva os pontos inferiores
    }
    if (S[s].surface == -2) {
      for (y=y1; y<S[s].surf[x]; y++) {
        pixel(x, y, S[s].c2); // cima
      }; // Salva os pontos inferiores
    }

    // pixel(x, y1, 0); // baixo
    // pixel(x, y2, 0); // cima

    for (y = y1; y < y2; y++) {
      pixel(x, y, c); // parde normal
    }
  }
}

int dist(int x1, int y1, int x2, int y2) {
  int distance = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
  return distance;
}

void draw3D() {
  int s, w, loop, wx[4], wy[4], wz[4]; float CS = M.cos[P.a], SN = M.sin[P.a];

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
      S[s].surface=1; // superficie inferior
    } else if (P.z>S[s].z2) {
      S[s].surface=2; // superficie superior
    } else {
      S[s].surface = 0; // sem superficie
    }
    for (loop=0; loop < 2; loop++) {

      for (w=S[s].ws; w<S[s].we; w++) {
        // Deslocamento de dois pontos inferiores ao jogador
        int x1 = W[w].x1 - P.x, y1 = W[w].y1 - P.y;
        int x2 = W[w].x2 - P.x, y2 = W[w].y2 - P.y;

        // troca para a superficie
        if (loop == 0) {
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
        wz[2] = wz[0] + S[s].z2;
        wz[3] = wz[1] + S[s].z2;

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
        drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], W[w].c, s);
      }
      S[s].d /= (S[s].we - S[s].ws); // Encontra a distancia média do setor
      S[s].surface*=-1; // vira para navegar até a superfície do desenho
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

int loadSectors[] = {
  // Inicio, fim, z1 altura, z2 altura, da parede
  0, 4, 0, 40, 2, 3, // setor 1
  4, 8, 0, 40, 4, 5, // setor 2
  8, 12, 0, 40, 6, 7, // setor 3
  12, 16, 0, 40, 0, 1 // setor 4
};

int loadWalls[] = {
  // x1, y1, x2, y2, cor
  0, 0, 32, 0, 0,
  32, 0, 32, 32, 1,
  32, 32, 0, 32, 0,
  0, 32, 0, 0, 1,

  64, 0, 96, 0, 2,
  96, 0, 96, 32, 3,
  96, 32, 64, 32, 2,
  64, 32, 64, 0, 3,

  64, 64, 96, 64, 4,
  96, 64, 96, 96, 5,
  96, 96, 64, 96, 4,
  64, 96, 64, 64, 5,

  0, 64, 32, 64, 6,
  32, 64, 32, 96, 7,
  32, 96, 0, 96, 6,
  0, 96, 0, 64, 7,
};

void init() {
  int x;

  // converte de radiano para graus (dividindo por 180 e multiplicando por pi)
  for (x=0; x<360; x++) {
    M.cos[x] = cos(x/180.0*M_PI);
    M.sin[x] = sin(x/180.0*M_PI);
  }

  // Inicializa o Jogador
  P.x = 70; P.y = -110; P.z = 20; P.a = 0; P.l = 0; // Variáveis iniciais do jogador
                                                    
  // Carrega setores
  int s, w, v1=0, v2=0;

  for (s = 0; s < numSect; s++) {
    S[s].ws = loadSectors[v1+0]; // numero que inicia a parede
    S[s].we = loadSectors[v1+1]; // numero que termina a parede
    S[s].z1 = loadSectors[v1+2]; // altura inferior da parede
    S[s].z2 = loadSectors[v1+3] - loadSectors[v1+2]; // altura superior da parede
    S[s].c1 = loadSectors[v1+4]; // Cor do topo do setor
    S[s].c2 = loadSectors[v1+5]; // Cor do fundo do setor
    v1 += 6;

    for (w=S[s].ws; w < S[s].we; w++) {
      W[w].x1=loadWalls[v2+0]; // x1 baixo
      W[w].y1=loadWalls[v2+1]; // y1 baixo
      W[w].x2=loadWalls[v2+2]; // x2 cima
      W[w].y2=loadWalls[v2+3]; // y2 cima
      W[w].c=loadWalls[v2+4]; // cor da parede
      v2+=5;
    }
  }
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
