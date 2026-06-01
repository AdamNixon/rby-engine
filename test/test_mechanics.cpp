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
    Battle state{};
    state.rng = 0u;

    const Species& tauros = species_def(SPECIES_TAUROS);
    const Species& gengar = species_def(SPECIES_GENGAR);
    const Species& snorlax = species_def(SPECIES_SNORLAX);

    // Test 1: Paralysis reduces speed to 25%
    Battler paralyzed_tauros{};
    paralyzed_tauros.speed = 100;
    paralyzed_tauros.speed_stage = 0;
    paralyzed_tauros.status = STATUS_PARALYSIS;
    int speed_with_paralysis = modified_speed(paralyzed_tauros, tauros);
    expect_equal(speed_with_paralysis, 25, "Paralysis reduces speed to 25%");

    Battler normal_tauros{};
    normal_tauros.speed = 100;
    normal_tauros.speed_stage = 0;
    normal_tauros.status = STATUS_NONE;
    int speed_without_paralysis = modified_speed(normal_tauros, tauros);
    expect_equal(speed_without_paralysis, 100, "Normal Tauros has full speed");

    // Test 2: Reflect reduces physical damage by 50%
    Battler attacker{};
    attacker.level = 100;
    attacker.attack = tauros.base_attack;
    attacker.defense = tauros.base_defense;
    attacker.speed = tauros.base_speed;
    attacker.special = tauros.base_special;
    attacker.attack_stage = 0;
    attacker.defense_stage = 0;
    attacker.speed_stage = 0;
    attacker.special_stage = 0;
    attacker.accuracy_stage = 0;
    attacker.evasion_stage = 0;
    attacker.status = STATUS_NONE;

    Battler defender_no_reflect{};
    defender_no_reflect.attack = gengar.base_attack;
    defender_no_reflect.defense = gengar.base_defense;
    defender_no_reflect.speed = gengar.base_speed;
    defender_no_reflect.special = gengar.base_special;
    defender_no_reflect.attack_stage = 0;
    defender_no_reflect.defense_stage = 0;
    defender_no_reflect.speed_stage = 0;
    defender_no_reflect.special_stage = 0;
    defender_no_reflect.accuracy_stage = 0;
    defender_no_reflect.evasion_stage = 0;
    defender_no_reflect.status = STATUS_NONE;
    defender_no_reflect.volatiles = 0;

    Battler defender_reflect{};
    defender_reflect = defender_no_reflect;
    defender_reflect.volatiles = VF_REFLECT;

    const Move& earthquake = move_def(MOVE_EARTHQUAKE);
    state.rng = 0u;
    int damage_no_reflect = calculate_damage(state, attacker, tauros, earthquake, defender_no_reflect, gengar);

    state.rng = 0u;
    int damage_with_reflect = calculate_damage(state, attacker, tauros, earthquake, defender_reflect, gengar);

    expect_true(damage_no_reflect > damage_with_reflect, "Reflect reduces physical damage");

    // Test 3: Hyper Beam sets recharge flag
    Battler hyper_beam_user{};
    hyper_beam_user.hp = 100;
    hyper_beam_user.max_hp = 100;
    hyper_beam_user.level = 100;
    hyper_beam_user.attack = snorlax.base_attack;
    hyper_beam_user.defense = snorlax.base_defense;
    hyper_beam_user.speed = snorlax.base_speed;
    hyper_beam_user.special = snorlax.base_special;
    hyper_beam_user.species_id = SPECIES_SNORLAX;
    hyper_beam_user.attack_stage = 0;
    hyper_beam_user.defense_stage = 0;
    hyper_beam_user.speed_stage = 0;
    hyper_beam_user.special_stage = 0;
    hyper_beam_user.accuracy_stage = 0;
    hyper_beam_user.evasion_stage = 0;
    hyper_beam_user.status = STATUS_NONE;
    hyper_beam_user.volatiles = 0;
    hyper_beam_user.moves[0] = MOVE_HYPER_BEAM;
    hyper_beam_user.pp[0] = 5;

    Battler target{};
    target.hp = 50;
    target.max_hp = 50;
    target.level = 100;
    target.attack = gengar.base_attack;
    target.defense = gengar.base_defense;
    target.speed = gengar.base_speed;
    target.special = gengar.base_special;
    target.species_id = SPECIES_GENGAR;
    target.attack_stage = 0;
    target.defense_stage = 0;
    target.speed_stage = 0;
    target.special_stage = 0;
    target.accuracy_stage = 0;
    target.evasion_stage = 0;
    target.status = STATUS_NONE;
    target.volatiles = 0;

    Battle hyper_state{};
    hyper_state.rng = 0xDEADBEEFu;
    hyper_state.turn = 0;
    hyper_state.sides[0].battlers[0] = hyper_beam_user;
    hyper_state.sides[0].active = 0;
    hyper_state.sides[1].battlers[0] = target;
    hyper_state.sides[1].active = 0;

    use_move(hyper_state, 0, 0, 0);
    expect_true((hyper_state.sides[0].battlers[0].volatiles & VF_RECHARGE) != 0, "Hyper Beam sets recharge flag");

    std::cout << "All new mechanic tests passed.\n";
    return 0;
}
