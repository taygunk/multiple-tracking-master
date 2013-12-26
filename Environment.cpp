#include "Environment.h"
//Utility Functions
float** create2dKernel(int xSize, int ySize)
{
	int i;
	float **kernel = new float*[xSize]; 
	for(i = 0; i < xSize; i++){
		kernel[i] = new float[ySize];
	}
	return kernel;
}
int** create2dKernel(int xSize, int ySize, int scalar)
{
	int i;
	int **kernel = new int*[xSize]; 
	for(i = 0; i < xSize; i++){
		kernel[i] = new int[ySize];
	}
	for(int i = 0; i < xSize; i++ ){
		for(int j = 0; j < ySize ; j++){
			kernel[i][j] = scalar;
		}
	}
	return kernel;
}
bool** create2dKernelBool(int xSize, int ySize, bool scalar)
{
	int i;
	bool **kernel = new bool*[xSize]; 
	for(i = 0; i < xSize; i++){
		kernel[i] = new bool[ySize];
	}
	for(int i = 0; i < xSize; i++ ){
		for(int j = 0; j < ySize ; j++){
			kernel[i][j] = scalar;
		}
	}
	return kernel;
}

void delete2dKernel(float** kernel, int xSize)
{
	int i;
	for(i = 0; i < xSize; i++){
		delete[] kernel[i];
	}
	delete[] kernel;
}
int*** create3dKernel(int numOfKernel, int xSize, int ySize)
{
	int ***kernel;
	int i, j, k;
	// Allocate memory
	kernel = new int**[numOfKernel];
	for (i = 0; i < numOfKernel; ++i) {
		kernel[i] = new int*[xSize];
		for (j = 0; j < xSize; ++j)
			kernel[i][j] = new int[ySize];
	}
	return kernel;
}

int**** create4dKernel(int numOfKernel, int xSize, int ySize, int lastSize)
{
	int ****kernel;
	int i, j, k;
	// Allocate memory
	kernel = new int***[numOfKernel];
	for (i = 0; i < numOfKernel; ++i) {
		kernel[i] = new int**[xSize];
		for (j = 0; j < xSize; ++j){
			kernel[i][j] = new int*[ySize];
			for(k = 0; k < ySize ; k++){
				kernel[i][j][k] = new int[lastSize];
			}
		}
	}
	return kernel;
}

float***** create5dKernel(int numOfKernel, int xSize, int ySize, int lastSize, int lastlastSize)
{
	float *****kernel;
	int i, j, k, y;
	// Allocate memory
	kernel = new float****[numOfKernel];
	for (i = 0; i < numOfKernel; ++i) {
		kernel[i] = new float***[xSize];
		for (j = 0; j < xSize; ++j){
			kernel[i][j] = new float**[ySize];
			for(k = 0; k < ySize ; k++){
				kernel[i][j][k] = new float*[lastSize];
				for(y = 0; y < lastSize ; y++){
					kernel[i][j][k][y] = new float[lastlastSize];
				}
			}
		}
	}
	return kernel;
}

void delete3dKernel(float*** kernel, int numOfKernel, int xSize)
{
	int i,j;
	for (i = 0; i < numOfKernel; ++i) {
		for (j = 0; j < xSize; ++j)
			delete [] kernel[i][j];
		delete [] kernel[i];
	}
	delete [] kernel;
}

float round(float d)
{
  return floor(d + 0.5);
}