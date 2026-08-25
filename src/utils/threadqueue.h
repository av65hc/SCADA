#ifndef THREADQUEUE_H
#define THREADQUEUE_H

#include <QMutexLocker>
#include <QQueue>
#include <QWaitCondition>
#include <QMutex>

template<typename T>
class ThreadQueue{
public:
    ThreadQueue() = default;
    ~ThreadQueue() = default;

    ThreadQueue(const ThreadQueue&) = delete;
    ThreadQueue& operator=(const ThreadQueue&)=delete;

    void enqueue(const T& t){
        QMutexLocker locker(&m_mutex);
        m_queue.enqueue(t);
        m_wait.wakeOne();
    }
    //消费者从队列调出数据
    bool dequeue(T& out){
        QMutexLocker locker(&m_mutex);
        while(m_queue.isEmpty() && !m_stopped){
            m_wait.wait(&m_mutex);
        }
        if(m_queue.isEmpty()){
            return false;
        }
        out = m_queue.dequeue();
        return true;
    }
    void stop(){
        QMutexLocker locker(&m_mutex);
        m_stopped =true;
        m_wait.wakeAll();
    }
    int sizeOf() const{
        QMutexLocker locker(&m_mutex);
        return m_queue.size();
    }
private:
    mutable QMutex m_mutex;
    QQueue<T> m_queue;
    QWaitCondition m_wait;
    bool m_stopped = false;
};

#endif