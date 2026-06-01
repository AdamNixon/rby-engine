#include "../src/rby_c_api.h"
#include "../src/pokedex.hpp"

#include <cassert>
#include <iostream>

int main() {
    RbyBattle* battle = rby_create_battle();
    assert(battle != nullptr);

    RbyTeam p1{};
    p1.active = 0;
    p1.pokemon[0].species_id = SPECIES_SNORLAX;
    p1.pokemon[0].level = 100;
    p1.pokemon[0].hp = 160;
    p1.pokemon[0].max_hp = 160;
    p1.pokemon[0].attack = 110;
    p1.pokemon[0].defense = 65;
    p1.pokemon[0].speed = 30;
    p1.pokemon[0].special = 65;
    p1.pokemon[0].moves[0] = MOVE_BODY_SLAM;
    p1.pokemon[0].pp[0] = 15;

    RbyTeam p2{};
    p2.active = 0;
    p2.pokemon[0].species_id = SPECIES_ZAPDOS;
    p2.pokemon[0].level = 100;
    p2.pokemon[0].hp = 90;
    p2.pokemon[0].max_hp = 90;
    p2.pokemon[0].attack = 90;
    p2.pokemon[0].defense = 85;
    p2.pokemon[0].speed = 100;
    p2.pokemon[0].special = 125;
    p2.pokemon[0].moves[0] = MOVE_THUNDERBOLT;
    p2.pokemon[0].pp[0] = 15;

    rby_reset(battle, 0xDEADBEEFull, &p1, &p2);

    uint32_t legal1 = rby_legal_mask(battle, 0);
    uint32_t legal2 = rby_legal_mask(battle, 1);
    assert((legal1 & 1u) != 0);
    assert((legal2 & 1u) != 0);

    assert(rby_observation_size() == RBY_OBSERVATION_SIZE);

    // Expect no switch options when only one Pokemon is alive per side.
    assert((legal1 & (1u << RBY_ACTION_SWITCH_BASE)) == 0);
    float obs[RBY_OBSERVATION_SIZE];
    rby_encode(battle, 0, obs);
    assert(obs[0] == 0.0f);

    // verify switch legality when a second Pokémon is available
    RbyTeam p1_two = p1;
    p1_two.pokemon[1] = p1_two.pokemon[0];
    p1_two.pokemon[1].hp = 80;
    p1_two.pokemon[1].max_hp = 80;
    p1_two.pokemon[1].moves[0] = MOVE_BODY_SLAM;
    p1_two.pokemon[1].pp[0] = 15;
    p1_two.active = 0;
    rby_reset(battle, 0xCAFECAFEULL, &p1_two, &p2);
    uint32_t legal1_two = rby_legal_mask(battle, 0);
    assert((legal1_two & (1u << (RBY_ACTION_SWITCH_BASE + 1))) != 0);

    RbyTeam p2_two = p2;
    p2_two.pokemon[1] = p2_two.pokemon[0];
    p2_two.pokemon[1].hp = 90;
    p2_two.pokemon[1].max_hp = 90;
    p2_two.pokemon[1].moves[0] = MOVE_THUNDERBOLT;
    p2_two.pokemon[1].pp[0] = 15;
    p2_two.pokemon[0].hp = 1;
    p2_two.active = 0;
    p1_two.pokemon[0].moves[0] = MOVE_SEISMIC_TOSS;
    p1_two.pokemon[0].pp[0] = 20;
    rby_reset(battle, 0x12345678ULL, &p1_two, &p2_two);
    RbyStepResult step = rby_step(battle, 0, 255);
    Battle* internal = reinterpret_cast<Battle*>(battle);
    assert(internal->sides[1].active == 1);
    std::cout << "step.done=" << step.done << " reward_p1=" << step.reward_p1 << " hash=" << step.hash << "\n";
    assert(step.hash != 0);

    RbyBattle* clone = rby_clone_battle(battle);
    assert(clone != nullptr);
    RbyStepResult clone_step = rby_step(clone, 0, 0);
    assert(clone_step.hash != 0);
    rby_destroy_battle(clone);

    rby_destroy_battle(battle);
    std::cout << "C API smoke test passed." << std::endl;
    return 0;
}
