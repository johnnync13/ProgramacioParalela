#include <iostream>
#include <boost/lexical_cast.hpp>
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


/*  */
void (*func_ptr[3])(uchar3*, uchar4*, int, int) = {
    convertBRG2RGBA,
    convertBRG2RGBA_2,
    convertBRG2RGBA_3
};

/* FUNCIO PRINCIPAL */
int main(int argc, char *argv[])
{
    /* argv: 1 -> numero de l'exercici, 2 -> numero d'experiments */
    
    /* DECLARACIO DE VARIABLES */
    uchar3 *h_brg;
    uchar4 *h_rgba;
    
    /* func_ptr_conver: Punter a la funcio de conversio */
    void (*func_ptr_conver)(uchar3*, uchar4*, int, int);

    int bar_widht       = HEIGHT/3;
    
    /* numero_exercici: El numero de l'exercici que volem executar */
    /* experiments: El numero d'experiments que volem assajar */
    int numero_exercici = boost::lexical_cast<int>(argv[1]) - 1;
    int experiment      = boost::lexical_cast<int>(argv[2]);
    
    /* duration: Emmagatzema la duracio de totes les execucions del programa */
    float duration      = 0.0;
    
    /* Escollim la funcio de conversio */
    func_ptr_conver = func_ptr[numero_exercici];

    // Alloc and generate BRG bars.
    h_brg = (uchar3*)malloc(sizeof(uchar3)*WIDTH*HEIGHT);
    for (int i=0; i < WIDTH * HEIGHT; ++i) {
        if (i < bar_widht) { h_brg[i] = { 255, 0, 0 }; }
        else if (i < bar_widht*2) { h_brg[i] = { 0, 255, 0 }; }
        else { h_brg[i] = { 0, 0, 255 }; }
    }

    // Alloc RGBA pointers
    h_rgba = (uchar4*)malloc(sizeof(uchar4)*WIDTH*HEIGHT);
    
    /* Executant el numero d'experiments */
    #pragma omp parallel
    {
    #pragma omp critical
    {
        std::cout << "Soc el fil numero " 
        << omp_get_thread_num() << std::endl;
    }
    #pragma omp single
    for (int t = 0; t < experiment; t++)
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i=0; i<EXPERIMENT_ITERATIONS; ++i) 
        {
            #pragma omp task
            func_ptr_conver(h_brg, h_rgba, WIDTH, HEIGHT);
        }
        auto t2 = std::chrono::high_resolution_clock::now();

        duration += std::chrono::duration_cast<std::chrono::microseconds>
                ( t2 - t1 ).count();
    }
    }
    
    std::cout << "convertBRG2RGBA time for " << EXPERIMENT_ITERATIONS \
    << " iterations = "<< duration / experiment << "us" << std::endl;
    
    std::cout << "Executed " << experiment << " times" << std::endl;

    bool ok = checkResults(h_rgba, h_brg, WIDTH*HEIGHT);

    if (ok) {
        std::cout << "Executed!! Results OK." << std::endl;
    } else {
        std::cout << "Executed!! Results NOT OK." << std::endl;
    }

    return 0;

}
