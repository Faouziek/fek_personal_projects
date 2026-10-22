#include "carte/Carte.hpp"
#include <array>
#include <stdexcept>

namespace cartes {
bool Carte::operator==(const Carte& autre) const {
    return std::tie(type, couleur, valeur) == std::tie(autre.type, autre.couleur, autre.valeur);
}
bool Carte::operator<(const Carte& autre) const {
    return std::tie(type, couleur, valeur) < std::tie(autre.type, autre.couleur, autre.valeur);
}
std::string nomCouleur(Couleur couleur) {
    static const std::array<std::string, 9> noms = {
        "Pique", "Coeur", "Carreau", "Trefle", "Rouge", "Jaune", "Vert", "Bleu", "Sans couleur"};
    return noms.at(static_cast<std::size_t>(couleur));
}
std::string Carte::texte() const {
    if (type == TypeCarte::Excuse) return "Excuse";
    if (type == TypeCarte::Atout) return "Atout " + std::to_string(valeur);
    if (type == TypeCarte::Famille) {
        static const std::array<std::string, 6> membres = {
            "Grand-pere", "Grand-mere", "Pere", "Mere", "Fils", "Fille"};
        return "Famille " + std::to_string(static_cast<int>(couleur) + 1) + " / " + membres.at(static_cast<std::size_t>(valeur));
    }
    if (type == TypeCarte::Uno) {
        if (valeur == Joker) return "Joker";
        if (valeur == Plus4) return "Joker +4";
        std::string nom = std::to_string(valeur);
        if (valeur == Bloque) nom = "Passe";
        if (valeur == Reverse) nom = "Inversion";
        if (valeur == Plus2) nom = "+2";
        return nomCouleur(couleur) + " " + nom;
    }
    std::string nom = std::to_string(valeur);
    if (type == TypeCarte::Tarot) {
        if (valeur == 1) nom = "As";
        if (valeur == 11) nom = "Valet";
        if (valeur == 12) nom = "Cavalier";
        if (valeur == 13) nom = "Dame";
        if (valeur == 14) nom = "Roi";
    } else {
        if (valeur == 11) nom = "Valet";
        if (valeur == 12) nom = "Dame";
        if (valeur == 13) nom = "Roi";
        if (valeur == 14) nom = "As";
    }
    return nom + " de " + nomCouleur(couleur);
}
std::vector<Carte> paquetUno() {
    Main cartes;
    for (int c = 4; c < 8; ++c) {
        const auto couleur = static_cast<Couleur>(c);
        cartes.push_back({TypeCarte::Uno, couleur, 0});
        for (int valeur = 1; valeur <= Plus2; ++valeur)
            for (int copie = 0; copie < 2; ++copie) cartes.push_back({TypeCarte::Uno, couleur, valeur});
    }
    for (int i = 0; i < 4; ++i) {
        cartes.push_back({TypeCarte::Uno, Couleur::Aucune, Joker});
        cartes.push_back({TypeCarte::Uno, Couleur::Aucune, Plus4});
    }
    return cartes;
}
std::vector<Carte> paquetClassique() {
    Main cartes;
    for (int c = 0; c < 4; ++c)
        for (int v = 2; v <= 14; ++v) cartes.push_back({TypeCarte::Classique, static_cast<Couleur>(c), v});
    return cartes;
}
std::vector<Carte> paquetFamilles() {
    Main cartes;
    for (int f = 0; f < 7; ++f)
        for (int m = 0; m < 6; ++m) cartes.push_back({TypeCarte::Famille, static_cast<Couleur>(f), m});
    return cartes;
}
std::vector<Carte> paquetTarot() {
    Main cartes;
    for (int c = 0; c < 4; ++c)
        for (int v = 1; v <= 14; ++v) cartes.push_back({TypeCarte::Tarot, static_cast<Couleur>(c), v});
    for (int v = 1; v <= 21; ++v) cartes.push_back({TypeCarte::Atout, Couleur::Aucune, v});
    cartes.push_back({TypeCarte::Excuse, Couleur::Aucune, 0});
    return cartes;
}
int pointsUno(const Carte& carte) {
    return carte.valeur >= Joker ? 50 : (carte.valeur >= Bloque ? 20 : carte.valeur);
}
bool estBout(const Carte& carte) {
    return carte.type == TypeCarte::Excuse ||
        (carte.type == TypeCarte::Atout && (carte.valeur == 1 || carte.valeur == 21));
}
int pointsTarotDemi(const Carte& carte) {
    if (estBout(carte)) return 9;
    if (carte.type == TypeCarte::Tarot && carte.valeur >= 11) return 2 * (carte.valeur - 10) + 1;
    return 1;
}
} // namespace cartes
