#include "jeu/JeuUNO.hpp"
#include "jeu/Jeu7Fam.hpp"
#include "jeu/JeuPoker.hpp"
#include "jeu/JeuTarot.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <set>
#include <sstream>

using namespace cartes;
namespace {
int tests = 0, echecs = 0;
void verifier(bool condition, const std::string& message = "condition non satisfaite") {
    if (!condition) throw std::runtime_error(message);
}
template<class E = std::exception, class F> void doitLever(F fonction) {
    bool leve = false;
    try { fonction(); } catch (const E&) { leve = true; }
    verifier(leve, "exception attendue absente");
}
void test(const std::string& nom, const std::function<void()>& fonction) {
    ++tests;
    try { fonction(); std::cout << "OK " << nom << '\n'; }
    catch (const std::exception& e) { ++echecs; std::cerr << "ECHEC " << nom << " : " << e.what() << '\n'; }
}
Carte uno(int valeur, Couleur couleur = Couleur::Rouge) { return {TypeCarte::Uno, couleur, valeur}; }
Carte couleur(int valeur, Couleur c = Couleur::Pique) { return {TypeCarte::Tarot, c, valeur}; }
Carte atout(int valeur) { return {TypeCarte::Atout, Couleur::Aucune, valeur}; }
Carte excuse() { return {TypeCarte::Excuse, Couleur::Aucune, 0}; }
Main poker(std::initializer_list<int> rangs, bool flush = false) {
    Main main;
    int c = 0;
    for (int v : rangs) { main.push_back({TypeCarte::Classique, static_cast<Couleur>(flush ? 0 : c % 4), v}); ++c; }
    return main;
}
}
int main() {
    test("composition des quatre paquets", [] {
        verifier(paquetUno().size() == 108 && paquetClassique().size() == 52 && paquetFamilles().size() == 42 && paquetTarot().size() == 78);
        for (const auto& paquet : {paquetClassique(), paquetFamilles(), paquetTarot()}) verifier(std::set<Carte>(paquet.begin(), paquet.end()).size() == paquet.size());
        const auto paquet = paquetUno();
        for (int c = 4; c < 8; ++c) verifier(std::count_if(paquet.begin(), paquet.end(), [c](const Carte& carte) { return static_cast<int>(carte.couleur) == c; }) == 25);
        verifier(std::count(paquet.begin(), paquet.end(), uno(0)) == 1);
        verifier(std::count(paquet.begin(), paquet.end(), uno(5)) == 2);
    });
    test("valeurs des cartes et libelles sans ambiguite", [] {
        verifier(pointsUno(uno(9)) == 9 && pointsUno(uno(Reverse)) == 20 && pointsUno(uno(Plus4)) == 50);
        int total = 0, bouts = 0;
        for (const auto& c : paquetTarot()) { total += pointsTarotDemi(c); if (estBout(c)) ++bouts; }
        verifier(total == 182 && bouts == 3);
        verifier(couleur(14).texte() == "Roi de Pique");
        verifier(poker({14, 2, 3, 4, 5})[0].texte() == "As de Pique");
    });
    test("melange reproductible et sans perte", [] {
        Aleatoire a(42), b(42); auto x = paquetTarot(), y = x, origine = x;
        a.melanger(x); b.melanger(y); verifier(x == y && x != origine);
        trier(x); trier(origine); verifier(x == origine);
        doitLever<std::invalid_argument>([&] { a.index(0); });
    });
    test("recyclage UNO conserve le dessus", [] {
        Aleatoire rng(1); Main pioche, defausse{uno(1), uno(2), uno(3)}, main;
        verifier(piocherUno(pioche, defausse, main, rng));
        verifier(defausse == Main{uno(3)} && main.size() == 1 && pioche.size() == 1);
        verifier(piocherUno(pioche, defausse, main, rng));
        verifier(!piocherUno(pioche, defausse, main, rng));
        doitLever<std::out_of_range>([&] { retirer(main, 99); });
    });
    test("UNO couleur, valeur, joker", [] {
        const Main main{uno(7), uno(2, Couleur::Bleu), uno(Joker, Couleur::Aucune)};
        verifier(jouableUno(main[0], uno(1), Couleur::Rouge, main));
        verifier(jouableUno(main[1], uno(2), Couleur::Rouge, main));
        verifier(!jouableUno(main[1], uno(3), Couleur::Rouge, main));
        verifier(jouableUno(main[2], uno(3), Couleur::Rouge, main));
        verifier(!jouableUno(couleur(2), uno(2), Couleur::Rouge, main));
    });
    test("UNO +4 interdit avec couleur active", [] {
        const Carte joker = uno(Plus4, Couleur::Aucune);
        verifier(!jouableUno(joker, uno(5), Couleur::Rouge, {uno(2), joker}));
        verifier(jouableUno(joker, uno(5), Couleur::Rouge, {uno(5, Couleur::Bleu), joker}));
    });
    test("UNO inversion a deux, penalites et rotation", [] {
        const auto e = effetUno(uno(Reverse), 1, 2);
        verifier(e.sens == -1 && e.pas == 2 && suivantUno(0, e.sens, 2, e.pas) == 0);
        verifier(effetUno(uno(Plus2), -1, 4).pioche == 2);
        verifier(effetUno(uno(Plus4), 1, 4).pas == 2);
        verifier(suivantUno(0, -1, 4) == 3);
        verifier(choisirCouleurUno({uno(1, Couleur::Bleu), uno(2, Couleur::Bleu), uno(4)}) == Couleur::Bleu);
    });
    test("familles demandes legales seulement", [] {
        const Main main{{TypeCarte::Famille, Couleur::Pique, 0}, {TypeCarte::Famille, Couleur::Pique, 1}};
        const auto demandes = demandesFamilles(main);
        verifier(demandes.size() == 4);
        for (const auto& d : demandes) verifier(d.famille == 0 && d.membre >= 2);
    });
    test("familles pose complete et sans double comptage", [] {
        Main main;
        for (int m = 0; m < 6; ++m) main.push_back({TypeCarte::Famille, Couleur::Pique, m});
        main.push_back({TypeCarte::Famille, Couleur::Coeur, 0});
        std::array<bool, 7> posees{};
        verifier(poserFamilles(main, posees) == 1 && main.size() == 1 && posees[0]);
        verifier(poserFamilles(main, posees) == 0);
    });
    test("poker classement des neuf categories", [] {
        const std::vector<Main> mains = {
            poker({14, 11, 9, 7, 3}), poker({8, 8, 14, 7, 3}), poker({8, 8, 7, 7, 3}),
            poker({8, 8, 8, 14, 3}), poker({6, 5, 4, 3, 2}), poker({14, 11, 9, 7, 3}, true),
            poker({8, 8, 8, 7, 7}), poker({8, 8, 8, 8, 14}), poker({14, 13, 12, 11, 10}, true)};
        for (std::size_t i = 0; i < mains.size(); ++i) {
            verifier(evaluerPoker(mains[i]).cle[0] == static_cast<int>(i));
            if (i) verifier(evaluerPoker(mains[i - 1]) < evaluerPoker(mains[i]));
        }
    });
    test("poker As bas et tous les departages", [] {
        verifier(evaluerPoker(poker({14, 5, 4, 3, 2})).cle[1] == 5);
        verifier(evaluerPoker(poker({14, 5, 4, 3, 2})) < evaluerPoker(poker({6, 5, 4, 3, 2})));
        verifier(evaluerPoker(poker({8, 8, 14, 7, 3})) < evaluerPoker(poker({8, 8, 14, 9, 2})));
        verifier(evaluerPoker(poker({8, 8, 7, 7, 3})) < evaluerPoker(poker({8, 8, 7, 7, 4})));
        verifier(evaluerPoker(poker({7, 7, 7, 14, 14})) < evaluerPoker(poker({8, 8, 8, 2, 2})));
        verifier(evaluerPoker(poker({8, 8, 8, 8, 2})) < evaluerPoker(poker({8, 8, 8, 8, 3})));
    });
    test("poker mains invalides rejetees", [] {
        doitLever<std::invalid_argument>([] { evaluerPoker({}); });
        auto main = poker({14, 13, 12, 11, 10}); main[1] = main[0];
        doitLever<std::invalid_argument>([&] { evaluerPoker(main); });
    });
    test("poker tapis : pot principal et secondaire", [] {
        const auto a = evaluerPoker(poker({14, 14, 14, 14, 2}));
        const auto b = evaluerPoker(poker({13, 13, 13, 2, 2}));
        const auto c = evaluerPoker(poker({10, 9, 8, 7, 6}));
        verifier(partagerPots({50, 100, 100}, {false, false, false}, {a, b, c}, 0) == std::vector<int>({150, 100, 0}));
    });
    test("poker egalite, jeton impair et joueur couche", [] {
        const auto fort = evaluerPoker(poker({14, 14, 14, 14, 2}));
        const auto faible = evaluerPoker(poker({2, 2, 5, 8, 9}));
        verifier(partagerPots({5, 5, 5}, {false, false, true}, {faible, faible, fort}, 1) == std::vector<int>({7, 8, 0}));
        verifier(partagerPots({100, 50}, {false, true}, {faible, fort}, 0) == std::vector<int>({150, 0}));
        verifier(partagerPots({100, 50}, {true, false}, {fort, faible}, 0) == std::vector<int>({50, 100}));
    });
    test("tarot fournir la couleur avant de couper", [] {
        const Main main{couleur(2), atout(21), excuse(), couleur(14, Couleur::Coeur)};
        verifier(jouablesTarot(main, {{0, couleur(10)}}) == std::vector<std::size_t>({0, 2}));
    });
    test("tarot couper et surcouper obligatoirement", [] {
        const Main main{atout(5), atout(18), excuse(), couleur(14, Couleur::Coeur)};
        verifier(jouablesTarot(main, {{0, couleur(10)}, {1, atout(12)}}) == std::vector<std::size_t>({1, 2}));
        verifier(jouablesTarot(main, {{0, atout(19)}}) == std::vector<std::size_t>({0, 1, 2}));
        verifier(jouablesTarot({couleur(4), couleur(5, Couleur::Coeur)}, {{0, atout(19)}}).size() == 2);
    });
    test("tarot excuse en entame et en dernier pli", [] {
        const Main main{couleur(2), atout(5), excuse()};
        verifier(jouablesTarot(main, {{0, excuse()}}).size() == 3);
        verifier(jouablesTarot(main, {{0, excuse()}, {1, couleur(9)}}) == std::vector<std::size_t>({0, 2}));
        const Pli pli{{0, excuse()}, {1, atout(21)}, {2, couleur(14)}, {3, atout(2)}};
        verifier(gagnantPliTarot(pli) == 1 && gagnantPliTarot(pli, 0, true) == 0);
    });
    test("tarot ordre des cartes et atouts", [] {
        verifier(gagnantPliTarot({{0, couleur(10)}, {1, couleur(14, Couleur::Coeur)}, {2, couleur(11)}, {3, excuse()}}) == 2);
        verifier(gagnantPliTarot({{0, couleur(14)}, {1, atout(3)}, {2, atout(2)}, {3, atout(5)}}) == 3);
    });
    test("tarot petit sec avec et sans excuse", [] {
        verifier(petitSec({atout(1), couleur(2)}));
        verifier(!petitSec({atout(1), excuse(), couleur(2)}));
        verifier(!petitSec({atout(1), atout(2)}));
    });
    test("tarot ecart interdit rois/bouts et atouts forces", [] {
        Main main{couleur(14), atout(1), atout(21), excuse(), couleur(4), atout(2)};
        verifier(ecartablesTarot(main, 6) == std::vector<std::size_t>({4}));
        retirer(main, 4);
        verifier(ecartablesTarot(main, 5) == std::vector<std::size_t>({4}));
    });
    test("tarot niveaux des poignees", [] {
        Main main;
        for (int i = 1; i <= 9; ++i) main.push_back(atout(i));
        verifier(primePoignee(main) == 0); main.push_back(excuse()); verifier(primePoignee(main) == 20);
        main.push_back(atout(10)); main.push_back(atout(11)); main.push_back(atout(12)); verifier(primePoignee(main) == 30);
        main.push_back(atout(13)); main.push_back(atout(14)); verifier(primePoignee(main) == 40);
    });
    test("tarot exemples de marque FFT", [] {
        verifier(scoreTarot(98, 2, 2, 1, 20, false, 9) == 106);
        verifier(scoreTarot(90, 2, 3, -1, 0, false, 9) == 76);
        verifier(scoreTarot(68, 2, 1, 1, 20, false, 9) == -42);
        verifier(scoreTarot(174, 2, 2, 1, 20, true, 18) == 582);
    });
    test("tarot juste fait, chute d'un demi-point et chelems", [] {
        verifier(scoreTarot(82, 2, 1, 0, 0, false, 9) == 25);
        verifier(scoreTarot(81, 2, 1, 0, 0, false, 9) == -25.5);
        verifier(scoreTarot(182, 3, 1, 0, 0, false, 18) == 280);
        verifier(scoreTarot(82, 2, 1, 0, 0, true, 9) == -175);
        doitLever<std::invalid_argument>([] { scoreTarot(183, 2, 1, 0, 0, false, 9); });
    });
    test("saisie entiers invalides et fin de fichier", [] {
        std::istringstream entree("abc\n1.5\n99\n2\n"); std::ostringstream sortie;
        IHM ihm(entree, sortie); verifier(ihm.entier("test", 0, 3) == 2);
        doitLever<Quitter>([&] { ihm.entier("test", 0, 3); });
    });
    test("saisie selection : doublons, debordement, espaces", [] {
        std::istringstream entree("1 1\n0\n99\n1x\n1 3\n\nq\n"); std::ostringstream sortie;
        IHM ihm(entree, sortie);
        verifier(ihm.indices("test", 5, 3) == std::vector<std::size_t>({0, 2}));
        verifier(ihm.indices("test", 5, 3).empty());
        doitLever<Quitter>([&] { ihm.indices("test", 5, 3); });
    });
    test("options invalides rejetees", [] {
        std::istringstream entree; std::ostringstream sortie; IHM ihm(entree, sortie, true); Aleatoire rng(0);
        doitLever<std::invalid_argument>([&] { JeuTarot{}.jouer(ihm, rng, {3, 0, 1}); });
        doitLever<std::invalid_argument>([&] { JeuUNO{}.jouer(ihm, rng, {11, 0, 1}); });
        doitLever<std::invalid_argument>([&] { JeuPoker{}.jouer(ihm, rng, {4, 2, 1}); });
        doitLever<std::invalid_argument>([] { creerJeu("inconnu"); });
    });
    test("parties automatiques : 100 graines et tous effectifs", [] {
        std::istringstream entree; std::ostringstream sortie; IHM ihm(entree, sortie, true);
        int sessions = 0;
        for (std::uint32_t graine = 0; graine < 100; ++graine) {
            for (const std::string nom : {"uno", "familles", "poker", "tarot"}) {
                const int maximum = nom == "uno" ? 10 : (nom == "tarot" ? 4 : 6);
                const int minimum = nom == "tarot" ? 4 : 2;
                for (int n = minimum; n <= maximum; ++n) {
                    Aleatoire rng(graine);
                    const auto bilan = creerJeu(nom)->jouer(ihm, rng, {n, 0, 3});
                    verifier(bilan.manches >= 1 && bilan.manches <= 3 && bilan.actions > 0 && bilan.scores.size() == static_cast<std::size_t>(n));
                    const double somme = std::accumulate(bilan.scores.begin(), bilan.scores.end(), 0.0);
                    if (nom == "tarot") verifier(somme == 0);
                    if (nom == "poker") verifier(somme == 200 * n);
                    if (nom == "familles") verifier(somme == 7 * bilan.manches);
                    if (nom == "uno") verifier(bilan.nulles == 0 && somme > 0);
                    ++sessions;
                }
            }
        }
        verifier(sessions == 2000);
    });
    test("sessions reproductibles", [] {
        std::istringstream entree; std::ostringstream sortie; IHM ihm(entree, sortie, true);
        for (const std::string nom : {"uno", "familles", "poker", "tarot"}) {
            Aleatoire a(123456), b(123456);
            const auto x = creerJeu(nom)->jouer(ihm, a, {4, 0, 20});
            const auto y = creerJeu(nom)->jouer(ihm, b, {4, 0, 20});
            verifier(x.scores == y.scores && x.actions == y.actions && x.manches == y.manches);
        }
    });
    std::cout << tests << " tests, " << echecs << " echec(s).\n";
    return echecs ? 1 : 0;
}
