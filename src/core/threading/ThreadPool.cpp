#include <spdlog/spdlog.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <functional>
#include <vector>
#include <thread>
#include <iostream>
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
          workers(),
          task_queue(),
          queue_mutex(),
          condition(),
          stop_flag(false)
    {
        for (std::size_t i = 0; i < num_threads; ++i)
        {
            workers.push_back(std::thread(&ThreadPool::worker_thread, this));
        }
    }

    void ThreadPool::worker_thread()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this]
                               { return stop_flag || !task_queue.empty(); });

                if (stop_flag && task_queue.empty())
                    return;

                task = std::move(task_queue.front());
                task_queue.pop_front();
            }

            try
            {
                task();
            }
            catch (const std::exception &e)
            {
                spdlog::error("Exception in thread pool worker: {}", e.what());
            }

            // Réduire le nombre de threads dynamiques si la queue est vide et que nous avons trop de threads
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                if (task_queue.size() < max_queue_size / 2 && current_threads > workers.size())
                {
                    spdlog::info("Scaling down threads...");
                    if (workers.size() > current_threads)
                    {
                        workers.back().join();
                        workers.pop_back();
                    }
                }
            }
        }
    }

    bool ThreadPool::enqueue(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (task_queue.size() >= max_queue_size)
            {
                if (current_threads.load() < max_dynamic_threads)
                {
                    workers.push_back(std::thread(&ThreadPool::worker_thread, this));
                    current_threads.fetch_add(1);
                    spdlog::info("Added new dynamic thread.");
                    return true;
                }
                else
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    if (condition.wait_for(lock, timeout, [this]
                                           { return task_queue.size() < max_queue_size || stop_flag; }))
                    {
                        task_queue.push_back(std::move(task));
                        return true;
                    }
                    else
                    {
                        spdlog::warn("Queue full and timeout reached.");
                        return false;
                    }
                }
            }
            task_queue.push_back(std::move(task));
        }

        condition.notify_all();
        return true;
    }

    void ThreadPool::stop()
    {
        {
            std::lock_guard<std::mutex> lock(stop_mutex);
            stop_flag = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
        {
            if (worker.joinable())
                worker.join();
        }
    }

    ThreadPool::~ThreadPool()
    {
        stop();
    }
} // namespace Softadastra
