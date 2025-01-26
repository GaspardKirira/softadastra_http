#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>

// Thread Pool
class ThreadPool
{
public:
    explicit ThreadPool(std::size_t num_threads)
        : workers(), task_queue(), queue_mutex(), condition(), stop_flag(false) // Initialisation dans la liste
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
    void enqueue(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            task_queue.push(std::move(task));
        }
        condition.notify_one();
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
};