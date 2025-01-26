#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <atomic>
#include <memory>

class ThreadPool
{
public:
    // Constructor: initialize with a number of threads, max queue size, and max dynamic threads.
    explicit ThreadPool(std::size_t num_threads,
                        std::size_t max_queue_size = 100,
                        std::size_t max_dynamic_threads = 20,
                        std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
        : max_queue_size(max_queue_size),
          max_dynamic_threads(max_dynamic_threads),
          timeout(timeout),
          current_threads(num_threads), // L'initialisation dans l'ordre de la déclaration
          workers(num_threads),         // Initialisation des threads
          task_queue(),                 // Initialisation de la file d'attente des tâches
          queue_mutex(),                // Initialisation du mutex
          condition(),                  // Initialisation explicite de la condition
          stop_flag(false)              // Initialisation du drapeau d'arrêt
    {
        // Create the initial set of worker threads
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
                                         task(); // Execute the task
                                     } });
        }
    }

    // Enqueue a task with timeout handling and dynamic thread adjustment
    bool enqueue(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            // Si la queue est pleine et que nous n'avons pas encore atteint le nombre max de threads
            if (task_queue.size() >= max_queue_size)
            {
                // Vérifiez si nous pouvons augmenter le nombre de threads
                if (workers.size() < max_dynamic_threads)
                {
                    // Créez un nouveau thread pour traiter les tâches
                    workers.emplace_back([this]
                                         {
                                         while (true)
                                         {
                                             std::function<void()> task;
                                             {
                                                 std::unique_lock<std::mutex> lock(queue_mutex);  // Utilisation de unique_lock ici
                                                 condition.wait(lock, [this] { return stop_flag || !task_queue.empty(); });

                                                 if (stop_flag && task_queue.empty())
                                                     return;

                                                 task = std::move(task_queue.front());
                                                 task_queue.pop();
                                             }
                                             task(); // Exécutez la tâche
                                         } });
                    ++current_threads; // Augmenter le nombre de threads
                    return true;       // Nous avons ajouté un nouveau thread
                }
                else
                {
                    // Si la queue est pleine et que nous ne pouvons pas ajouter plus de threads, essayez d'attendre
                    std::unique_lock<std::mutex> lock(queue_mutex); // Utilisez unique_lock ici

                    if (condition.wait_for(lock, timeout, [this]
                                           { return task_queue.size() < max_queue_size || stop_flag; }))
                    {
                        task_queue.push(std::move(task)); // Si de la place devient disponible, ajouter la tâche
                        return true;
                    }
                    else
                    {
                        return false; // Timeout, rejet de la tâche
                    }
                }
            }

            // Si la queue a de la place, ajoutez directement la tâche
            task_queue.push(std::move(task));
        }
        condition.notify_one();
        return true;
    }

    // Stop all threads in the pool
    void stop()
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

    ~ThreadPool() { stop(); }

private:
    std::size_t max_queue_size;                   // Maximum size of the queue
    std::size_t max_dynamic_threads;              // Maximum number of threads
    std::chrono::milliseconds timeout;            // Timeout for waiting tasks
    std::size_t current_threads;                  // Current number of threads
    std::vector<std::thread> workers;             // Worker threads in the pool
    std::queue<std::function<void()>> task_queue; // Queue to store tasks
    std::mutex queue_mutex;                       // Mutex for the task queue
    std::condition_variable condition;            // Condition variable for the task queue
    bool stop_flag;                               // Stop flag for the thread pool
};
