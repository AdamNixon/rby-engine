#include "pokedex.hpp"
#include <cassert>
#include <algorithm>

namespace {
constexpr uint64_t splitmix64(uint64_t x) noexcept {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

template <size_t N>
constexpr std::array<uint64_t, N> make_zobrist_table(uint64_t seed) noexcept {
    std::array<uint64_t, N> result{};
    for (size_t i = 0; i < N; ++i) {
        result[i] = splitmix64(seed + i);
    }
    return result;
}

constexpr std::array<uint64_t, 12> ZOBRIST_SIDE_ACTIVE = make_zobrist_table<12>(0x9d39247e33776d41ULL);
constexpr std::array<uint64_t, 7> ZOBRIST_STATUS = make_zobrist_table<7>(0x4f1bbcdcbfa54012ULL);
constexpr std::array<uint64_t, 8> ZOBRIST_SLEEP = make_zobrist_table<8>(0x2c8d77d34b6ad6a1ULL);
constexpr std::array<uint64_t, 13> ZOBRIST_STAGE = make_zobrist_table<13>(0x7c9d2ad0abebf3f1ULL);
constexpr std::array<uint64_t, 32> ZOBRIST_VOLATILE = make_zobrist_table<32>(0x1f1e7c5a423e4f9cULL);
constexpr std::array<uint64_t, 8> ZOBRIST_CONFUSION = make_zobrist_table<8>(0xc2e5d7b14f9a3207ULL);
constexpr std::array<uint64_t, 256> ZOBRIST_SUBSTITUTE = make_zobrist_table<256>(0x89abcdef01234567ULL);
constexpr std::array<uint64_t, 8> ZOBRIST_BIDE_TURNS = make_zobrist_table<8>(0x123456789abcdef0ULL);
inline uint32_t byte_accuracy(uint32_t accuracy) noexcept {
    return accuracy <= 100 ? (accuracy * 255u + 50u) / 100u : accuracy;
}

constexpr std::array<std::array<uint64_t, 256>, 4> ZOBRIST_MOVE_ID = {
    make_zobrist_table<256>(0xabcdef0123456789ULL),
    make_zobrist_table<256>(0x23456789abcdef01ULL),
    make_zobrist_table<256>(0x3456789abcdef012ULL),
    make_zobrist_table<256>(0x456789abcdef0123ULL)
};
constexpr std::array<std::array<uint64_t, 256>, 4> ZOBRIST_PP = {
    make_zobrist_table<256>(0x56789abcdef01234ULL),
    make_zobrist_table<256>(0x6789abcdef012345ULL),
    make_zobrist_table<256>(0x789abcdef0123456ULL),
    make_zobrist_table<256>(0x89abcdef01234567ULL)
};

inline uint64_t mix_u64(uint64_t hash, uint64_t value) noexcept {
    hash ^= value + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
    return hash;
}

inline int apply_stage(int base, int8_t stage) noexcept {
    if (stage >= 0) {
        return (base * (2 + stage) + 1) / 2;
    }
    return (base * 2) / (2 - stage);
}

inline int accuracy_multiplier(int8_t stage) noexcept {
    if (stage >= 0) {
        return (100 * (3 + stage) + 1) / 3;
    }
    return (300 + (3 - stage) - 1) / (3 - stage);
}

} // namespace

int modified_attack(const Battler& b, const Species&) {
    return apply_stage(b.attack, b.attack_stage);
}

int modified_defense(const Battler& b, const Species&) {
    return apply_stage(b.defense, b.defense_stage);
}

int modified_speed(const Battler& b, const Species&) {
    int speed = apply_stage(b.speed, b.speed_stage);
    if (b.status == STATUS_PARALYSIS) {
        speed = (speed * 25 + 50) / 100;
    }
    return speed;
}

int modified_special(const Battler& b, const Species&) {
    return apply_stage(b.special, b.special_stage);
}

inline bool is_physical_type(Type type) noexcept {
    return type == NORMAL || type == FIGHTING || type == GROUND || type == ROCK || type == BUG || type == GHOST;
}

inline uint32_t gen1_crit_threshold(uint32_t speed) noexcept {
    return std::min<uint32_t>(255u, speed / 2u);
}

inline uint32_t gen1_random_factor(Battle& state) noexcept {
    return 217u + (rng_u8(state) % 39u);
}

inline const Species& species_of(const Battler& battler) noexcept {
    return species_def(static_cast<SpeciesId>(battler.species_id));
}

inline const Move& move_of(const Battler& battler, uint8_t slot) noexcept {
    return move_def(static_cast<MoveId>(battler.moves[slot]));
}

inline bool is_fainted(const Battler& battler) noexcept {
    return battler.hp == 0;
}

inline bool has_usable_pokemon(const Side& side) noexcept {
    for (int idx = 0; idx < 6; ++idx) {
        if (side.battlers[idx].hp > 0) {
            return true;
        }
    }
    return false;
}

inline const Battler& active_battler(const Battle& state, int side_idx) noexcept {
    return state.sides[side_idx].battlers[state.sides[side_idx].active];
}

inline Battler& active_battler(Battle& state, int side_idx) noexcept {
    return state.sides[side_idx].battlers[state.sides[side_idx].active];
}

bool accuracy_check(Battle& state, const Battler& attacker, const Move& move, const Battler& defender, const Species&, const Species&) {
    if (move.accuracy == 0) {
        return true;
    }

    const int attacker_multiplier = accuracy_multiplier(attacker.accuracy_stage);
    const int defender_multiplier = accuracy_multiplier(defender.evasion_stage);
    const int accuracy = (static_cast<int>(byte_accuracy(move.accuracy)) * attacker_multiplier + defender_multiplier / 2) / defender_multiplier;
    if (accuracy <= 0) {
        return false;
    }
    if (accuracy >= 256) {
        return true;
    }
    return rng_chance_256(state, static_cast<uint8_t>(accuracy));
}

int calculate_damage(Battle& state, const Battler& attacker, const Species& atk_species, const Move& move, const Battler& defender, const Species& def_species, int defender_side) {
    const int level = attacker.level > 0 ? attacker.level : 100;
    if (move.effect == EFFECT_LEVEL_DAMAGE) {
        return level;
    }
    if (move.power == 0) {
        return 0;
    }

    const bool physical = is_physical_type(Type(move.type));
    const bool critical = rng_chance_256(state, static_cast<uint8_t>(gen1_crit_threshold(attacker.speed)));

    int eff_level = level;
    int atk = physical ? modified_attack(attacker, atk_species) : modified_special(attacker, atk_species);
    int def = physical ? modified_defense(defender, def_species) : modified_special(defender, def_species);

    if (critical) {
        eff_level = level * 2;
        atk = physical ? attacker.attack : attacker.special;
        def = physical ? defender.defense : defender.special;
    }
    if (atk == 0) {
        atk = 1;
    }
    if (def == 0) {
        def = 1;
    }

    if (!critical && physical && attacker.status == STATUS_BURN) {
        atk = std::max(1, atk / 2);
    }

    if (move.effect == EFFECT_EXPLODE) {
        def = std::max(1, def / 2);
    }

    int base_damage = (((eff_level * 2 / 5 + 2) * move.power * atk) / def) / 50 + 2;
    const int type_mult = type_effectiveness(move, Type(def_species.type1), Type(def_species.type2));
    if (type_mult == 0) {
        return 0;
    }

    if (move.type == atk_species.type1 || move.type == atk_species.type2) {
        base_damage = (base_damage * 3) / 2;
    }

    int damage = (base_damage * type_mult + 50) / 100;

    // Apply Reflect / Light Screen if present on defender's side (Gen 1: ignored by critical hits)
    bool def_has_reflect = false;
    bool def_has_light = false;
    if (defender_side >= 0 && defender_side < 2) {
        def_has_reflect = state.sides[defender_side].reflect;
        def_has_light = state.sides[defender_side].light_screen;
    } else {
        def_has_reflect = (defender.volatiles & VF_REFLECT) != 0;
        def_has_light = (defender.volatiles & VF_LIGHT_SCREEN) != 0;
    }
    if (!critical) {
        if (physical && def_has_reflect) {
            damage = (damage + 1) / 2;
        }
        if (!physical && def_has_light) {
            damage = (damage + 1) / 2;
        }
    }

    const uint32_t random_factor = gen1_random_factor(state);
    damage = (damage * static_cast<int>(random_factor)) / 255;
    return damage > 0 ? damage : 1;
}

bool maybe_apply_secondary_status(Battle& state, Battler& target, const Move& move, const Species& target_species) {
    if (move.effect != EFFECT_DAMAGE || move.arg1 == STATUS_NONE || move.chance == 0) {
        return false;
    }
    if (target.status != STATUS_NONE) {
        return false;
    }
    if (type_effectiveness(move, Type(target_species.type1), Type(target_species.type2)) == 0) {
        return false;
    }
    if ((target_species.type1 == move.type || target_species.type2 == move.type) &&
        (move.arg1 == STATUS_SLEEP || move.arg1 == STATUS_PARALYSIS || move.arg1 == STATUS_BURN || move.arg1 == STATUS_POISON || move.arg1 == STATUS_FREEZE)) {
        return false;
    }
    if (move.arg1 == STATUS_POISON && (target_species.type1 == POISON || target_species.type2 == POISON)) {
        return false;
    }
    if (rng_chance_256(state, move.chance)) {
        apply_status(state, target, move.arg1);
        return true;
    }
    return false;
}

void apply_status(Battle& state, Battler& target, uint8_t status) {
    if (target.status == STATUS_NONE) {
        target.status = status;
        if (status == STATUS_SLEEP) {
            target.sleep_turns = rng_range(state, 1u, 7u);
        }
    }
}

void modify_stage(Battler& target, Stat stat, int8_t delta) {
    int8_t* stages[] = {
        &target.attack_stage,
        &target.defense_stage,
        &target.speed_stage,
        &target.special_stage,
        &target.accuracy_stage,
        &target.evasion_stage
    };
    int8_t& current = *stages[stat];
    current += delta;
    if (current > 6) {
        current = 6;
    } else if (current < -6) {
        current = -6;
    }
}

bool switch_pokemon(Battle& state, int side_idx, uint8_t new_active) {
    if (side_idx < 0 || side_idx > 1) {
        return false;
    }
    Side& side = state.sides[side_idx];
    if (new_active >= 6 || new_active == side.active) {
        return false;
    }
    if (side.battlers[new_active].hp == 0) {
        return false;
    }
    side.active = new_active;
    // In Gen 1, Reflect/Light Screen are lost when the active Pokémon switches
    side.reflect = false;
    side.light_screen = false;
    return true;
}

bool is_battle_over(const Battle& state) noexcept {
    return !has_usable_pokemon(state.sides[0]) || !has_usable_pokemon(state.sides[1]);
}

int winning_side(const Battle& state) noexcept {
    const bool side0_alive = has_usable_pokemon(state.sides[0]);
    const bool side1_alive = has_usable_pokemon(state.sides[1]);
    if (side0_alive == side1_alive) {
        return -1;
    }
    return side0_alive ? 0 : 1;
}

void use_move(Battle& state, int side_idx, int battler_idx, uint8_t move_slot) {
    if (side_idx < 0 || side_idx > 1) {
        return;
    }
    Battler& user = state.sides[side_idx].battlers[battler_idx];
    if (user.hp == 0 || move_slot >= 4 || user.pp[move_slot] == 0) {
        return;
    }

    if (user.volatiles & VF_RECHARGE) {
        user.volatiles &= ~VF_RECHARGE;
        return;
    }

    const Move& move = move_of(user, move_slot);
    Battler& target = state.sides[1 - side_idx].battlers[state.sides[1 - side_idx].active];
    const Species& user_species = species_of(user);
    const Species& target_species = species_of(target);

    --user.pp[move_slot];
    rng_next(state);

    if (!accuracy_check(state, user, move, target, user_species, target_species)) {
        return;
    }

    const auto apply_damage = [&](int damage) {
        if (damage <= 0) {
            return;
        }
        target.hp = static_cast<uint16_t>(damage >= target.hp ? 0 : target.hp - damage);
    };

    const auto heal_user = [&](uint16_t amount) {
        user.hp = static_cast<uint16_t>(std::min<int>(user.max_hp, user.hp + amount));
    };

    switch (move.effect) {
        case EFFECT_DAMAGE:
        case EFFECT_LEVEL_DAMAGE:
        case EFFECT_HYPER_BEAM:
        case EFFECT_EXPLODE:
        case EFFECT_DRAIN:
        case EFFECT_RECOIL: {
            const int damage = calculate_damage(state, user, user_species, move, target, target_species, 1 - side_idx);
            apply_damage(damage);
            if (move.effect == EFFECT_DRAIN) {
                const uint16_t heal = static_cast<uint16_t>((static_cast<int>(damage) * move.arg1) / 100);
                heal_user(heal);
            }
            if (move.effect == EFFECT_RECOIL) {
                const uint16_t recoil = static_cast<uint16_t>((static_cast<int>(damage) * move.arg1 + 99) / 100);
                const uint16_t recoil_damage = std::min<uint16_t>(recoil, user.hp - 1);
                user.hp = static_cast<uint16_t>(user.hp - recoil_damage);
            }
            if (move.effect == EFFECT_EXPLODE) {
                user.hp = 0;
            }
            if (move.effect == EFFECT_HYPER_BEAM) {
                user.volatiles |= VF_RECHARGE;
            }
            if ((move.effect == EFFECT_DAMAGE || move.effect == EFFECT_RECOIL) && target.hp > 0) {
                maybe_apply_secondary_status(state, target, move, target_species);
            }
            break;
        }
        case EFFECT_STATUS: {
            apply_status(state, target, move.arg1);
            break;
        }
        case EFFECT_HEAL: {
            const uint16_t heal = static_cast<uint16_t>((static_cast<int>(user.max_hp) * move.arg1 + 99) / 100);
            heal_user(heal);
            break;
        }
        case EFFECT_REST: {
            user.hp = user.max_hp;
            user.status = STATUS_SLEEP;
            user.sleep_turns = rng_range(state, 2u, 3u);
            break;
        }
        case EFFECT_STAT_UP: {
            modify_stage(user, static_cast<Stat>(move.arg1), static_cast<int8_t>(move.arg2));
            break;
        }
        case EFFECT_STAT_DOWN: {
            modify_stage(target, static_cast<Stat>(move.arg1), static_cast<int8_t>(-move.arg2));
            break;
        }
        case EFFECT_REFLECT: {
            // Gen 1: Screens are on/off and are lost on switch; mark side flag true
            state.sides[side_idx].reflect = true;
            break;
        }
        case EFFECT_LIGHT_SCREEN: {
            // Gen 1: Screens are on/off and are lost on switch; mark side flag true
            state.sides[side_idx].light_screen = true;
            break;
        }
        case EFFECT_SUBSTITUTE: {
            if (!(user.volatiles & VF_SUBSTITUTE)) {
                const uint16_t substitute_hp = std::max<uint16_t>(1, user.max_hp / 4);
                if (user.hp > substitute_hp) {
                    user.hp = static_cast<uint16_t>(user.hp - substitute_hp);
                    user.substitute_hp = static_cast<uint8_t>(std::min<uint16_t>(substitute_hp, 255u));
                    user.volatiles |= VF_SUBSTITUTE;
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void end_turn(Battle& state) {
    ++state.turn;
    rng_next(state);

    for (int side = 0; side < 2; ++side) {
        for (int idx = 0; idx < 6; ++idx) {
            Battler& battler = state.sides[side].battlers[idx];
            if (battler.hp == 0) {
                battler.volatiles &= ~VF_FLINCH;
                continue;
            }

            if (battler.status == STATUS_SLEEP) {
                if (battler.sleep_turns > 0) {
                    --battler.sleep_turns;
                }
                if (battler.sleep_turns == 0) {
                    battler.status = STATUS_NONE;
                }
            }

            if (battler.status == STATUS_BURN) {
                const uint16_t burn_damage = std::max<uint16_t>(1, battler.max_hp / 16);
                battler.hp = static_cast<uint16_t>(burn_damage >= battler.hp ? 0 : battler.hp - burn_damage);
            }

            if (battler.status == STATUS_POISON) {
                const uint16_t poison_damage = std::max<uint16_t>(1, battler.max_hp / 8);
                battler.hp = static_cast<uint16_t>(poison_damage >= battler.hp ? 0 : battler.hp - poison_damage);
            }

            battler.volatiles &= ~VF_FLINCH;
        }
        // Screens are cleared on switch; no per-turn decrement needed for on/off flags.
    }
}

uint64_t hash_battle(const Battle& state) noexcept {
    uint64_t hash = 1469598103934665603ULL;
    for (int side = 0; side < 2; ++side) {
        const uint32_t active = std::min<uint32_t>(state.sides[side].active, 5u);
        hash = mix_u64(hash, ZOBRIST_SIDE_ACTIVE[side * 6 + active]);
        for (int idx = 0; idx < 6; ++idx) {
            const Battler& battler = state.sides[side].battlers[idx];
            const uint32_t status_idx = std::min<uint32_t>(battler.status, ZOBRIST_STATUS.size() - 1);
            const uint32_t sleep_idx = std::min<uint32_t>(battler.sleep_turns, ZOBRIST_SLEEP.size() - 1);
            const uint32_t atk_stage = static_cast<uint32_t>(std::clamp<int>(battler.attack_stage, -6, 6) + 6);
            const uint32_t def_stage = static_cast<uint32_t>(std::clamp<int>(battler.defense_stage, -6, 6) + 6);
            const uint32_t spd_stage = static_cast<uint32_t>(std::clamp<int>(battler.speed_stage, -6, 6) + 6);
            const uint32_t spc_stage = static_cast<uint32_t>(std::clamp<int>(battler.special_stage, -6, 6) + 6);
            const uint32_t acc_stage = static_cast<uint32_t>(std::clamp<int>(battler.accuracy_stage, -6, 6) + 6);
            const uint32_t eva_stage = static_cast<uint32_t>(std::clamp<int>(battler.evasion_stage, -6, 6) + 6);

            hash = mix_u64(hash, ZOBRIST_STATUS[status_idx]);
            hash = mix_u64(hash, ZOBRIST_SLEEP[sleep_idx]);
            hash = mix_u64(hash, ZOBRIST_STAGE[atk_stage]);
            hash = mix_u64(hash, ZOBRIST_STAGE[def_stage]);
            hash = mix_u64(hash, ZOBRIST_STAGE[spd_stage]);
            hash = mix_u64(hash, ZOBRIST_STAGE[spc_stage]);
            hash = mix_u64(hash, ZOBRIST_STAGE[acc_stage]);
            hash = mix_u64(hash, ZOBRIST_STAGE[eva_stage]);
            for (uint32_t bit = 0; bit < 32; ++bit) {
                if (battler.volatiles & (1u << bit)) {
                    hash = mix_u64(hash, ZOBRIST_VOLATILE[bit]);
                }
            }
            const uint32_t conf_idx = std::min<uint32_t>(battler.confusion_turns, ZOBRIST_CONFUSION.size() - 1);
            const uint32_t sub_hp = std::min<uint32_t>(battler.substitute_hp, ZOBRIST_SUBSTITUTE.size() - 1);
            const uint32_t bide_turns = std::min<uint32_t>(battler.bide_turns, ZOBRIST_BIDE_TURNS.size() - 1);
            const uint32_t bide_dmg = static_cast<uint32_t>(battler.bide_damage);

            hash = mix_u64(hash, ZOBRIST_CONFUSION[conf_idx]);
            hash = mix_u64(hash, ZOBRIST_SUBSTITUTE[sub_hp]);
            hash = mix_u64(hash, ZOBRIST_BIDE_TURNS[bide_turns]);
            hash = mix_u64(hash, bide_dmg);
            for (int move_idx = 0; move_idx < 4; ++move_idx) {
                const uint32_t move_id = battler.moves[move_idx];
                const uint32_t pp_idx = battler.pp[move_idx];
                hash = mix_u64(hash, ZOBRIST_MOVE_ID[move_idx][move_id]);
                hash = mix_u64(hash, ZOBRIST_PP[move_idx][pp_idx]);
            }
            hash = mix_u64(hash, static_cast<uint64_t>(battler.species_id));
            hash = mix_u64(hash, static_cast<uint64_t>(battler.hp));
            hash = mix_u64(hash, static_cast<uint64_t>(battler.max_hp));
            hash = mix_u64(hash, static_cast<uint64_t>(battler.level));
            hash = mix_u64(hash, static_cast<uint64_t>(battler.attack));
            hash = mix_u64(hash, static_cast<uint64_t>(battler.defense));
            hash = mix_u64(hash, static_cast<uint64_t>(battler.speed));
            hash = mix_u64(hash, static_cast<uint64_t>(battler.special));
        }
        // include side-level reflect/light screen flags
        hash = mix_u64(hash, static_cast<uint64_t>(state.sides[side].reflect));
        hash = mix_u64(hash, static_cast<uint64_t>(state.sides[side].light_screen));
    }
    hash = mix_u64(hash, state.rng);
    hash = mix_u64(hash, state.turn);
    return hash;
}
