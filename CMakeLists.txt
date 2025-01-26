# Spécifie la version minimale de CMake requise
cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(MonProjet CXX)

# Définir les options de compilation
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weffc++ -g -fsanitize=address")

# Ajouter Boost (vérifie si Boost est installé sur ton système)
find_package(Boost 1.78 REQUIRED)

# Si Boost a été trouvé, lier Boost à ton projet
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# Ajouter les répertoires d'inclusion de MySQL Connector/C++
include_directories(/usr/include/mysql)

# Ajouter les répertoires de bibliothèque MySQL Connector/C++
link_directories(/usr/lib)

# Ajouter le répertoire d'inclusion de spdlog
include_directories(/usr/local/include/spdlog)

# Ajouter le répertoire où la bibliothèque spdlog a été installée
link_directories(/usr/local/lib)

# Trouver OpenSSL sur ton système
find_package(OpenSSL REQUIRED)

# Ajouter le répertoire d'inclusion de OpenSSL
include_directories(${OPENSSL_INCLUDE_DIR})

# Ajouter le répertoire src/config où se trouvent Config.hpp et Config.cpp
include_directories(${CMAKE_SOURCE_DIR}/src/config)

# Ajouter tous les fichiers .cpp dans src et src/config
# Assure-toi que Config.cpp est bien inclus
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/config/*.cpp)

# Ajouter l'exécutable et spécifier les fichiers sources
add_executable(prog ${SOURCE_FILES})

# Lier la bibliothèque MySQL Connector/C++ au projet
target_link_libraries(prog PRIVATE mysqlcppconn)

# Lier Boost à ton projet
target_link_libraries(prog PRIVATE ${Boost_LIBRARIES})

# Lier spdlog à ton projet
target_link_libraries(prog PRIVATE spdlog)

# Lier OpenSSL à ton projet
target_link_libraries(prog PRIVATE OpenSSL::SSL OpenSSL::Crypto)
