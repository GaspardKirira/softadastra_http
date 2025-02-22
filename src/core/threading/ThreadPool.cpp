#include <pthread.h>
#include <spdlog/spdlog.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
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
        // Taille de la pile souhaitée (en octets, par exemple 8 Mo)
        const size_t stack_size = 8 * 1024 * 1024; // 8MB

        // Crée les threads de travail à la construction avec une pile personnalisée
        for (std::size_t i = 0; i < num_threads; ++i)
        {
            pthread_t thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setstacksize(&attr, stack_size);

            int result = pthread_create(&thread, &attr, &ThreadPool::worker_thread, this);
            if (result != 0)
            {
                spdlog::error("Failed to create thread with custom stack size: {}", result);
                continue;
            }
            workers.push_back(thread);
        }
    }

    void *ThreadPool::worker_thread(void *arg)
    {
        ThreadPool *pool = static_cast<ThreadPool *>(arg);

        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(pool->queue_mutex);
                pool->condition.wait(lock, [pool]
                                     { return pool->stop_flag || !pool->task_queue.empty(); });

                if (pool->stop_flag && pool->task_queue.empty())
                    return nullptr; // Fin du thread, on retourne nullptr comme prévu par pthread_create.

                task = std::move(pool->task_queue.front());
                pool->task_queue.pop();
            }

            try
            {
                task(); // Traite la tâche
            }
            catch (const std::exception &e)
            {
                spdlog::error("Exception in thread pool worker: {}", e.what());
            }

            // Dynamically scale down if task queue is small and excess workers
            if (pool->task_queue.size() < pool->max_queue_size / 2 && pool->workers.size() > pool->current_threads)
            {
                // Code to reduce thread count dynamically
                spdlog::info("Scaling down threads...");
            }
        }

        return nullptr; // Respecter la signature de retour.
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
                    pthread_t thread;
                    pthread_attr_t attr;
                    pthread_attr_init(&attr);
                    pthread_attr_setstacksize(&attr, 8 * 1024 * 1024); // 8 MB de pile

                    int result = pthread_create(&thread, &attr, &ThreadPool::worker_thread, this);
                    if (result == 0)
                    {
                        workers.push_back(thread);
                        ++current_threads; // Augmente le nombre de threads
                        return true;
                    }
                    else
                    {
                        spdlog::error("Failed to create dynamic thread with custom stack size: {}", result);
                        return false;
                    }
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

        for (pthread_t &worker : workers)
        {
            pthread_join(worker, nullptr); // Attend que chaque thread se termine
        }
    }

    ThreadPool::~ThreadPool() { stop(); }
} // namespace Softadastra
