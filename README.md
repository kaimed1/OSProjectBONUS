# Operating Systems Project 5 Bonus  
**CS3113 – Introduction to Operating Systems – Spring 2025**

## Project Description  
This project implements a parallel array summation using a hierarchy of processes arranged as a complete binary tree. Each node in the tree is a separate Unix process, and inter-process communication (IPC) is handled through pipes. This approach simulates structured computation and controlled termination without using shared memory.

Each leaf process computes the sum of a portion of the input array, sends it to its parent process through a pipe, and terminates after receiving a shutdown signal. Internal processes aggregate the results from their two child processes and pass the computed sum up the tree. The root process computes the final sum of the entire array.

## Features  
- **Process Tree Structure**: Each node in the binary tree is a separate process created via `fork()`.  
- **Inter-Process Communication**: Child processes send results to parent processes using pipes.  
- **Chunked Array Summation**: The input array is divided among the leaf processes.  
- **Controlled Termination**: Processes only terminate after receiving a termination signal through a dedicated pipe.  
- **Logging**: Each process prints its PID, index, level, position, computed sum, and termination status.

## Files Included  
- `CS3113_ProjectBonus.cpp`: Source code implementing process-based summation using pipes.  
- `BonusDetails.pdf`: Project specification including the bonus project description.  
- `README.md`: This documentation.

## Input Format  
The program reads input through standard input redirection. The input format is:
1. An integer representing the height of the binary tree \( H \)  
2. An integer representing the number of array elements \( M \)  
3. \( M \) integers representing the elements of the input array

If the number of elements is not divisible by the number of leaf processes, the array is padded with zeros.

## Compilation and Execution  
To compile and run the program:
```bash
g++ CS3113_ProjectBonus.cpp -o os_project5_bonus
./os_project5_bonus < input.txt
