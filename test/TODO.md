1. Golden primitive tests

Exact expected result for each mechanic:

damage()
accuracy()
crit()
secondary effects()
status application()
switch legality()
turn order()
end-of-turn effects()

These should assert:

input state + action + seed
→ output state
→ RNG consumed
→ event log
→ legal actions
2. Cross-oracle tests

Compare against a trusted simulator, ideally:

Pokemon Showdown Gen 1
pkmn engine
known cartridge research cases

Use fixed teams, fixed seeds, and thousands of generated states. Any mismatch produces a minimized repro state.

3. Replay validation tests

Feed real RBY battle logs:

initial teams + observed choices + observed outcomes

Assert that every observed event was legal and reachable. This catches sequencing bugs better than isolated tests.

4. Exhaustive small-state tests

For compact mechanics, brute-force all relevant combinations:

status × type × move effect
stage -6..+6 × base stat
accuracy stage × evasion stage
fainted/alive × switch target
PP 0/nonzero × disabled/locked moves

This is ideal for RBY because many mechanics are table-like.

5. Property/invariant tests

Assert things that must always hold:

0 <= hp <= max_hp
stages always in [-6, +6]
PP never negative
fainted Pokémon cannot act
same seed + same actions = same result
clone + step == original + step
hash changes when meaningful state changes
legal actions never include impossible moves/switches
6. RNG-consumption tests

Very important for replay/MCTS.

For every action branch, assert exact RNG usage:

missed move consumes accuracy roll only
hit damaging move consumes accuracy + crit + damage
secondary effect consumes only when eligible
switch consumes no RNG

Also log roll purpose:

RNG_ACC, RNG_CRIT, RNG_DAMAGE, RNG_SECONDARY
7. Serialization/hash tests

For dataset/search use:

encode(decode(state)) == state
hash(state) deterministic
equal states have equal hash
different species/PP/status/RNG/volatiles affect hash
clone is bit-identical
8. Performance regression tests

Keep fixed benchmarks in CI:

clone/sec
step/sec
legal_actions/sec
encode_batch/sec
self-play games/sec