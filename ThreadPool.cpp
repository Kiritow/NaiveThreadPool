#include "ThreadPool.h"

struct ThreadWorkerData
{
    std::function<void()> func;
    std::mutex m;
    std::condition_variable cond;
    bool started;
    bool finished;
    bool running;
};

using namespace std;

static void _global_thread_worker_main(ThreadWorkerData* ptdd)
{
    unique_lock<mutex> ulk(ptdd->m);
    while(ptdd->running)
    {
        ptdd->started=true;
        ptdd->func();
        ptdd->finished=true;
        ptdd->cond.wait(ulk); /// wait for new work. (will unlock ptdd->m while waiting...)
    }
}

ThreadPool::ThreadPool(int n) : left(0),busy(0),total(n)
{

}

ThreadPool::~ThreadPool()
{
    for(int i=0;i<left+busy;i++)
    {
        //cout<<"Waiting... thread "<<tvec[i]->get_id()<<endl;

        /// wait for it finish.
        while(true)
        {
            dvec[i]->m.lock();
            if(dvec[i]->finished)
            {
                dvec[i]->m.unlock();
                break;
            }
            else
            {
                dvec[i]->m.unlock();
                this_thread::sleep_for(chrono::seconds(1));
            }
        }

        dvec[i]->running=false;
        /// notify it
        dvec[i]->cond.notify_all();
        /// wait for it finish.
        tvec[i]->join();
        /// release resource
        delete dvec[i];
        dvec[i]=nullptr;
        delete tvec[i];
        tvec[i]=nullptr;
    }
}

int ThreadPool::start(const function<void()>& func)
{
    if(left>0)
    {
        bool done=false;
        int idx=-1;

        for(int i=0;i<left+busy;i++)
        {
            if(!dvec[i]->m.try_lock())
            {
                /// Failed to lock.
                continue;
            }
            if(dvec[i]->finished)
            {
                /// Found a finished work! Now we reuse it.
                dvec[i]->func=func;
                dvec[i]->started=false;
                dvec[i]->finished=false;
                dvec[i]->running=true;
                dvec[i]->cond.notify_all();

                done=true;
                idx=i;

                dvec[i]->m.unlock();
                break;
            }
            else
            {
                dvec[i]->m.unlock();
            }
        }

        if(done)
        {
            left--;
            busy++;
            return idx;
        }
        else
        {
            return -2;
        }
    }
    else
    {
        if(busy<total)
        {
            /// Still can create thread
            ThreadWorkerData* ptdd=new ThreadWorkerData;
            ptdd->func=func;
            ptdd->started=false;
            ptdd->finished=false;
            ptdd->running=true;
            dvec.push_back(ptdd);

            thread* t=new thread(_global_thread_worker_main,ptdd);
            tvec.push_back(t);

            busy++;

            return tvec.size()-1;
        }
        else
        {
            /// Check if we can reuse finished thread
            for(int i=0;i<total;i++)
            {
                if(!dvec[i]->m.try_lock())
                {
                    /// Failed to lock. continue.
                    continue;
                }
                if(dvec[i]->finished)
                {
                    left++;
                    busy--;
                }
                dvec[i]->m.unlock();
            }

            if(left>0) return start(func);
            else return -1;
        }
    }
}
