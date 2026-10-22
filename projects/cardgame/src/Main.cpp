#include "jeu/Jeu.hpp"
#include <charconv>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>

namespace {
std::uint32_t entierArgument(const std::string& texte) {
    std::uint32_t valeur = 0;
    const auto resultat = std::from_chars(texte.data(), texte.data() + texte.size(), valeur);
    if (texte.empty() || resultat.ec != std::errc{} || resultat.ptr != texte.data() + texte.size())
        throw std::invalid_argument("Entier positif invalide : " + texte);
    return valeur;
}
int petitArgument(const std::string& texte) {
    const auto valeur = entierArgument(texte);
    if (valeur > 10000) throw std::invalid_argument("Argument numerique trop grand : " + texte);
    return static_cast<int>(valeur);
}
void afficherBilan(const cartes::Bilan& bilan, const cartes::Options& options, std::uint32_t graine, bool json) {
    if (json) {
        std::cout << "{\"game\":\"" << bilan.jeu << "\",\"seed\":" << graine << ",\"rounds\":" << bilan.manches
                  << ",\"actions\":" << bilan.actions << ",\"draws\":" << bilan.nulles << ",\"scores\":[";
        for (std::size_t i = 0; i < bilan.scores.size(); ++i) std::cout << (i ? "," : "") << bilan.scores[i];
        std::cout << "]}\n";
    } else {
        std::cout << "\n=== Resultat " << bilan.jeu << " / graine " << graine << " ===\n";
        const auto joueurs = cartes::creerJoueurs(options);
        for (std::size_t i = 0; i < bilan.scores.size(); ++i) std::cout << joueurs[i].nom << " : " << bilan.scores[i] << (bilan.jeu == "poker" ? " jetons" : (bilan.jeu == "familles" ? " familles" : " points")) << '\n';
        const auto meilleur = *std::max_element(bilan.scores.begin(), bilan.scores.end());
        std::cout << "En tete : ";
        bool premier = true;
        for (std::size_t i = 0; i < bilan.scores.size(); ++i) if (bilan.scores[i] == meilleur) {
            std::cout << (premier ? "" : ", ") << joueurs[i].nom; premier = false;
        }
        std::cout << "\nManches terminees : " << bilan.manches << "; actions : " << bilan.actions;
        if (bilan.nulles) std::cout << "; manches bloquees : " << bilan.nulles;
        std::cout << '\n';
    }
}
void aide() {
    std::cout << R"(JeuCartes - plateforme C++17, interface console

Sans argument : menu interactif (vous contre des robots).
  --game uno|familles|poker|tarot
  --players N     UNO 2..10, familles/poker 2..6, tarot 4 (defaut 4)
  --rounds N      Nombre de manches/donnes, 1..10000 (defaut 1)
  --seed N        Graine reproductible, 0..4294967295
  --simulate      Robots uniquement (equivaut a --humans 0)
  --humans 0|1    Un humain au maximum (defaut 1)
  --quiet         Afficher uniquement le bilan, en simulation
  --json          Bilan JSON seul, en simulation
  --help          Cette aide
  --version       Version du projet

Exemple : bin/jeuCartes --game tarot --simulate --rounds 20 --seed 42 --json
En jeu : q ou fin d'entree pour quitter proprement.
Regles et variantes : docs/REGLES.md
)";
}
}
int main(int argc, char* argv[]) {
    try {
        std::string nom;
        cartes::Options options;
        std::uint32_t graine = std::random_device{}();
        bool silencieux = false, json = false;
        for (int i = 1; i < argc; ++i) {
            const std::string argument = argv[i];
            auto valeur = [&]() -> std::string {
                if (++i >= argc) throw std::invalid_argument("Valeur manquante pour " + argument);
                return argv[i];
            };
            if (argument == "--help" || argument == "-h") { aide(); return 0; }
            if (argument == "--version") { std::cout << "JeuCartes 1.0.0\n"; return 0; }
            if (argument == "--game") nom = valeur();
            else if (argument == "--players") options.joueurs = petitArgument(valeur());
            else if (argument == "--rounds") options.manches = petitArgument(valeur());
            else if (argument == "--seed") graine = entierArgument(valeur());
            else if (argument == "--humans") options.humains = petitArgument(valeur());
            else if (argument == "--simulate") options.humains = 0;
            else if (argument == "--quiet") silencieux = true;
            else if (argument == "--json") { json = true; silencieux = true; }
            else throw std::invalid_argument("Option inconnue : " + argument);
        }
        if (silencieux && options.humains != 0) throw std::invalid_argument("--quiet et --json necessitent --simulate ou --humans 0.");
        if (nom.empty() && argc > 1) throw std::invalid_argument("Precisez --game ou lancez sans argument pour ouvrir le menu.");
        cartes::IHM ihm(std::cin, std::cout, silencieux);
        if (!nom.empty()) {
            auto jeu = cartes::creerJeu(nom);
            if (!json) std::cout << "Graine : " << graine << '\n';
            cartes::Aleatoire rng(graine);
            const auto bilan = jeu->jouer(ihm, rng, options);
            afficherBilan(bilan, options, graine, json);
        } else {
            const std::vector<std::string> noms = {"", "uno", "familles", "poker", "tarot"};
            for (;;) {
                ihm.dire("\nJEUCARTES\n1 UNO\n2 Sept familles\n3 Poker ferme a 5 cartes\n4 Tarot a 4 joueurs\n0 Quitter");
                const int choix = ihm.entier("Jeu", 0, 4);
                if (choix == 0) break;
                options.joueurs = choix == 4 ? 4 : ihm.entier("Nombre de joueurs (vous compris)", 2, choix == 1 ? 10 : 6);
                options.manches = ihm.entier("Nombre de manches / donnes", 1, 100);
                cartes::Aleatoire rng(graine);
                ihm.dire("Graine de cette session : " + std::to_string(graine));
                const auto bilan = cartes::creerJeu(noms[static_cast<std::size_t>(choix)])->jouer(ihm, rng, options);
                afficherBilan(bilan, options, graine, false);
                ++graine;
            }
        }
        return 0;
    } catch (const cartes::Quitter& fin) {
        std::cout << '\n' << fin.what() << '\n';
        return 0;
    } catch (const std::exception& erreur) {
        std::cerr << "Erreur : " << erreur.what() << '\n';
        return 1;
    }
}
