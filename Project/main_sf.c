/**
 *
 * CENG342 Project-1
 *
 * Histogram equalization 
 *
 * Usage:  main <input.jpg> <output.jpg> 
 *
 * @group_id 
 * @author  your names
 *
 * @version 1.0, 02 April 2022
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#define CHANNEL_NUM 1

void seq_histogram_equalizer(uint8_t* rgb_image,int width, int height);
void par_histogram_equalizer(uint8_t* rgb_image,int width, int height);

int main(int argc,char* argv[]) 
{	
	MPI_Init(&argc,&argv);
    int width, height, bpp;
	
	// Reading the image in grey colors
    uint8_t* rgb_image = stbi_load(argv[1], &width, &height, &bpp, CHANNEL_NUM);
	
    //printf("Width: %d  Height: %d \n",width,height);
	//printf("Input: %s , Output: %s  \n",argv[1],argv[2]);
	
	// start the timer
	double time1= MPI_Wtime();	
	
	//seq_histogram_equalizer(rgb_image,width, height);
	par_histogram_equalizer(rgb_image,width, height);
    
	double time2= MPI_Wtime();	
	printf("Elapsed time: %lf \n",time2-time1);	
	
	// Storing the image 
    stbi_write_jpg(argv[2], width, height, CHANNEL_NUM, rgb_image, 100);
    stbi_image_free(rgb_image);

	MPI_Finalize();
    return 0;
}
// You must only modifty this function
void par_histogram_equalizer(uint8_t* rgb_image,int width, int height)
{
	int rank,num_proc;
	int *hist = (int*)calloc(256,sizeof(int));
		MPI_Comm_size(MPI_COMM_WORLD,&num_proc);
    	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   	
   	//hist[rgb_image[i*width + j]]++;
 	
 
   	int pb = width*height;
   	
   	//int piece;
	  uint8_t *piece = (uint8_t*)malloc(sizeof(uint8_t) * pb);
	   MPI_Scatter(rgb_image, pb ,MPI_UINT8_T, piece , pb  , MPI_UINT8_T, 0, MPI_COMM_WORLD);
	if(rank==0){

	  // rgb_image = 3000 3001 3002
	  // olmazsa scatteri forun içine koycaz
	 
	  	 for(int i=0;i<pb/num_proc;i++){
	  	 	hist[(*piece++)]++;
		   }
		  // piece-=width*height/num_proc;
		    MPI_Barrier(MPI_COMM_WORLD);
   	}
	else
	{
  	int count=(pb/num_proc)*rank;
	  //MPI_Scatter(rgb_image, pb  , MPI_UINT8_T, piece , pb, MPI_UINT8_T, 0, MPI_COMM_WORLD);
	  	 for(int i=(pb/num_proc)*(rank-1);i<(pb/num_proc)*(rank);i++){
	  	 	hist[(piece[count++])]++;
		   }
		    MPI_Barrier(MPI_COMM_WORLD);
		  
	  // piece-=width*height/num_proc;
	}
	
	
    double size = width * height;	

    
 	
     //cumulative sum for histogram values
    int cumhistogram[256];
	cumhistogram[0] = hist[0];
    for(int i = 1; i < 256; i++)
    {
        cumhistogram[i] = hist[i] + cumhistogram[i-1];
    }    
	
    int alpha[256];
    for(int i = 0; i < 256; i++)
    {
        alpha[i] = round((double)cumhistogram[i] * (255.0/size));
    }
	
		
    // histogram equlized image
    for(int y = 0; y <height ; y++){
        for(int x = 0; x < width; x++){
            rgb_image[y*width + x] = alpha[rgb_image[y*width + x]];
	}
    }
    	
   
}

void seq_histogram_equalizer(uint8_t* rgb_image,int width, int height)
{
	int *hist = (int*)calloc(256,sizeof(int));
	
	for(int i=0; i<height ; i++){
		for(int j=0; j<width; j++){
			// kaç tane 0 kaç tane 255 var countluyor
			// iç kısım degerini dönüyor 
			// historgramda o deger aslında index
			
			hist[rgb_image[i*width + j]]++;

 		}
	}
			
	double size = width * height;
  
	float hist_avg[256];
    for(int i = 0; i < 256; i++)
    {
        hist_avg[i] = (double)hist[i] / size;
    }  
 
     //cumulative sum for histogram values
    int cumhistogram[256];
	cumhistogram[0] = hist[0];
    for(int i = 1; i < 256; i++)
    {
        cumhistogram[i] = hist[i] + cumhistogram[i-1];
    }    
	
    int alpha[256];
    for(int i = 0; i < 256; i++)
    {
        alpha[i] = round((double)cumhistogram[i] * (255.0/size));
    }
			
    // histogram equlized image		
    for(int y = 0; y <height ; y++){
        for(int x = 0; x < width; x++){
            rgb_image[y*width + x] = alpha[rgb_image[y*width + x]];
		}
	}
	
}

