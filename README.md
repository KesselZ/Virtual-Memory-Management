# Virtual-Memory-Management

In this project, I will simulate the operation of an Operating System’s Virtual Memory Manager which maps the virtual address spaces of multiple processes onto physical frames using page table translation. 

To run it, please use 
1.  make
2.  ./mmu -[f/r/c/e/a/w][frame number(1-32)] -ac –oOPFS ./inputs/in[1...9] rfile

A example of step2: ./mmu -f4 -ac –oOPFS ./inputs/in1 rfile
