#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>     /* OpenGL functions */
#endif

unsigned int first = 1;
char desenhaBorda = 1;

QuadNode* newNode(int x, int y, int width, int height)
{
    QuadNode* n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}

QuadNode* Executa(RGBPixel *pixels, int *blackWhite, int height, int width ,int x, int y, float minError){
    QuadNode* auxiliar = newNode(x, y, width, height);
    
    unsigned char corMedia[3];
    int i, j;
    int histograma[256] = {0};
    int quantPixels, aux, intensidade, erro, r, g, b = 0;
    
    for (i = 0; i < height; i++)
    {
        for ( j = 0; i < width; j++)
        {
            r += pixels[i * width + j].r;
            g += pixels[i * width + j].g;
            b += pixels[i * width + j].b;
            quantPixels++;
        }
    }

    int meiaLargura = width/2;
    int meiaAltura = height/2;

    corMedia[0] = r/quantPixels;
    corMedia[1] = g/quantPixels;
    corMedia[2] = b/quantPixels;

    auxiliar->color[0] = corMedia[0];
    auxiliar->color[1] = corMedia[1];
    auxiliar->color[2] = corMedia[2];

    for (i = 0; i < height; i++)
    {
        for ( j = 0; i < width; j++)
        {
            histograma[blackWhite[i * width + j]] += 1;
        }
    }

    for (i = 0; i< 256 ; i++){
        intensidade += histograma[i] * i;
    }

    for (i = 0; i < height; i++)
    {
        for ( j = 0; i < width; j++)
        {
            aux +=  pow((blackWhite[i * width + j] - intensidade),2);
        }
    }
    erro = (1/width * height) * aux;
    erro = sqrt(erro);

    if(minError < erro){
        QuadNode* nw = Executa(pixels, blackWhite, meiaAltura, meiaLargura, 0, 0, erro);
        QuadNode* ne = Executa(pixels, blackWhite, meiaAltura, meiaLargura, 0, meiaLargura, erro);
        QuadNode* sw = Executa(pixels, blackWhite, meiaAltura, meiaLargura, meiaAltura, 0, erro);
        QuadNode* se = Executa(pixels, blackWhite, meiaAltura, meiaLargura, meiaAltura, meiaLargura, erro);
        auxiliar->NW = nw;
        auxiliar->NE = ne;
        auxiliar->SW = sw;
        auxiliar->SE = se;
    }
    return auxiliar;
}

QuadNode* geraQuadtree(Img* pic, float minError)
{
    // RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;


    RGBPixel *pixels = malloc(pic->height * pic->width * sizeof(RGBPixel));
    int *blackAndWhite = malloc(pic->height * pic->width * sizeof(int));

    int i, j;
    int width = pic->width;
    int height = pic->height;
    // for(i=0; i<height; i++){
    //     blackAndWhite[i] = malloc(width * height * sizeof(unsigned char));  
    // }


    for (i = 0; i < height; i++)
    {
        for ( j = 0; j < width; j++)
        {
            int pixelCinza = (0.3 * pixels[i * width + j].r) + (0.59 * pixels[i * width + j].g) + (0.11 * pixels[i * width + j].b);
            blackAndWhite[i * width + j] = pixelCinza;
        }
        
    }
    
    QuadNode* raiz = Executa(pixels, blackAndWhite, height, width, 0, 0, minError);

    free(blackAndWhite);

    return raiz;
#define DEMO
#ifdef DEMO


    // QuadNode* raiz = newNode(0,0,width,height);
    // raiz->status = PARCIAL;
    // raiz->color[0] = 0;
    // raiz->color[1] = 0;
    // raiz->color[2] = 255;

    // int meiaLargura = width/2;
    // int meiaAltura = height/2;

    // QuadNode* nw = newNode(meiaLargura, 0, meiaLargura, meiaAltura);
    // nw->status = PARCIAL;
    // nw->color[0] = 0;
    // nw->color[1] = 0;
    // nw->color[2] = 255;

    // // Aponta da raiz para o nodo nws
    // raiz->NW = nw;

    // QuadNode* nw2 = newNode(meiaLargura+meiaLargura/2, 0, meiaLargura/2, meiaAltura/2);
    // nw2->status = CHEIO;
    // nw2->color[0] = 255;
    // nw2->color[1] = 0;
    // nw2->color[2] = 0;

    // // Aponta do nodo nw para o nodo nw2
    // nw->NW = nw2;
    
#endif
    // return raiz;
}

// Limpa a memória ocupada pela árvore
void clearTree(QuadNode* n)
{
    if(n == NULL) return;
    if(n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    //printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder() {
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode* raiz) {
    if(raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode* raiz) {
    FILE* fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE* fp, QuadNode* n)
{
    if(n == NULL) return;

    if(n->NE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if(n->NW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if(n->SE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if(n->SW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode* n)
{
    if(n == NULL) return;

    glLineWidth(0.1);

    if(n->status == CHEIO) {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x+n->width-1, n->y);
        glVertex2f(n->x+n->width-1, n->y+n->height-1);
        glVertex2f(n->x, n->y+n->height-1);
        glEnd();
    }

    else if(n->status == PARCIAL)
    {
        if(desenhaBorda) {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x+n->width-1, n->y);
            glVertex2f(n->x+n->width-1, n->y+n->height-1);
            glVertex2f(n->x, n->y+n->height-1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!

}

    

