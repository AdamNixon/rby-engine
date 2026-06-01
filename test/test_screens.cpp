#include "pokedex.hpp"

#include <cstdlib>
#include <iostream>

static void expect_equal(int actual, int expected, const char* message) {
    if (actual != expected) {
        std::cerr << message << " failed: expected " << expected << ", got " << actual << "\n";
        std::exit(1);
    }
    std::cout << message << " passed: " << actual << "\n";
}

static void expect_true(bool actual, const char* message) {
    if (!actual) {
        std::cerr << message << " failed: expected true\n";
        std::exit(1);
    }
    std::cout << message << " passed\n";
}

int main() {
    // Setup species and moves
    const Species& tauros = species_def(SPECIES_TAUROS);
    const Species& gengar = species_def(SPECIES_GENGAR);

    const Move& earthquake = move_def(MOVE_EARTHQUAKE);
    const Move& psychic = move_def(MOVE_PSYCHIC);

    // Common attacker
    Battler attacker{};
    attacker.level = 100;
    attacker.attack = tauros.base_attack;
    attacker.defense = tauros.base_defense;
    attacker.speed = 200;
    attacker.special = tauros.base_special;
    attacker.attack_stage = 0;
    attacker.defense_stage = 0;
    attacker.speed_stage = 0;
    attacker.special_stage = 0;
    attacker.accuracy_stage = 0;
    attacker.evasion_stage = 0;
    attacker.status = STATUS_NONE;

    // Common defender
    Battler defender{};
    defender.hp = 100;
    defender.max_hp = 100;
    defender.level = 100;
    defender.attack = gengar.base_attack;
    defender.defense = gengar.base_defense;
    defender.speed = gengar.base_speed;
    defender.special = gengar.base_special;
    defender.attack_stage = 0;
    defender.defense_stage = 0;
    defender.speed_stage = 0;
    defender.special_stage = 0;
    defender.accuracy_stage = 0;
    defender.evasion_stage = 0;
    defender.status = STATUS_NONE;

    Battle state{};
    state.turn = 0;

    // Place battlers
    state.sides[0].battlers[0] = attacker;
    state.sides[0].active = 0;
    state.sides[1].battlers[0] = defender;
    state.sides[1].active = 0;

    // Test 1: Reflect reduces physical damage when side flag is set (force non-critical by lowering speed)
    state.sides[0].battlers[0].speed = 2;
    state.sides[1].reflect = false;
    state.rng = 0u;
    int dmg_no_reflect = calculate_damage(state, state.sides[0].battlers[0], tauros, earthquake, state.sides[1].battlers[0], gengar, 1);

    state.sides[1].reflect = true;
    state.rng = 0u;
    int dmg_with_reflect = calculate_damage(state, state.sides[0].battlers[0], tauros, earthquake, state.sides[1].battlers[0], gengar, 1);

    expect_true(dmg_with_reflect < dmg_no_reflect, "Side Reflect halves physical damage");

    // Test 2: Light Screen reduces special damage when side flag is set
    // Test 2: Light Screen reduces special damage when side flag is set (force non-critical)
    state.sides[0].battlers[0].speed = 2;
    state.sides[1].light_screen = false;
    state.rng = 0u;
    int dmg_no_light = calculate_damage(state, state.sides[0].battlers[0], tauros, psychic, state.sides[1].battlers[0], gengar, 1);

    state.sides[1].light_screen = true;
    state.rng = 0u;
    int dmg_with_light = calculate_damage(state, state.sides[0].battlers[0], tauros, psychic, state.sides[1].battlers[0], gengar, 1);

    expect_true(dmg_with_light < dmg_no_light, "Side Light Screen halves special damage");

    // Test 3: Screens are cleared on switch
    state.sides[1].reflect = true;
    // Ensure a second Pokémon exists to switch to
    state.sides[1].battlers[1] = defender;
    state.sides[1].battlers[1].hp = 50;
    state.sides[1].active = 0;
    bool switched = switch_pokemon(state, 1, 1);
    expect_true(switched, "Switch succeeded");
    expect_true(!state.sides[1].reflect, "Reflect cleared on switch");

    // Test 4: Reflect is ignored by critical hits
    state.sides[1].reflect = true;
    // Force RNG such that critical will occur (high speed -> larger threshold)
    state.sides[0].battlers[0].speed = 200;
    state.sides[1].active = 0;
    state.rng = 0u;
    int dmg_reflect_crit;
    dmg_reflect_crit = calculate_damage(state, state.sides[0].battlers[0], tauros, earthquake, state.sides[1].battlers[0], gengar, 1);

    state.sides[1].reflect = false;
    state.rng = 0u;
    int dmg_no_reflect_crit = calculate_damage(state, state.sides[0].battlers[0], tauros, earthquake, state.sides[1].battlers[0], gengar, 1);

    expect_equal(dmg_reflect_crit, dmg_no_reflect_crit, "Reflect ignored on critical hits");

    std::cout << "All screen behavior tests passed.\n";
    return 0;
}
