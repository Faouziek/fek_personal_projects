#include "jeu/Jeu7Fam.hpp"
#include <cstdint>
#include <numeric>

namespace cartes {
std::vector<Demande> demandesFamilles(const Main& main) {
    std::array<std::array<bool, 6>, 7> cartes{};
    for (const auto& c : main) cartes.at(static_cast<std::size_t>(c.couleur)).at(static_cast<std::size_t>(c.valeur)) = true;
    std::vector<Demande> demandes;
    for (int f = 0; f < 7; ++f) {
        const auto& famille = cartes[static_cast<std::size_t>(f)];
        if (std::any_of(famille.begin(), famille.end(), [](bool v) { return v; }))
            for (int m = 0; m < 6; ++m) if (!famille[static_cast<std::size_t>(m)]) demandes.push_back({f, m});
    }
    return demandes;
}
int poserFamilles(Main& main, std::array<bool, 7>& posees) {
    int resultat = 0;
    for (int f = 0; f < 7; ++f) {
        std::array<bool, 6> membres{};
        for (const auto& c : main) if (static_cast<int>(c.couleur) == f) membres.at(static_cast<std::size_t>(c.valeur)) = true;
        if (!posees[static_cast<std::size_t>(f)] && std::all_of(membres.begin(), membres.end(), [](bool v) { return v; })) {
            main.erase(std::remove_if(main.begin(), main.end(), [f](const Carte& c) { return static_cast<int>(c.couleur) == f; }), main.end());
            posees[static_cast<std::size_t>(f)] = true;
            ++resultat;
        }
    }
    return resultat;
}
Bilan Jeu7Fam::jouer(IHM& ihm, Aleatoire& rng, const Options& options) {
    validerOptions(options, 2, 6);
    Bilan bilan{"familles", std::vector<double>(static_cast<std::size_t>(options.joueurs)), 0, 0, 0};
    for (int manche = 0; manche < options.manches; ++manche) {
        auto joueurs = creerJoueurs(options);
        Main pioche = paquetFamilles(); rng.melanger(pioche);
        std::array<bool, 7> posees{};
        std::array<int, 42> proprietaire; proprietaire.fill(-1);
        std::array<std::uint32_t, 42> absences{};
        std::vector<int> points(static_cast<std::size_t>(options.joueurs));
        auto piocher = [&](int joueur) {
            if (pioche.empty()) return false;
            auto& j = joueurs[static_cast<std::size_t>(joueur)];
            j.main.push_back(pioche.back()); pioche.pop_back();
            for (auto& masque : absences) masque &= ~(std::uint32_t{1} << joueur);
            if (j.humain) ihm.dire("Vous piochez : " + j.main.back().texte());
            return true;
        };
        auto poser = [&](int joueur) {
            const int nombre = poserFamilles(joueurs[static_cast<std::size_t>(joueur)].main, posees);
            points[static_cast<std::size_t>(joueur)] += nombre;
            if (nombre) ihm.dire(joueurs[static_cast<std::size_t>(joueur)].nom + " pose " + std::to_string(nombre) + " famille(s) !");
        };
        for (int c = 0; c < 6; ++c) for (int j = 0; j < options.joueurs; ++j) piocher(j);
        for (int j = 0; j < options.joueurs; ++j) poser(j);
        int actuel = manche % options.joueurs;
        ihm.dire("\n--- Sept familles / manche " + std::to_string(manche + 1) + " ---");
        for (int tour = 0; std::count(posees.begin(), posees.end(), true) < 7; ++tour) {
            if (tour >= 100000) throw std::runtime_error("Limite de tours familles atteinte.");
            ++bilan.actions;
            auto& joueur = joueurs[static_cast<std::size_t>(actuel)];
            if (joueur.main.empty()) { piocher(actuel); poser(actuel); }
            if (joueur.main.empty()) { actuel = (actuel + 1) % options.joueurs; continue; }
            trier(joueur.main);
            const auto demandes = demandesFamilles(joueur.main);
            if (demandes.empty()) throw std::logic_error("Famille complete non posee.");
            std::vector<int> adversaires;
            for (int j = 0; j < options.joueurs; ++j)
                if (j != actuel && !joueurs[static_cast<std::size_t>(j)].main.empty()) adversaires.push_back(j);
            if (adversaires.empty()) {
                if (!piocher(actuel)) throw std::logic_error("Cartes familles inaccessibles.");
                poser(actuel); continue;
            }
            int cible = -1;
            Demande demande = demandes.front();
            if (joueur.humain) {
                ihm.dire("\nVotre main :"); ihm.afficherMain(joueur.main);
                for (const int j : adversaires) ihm.dire(std::to_string(j + 1) + " = " + joueurs[static_cast<std::size_t>(j)].nom + " (" + std::to_string(joueurs[static_cast<std::size_t>(j)].main.size()) + " cartes)");
                do { cible = ihm.entier("A qui demander ?", 1, options.joueurs) - 1; }
                while (std::find(adversaires.begin(), adversaires.end(), cible) == adversaires.end());
                for (;;) {
                    demande.famille = ihm.entier("Famille", 1, 7) - 1;
                    demande.membre = ihm.entier("Membre : 1 grand-pere, 2 grand-mere, 3 pere, 4 mere, 5 fils, 6 fille", 1, 6) - 1;
                    if (std::any_of(demandes.begin(), demandes.end(), [&](const Demande& d) { return d.famille == demande.famille && d.membre == demande.membre; })) break;
                    ihm.dire("Possedez une carte de la famille, sans deja posseder la carte demandee.");
                }
            } else {
                struct Choix { Demande demande; int cible; int priorite; };
                std::vector<Choix> choix;
                int meilleur = -1;
                for (const auto& d : demandes) for (const int j : adversaires) {
                    const auto id = static_cast<std::size_t>(d.famille * 6 + d.membre);
                    if (absences[id] & (std::uint32_t{1} << j)) continue;
                    const int groupe = static_cast<int>(std::count_if(joueur.main.begin(), joueur.main.end(), [&](const Carte& c) { return static_cast<int>(c.couleur) == d.famille; }));
                    const int priorite = (proprietaire[id] == j ? 100 : 0) + groupe;
                    meilleur = std::max(meilleur, priorite);
                    choix.push_back({d, j, priorite});
                }
                choix.erase(std::remove_if(choix.begin(), choix.end(), [meilleur](const Choix& c) { return c.priorite < meilleur; }), choix.end());
                if (choix.empty()) { // Missing cards are still in the draw pile.
                    demande = demandes[rng.index(demandes.size())];
                    cible = adversaires[rng.index(adversaires.size())];
                } else {
                    const auto& selection = choix[rng.index(choix.size())];
                    demande = selection.demande; cible = selection.cible;
                }
            }
            const Carte carte{TypeCarte::Famille, static_cast<Couleur>(demande.famille), demande.membre};
            const auto id = static_cast<std::size_t>(demande.famille * 6 + demande.membre);
            auto& mainCible = joueurs[static_cast<std::size_t>(cible)].main;
            const auto trouve = std::find(mainCible.begin(), mainCible.end(), carte);
            ihm.dire(joueur.nom + " demande " + carte.texte() + " a " + joueurs[static_cast<std::size_t>(cible)].nom + ".");
            bool rejouer = false;
            if (trouve != mainCible.end()) {
                joueur.main.push_back(*trouve); mainCible.erase(trouve);
                proprietaire[id] = actuel; absences[id] = 0;
                ihm.dire("Carte obtenue : le joueur rejoue."); rejouer = true;
            } else {
                absences[id] |= std::uint32_t{1} << cible;
                if (piocher(actuel)) {
                    rejouer = joueur.main.back() == carte;
                    if (rejouer) { proprietaire[id] = actuel; absences[id] = 0; }
                    ihm.dire(rejouer ? "Bonne pioche ! Le joueur rejoue." : "Mauvaise pioche.");
                } else ihm.dire("Carte absente, pioche vide.");
            }
            poser(actuel);
            std::size_t total = pioche.size() + 6 * static_cast<std::size_t>(std::count(posees.begin(), posees.end(), true));
            for (const auto& j : joueurs) total += j.main.size();
            if (total != 42) throw std::logic_error("Conservation des cartes familles violee.");
            if (!rejouer) actuel = cible;
        }
        for (int j = 0; j < options.joueurs; ++j) {
            bilan.scores[static_cast<std::size_t>(j)] += points[static_cast<std::size_t>(j)];
            ihm.dire(joueurs[static_cast<std::size_t>(j)].nom + " : " + std::to_string(points[static_cast<std::size_t>(j)]) + " famille(s).");
        }
        ++bilan.manches;
    }
    return bilan;
}
} // namespace cartes
