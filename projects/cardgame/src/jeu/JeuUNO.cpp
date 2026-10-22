#include "jeu/JeuUNO.hpp"
#include <array>
#include <numeric>

namespace cartes {
bool jouableUno(const Carte& carte, const Carte& dessus, Couleur active, const Main& main) {
    if (carte.type != TypeCarte::Uno) return false;
    if (carte.valeur == Joker) return true;
    if (carte.valeur == Plus4)
        return std::none_of(main.begin(), main.end(), [active](const Carte& c) { return c.couleur == active; });
    return carte.couleur == active || carte.valeur == dessus.valeur;
}
int suivantUno(int joueur, int sens, int nombre, int pas) {
    if (nombre < 2 || (sens != 1 && sens != -1)) throw std::invalid_argument("Rotation UNO invalide.");
    return ((joueur + sens * pas) % nombre + nombre) % nombre;
}
EffetUno effetUno(const Carte& carte, int sens, int nombre) {
    if (carte.valeur == Reverse) return {-sens, nombre == 2 ? 2 : 1, 0};
    if (carte.valeur == Bloque) return {sens, 2, 0};
    if (carte.valeur == Plus2) return {sens, 2, 2};
    if (carte.valeur == Plus4) return {sens, 2, 4};
    return {sens, 1, 0};
}
Couleur choisirCouleurUno(const Main& main) {
    std::array<int, 4> compte{};
    for (const auto& c : main) {
        const int couleur = static_cast<int>(c.couleur) - 4;
        if (couleur >= 0 && couleur < 4) ++compte[static_cast<std::size_t>(couleur)];
    }
    return static_cast<Couleur>(4 + std::distance(compte.begin(), std::max_element(compte.begin(), compte.end())));
}
namespace {
Couleur demanderCouleur(IHM& ihm, const Joueur& joueur) {
    if (!joueur.humain) return choisirCouleurUno(joueur.main);
    return static_cast<Couleur>(3 + ihm.entier("Couleur : 1 Rouge, 2 Jaune, 3 Vert, 4 Bleu", 1, 4));
}
}
Bilan JeuUNO::jouer(IHM& ihm, Aleatoire& rng, const Options& options) {
    validerOptions(options, 2, 10);
    Bilan bilan{"uno", std::vector<double>(static_cast<std::size_t>(options.joueurs)), 0, 0, 0};
    for (int manche = 0; manche < options.manches; ++manche) {
        auto joueurs = creerJoueurs(options);
        Main pioche = paquetUno(), defausse;
        rng.melanger(pioche);
        for (int c = 0; c < 7; ++c) for (auto& j : joueurs) {
            j.main.push_back(pioche.back()); pioche.pop_back();
        }
        while (pioche.back().valeur == Plus4) rng.melanger(pioche);
        defausse.push_back(pioche.back()); pioche.pop_back();
        int sens = 1;
        const int donneur = manche % options.joueurs;
        int actuel = suivantUno(donneur, sens, options.joueurs);
        Couleur active = defausse.back().couleur;
        if (defausse.back().valeur == Joker) active = demanderCouleur(ihm, joueurs[static_cast<std::size_t>(actuel)]);
        if (defausse.back().valeur == Reverse) { sens = -1; actuel = donneur; }
        if (defausse.back().valeur == Bloque || defausse.back().valeur == Plus2) {
            if (defausse.back().valeur == Plus2)
                for (int k = 0; k < 2; ++k) piocherUno(pioche, defausse, joueurs[static_cast<std::size_t>(actuel)].main, rng);
            actuel = suivantUno(actuel, sens, options.joueurs);
        }
        ihm.dire("\n--- UNO / manche " + std::to_string(manche + 1) + " ---");
        int gagnant = -1, sansMouvement = 0;
        for (int tour = 0; tour < 100000; ++tour) {
            ++bilan.actions;
            auto& joueur = joueurs[static_cast<std::size_t>(actuel)];
            trier(joueur.main);
            std::vector<std::size_t> autorises;
            for (std::size_t i = 0; i < joueur.main.size(); ++i)
                if (jouableUno(joueur.main[i], defausse.back(), active, joueur.main)) autorises.push_back(i);
            ihm.dire("\n" + joueur.nom + " : " + std::to_string(joueur.main.size()) + " cartes. Dessus : " + defausse.back().texte() + "; couleur : " + nomCouleur(active));
            int choix = -1;
            if (joueur.humain) {
                ihm.afficherMain(joueur.main, autorises);
                for (;;) {
                    const int numero = ihm.entier("Carte a jouer (*), 0 = piocher", 0, static_cast<int>(joueur.main.size()));
                    if (numero == 0) break;
                    const auto index = static_cast<std::size_t>(numero - 1);
                    if (std::find(autorises.begin(), autorises.end(), index) != autorises.end()) { choix = numero - 1; break; }
                    ihm.dire("Cette carte n'est pas autorisee.");
                }
            } else if (!autorises.empty()) {
                // Prefer a coloured action card; save wilds for later.
                choix = static_cast<int>(*std::max_element(autorises.begin(), autorises.end(), [&](std::size_t a, std::size_t b) {
                    auto priorite = [&](std::size_t i) { return joueur.main[i].valeur >= Joker ? -1 : pointsUno(joueur.main[i]); };
                    return priorite(a) < priorite(b);
                }));
            }
            bool mouvement = false;
            if (choix < 0) {
                mouvement = piocherUno(pioche, defausse, joueur.main, rng);
                ihm.dire(joueur.nom + (mouvement ? " pioche une carte." : " ne peut plus piocher."));
                if (mouvement && jouableUno(joueur.main.back(), defausse.back(), active, joueur.main)) {
                    if (joueur.humain) ihm.dire("Carte piochee : " + joueur.main.back().texte());
                    if (!joueur.humain || ihm.entier("Jouer cette carte ? 1 oui, 0 non", 0, 1) == 1)
                        choix = static_cast<int>(joueur.main.size()) - 1;
                } else if (mouvement && joueur.humain) ihm.dire("Carte piochee : " + joueur.main.back().texte());
            }
            if (choix >= 0) {
                mouvement = true;
                const Carte carte = retirer(joueur.main, static_cast<std::size_t>(choix));
                defausse.push_back(carte);
                active = carte.valeur >= Joker ? demanderCouleur(ihm, joueur) : carte.couleur;
                ihm.dire(joueur.nom + " joue " + carte.texte() + ".");
                if (joueur.main.size() == 1) ihm.dire(joueur.nom + " : UNO !");
                const auto effet = effetUno(carte, sens, options.joueurs);
                sens = effet.sens;
                if (effet.pioche) {
                    auto& victime = joueurs[static_cast<std::size_t>(suivantUno(actuel, sens, options.joueurs))];
                    for (int k = 0; k < effet.pioche; ++k) piocherUno(pioche, defausse, victime.main, rng);
                    ihm.dire(victime.nom + " pioche " + std::to_string(effet.pioche) + " cartes et passe.");
                }
                if (joueur.main.empty()) gagnant = actuel;
                actuel = suivantUno(actuel, sens, options.joueurs, effet.pas);
            } else actuel = suivantUno(actuel, sens, options.joueurs);
            std::size_t total = pioche.size() + defausse.size();
            for (const auto& j : joueurs) total += j.main.size();
            if (total != 108) throw std::logic_error("Conservation des cartes UNO violee.");
            if (gagnant >= 0) break;
            sansMouvement = mouvement ? 0 : sansMouvement + 1;
            if (sansMouvement >= options.joueurs) { ++bilan.nulles; break; }
            if (tour == 99999) throw std::runtime_error("Limite de tours UNO atteinte.");
        }
        if (gagnant >= 0) {
            int points = 0;
            for (const auto& j : joueurs) for (const auto& c : j.main) points += pointsUno(c);
            bilan.scores[static_cast<std::size_t>(gagnant)] += points;
            ihm.dire(joueurs[static_cast<std::size_t>(gagnant)].nom + " gagne " + std::to_string(points) + " points.");
        } else ihm.dire("Manche bloquee : aucun mouvement possible, aucun point attribue.");
        ++bilan.manches;
    }
    return bilan;
}
} // namespace cartes
