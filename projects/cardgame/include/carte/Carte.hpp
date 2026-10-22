#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace cartes {

enum class TypeCarte { Classique, Uno, Famille, Tarot, Atout, Excuse };
enum class Couleur { Pique, Coeur, Carreau, Trefle, Rouge, Jaune, Vert, Bleu, Aucune };
enum ValeurUno { Bloque = 10, Reverse = 11, Plus2 = 12, Joker = 13, Plus4 = 14 };

// A card is a value object. UNO duplicates deliberately compare equal.
struct Carte {
    TypeCarte type;
    Couleur couleur;
    int valeur;
    bool operator==(const Carte& autre) const;
    bool operator<(const Carte& autre) const;
    std::string texte() const;
};

using Main = std::vector<Carte>;
std::string nomCouleur(Couleur couleur);
std::vector<Carte> paquetUno();
std::vector<Carte> paquetClassique();
std::vector<Carte> paquetFamilles();
std::vector<Carte> paquetTarot();
int pointsUno(const Carte& carte);
int pointsTarotDemi(const Carte& carte);
bool estBout(const Carte& carte);

} // namespace cartes
