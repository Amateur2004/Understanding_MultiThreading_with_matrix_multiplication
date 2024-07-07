Input: 
Give n,k,matrix values in the input file and give the file name and method as command line arguments
for implementation of Mixed method use mixed with same case as mentioned i.e all in small case
similarly for Chunk method use Chunk.

Compilation:
g++ <filename>.cpp 

To run use:
./a.out <input_fileName> <method_name>
    Ex:
    ./a.out input.txt chunk
    ./a.out inputFile.txt mixed

Output: 
Output file named outputFile_<methodName>.txt is created which has the output matrix row wise in each line followed by time taken to complete the task in microseconds.
    Terminal output:
    I am printing time taken for calculation using threads and matrix multiplication to compare efficiency
	Ex:
	Time taken using threads: 25577781 microseconds
	Time taken in matrix multiplication: 99069017 microseconds

