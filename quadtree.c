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

QuadNode* Executa(int height, int width, RGBPixel **pixels , unsigned char** blackWhite, int x, int y, float minError){
    QuadNode* auxiliar = newNode(x, y, width, height);
    
    unsigned char color[3];
    int i, j;
    int histograma[256] = {0};
    int quantPixels = 0, aux = 0, intensidade = 0, r = 0, g = 0, b = 0;
    double erro = 0;
    
     for (i = y; i < y + height; i++) {
        for (j = x; j < x + width; j++) {
            r += pixels[i][j].r;
            g += pixels[i][j].g;
            b += pixels[i][j].b;
            quantPixels++;
        }
    }

    int meiaLargura = width/2;
    int meiaAltura = height/2;

    color[0] = r/quantPixels;
    color[1] = g/quantPixels;
    color[2] = b/quantPixels;

    auxiliar->color[0] = color[0];
    auxiliar->color[1] = color[1];
    auxiliar->color[2] = color[2];

    for (i = y; i < y + height; i++)
    {
        for (j = x; j < x + width; j++)
        {
            histograma[blackWhite[i][j]] += 1;
        }
    }

    for (i = 0; i< 256 ; i++){
        intensidade += histograma[i] * i;
    }
    intensidade = intensidade/quantPixels;

    for (i = y; i < y + height; i++)
    {
        for (j = x; j < x + width; j++)
        {
            aux = blackWhite[i][j] - intensidade;
            erro += aux * aux;
        }
    }
    aux = erro / (width * height);
    erro = sqrt(aux);
    if(erro <= minError){
        auxiliar->status = CHEIO;
        return auxiliar;
    }else{
        auxiliar->status = PARCIAL;
        auxiliar->NW = Executa(meiaAltura, meiaLargura, pixels, blackWhite, x, y, minError);
        auxiliar->NE = Executa(meiaAltura, meiaLargura, pixels, blackWhite, x + meiaLargura , y , minError);
        auxiliar->SW = Executa(meiaAltura, meiaLargura, pixels, blackWhite, x , y + meiaAltura , minError);
        auxiliar->SE = Executa(meiaAltura, meiaLargura, pixels, blackWhite, x + meiaLargura, y + meiaAltura, minError);
        return auxiliar;
    }
}

QuadNode* geraQuadtree(Img* pic, float minError)
{
    RGBPixel **pixels = (RGBPixel **)malloc(pic->height * sizeof(RGBPixel *));
    unsigned char** blackAndWhite = (unsigned char**)malloc(pic->height * sizeof(unsigned char*));

    int i, j;
    int width = pic->width;
    int height = pic->height;
    

    for(i = 0; i < height; i ++){
        pixels[i] = (RGBPixel *)malloc(width * sizeof(RGBPixel));
        blackAndWhite[i] = (unsigned char*)malloc(width * sizeof(unsigned char));
    }


    for(i = 0; i< height; i++){
        for(j = 0; j < width; j++){
            pixels[i][j] = pic->img[i * width + j];
            blackAndWhite[i][j] = (0.3 * pixels[i][j].r) + (0.59 * pixels[i][j].g) + (0.11 * pixels[i][j].b);
        }
    }

    QuadNode* raiz = Executa(height, width, pixels, blackAndWhite, 0, 0, minError);
    
    return raiz;
    
    // RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;

    // int i, j;
    // int width = pic->width;
    // int height = pic->height;

    // unsigned char** blackAndWhite = (unsigned char**)malloc(pic->height * sizeof(unsigned char*));
    // // for (int i = 0; i < pic->height; i++) {
    // //     blackAndWhite[i] = (unsigned char*)malloc(width * sizeof(unsigned char));
    // // }




    // for (i = 0; i < height; i++)
    // {
    //     for ( j = 0; j < width; j++)
    //     {
    //         int pixelCinza = (0.3 * pixels[i][j].r) + (0.59 * pixels[i][j].g) + (0.11 * pixels[i][j].b);
    //         blackAndWhite[i][j] = (unsigned char)pixelCinza;
    //     }
        
    // }
    

    // return Executa(height, width, pixels, blackAndWhite, 0, 0, minError);
    
    // for (int i = 0; i < height; i++) {
    //     free(blackAndWhite[i]);
    // }
    // free(blackAndWhite);
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
