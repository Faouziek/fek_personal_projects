#pragma once
#include "jeu/Jeu.hpp"
#include <array>

namespace cartes {
struct Demande { int famille; int membre; };
std::vector<Demande> demandesFamilles(const Main& main);
int poserFamilles(Main& main, std::array<bool, 7>& posees);
class Jeu7Fam final : public Jeu {
public:
    Bilan jouer(IHM& ihm, Aleatoire& rng, const Options& options) override;
};
} // namespace cartes
