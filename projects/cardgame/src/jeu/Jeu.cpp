#include "jeu/Jeu.hpp"
#include "jeu/JeuUNO.hpp"
#include "jeu/Jeu7Fam.hpp"
#include "jeu/JeuPoker.hpp"
#include "jeu/JeuTarot.hpp"

namespace cartes {
void validerOptions(const Options& options, int minimum, int maximum) {
    if (options.joueurs < minimum || options.joueurs > maximum)
        throw std::invalid_argument("Nombre de joueurs attendu : " + std::to_string(minimum) + " a " + std::to_string(maximum));
    if (options.humains < 0 || options.humains > 1 || options.manches < 1 || options.manches > 10000)
        throw std::invalid_argument("Humains : 0 ou 1 ; manches : 1 a 10000.");
}
std::vector<Joueur> creerJoueurs(const Options& options) {
    std::vector<Joueur> joueurs;
    for (int i = 0; i < options.joueurs; ++i)
        joueurs.push_back({i == 0 && options.humains ? "Vous" : "Robot " + std::to_string(i + 1), i == 0 && options.humains != 0, {}});
    return joueurs;
}
std::unique_ptr<Jeu> creerJeu(const std::string& nom) {
    if (nom == "uno") return std::make_unique<JeuUNO>();
    if (nom == "familles") return std::make_unique<Jeu7Fam>();
    if (nom == "poker") return std::make_unique<JeuPoker>();
    if (nom == "tarot") return std::make_unique<JeuTarot>();
    throw std::invalid_argument("Jeu inconnu : " + nom);
}
} // namespace cartes
