#pragma once
#include "jeu/Jeu.hpp"

namespace cartes {
bool jouableUno(const Carte& carte, const Carte& dessus, Couleur active, const Main& main);
int suivantUno(int joueur, int sens, int nombre, int pas = 1);
struct EffetUno { int sens; int pas; int pioche; };
EffetUno effetUno(const Carte& carte, int sens, int nombre);
Couleur choisirCouleurUno(const Main& main);
class JeuUNO final : public Jeu {
public:
    Bilan jouer(IHM& ihm, Aleatoire& rng, const Options& options) override;
};
} // namespace cartes
