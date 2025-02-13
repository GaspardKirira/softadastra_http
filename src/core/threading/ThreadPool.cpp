#include "ThreadPool.hpp"
#include <iostream>
#include <atomic>
#include <chrono>
#include <spdlog/spdlog.h>

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
        // Crée les threads de travail à la construction
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

                                         // Traite la tâche
                                         try
                                         {
                                             task(); // Traite la tâche
                                         }
                                         catch (const std::exception &e)
                                         {
                                             spdlog::error("Exception in thread pool worker: {}", e.what());
                                         }
                                     } });
        }
    }

    bool ThreadPool::enqueue(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            // Si la queue est trop pleine, on essaie d'ajouter un thread si possible
            if (task_queue.size() >= max_queue_size)
            {
                if (workers.size() < max_dynamic_threads)
                {
                    // Crée un thread supplémentaire pour traiter la surcharge
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

                                                 try
                                                 {
                                                     task(); // Traite la tâche
                                                 }
                                                 catch (const std::exception &e)
                                                 {
                                                     spdlog::error("Exception in dynamically created thread: {}", e.what());
                                                 }
                                             } });
                    ++current_threads; // Augmente le nombre de threads
                    return true;
                }
                else
                {
                    // Si trop de threads sont déjà créés, on attend un peu avant de réessayer
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

            task_queue.push(std::move(task)); // Ajoute la tâche normalement
        }

        condition.notify_one(); // Réveille un thread pour traiter la tâche
        return true;
    }

    void ThreadPool::stop()
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop_flag = true;
        }

        condition.notify_all(); // Réveille tous les threads pour qu'ils vérifient s'ils doivent se terminer

        for (std::thread &worker : workers)
        {
            if (worker.joinable())
                worker.join(); // Attend que chaque thread se termine
        }
    }

    ThreadPool::~ThreadPool() { stop(); }
} // namespace Softadastra
