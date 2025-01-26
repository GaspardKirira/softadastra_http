#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>


class ThreadPool
{
public:
    explicit ThreadPool(std::size_t num_threads, std::size_t max_queue_size = 100)
        : workers(), task_queue(), queue_mutex(), condition(), stop_flag(false), max_queue_size(max_queue_size) // Initialisation dans la liste
    {
        for (std::size_t i = 0; i < num_threads; ++i)
        {
            workers.emplace_back([this]
                                 {
                                     while (true) {
                                         std::function<void()> task;
                                         {
                                             std::unique_lock<std::mutex> lock(queue_mutex);
                                             condition.wait(lock, [this] { return stop_flag || !task_queue.empty(); });
                                             if (stop_flag && task_queue.empty()) return;
                                             task = std::move(task_queue.front());
                                             task_queue.pop();
                                         }
                                         task();
                                     } });
        }
    }

    // Ajoute une tâche à la file d'attente
    bool enqueue(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            // Si la file d'attente est pleine, on rejette la tâche
            if (task_queue.size() >= max_queue_size)
            {
                return false; // Retourne faux pour signaler que la tâche a été rejetée
            }

            task_queue.push(std::move(task));
        }
        condition.notify_one();
        return true;
    }

    // Stoppe tous les threads
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
    std::vector<std::thread> workers;             // Liste des threads du pool
    std::queue<std::function<void()>> task_queue; // File d'attente des tâches
    std::mutex queue_mutex;                       // Mutex pour protéger l'accès à la file d'attente
    std::condition_variable condition;            // Variable de condition pour la synchronisation des threads
    bool stop_flag;                               // Flag pour arrêter les threads
    std::size_t max_queue_size;                   // Taille maximale de la file d'attente
};
