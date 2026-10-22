#pragma once
#include "jeu/Jeu.hpp"
#include <array>

namespace cartes {
// Category followed by all tie breakers, most significant first.
struct ForcePoker {
    std::array<int, 6> cle{};
    bool operator<(const ForcePoker& autre) const { return cle < autre.cle; }
    bool operator==(const ForcePoker& autre) const { return cle == autre.cle; }
    std::string texte() const;
};
ForcePoker evaluerPoker(const Main& main);
std::vector<int> partagerPots(const std::vector<int>& contributions,
    const std::vector<bool>& couches, const std::vector<ForcePoker>& forces, int premier);
class JeuPoker final : public Jeu {
public:
    Bilan jouer(IHM& ihm, Aleatoire& rng, const Options& options) override;
};
} // namespace cartes
