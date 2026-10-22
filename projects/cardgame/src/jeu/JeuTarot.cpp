#include "jeu/JeuTarot.hpp"
#include <array>
#include <cmath>
#include <numeric>
#include <set>
#include <sstream>

namespace cartes {
std::vector<std::size_t> jouablesTarot(const Main& main, const Pli& pli) {
    const auto premiere = std::find_if(pli.begin(), pli.end(), [](const CartePli& p) { return p.carte.type != TypeCarte::Excuse; });
    std::vector<std::size_t> autorises;
    if (premiere == pli.end()) {
        for (std::size_t i = 0; i < main.size(); ++i) autorises.push_back(i);
        return autorises;
    }
    const Carte demande = premiere->carte;
    const bool couleur = demande.type == TypeCarte::Tarot && std::any_of(main.begin(), main.end(), [&](const Carte& c) { return c.type == TypeCarte::Tarot && c.couleur == demande.couleur; });
    const bool atout = std::any_of(main.begin(), main.end(), [](const Carte& c) { return c.type == TypeCarte::Atout; });
    int plusHaut = 0;
    for (const auto& p : pli) if (p.carte.type == TypeCarte::Atout) plusHaut = std::max(plusHaut, p.carte.valeur);
    const bool surcoupe = std::any_of(main.begin(), main.end(), [plusHaut](const Carte& c) { return c.type == TypeCarte::Atout && c.valeur > plusHaut; });
    for (std::size_t i = 0; i < main.size(); ++i) {
        const auto& c = main[i];
        if (c.type == TypeCarte::Excuse ||
            (couleur && c.type == TypeCarte::Tarot && c.couleur == demande.couleur) ||
            (!couleur && atout && c.type == TypeCarte::Atout && (!surcoupe || c.valeur > plusHaut)) ||
            (!couleur && !atout)) autorises.push_back(i);
    }
    return autorises;
}
int gagnantPliTarot(const Pli& pli, int preneur, bool excuseChelem) {
    if (pli.empty()) throw std::invalid_argument("Pli vide.");
    if (excuseChelem) for (const auto& p : pli)
        if (p.joueur == preneur && p.carte.type == TypeCarte::Excuse) return preneur;
    const auto premiere = std::find_if(pli.begin(), pli.end(), [](const CartePli& p) { return p.carte.type != TypeCarte::Excuse; });
    if (premiere == pli.end()) throw std::invalid_argument("Pli sans couleur ni atout.");
    CartePli meilleure = *premiere;
    for (const auto& p : pli) {
        if (p.carte.type == TypeCarte::Excuse) continue;
        if ((p.carte.type == TypeCarte::Atout && meilleure.carte.type != TypeCarte::Atout) ||
            (p.carte.type == meilleure.carte.type && p.carte.couleur == meilleure.carte.couleur && p.carte.valeur > meilleure.carte.valeur)) meilleure = p;
    }
    return meilleure.joueur;
}
bool petitSec(const Main& main) {
    return std::count_if(main.begin(), main.end(), [](const Carte& c) { return c.type == TypeCarte::Atout; }) == 1 &&
        std::any_of(main.begin(), main.end(), [](const Carte& c) { return c.type == TypeCarte::Atout && c.valeur == 1; }) &&
        std::none_of(main.begin(), main.end(), [](const Carte& c) { return c.type == TypeCarte::Excuse; });
}
std::vector<std::size_t> ecartablesTarot(const Main& main, int restant) {
    if (restant <= 0) return {};
    std::vector<std::size_t> couleurs, atouts;
    for (std::size_t i = 0; i < main.size(); ++i) {
        const auto& c = main[i];
        if (c.type == TypeCarte::Tarot && c.valeur != 14) couleurs.push_back(i);
        if (c.type == TypeCarte::Atout && !estBout(c)) atouts.push_back(i);
    }
    // If forced to discard trumps, first discard every eligible suited card.
    return couleurs.empty() ? atouts : couleurs;
}
int primePoignee(const Main& main) {
    const auto nombre = std::count_if(main.begin(), main.end(), [](const Carte& c) { return c.type == TypeCarte::Atout || c.type == TypeCarte::Excuse; });
    return nombre >= 15 ? 40 : (nombre >= 13 ? 30 : (nombre >= 10 ? 20 : 0));
}
double scoreTarot(int pointsDemi, int bouts, int contrat, int petitAuBout, int poignees, bool chelemAnnonce, int plisPreneur) {
    static const std::array<int, 4> seuils = {56, 51, 41, 36};
    static const std::array<int, 5> coefficients = {0, 1, 2, 4, 6};
    if (pointsDemi < 0 || pointsDemi > 182 || bouts < 0 || bouts > 3 || contrat < 1 || contrat > 4 || std::abs(petitAuBout) > 1 || poignees < 0 || plisPreneur < 0 || plisPreneur > 18)
        throw std::invalid_argument("Score tarot invalide.");
    const double difference = pointsDemi / 2.0 - seuils[static_cast<std::size_t>(bouts)];
    const double signe = difference >= 0 ? 1.0 : -1.0;
    double score = (signe * (25.0 + std::abs(difference)) + petitAuBout * 10) * coefficients[static_cast<std::size_t>(contrat)] + signe * poignees;
    if (chelemAnnonce) score += plisPreneur == 18 ? 400 : -200;
    else if (plisPreneur == 18) score += 200;
    else if (plisPreneur == 0) score -= 200;
    return score;
}
namespace {
const std::array<std::string, 5> contrats = {"Passe", "Prise", "Garde", "Garde sans", "Garde contre"};
int forceAnnonce(const Main& main) {
    int force = 0;
    for (const auto& c : main) {
        if (c.type == TypeCarte::Atout) force += 1 + (c.valeur >= 16 ? 1 : 0);
        if (estBout(c)) force += 4;
        if (c.type == TypeCarte::Tarot && c.valeur == 14) force += 2;
    }
    return force;
}
std::string nombre(double valeur) { std::ostringstream flux; flux << valeur; return flux.str(); }
}
Bilan JeuTarot::jouer(IHM& ihm, Aleatoire& rng, const Options& options) {
    validerOptions(options, 4, 4);
    Bilan bilan{"tarot", std::vector<double>(4), 0, 0, 0};
    int donneur = 3;
    for (int manche = 0; manche < options.manches; ++manche) {
        std::vector<Joueur> joueurs;
        Main chien;
        int contrat = 0, preneur = -1;
        for (int essai = 0; contrat == 0; ++essai) {
            if (essai >= 10000) throw std::runtime_error("Trop de donnes passees au tarot.");
            donneur = (donneur + 1) % 4;
            joueurs = creerJoueurs(options); chien.clear();
            Main pioche = paquetTarot(); rng.melanger(pioche);
            std::vector<int> positions(23); std::iota(positions.begin(), positions.end(), 0); rng.melanger(positions);
            std::set<int> positionsChien(positions.begin(), positions.begin() + 6);
            for (int paquet = 0; paquet < 24; ++paquet) {
                auto& main = joueurs[static_cast<std::size_t>((donneur + 1 + paquet) % 4)].main;
                for (int c = 0; c < 3; ++c) { main.push_back(pioche.back()); pioche.pop_back(); }
                if (positionsChien.count(paquet)) { chien.push_back(pioche.back()); pioche.pop_back(); }
            }
            if (!pioche.empty() || chien.size() != 6) throw std::logic_error("Distribution tarot invalide.");
            bool annulee = false;
            for (auto& j : joueurs) { trier(j.main); annulee = annulee || petitSec(j.main); }
            if (annulee) { ihm.dire("Petit sec : nouvelle distribution."); continue; }
            ihm.dire("\n--- Tarot / donne " + std::to_string(manche + 1) + " ---");
            ihm.dire("Ordre de jeu : numeros croissants, places disposees dans le sens antihoraire.");
            for (int k = 0; k < 4; ++k) {
                const auto j = static_cast<std::size_t>((donneur + 1 + k) % 4);
                int annonce = 0;
                if (joueurs[j].humain) {
                    ihm.dire("Votre main :"); ihm.afficherMain(joueurs[j].main);
                    for (;;) {
                        annonce = ihm.entier("Enchere : 0 passe, 1 prise, 2 garde, 3 sans, 4 contre (enchere actuelle : " + contrats[static_cast<std::size_t>(contrat)] + ")", 0, 4);
                        if (annonce == 0 || annonce > contrat) break;
                        ihm.dire("Il faut annoncer un contrat superieur ou passer.");
                    }
                } else {
                    const int force = forceAnnonce(joueurs[j].main);
                    const int souhait = force >= 32 ? 4 : (force >= 27 ? 3 : (force >= 21 ? 2 : (force >= 14 ? 1 : 0)));
                    if (souhait > contrat) annonce = souhait;
                }
                ++bilan.actions;
                ihm.dire(joueurs[j].nom + " : " + contrats[static_cast<std::size_t>(annonce)]);
                if (annonce) { contrat = annonce; preneur = static_cast<int>(j); }
            }
            if (contrat == 0) ihm.dire("Tout le monde passe : nouvelle distribution.");
        }
        auto& mainPreneur = joueurs[static_cast<std::size_t>(preneur)].main;
        Main plisAttaque, plisDefense, atoutsEcartes;
        if (contrat <= 2) {
            std::string texteChien = "Chien : ";
            for (const auto& c : chien) texteChien += c.texte() + "; ";
            ihm.dire(texteChien);
            mainPreneur.insert(mainPreneur.end(), chien.begin(), chien.end());
            for (int restant = 6; restant > 0; --restant) {
                trier(mainPreneur);
                const auto autorises = ecartablesTarot(mainPreneur, restant);
                if (autorises.empty()) throw std::logic_error("Ecart tarot impossible.");
                std::size_t choix = autorises.front();
                if (joueurs[static_cast<std::size_t>(preneur)].humain) {
                    ihm.dire("Ecart : encore " + std::to_string(restant) + " carte(s). Seules les cartes * sont autorisees.");
                    ihm.afficherMain(mainPreneur, autorises);
                    for (;;) {
                        choix = static_cast<std::size_t>(ihm.entier("Carte a ecarter", 1, static_cast<int>(mainPreneur.size())) - 1);
                        if (std::find(autorises.begin(), autorises.end(), choix) != autorises.end()) break;
                        ihm.dire("Roi, bout ou atout non force : ecart interdit.");
                    }
                } else {
                    choix = *std::min_element(autorises.begin(), autorises.end(), [&](std::size_t a, std::size_t b) { return mainPreneur[a].valeur < mainPreneur[b].valeur; });
                }
                Carte carte = retirer(mainPreneur, choix);
                plisAttaque.push_back(carte);
                if (carte.type == TypeCarte::Atout) {
                    atoutsEcartes.push_back(carte);
                    ihm.dire("Atout ecarte obligatoirement, montre a tous : " + carte.texte());
                }
            }
        } else if (contrat == 3) plisAttaque = chien;
        else plisDefense = chien;
        chien.clear();
        bool chelemAnnonce = false;
        if (joueurs[static_cast<std::size_t>(preneur)].humain)
            chelemAnnonce = ihm.entier("Annoncer un chelem (tous les plis) ? 1 oui, 0 non", 0, 1) == 1;
        int entame = chelemAnnonce ? preneur : (donneur + 1) % 4;
        int plisPreneur = 0, petitAuBout = 0, petitAvantDernier = 0, primes = 0, compensation = 0;
        for (int numero = 0; numero < 18; ++numero) {
            Pli pli;
            ihm.dire("\nPli " + std::to_string(numero + 1));
            for (int k = 0; k < 4; ++k) {
                const int j = (entame + k) % 4;
                auto& joueur = joueurs[static_cast<std::size_t>(j)];
                if (numero == 0) {
                    Main poignee = joueur.main;
                    if (j == preneur) poignee.insert(poignee.end(), atoutsEcartes.begin(), atoutsEcartes.end());
                    const int maximum = primePoignee(poignee);
                    int prime = maximum;
                    if (maximum && joueur.humain) {
                        const int niveauMax = maximum == 40 ? 3 : (maximum == 30 ? 2 : 1);
                        const int niveau = ihm.entier("Poignee : 0 aucune, 1 simple (20), 2 double (30), 3 triple (40)", 0, niveauMax);
                        prime = niveau == 0 ? 0 : (niveau == 1 ? 20 : (niveau == 2 ? 30 : 40));
                    }
                    if (prime) {
                        const int compte = prime == 40 ? 15 : (prime == 30 ? 13 : 10);
                        Main presentation;
                        for (const auto& c : poignee) if (c.type == TypeCarte::Atout) presentation.push_back(c);
                        trier(presentation);
                        if (static_cast<int>(presentation.size()) < compte) {
                            const auto excuse = std::find_if(poignee.begin(), poignee.end(), [](const Carte& c) { return c.type == TypeCarte::Excuse; });
                            if (excuse == poignee.end()) throw std::logic_error("Poignee invalide.");
                            presentation.push_back(*excuse);
                        }
                        std::string annonce = joueur.nom + " presente une poignee (" + std::to_string(prime) + ") : ";
                        for (int i = 0; i < compte; ++i) annonce += presentation[static_cast<std::size_t>(i)].texte() + "; ";
                        ihm.dire(annonce); primes += prime;
                    }
                }
                trier(joueur.main);
                const auto autorises = jouablesTarot(joueur.main, pli);
                if (autorises.empty()) throw std::logic_error("Aucune carte tarot jouable.");
                std::size_t choix = autorises.front();
                if (joueur.humain) {
                    ihm.dire("Votre main (* = autorisee) :"); ihm.afficherMain(joueur.main, autorises);
                    for (;;) {
                        choix = static_cast<std::size_t>(ihm.entier("Carte", 1, static_cast<int>(joueur.main.size())) - 1);
                        if (std::find(autorises.begin(), autorises.end(), choix) != autorises.end()) break;
                        ihm.dire("Fournissez la couleur, sinon coupez et surcoupez si possible. L'Excuse reste autorisee.");
                    }
                } else {
                    // A small, non-omniscient strategy using only this hand and this trick.
                    const bool pliOuvert = std::any_of(pli.begin(), pli.end(), [](const CartePli& p) { return p.carte.type != TypeCarte::Excuse; });
                    const bool partenaireMaitre = pliOuvert && j != preneur && gagnantPliTarot(pli) != preneur;
                    int meilleur = -100000;
                    for (const auto i : autorises) {
                        const auto& c = joueur.main[i];
                        Pli essai = pli; essai.push_back({j, c});
                        int valeur = -c.valeur;
                        if (c.type == TypeCarte::Excuse) valeur = numero >= 12 && numero < 17 ? 100 : -100;
                        else if (partenaireMaitre) valeur = pointsTarotDemi(c) * 10 - c.valeur;
                        else if (gagnantPliTarot(essai) == j) valeur += 40;
                        if (valeur > meilleur) { meilleur = valeur; choix = i; }
                    }
                }
                const Carte carte = retirer(joueur.main, choix);
                pli.push_back({j, carte}); ++bilan.actions;
                ihm.dire(joueur.nom + " joue " + carte.texte());
            }
            int excuseJoueur = -1;
            for (const auto& p : pli) if (p.carte.type == TypeCarte::Excuse) excuseJoueur = p.joueur;
            const bool excuseChelem = numero == 17 && excuseJoueur >= 0 &&
                ((excuseJoueur == preneur && plisPreneur == 17) || (excuseJoueur != preneur && plisPreneur == 0));
            const int gagnant = gagnantPliTarot(pli, excuseJoueur, excuseChelem);
            if (gagnant == preneur) ++plisPreneur;
            const bool petit = std::any_of(pli.begin(), pli.end(), [](const CartePli& p) { return p.carte.type == TypeCarte::Atout && p.carte.valeur == 1; });
            if (numero == 16 && petit) petitAvantDernier = gagnant == preneur ? 1 : -1;
            if (numero == 17 && petit) petitAuBout = gagnant == preneur ? 1 : -1;
            if (excuseChelem && petitAvantDernier) petitAuBout = petitAvantDernier;
            for (const auto& p : pli) {
                int beneficiaire = gagnant;
                if (p.carte.type == TypeCarte::Excuse && numero < 17) {
                    beneficiaire = p.joueur;
                    if ((p.joueur == preneur) != (gagnant == preneur)) compensation += p.joueur == preneur ? -1 : 1;
                }
                (beneficiaire == preneur ? plisAttaque : plisDefense).push_back(p.carte);
            }
            ihm.dire(joueurs[static_cast<std::size_t>(gagnant)].nom + " remporte le pli.");
            entame = gagnant;
        }
        int pointsDemi = compensation, bouts = 0;
        for (const auto& c : plisAttaque) { pointsDemi += pointsTarotDemi(c); if (estBout(c)) ++bouts; }
        int totalDemi = 0;
        for (const auto& c : plisAttaque) totalDemi += pointsTarotDemi(c);
        for (const auto& c : plisDefense) totalDemi += pointsTarotDemi(c);
        std::set<Carte> uniques(plisAttaque.begin(), plisAttaque.end());
        uniques.insert(plisDefense.begin(), plisDefense.end());
        if (plisAttaque.size() + plisDefense.size() != 78 || uniques.size() != 78 || totalDemi != 182)
            throw std::logic_error("Conservation des cartes ou points tarot violee.");
        const double score = scoreTarot(pointsDemi, bouts, contrat, petitAuBout, primes, chelemAnnonce, plisPreneur);
        for (int j = 0; j < 4; ++j) bilan.scores[static_cast<std::size_t>(j)] += j == preneur ? 3 * score : -score;
        ihm.dire(joueurs[static_cast<std::size_t>(preneur)].nom + " / " + contrats[static_cast<std::size_t>(contrat)] + " : " + nombre(pointsDemi / 2.0) + " points, " + std::to_string(bouts) + " bout(s), " + std::to_string(plisPreneur) + " plis. Marque : " + nombre(3 * score));
        ++bilan.manches;
    }
    return bilan;
}
} // namespace cartes
