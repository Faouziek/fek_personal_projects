#pragma once
#include "jeu/Jeu.hpp"

namespace cartes {
struct CartePli { int joueur; Carte carte; };
using Pli = std::vector<CartePli>;
std::vector<std::size_t> jouablesTarot(const Main& main, const Pli& pli);
int gagnantPliTarot(const Pli& pli, int preneur = -1, bool excuseChelem = false);
bool petitSec(const Main& main);
std::vector<std::size_t> ecartablesTarot(const Main& main, int restant);
int primePoignee(const Main& main);
double scoreTarot(int pointsDemi, int bouts, int contrat, int petitAuBout,
                  int poignees, bool chelemAnnonce, int plisPreneur);
class JeuTarot final : public Jeu {
public:
    Bilan jouer(IHM& ihm, Aleatoire& rng, const Options& options) override;
};
} // namespace cartes
