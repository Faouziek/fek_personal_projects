# Cardgame

A **C++17** terminal-based card game platform featuring **UNO**, **Seven Families**, **Five-Card Draw Poker**, and **four-player French Tarot**.

A human player can compete against bots. A fully automated mode allows game simulations and reproducible debugging using a random seed. No external libraries are required to play or run the C++ tests.

GitHub Actions is configured for Linux, Windows, and macOS, with additional memory sanitizer checks on Linux. The CI workflow is automatically triggered when changes are pushed to GitHub.

## Quick Start — Linux / macOS / WSL

**Requirements:** A C++17 compiler (GCC or Clang) and GNU Make. Python 3 is only required for console interface tests.

```sh
make -j2
./bin/jeuCartes
```

The menu allows users to select a game, the number of players, and the number of rounds. Game-specific rules are documented in [RULES.md](RULES.md).

Entering `q` or reaching the end of input gracefully terminates the game. Bots' cards remain hidden until the poker showdown, and the Tarot kitty is revealed only when permitted by the contract.

## Windows with Visual Studio / CMake

From a developer terminal with a C++17 compiler and CMake installed:

```powershell
cmake -S . -B out
cmake --build out --config Release
.\out\Release\jeuCartes.exe
ctest --test-dir out -C Release --output-on-failure
```

With a single-configuration CMake generator, the executable is located directly in `out`.

## Command-Line Usage

```sh
# You and three bots, five rounds
./bin/jeuCartes --game uno --players 4 --rounds 5

# Reproducible Seven Families game, bots only
./bin/jeuCartes --game familles --players 6 --simulate --seed 42

# Poker: 200 starting chips per player, up to 30 hands
./bin/jeuCartes --game poker --players 4 --simulate --rounds 30 --seed 42

# Tarot: 100 hands without logs, with machine-readable JSON results
./bin/jeuCartes --game tarot --simulate --rounds 100 --seed 42 --json

./bin/jeuCartes --help
```

| Option | Value / Description |
| --- | --- |
| `--game` | `uno`, `familles`, `poker`, `tarot` |
| `--players` | UNO: 2–10; Seven Families and Poker: 2–6; Tarot: 4 |
| `--rounds` | 1–10,000 rounds/hands; default: 1 |
| `--humans` | 0 or 1; default: 1 |
| `--simulate` | Equivalent to `--humans 0` |
| `--seed` | Integer from 0 to 4,294,967,295; the seed is displayed for reproducibility |
| `--quiet` | Suppresses game logs while keeping the final summary; simulation mode only |
| `--json` | Outputs a single JSON object; simulation mode only |

When no arguments are provided, the interactive menu opens. If command-line options are specified, `--game` is required.

In Poker, a session ends before the specified limit if only one player has chips remaining. In UNO, the number of rounds is determined by the user; the game does not automatically stop when a player reaches 500 points.

The `mt19937` random number generator and Fisher–Yates shuffle use explicit bounded random sampling to ensure results are independent of the STL implementation of `std::shuffle`. Simulations are reproducible using the same options and random seed.

## Testing and Validation

```sh
make test       # C++ tests and console interface tests using Python 3
make smoke      # Console interface tests only
make sanitize   # Linux: AddressSanitizer + UndefinedBehaviorSanitizer
make clean
```

Alternatively, using CMake:

```sh
cmake -S . -B out -DCMAKE_BUILD_TYPE=Release
cmake --build out
ctest --test-dir out --output-on-failure
```

The tests cover game rules, tie-breaking, side pots, scoring, user input handling, and complete games.

The simulation test suite runs across 100 random seeds and all supported player counts, totaling **2,000 game sessions**.

Card and chip conservation invariants are enforced by the game engines.

C++ validation checks throw exceptions on failure, ensuring they remain active even in Release builds.