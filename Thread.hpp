#ifndef __THREAD_H
#define __THREAD_H

#include "Headers.hpp"
#include "PCQueue.hpp"
#include "Barrier.hpp"

typedef struct Task_t{
    uint start_row;
    uint end_row;
    uint field_width;
    uint field_height;
    uint phase;
    int** field;
    int** tempfield;

}Task;

class Thread
{
public:

	Thread(uint thread_id)
	{
		// Only places thread_id
		m_thread_id = thread_id;
	} 
	virtual ~Thread() {} // Does nothing 

	/** Returns true if the thread was successfully started, false if there was an error starting the thread */
	// Creates the internal thread via pthread_create 
	bool start()
	{
        if(pthread_create(&m_thread,nullptr,entry_func, this)!=0)
            return false;
        else
            return true;
	}

	/** Will not return until the internal thread has exited. */
	void join()
	{
        pthread_join(this->m_thread, nullptr);
	}

	/** Returns the thread_id **/
	uint thread_id()
	{
		return m_thread_id;
	}
protected:
	// Implement this method in your subclass with the code you want your thread to run. 
	virtual void thread_workload() = 0;
	uint m_thread_id; // A number from 0 -> Number of threads initialized, providing a simple numbering for you to use

private:
	static void * entry_func(void * thread) { ((Thread *)thread)->thread_workload(); return nullptr; }
	pthread_t m_thread;

};

static int live_neighbors_count(int ** field, int width, int height, int i, int j) {
    int counter=0;
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            if(i-1+x>=height || i-1+x<0 || j-1+y>=width || j-1+y<0 || (x==1&&y==1)){
                continue;
            }
            else{
                if(field[i-1+x][j-1+y]>0)
                    counter++;
            }
        }
    }
    return counter;
}
static int sum_of_neighbors(int ** field, int width, int height, int i, int j) {
    int counter=0;
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            if(i-1+x>=height || i-1+x<0 || j-1+y>=width || j-1+y<0){
                continue;
            }
            else{
                counter+=field[i-1+x][j-1+y];
            }
        }
    }

    return counter;
}
static int calc_dom_species(int ** field, int width, int height, int i, int j){
    int colors[7];
    for (int k = 0; k < 7; ++k) {
        colors[k]=0;
    }
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            if(i-1+x>=height || i-1+x<0 || j-1+y>=width || j-1+y<0){
                continue;
            }
            else{
                if(field[i - 1 + x][j - 1 + y]==1)
                        colors[0]+=1;
                if(field[i - 1 + x][j - 1 + y]==2)
                        colors[1]+=2;
                if(field[i - 1 + x][j - 1 + y]==3)
                        colors[2]+=3;
                if(field[i - 1 + x][j - 1 + y]==4)
                        colors[3]+=4;
                if(field[i - 1 + x][j - 1 + y]==5)
                        colors[4]+=5;
                if(field[i - 1 + x][j - 1 + y]==6)
                        colors[5]+=6;
                if(field[i - 1 + x][j - 1 + y]==7)
                        colors[6]+=7;
            }
        }
    }
    int colorindex=0;
    int max=0;
    for (int k = 0; k < 7; ++k) {
        if(colors[k]>max){
            max=colors[k];
            colorindex = k;
        }
    }
    return colorindex+1;
}
class WorkerThread:public Thread{
public:
    WorkerThread(uint thread_id,PCQueue<Task*>* taskqueue, vector<double>* m_tile_hist,
                                                pthread_mutex_t* mutex,Barrier* barrier):Thread(thread_id){
        this->taskqueue = taskqueue;
        this->m_tile_hist = m_tile_hist;
        this->mutex = mutex;
        this->barrier = barrier;
    }
    void phase1(Task* my_task){
        for(int i=my_task->start_row; i<=my_task->end_row;i++) { //phase 1
            for(int j=0;j<my_task->field_width;j++) {
                int live_neighbors = live_neighbors_count(my_task->field,my_task->field_width,
                                                          my_task->field_height,i,j);
                if(live_neighbors == 3) {
                    if(my_task->field[i][j]==0){ //dead
                        my_task->tempfield[i][j] = calc_dom_species(my_task->field,my_task->field_width,
                                                                              my_task->field_height,i,j);
                    }
                    else{ //alive
                        my_task->tempfield[i][j] = my_task->field[i][j];
                    }

                }
                else if(live_neighbors == 2 && my_task->field[i][j] > 0) { //alive
                    my_task->tempfield[i][j] = my_task->field[i][j];
                }

                else { //dead
                    my_task->tempfield[i][j] = 0;
                }
            }
        }
    }
    void phase2(Task* my_task){
        for(int i=my_task->start_row; i<=my_task->end_row;i++) {
                for(int j=0;j<my_task->field_width;j++) {
                    if(my_task->tempfield[i][j]>0){
                        int sum_neighbors = sum_of_neighbors(my_task->tempfield,my_task->field_width,
                                                             my_task->field_height,i,j);
                        int live_neighbors = live_neighbors_count(my_task->tempfield,my_task->field_width,
                                                                  my_task->field_height,i,j);
                        my_task->field[i][j] = (int)round((double)sum_neighbors/(live_neighbors+1));
                    }
                    else{
                        my_task->field[i][j]=0;
                    }
                }
        }
        my_task->phase=1;
    }
    void thread_workload() override{
        while(true) {
            Task *my_task = taskqueue->pop();
            auto tile_start = std::chrono::system_clock::now(); // tile_hist
            if(my_task->start_row==-1){
                return;
            }
            if(my_task->phase==1) {
                phase1(my_task); //do phase 1
            }
            else if(my_task->phase == 2) {
                phase2(my_task); //do phase 2
            }
            auto tile_end = std::chrono::system_clock::now();  // tile_hist
            pthread_mutex_lock(mutex);
            m_tile_hist->push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(tile_end - tile_start).count());
            pthread_mutex_unlock(mutex);
            barrier->decrease();
        }
    }
private:
    PCQueue<Task*>* taskqueue;
    vector<double>* m_tile_hist;
    pthread_mutex_t* mutex;
    Barrier* barrier;
};

#endif
