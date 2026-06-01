#pragma once

#include <array>
#include <cstdint>

enum SpeciesId : uint8_t {
    SPECIES_GENGAR,
    SPECIES_EXEGGUTOR,
    SPECIES_ZAPDOS,
    SPECIES_CHANSEY,
    SPECIES_SNORLAX,
    SPECIES_TAUROS,
    SPECIES_STARMIE,
    SPECIES_JYNX,
    SPECIES_RHYDON,
    SPECIES_COUNT
};

inline constexpr std::array<Species, SPECIES_COUNT> SPECIES_DEFS = {{
    {60, 65, 60, 110, 130, GHOST, POISON},
    {95, 95, 85, 55, 125, GRASS, PSYCHIC},
    {90, 90, 85, 100, 125, ELECTRIC, FLYING},
    {250, 5, 5, 50, 105, NORMAL, NORMAL},
    {160, 110, 65, 30, 65, NORMAL, NORMAL},
    {100, 100, 70, 110, 75, NORMAL, NORMAL},
    {60, 75, 85, 115, 100, WATER, PSYCHIC},
    {65, 50, 35, 95, 95, ICE, PSYCHIC},
    {105, 130, 120, 40, 45, GROUND, ROCK}
}};

inline constexpr const Species& species_def(SpeciesId id) noexcept {
    return SPECIES_DEFS[static_cast<size_t>(id)];
}
