# Importer le projet dans GitHub

## Dépôt dédié

1. Décompresser l'archive. Le dossier contenant `README.md`, `Makefile` et `CMakeLists.txt` est la racine du projet.
2. Compiler et tester avec les commandes du README.
3. Sur GitHub, créer un dépôt vide nommé par exemple `jeucartes-cpp`. Pour ce premier import, ne pas générer automatiquement de README, de gitignore ou de licence dans le dépôt distant.
4. Depuis la racine locale :

```sh
git init -b main
git add .
git status
git commit -m "Complete the C++ card game platform"
git remote add origin https://github.com/VOTRE_IDENTIFIANT/jeucartes-cpp.git
git push -u origin main
```

Remplacer `VOTRE_IDENTIFIANT` par votre identifiant réel. GitHub demandera l'authentification selon votre configuration. Aucun secret ni jeton ne doit être ajouté aux fichiers du projet.

Le `.gitignore` exclut les objets, exécutables, résultats de compilation, archives et réglages locaux. Le workflow de tests est inclus : ouvrir l'onglet Actions après le premier push pour consulter les résultats sur chaque système.

## Dans un dépôt regroupant plusieurs projets

Copier le dossier `jeucartes` dans un sous-dossier de ce dépôt, sans y faire `git init`. Puis, depuis la racine de ce dépôt, utiliser `git add`, `git commit` et `git push`.

Le workflow livré suppose que JeuCartes est à la racine. Si le projet est dans `projets/jeucartes`, déplacer le workflow vers le `.github/workflows` du dépôt parent et adapter les chemins/working-directory des commandes. Il est souvent plus simple de conserver un dépôt dédié et de le référencer depuis un dépôt portfolio.

## Présentation

Description possible : « Plateforme de quatre jeux de cartes en C++17 : règles, robots, interface console, tests et simulations reproductibles. Reprise d'un projet académique collectif. »

Conserver le fichier de provenance et compléter, si nécessaire, la description de votre contribution personnelle au projet de 2024. Les variantes et adaptations sont documentées dans `docs/REGLES.md`. Une licence explicite devra être choisie par les titulaires des contributions avant d'autoriser la réutilisation sous cette licence.

L'archive ne crée aucun dépôt distant et n'effectue aucun push automatiquement.
