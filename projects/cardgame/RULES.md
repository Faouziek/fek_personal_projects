# Rules and Selected Variants

All four games are playable from start to final scoring in a terminal, with one human player and bots, or exclusively bots. The conventions implemented in this version are explicitly described below. Bots do not have access to opponents' hands. In Tarot, seat numbers follow counterclockwise order and increase in the direction of play.

## UNO — Classic 108-Card Deck

- Seven cards are dealt to each player. Cards can be played by matching the active color, number, or symbol, including wild cards.
- Skip, Reverse, Draw Two (+2), Wild, and Wild Draw Four (+4) effects are implemented. In a two-player game, Reverse allows the player who played it to take another turn.
- Draw penalties require the next player to draw cards and skip their turn. Draw Two and Wild Draw Four penalties cannot be stacked.
- A Wild Draw Four cannot be played if the player's hand contains a card matching the active color. This restriction is enforced by the game engine, so bluffing and challenging a Wild Draw Four are not supported.
- Players may voluntarily draw a card. After drawing, only the newly drawn card may be played, provided it is legal.
- The game automatically announces "UNO" when a player has one card remaining. No penalty applies for failing to announce UNO.
- An action card revealed at the start of a round immediately applies its effect. If a Wild Draw Four is revealed, it is returned to the deck and another starting card is drawn. An initial Reverse makes the dealer play first in the opposite direction.
- When necessary, the discard pile is reshuffled into the draw pile, keeping the top discard card in place.
- A round ends when a player has no cards remaining. If the final card is a Draw Two or Wild Draw Four, its penalty is applied before scoring.
- The winner scores points based on the cards remaining in opponents' hands: number cards count their face value, action cards count 20 points, and wild cards count 50 points. Scores accumulate across the requested number of rounds. This version does not enforce the traditional 500-point target.
- If all cards are held by players and nobody can play or draw during an entire rotation of turns, the round is declared blocked and no points are awarded. This convention prevents an infinite loop.

## Seven Families — 42 Cards

The deck contains seven numbered families, each consisting of six members: grandfather, grandmother, father, mother, son, and daughter. The custom family names used in the initial prototype have been replaced with numbers.

Six cards are dealt to each player, and the remaining cards form the draw pile.

A player asks a specific opponent who still holds cards for a particular card. The requesting player must already possess at least one card from that family and must not already hold the requested card.

If the opponent has the requested card, they must hand it over, and the requesting player takes another turn. Otherwise, the requesting player draws a card. They may play again only if the drawn card is exactly the one requested (a "lucky draw"). After an unsuccessful request and draw, the turn passes to the opponent who was asked.

Players with empty hands draw one card on their turn, if possible; otherwise, they are skipped.

A completed family is automatically placed on the table and earns one point. The game ends when all seven families have been completed. Ties are preserved. Across multiple rounds, the number of completed families is accumulated.

Bots remember announced card transfers and unsuccessful requests. After an opponent draws a hidden card, bots no longer assume that the opponent lacks a card they were previously known not to hold.

## Five-Card Draw Poker — Virtual Chips

This is five-card draw poker, not Texas Hold'em.

Each player starts a session with 200 virtual chips. The dealer rotates after each hand among players who still have chips. Each participant contributes an ante of 10 chips, limited by their remaining stack, and receives five cards.

Each hand follows four stages:

1. First betting round, with a fixed raise increment of 10 chips.
2. Each player still in the hand may exchange between zero and five cards.
3. Second betting round, with a fixed raise increment of 20 chips.
4. Showdown and pot distribution. If every other player folds, the remaining player wins without revealing their cards.

Each betting round begins with the player seated after the dealer. Players may check, call, fold, or raise.

The betting system allows an opening bet followed by at most three full raises. An all-in raise smaller than the required increment does not reopen raising rights for players who have already acted; they may only call the additional amount or fold. A player who is the only participant with remaining chips against all-in opponents cannot create an additional bet that no opponent can match.

All-in situations are supported. Each player can win only the portion of the pots covered by their contribution. Uncalled contributions are returned.

Tied hands split the pot equally. Any indivisible remaining chips are awarded according to seat order, starting after the dealer.

Cards discarded by a player cannot return to that player's hand during the same exchange. If necessary, cards discarded by previous players are reshuffled into the available deck.

**Poker hand rankings**, from lowest to highest:

1. High card
2. One pair
3. Two pair
4. Three of a kind
5. Straight
6. Flush
7. Full house
8. Four of a kind
9. Straight flush

The five relevant card ranks are used to break ties. Suits never break ties.

An Ace may be used in either an A-2-3-4-5 straight or a 10-J-Q-K-A straight, but no other wraparound sequence is permitted.

The session ends after the requested number of hands or when only one player has chips remaining. Final rankings are based on remaining chip stacks.

No real money, purchases, or external betting are involved.

## French Tarot — Four Players

The game uses a 78-card deck consisting of four suits of 14 cards, 21 trumps, and the Excuse (Fool).

Court cards, in ascending order, are Jack, Knight, Queen, and King.

Cards are dealt in packets of three, with six cards placed individually into the *chien* (kitty) between packets. The first and last cards dealt cannot be placed in the kitty.

A hand containing the Petit (trump 1) as the only trump, without the Excuse, is invalid and must be redealt.

### Bidding and the Kitty

Each player bids once, choosing among:

- Pass
- Prise (Take)
- Garde (Guard)
- Garde Sans (Guard Without the Kitty)
- Garde Contre (Guard Against the Kitty)

Each new bid must outrank the previous one. If all players pass, the dealer rotates and the cards are redealt.

For a Prise or Garde contract, the kitty is revealed and added to the taker's hand. The taker then discards six cards.

Kings and *bouts* (the three key trumps: trump 1, trump 21, and the Excuse) cannot be discarded. Ordinary trumps may be discarded only when unavoidable, in which case they must be shown.

When all eligible suit cards must be discarded, the interface requires them to be selected before other cards. This does not restrict the set of legally permitted discards.

For Garde Sans, the unrevealed kitty belongs to the taker. For Garde Contre, it belongs to the defenders.

### Trick-Taking Rules

The player seated after the dealer leads the first trick. The winner of each trick leads the next.

Players must follow the led suit whenever possible. If they cannot follow suit, they must play a trump and overtrump whenever possible.

The obligation to play a higher trump also applies when a partner has already played a trump.

If a player cannot follow suit and has no trumps, they may discard any card.

The Excuse is exempt from these obligations. If it leads a trick, the next card played determines the required suit.

### The Excuse and Slam

Before the final trick, the Excuse remains with the side that played it, with a half-point card-value compensation to the opposing side when necessary.

This compensation is recorded in the scoring system without physically transferring another card.

On the final trick, the Excuse belongs to the winning side, except in the special case of a slam completed with the Excuse as the final winning card.

The taker may announce a slam after discarding. In this case, the taker leads the first trick.

Unannounced slams and slams achieved by the defenders are also taken into account when scoring.

### Handfuls and Bonuses

Single, double, and triple handfuls (*poignées*) are optional:

| Handful | Required trumps | Bonus |
|---|---:|---:|
| Single | 10 | 20 points |
| Double | 13 | 30 points |
| Triple | 15 | 40 points |

The cards presented in a handful are automatically selected and displayed. The Excuse replaces a trump only when necessary.

Trumps that the taker was required to discard are taken into account when determining the taker's handful presentation.

Winning the final trick with the Petit (*petit au bout*) awards a signed bonus of 10 points multiplied by the contract coefficient.

If a slam ends with the Excuse as the final winning card, the Petit played on the penultimate trick is treated as having been played on the final trick.

### Card Values and Scoring

Card values are stored internally as integer half-points, avoiding floating-point rounding errors.

The deck contains a total of **91 points**.

| Card | Point value |
|---|---:|
| Bouts and Kings | 4.5 |
| Queens | 3.5 |
| Knights | 2.5 |
| Jacks | 1.5 |
| All other cards | 0.5 |

The number of bouts captured by the taker determines the required point threshold:

| Bouts | Required points |
|---|---:|
| 0 | 56 |
| 1 | 51 |
| 2 | 41 |
| 3 | 36 |

The base score adds 25 points to the difference between the taker's score and the required threshold, then applies the contract coefficient:

| Contract | Coefficient |
|---|---:|
| Prise | ×1 |
| Garde | ×2 |
| Garde Sans | ×4 |
| Garde Contre | ×6 |

The Petit au Bout bonus is multiplied by the same coefficient.

Handful bonuses are awarded to the winning side.

Slam bonuses and penalties are:

- **+400 points** for an announced and successful slam.
- **+200 points** for a successful unannounced slam.
- **−200 points** for an announced but unsuccessful slam.

A slam achieved by the defending side benefits the defenders.

The taker receives three times the unit score, while each defender receives its opposite.

The sum of all players' scores always remains zero. Half-points are preserved without rounding.