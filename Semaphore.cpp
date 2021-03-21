#include "Semaphore.hpp"

Semaphore::Semaphore() {
    this->counter=0;
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
}

Semaphore::Semaphore(unsigned int val) {
    this->counter=val;
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
}

void Semaphore::up() {
    pthread_mutex_lock(&mutex);
    counter++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void Semaphore::down() {
    pthread_mutex_lock(&mutex);
    while(counter == 0) {
        pthread_cond_wait(&cond,&mutex);
    }
    counter--;
    pthread_mutex_unlock(&mutex);
}




