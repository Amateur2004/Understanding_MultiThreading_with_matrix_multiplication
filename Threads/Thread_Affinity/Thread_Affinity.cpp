#include<iostream>
#include <fstream>
#include<pthread.h>
#include<cstring>
#include<unistd.h>
#include<chrono>
#include<string.h>
#include<vector>

#define methodSize 20

struct threadParameters{
    char *method;
    int i; //index of thread
    int n;
    int k;
    int **inpMatrix;
    int **resMatrix;
};

std::vector<std::chrono::microseconds> time_per_thread;

struct threadParameters* createStructthreadPara(char *method, int i, int n, int k, int **inpMatrix, int **resMatrix){
    struct threadParameters* tPara = new struct threadParameters;
    tPara->i = i;
    tPara->n = n;
    tPara->k = k;
    tPara->method = method;
    tPara->inpMatrix = inpMatrix;
    tPara->resMatrix = resMatrix;
    return tPara;
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
    auto start = std::chrono::high_resolution_clock::now();

    struct threadParameters* tPara = (struct threadParameters*)tAttrb;
    if(strcmp(tPara->method,"mixed") == 0){
        for(int rowElement_i = tPara->i;rowElement_i<tPara->n;rowElement_i+=tPara->k){
            for(int col_i = 0;col_i<tPara->n;col_i++){
                cellInResMatrix(tPara->n,tPara->inpMatrix,tPara->resMatrix,rowElement_i,col_i);
            }
        }
        //printf("in mixed\n");
    }
    else if(strcmp(tPara->method,"chunk") == 0){
        int rowStart = 0;int numOfRows = 0;
        int rem = tPara->n%tPara->k;
        int p = tPara->n/tPara->k;
        if(tPara->i < rem){
            rowStart = (tPara->i)*(p+1);
            numOfRows = p+1;
        }
        else{
            rowStart = (rem)*(p+1) + ((tPara->i-rem)*p);
            numOfRows = p;
        }
        
        for(int rowElement_i = rowStart;rowElement_i< (rowStart+numOfRows) && rowElement_i<tPara->n;rowElement_i++){
            for(int col_i = 0;col_i<tPara->n;col_i++){
                cellInResMatrix(tPara->n,tPara->inpMatrix,tPara->resMatrix,rowElement_i,col_i);
            }
        }
        //printf("in chunk\n");
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    time_per_thread[tPara->i] = duration;

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


    //other input variables
    int n,k,c,bt;
    inputFile >> n >> k >> c >> bt ;

    std :: cout << n << " " << k << " "<< c<<" " << bt << " "<<std :: endl;

    //these need not be continuous the pointers of each row are continuous and also int in each row are continuous
 
    //allocating memory to input and resultant matrices
    int **matrix = new int*[n];
    int **resultantMatrix = new int*[n];
    for(int i=0;i<n;i++){
        matrix[i] = new int[n];
        resultantMatrix[i] = new int[n];
    }
    while(!inputFile.eof()){
        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                inputFile >> matrix[i][j];
            }
        }
    }

    inputFile.close();

    //resizing the vector as per the number of threads
    time_per_thread.resize(k);
    auto start = std::chrono::high_resolution_clock::now();

    int rowStart = 0;int numOfRows = 0;
    int b = k/c;

    int resCores = 0; //no_of_cores_for_restricted_threads
    if(strcmp(argv[3],"2") == 0){
        if(bt!=0){
            resCores = c/2;
        }
        else{
            resCores = 0;
        }
    }
    else{
        if(b==0 && bt!=0){
        resCores = 1;
        }
        else if(b==0){
            resCores = 0;
        }
        else{
            resCores = bt/b; 
            if(resCores > c){
                resCores = c;
            }
        }
    }
    printf("rescores = %d\n",resCores);
    
    pthread_attr_t *arrOfThreadAttr = new pthread_attr_t[resCores];
    struct threadParameters **arrOfThreadParameters = new struct threadParameters* [n];
    pthread_t *threads = new pthread_t[k];
    cpu_set_t cpus; //The  cpu_set_t data structure represents a set of CPUs.
    
    
    //creating array of attributes related to setting affinity for thread
    for(int core_i=0;core_i<resCores;core_i++){
        CPU_ZERO(&cpus); //Clears set, so that it contains no CPUs
        CPU_SET(core_i, &cpus); //Add CPU cpu to set
        pthread_attr_init(&arrOfThreadAttr[core_i]);
        pthread_attr_setaffinity_np(&arrOfThreadAttr[core_i],sizeof(cpu_set_t), &cpus);
    }
    
    //creating threads

    //to distribute equally but not continuously
    for(int bt_i=0;bt_i<bt;bt_i++){
        arrOfThreadParameters[bt_i] = createStructthreadPara(Method,bt_i,n,k,matrix,resultantMatrix);
        void *threadPara = arrOfThreadParameters[bt_i];
        int core_i = bt_i%resCores;
        CPU_ZERO(&cpus);
        CPU_SET(core_i,&cpus);
        pthread_attr_setaffinity_np(&arrOfThreadAttr[core_i],sizeof(cpu_set_t), &cpus);
        pthread_create(&threads[bt_i],&arrOfThreadAttr[core_i],tFunction,threadPara);
    }

    //creating threads without specifying core i.e by fixing thread attribute as NULL
    for(int thread_i=bt;thread_i<k;thread_i++){
        arrOfThreadParameters[thread_i] = createStructthreadPara(Method,thread_i,n,k,matrix,resultantMatrix);
        void *threadPara = arrOfThreadParameters[thread_i];
        int validThread = pthread_create(&threads[thread_i],NULL,tFunction,threadPara); // fn and its attribute
        if(validThread != 0){
            std::cout << "Thread is not created\nCreating other\n";
            thread_i--;
            continue;
        }
    }

    //joining the threads
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
    
    int thread_index = 0;
    std::chrono::microseconds total_duration_bounded_threads(0);
    std::chrono::microseconds total_duration_normal_threads(0);

    for( std::chrono::microseconds duration : time_per_thread ){
        outputFile << "Time taken for " << thread_index <<" : " << duration.count() << " microseconds" << std::endl;
        if(thread_index<bt){
            total_duration_bounded_threads += time_per_thread[thread_index];
        }
        else{
            total_duration_normal_threads += time_per_thread[thread_index];
        }
        thread_index++;
    }
    std::chrono::microseconds avg_duration_bounded_threads(0);
    std::chrono::microseconds avg_duration_normal_threads(0);

    if(bt!=0){
        avg_duration_bounded_threads = total_duration_bounded_threads/bt;
    }
    if(bt!=k){
        avg_duration_normal_threads = total_duration_normal_threads/(k-bt);
    }
    
    outputFile << "Average time per bounded thread: " << avg_duration_bounded_threads.count() << " microseconds" << std::endl;
    outputFile << "Average time per normal thread:: " << avg_duration_normal_threads.count() << " microseconds" << std::endl;

    outputFile.close();
    std::cout << "Average time per bounded thread: " << avg_duration_bounded_threads.count() << " microseconds" << std::endl;
    std::cout << "Average time per normal thread:: " << avg_duration_normal_threads.count() << " microseconds" << std::endl;

    //freeing memory allocated to matrix
    
    for (int i=0; i<n; i++) {
        delete[] matrix[i];
        delete[] resultantMatrix[i];
        delete[] arrOfThreadParameters[i]; //the memory allocated to structures is also freed
    }
    
    delete[] matrix;
    delete[] resultantMatrix;
    delete[] arrOfThreadParameters;
    delete[] threads;
    delete[] arrOfThreadAttr;
    
return 0;
}