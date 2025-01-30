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
    class ThreadPool
    {
    public:
        // Constructor: initialize with a number of threads, max queue size, and max dynamic threads.
        explicit ThreadPool(std::size_t num_threads,
                            std::size_t max_queue_size = 100,
                            std::size_t max_dynamic_threads = 20,
                            std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

        // Enqueue a task with timeout handling and dynamic thread adjustment
        bool enqueue(std::function<void()> task);

        // Stop all threads in the pool
        void stop();

        ~ThreadPool();

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

}

#endif