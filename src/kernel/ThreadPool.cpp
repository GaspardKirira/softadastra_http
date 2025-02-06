#include "ThreadPool.hpp"

namespace Softadastra
{
    ThreadPool::ThreadPool(std::size_t num_threads,
                           std::size_t max_queue_size,
                           std::size_t max_dynamic_threads,
                           std::chrono::milliseconds timeout)
        : max_queue_size(max_queue_size),
          max_dynamic_threads(max_dynamic_threads),
          timeout(timeout),
          current_threads(num_threads),
          workers(num_threads),
          task_queue(),
          queue_mutex(),
          condition(),
          stop_flag(false)
    {
        for (std::size_t i = 0; i < num_threads; ++i)
        {
            workers.emplace_back([this]
                                 {
                                 while (true)
                                 {
                                     std::function<void()> task;
                                     {
                                         std::unique_lock<std::mutex> lock(queue_mutex);
                                         condition.wait(lock, [this] { return stop_flag || !task_queue.empty(); });

                                         if (stop_flag && task_queue.empty())
                                             return;

                                         task = std::move(task_queue.front());
                                         task_queue.pop();
                                     }
                                     task(); 
                                 } });
        }
    }

    bool ThreadPool::enqueue(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            if (task_queue.size() >= max_queue_size)
            {
                if (workers.size() < max_dynamic_threads)
                {
                    workers.emplace_back([this]           
                                         {
                                         while (true)
                                         {
                                             std::function<void()> task;
                                             {
                                                 std::unique_lock<std::mutex> lock(queue_mutex); 
                                                 condition.wait(lock, [this] { return stop_flag || !task_queue.empty(); });

                                                 if (stop_flag && task_queue.empty())
                                                     return;

                                                 task = std::move(task_queue.front());
                                                 task_queue.pop();
                                             }
                                             task(); 
                                         } });
                    ++current_threads;
                    return true;
                }
                else
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    if (condition.wait_for(lock, timeout, [this]
                                           { return task_queue.size() < max_queue_size || stop_flag; }))
                    {
                        task_queue.push(std::move(task));
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            task_queue.push(std::move(task));
        }
        condition.notify_one();
        return true;
    }

    void ThreadPool::stop()
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop_flag = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
        {
            worker.join();
        }
    }

    ThreadPool::~ThreadPool() { stop(); }
} // namespace Softadastra