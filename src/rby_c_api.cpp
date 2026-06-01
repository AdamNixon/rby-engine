#include "rby_c_api.h"
#include "pokedex.hpp"

#include <algorithm>
#include <cstring>

struct RbyBattle : Battle {};

static uint32_t clamp_active(uint8_t active) noexcept {
    return active < RBY_MAX_TEAM_SIZE ? active : 0u;
}

static void ensure_active(Battle& state, int side_idx) noexcept {
    Side& side = state.sides[side_idx];
    if (side.active < RBY_MAX_TEAM_SIZE && side.battlers[side.active].hp > 0) {
        return;
    }
    for (uint8_t idx = 0; idx < RBY_MAX_TEAM_SIZE; ++idx) {
        if (side.battlers[idx].hp > 0) {
            side.active = idx;
            return;
        }
    }
    side.active = 0;
}

static void copy_pokemon(const RbyPokemon& src, Battler& dst) noexcept {
    dst.species_id = static_cast<SpeciesId>(src.species_id);
    dst.level = src.level;
    dst.hp = src.hp;
    dst.max_hp = src.max_hp;
    dst.attack = src.attack;
    dst.defense = src.defense;
    dst.speed = src.speed;
    dst.special = src.special;
    dst.attack_stage = src.attack_stage;
    dst.defense_stage = src.defense_stage;
    dst.speed_stage = src.speed_stage;
    dst.special_stage = src.special_stage;
    dst.accuracy_stage = src.accuracy_stage;
    dst.evasion_stage = src.evasion_stage;
    dst.status = src.status;
    dst.sleep_turns = src.sleep_turns;
    dst.volatiles = src.volatiles;
    dst.confusion_turns = src.confusion_turns;
    dst.substitute_hp = src.substitute_hp;
    dst.bide_turns = src.bide_turns;
    dst.bide_damage = src.bide_damage;
    std::memcpy(dst.moves, src.moves, sizeof(dst.moves));
    std::memcpy(dst.pp, src.pp, sizeof(dst.pp));
}

static uint32_t total_hp(const Side& side) noexcept {
    uint32_t sum = 0;
    for (uint8_t idx = 0; idx < RBY_MAX_TEAM_SIZE; ++idx) {
        sum += side.battlers[idx].hp;
    }
    return sum;
}

static double make_step_reward(const Battle& before, const Battle& after) noexcept {
    const uint32_t before_p1 = total_hp(before.sides[0]);
    const uint32_t before_p2 = total_hp(before.sides[1]);
    const uint32_t after_p1 = total_hp(after.sides[0]);
    const uint32_t after_p2 = total_hp(after.sides[1]);

    double reward = 0.0;
    reward += static_cast<double>(before_p2) - static_cast<double>(after_p2);
    reward -= static_cast<double>(before_p1) - static_cast<double>(after_p1);
    reward *= 0.01;

    if (is_battle_over(after)) {
        const int winner = winning_side(after);
        if (winner == 0) {
            reward += 1.0;
        } else if (winner == 1) {
            reward -= 1.0;
        }
    }
    return reward;
}

static inline bool is_switch_action(uint8_t action) noexcept {
    return action >= RBY_ACTION_SWITCH_BASE && action < RBY_ACTION_COUNT;
}

static inline uint8_t switch_index_from_action(uint8_t action) noexcept {
    return static_cast<uint8_t>(action - RBY_ACTION_SWITCH_BASE);
}

static uint32_t mask_for_side(const Battle* b, int player) noexcept {
    if (!b || player < 0 || player > 1) {
        return 0u;
    }
    if (is_battle_over(*b)) {
        return 0u;
    }
    const Side& side = b->sides[player];
    uint32_t mask = 0u;
    const Battler& battler = side.battlers[side.active];

    const bool is_recharging = (battler.volatiles & VF_RECHARGE) != 0;
    if (!is_recharging) {
        for (uint32_t move_idx = 0; move_idx < RBY_MOVE_SLOTS; ++move_idx) {
            const bool has_pp = battler.hp > 0 && battler.pp[move_idx] > 0;
            if (has_pp) {
                mask |= (1u << move_idx);
            }
        }
    }

    for (uint32_t switch_idx = 0; switch_idx < RBY_MAX_TEAM_SIZE; ++switch_idx) {
        if (switch_idx == side.active) {
            continue;
        }
        const Battler& candidate = side.battlers[switch_idx];
        if (candidate.hp > 0) {
            mask |= (1u << (RBY_ACTION_SWITCH_BASE + switch_idx));
        }
    }
    return mask;
}

static void execute_action(Battle& state, int side_idx, uint8_t action) noexcept {
    if (side_idx < 0 || side_idx > 1) {
        return;
    }
    Side& side = state.sides[side_idx];
    if (side.active >= RBY_MAX_TEAM_SIZE) {
        return;
    }
    Battler& user = side.battlers[side.active];
    if (user.hp == 0) {
        return;
    }

    if (action < RBY_MOVE_SLOTS) {
        if (action < RBY_MOVE_SLOTS && user.pp[action] > 0) {
            use_move(state, side_idx, side.active, action);
        }
        return;
    }

    if (is_switch_action(action)) {
        const uint8_t target_idx = switch_index_from_action(action);
        if (target_idx < RBY_MAX_TEAM_SIZE) {
            switch_pokemon(state, side_idx, target_idx);
        }
    }
}

RbyBattle* rby_create_battle(void) {
    return new RbyBattle();
}

void rby_destroy_battle(RbyBattle* b) {
    delete b;
}

RbyBattle* rby_clone_battle(const RbyBattle* source) {
    if (!source) {
        return nullptr;
    }
    return new RbyBattle(*source);
}

void rby_reset(RbyBattle* b, uint64_t seed, const RbyTeam* p1, const RbyTeam* p2) {
    if (!b) {
        return;
    }
    std::memset(b, 0, sizeof(*b));
    b->rng = static_cast<uint32_t>(seed ^ (seed >> 32));
    b->turn = 0;

    if (p1) {
        for (uint8_t idx = 0; idx < RBY_MAX_TEAM_SIZE; ++idx) {
            copy_pokemon(p1->pokemon[idx], b->sides[0].battlers[idx]);
        }
        b->sides[0].active = clamp_active(p1->active);
        ensure_active(*b, 0);
    }

    if (p2) {
        for (uint8_t idx = 0; idx < RBY_MAX_TEAM_SIZE; ++idx) {
            copy_pokemon(p2->pokemon[idx], b->sides[1].battlers[idx]);
        }
        b->sides[1].active = clamp_active(p2->active);
        ensure_active(*b, 1);
    }
}

RbyStepResult rby_step(RbyBattle* b, uint8_t p1_action, uint8_t p2_action) {
    if (!b) {
        return RbyStepResult{0.0, 0.0, 0u, 0u, 0u, true};
    }
    Battle before = *b;
    Battle& state = *b;

    if (is_battle_over(state)) {
        const int winner = winning_side(state);
        const double reward = winner == 0 ? 1.0 : (winner == 1 ? -1.0 : 0.0);
        return RbyStepResult{reward, -reward, hash_battle(state), 0u, 0u, true};
    }

    ensure_active(state, 0);
    ensure_active(state, 1);

    const Battler& p1_battler = state.sides[0].battlers[state.sides[0].active];
    const Battler& p2_battler = state.sides[1].battlers[state.sides[1].active];
    const bool p1_first = p1_battler.speed >= p2_battler.speed;
    if (p1_first) {
        execute_action(state, 0, p1_action);
        execute_action(state, 1, p2_action);
    } else {
        execute_action(state, 1, p2_action);
        execute_action(state, 0, p1_action);
    }

    ensure_active(state, 0);
    ensure_active(state, 1);
    end_turn(state);
    ensure_active(state, 0);
    ensure_active(state, 1);

    const double reward_p1 = make_step_reward(before, state);
    const double reward_p2 = -reward_p1;
    const uint32_t legal_p1 = mask_for_side(&state, 0);
    const uint32_t legal_p2 = mask_for_side(&state, 1);
    const bool done = is_battle_over(state);
    const uint64_t hash = hash_battle(state);
    return RbyStepResult{reward_p1, reward_p2, hash, legal_p1, legal_p2, done};
}

void rby_encode(const RbyBattle* b, int player, float* out) {
    if (!b || !out) {
        return;
    }
    const Battle& state = *reinterpret_cast<const Battle*>(b);
    size_t index = 0;

    int side_order[2] = {0, 1};
    if (player == 1) {
        side_order[0] = 1;
        side_order[1] = 0;
    }

    for (int order = 0; order < 2; ++order) {
        const int side = side_order[order];
        out[index++] = static_cast<float>(state.sides[side].active);
        for (uint8_t idx = 0; idx < RBY_MAX_TEAM_SIZE; ++idx) {
            const Battler& battler = state.sides[side].battlers[idx];
            out[index++] = static_cast<float>(battler.hp);
            out[index++] = static_cast<float>(battler.max_hp);
            out[index++] = static_cast<float>(battler.level);
            out[index++] = static_cast<float>(battler.attack);
            out[index++] = static_cast<float>(battler.defense);
            out[index++] = static_cast<float>(battler.speed);
            out[index++] = static_cast<float>(battler.special);
            out[index++] = static_cast<float>(battler.species_id);
            out[index++] = static_cast<float>(battler.attack_stage);
            out[index++] = static_cast<float>(battler.defense_stage);
            out[index++] = static_cast<float>(battler.speed_stage);
            out[index++] = static_cast<float>(battler.special_stage);
            out[index++] = static_cast<float>(battler.accuracy_stage);
            out[index++] = static_cast<float>(battler.evasion_stage);
            out[index++] = static_cast<float>(battler.status);
            out[index++] = static_cast<float>(battler.sleep_turns);
            out[index++] = static_cast<float>(battler.volatiles);
            out[index++] = static_cast<float>(battler.confusion_turns);
            out[index++] = static_cast<float>(battler.substitute_hp);
            out[index++] = static_cast<float>(battler.bide_turns);
            out[index++] = static_cast<float>(battler.bide_damage);
            for (uint8_t move_idx = 0; move_idx < RBY_MOVE_SLOTS; ++move_idx) {
                out[index++] = static_cast<float>(battler.moves[move_idx]);
            }
            for (uint8_t pp_idx = 0; pp_idx < RBY_MOVE_SLOTS; ++pp_idx) {
                out[index++] = static_cast<float>(battler.pp[pp_idx]);
            }
        }
        // include side-level reflect / light screen flags
        out[index++] = static_cast<float>(state.sides[side].reflect);
        out[index++] = static_cast<float>(state.sides[side].light_screen);
    }
    out[index++] = static_cast<float>(state.rng);
    out[index++] = static_cast<float>(state.turn);

    if (index != RBY_OBSERVATION_SIZE) {
        // Observation size must remain stable.
    }
}

uint32_t rby_legal_mask(const RbyBattle* b, int player) {
    return mask_for_side(reinterpret_cast<const Battle*>(const_cast<RbyBattle*>(b)), player);
}

size_t rby_observation_size(void) {
    return RBY_OBSERVATION_SIZE;
}
