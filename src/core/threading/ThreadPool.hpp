#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

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

namespace Softadastra
{
    /**
     * @class ThreadPool
     * @brief A thread pool to manage a collection of threads that can execute tasks asynchronously.
     *
     * The ThreadPool class creates a pool of worker threads, which can take tasks from a queue
     * and execute them concurrently. It handles the management of threads, task queue, and stopping logic.
     */
    class ThreadPool
    {
    public:
        /**
         * @brief Constructor that initializes the thread pool with a specified number of threads and optional parameters.
         *
         * @param num_threads The number of threads in the thread pool.
         * @param max_queue_size The maximum number of tasks that can be enqueued before new tasks are rejected (default: 100).
         * @param max_dynamic_threads The maximum number of threads the pool can dynamically scale to (default: 20).
         * @param timeout The amount of time a thread will wait for a task before checking if the pool should stop (default: 100 ms).
         */
        explicit ThreadPool(std::size_t num_threads,
                            std::size_t max_queue_size = 100,
                            std::size_t max_dynamic_threads = 20,
                            std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

        /**
         * @brief Enqueues a task to the thread pool for execution.
         *
         * If the task queue is full, this method will return false and the task will not be added.
         *
         * @param task The task to be executed, encapsulated in a std::function<void()>.
         * @return true if the task was successfully added to the queue, false if the queue was full.
         */
        bool enqueue(std::function<void()> task);

        /**
         * @brief Stops the thread pool, causing all threads to finish their tasks and stop.
         *
         * This method signals the threads to stop and waits for them to finish executing.
         */
        void stop();

        /**
         * @brief Destructor that cleans up the thread pool resources.
         *
         * Ensures that all threads are properly joined before destroying the thread pool object.
         */
        ~ThreadPool();

    private:
        std::size_t max_queue_size;                   ///< Maximum number of tasks that can be queued.
        std::size_t max_dynamic_threads;              ///< Maximum number of threads that can be dynamically created.
        std::chrono::milliseconds timeout;            ///< Timeout duration for threads when idle.
        std::size_t current_threads;                  ///< Current number of active threads in the pool.
        std::vector<std::thread> workers;             ///< Vector of worker threads.
        std::queue<std::function<void()>> task_queue; ///< Queue holding the tasks to be executed.
        std::mutex queue_mutex;                       ///< Mutex to protect access to the task queue.
        std::condition_variable condition;            ///< Condition variable to manage thread synchronization.
        bool stop_flag;                               ///< Flag indicating whether the thread pool should stop.
    };

}

#endif
