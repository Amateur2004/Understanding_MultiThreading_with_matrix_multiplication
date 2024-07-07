#include<iostream>
#include <fstream>
#include<pthread.h>
#include<cstring>
#include<unistd.h>
#include<chrono>
#include<string.h>
#include<atomic>
#include<vector>

using namespace std;

#define methodSize 20

int c = 0; //counter - shared by all threads
int rowInc = 0;
int n = 0;
int num_threads = 0;
//these values are set in main

//atomic<int> counter_threads_completion_of_tasks(0);
atomic<bool> lock(false);
atomic<bool> exit_check(false);
atomic<int> c_atomic(0);

atomic<bool> lock_CAS(false);
 
atomic_flag lock_TAS = ATOMIC_FLAG_INIT;

vector<bool> waiting;

vector<pthread_t> threads;

struct threadAtt{
    int i; //index of thread
    int n;
    int k;
    int **inpMatrix;
    int **resMatrix;
    char *mem; //mutual exlusion method 
};

struct threadAtt* createStructthreadAtt(int i,int n,int k,int **inpMatrix, int **resMatrix,char *method){
    struct threadAtt* tAttr = new struct threadAtt;
    tAttr->i = i;
    tAttr->n = n;
    tAttr->k = k;
    tAttr->inpMatrix = inpMatrix;
    tAttr->resMatrix = resMatrix;
    tAttr->mem = method;
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

bool testAndSet(atomic<bool> *lock_loc){ //lock_loc = lock location
    bool prevLockVal = *lock_loc;
    *lock_loc = true;
    return prevLockVal;
}

bool compareAndSwap(atomic<bool> *lock_loc,bool expected,bool newValue){ //lock_loc = lock location
    bool prevLockVal = *lock_loc;
    lock_loc->compare_exchange_weak(prevLockVal, newValue);
    return prevLockVal; 
}

void* tFunction(void* tAttrb){  
    do{
        struct threadAtt* tAttr = (struct threadAtt*)tAttrb;
        if( strcmp(tAttr->mem,"TAS") == 0 ){

            while(lock_TAS.test_and_set()){
            }; //loop for waiting threads
            /* cs */
            int rowStart = c;
            int numOfRows = rowInc;
            if(c>=tAttr->n){
                numOfRows = 0;
                lock_TAS.clear();
                pthread_exit(NULL);
            } 
            if( ((n-1)-c+1) < rowInc){
                numOfRows = n-c;
            }
            c+=numOfRows;
            /* cs */

            lock_TAS.clear();
            
            for(int rowElement_i = rowStart;rowElement_i< (rowStart+numOfRows) && rowElement_i<tAttr->n;rowElement_i++){
                for(int col_i = 0;col_i<tAttr->n;col_i++){
                    cellInResMatrix(tAttr->n,tAttr->inpMatrix,tAttr->resMatrix,rowElement_i,col_i);
                }
            }

        }

    
        else if( strcmp(tAttr->mem,"CAS") == 0 ){

            while(compareAndSwap(&lock_CAS,false,true)){
            };

            /* cs */
            int rowStart = c; //made it local so that the multiplication can be done without any worries of changing c rowstart
            int numOfRows = rowInc;
            if(c>=tAttr->n){ 
                numOfRows = 0;
                lock_CAS = false;
                pthread_exit(NULL);
            } 
            if( ((n-1)-c+1) < rowInc){
                numOfRows = n-c;
            }
            c+=numOfRows;
            /* cs */

            lock_CAS = false;
            
            for(int rowElement_i = rowStart;rowElement_i< (rowStart+numOfRows) && rowElement_i<tAttr->n;rowElement_i++){
                for(int col_i = 0;col_i<tAttr->n;col_i++){
                    cellInResMatrix(tAttr->n,tAttr->inpMatrix,tAttr->resMatrix,rowElement_i,col_i);
                }
            }

        }
    
        else if( strcmp(tAttr->mem,"bCAS") == 0 ){

            waiting[tAttr->i] = true;
            bool key = true;
            while( waiting[tAttr->i] && key == true ){
                key = compareAndSwap(&lock,false,true);
            };
            waiting[tAttr->i] = false;

            /* cs */
            int rowStart = c;
            int numOfRows = rowInc;
            if(c>=tAttr->n){ 
                numOfRows = 0;
                lock = false;
                pthread_exit(NULL);
            } 
            if( ((n-1)-c+1) < rowInc){
                numOfRows = n-c;
            }
            c+=numOfRows;

            int nextThread = ((tAttr->i)+1)%(tAttr->k);
            while( !waiting[nextThread] && nextThread!= tAttr->i ){
                nextThread = (nextThread+1)%(tAttr->k);
            }

            if(nextThread != (tAttr->i)){
                waiting[nextThread] = false;
            }
            else{
                lock = false;
            }
            
            /* cs */
            
            for(int rowElement_i = rowStart;rowElement_i< (rowStart+numOfRows) && rowElement_i<tAttr->n;rowElement_i++){
                for(int col_i = 0;col_i<tAttr->n;col_i++){
                    cellInResMatrix(tAttr->n,tAttr->inpMatrix,tAttr->resMatrix,rowElement_i,col_i);
                }
            }

        }

        else if( strcmp(tAttr->mem,"atomic") == 0 ){

            int rowStart = c_atomic;
            int numOfRows = rowInc;
            if(c_atomic>=tAttr->n){ 
                numOfRows = 0;
                pthread_exit(NULL);
            } 
            if( ((n-1)-c_atomic+1) < rowInc){
                numOfRows = n-c;
            }
            
            c_atomic+=numOfRows;

            for(int rowElement_i = rowStart;rowElement_i< (rowStart+numOfRows) && rowElement_i<tAttr->n;rowElement_i++){
                for(int col_i = 0;col_i<tAttr->n;col_i++){
                    cellInResMatrix(tAttr->n,tAttr->inpMatrix,tAttr->resMatrix,rowElement_i,col_i);
                }
            }

        }
    
    }while(true);
    
        
}


//terminal based input
int main(int argc, char* argv[]){

    
    //reading input file
    std::ifstream inputFile;
    inputFile.open(argv[1]);
    //inputFile.open("inp.txt");
    int k; //n,rowInc are global variables as they are constant throught the program there is no issue of synchronisation with it
    inputFile >> n >> k >> rowInc;

    num_threads = (n%rowInc == 0) ? n/rowInc : (n/rowInc +1);
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
    struct threadAtt **arrOfThreadAttr = new struct threadAtt* [n];

    threads.resize(k);
    waiting.resize(k);

    char method[methodSize];
    strcpy(method,argv[2]);
    //cout << method << endl;

    for(int thread_i=0;thread_i<k;thread_i++){
        arrOfThreadAttr[thread_i] = createStructthreadAtt(thread_i,n,k,matrix,resultantMatrix,method);
        void *threadAtt = arrOfThreadAttr[thread_i];
        waiting[thread_i] = false;
        int validThread = pthread_create(&threads[thread_i],NULL,tFunction,threadAtt); // fn and its attribute

         //initialise with false
        if(validThread != 0){
            thread_i--;
            continue;
        }
    }

    int count_threads = 0;
    for(int thread_i=0;thread_i<k;thread_i++){
       if(threads[thread_i]!=0){
        count_threads++;
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
    outputFile << "The resulting square matrix produced by each mutual exclusion method "<< argv[2] << ": " << endl;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            outputFile << resultantMatrix[i][j] << " ";
        }
        outputFile << '\n';
    }
    outputFile << "Time taken: " << duration.count() << " microseconds" << std::endl;
    outputFile.close();
    
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