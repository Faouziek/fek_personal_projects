#pragma once
#include "carte/Carte.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cartes {
class Quitter : public std::runtime_error {
public:
    Quitter() : std::runtime_error("Partie interrompue.") {}
};

class IHM {
    std::istream& entree_;
    std::ostream& sortie_;
    bool silencieux_;
public:
    IHM(std::istream& entree, std::ostream& sortie, bool silencieux = false)
        : entree_(entree), sortie_(sortie), silencieux_(silencieux) {}
    void dire(const std::string& message);
    int entier(const std::string& invite, int minimum, int maximum);
    std::vector<std::size_t> indices(const std::string& invite, std::size_t taille,
                                     std::size_t maximum);
    void afficherMain(const Main& main, const std::vector<std::size_t>& autorises = {});
    std::ostream& sortie() { return sortie_; }
};
} // namespace cartes
