#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <utility>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <pthread.h>
#include <unordered_map>
#include <shared_mutex>

namespace Softadastra
{

    // Structure pour gérer les tâches avec priorité
    struct Task
    {
        std::function<void()> func;
        int priority;

        // Constructeur prenant un callable et une priorité
        Task(std::function<void()> f, int p) : func(f), priority(p) {}

        // Constructeur par défaut si nécessaire
        Task() : func(nullptr), priority(0) {}

        // Définir l'opérateur < pour trier les tâches par priorité
        bool operator<(const Task &other) const
        {
            return priority < other.priority; // La tâche avec la plus haute priorité doit être en tête
        }
    };

    extern thread_local int threadId;

    class ThreadPool
    {
    private:
        std::vector<std::thread> workers;
        std::priority_queue<Task> tasks;
        std::mutex m;
        std::condition_variable condition;
        std::atomic<bool> stop;
        std::atomic<bool> stopPeriodic;
        size_t maxThreads;
        std::unordered_map<std::thread::id, int> threadAffinity;
        std::atomic<int> activeTasks;

        // Nouveau membre pour stocker la priorité des threads
        int threadPriority; // Priorité des threads

        // Fonction pour affecter une affinité CPU à un thread
        void setThreadAffinity(int id)
        {
#ifdef __linux__
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(id % std::thread::hardware_concurrency(), &cpuset);
            pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#endif
        }

    public:
        ThreadPool(size_t threadCount, size_t maxThreadCount, int priority, std::chrono::milliseconds interval)
            : workers(),
              threadAffinity(),
              tasks(),
              stop(false),
              stopPeriodic(false),
              maxThreads(maxThreadCount),
              activeTasks(0),
              threadPriority(priority) // Initialisation correcte
        {
            // Initialisation des threads
            for (size_t i = 0; i < threadCount; ++i)
            {
                createThread(i);
            }
            // Utilisation de l'intervalle pour les tâches périodiques si nécessaire
        }

        // Fonction pour créer un nouveau thread
        void createThread(int id)
        {
            workers.emplace_back([this, id]
                                 {
                threadId = id;
                threadAffinity[std::this_thread::get_id()] = id;
    
                setThreadAffinity(id);  // Configurer l'affinité du thread
    
                while (true) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(m);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
    
                        if (stop && tasks.empty()) return;
    
                        task = std::move(tasks.top());
                        tasks.pop();
                        ++activeTasks;
                    }
                    try {
                        task.func();
                    } catch (const std::exception& e) {
                        std::cerr << "Exception dans le thread " << threadId << ": " << e.what() << std::endl;
                    }
                    --activeTasks;
                    condition.notify_one();
                } });
        }

        template <class F, class... Args>
        auto enqueue(int priority, F &&f, Args &&...args) -> std::future<typename std::invoke_result<F, Args...>::type>
        {
            using ReturnType = typename std::invoke_result<F, Args...>::type;
            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            std::future<ReturnType> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(m);
                tasks.push(Softadastra::Task{[task]()
                                             {
                                                 try
                                                 {
                                                     (*task)();
                                                 }
                                                 catch (const std::exception &e)
                                                 {
                                                     std::cerr << "Exception in task: " << e.what() << std::endl;
                                                 }
                                             },
                                             priority});

                if (workers.size() < maxThreads)
                {
                    createThread(workers.size());
                }
            }
            condition.notify_one();
            return res;
        }

        void periodicTask(int priority, std::function<void()> func, std::chrono::milliseconds interval)
        {
            auto loop = [this, priority, func, interval]()
            {
                while (!stopPeriodic)
                {
                    try
                    {
                        auto future = enqueue(priority, func);
                        if (future.wait_for(interval) == std::future_status::timeout)
                        {
                            std::cerr << "Tâche périodique annulée pour dépassement du temps limite." << std::endl;
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "Exception dans la tâche périodique : " << e.what() << std::endl;
                    }
                    std::this_thread::sleep_for(interval);
                }
            };
            std::thread(loop).detach();
        }

        // Vérifie si le pool est inactif (pas de tâches et pas de threads actifs)
        bool isIdle()
        {
            return activeTasks.load() == 0 && tasks.empty();
        }

        // Arrêter toutes les tâches périodiques
        void stopPeriodicTasks()
        {
            stopPeriodic = true;
        }

        // Destruction du pool de threads`
        ~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(m);
                stop = true;
                stopPeriodic = true;
            }
            condition.notify_all();
            for (std::thread &worker : workers)
            {
                if (worker.joinable())
                {
                    worker.join();
                }
            }
        }
    };

} // namespace Softadastra

#endif // THREADPOOL_HPP
