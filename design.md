
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

The player will have two stats: Health and energy. Energy's use depends on the class, but in
all cases it denotes how tired the player is. Three for each is about average.

### Plants

Plants are not hostile by default, but they can grow hostile flowers, vines, but also fruit
to replenish energy.

## TODO

[-] Get a basic framework: Menu, Settings, etc.
[-] Get a character moving around.
