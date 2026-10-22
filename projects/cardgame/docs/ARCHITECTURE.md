# Architecture

## De l'ébauche au projet jouable

L'archive initiale séparait déjà cartes, joueurs, jeux, plateau et vues. Elle contenait des getters/setters et beaucoup de méthodes non implémentées. La version actuelle conserve cette séparation conceptuelle mais remplace l'API incomplète.

`Carte` est un objet valeur comparable. Les types distinguent notamment les cartes ordinaires du poker et celles du tarot : un rang 14 désigne ainsi un As au poker et un Roi au tarot, sans ambiguïté d'affichage. Les doublons du UNO sont intentionnels ; l'identité physique est portée par les positions dans les conteneurs.

`Joueur` est une donnée commune contenant nom, main et rôle humain/robot. Les sous-classes de joueurs ne sont plus nécessaires : les décisions spécifiques vivent avec les règles du jeu. Aucun joueur dérivé n'est copié dans une base, ce qui supprime le slicing de l'ébauche.

`Jeu` fournit une interface abstraite `jouer` et un destructeur virtuel. Une fabrique construit les quatre implémentations. Les fonctions de règles pures sont exposées dans les en-têtes pour être testées séparément : légalité au UNO, familles posables, force des mains/pots au poker, cartes autorisées et score au tarot.

`IHM` encapsule les flux standard. Les choix sont validés avant mutation ; les mains adverses ne sont pas présentées à l'utilisateur. Les demandes EOF et `q` lèvent `Quitter`, interceptée à l'entrée du programme. Les erreurs d'invariant remontent avec un code de sortie non nul.

## Invariants

- UNO : 108 cartes réparties entre les mains, la pioche et la défausse.
- Familles : 42 cartes réparties entre les mains, la pioche et les familles posées.
- Poker : 52 cartes ; jetons totaux constants entre les joueurs après chaque donne. Les pots sont décomposés par niveaux de contribution, indépendamment des forces des mains.
- Tarot : 78 cartes distinctes, 182 demi-points au décompte, scores à somme nulle. L'Excuse reste physiquement dans son camp d'attribution ; la compensation est ajoutée séparément.

Les limites défensives de tours lèvent une erreur explicite ; elles n'inventent jamais un vainqueur. Le UNO dispose en plus d'une règle explicite de manche bloquée sans mouvement.

## Robots

Les règles ne dépendent pas d'un moteur externe. Les robots utilisent des heuristiques modestes : cartes d'action/couleur dominante au UNO, mémoire publique et progression des familles, combinaisons propres au poker, force de main et situation du pli au tarot. Le mélange et les décisions aléatoires partagent un générateur explicite pour reproduire les sessions automatiques. Ils n'accèdent pas aux mains adverses pour choisir une action.

## Construction

Le Makefile crée les répertoires à la demande et suit les dépendances d'en-têtes avec `-MMD -MP`. Le `main` du programme et celui des tests sont liés dans deux exécutables distincts. Les objets instrumentés ont leur propre répertoire. CMake fournit une autre voie de construction, notamment pour Visual Studio. Les tests C++ n'utilisent pas CppUnit ; ils n'exigent donc aucune installation de framework.

Pour ajouter une variante : créer une implémentation de `Jeu`, l'enregistrer dans `creerJeu` et le menu, ajouter ses tests et préciser ses règles. Il n'est pas nécessaire de créer de nouvelles sous-classes de joueurs ou d'IHM pour chaque variante.
