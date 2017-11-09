#pragma once
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>

struct ThreadWorkerData;

class ThreadPool
{
public:
    ThreadPool(int n);
    /// NonCopyable, NonMovable.
    ThreadPool(const ThreadPool&)=delete;
    ThreadPool& operator = (const ThreadPool&)=delete;
    ThreadPool(ThreadPool&&)=delete;
    ThreadPool& operator = (ThreadPool&&)=delete;
    ~ThreadPool();

    int start(const std::function<void()>&);
private:
    std::vector<ThreadWorkerData*> dvec;
    std::vector<std::thread*> tvec;
    int left;
    int busy;
    int total;
};
