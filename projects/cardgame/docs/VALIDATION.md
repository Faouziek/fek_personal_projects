# Vérifications avant livraison

Vérifications réalisées le 15 octobre 2026, sur Linux, avec GCC 13.3 et Python 3.12.14.

| Vérification | Résultat |
| --- | --- |
| Construction Make / C++17 | Réussie |
| Avertissements `-Wall -Wextra -Wpedantic -Werror` | Aucun avertissement bloquant |
| Suite C++ | 28 tests, aucun échec |
| Simulations incluses dans la suite | 2 000 sessions : 100 graines × tous les effectifs des quatre jeux, jusqu'à trois manches par session |
| Reproductibilité | Résultats identiques avec les mêmes options et graines pour les quatre jeux |
| Tests de ligne de commande et de console | 44 exécutions réussies |
| Sessions interactives complètes pilotées par les invites publiques | UNO, sept familles, poker et deux sessions tarot, dont une avec écart humain |
| AddressSanitizer | Aucun accès mémoire invalide détecté sur les tests exécutés |
| UndefinedBehaviorSanitizer | Aucun comportement indéfini détecté sur les tests exécutés |

Les tests de règles couvrent notamment l'interdiction d'un `+4` illégal, l'inversion à deux joueurs, le recyclage, les familles complètes, les neuf catégories du poker et leurs départages, les pots secondaires et jetons impairs, les obligations de couleur/coupe/surcoupe, l'Excuse, le petit sec, l'écart, les poignées et des exemples de score du règlement FFT. Les moteurs contrôlent en plus leurs invariants pendant les simulations.

Les tests de console vérifient des arguments invalides, les bornes numériques, les sorties JSON, la fin d'entrée, l'abandon volontaire et des parties menées jusqu'au bilan via de vraies saisies dans le programme. Le pilote ne contourne pas les règles ni l'interface.

## Limites de cette validation

La détection des fuites de LeakSanitizer ne peut pas fonctionner dans cet environnement restreint : elle échoue en accédant à `/proc`. Les tests instrumentés ont donc été exécutés avec `ASAN_OPTIONS=detect_leaks=0`. Cela désactive uniquement la recherche de fuites ; les contrôles d'accès mémoire et de comportement indéfini restent actifs. Aucune absence de fuite n'est revendiquée.

CMake, Windows et macOS n'étaient pas disponibles localement. Le chemin Make a été réellement compilé et testé. Le fichier CMake et la matrice GitHub Actions sont fournis pour vérifier les autres plateformes après publication, mais cette CI distante n'a pas encore été exécutée. Le projet ne dépend d'aucune bibliothèque graphique ou de service réseau.

La suite ne constitue pas une preuve formelle de toutes les situations possibles. Les variantes amicales et les automatismes de cette version sont explicités dans `REGLES.md` ; le tarot n'est pas une implémentation certifiée de l'arbitrage FFT et les robots ne sont pas des joueurs experts.
