
# BileBio Design Document

## Goals

* KISS (Keep it simple stupid), interface, mechanics, controls, everything.
  * This will be in the back of the mind constantly.
* Porting to mobile.
  * The project will move from C to C++ and use SFML (which claims that it will have mobile support in the future)
* Moving from text to animated sprites.
  * SFML
  * Placeholder graphics for now. Animation later.
* Random, but smaller maps.
  * I can either use my libdrunkard library, or something simpler.
* Simpler, more predictable plant growth. (Realtime)
  * The plants have to be a problem immediately and gradually become more of a threat.
  * Maze like caves where you don't know the exit. Plants immediately in the way.
  * Plants grow predictable outwards at a specific rate. There is a "growth animation" on where it's currently growing.
  * Each animal has it's own way of dealing with the plants.
* Simpler, more intuitive skill progression. (And controls will follow)
  * WIP
* Animal as the playable character. (Probably several to pick from)

## Mechanics

### Player

The player will have two stats: Health and energy. Energy's use depends on the animal, but in
all cases it denotes how tired the player is. Three for each is about average.

Each animal, asside from having energy, has 3 basic special moves. These three moves
can be selected with the keys 1, 2, and 3. When pressing shift+direction, you use the
special towards that direction. Sometimes a special move doesn't require a direction
in this case pressing any direction with shift works. Some specials don't even need
to be activated. Sometimes holding shift will be required if the special has a range.

Pressing '4' selects a special item, this item can be used in the same way a special
move can. You can only carry one special item at a time.

When you kill a root, or just by exploring, you have a chance to find upgrades. These
can increase your health, energy, damage, or skills. However, you can only place 5 skills
and 10 attributes.

#### Possible Animals

* Jaguar (or Panther). Can pounce, average health, strong energy, strong attack.
* Snake. Can slither through vines, weak health, average energy, weak attack.
* Bear. Strong health, strong energy, average attack.
* Tiger. Strong health, average energy, strong attack.

#### Panther

The first playable animal will be the panther. Which has the following stats:

3 hearts (+1)

3 energy (+1)

2 damage, 66% to hit (+0.5)

##### 1. Pounce
Pounces to a target location dealing double damage that can't miss to a random
target within 1 square.

**Level 1**: 1 energy, 3 max pounce range

**Level 2**: 1 energy, 4 max pounce range

**Level 3**: 1 energy, 5 max pounce range
##### 2. Sneak
**Not sure**
##### 3. Fury
Deals damage to every target within 1 square around the panther.

**Level 1**: 1 energy, 1 bonus damage

**Level 2**: 1 energy, 2 bonus damage

**Level 3**: 1 energy, 3 bonus damage
### Plants

Plants are not hostile by default, but they can grow hostile flowers, vines, but also fruit
to replenish energy. Perhaps you can even find treasure that you can feed the plant to
make it grow something or change it's disposition.

As you progress through the levels, plants increase in hostility and grow more interesting
and deadly flowers and vines.

Plants have 3 dispositions: evil, neutral, good. Plants in a good disposition grow
nice, unharming plants and fruits, plants in an evil disposition grow harmful plants and
deadly flowers.

**Types of plants**
* Roots, roots grow vines right next to it.
* Vines, vines grow at any square within 5.
  * Vines
  * Thick vines (lots of health)
  * Spiny vines (attacks when within 1 square of it)
* Flowers
  * Red flowers (spits needles from 5 squares away)
  * Blue flowers
* Fruits
  * Red fruit
  * Blue fruit

Only roots and vines grow more roots and vines. A growth process is typically 3
turns, and a plant can only grow 1 thing at a time. Vines grow *towards* the
player if the player is in range, usually as far away as possible to *trap* the player.

## TODO

[-] Get a basic framework: Menu, Settings, etc.
[-] Get a character moving around.
