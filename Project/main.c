/**
 *
 * CENG342 Project-1
 *
 * Histogram equalization 
 *
 * Usage:  main <input.jpg> <output.jpg> 
 *
 * @group_id 01
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
#define RANGE 256

#include "stb_image.h"
#include "stb_image_write.h"
#define CHANNEL_NUM 1

void seq_histogram_equalizer(uint8_t* rgb_image,int width, int height);
void par_histogram_equalizer(uint8_t* rgb_image,int *hist,int width, int height,int rank,int size);

int main(int argc,char* argv[]) 
{	
    int rank,size;
    int *hist = (int*)calloc(RANGE,sizeof(int));
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
	int width, height, bpp;
	// Reading the image in grey colors
    uint8_t* rgb_image = stbi_load(argv[2], &width, &height, &bpp, CHANNEL_NUM);
	
    printf("Width: %d  Height: %d \n",width,height);
	printf("Input: %s , Output: %s  \n",argv[2],argv[3]);
	
	// start the timer
	double time1= MPI_Wtime();	
	
	//seq_histogram_equalizer(rgb_image,width, height);
	par_histogram_equalizer(rgb_image,hist,width, height,rank,size);
    
	double time2= MPI_Wtime();	
	printf("Elapsed time: %lf \n",time2-time1);	
	
	// Storing the image 
    stbi_write_jpg(argv[3], width, height, CHANNEL_NUM, rgb_image, 100);
    stbi_image_free(rgb_image);

	MPI_Finalize();
    return 0;
}

// You must only modify this function
void par_histogram_equalizer(uint8_t* rgb_image,int *hist,int width, int height,int rank,int size)
{ 
     int *local_hist = (int*)calloc(RANGE,sizeof(int)); 
     uint8_t *local_image = (uint8_t*)calloc(RANGE,sizeof(uint8_t)); 
     double size_new = width * height;
     int y=0;
//     int local_image_width = round(width/num_proc); // This will  be the size of piece of the imaage that each process will get
    /*

         _ _ _ _ _ _ _ _
        |P0| P1 | P3 |P4| } This is one row of 1k height.rgb_image[i].It will divided equally between processes 
        |               |
    5K  |               |
        |               |
        |_ _ _ _ _ _ _ _|

                7K
    */


	if(rank==0){
   // for(int i=0; i<height ; i++){ // row-wise  iteration



        for(int i =1;i<size;i++)MPI_Send(
	 rgb_image/*what we send*/
	,height*width /*size of variale we send*/
	,MPI_UINT8_T
	,i/*who we send*/
	,123/*tag*/
	,MPI_COMM_WORLD);
	//MPI_Scatter(rgb_image, size/num_proc ,MPI_INT, piece , size/num_proc, MPI_INT, 0, MPI_COMM_WORLD);

}
	else
	{
	//MPI_Scatter(rgb_image, size/num_proc ,MPI_INT, piece , size/num_proc, MPI_INT, 0, MPI_COMM_WORLD);
	
	uint8_t *temp;
	//temp= (uint8_t*)malloc((height*width)/size*sizeof(uint8_t));
	for(int q=0;q<size-1;q++)
	{
	MPI_Recv(
	&temp/*where we recive data*/
	,height*width/*Max number of element we recive in temp*/
	,MPI_UINT8_T
	,0
	,123
	,MPI_COMM_WORLD
	,MPI_STATUS_IGNORE/*MPI sStatus*/);
	for(int w=0;w<height*width/3;w++)local_hist[temp[w]]++;
		}
	}
	 // whole image is consumed
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Reduce(    //Merge the local histograms 
        local_hist,
        hist,
        RANGE,
        MPI_INT,
        MPI_SUM,
        0,
        MPI_COMM_WORLD);

    int cumhistogram[RANGE];
	cumhistogram[0] = hist[0];
    for(int i = 1; i < RANGE; i++)
    {
        cumhistogram[i] = hist[i] + cumhistogram[i-1];
    }    
	
    int alpha[RANGE];
    for(int i = 0; i < RANGE; i++)
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




