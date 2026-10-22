#pragma once
#include "carte/Carte.hpp"
#include <string>
#include <utility>

namespace cartes {
struct Joueur {
    std::string nom;
    bool humain = false;
    Main main;
};
} // namespace cartes
