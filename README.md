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
  - stable C API wrapper in `src/rby_c_api.h` / `src/rby_c_api.cpp`
  - turn resolution with forced faint switching and end-of-turn status effects
- Test coverage:
  - `test/test_body_slam.cpp` validates Body Slam damage and paralysis results
  - `test/test_damage.cpp` validates Seismic Toss level damage, accuracy thresholds, crit thresholds, and Body Slam secondary effect triggering

## What is working now

- `g++ -std=c++23 -O3 -I src src/pokedex.cpp test/test_body_slam.cpp -o test/test_body_slam`
- `./test/test_body_slam` passes
- Body Slam damage and secondary paralysis logic are functioning for Snorlax, Gengar, and Zapdos
- Core OU roster: Alakazam, Starmie, Exeggutor, Chansey, Snorlax, Tauros, Rhydon, Zapdos, Jynx, Gengar
- Example teams in `examples/team1.txt` and `examples/team2.txt` have full species and move coverage
- C API fully functional: reset, step, encode, legal_mask, clone for deterministic rollouts

## What we still want to add

### Simulator completeness

- Full move resolution
  - ✓ damage moves, recoil, level-damage, drain, status application, healing
  - ✓ Substitute, Reflect, Light Screen volatile setup
  - ✗ multi-turn moves (Hyper Beam lock, thrashing, bide)
  - ✗ priority, secondary effects on defense (confusion, paralysis accuracy drop)
- End-of-turn and turn-order logic
  - ✓ sleep countdown, burn damage, poison damage
  - ✓ flinch clear
  - ✗ confusion accuracy drop, paralysis speed reduction, multi-turn move tracking
  - ✗ Leech Seed, Rage, Bide damage accumulation
- Switching mechanics
  - ✓ force switches on faint with auto-next-alive
  - ✓ manual switch action legality
  - ✗ forced switch side-effects (Mist, etc.)
  - ✗ switch-in move interactions (Spikes, entry hazards)
- Party and battle flow
  - ✓ 6v6 party handling
  - ✓ move/switch action dispatch and turn resolution
  - ✓ win/loss detection and battle termination
  - ✗ move selection AI or action prioritization

### Gen 1 mechanics and correctness

- Implemented correctly:
  - type effectiveness and STAB
  - Gen 1 damage formula with crits and random factor
  - burn attack drop, Explosion defense halving
  - secondary status application (paralysis immunity for same-type, poison immunity for Poison)
  - stage modification (+/- up to 6)
  - paralysis speed reduction (75% speed penalty)
  - Hyper Beam recharge turn enforcement
  - Reflect/Light Screen damage reduction (50% mitigation)
- Known gaps vs Showdown:
  - confusion does not reduce accuracy
  - no priority system (all moves resolve by speed)
  - no multi-hit move behavior or damage overflow
  - Substitute does not reduce move damage (absorbs all or nothing)
  - no Mist or Aurora Veil interactions
  - Earthquake/surf/etc. do not handle Substitute-blocked state

## C API readiness for training

The stable C API (`src/rby_c_api.h`) is ready for Cython/Python integration:

- **Deterministic**: fixed RNG seeding, full state hashing
- **Fast cloning**: trivially copyable POD battle state
- **Complete action support**: move slots 0-3, switch slots 4-9 (RBY_ACTION_SWITCH_BASE)
- **Observation encoding**: fixed-size float vector (RBY_OBSERVATION_SIZE = 352)
- **Legal mask**: per-player valid action bits
- **Reward shaping**: step-wise HP delta + terminal win/loss bonus

You can start MCTS training immediately, but be aware of the Showdown compatibility gaps below.

## Showdown compatibility for Gen 1 OU

**Good match**: damage calculation, basic status effects, type interactions, switching, faint resolution.

**Missing for full compatibility**:
- Paralysis speed reduction
- Confusion accuracy drop
- Priority moves
- Multi-turn move behavior (Hyper Beam recharge, Thrashing/Bide)
- Reflect/Light Screen damage reduction
- Substitute interaction details
- Some edge cases and RBY-specific quirks

The simulator is **suitable for self-play training** and **deterministic rollouts**, but replays vs live Showdown may diverge. For research/datasets, run through the C API but validate critical battles independently.

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
cd /home/nixona/projects/rby-engine
g++ -std=c++23 -O3 -I src src/pokedex.cpp test/test_damage.cpp -o test/test_damage
./test/test_damage
```

To compile the C API wrapper and run a basic C API smoke test:

```bash
g++ -std=c++23 -O3 -I src src/pokedex.cpp src/rby_c_api.cpp test/test_c_api.cpp -o test/test_c_api
./test/test_c_api
```
