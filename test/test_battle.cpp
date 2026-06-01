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

static void expect_equal(uint16_t actual, uint16_t expected, const char* message) {
    if (actual != expected) {
        std::cerr << message << " failed: expected " << expected << ", got " << actual << "\n";
        std::exit(1);
    }
    std::cout << message << " passed: " << actual << "\n";
}

static void expect_equal(bool actual, bool expected, const char* message) {
    if (actual != expected) {
        std::cerr << message << " failed: expected " << expected << ", got " << actual << "\n";
        std::exit(1);
    }
    std::cout << message << " passed: " << actual << "\n";
}

int main() {
    Battle state{};
    state.rng = 0u;
    state.turn = 1;
    state.sides[0].active = 0;
    state.sides[1].active = 0;

    Battler attacker{};
    attacker.species_id = SPECIES_TAUROS;
    attacker.hp = 50;
    attacker.max_hp = 100;
    attacker.pp[0] = 10;
    attacker.moves[0] = MOVE_RECOVER;
    attacker.status = STATUS_NONE;
    state.sides[0].battlers[0] = attacker;

    Battler defender{};
    defender.species_id = SPECIES_ZAPDOS;
    defender.hp = 100;
    defender.max_hp = 100;
    defender.pp[0] = 10;
    defender.moves[0] = MOVE_THUNDERBOLT;
    defender.status = STATUS_NONE;
    state.sides[1].battlers[0] = defender;

    use_move(state, 0, 0, 0);
    expect_equal(state.sides[0].battlers[0].hp, static_cast<uint16_t>(100), "Recover heals the user to full HP");

    state.sides[0].battlers[0].hp = 30;
    state.sides[0].battlers[0].moves[0] = MOVE_THUNDER_WAVE;
    state.sides[0].battlers[0].pp[0] = 10;
    state.sides[1].battlers[0].status = STATUS_NONE;
    use_move(state, 0, 0, 0);
    expect_equal(state.sides[1].battlers[0].status == STATUS_PARALYSIS, true, "Thunder Wave applies paralysis to the opposing active Pokemon");

    state.sides[0].battlers[0].hp = 50;
    state.sides[0].battlers[0].status = STATUS_BURN;
    state.sides[0].battlers[0].moves[0] = MOVE_REST;
    state.sides[0].battlers[0].pp[0] = 10;
    use_move(state, 0, 0, 0);
    expect_equal(state.sides[0].battlers[0].hp, static_cast<uint16_t>(100), "Rest fully heals the user");
    expect_equal(state.sides[0].battlers[0].status == STATUS_SLEEP, true, "Rest puts the user to sleep");
    expect_equal(state.sides[0].battlers[0].sleep_turns >= 2 && state.sides[0].battlers[0].sleep_turns <= 3, true, "Rest gives 2-3 sleep turns");

    state.sides[0].active = 0;
    state.sides[0].battlers[1].hp = 100;
    state.sides[0].battlers[1].max_hp = 100;
    bool switched = switch_pokemon(state, 0, 1);
    expect_equal(switched, true, "Valid switch succeeds");
    expect_equal(state.sides[0].active, 1, "Side 0 active Pokemon changes when switching");

    state.sides[1].battlers[0].hp = 0;
    expect_equal(winning_side(state), 0, "Side 0 wins when side 1 has no usable Pokemon");
    expect_equal(is_battle_over(state), true, "Battle is over when one side is eliminated");

    std::cout << "Battle core tests passed.\n";
    return 0;
}
