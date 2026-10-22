# Provenance et contributions

## Origine

Ce projet est issu de l'archive académique `jeucartes(1).zip`, datée d'avril 2024. Les en-têtes créditaient **GroupeA6**. Le HEAD du dépôt inclus dans l'archive était `7f82a64`.

L'historique initial contient les identités d'auteurs suivantes (alias conservés tels qu'ils apparaissent, sans adresses électroniques) : Paul Mehaud, Lucas Joubin, Bsevellec, acerezo, gauffret, Joubin Lucas, faouzi, “Brendan, Arthur Cerezo, El Kahla Faouzi, Mehaud Paul, lucas7582. Plusieurs identités peuvent correspondre à une même personne ; cette liste n'essaie pas de les fusionner ni d'attribuer des contributions non vérifiées.

## Reprise — version 1.0.0, octobre 2026

La reprise a été demandée par Faouzi El Kahla et réalisée avec l'assistance de ChatGPT/Codex. Elle comprend les quatre moteurs jouables, la refonte des modèles, l'interface console, la construction, les tests et la documentation. Elle ne doit pas être présentée comme une réalisation intégrale de 2024 ni comme un projet entièrement individuel.

L'ancienne documentation générée et les tests CppUnit incomplets ne sont pas inclus dans la distribution courante : ils décrivaient une API abandonnée. L'ancien dossier Git, sa configuration distante et les exécutables précompilés sont exclus de l'archive de livraison. L'archive source initiale et son historique demeurent la référence pour les contributions académiques antérieures.

La séparation conceptuelle cartes / joueurs / plateau / jeux / vue a été conservée. Le code actuel remplace largement les méthodes incomplètes, plutôt que de simuler leur fonctionnement avec des retours provisoires.
