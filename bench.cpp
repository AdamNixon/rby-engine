#include "src/pokedex.hpp"

#include <chrono>
#include <iostream>
#include <random>

int main() {
    const size_t iterations = 1000000;
    Battle state{};
    state.turn = 1;
    state.rng = 0x12345678u;

    std::mt19937_64 engine(0x42);
    std::uniform_int_distribution<uint16_t> stat_dist(1, 255);
    std::uniform_int_distribution<uint8_t> stage_dist(0, 12);
    std::uniform_int_distribution<uint8_t> status_dist(0, STATUS_FREEZE);
    std::uniform_int_distribution<uint8_t> move_dist(0, 14);
    std::uniform_int_distribution<uint8_t> pp_dist(0, 15);
    std::uniform_int_distribution<uint32_t> volatile_dist(0, 0x7FFu);

    for (int side = 0; side < 2; ++side) {
        state.sides[side].active = 0;
        for (int idx = 0; idx < 6; ++idx) {
            Battler& battler = state.sides[side].battlers[idx];
            battler.hp = stat_dist(engine);
            battler.max_hp = 255;
            battler.attack = stat_dist(engine);
            battler.defense = stat_dist(engine);
            battler.speed = stat_dist(engine);
            battler.special = stat_dist(engine);
            battler.attack_stage = static_cast<int8_t>(stage_dist(engine) - 6);
            battler.defense_stage = static_cast<int8_t>(stage_dist(engine) - 6);
            battler.speed_stage = static_cast<int8_t>(stage_dist(engine) - 6);
            battler.special_stage = static_cast<int8_t>(stage_dist(engine) - 6);
            battler.accuracy_stage = static_cast<int8_t>(stage_dist(engine) - 6);
            battler.evasion_stage = static_cast<int8_t>(stage_dist(engine) - 6);
            battler.status = status_dist(engine);
            battler.sleep_turns = static_cast<uint8_t>(engine() % 8);
            battler.volatiles = volatile_dist(engine);
            battler.confusion_turns = static_cast<uint8_t>(engine() % 8);
            battler.substitute_hp = static_cast<uint8_t>(engine() % 256);
            battler.bide_turns = static_cast<uint8_t>(engine() % 8);
            battler.bide_damage = static_cast<uint16_t>(engine() % 256);
            for (int move = 0; move < 4; ++move) {
                battler.moves[move] = move_dist(engine);
                battler.pp[move] = pp_dist(engine);
            }
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    uint64_t hash = 0;
    for (size_t i = 0; i < iterations; ++i) {
        Battle copy = clone_battle(state);
        hash ^= hash_battle(copy);
    }
    auto end = std::chrono::high_resolution_clock::now();

    const auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    std::cout << "Battle size: " << sizeof(Battle) << " bytes\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Clone+hash elapsed: " << elapsed << " sec\n";
    std::cout << "Throughput: " << (iterations / elapsed) << " ops/sec\n";
    std::cout << "Checksum: " << hash << "\n";
    return 0;
}
