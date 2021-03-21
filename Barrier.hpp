
#ifndef HW3_BARRIER_H
#define HW3_BARRIER_H
#include "Headers.hpp"

class Barrier{
private:
    int working;
    pthread_mutex_t m;
public:
    Barrier(){
        working = 0;
        pthread_mutex_init(&m,NULL);
    }
    void increase(){
        pthread_mutex_lock(&m);
        working++;
        pthread_mutex_unlock(&m);
    }
    void decrease(){
        pthread_mutex_lock(&m);
        working--;
        pthread_mutex_unlock(&m);
    }
    void wait() const{
        while(working!=0){
            sched_yield();
        }
    }
};

#endif //HW3_BARRIER_H
