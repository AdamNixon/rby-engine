#include "pokedex.hpp"

#include <iostream>

static void print_damage(const char* attacker_name, const char* target_name, const Battler& attacker, const Species& atk_species, const Battler& defender, const Species& def_species, const Move& move, Battle state) {
    int damage = calculate_damage(state, attacker, atk_species, move, defender, def_species);
    std::cout << attacker_name << " using " << target_name << " -> " << damage << " damage\n";
}

static void test_body_slam_paralysis(const Species& defender_species, const char* defender_name) {
    Battle state{};
    state.rng = 0x12345678u;

    Battler attacker{};
    const Species& tauros = species_def(SPECIES_TAUROS);
    attacker.attack = tauros.base_attack;
    attacker.attack_stage = 0;
    attacker.accuracy_stage = 0;

    const Move& body_slam = move_def(MOVE_BODY_SLAM);
    int successes = 0;
    const int trials = 1000;
    for (int i = 0; i < trials; ++i) {
        Battle trial_state = state;
        trial_state.rng += static_cast<uint32_t>(i);
        Battler defender{};
        defender.defense = defender_species.base_defense;
        defender.status = STATUS_NONE;
        if (maybe_apply_secondary_status(trial_state, defender, body_slam, defender_species)) {
            ++successes;
        }
    }

    std::cout << "Body Slam secondary paralysis on " << defender_name << ": " << successes << "/" << trials << " (~" << (successes * 100.0 / trials) << "%)\n";
}

int main() {
    Battle state{};
    state.rng = 0x1;

    const Species& tauros = species_def(SPECIES_TAUROS);
    const Move& body_slam = move_def(MOVE_BODY_SLAM);

    Battler attacker{};
    attacker.attack = tauros.base_attack;
    attacker.attack_stage = 0;
    attacker.accuracy_stage = 0;

    const auto evaluate = [&](SpeciesId target_id, const char* target_name) {
        const Species& target_species = species_def(target_id);
        Battler defender{};
        defender.defense = target_species.base_defense;
        defender.status = STATUS_NONE;
        print_damage("Tauros", target_name, attacker, tauros, defender, target_species, body_slam, state);
        test_body_slam_paralysis(target_species, target_name);
    };

    evaluate(SPECIES_SNORLAX, "Snorlax");
    evaluate(SPECIES_GENGAR, "Gengar");
    evaluate(SPECIES_ZAPDOS, "Zapdos");

    return 0;
}
