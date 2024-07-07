Input: 
Give n,k,rowInc,matrix values in the input file and give the input file name and method as command line arguments
for implementation of bounded CAS method use bCAS with same case as mentioned i.e all in small case

similarly please check the appropriate terminal commands below
for various mutual exclusion methods use the following with same case
    TAS use TAS
    CAS use CAS
    Bounded CAS use bCAS
    Atomic use atomic

Compilation:
g++  <filename>.cpp -pthread

To run, use:
./a.out <input_fileName> <method_name> 
    Ex:
    ./a.out input.txt bCAS
    ./a.out inputFile.txt atomic
    ./a.out inp.txt TAS
    ./a.out inputs.txt CAS

Output: 
Output file named outputFile_<methodName>.txt is created which has the output matrix row wise in each line followed by time taken to complete the task in microseconds.
    Terminal output:
    I am printing time taken for calculation of total time
    Ex:
    Time taken using threads: 3393580 microseconds


