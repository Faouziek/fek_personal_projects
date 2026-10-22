# JeuCartes

Plateforme de jeux de cartes en **C++17**, jouable dans un terminal : **UNO**, **sept familles**, **poker fermé à cinq cartes** et **tarot français à quatre joueurs**.

Un humain joue contre des robots. Un mode entièrement automatique permet de simuler des parties et de reproduire un problème avec une graine aléatoire. Aucune bibliothèque externe n'est nécessaire pour jouer ou exécuter les tests C++.

## Démarrage rapide — Linux / macOS / WSL

Prérequis : compilateur C++17 (GCC ou Clang), GNU Make. Python 3 est utilisé uniquement par les tests de l'interface console.

```sh
make -j2
./bin/jeuCartes
```

Le menu permet de choisir le jeu, le nombre de joueurs et le nombre de manches. Les règles propres à chaque variante sont dans [docs/REGLES.md](docs/REGLES.md). `q` ou la fin de l'entrée interrompt proprement la partie. Les cartes des robots restent cachées jusqu'à l'abattage du poker ; le chien du tarot n'est révélé que lorsque le contrat le permet.

Exemple d'une session UNO :

```text
Vous : 7 cartes. Dessus : Rouge 5; couleur : Rouge
  1. Rouge 2 *
  2. Bleu 5 *
  ...
Carte a jouer (*), 0 = piocher [0..7, q pour quitter] :
```

Les numéros et les cartes ci-dessus sont illustratifs. `*` indique une carte autorisée. Les saisies incorrectes sont refusées sans interrompre la partie.

## Windows avec Visual Studio / CMake

Depuis un terminal développeur disposant d'un compilateur C++17 et de CMake :

```powershell
cmake -S . -B out
cmake --build out --config Release
.\out\Release\jeuCartes.exe
ctest --test-dir out -C Release --output-on-failure
```

Avec un générateur CMake à configuration unique, l'exécutable est directement dans `out` et l'option `-C Release` de CTest peut être omise. Avec WSL ou MSYS2/MinGW et GNU Make, les commandes du démarrage rapide conviennent également.

## Ligne de commande

```sh
# Vous et trois robots, cinq manches
./bin/jeuCartes --game uno --players 4 --rounds 5

# Partie reproductible de sept familles, robots uniquement
./bin/jeuCartes --game familles --players 6 --simulate --seed 42

# Poker : 200 jetons par joueur au départ, jusqu'à 30 donnes
./bin/jeuCartes --game poker --players 4 --simulate --rounds 30 --seed 42

# Tarot : 100 donnes sans journal, avec bilan exploitable en JSON
./bin/jeuCartes --game tarot --simulate --rounds 100 --seed 42 --json

./bin/jeuCartes --help
```

| Option | Valeur / effet |
| --- | --- |
| `--game` | `uno`, `familles`, `poker`, `tarot` |
| `--players` | UNO : 2–10 ; familles et poker : 2–6 ; tarot : 4 |
| `--rounds` | 1–10 000 manches/donnes ; défaut 1 |
| `--humans` | 0 ou 1 ; défaut 1 |
| `--simulate` | Équivalent à `--humans 0` |
| `--seed` | Entier de 0 à 4 294 967 295 ; graine affichée pour rejouer |
| `--quiet` | Journal masqué, bilan final conservé ; simulation uniquement |
| `--json` | Un objet JSON seul ; simulation uniquement |

Sans argument, le menu s'ouvre. Si des options sont fournies, `--game` est requis. Au poker, une session se termine avant la limite si un seul joueur a encore des jetons. Au UNO, le nombre de manches est fixé par l'utilisateur : le classement n'est pas automatiquement arrêté à 500 points.

Le générateur `mt19937` et le mélange de Fisher–Yates utilisent un tirage borné explicite pour rendre le résultat indépendant de l'implémentation STL de `std::shuffle`. Les mêmes options et la même graine reproduisent les simulations. Une partie humaine exige également les mêmes décisions.

## Vérifier le projet

```sh
make test       # Tests C++ et tests de l'interface via Python 3
make smoke      # Tests de l'interface seulement
make sanitize   # Linux : AddressSanitizer + UndefinedBehaviorSanitizer
make clean
```

Alternative CMake :

```sh
cmake -S . -B out -DCMAKE_BUILD_TYPE=Release
cmake --build out
ctest --test-dir out --output-on-failure
```

Les tests vérifient les règles, les départages, les pots secondaires, les scores, les saisies et des parties complètes. La suite de simulation parcourt 100 graines et tous les effectifs autorisés, soit 2 000 sessions. Les invariants de conservation des cartes et des jetons sont contrôlés par les moteurs. Les contrôles C++ lèvent des exceptions en cas d'échec ; ils restent donc actifs en compilation Release.

GitHub Actions est configuré dans [`.github/workflows/ci.yml`](.github/workflows/ci.yml) pour Linux, Windows et macOS, plus un passage des analyseurs mémoire sur Linux. L'exécution de cette CI démarre lorsque le dépôt est envoyé sur GitHub ; sa présence ne signifie pas qu'elle a déjà été exécutée. Le compte rendu des vérifications réalisées avant livraison est dans [docs/VALIDATION.md](docs/VALIDATION.md).

## Organisation

| Répertoire | Responsabilité |
| --- | --- |
| `include/carte`, `src/carte` | Cartes, paquets et valeurs |
| `include/joueur` | Nom, rôle humain/robot, main |
| `include/plateau`, `src/plateau` | Mélange, tirage, recyclage et manipulation des mains |
| `include/jeu`, `src/jeu` | Interface polymorphe et règles des quatre jeux |
| `include/vue`, `src/vue` | Saisies et affichage console |
| `tests` | Tests des règles, simulations et interface |
| `docs` | Règles, architecture, provenance, validation, import GitHub |

Le moteur de chaque jeu prend ses décisions automatiques à partir de sa propre main et des informations publiques. Les robots sont des heuristiques simples ; ils ne constituent pas une intelligence artificielle entraînée ou un adversaire de niveau expert.

## Publication et provenance

Les étapes d'import sont dans [docs/GITHUB.md](docs/GITHUB.md).

Ce projet reprend un projet académique collectif de 2024 crédité **GroupeA6**. La version 1.0 refond les modèles incomplets et développe les moteurs de jeu. Les détails et les auteurs présents dans l'historique source figurent dans [docs/PROVENANCE.md](docs/PROVENANCE.md).

Aucune nouvelle licence de redistribution n'est attribuée aux contributions collectives dans cette archive. [NOTICE.md](NOTICE.md) explique ce point. Les noms des jeux servent à identifier les variantes implémentées ; aucun visuel officiel ni logo n'est fourni.
