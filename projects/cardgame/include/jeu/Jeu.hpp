#pragma once
#include "joueur/Joueur.hpp"
#include "plateau/Plateau.hpp"
#include "vue/IHM.hpp"
#include <memory>

namespace cartes {
struct Options {
    int joueurs = 4;
    int humains = 1;
    int manches = 1;
};
struct Bilan {
    std::string jeu;
    std::vector<double> scores;
    int manches = 0;
    int actions = 0;
    int nulles = 0;
};
std::vector<Joueur> creerJoueurs(const Options& options);
void validerOptions(const Options& options, int minimum, int maximum);
class Jeu {
public:
    virtual ~Jeu() = default;
    virtual Bilan jouer(IHM& ihm, Aleatoire& rng, const Options& options) = 0;
};
std::unique_ptr<Jeu> creerJeu(const std::string& nom);
} // namespace cartes
