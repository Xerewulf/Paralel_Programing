/**
 *
 * CENG342 Project-2
 *
 * Histogram equalization with OpenMP
 *
 * Usage:  main <input.jpg> <output.jpg> 
 *
 * @group_id 00
 * @author  your names
 *
 * @version 1.0, 02 April 2022
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define RANGE 256

#include "stb_image.h"
#include "stb_image_write.h"
#define CHANNEL_NUM 1

void seq_histogram_equalizer(uint8_t* rgb_image,int width, int height);
void par_histogram_equalizer(uint8_t* rgb_image,int width, int height);

int main(int argc,char* argv[]) 
{		
    int width, height, bpp;
	
	// Reading the image in grey colors
    uint8_t* rgb_image = stbi_load(argv[1], &width, &height, &bpp, CHANNEL_NUM);
	
    printf("Width: %d  Height: %d \n",width,height);
	printf("Input: %s , Output: %s  \n",argv[1],argv[2]);
	
	// start the timer
	double time1= omp_get_wtime();	
	
	//seq_histogram_equalizer(rgb_image,width, height);
	par_histogram_equalizer(rgb_image,width, height);
    
	double time2= omp_get_wtime();	
	printf("Elapsed time: %lf \n",time2-time1);	
	
	// Storing the image 
    stbi_write_jpg(argv[2], width, height, CHANNEL_NUM, rgb_image, 100);
    stbi_image_free(rgb_image);

    return 0;
}


void par_histogram_equalizer(uint8_t* rgb_image,int width, int height)
{
	int *hist = (int*)calloc(RANGE,sizeof(int));
    double size = width * height;
    int i,j;

#pragma omp parallel for collapse(2) default(none) private (i,j) shared(height,width,rgb_image,hist)  schedule(guided,1000)
	for(i=0; i<height ; i++){
		for(j=0; j<width; j++){
			hist[rgb_image[i*width + j]]++;
		}
	}
	// reduction(+:hist[:RANGE]) 		
    //cumulative sum for histogram values
    int cumhistogram[RANGE];
	cumhistogram[0] = hist[0];
    int k;

//following loop has dependencies so no parallelism has been applied
    for(k = 1; k < RANGE; k++)
    {
        cumhistogram[k] = hist[k] + cumhistogram[k-1];
    }    
	
    int alpha[RANGE];
    int m;

#pragma omp parallel for default (none) private (m) shared(cumhistogram,alpha,size)  schedule(guided,1000)
    for(m = 0; m < RANGE; m++)
    {
        alpha[m] = round((double)cumhistogram[m] * (255.0/size));
    }
			
    // histogram equlized image	
    int x,y;	

#pragma omp parallel for collapse(2)  default(none) private (x,y) shared(height,width,rgb_image,alpha) schedule(guided,1000)
    for(y = 0; y <height ; y++){
        for(x = 0; x < width; x++){
            rgb_image[y*width + x] = alpha[rgb_image[y*width + x]];
		}
	}
}
