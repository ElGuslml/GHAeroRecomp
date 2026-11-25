# The game's own debug menu

**Status: added, off by default (2026-11-24):** the launcher's Settings → Game → "Game debug menu" row, or
`ghaerosmith_debug_menu = true` in `ghaerosmith.toml` (or `--ghaerosmith_debug_menu=true`).

## What it is

The game ships Neversoft's debug menu (`scripts\guitar\guitar_debug_menu.qb`). With the option on, the main menu gets a
**Debug Menu** item at its end and the pause menu a **Debug** item. It has, among others: Unlock All, Repeat Last Song,
Play Song, Settings, Character Select, Skip Into Song, Save Replay Buffer, Load Replay, Play Credits, Difficulty
Analyzer, Toggle Changelist Number, UI Physics Test. Its texts are in English whatever the game's language. Some items
need Neversoft's development tools and may do nothing. Unlock All changes the save like playing does.

## How

The menus add the item when the script global `enable_button_cheats` is 1; the game defines it as 0. The same global
also turns on development shortcuts that change the save without asking (unlocking every song or completing one from
the setlist, the store's money) and, read by `guitar_startup`, Neversoft's viewer. So the global stays 0, and
`src/debug_menu.cpp` (hooks in `ghaerosmith_functions.toml`) makes it read as 1 only in the two scripts that build the
main menu and the pause menu, as GH Metallica's does: Guitar Hero III's interpreter resolves a `$name` in
`sub_8219BF30`, and its statement (`sub_82179D38`) says which script is evaluating. The log names each script that
read it ("offers the debug menu" or "left at 0").

## Turning it off

The launcher's row off, or `ghaerosmith_debug_menu = false` (the default): the menus are as on the console.
