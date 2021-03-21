# GameofLife
Multi-Threaded Project that is compatible with linux operating system
this project uses a Thread Pool to synchronize between different threads:
A thread pool is a design pattern where a number of threads are created to perform a number of tasks,
usually organized in a Producer-Consumer queue. In real applications, the number of tasks would be
much higher than the number of workers working on them, achieving constant reuse of the threads. As
soon as a thread completes its task, it will request the next task from the queue until all tasks have been
completed. The thread will then sleep until there are new tasks available.

Game of Life
The Game of Life (or simply Life) is not a game in the conventional sense. There are no players, and no
winning or losing. Once the "pieces" are placed in the starting position, the rules determine everything
that happens later.
Life is played on a grid of square cells-like a chess board but extending infinitely in every direction. A cell
can be alive or dead. A live cell is shown by putting a marker on its square. A dead cell is shown by
leaving the square empty. Each cell in the grid has a neighborhood consisting of the eight cells in every
direction including diagonals, each cell belongs to a certain species, and each species is color-coded (see
source code).
The cycle of life in this game is divided into 2 phases, the first phase is where new cells are created and
some cells die :( , and in the second phase the cells get to know their lovely neighborhood and change its
properties according to the neighborhood it is in.

*note: to synchronize between threads we use a semaphore, which we implement using mutex (look source code).
