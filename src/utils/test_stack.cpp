
// 1. Créer une pile et ajouter des éléments (push)

// #include <iostream>
// #include "Stack.hpp"

// int main() {
//     Softadastra::Stack<int> stack;  // Crée une pile d'entiers

//     // Ajouter des éléments à la pile
//     stack.push(10);
//     stack.push(20);
//     stack.push(30);

//     std::cout << "La pile contient des éléments !" << std::endl;
//     return 0;
// }

//////////////////////////////////////////////////////////////////////////////////
// 2. Accéder au sommet de la pile sans le retirer(top)

//  #include <iostream>
//  #include "Stack.hpp"

// int main()
// {
//     Softadastra::Stack<int> stack;

//     stack.push(10);
//     stack.push(20);
//     stack.push(30);

//     std::cout << "Élément au sommet de la pile : " << stack.top() << std::endl;
//     // Affiche "Élément au sommet de la pile : 30"

//     return 0;
// }

////////////////////////////////////////////////////////////////////////////////////
// 3. Retirer un élément du sommet de la pile (pop)

// #include <iostream>
// #include "Stack.hpp"

// int main() {
//     Softadastra::Stack<int> stack;

//     stack.push(10);
//     stack.push(20);
//     stack.push(30);

//     std::cout << "Élément retiré de la pile : " << stack.pop() << std::endl;
//     // Affiche "Élément retiré de la pile : 30"

//     std::cout << "Élément au sommet de la pile après pop : " << stack.top() << std::endl;
//     // Affiche "Élément au sommet de la pile après pop : 20"

//     return 0;
// }

/////////////////////////////////////////////////////////////////////////////////////
// 5. Obtenir la taille de la pile (size)

// #include <iostream>
// #include "Stack.hpp"

// int main() {
//     Softadastra::Stack<int> stack;

//     stack.push(10);
//     stack.push(20);
//     stack.push(30);

//     std::cout << "Taille de la pile : " << stack.size() << std::endl;
//     // Affiche "Taille de la pile : 3"

//     return 0;
// }

/////////////////////////////////////////////////////////////////////////////////////////
// 6. Obtenir la capacité de la pile (capacity)

// #include <iostream>
// #include "Stack.hpp"

// int main() {
//     Softadastra::Stack<int> stack;

//     stack.push(10);
//     stack.push(20);

//     std::cout << "Capacité de la pile : " << stack.capacity() << std::endl;
//     // Affiche la capacité, qui dépend de la mise en œuvre du vecteur.

//     return 0;
// }

///////////////////////////////////////////////////////////////////////////////////////////
// 7. Gérer les erreurs avec des exceptions

// #include <iostream>
// #include "Stack.hpp"
// #include <stdexcept>  // Pour std::underflow_error

// int main() {
//     Softadastra::Stack<int> stack;

//     try {
//         // Essayer de pop sur une pile vide
//         std::cout << "Tentative de pop sur une pile vide : " << stack.pop() << std::endl;
//     }
//     catch (const std::underflow_error& e) {
//         std::cout << "Erreur : " << e.what() << std::endl;
//         // Affiche "Erreur : La pile est vide. Impossible de retirer un élément."
//     }

//     try {
//         // Essayer de top sur une pile vide
//         std::cout << "Tentative de top sur une pile vide : " << stack.top() << std::endl;
//     }
//     catch (const std::underflow_error& e) {
//         std::cout << "Erreur : " << e.what() << std::endl;
//         // Affiche "Erreur : La pile est vide. Impossible d'accéder au sommet."
//     }

//     return 0;
// }

////////////////////////////////////////////////////////////////////////////////////////////////
// 8. Exemple complet avec toutes les méthodes

// #include <iostream>
// #include "Stack.hpp"

// int main() {
//     Softadastra::Stack<int> stack;

//     // Ajouter des éléments à la pile
//     stack.push(10);
//     stack.push(20);
//     stack.push(30);

//     // Vérifier la taille et la capacité
//     std::cout << "Taille de la pile : " << stack.size() << std::endl;
//     std::cout << "Capacité de la pile : " << stack.capacity() << std::endl;

//     // Accéder au sommet sans retirer
//     std::cout << "Sommet de la pile : " << stack.top() << std::endl;

//     // Retirer un élément
//     std::cout << "Élément retiré : " << stack.pop() << std::endl;

//     // Vérifier à nouveau le sommet
//     std::cout << "Sommet après pop : " << stack.top() << std::endl;

//     // Vérifier si la pile est vide
//     std::cout << "La pile est-elle vide ? " << (stack.empty() ? "Oui" : "Non") << std::endl;

//     // Essayer de retirer un élément d'une pile vide
//     try {
//         Softadastra::Stack<int> emptyStack;
//         emptyStack.pop();  // Déclenche une exception
//     }
//     catch (const std::underflow_error& e) {
//         std::cout << "Erreur : " << e.what() << std::endl;
//     }

//     return 0;
// }

///////////////////////////////////////////////////////////////////////////////////////////////////

// 9. Construction par copie et affectation

// Softadastra::Stack<int> stack1;
// stack1.push(10);
// stack1.push(20);

// // Copie de la pile
// Softadastra::Stack<int> stack2 = stack1;
// std::cout << "Top de stack2 : " << stack2.top() << std::endl; // Affiche 20

// // Affectation par copie
// Softadastra::Stack<int> stack3;
// stack3 = stack1;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// 10. Réduction de la capacité avec shrink_to_fit

// Softadastra::Stack<int> stack;
// stack.push(1);
// stack.push(2);
// stack.push(3);

// // Réduire la capacité pour optimiser l'utilisation de la mémoire
// stack.shrink_to_fit();

//////////////////////////////////////////////////////////////////////////////////////////////////////
// 11. Échange de piles avec swap

// Softadastra::Stack<int> stack1;
// Softadastra::Stack<int> stack2;

// stack1.push(10);
// stack2.push(20);

// // Échanger les contenus des deux piles
// stack1.swap(stack2);
// std::cout << "Top de stack1 après swap : " << stack1.top() << std::endl; // Affiche 20
// std::cout << "Top de stack2 après swap : " << stack2.top() << std::endl; // Affiche 10

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// 12. Utilisation des itérateurs

// Softadastra::Stack<int> stack;
// stack.push(1);
// stack.push(2);
// stack.push(3);

// // Utilisation des itérateurs pour afficher les éléments
// for (auto it = stack.begin(); it != stack.end(); ++it)
// {
//     std::cout << *it << " ";
// }
// // Affiche 1 2 3
