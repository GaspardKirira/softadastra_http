cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Spécifier le standard C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weffc++ -g -fsanitize=address")

# Ajouter le répertoire contenant les en-têtes de bcrypt
include_directories(${CMAKE_SOURCE_DIR}/bcrypt)

# Ajouter le répertoire contenant la bibliothèque statique libbcrypt.a
link_directories(${CMAKE_SOURCE_DIR}/bcrypt)

# Ajouter les fichiers sources
set(SOURCES
    main.cpp
    Hash.cpp
)

# Créer l'exécutable
add_executable(main ${SOURCES})

# Lier les bibliothèques nécessaires
target_link_libraries(main bcrypt ssl crypto)

# Ajouter des options de linker
set_target_properties(main PROPERTIES LINK_FLAGS "-fsanitize=address")
