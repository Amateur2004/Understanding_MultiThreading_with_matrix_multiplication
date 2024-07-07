#include<iostream>
#include <fstream>
#include<pthread.h>
#include<cstring>
#include<unistd.h>
#include<chrono>
#include<string.h>

#define methodSize 20

struct threadAtt{
    char *method;
    int i; //index of thread
    int n;
    int k;
    int **inpMatrix;
    int **resMatrix;
};

struct threadAtt* createStructthreadAtt(char *method, int i, int n, int k, int **inpMatrix, int **resMatrix){
    struct threadAtt* tAttr = new struct threadAtt;
    tAttr->i = i;
    tAttr->n = n;
    tAttr->k = k;
    tAttr->method = method;
    tAttr->inpMatrix = inpMatrix;
    tAttr->resMatrix = resMatrix;
    return tAttr;
}

void printMatrix(int row,int col, int **matrix){
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            std::cout << matrix[i][j] << " ";
        }
        std::cout << '\n';
    }
}

void matrixMultiplication(int n, int **matrix,int **resultantMatrix){
    for(int inpMatrix_row=0;inpMatrix_row<n;inpMatrix_row++){
        for(int inpMatrix_col=0;inpMatrix_col<n;inpMatrix_col++){
            resultantMatrix[inpMatrix_row][inpMatrix_col] = 0;
            for(int k = 0;k<n;k++){
                resultantMatrix[inpMatrix_row][inpMatrix_col] += matrix[inpMatrix_row][k]*matrix[k][inpMatrix_col];
            }
        }
    }
}

void cellInResMatrix(int n,int **matrix,int **resultantMatrix,int rowNum,int colNum){
    resultantMatrix[rowNum][colNum] = 0;
    for(int k = 0;k<n;k++){
        resultantMatrix[rowNum][colNum] += matrix[rowNum][k]*matrix[k][colNum];
    }
}

void* tFunction(void* tAttrb){
    struct threadAtt* tAttr = (struct threadAtt*)tAttrb;
    if(strcmp(tAttr->method,"mixed") == 0){
        printf("thread created - %d\n",tAttr->i);
        std::cout << "mixed" << std::endl;
        for(int rowElement_i = tAttr->i;rowElement_i<tAttr->n;rowElement_i+=tAttr->k){
            printf("thread - %d\n",tAttr->i);
            for(int col_i = 0;col_i<tAttr->n;col_i++){
                cellInResMatrix(tAttr->n,tAttr->inpMatrix,tAttr->resMatrix,rowElement_i,col_i);
            }
        }
    }
    else if(strcmp(tAttr->method,"chunk") == 0){
        //std::cout << "chunk" << std::endl;
        int rowStart = 0;int numOfRows = 0;
        int rem = tAttr->n%tAttr->k;
        int p = tAttr->n/tAttr->k;
        if(tAttr->i < rem){
            rowStart = (tAttr->i)*(p+1);
            numOfRows = p+1;
        }
        else{
            rowStart = (rem)*(p+1) + ((tAttr->i-rem)*p);
            numOfRows = p;
        }
        
        for(int rowElement_i = rowStart;rowElement_i< (rowStart+numOfRows) && rowElement_i<tAttr->n;rowElement_i++){
            for(int col_i = 0;col_i<tAttr->n;col_i++){
                cellInResMatrix(tAttr->n,tAttr->inpMatrix,tAttr->resMatrix,rowElement_i,col_i);
            }
        }
    }
    //return NULL;
    pthread_exit(NULL);
}

//terminal based input
int main(int argc, char* argv[]){
    //reading input file
    std::ifstream inputFile;
    inputFile.open(argv[1]);

    //reading method type
    char Method[methodSize];
    strcpy(Method,argv[2]); 
    int n,k,count = 0;
    while(inputFile >> n >> k){
        count++;
        if(count == 1){
            break;
        }
    }

    std :: cout << n << " " << k << std :: endl;

    //these need not be continuous the pointers of each row are continuous and also int in each row are continuous
    //allocating memory to input matrix
    int **matrix = new int*[n];
    for(int i=0;i<n;i++){
        matrix[i] = new int[n];
    }
    while(!inputFile.eof()){
        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                inputFile >> matrix[i][j];
            }
        }
    }

    inputFile.close();

    //allocating memory to resultant matrix
    int **resultantMatrix = new int*[n];
    for(int i=0;i<n;i++){
        resultantMatrix[i] = new int[n];
    }

    auto start = std::chrono::high_resolution_clock::now();

    //creating threads
    struct threadAtt **arrOfThreadAttr = new struct threadAtt* [k];
    pthread_t threads[k];
    for(int thread_i=0;thread_i<k;thread_i++){
        arrOfThreadAttr[thread_i] = createStructthreadAtt(Method,thread_i,n,k,matrix,resultantMatrix);
        void *threadAtt = arrOfThreadAttr[thread_i];
        int validThread = pthread_create(&threads[thread_i],NULL,tFunction,threadAtt); // fn and its attribute
        if(validThread != 0){
            std::cout << "Thread is not created\nCreating other\n";
            thread_i--;
            continue;
        }
    }

    for(int thread_i=0;thread_i<k;thread_i++){
        pthread_join(threads[thread_i],NULL);
    }

    auto end = std::chrono::high_resolution_clock::now();
    
    //calculating the duration
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Time taken using threads: " << duration.count() << " microseconds" << std::endl;
    
    //writing to output file
    char outFile[20];
    sprintf(outFile,"outputFile_%s.txt",argv[2]);
    std::ofstream outputFile(outFile);
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            outputFile << resultantMatrix[i][j] << " ";
        }
        outputFile << '\n';
    }
    outputFile << "Time taken: " << duration.count() << " microseconds" << std::endl;
    outputFile.close();

    start = std::chrono::high_resolution_clock::now();
    matrixMultiplication(n,matrix,resultantMatrix);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time taken in matrix multiplication: " << duration.count() << " microseconds" << std::endl;

    //freeing memory allocated to matrix
    for (int i=0; i<n; i++) {
        delete[] matrix[i];
        delete[] resultantMatrix[i];
        delete[] arrOfThreadAttr[i];
    }
    delete[] matrix;
    delete[] resultantMatrix;
    delete[] arrOfThreadAttr;

return 0;
}