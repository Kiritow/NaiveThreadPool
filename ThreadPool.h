#pragma once
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>

struct ThreadWorkerData
{
    std::function<void()> func;
    std::mutex m;
    std::condition_variable cond;
    bool started;
    bool finished;
    bool running;
};

void _global_thread_worker_main(ThreadWorkerData* ptdd);


class ThreadPool
{
public:
    ThreadPool(int n);
    ~ThreadPool();

    int start(const std::function<void()>& func);
private:
    std::vector<ThreadWorkerData*> dvec;
    std::vector<std::thread*> tvec;
    int left;
    int busy;
    int total;
};
