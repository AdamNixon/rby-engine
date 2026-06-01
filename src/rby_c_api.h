#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RBY_MAX_TEAM_SIZE 6
#define RBY_MOVE_SLOTS 4
#define RBY_ACTION_SWITCH_BASE RBY_MOVE_SLOTS
#define RBY_ACTION_COUNT (RBY_MOVE_SLOTS + RBY_MAX_TEAM_SIZE)
#define RBY_OBSERVATION_SIZE 356

typedef struct RbyPokemon {
    uint8_t species_id;
    uint8_t level;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t attack;
    uint16_t defense;
    uint16_t speed;
    uint16_t special;
    int8_t attack_stage;
    int8_t defense_stage;
    int8_t speed_stage;
    int8_t special_stage;
    int8_t accuracy_stage;
    int8_t evasion_stage;
    uint8_t status;
    uint8_t sleep_turns;
    uint32_t volatiles;
    uint8_t confusion_turns;
    uint8_t substitute_hp;
    uint8_t bide_turns;
    uint16_t bide_damage;
    uint8_t moves[RBY_MOVE_SLOTS];
    uint8_t pp[RBY_MOVE_SLOTS];
} RbyPokemon;

typedef struct RbyTeam {
    RbyPokemon pokemon[RBY_MAX_TEAM_SIZE];
    uint8_t active;
} RbyTeam;

typedef struct RbyBattle RbyBattle;

typedef struct RbyStepResult {
    double reward_p1;
    double reward_p2;
    uint64_t hash;
    uint32_t legal_p1;
    uint32_t legal_p2;
    bool done;
} RbyStepResult;

RbyBattle* rby_create_battle(void);
void rby_destroy_battle(RbyBattle* b);

void rby_reset(RbyBattle* b, uint64_t seed, const RbyTeam* p1, const RbyTeam* p2);
RbyBattle* rby_clone_battle(const RbyBattle* source);

RbyStepResult rby_step(RbyBattle* b, uint8_t p1_action, uint8_t p2_action);

void rby_encode(const RbyBattle* b, int player, float* out);
uint32_t rby_legal_mask(const RbyBattle* b, int player);
size_t rby_observation_size(void);

#ifdef __cplusplus
}
#endif
