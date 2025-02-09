// Explication des fonctionnalités :
// 1. Constructeurs et opérateurs de copie/déplacement : La classe prend en charge la copie et le déplacement, ce qui la rend plus flexible pour les usages dans des contextes variés.

// 2. Méthode insert : Permet d'insérer des paires clé-valeur dans la map. Elle permet également d'insérer des objets déplacés pour plus d'efficacité.

// 3. Accès via l'opérateur [] : Utilisation de l'opérateur [] pour accéder aux valeurs. Si la clé n'existe pas, elle est créée avec une valeur par défaut. La version constante permet de consulter la valeur sans la modifier.

// 4. Recherche avec contains : Permet de vérifier si une clé existe dans la map sans essayer d’y accéder directement.

// 5. Suppression avec erase : Permet de supprimer un élément basé sur sa clé.

// 6. Gestion de la taille et de la capacité : La méthode size retourne le nombre d'éléments dans la map, et la méthode capacity retourne la capacité estimée de la map (en fonction du nombre de "buckets" utilisés dans la table de hachage).

// 7. Méthodes d'itération : Vous pouvez itérer sur les éléments de la map avec les itérateurs, comme pour un std::unordered_map.

// 8. Exceptions personnalisées : Si une clé est recherchée mais non trouvée, une exception KeyNotFoundException est levée.

// 9. Méthode get_pair : Permet de récupérer une paire clé-valeur associée à une clé. Si la clé n'existe pas, une exception est levée.

// 10. Méthode swap : Permet d’échanger deux UnorderedMap efficacement.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. Insertion et accès aux éléments

// #include <iostream>
// #include "UnorderedMap.hpp"

// int main()
// {
//     Softadastra::UnorderedMap<int, std::string> map;

//     // Ajouter des éléments
//     map.insert(1, "Apple");
//     map.insert(2, "Banana");
//     map.insert(3, "Cherry");

//     // Accéder à un élément
//     std::cout << "Clé 2 : " << map[2] << std::endl; // Affiche "Banana"

//     // Modifier un élément
//     map[2] = "Blueberry";
//     std::cout << "Clé 2 après modification : " << map[2] << std::endl; // Affiche "Blueberry"

//     return 0;
// }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2. Vérification de l'existence d'une clé avec contains

// if (map.contains(3))
// {
//     std::cout << "La clé 3 existe." << std::endl;
// }
// else
// {
//     std::cout << "La clé 3 n'existe pas." << std::endl;
// }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3. Suppression d'un élément 

// map.erase(2);  // Supprimer l'élément avec la clé 2
// std::cout << "Clé 2 supprimée." << std::endl;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 4. Afficher le contenu de la map avec print
// map.print();  // Affiche le contenu de la map clé-valeur

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 5. Utilisation des itérateurs

// for (auto it = map.begin(); it != map.end(); ++it)
// {
//     std::cout << it->first << " : " << it->second << std::endl;
// }
