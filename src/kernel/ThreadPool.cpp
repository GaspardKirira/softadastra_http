#include "ThreadPool.hpp"

namespace Softadastra
{
    // ThreadPool.cpp
    ThreadPool::ThreadPool(std::size_t num_threads,
                           std::size_t max_queue_size,
                           std::size_t max_dynamic_threads,
                           std::chrono::milliseconds timeout)
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
        // Créez le pool de threads
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
                                     task(); // Exécutez la tâche
                                 } });
        }
    }

    // Enqueue a task with timeout handling and dynamic thread adjustment
    bool ThreadPool::enqueue(std::function<void()> task)
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
    void ThreadPool::stop()
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

    ThreadPool::~ThreadPool() { stop(); }
} // namespace Softadastra

// std::optional<User> get_user_by_id(int userId)
//         {
//             try
//             {
//                 // Utilisation de shared_ptr pour gérer la connexion à la base de données
//                 std::unique_ptr<sql::mysql::MySQL_Driver> driver(sql::mysql::get_driver_instance());
//                 if (!driver)
//                 {
//                     std::cerr << "Failed to get MYSQL driver" << std::endl;
//                 }

//                 std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1", "root", ""));
//                 if (!con)
//                 {
//                     std::cerr << "Failed to connect to the database" << std::endl;
//                 }
//                 con->setSchema("softadastra");
//                 std::unique_ptr<sql::Statement> stmt(con->createStatement());
//                 if (!stmt)
//                 {
//                     std::cerr << "Failed to create statement" << std::endl;
//                 }

//                 // Création d'un PreparedStatement avec shared_ptr
//                 std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM users WHERE id = ?"));
//                 // Définir le paramètre de la requête
//                 pstmt->setInt(1, userId);
//                 // Exécuter la requête et obtenir le résultat
//                 std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

//                 // Si un utilisateur est trouvé
//                 if (res->next())
//                 {
//                     User user;
//                     user.setId(res->getInt("id"));
//                     user.setFullName(res->getString("full_name"));
//                     user.setEmail(res->getString("email"));

//                     return user; // Retourner l'utilisateur trouvé
//                 }

//                 // Aucun utilisateur trouvé, retourner std::nullopt
//                 return std::nullopt;
//             }
//             catch (const sql::SQLException &e)
//             {
//                 // Gestion des erreurs SQL
//                 std::cerr << "Erreur SQL : " << e.what() << std::endl;
//                 throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
//             }
//             catch (const std::exception &e)
//             {
//                 // Gestion des autres exceptions
//                 std::cerr << "Erreur générique : " << e.what() << std::endl;
//                 throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
//             }
//         }

// std::optional<User> get_user_by_id(int userId)
//         {
//             try
//             {
//                 // Utilisation de Config pour obtenir la connexion à la base de données
//                 Config &config = Config::getInstance();
//                 config.loadConfig();

//                 std::unique_ptr<sql::Connection> con = config.getDbConnection();

//                 // Assurez-vous que la connexion est toujours valide
//                 if (!con)
//                 {
//                     throw std::runtime_error("La connexion à la base de données a échoué.");
//                 }

//                 // Utilisation d'un unique_ptr pour gérer automatiquement le PreparedStatement
//                 std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM users WHERE id = ?"));

//                 // Définir le paramètre de la requête
//                 pstmt->setInt(1, userId);

//                 // Exécuter la requête et obtenir le résultat
//                 std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

//                 // Si un utilisateur est trouvé
//                 if (res->next())
//                 {
//                     User user;
//                     user.setId(res->getInt("id"));
//                     user.setFullName(res->getString("full_name"));
//                     user.setEmail(res->getString("email"));

//                     return user; // Retourner l'utilisateur trouvé
//                 }

//                 // Aucun utilisateur trouvé, retourner std::nullopt
//                 return std::nullopt;
//             }
//             catch (const sql::SQLException &e)
//             {
//                 // Gestion des erreurs SQL
//                 std::cerr << "Erreur SQL : " << e.what() << std::endl;
//                 throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
//             }
//             catch (const std::exception &e)
//             {
//                 // Gestion des autres exceptions
//                 std::cerr << "Erreur générique : " << e.what() << std::endl;
//                 throw std::runtime_error("Erreur lors de la récupération de l'utilisateur : " + std::string(e.what()));
//             }
//         }