#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

// 线程池类，用于管理和执行异步任务
class ThreadPool
{
public:
    // 构造函数，指定线程数量
    ThreadPool(size_t threads);

    // 析构函数，停止所有线程
    ~ThreadPool();

    // 提交任务到线程池，返回future对象用于获取结果
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    // 工作线程
    std::vector<std::thread> workers;

    // 任务队列
    std::queue<std::function<void()>> tasks;

    // 同步原语
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// 线程池实现
inline ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    // 创建指定数量的工作线程
    for (size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back(
            [this]
            {
                for (;;)
                {
                    std::function<void()> task;

                    // 从任务队列获取任务
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                                             [this]
                                             { return this->stop || !this->tasks.empty(); });

                        if (this->stop && this->tasks.empty())
                            return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    // 执行任务
                    task();
                }
            });
    }
}

// 析构函数，停止所有线程
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }

    condition.notify_all();

    // 等待所有工作线程完成
    for (std::thread &worker : workers)
        worker.join();
}

// 提交任务到线程池
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{

    using return_type = typename std::result_of<F(Args...)>::type;

    // 创建一个包装任务
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    // 获取future对象
    std::future<return_type> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // 如果线程池已停止，不能提交新任务
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        // 将任务添加到队列
        tasks.emplace([task]()
                      { (*task)(); });
    }

    // 通知一个工作线程有新任务
    condition.notify_one();
    return res;
}
