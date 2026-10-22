#include "plateau/Plateau.hpp"
#include <limits>
#include <stdexcept>

namespace cartes {
std::size_t Aleatoire::index(std::size_t taille) {
    if (taille == 0 || taille > std::numeric_limits<std::uint32_t>::max())
        throw std::invalid_argument("Taille aleatoire invalide.");
    const auto borne = static_cast<std::uint32_t>(taille);
    const std::uint32_t seuil = (std::uint32_t{0} - borne) % borne;
    std::uint32_t valeur;
    do { valeur = static_cast<std::uint32_t>(moteur_()); } while (valeur < seuil);
    return valeur % borne;
}
Carte retirer(Main& main, std::size_t index) {
    const Carte carte = main.at(index);
    main.erase(main.begin() + static_cast<std::ptrdiff_t>(index));
    return carte;
}
void trier(Main& main) { std::sort(main.begin(), main.end()); }
bool piocherUno(Main& pioche, Main& defausse, Main& destination, Aleatoire& rng) {
    if (pioche.empty() && defausse.size() > 1) {
        const Carte dessus = defausse.back();
        defausse.pop_back();
        pioche.swap(defausse);
        defausse.push_back(dessus);
        rng.melanger(pioche);
    }
    if (pioche.empty()) return false;
    destination.push_back(pioche.back());
    pioche.pop_back();
    return true;
}
} // namespace cartes
