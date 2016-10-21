#ifndef __FIFO__H
#define __FIFO_H
#include <list>
#include <pthread.h>
class ScopeLock
{
        public:
                ScopeLock(pthread_mutex_t *mutex) : mutex_(mutex)
                {
                        pthread_mutex_lock(mutex_);
                }
                ~ScopeLock()
                {
                        pthread_mutex_unlock(mutex_);
                }
        private:
                pthread_mutex_t *mutex_;
};
template <class T>
class Fifo
{
        public:
                Fifo(int max_size = 0);
                ~Fifo();
                int size()
                {
                        ScopeLock mutex_lock(&mutex_);
                        return list_.size();
                }
                void enqueue(const T& job);
                void dequeue(T *job);
        private:
                int max_size_;
                std::list<T> list_;
                pthread_mutex_t mutex_;
                pthread_cond_t non_empty_cond_;
                pthread_cond_t has_space_cond_;
};

template<class T>
Fifo<T>::Fifo(int max_size) : max_size_(max_size) {
        pthread_mutex_init(&mutex_, 0);
        pthread_cond_init(&non_empty_cond_, 0);
        pthread_cond_init(&has_space_cond_, 0);
}
template<class T>
Fifo<T>::~Fifo() {
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&non_empty_cond_);
        pthread_cond_destroy(&has_space_cond_);
}
template<class T>
void Fifo<T>::enqueue(const T& job) {
        ScopeLock mutexlock(&mutex_);
        while (true) {
                if (!max_size_ || list_.size() < max_size_) {
                        list_.push_back(job);
                        break;
                }
                else {
                        pthread_cond_wait(&has_space_cond_, &mutex_);
                }
        }
        pthread_cond_signal(&non_empty_cond_);
}
template<class T>
void Fifo<T>::dequeue(T *job) {
        ScopeLock mutexlock(&mutex_);
        while (true) {
                if (list_.size() > 0) {
                        *job = list_.font();
                        list_.pop_font();
                        if (max_size_ && list_.size() < max_size_) {
                                pthread_cond_signal(&has_space_cond_);
                        }
                        break;
                }
                else {
                        pthread_cond_wait(&non_empty_cond_, &mutex_);
                }
        }
}
#endif //fifo.h
