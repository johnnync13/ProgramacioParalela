#include <iostream>
#include <omp.h>
#include <chrono>

#define WIDTH 3840
#define HEIGHT 2160

#define EXPERIMENT_ITERATIONS 1000

typedef unsigned char uchar;

struct _uchar3 {
    uchar x;
    uchar y;
    uchar z;
} __attribute__ ((aligned (4)));

using uchar3 = _uchar3;

struct _uchar4 {
    uchar x;
    uchar y;
    uchar z;
    uchar w;
};

using uchar4 = _uchar4;

bool checkResults(uchar4* rgba, uchar3* brg, int size) {

    bool correct = true;

    for (int i=0; i < size; ++i) {
        correct &= rgba[i].x == brg[i].y;
        correct &= rgba[i].y == brg[i].z;
        correct &= rgba[i].z == brg[i].x;
        correct &= rgba[i].w == 255;
    }

    return correct;
}

void convertBRG2RGBA(uchar3* brg, uchar4* rgba, int width, int height) {
    for (int x=0; x<width; ++x) {
    	for (int y=0; y<height; ++y) {	
            rgba[width * y + x].x = brg[width * y + x].y;
            rgba[width * y + x].y = brg[width * y + x].z;
            rgba[width * y + x].z = brg[width * y + x].x;
            rgba[width * y + x].w = 255;
        }
    }
}

void convertBRG2RGBA_2(uchar3* brg, uchar4* rgba, int width, int height) 
{
    /* Per optimitzar el codi accedim de manera lineal al vector, de manera que */
    /* cada accés estigui a una posició contigua a la memoria de l'accés anterior */
    for (int y=0; y<height; ++y)
    {
        for (int x=0; x<width; ++x)
        {	
            uchar3 tmp_3 = brg[width* y + x];
            uchar4 tmp_4;
            
            tmp_4.x = tmp_3.y;
            tmp_4.y = tmp_3.z;
            tmp_4.z = tmp_3.x;
            tmp_4.w = 255;
            
            rgba[width* y + x] = tmp_4;
        }
    }
}

void convertBRG2RGBA_3(uchar3* brg, uchar4* rgba, int width, int height) 
{
    /* Per paral·lelitzar la e */ 
    for (int y=0; y<height; ++y)
    {
#pragma omp parallel for
        for (int x=0; x<width; ++x)
        {	
            uchar3 tmp_3 = brg[width* y + x];
            uchar4 tmp_4;
            
            tmp_4.x = tmp_3.y;
            tmp_4.y = tmp_3.z;
            tmp_4.z = tmp_3.x;
            tmp_4.w = 255;
            
            rgba[width* y + x] = tmp_4;
        }
    }
}

int main(int argc, char *argv[]) {

    uchar3 *h_brg;
    uchar4 *h_rgba;

    int bar_widht = HEIGHT/3;

    // Alloc and generate BRG bars.
    h_brg = (uchar3*)malloc(sizeof(uchar3)*WIDTH*HEIGHT);
    for (int i=0; i < WIDTH * HEIGHT; ++i) {
        if (i < bar_widht) { h_brg[i] = { 255, 0, 0 }; }
        else if (i < bar_widht*2) { h_brg[i] = { 0, 255, 0 }; }
        else { h_brg[i] = { 0, 0, 255 }; }
    }

    // Alloc RGBA pointers
    h_rgba = (uchar4*)malloc(sizeof(uchar4)*WIDTH*HEIGHT);

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i=0; i<EXPERIMENT_ITERATIONS; ++i) 
    {    
        convertBRG2RGBA_3(h_brg, h_rgba, WIDTH, HEIGHT);
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << "convertBRG2RGBA time for " << EXPERIMENT_ITERATIONS \
    << " iterations = "<< duration << "us" << std::endl;

    bool ok = checkResults(h_rgba, h_brg, WIDTH*HEIGHT);

    if (ok) {
        std::cout << "Executed!! Results OK." << std::endl;
    } else {
        std::cout << "Executed!! Results NOT OK." << std::endl;
    }

    return 0;

}
