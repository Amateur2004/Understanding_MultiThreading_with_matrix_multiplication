Input: 
Give n,k,c,bt,matrix values in the input file and give the file name, method and experiment number as command line arguments
for implementation of Mixed method use mixed with same case as mentioned i.e all in small case
similarly for Chunk method use Chunk

Compilation:
g++  <filename>.cpp 

To run, use:
./a.out <input_fileName> <method_name> <experiment_number>
    Ex:
    ./a.out input.txt chunk 1
    ./a.out inputFile.txt mixed 2

Output: 
Output file named outputFile_<methodName>.txt is created which has the output matrix row wise in each line followed by time taken to complete the task in microseconds and also time taken by each thread and average time.
    Terminal output:
    I am printing time taken for calculation of total time,time per bounded and normal threads and the number of cores to which bounded threads are alloted.
	Ex:
    rescores = 6
    Time taken using threads: 3393580 microseconds
	Average time per bounded thread: 2918781 microseconds
    Average time per normal thread:: 2145319 microseconds

