#include "jeu/JeuPoker.hpp"
#include <map>
#include <numeric>
#include <set>

namespace cartes {
std::string ForcePoker::texte() const {
    static const std::array<std::string, 9> noms = {"Carte haute", "Paire", "Deux paires", "Brelan", "Quinte", "Couleur", "Full", "Carre", "Quinte flush"};
    return noms.at(static_cast<std::size_t>(cle[0]));
}
ForcePoker evaluerPoker(const Main& main) {
    if (main.size() != 5) throw std::invalid_argument("Une main de poker doit contenir 5 cartes.");
    std::map<int, int> compte;
    std::set<Carte> uniques;
    for (const auto& c : main) {
        if (c.type != TypeCarte::Classique || c.valeur < 2 || c.valeur > 14 || static_cast<int>(c.couleur) > 3 || !uniques.insert(c).second)
            throw std::invalid_argument("Main de poker invalide.");
        ++compte[c.valeur];
    }
    std::vector<int> rangs;
    for (const auto& c : main) rangs.push_back(c.valeur);
    std::sort(rangs.rbegin(), rangs.rend());
    const bool couleur = std::all_of(main.begin(), main.end(), [&](const Carte& c) { return c.couleur == main.front().couleur; });
    int quinte = 0;
    if (compte.size() == 5 && rangs.front() - rangs.back() == 4) quinte = rangs.front();
    if (rangs == std::vector<int>{14, 5, 4, 3, 2}) quinte = 5;
    std::vector<std::pair<int, int>> groupes;
    for (const auto& entree : compte) groupes.emplace_back(entree.second, entree.first);
    std::sort(groupes.rbegin(), groupes.rend());
    ForcePoker force;
    auto& cle = force.cle;
    if (quinte && couleur) { cle[0] = 8; cle[1] = quinte; }
    else if (groupes[0].first == 4) { cle[0] = 7; cle[1] = groupes[0].second; cle[2] = groupes[1].second; }
    else if (groupes[0].first == 3 && groupes[1].first == 2) { cle[0] = 6; cle[1] = groupes[0].second; cle[2] = groupes[1].second; }
    else if (couleur) { cle[0] = 5; std::copy(rangs.begin(), rangs.end(), cle.begin() + 1); }
    else if (quinte) { cle[0] = 4; cle[1] = quinte; }
    else if (groupes[0].first == 3) { cle[0] = 3; for (std::size_t i = 0; i < groupes.size(); ++i) cle[i + 1] = groupes[i].second; }
    else if (groupes[0].first == 2 && groupes[1].first == 2) { cle[0] = 2; for (std::size_t i = 0; i < groupes.size(); ++i) cle[i + 1] = groupes[i].second; }
    else if (groupes[0].first == 2) { cle[0] = 1; for (std::size_t i = 0; i < groupes.size(); ++i) cle[i + 1] = groupes[i].second; }
    else std::copy(rangs.begin(), rangs.end(), cle.begin() + 1);
    return force;
}
std::vector<int> partagerPots(const std::vector<int>& contributions, const std::vector<bool>& couches,
                             const std::vector<ForcePoker>& forces, int premier) {
    const auto nombre = contributions.size();
    if (nombre < 2 || couches.size() != nombre || forces.size() != nombre || premier < 0 || static_cast<std::size_t>(premier) >= nombre)
        throw std::invalid_argument("Donnees de pot invalides.");
    std::set<int> niveaux;
    for (const int c : contributions) { if (c < 0) throw std::invalid_argument("Contribution negative."); if (c) niveaux.insert(c); }
    std::vector<int> gains(nombre);
    int precedent = 0;
    for (const int niveau : niveaux) {
        int participants = 0, seul = -1;
        std::vector<int> eligibles;
        for (std::size_t j = 0; j < nombre; ++j) if (contributions[j] >= niveau) {
            ++participants; seul = static_cast<int>(j);
            if (!couches[j]) eligibles.push_back(static_cast<int>(j));
        }
        const int pot = (niveau - precedent) * participants;
        precedent = niveau;
        if (participants == 1) { gains[static_cast<std::size_t>(seul)] += pot; continue; } // Uncalled part.
        if (eligibles.empty()) throw std::logic_error("Pot sans joueur eligible.");
        const auto meilleur = *std::max_element(eligibles.begin(), eligibles.end(), [&](int a, int b) { return forces[static_cast<std::size_t>(a)] < forces[static_cast<std::size_t>(b)]; });
        eligibles.erase(std::remove_if(eligibles.begin(), eligibles.end(), [&](int j) { return !(forces[static_cast<std::size_t>(j)] == forces[static_cast<std::size_t>(meilleur)]); }), eligibles.end());
        const int part = pot / static_cast<int>(eligibles.size());
        int reste = pot % static_cast<int>(eligibles.size());
        for (const int j : eligibles) gains[static_cast<std::size_t>(j)] += part;
        for (std::size_t offset = 0; offset < nombre && reste; ++offset) {
            const int j = (premier + static_cast<int>(offset)) % static_cast<int>(nombre);
            if (std::find(eligibles.begin(), eligibles.end(), j) != eligibles.end()) { ++gains[static_cast<std::size_t>(j)]; --reste; }
        }
    }
    return gains;
}
namespace {
void miser(IHM& ihm, Aleatoire& rng, const std::vector<Joueur>& joueurs, std::vector<int>& jetons,
           std::vector<int>& contributions, std::vector<bool>& couches, int premier, int unite, int& actions) {
    const std::size_t n = joueurs.size();
    std::vector<int> mises(n);
    std::vector<bool> attendre(n, true), peutRelancer(n, true);
    int niveau = 0, relances = 0;
    auto vivants = [&] { return std::count(couches.begin(), couches.end(), false); };
    auto payer = [&](std::size_t j, int montant) {
        jetons[j] -= montant; contributions[j] += montant; mises[j] += montant;
    };
    for (int pas = 0; ; ++pas) {
        if (pas > 10000) throw std::logic_error("Encheres poker non terminees.");
        if (vivants() <= 1) return;
        bool reste = false;
        for (std::size_t j = 0; j < n; ++j) if (!couches[j] && jetons[j] > 0 && attendre[j]) reste = true;
        if (!reste) return;
        const auto j = static_cast<std::size_t>((premier + pas) % static_cast<int>(n));
        if (couches[j] || jetons[j] == 0 || !attendre[j]) continue;
        const int aPayer = niveau - mises[j];
        bool adversaireActif = false;
        for (std::size_t k = 0; k < n; ++k) if (k != j && !couches[k] && jetons[k] > 0) adversaireActif = true;
        if (!adversaireActif && aPayer == 0) { attendre[j] = false; continue; }
        const bool relancePossible = adversaireActif && peutRelancer[j] && relances < 3 && jetons[j] > aPayer;
        int choix = 1;
        if (joueurs[j].humain) {
            ihm.dire("\nVotre main :"); ihm.afficherMain(joueurs[j].main);
            ihm.dire("Jetons : " + std::to_string(jetons[j]) + "; a suivre : " + std::to_string(aPayer) + "; pot : " + std::to_string(std::accumulate(contributions.begin(), contributions.end(), 0)));
            choix = ihm.entier("0 se coucher, 1 suivre/parole" + std::string(relancePossible ? ", 2 miser/relancer de " + std::to_string(unite) : ""), 0, relancePossible ? 2 : 1);
        } else {
            const int categorie = evaluerPoker(joueurs[j].main).cle[0];
            if (aPayer > 0 && categorie == 0 && rng.index(4) == 0) choix = 0;
            else if (relancePossible && (categorie >= 2 || (categorie == 1 && rng.index(3) == 0))) choix = 2;
        }
        ++actions;
        attendre[j] = false;
        peutRelancer[j] = false;
        if (choix == 0) { couches[j] = true; ihm.dire(joueurs[j].nom + " se couche."); continue; }
        if (choix == 1) {
            const int montant = std::min(aPayer, jetons[j]); payer(j, montant);
            ihm.dire(joueurs[j].nom + (montant ? " suit pour " + std::to_string(montant) : " fait parole.") + (jetons[j] == 0 ? " (tapis)" : ""));
        } else {
            const int montant = std::min(aPayer + unite, jetons[j]);
            const int augmentation = mises[j] + montant - niveau;
            const int ancienNiveau = niveau;
            payer(j, montant); niveau = mises[j];
            const bool complete = augmentation >= unite;
            if (complete && ancienNiveau > 0) ++relances;
            for (std::size_t k = 0; k < n; ++k) if (k != j && !couches[k] && jetons[k] > 0) {
                if (mises[k] < niveau) attendre[k] = true;
                if (complete) peutRelancer[k] = true;
            }
            ihm.dire(joueurs[j].nom + " porte la mise a " + std::to_string(niveau) + (jetons[j] == 0 ? " (tapis)." : "."));
        }
    }
}
}
Bilan JeuPoker::jouer(IHM& ihm, Aleatoire& rng, const Options& options) {
    validerOptions(options, 2, 6);
    Bilan bilan{"poker", std::vector<double>(static_cast<std::size_t>(options.joueurs), 200), 0, 0, 0};
    auto joueurs = creerJoueurs(options);
    const auto n = joueurs.size();
    std::vector<int> jetons(n, 200);
    int donneur = options.joueurs - 1;
    for (int manche = 0; manche < options.manches; ++manche) {
        if (std::count_if(jetons.begin(), jetons.end(), [](int v) { return v > 0; }) < 2) break;
        do { donneur = (donneur + 1) % options.joueurs; } while (jetons[static_cast<std::size_t>(donneur)] == 0);
        const int premier = (donneur + 1) % options.joueurs;
        Main pioche = paquetClassique(), defausse;
        rng.melanger(pioche);
        std::vector<int> contributions(n);
        std::vector<bool> couches(n);
        for (std::size_t j = 0; j < n; ++j) {
            joueurs[j].main.clear(); couches[j] = jetons[j] == 0;
            contributions[j] = std::min(10, jetons[j]); jetons[j] -= contributions[j];
        }
        for (int c = 0; c < 5; ++c) for (std::size_t k = 0; k < n; ++k) {
            const auto j = (static_cast<std::size_t>(premier) + k) % n;
            if (!couches[j]) { joueurs[j].main.push_back(pioche.back()); pioche.pop_back(); }
        }
        for (auto& j : joueurs) trier(j.main);
        ihm.dire("\n--- Poker ferme / donne " + std::to_string(manche + 1) + " (ante 10) ---");
        miser(ihm, rng, joueurs, jetons, contributions, couches, premier, 10, bilan.actions);
        if (std::count(couches.begin(), couches.end(), false) > 1) {
            for (std::size_t k = 0; k < n; ++k) {
                const auto j = (static_cast<std::size_t>(premier) + k) % n;
                if (couches[j]) continue;
                auto& main = joueurs[j].main;
                std::vector<std::size_t> selection;
                if (joueurs[j].humain) {
                    ihm.dire("\nEchange :"); ihm.afficherMain(main);
                    selection = ihm.indices("Cartes a remplacer (0 a 5)", 5, 5);
                } else if (evaluerPoker(main).cle[0] < 4) {
                    std::map<int, int> compte;
                    for (const auto& c : main) ++compte[c.valeur];
                    for (std::size_t i = 0; i < main.size(); ++i) if (compte[main[i].valeur] == 1 && (compte.size() < 5 || main[i].valeur < 12)) selection.push_back(i);
                }
                Main cartesEcartees;
                for (auto it = selection.rbegin(); it != selection.rend(); ++it) cartesEcartees.push_back(retirer(main, *it));
                for (std::size_t i = 0; i < selection.size(); ++i) {
                    if (pioche.empty()) { pioche.swap(defausse); rng.melanger(pioche); }
                    if (pioche.empty()) throw std::logic_error("Paquet poker epuise.");
                    main.push_back(pioche.back()); pioche.pop_back();
                }
                // Never draw one's own discards during the same exchange.
                defausse.insert(defausse.end(), cartesEcartees.begin(), cartesEcartees.end());
                trier(main);
                ihm.dire(joueurs[j].nom + " remplace " + std::to_string(selection.size()) + " carte(s).");
            }
            miser(ihm, rng, joueurs, jetons, contributions, couches, premier, 20, bilan.actions);
        }
        std::vector<ForcePoker> forces(n);
        const int restants = static_cast<int>(std::count(couches.begin(), couches.end(), false));
        for (std::size_t j = 0; j < n; ++j) if (!couches[j]) {
            forces[j] = evaluerPoker(joueurs[j].main);
            if (restants > 1) {
                std::string cartes;
                for (const auto& c : joueurs[j].main) cartes += c.texte() + "; ";
                ihm.dire(joueurs[j].nom + " : " + forces[j].texte() + " / " + cartes);
            }
        }
        const auto gains = partagerPots(contributions, couches, forces, premier);
        for (std::size_t j = 0; j < n; ++j) {
            jetons[j] += gains[j];
            if (gains[j]) ihm.dire(joueurs[j].nom + " recoit " + std::to_string(gains[j]) + " jetons (gains et restitutions).");
        }
        std::size_t cartes = pioche.size() + defausse.size();
        for (const auto& j : joueurs) cartes += j.main.size();
        if (cartes != 52 || std::accumulate(jetons.begin(), jetons.end(), 0) != options.joueurs * 200)
            throw std::logic_error("Conservation des cartes ou jetons poker violee.");
        ++bilan.manches;
    }
    for (std::size_t j = 0; j < n; ++j) bilan.scores[j] = jetons[j];
    return bilan;
}
} // namespace cartes
