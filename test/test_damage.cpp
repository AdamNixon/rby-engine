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

static void expect_not_equal(uint64_t actual, uint64_t expected, const char* message) {
    if (actual == expected) {
        std::cerr << message << " failed: expected values to differ, got " << actual << "\n";
        std::exit(1);
    }
    std::cout << message << " passed\n";
}

int main() {
    Battle state{};
    state.rng = 0u;

    const Species& tauros = species_def(SPECIES_TAUROS);
    const Species& gengar = species_def(SPECIES_GENGAR);
    const Species& starmie = species_def(SPECIES_STARMIE);

    const Move& seismic = move_def(MOVE_SEISMIC_TOSS);
    const Move& thunderbolt = move_def(MOVE_THUNDERBOLT);

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

    Battler defender_gengar{};
    defender_gengar.attack = gengar.base_attack;
    defender_gengar.defense = gengar.base_defense;
    defender_gengar.speed = gengar.base_speed;
    defender_gengar.special = gengar.base_special;
    defender_gengar.attack_stage = 0;
    defender_gengar.defense_stage = 0;
    defender_gengar.speed_stage = 0;
    defender_gengar.special_stage = 0;
    defender_gengar.accuracy_stage = 0;
    defender_gengar.evasion_stage = 0;
    defender_gengar.status = STATUS_NONE;

    Battler defender_starmie{};
    defender_starmie.attack = starmie.base_attack;
    defender_starmie.defense = starmie.base_defense;
    defender_starmie.speed = starmie.base_speed;
    defender_starmie.special = starmie.base_special;
    defender_starmie.attack_stage = 0;
    defender_starmie.defense_stage = 0;
    defender_starmie.speed_stage = 0;
    defender_starmie.special_stage = 0;
    defender_starmie.accuracy_stage = 0;
    defender_starmie.evasion_stage = 0;
    defender_starmie.status = STATUS_NONE;

    int damage = calculate_damage(state, attacker, tauros, seismic, defender_gengar, gengar);
    expect_equal(damage, 100, "Seismic Toss fixed-level damage against Ghost at level 100");

    attacker.level = 50;
    int damage50 = calculate_damage(state, attacker, tauros, seismic, defender_gengar, gengar);
    expect_equal(damage50, 50, "Seismic Toss fixed-level damage against Ghost at level 50");

    attacker.level = 100;
    state.rng = 0u;
    int thunderbolt_damage = calculate_damage(state, attacker, tauros, thunderbolt, defender_starmie, starmie);
    std::cout << "Thunderbolt damage against Starmie with rng=0: " << thunderbolt_damage << "\n";

    Move fifty_acc = thunderbolt;
    fifty_acc.accuracy = 50;
    state.rng = 158u; // rng_u8 == 127
    bool fifty_hits = accuracy_check(state, attacker, fifty_acc, defender_starmie, tauros, starmie);
    expect_equal(fifty_hits ? 1 : 0, 1, "50% accuracy hits when rng < threshold");

    state.rng = 3u; // rng_u8 == 128
    bool fifty_misses = accuracy_check(state, attacker, fifty_acc, defender_starmie, tauros, starmie);
    expect_equal(fifty_misses ? 1 : 0, 0, "50% accuracy misses when rng == threshold");

    state.rng = 61u; // rng_u8 == 50
    bool crit_hits = rng_chance_256(state, static_cast<uint8_t>(attacker.speed / 2));
    expect_equal(crit_hits ? 1 : 0, 1, "Critical chance triggers when rng < threshold");

    state.rng = 138u; // rng_u8 == 251
    bool crit_misses = rng_chance_256(state, static_cast<uint8_t>(attacker.speed / 2));
    expect_equal(crit_misses ? 1 : 0, 0, "Critical chance misses when rng >= threshold");

    const Move& body_slam = move_def(MOVE_BODY_SLAM);
    const Species& zapdos_target = species_def(SPECIES_ZAPDOS);
    Battler defender_gengar2{};
    defender_gengar2.defense = zapdos_target.base_defense;
    defender_gengar2.status = STATUS_NONE;
    defender_gengar2.accuracy_stage = 0;
    defender_gengar2.evasion_stage = 0;

    state.rng = 244u; // rng_u8 == 29
    bool secondary_applies = maybe_apply_secondary_status(state, defender_gengar2, body_slam, zapdos_target);
    expect_equal(secondary_applies ? 1 : 0, 1, "Body Slam secondary effect triggers when rng < chance");

    defender_gengar2.status = STATUS_NONE;
    state.rng = 89u; // rng_u8 == 30
    bool secondary_skips = maybe_apply_secondary_status(state, defender_gengar2, body_slam, zapdos_target);
    expect_equal(secondary_skips ? 1 : 0, 0, "Body Slam secondary effect does not trigger when rng == chance");

    state.rng = 107u; // deterministic roll = 200
    Battler defender_dummy{};
    defender_dummy.evasion_stage = 0;
    bool thunderbolt_hits = accuracy_check(state, attacker, thunderbolt, defender_dummy, tauros, gengar);
    expect_equal(thunderbolt_hits ? 1 : 0, 1, "100% accuracy Thunderbolt hits at high RNG roll");

    Battle hash_a{};
    Battle hash_b = hash_a;
    hash_a.sides[0].battlers[0].pp[0] = 17;
    hash_b.sides[0].battlers[0].pp[0] = 18;
    expect_not_equal(hash_battle(hash_a), hash_battle(hash_b), "Hash differs for different PP values above 16");

    Battle hash_c{};
    Battle hash_d = hash_c;
    hash_c.sides[0].battlers[0].bide_damage = 255;
    hash_d.sides[0].battlers[0].bide_damage = 256;
    expect_not_equal(hash_battle(hash_c), hash_battle(hash_d), "Hash differs for bide_damage above 255");

    std::cout << "Seismic Toss level-based damage tests passed.\n";
    return 0;
}
