#include "vue/IHM.hpp"
#include <algorithm>
#include <set>
#include <sstream>

namespace cartes {
void IHM::dire(const std::string& message) {
    if (!silencieux_) sortie_ << message << '\n';
}
int IHM::entier(const std::string& invite, int minimum, int maximum) {
    if (minimum > maximum) throw std::logic_error("Intervalle de saisie vide.");
    for (;;) {
        sortie_ << invite << " [" << minimum << ".." << maximum << ", q pour quitter] : " << std::flush;
        std::string ligne;
        if (!std::getline(entree_, ligne)) throw Quitter();
        if (ligne == "q" || ligne == "Q" || ligne == "quit") throw Quitter();
        std::istringstream flux(ligne);
        int valeur;
        std::string reste;
        if ((flux >> valeur) && !(flux >> reste) && valeur >= minimum && valeur <= maximum) return valeur;
        sortie_ << "Saisie invalide. Entrez un entier dans l'intervalle indique.\n";
    }
}
std::vector<std::size_t> IHM::indices(const std::string& invite, std::size_t taille, std::size_t maximum) {
    for (;;) {
        sortie_ << invite << " (numeros separes par des espaces, Entree = aucune, q = quitter) : " << std::flush;
        std::string ligne;
        if (!std::getline(entree_, ligne)) throw Quitter();
        if (ligne == "q" || ligne == "Q") throw Quitter();
        std::istringstream flux(ligne);
        std::set<std::size_t> selection;
        bool valide = true;
        std::string mot;
        while (flux >> mot) {
            std::istringstream nombre(mot);
            int indice;
            char reste;
            if (!(nombre >> indice) || (nombre >> reste) || indice < 1 || static_cast<std::size_t>(indice) > taille) {
                valide = false; break;
            }
            if (!selection.insert(static_cast<std::size_t>(indice - 1)).second) { valide = false; break; }
        }
        if (valide && selection.size() <= maximum) return {selection.begin(), selection.end()};
        sortie_ << "Selection invalide : numeros uniques, au plus " << maximum << " cartes.\n";
    }
}
void IHM::afficherMain(const Main& main, const std::vector<std::size_t>& autorises) {
    if (silencieux_) return;
    for (std::size_t i = 0; i < main.size(); ++i) {
        sortie_ << "  " << i + 1 << ". " << main[i].texte();
        if (!autorises.empty() && std::find(autorises.begin(), autorises.end(), i) != autorises.end()) sortie_ << " *";
        sortie_ << '\n';
    }
}
} // namespace cartes
