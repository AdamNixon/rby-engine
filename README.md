# PokemonSim

A compact Generation 1 Pokemon battle simulator prototype written in C++23.

## Current status

The simulator currently has:

- Core immutable data separated from runtime state:
  - `src/move_defs.hpp` for move definitions
  - `src/species_defs.hpp` for species base stats and typings
- Mutable battle state in `src/pokedex.hpp` / `src/pokedex.cpp`
  - `Battler`, `Side`, and `Battle` structs
  - explicit RNG state stored in `Battle::rng`
  - fast clone-friendly POD design for rollouts and hashing
  - public RNG helpers for exact `0..255` rolls and 256-based chance checks
- Basic move mechanics implemented:
  - accuracy calculation
  - Gen 1 damage calculation with STAB, crits, random factor, burn attack drop, and Explosion defense halving
  - fixed damage moves like Seismic Toss using attacker level
  - Body Slam secondary paralysis checks with same-type immunity
  - status application and stage modification
- Basic simulator utilities:
  - RNG advancement via `rng_u8`, `rng_chance_256`, and `rng_range`
  - battle hashing for state identification
  - a simple `switch_pokemon` function for active-slot changes
- Test coverage:
  - `test/test_body_slam.cpp` validates Body Slam damage and paralysis results
  - `test/test_damage.cpp` validates Seismic Toss level damage, accuracy thresholds, crit thresholds, and Body Slam secondary effect triggering

## What is working now

- `g++ -std=c++23 -O3 -I src src/pokedex.cpp test/test_body_slam.cpp -o test/test_body_slam`
- `./test/test_body_slam` passes
- Body Slam damage and secondary paralysis logic are functioning for Snorlax, Gengar, and Zapdos
- New sample species added: Starmie, Jynx, Rhydon in addition to the existing roster

## What we still want to add

### Simulator completeness

- Full move resolution
  - resolve move effects beyond PP and RNG consumption
  - implement damage roll randomness, critical hits, and STAB
  - support non-damaging moves, status moves, and auxiliary effects
- End-of-turn and turn-order logic
  - sleep countdown, burn damage, poison damage
  - confusion, flinching, paralysis speed drops, and multi-turn move behavior
- Switching mechanics
  - force switches on faint
  - validate switch legality and party ordering
  - incorporate switching into action selection and turn resolution
- Party and battle flow
  - full 6v6 party handling
  - move selection for both sides with an action queue
  - win/loss detection and battle termination

### Gen 1 mechanics and correctness

- Expand move coverage for more accurate Gen 1 battles
- Add more core OU Pokemon and representative movesets
- Implement RBY-specific quirks
  - same-type status immunity
  - type immunities like Ghost vs Normal
  - poison immunity for Poison types
  - multi-hit moves and damage overflow rules
  - priority and speed-tie behavior

### MCTS readiness

- Add a complete deterministic simulator API for playouts
- Keep RNG in the battle state so clones can branch cleanly
- Add a `clone_battle()` or copy-based rollout interface
- Build a lightweight MCTS harness once move execution and switching are reliable
- Measure performance for:
  - battle copy throughput
  - move resolution speed
  - state hashing and transposition handling

## Goals

- Create a compact, high-performance RBY battle engine suitable for Monte Carlo Tree Search
- Preserve correctness for key Gen 1 mechanics while keeping the engine easy to clone and simulate
- Separate data from logic so the core simulator can scale to more Pokemon, moves, and battle rules
- Use battle state hashing and explicit RNG state to support search, caching, and reproducible rollouts
 - Support library-style search/research usage rather than game-engine gameplay
   - unit tests for RBY mechanics and Showdown-compatible cases
   - replay validation with exact RNG and state progression
   - dataset generation from intermediate battle states
   - MCTS/expectiminimax rollouts and self-play evaluation

## Next steps

1. Implement full move execution in `src/pokedex.cpp`
2. Add party initialization utilities and `clone_battle` support
3. Add more Pokemon and move definitions to `src/species_defs.hpp` and `src/move_defs.hpp`
4. Expand tests for status effects, type immunities, switching, and end-of-turn behavior
5. Prototype an MCTS search loop that uses the simulator for playouts

## Build / Test

From the repo root:

```bash
cd /home/nixona/projects/PokemonSim/poke-sim
g++ -std=c++23 -O3 -I src src/pokedex.cpp test/test_damage.cpp -o test/test_damage
./test/test_damage
```
