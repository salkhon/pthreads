#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
  //	Write the code for initializing your read-write lock.
    pthread_mutex_init(&rw->fastmutex, NULL);
    pthread_cond_init(&rw->cond, NULL);
    rw->readers = 0;
    rw->writers = 0;
    rw->write_req = 0;
}

void ReaderLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
    pthread_mutex_lock(&rw->fastmutex);
    while (rw->writers > 0 || rw->write_req > 0) {
        pthread_cond_wait(&rw->cond, &rw->fastmutex);
    }
    rw->readers++;
    pthread_mutex_unlock(&rw->fastmutex);
}

void ReaderUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
    pthread_mutex_lock(&rw->fastmutex);
    rw->readers--;
    pthread_cond_broadcast(&rw->cond);
    pthread_mutex_unlock(&rw->fastmutex);

}

void WriterLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
    pthread_mutex_lock(&rw->fastmutex);
    rw->write_req++;
    while (rw->readers > 0 || rw->writers > 0) {
        pthread_cond_wait(&rw->cond, &rw->fastmutex);
    }
    rw->write_req--;
    rw->writers++;
    pthread_mutex_unlock(&rw->fastmutex);

}

void WriterUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
    pthread_mutex_lock(&rw->fastmutex);
    rw->writers--;
    pthread_cond_broadcast(&rw->cond);
    pthread_mutex_unlock(&rw->fastmutex);
}
