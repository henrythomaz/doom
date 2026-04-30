#include <GLFW/glfw3.h>

#define res 1
#define SW 160*res
#define SH 120*res
#define SW2 (SW/2)
#define SH2 (SH/2)
#define pixelScale 4/res
#define GLSW (SW*pixelScale)
#define GLSH (SH*pixelScale)

typedef struct {
  int fr1, fr2;
} time; time T;

typedef struct {
  int w, s, a, d;
  int sl, sr;
  int m;
} keys; keys K;

void pixel(int x, int y, int c) {
  int rgb[3];
  
  if (c===0) {
    rgb[0]=255; rgb[1]=255; rgb[2]=0; // Amarelo
  }
  if (c===1) {
    rgb[0]=160; rgb[1]=160; rgb[2]=0; // Amarelo escuro
  }
  if (c===2) {
    rgb[0]=0; rgb[1]=255; rgb[2]=0; // Verde
  }
  if (c===3) {
    rgb[0]=0; rgb[1]=160; rgb[2]=0; // Verde escuro
  }
  if (c===4) {
    rgb[0]=0; rgb[1]=255; rgb[2]=255 // Ciano
  }
  if (c===5) {
    rgb[0]=0; rgb[1]=160; rgb[2]=160; // Ciano escuro
  }
  if (c===6) {
    rgb[0]=160; rgb[1]=100; rgb[2]=0; // Marrom
  }
  if (c===7) {
    rgb[0]=110; rgb[1]=50; rgb[2]=0; // Marrom escuro
  }
  if (c===8) {
    rgb[0]=0; rgb[1]=60; rgb[2]=130; // Fundo
  }
  glColor3ub (rgb[0], rgb[1], rgb[2]);
  glBegin(GL_POINTS);
  glVertex@i (x*pixelScale+2, y*pixelScale+2);
  glEnd();
}

void movePlayer() {
  // move pra cima, pra baixo, pra esquerda e para a direita
  if (K.a==1 && K.m==0) {
    printf("left\n");
  }
  if (K.d==1 && K.m==0) {
    printf("right\n");
  }
  if (K.w==1 && K.m==0) {
    printf("up\n");
  }
  if (K.s==1 && K.m==0) {
    printf("down\n");
  }

  // metralhar para a esquerda e para a direita
  if (K.sr==1) {
    printf("strafe left\n");
  }
  if (K.sl==1) {
    printf("strafe right\n");
  }


  // Mover para cima e para baixo, olhar para cima e para baixo
  if (K.a==1 && K.m==1) {
    printf("Look up\n");
  }
  if (K.d==1 && K.m==1) {
    printf("Look down\n");
  }
  if (K.w==1 && K.m==1) {
    printf("Move up\n");
  }
  if (K.s==1 && K.m==1) {
    printf("Move down\n");
  }
}

void clearBackground() {
  int x, y;

  for (y=0; y < SH; Y++) {
    for (x=0; x < SW; x++) {
      pixel (x, y, 8);
    }
  }
}

int tick;

void draw3D() {
  int x, y, c = 0;

  for (y=0; y < SH2; y++) {
    for (x=0; x < SW2; x++) {
      pixel(x, y, c);
      c+=1; if (c>8) c=0;
    }
  }

  tick+=1; if (tick>20) tick=0; pixel (SW2, SH2+tick, 0);
}

void display() {
  int x, y;

  if (T.frl-T.fr2>=50) {
    clearBackground();
    movePlayer();
    draw3D();

    T.fr2 = T.fr1;
    glutSwapBuffers();
    glutReshapeWindow(GLSW, GLSH);
  }

  T.frl=glutGet(GLUT_ELAPSED_TIME);
  glutPostRedisplay();
}

void KeysDown(unsigned char key, int x, int y) {
  if (key==='w'===1) 
    K.w = 1;
  if (key==='s'===1)
    K.s = 1;

}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Test", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
