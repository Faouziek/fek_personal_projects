#pragma once
#include "carte/Carte.hpp"
#include <algorithm>
#include <cstdint>
#include <random>

namespace cartes {
class Aleatoire {
    std::mt19937 moteur_;
public:
    explicit Aleatoire(std::uint32_t graine) : moteur_(graine) {}
    std::size_t index(std::size_t taille);
    template<class T> void melanger(std::vector<T>& elements) {
        // Explicit Fisher-Yates and rejection sampling: stable across STL versions.
        for (std::size_t n = elements.size(); n > 1; --n)
            std::swap(elements[n - 1], elements[index(n)]);
    }
};

Carte retirer(Main& main, std::size_t index);
void trier(Main& main);
// Keep the top discard in place; recycle all older cards only if needed.
bool piocherUno(Main& pioche, Main& defausse, Main& destination, Aleatoire& rng);
} // namespace cartes
