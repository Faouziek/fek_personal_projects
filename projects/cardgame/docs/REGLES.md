# Règles et variantes retenues

Les quatre jeux sont jouables du début au calcul du résultat dans un terminal, avec un humain et des robots, ou uniquement des robots. Les conventions de cette version sont explicites ci-dessous. Les robots ne connaissent pas les mains adverses. Les numéros de siège du tarot suivent l'ordre antihoraire ; ils augmentent dans l'ordre de jeu.

## UNO — paquet classique de 108 cartes

- Sept cartes par joueur ; correspondance de couleur ou de valeur/symbole, jokers compris.
- `Passe`, `Inversion`, `+2`, joker et `+4` sont appliqués. À deux joueurs, une inversion rend la main à son auteur.
- Une pénalité fait piocher puis passer le joueur suivant. Les `+2` et `+4` ne s'empilent pas.
- Le `+4` est interdit si sa main contient une carte de la couleur active. La légalité est imposée par le logiciel ; il n'y a donc pas de bluff ni de contestation du `+4`.
- Il est permis de piocher volontairement. Après la pioche, seule la carte qui vient d'être tirée peut être jouée, si elle est légale.
- Le logiciel annonce automatiquement « UNO » lorsqu'il reste une carte : aucune pénalité d'oubli d'annonce n'est utilisée.
- Une carte d'action retournée au départ applique son effet. Un `+4` est remis dans le paquet ; un autre départ est tiré. Une inversion initiale fait commencer le donneur dans l'autre sens.
- La défausse est remélangée si nécessaire en conservant la carte du dessus.
- Une manche se termine quand une main est vide. La pénalité d'une dernière carte `+2` ou `+4` s'applique avant le score.
- Le gagnant marque les cartes restantes : nombres à leur valeur, actions à 20, jokers à 50. Le classement cumule les manches demandées ; cette version n'impose pas l'objectif de 500 points.
- Si toutes les cartes sont en main et aucun joueur ne peut jouer ou piocher pendant un tour de table complet, la manche est déclarée bloquée, sans score. Cette convention évite une boucle sans mouvement.

Référence du paquet et des effets : [notice Mattel W2085](https://service.mattel.com/instruction_sheets/W2085-UNO.pdf). Les automatismes d'annonce et la limitation par nombre de manches sont des conventions de l'application.

## Sept familles — 42 cartes

Sept familles numérotées, avec six membres chacune : grand-père, grand-mère, père, mère, fils, fille. Les noms de familles personnels de l'ébauche ont été remplacés par des numéros.

Six cartes sont distribuées à chaque joueur ; le reste forme la pioche. On demande une carte précise à un adversaire ayant encore des cartes. Il faut déjà posséder une carte de la famille et ne pas détenir celle que l'on demande. L'adversaire remet la carte s'il la possède et le demandeur rejoue. Sinon le demandeur pioche : il rejoue uniquement si c'est exactement la carte demandée (« bonne pioche »). Après un échec, la main passe au joueur interrogé. Les joueurs sans cartes piochent une carte à leur tour, si possible ; sinon ils sont sautés.

Une famille complète est posée automatiquement et vaut un point. La partie se termine lorsque les sept familles ont été posées. Les égalités sont conservées. Plusieurs manches cumulent le nombre de familles.

Les robots mémorisent les transferts annoncés et les demandes infructueuses. Après une pioche cachée, ils ne supposent plus que le joueur concerné est dépourvu de la carte demandée précédemment.

Référence d'une variante avec « bonne pioche » et main au joueur interrogé : [Géopolis, jeu des sept systèmes cristallins](https://www.geopolis.fr/boutique/jeux-crees-par-geopolis/401-jeu-des-7-familles-les-7-systemes-cristallins.html). La distribution de six cartes est la convention choisie ici.

## Poker fermé — cinq cartes, jetons fictifs

Il s'agit de poker fermé, pas de Texas Hold'em. Chaque joueur commence la session avec 200 jetons. Le donneur tourne à chaque donne parmi les joueurs qui ont des jetons. Chaque participant verse une ante de 10, limitée à son tapis ; cinq cartes sont distribuées.

1. Premier tour de mises, incrément fixe de 10.
2. Un échange de zéro à cinq cartes pour chaque joueur encore en jeu.
3. Second tour de mises, incrément fixe de 20.
4. Abattage et attribution des pots ; si tous les autres se couchent, la main restante gagne sans révéler ses cartes.

Chaque tour de mises commence après le donneur. Il permet de faire parole, suivre, se coucher ou relancer. Il autorise une ouverture puis au plus trois relances complètes. Une relance à tapis inférieure à l'incrément ne rouvre pas le droit de relancer aux joueurs ayant déjà parlé ; ils peuvent suivre le complément ou se coucher. Un joueur seul avec des jetons face à des tapis ne peut pas créer une mise sans adversaire capable de répondre.

Les tapis sont admis : chaque joueur ne peut gagner que la partie des pots couverte par sa contribution. Les contributions non suivies sont restituées. Les égalités partagent le pot ; les jetons indivisibles sont attribués dans l'ordre des sièges après le donneur. Les cartes écartées par un joueur ne peuvent pas lui revenir pendant son propre échange. Si nécessaire, les défausses des joueurs précédents sont remélangées.

Ordre des combinaisons : carte haute, paire, deux paires, brelan, quinte, couleur, full, carré, quinte flush. Les cinq rangs pertinents départagent les mains ; les couleurs ne départagent jamais une égalité. L'As peut former `A-2-3-4-5` ou `10-V-D-R-A`, sans autre bouclage.

La session se termine après le nombre de donnes demandé, ou lorsqu'un seul joueur conserve des jetons. Le classement porte sur les jetons restants. Aucun argent réel, achat ou pari extérieur n'est prévu.

Référence de la variante et des rangs : [PokerStars, Five Card Draw](https://www.pokerstars.fr/en/poker/games/draw/). Les antes et incréments chiffrés ci-dessus définissent la table amicale de cette application.

## Tarot français — quatre joueurs

Le paquet contient 78 cartes : quatre couleurs de 14 cartes, 21 atouts et l'Excuse. Les figures sont, par ordre croissant, valet, cavalier, dame, roi. La distribution se fait par paquets de trois, avec un chien de six cartes insérées une par une entre des paquets ; la première et la dernière carte ne vont pas au chien. Un petit sec sans Excuse annule la donne.

Chacun parle une fois : passe, prise, garde, garde sans, garde contre. Une enchère doit dépasser la précédente. Si tous passent, le donneur change et les cartes sont redistribuées. Sur prise/garde, le chien est révélé et intégré à la main du preneur, qui écarte six cartes. Les rois et bouts ne peuvent pas être écartés. Des atouts ordinaires ne sont écartés que si c'est indispensable et sont alors montrés. L'interface fait écarter d'abord les couleurs admissibles lorsque tous ces choix sont obligatoires ; cela ne réduit pas les ensembles d'écarts légaux. Sur garde sans, le chien fermé revient au preneur ; sur garde contre, à la défense.

Le joueur après le donneur entame, puis le gagnant de chaque pli entame le suivant. Il faut fournir la couleur ; à défaut, couper et surcouper si possible. La montée à l'atout s'impose aussi sur un partenaire. Sans couleur demandée ni atout, la défausse est libre. L'Excuse dispense de ces obligations ; si elle entame un pli, la carte suivante détermine la couleur demandée.

L'Excuse reste à son camp avant le dernier pli, avec compensation d'une demi-valeur de point au camp adverse lorsque nécessaire. La compensation est enregistrée comptablement, sans déplacer une autre carte physique. Au dernier pli elle revient au camp gagnant, sauf le cas du chelem avec Excuse maîtresse. Le preneur peut annoncer un chelem après l'écart ; il prend alors l'entame. Les chelems non annoncés et ceux de la défense sont également comptés.

Les poignées simple/double/triple sont facultatives : 10/13/15 atouts avec primes de 20/30/40. Les cartes de la présentation sont choisies et affichées automatiquement ; l'Excuse n'y remplace un atout que si nécessaire. Les atouts obligatoirement écartés sont pris en compte pour la présentation du preneur. Le petit au dernier pli vaut une prime signée de 10 multipliée par le contrat. Si un chelem se termine par l'Excuse maîtresse, le petit à l'avant-dernier pli est considéré au bout.

Les valeurs sont stockées en demi-points entiers : total 91 points, bouts et rois 4,5, dames 3,5, cavaliers 2,5, valets 1,5, autres cartes 0,5. Le seuil dépend des bouts pris : 56/51/41/36 pour 0/1/2/3 bouts. La marque ajoute 25 à l'écart au seuil et applique le coefficient 1/2/4/6. Le petit au bout est multiplié par ce coefficient ; les poignées bénéficient au camp gagnant. Chelem : +400 annoncé et réalisé, +200 non annoncé et réalisé, −200 annoncé et échoué ; un chelem de défense bénéficie à celle-ci. Le preneur reçoit trois fois la marque unitaire et chacun des défenseurs son opposé. Le total des scores reste nul. Les demi-points sont conservés sans arrondi.

Cette version propose le jeu amical à quatre : pas de tarot à trois ou cinq, pas d'annonce de chelem par un défenseur, pas de signalisation FFT des robots, pas de procédures d'arbitrage/tournoi. Les irrégularités ordinaires de jeu sont bloquées par le moteur plutôt que sanctionnées après coup.

Référence des règles : [Fédération française de tarot, règlement](https://fftarot.fr/assets/documents/Reglement%20FFT.pdf). L'application n'est pas certifiée par la FFT.
