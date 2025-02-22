#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <chrono>
#include <pthread.h> // Pour utiliser pthread et définir la taille de la pile

namespace Softadastra
{
    class ThreadPool
    {
    public:
        // Constructeur du ThreadPool
        ThreadPool(std::size_t num_threads,
                   std::size_t max_queue_size,
                   std::size_t max_dynamic_threads,
                   std::chrono::milliseconds timeout);

        // Méthode pour ajouter une tâche au ThreadPool
        bool enqueue(std::function<void()> task);

        // Méthode pour arrêter le ThreadPool et joindre tous les threads
        void stop();

        // Destructeur pour arrêter proprement le ThreadPool
        ~ThreadPool();

    private:
        // Méthode exécutée par chaque thread dans le pool
        static void *worker_thread(void *arg);

        // Liste des threads du pool
        std::vector<pthread_t> workers;

        // Taille maximale de la queue de tâches
        std::size_t max_queue_size;

        // Nombre maximal de threads dynamiques que le pool peut créer
        std::size_t max_dynamic_threads;

        // Durée d'attente avant d'ajouter de nouveaux threads
        std::chrono::milliseconds timeout;

        // Nombre actuel de threads dans le pool
        std::size_t current_threads;

        // Queue de tâches à traiter
        std::queue<std::function<void()>> task_queue;

        // Mutex pour protéger l'accès à la queue
        std::mutex queue_mutex;

        // Condition variable pour la synchronisation des threads
        std::condition_variable condition;

        // Flag pour arrêter les threads
        std::atomic<bool> stop_flag;
    };
} // namespace Softadastra

#endif // THREADPOOL_HPP
