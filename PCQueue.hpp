#ifndef _QUEUEL_H
#define _QUEUEL_H

#include "Semaphore.hpp"

// Single Producer - Multiple Consumer queue
template <typename T>
class PCQueue
{

public:
    PCQueue() : sem_reader(1) , sem_writer(1), consumers_count(0) {};
	// Blocks while queue is empty. When queue holds items, allows for a single
	// thread to enter and remove an item from the front of the queue and return it. 
	// Assumes multiple consumers.
	T pop() { //consumer
	    T temp;
        sem_reader.down();
        consumers_count++;
        while(q.empty()){
            pthread_yield();
        }
        if (consumers_count == 1)
            sem_writer.down();
        temp = q.front();
        q.pop();
        //std::cout << "number popped from queue: "<< temp << std::endl;
        consumers_count--;
        if (consumers_count==0)
            sem_writer.up();
        sem_reader.up();
        return temp;
	};

	// Allows for producer to enter with *minimal delay* and push items to back of the queue.
	// Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.  
	// Assumes single producer 
	void push(const T& item) { //producer
        sem_writer.down();
        //std::cout << "number pushed to queue: "<< item << std::endl;
        q.push(item);
        sem_writer.up();
	};


private:
	// Add your class memebers here
	bool writer_waiting;
	queue<T> q;
	Semaphore sem_reader;
	Semaphore sem_writer;
	int consumers_count;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif