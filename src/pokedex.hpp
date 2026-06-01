// Lookup information and battle state for a compact RBY simulator
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>

// types
enum Type : uint8_t {
    NORMAL,
    FIRE,
    WATER,
    ELECTRIC,
    GRASS,
    ICE,
    FIGHTING,
    POISON,
    GROUND,
    FLYING,
    PSYCHIC,
    BUG,
    ROCK,
    GHOST,
    DRAGON
};

constexpr size_t TYPE_COUNT = 15;
constexpr uint8_t TYPE_MULT_0 = 0;
constexpr uint8_t TYPE_MULT_50 = 50;
constexpr uint8_t TYPE_MULT_100 = 100;
constexpr uint8_t TYPE_MULT_200 = 200;

// Goals is to be Pokemon Showdown compatible, so we use the same type effectiveness chart as Showdown, 
// which is based on Gen 1 mechanics (e.g. no Fairy type, no special split). Note that in Gen 1, the "Special" stat is used for both Special Attack and Special Defense, and the type effectiveness is determined by the move's type against the defender's types using a fixed chart.
inline constexpr std::array<std::array<uint8_t, TYPE_COUNT>, TYPE_COUNT> TYPE_EFFECTIVENESS = {{
    // Attacker: NORMAL
    {{TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_0, TYPE_MULT_100}},
    // FIRE
    {{TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_50}},
    // WATER
    {{TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_50}},
    // ELECTRIC
    {{TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_50, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_0, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_50}},
    // GRASS
    {{TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_200, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_50}},
    // ICE
    {{TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200}},
    // FIGHTING
    {{TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_50, TYPE_MULT_50, TYPE_MULT_200, TYPE_MULT_0, TYPE_MULT_100}},
    // POISON
    {{TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_50, TYPE_MULT_100}},
    // GROUND
    {{TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_0, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100}},
    // FLYING
    {{TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100}},
    // PSYCHIC
    {{TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100}},
    // BUG
    {{TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_50, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100}},
    // ROCK
    {{TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_50, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100}},
    // GHOST
    {{TYPE_MULT_0, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200, TYPE_MULT_100}},
    // DRAGON
    {{TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_100, TYPE_MULT_200}}
}};

inline constexpr uint8_t type_effectiveness(Type attack, Type defender) noexcept {
    return TYPE_EFFECTIVENESS[attack][defender];
}

inline constexpr uint8_t type_effectiveness(Type attack, Type defender1, Type defender2) noexcept {
    const uint16_t first = type_effectiveness(attack, defender1);
    const uint16_t second = type_effectiveness(attack, defender2);
    return uint8_t((first * second) / TYPE_MULT_100);
}

enum Status : uint8_t {
    STATUS_NONE,
    STATUS_SLEEP,
    STATUS_PARALYSIS,
    STATUS_BURN,
    STATUS_POISON,
    STATUS_FREEZE
};

enum VolatileFlags : uint32_t {
    VF_NONE          = 0,

    VF_CONFUSION     = 1 << 0,
    VF_FLINCH        = 1 << 1,

    VF_REFLECT       = 1 << 2,
    VF_LIGHT_SCREEN  = 1 << 3,

    VF_LEECH_SEED    = 1 << 4,
    VF_MIST          = 1 << 5,

    VF_SUBSTITUTE    = 1 << 6,

    VF_BIDE          = 1 << 7,
    VF_RAGE          = 1 << 8,

    VF_FOCUS_ENERGY  = 1 << 9,

    VF_THRASHING     = 1 << 10,
    VF_RECHARGE      = 1 << 11,
};

enum Stat : uint8_t {
    STAT_ATTACK,
    STAT_DEFENSE,
    STAT_SPEED,
    STAT_SPECIAL,
    STAT_ACCURACY,
    STAT_EVASION
};

enum Effect : uint8_t {
    EFFECT_NONE,
    EFFECT_DAMAGE,
    EFFECT_LEVEL_DAMAGE,
    EFFECT_STATUS,
    EFFECT_STAT_UP,
    EFFECT_STAT_DOWN,
    EFFECT_HEAL,
    EFFECT_REST,
    EFFECT_RECOIL,
    EFFECT_DRAIN,
    EFFECT_HYPER_BEAM,
    EFFECT_EXPLODE,
    EFFECT_LEECH_SEED,
    EFFECT_SUBSTITUTE,
    EFFECT_REFLECT,
    EFFECT_LIGHT_SCREEN,
    EFFECT_CONFUSE,
    EFFECT_BIDE,
    EFFECT_RAGE,
    EFFECT_CONVERSION,
    EFFECT_TRANSFORM,
    EFFECT_METRONOME,
    EFFECT_MIRROR_MOVE
};

struct Move {
    uint8_t type;
    uint8_t power;
    uint8_t accuracy;
    uint8_t pp;
    uint8_t effect;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t chance;
};

inline constexpr uint8_t type_effectiveness(const Move& move, Type defender1, Type defender2) noexcept {
    return type_effectiveness(Type(move.type), defender1, defender2);
}

struct Species {
    uint16_t base_hp;
    uint16_t base_attack;
    uint16_t base_defense;
    uint16_t base_speed;
    uint16_t base_special;
    uint8_t type1;
    uint8_t type2;
};

#include "move_defs.hpp"
#include "species_defs.hpp"



// Per-battle mutable state for a single battler (small, POD, cheap to copy)
struct Battler {
    uint16_t hp;
    uint16_t max_hp;
    uint8_t level;
    uint16_t attack;
    uint16_t defense;
    uint16_t speed;
    uint16_t special;
    SpeciesId species_id;

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

    uint8_t moves[4];
    uint8_t pp[4];
};

struct Side {
    Battler battlers[6];
    uint8_t active; // index of active battler in party
    bool reflect;
    bool light_screen;
};

struct Battle {
    Side sides[2];
    uint32_t rng;
    uint16_t turn;
};

// Function prototypes (explicit types for clarity and to be implemented)
int modified_attack(const Battler& b, const Species& s);
int modified_defense(const Battler& b, const Species& s);
int modified_special(const Battler& b, const Species& s);
int modified_speed(const Battler& b, const Species& s);

bool accuracy_check(Battle& state, const Battler& attacker, const Move& move, const Battler& defender, const Species& atk_species, const Species& def_species);

bool maybe_apply_secondary_status(Battle& state, Battler& target, const Move& move, const Species& target_species);

int calculate_damage(Battle& state, const Battler& attacker, const Species& atk_species, const Move& move, const Battler& defender, const Species& def_species, int defender_side = -1);

void apply_status(Battle& state, Battler& target, uint8_t status);

void modify_stage(Battler& target, Stat stat, int8_t delta);

bool switch_pokemon(Battle& state, int side_idx, uint8_t new_active);

bool has_usable_pokemon(const Side& side) noexcept;
bool is_battle_over(const Battle& state) noexcept;
int winning_side(const Battle& state) noexcept;

void use_move(Battle& state, int side_idx, int battler_idx, uint8_t move_slot);

void end_turn(Battle& state);

inline uint32_t rng_step(uint32_t value) noexcept {
    return value * 1103515245u + 12345u;
}

inline uint32_t rng_next(Battle& state) noexcept {
    return state.rng = rng_step(state.rng);
}

inline uint8_t rng_u8(Battle& state) noexcept {
    return static_cast<uint8_t>(rng_next(state) & 0xFFu);
}

inline bool rng_chance_256(Battle& state, uint8_t threshold) noexcept {
    return rng_u8(state) < threshold;
}

inline uint8_t rng_range(Battle& state, uint8_t lo, uint8_t hi) noexcept {
    if (lo == hi) {
        return lo;
    }
    const uint16_t range = static_cast<uint16_t>(hi) - static_cast<uint16_t>(lo) + 1u;
    if (range == 256u) {
        return rng_u8(state);
    }
    const uint16_t limit = static_cast<uint16_t>(256u - (256u % range));
    while (true) {
        const uint16_t value = rng_u8(state);
        if (value < limit) {
            return static_cast<uint8_t>(lo + static_cast<uint8_t>(value % range));
        }
    }
}

inline Battle clone_battle(const Battle& source) noexcept {
    static_assert(std::is_trivially_copyable_v<Battle>, "Battle must be trivially copyable for fast copying");
    return source; // allow compiler to optimize copying
}

inline void copy_battle(const Battle& source, Battle& destination) noexcept {
    static_assert(std::is_trivially_copyable_v<Battle>, "Battle must be trivially copyable for fast copying");
    destination = source;
}

uint64_t hash_battle(const Battle& state) noexcept;
