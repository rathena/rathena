# AI Agent Instructions for roBrowserLegacy

## Project Overview

roBrowserLegacy is a web-based Ragnarok Online client built with ES6 modules and WebGL. It supports multiple platforms (browser, PWA, Electron desktop) and provides a complete game client experience.

### Key Statistics

- **14 subsystems** in `src/` (App, Audio, Controls, Core, DB, Engine, Loaders, Network, Plugins, Preferences, Renderer, UI, Utils, Vendors)
- **7 application entry points** in `src/App/`
- **99 UI Components** in `src/UI/Components/`
- **40 Renderer Effect modules** (31 effect JS files + 9 post-processing shaders)
- **17 Entity modules** in `src/Renderer/Entity/`
- **28 MapEngine subsystems** in `src/Engine/MapEngine/`
- **23 packet versions** supported (kRO 2003–2025) in `src/Network/Packets/`

## Architecture

### ES6 Modules

All source files use ES6 `import`/`export`. Vite bundles them into optimized ES modules.

```javascript
import { BinaryReader } from 'Utils/BinaryReader.js';
import Configs from 'Core/Configs.js';
```

- Base path: `src/` (directory structure mirrors module paths)
- Output: `dist/Web/` for web, `dist/Desktop/` for Electron
- Build: Vite + custom builder script (`applications/tools/builder-web.mjs`)

### Module Path Aliases

| Alias       | Path             |
| ----------- | ---------------- |
| App         | src/App/         |
| Audio       | src/Audio/       |
| Controls    | src/Controls/    |
| Core        | src/Core/        |
| DB          | src/DB/          |
| Engine      | src/Engine/      |
| Loaders     | src/Loaders/     |
| Network     | src/Network/     |
| Plugins     | src/Plugins/     |
| Preferences | src/Preferences/ |
| Renderer    | src/Renderer/    |
| UI          | src/UI/          |
| Utils       | src/Utils/       |
| Vendors     | src/Vendors/     |

Example: `import Sprite from 'Loaders/Sprite.js';`

### Core Service Layers

1. **Engine** (`src/Engine/`): GameEngine (orchestrator), LoginEngine (auth), CharEngine (character), MapEngine (28 subsystems), SessionStorage
2. **Network** (`src/Network/`): PacketRegister, PacketStructure, PacketCrypt, PacketVersions.js, PacketVerManager, PacketLength, NetworkManager, SocketHelpers (WebSocket/NodeSocket)
3. **Loaders** (`src/Loaders/`): 12 asset parsers — GameFileDecrypt, GameFile, Sprite, Action, Ground (.gnd), Altitude (.gat), World (.rsw), Str (.str effects), Model, GrannyModel, MapLoader, Targa
4. **Database** (`src/DB/`): DBManager + data subdirectories (Effects, Items, Jobs, Map, Monsters, Pets, Skills, Status) + Emotions, TownInfo
5. **Renderer** (`src/Renderer/`): Renderer.js (WebGL context), MapRenderer, EntityManager, EffectManager, ScreenEffectManager, SignboardManager, Camera, SpriteRenderer, ItemObject + subdirectories (Effects, Entity, Map)
6. **UI** (`src/UI/`): UIManager, UIVersionManager, GUIComponent (new base class, Shadow DOM), Custom Elements (`src/UI/Elements/`), CursorManager, Scrollbar, Background, 95 component directories
7. **Controls** (`src/Controls/`): EntityControl, MapControl, KeyEventHandler, MouseEventHandler, ProcessCommand, BattleMode, ScreenShot
8. **Audio** (`src/Audio/`): BGM.js, SoundManager.js
9. **Core** (`src/Core/`): Client, Configs, Context, Events, FileManager, FileSystem, MemoryItem, MemoryManager, Mobile, Preferences, Thread, ThreadEventHandler, AIDriver
10. **Preferences** (`src/Preferences/`): Audio, Graphics, Controls, UI, Camera, Map, ShortCutControls (7 modules)
11. **Plugins** (`src/Plugins/`): PluginManager.js
12. **Utils** (`src/Utils/`): BinaryReader, BinaryWriter, Struct, Inflate, PathFinding, WebGL, Texture, Executable, CRC32, CodepageManager, ConsoleManager, HTMLEntity, Queue, Base62, colors, partyColors, gl-matrix

### Data Flow

1. **Init**: GameEngine loads config → Client loads GRFs/executable → PacketVerManager detects PACKETVER from executable date
2. **Login**: LoginEngine → NetworkManager connects via WebSocket (browser) or NodeSocket (Electron) → authenticates with versioned packets
3. **Map Entry**: MapEngine loads map/entity data → Renderer displays → Controls enable input → 28 MapEngine subsystems manage live state
4. **Runtime**: InputHandler → ProcessCommand → NetworkManager sends → packet handlers update state → EntityManager updates → Renderer draws

### Multi-Platform Support

- **Browser**: WebSocket via wsProxy, ES modules loaded as `<script type='module'>`
- **PWA**: Progressive Web App with manifest and service worker support
- **Electron Desktop**: Direct TCP via NodeSocket (Electron preload), packaged executables

## Key Architectural Decisions

- **Entity system uses composition, not inheritance.** 17 mixin modules (`EntityWalk`, `EntityCast`, `EntityState`, `EntityRender`, etc.) are mixed into `Entity.js`. Do not refactor to class inheritance — the mixins are applied dynamically at runtime.
- **Packet versioning is date-based.** PACKETVER is auto-detected from the kRO executable's PE timestamp. `PacketVersions.js` uses date-range switches. Changing detection logic in `PacketVerManager.js` can break compatibility with 23 packet versions.
- **Two socket paths exist by design.** Browser uses `WebSocket.js` (requires wsProxy for TCP↔WS translation). Electron uses `NodeSocket.js` (direct TCP via preload contextBridge). Both implement the same interface for `NetworkManager`.
- **Vendors are frozen.** `src/Vendors/` is excluded from ESLint. Never modify vendored files.
- **UI has one component system** `GUIComponent` uses Shadow DOM + native DOM + Custom Elements. New components must use GUIComponent. Primary guide: `docs/UIComponent_to_GUIComponent.md` (L1). For edge cases see `docs/UIComponent_to_GUIComponent_Scars.md` (L0, archive); for quick reflex lookup `docs/UIComponent_to_GUIComponent_Firmware.md` (L2).
- **UI is asset-driven, not CSS-driven.** Window frames, buttons, and backgrounds come from BMP images in GRF files. Legacy components use `data-background` HTML attributes processed by `GUIComponent.parseHTML()`. New components use `<ui-button>`, `<ui-text>`, `<ui-image>` Custom Elements, both are useful. CSS is structural/positional only.

## Subsystem Reference

### Network Packet Handling

- **NetworkManager.js**: Connection management and packet dispatch
- **PacketRegister.js**: Maps packet IDs (0x69, 0x6a, etc.) to handlers
- **PacketStructure.js**: Defines AC/ZC/HC packet schemas
- **PacketVersions.js**: Per-version packet definitions (large switch on date)
- **PacketLength.js**: Packet length definitions
- **PacketCrypt.js**: Packet encryption/decryption
- **PacketVerManager.js**: Automatic PACKETVER detection from executable metadata
- **Packets/**: 23 version-specific length files (`packets2003_len_main.js` through `packets2025_len_main.js`)
- **SocketHelpers/**: WebSocket.js (browser), NodeSocket.js (Electron desktop)
- Pattern: Add handler in PacketRegister → define structure in PacketStructure → implement response in appropriate engine

### Asset Loaders (`src/Loaders/`)

- **GameFileDecrypt.js**: Decrypt GRF files (DES/RC4)
- **GameFile.js**: GRF file container parsing
- **Action.js**: Parse .act (sprite animation frames)
- **Sprite.js**: Parse .spr (sprite texture/palette data)
- **Ground.js**: Parse .gnd (ground mesh/texture data)
- **Altitude.js**: Parse .gat (map height/collision data)
- **World.js**: Parse .rsw (resource world — models, lights, sounds)
- **Str.js**: Parse .str (visual effect files, compiled .ezv format)
- **Model.js**: Parse .rsm (3D model data)
- **GrannyModel.js**: Parse Granny RSAC format (3D skeletal models)
- **MapLoader.js**: Map loading orchestration
- **Targa.js**: Parse .tga (image format)

### MapEngine Subsystems (`src/Engine/MapEngine/`, 28 files)

- **Main.js**: Core game loop and state management
- **Entity.js**: Main entity controller
- **UIOpen.js**: UI component lifecycle
- **MapState.js**: State persistence
- **Achievement.js, Quest.js, Guild.js, Clan.js**: Progress systems
- **Group.js, Friends.js, PrivateMessage.js, ChatRoom.js**: Social features
- **CashShop.js, Bank.js, Roulette.js**: Commerce systems
- **Homun.js, Pet.js, Mercenary.js**: Companion systems
- **NPC.js, Store.js, Trade.js**: Interaction systems
- **Item.js, Storage.js, Skill.js**: Core gameplay
- **Mail.js, Rodex.js**: Messaging systems
- **Captcha.js, PCGoldTimer.js**: Security/timer systems

### GUI Components (`src/UI/Components/`, 95 directories)

- GUIComponent uses `<ui-button>`, `<ui-text>`, `<ui-image>` (registered in `src/UI/Elements/Elements.js`) instead of `data-background`/`data-hover`/`data-down`/`data-text` attributes
- `this.ui` proxy on GUIComponent provides jQuery-compatible API for UIManager interop
- Both types coexist in UIManager — `addComponent()` accepts either

    > **Migration docs (3-layer memory):**
    >
    > - **L1 — Primary** [`docs/UIComponent_to_GUIComponent.md`](docs/UIComponent_to_GUIComponent.md): consolidated operational memory (invariants, rules, failure matrix). **Start here.**
    > - **L2 — Reflex** [`docs/UIComponent_to_GUIComponent_Firmware.md`](docs/UIComponent_to_GUIComponent_Firmware.md): compressed decision matrix for rapid lookup. Not for reasoning.
    > - **L0 — Archive** [`docs/UIComponent_to_GUIComponent_Scars.md`](docs/UIComponent_to_GUIComponent_Scars.md): full historical guide, postmortems, per-component reference. Consult only for edge cases / when L1 is insufficient.
    >   **Custom Elements**: [`docs/CustomElements.md`](docs/CustomElements.md) — reference for `<ui-button>`, `<ui-text>`, `<ui-image>` and how to create new ones
    >   **For Migrated & Remaining components list see:** https://github.com/MrAntares/roBrowserLegacy/issues/1140

- **Intro.js**: File upload & server selection
- **WinList.js**: Character selection
- **WinLogin.js**: Account authentication
- **Inventory, Equipment, Storage, Bank, CashShop, CartItems**: Item management
- **ChatBox, ChatRoom, ChatRoomCreate, WhisperBox, ChatBoxSettings**: Communication
- **NpcBox, NpcStore, NpcMenu**: NPC interaction
- **SkillList, SkillListMH, SkillDescription, SkillTargetSelection**: Skill system
- **Quest, Mail, Rodex, Trade, Vending, VendingShop, VendingReport**: Game systems
- **Refine, RefineWeaponSelection, Enchant, EnchantGrade, ItemCompare, ItemReform, LaphineSys, LaphineUpg**: Crafting/upgrade
- **Clan, Guild, PartyFriends, EntityRoom**: Social systems
- **MiniMap, WorldMap, Navigation, MapName**: Navigation
- **GrfViewer, ModelViewer, StrViewer, EffectViewer, GrannyModelViewer**: Asset viewers
- **BasicInfo, WinStats, StatusIcons, Sense**: Character info
- **ShortCut, ShortCuts, ShortCutOption**: Hotkey systems
- **Emoticons, Announce, FPS, Escape, Error, WinPopup, WinPrompt**: Misc UI
- **CharCreate, CharSelect, PincodeWindow**: Character management
- **HomunInformations, MercenaryInformations, PetInformations, PetEvolution**: Companion UI
- **CardIllustration, ItemInfo, ItemObtain, ItemPreview, ItemSelection**: Item display
- **MakeArrowSelection, MakeItemSelection, MakeReadBook**: Crafting UI
- **PlayerViewEquip, SwitchEquip, ChangeCart**: Equipment UI
- **GraphicsOption, SoundOption**: Settings
- **Captcha, CheckAttendance, Reputation, SlotMachine, PvPCount, PvPTimer, PCGoldTimer**: Misc systems
- **ContextMenu, EntitySignboard, JoystickUI, MobileUI**: Interface elements

### Renderer Effects (`src/Renderer/Effects/`)

- **Weather**: CloudWeatherEffect, PokJukWeatherEffect, RainWeather, SakuraWeatherEffect, SnowWeather, Sky
- **Auras/Spheres**: GroundAura, SwirlingAura, SpiritSphere, WarlockSphere, Level99Bubble, QuadHorn
- **Ground**: GroundEffect, FlatColorTile, PropertyGround, LPEffect, SpiderWeb, Tiles
- **Combat**: Damage, MagnumBreak, PoisonEffect, MagicTarget, MagicRing, LockOnTarget
- **Visual**: StrEffect, RsmEffect, ThreeDEffect, TwoDEffect, Cylinder, Songs
- **Screen**: PostProcess
- **Post-Processing Shaders** (`Effects/Shaders/`): Blind, Bloom, CAS, Cartoon, FXAA, GaussianBlur, Upsampling, VerticalFlip, Vibrance

## Code Conventions

### Style Rules

Formatting is handled by **Prettier** (defaults). ESLint extends `eslint:recommended` and `prettier`.

- **Quotes**: Single quotes (`avoidEscape: true`)
- **Semicolons**: Required
- **Trailing commas**: Forbidden (`comma-dangle: never`)
- **Curly braces**: Always required (`curly: 'all'`)
- **Variables**: `const` required where possible, `let` over `var`, `no-var: error`
- **No eval/implied-eval/new-func/with**: Error
- **Unused vars**: Warn (vars prefixed `_` ignored)
- **Vendors excluded**: `src/Vendors/**` ignored by ESLint

### Naming Conventions

- **Private vars**: Prefix with `_` (e.g., `_sockets`, `_currentSocket`)
- **Constants**: UPPER_SNAKE_CASE in db/const files
- **Classes/Constructors**: PascalCase (e.g., `EntityControl`, `BinaryReader`)
- **Functions/Methods**: camelCase (e.g., `sendPacket()`, `parseEntity()`)

### Modern Patterns (Preferred)

When writing new code or modifying existing code, **use modern JavaScript features**: arrow functions, template literals, `async`/`await`, ES6 classes, destructuring, spread/rest, native `Promise`, `for...of`, default parameters.

### Modernize on Touch

When touching files with legacy patterns, convert them:

| Legacy Pattern                                           | Modern Replacement                                    |
| -------------------------------------------------------- | ----------------------------------------------------- |
| `function(a, b) { ... }` callbacks                       | `(a, b) => { ... }`                                   |
| `'string ' + variable + ' end'`                          | `` `string ${variable} end` ``                        |
| `function Constructor() { this.x = 1; }`                 | `class Constructor { constructor() { this.x = 1; } }` |
| `Constructor.prototype.method = function() {}`           | `class Constructor { method() {} }`                   |
| `const Singleton = {}; Singleton.method = function() {}` | `class Singleton { static method() {} }`              |
| `var x = obj.x \|\| defaultVal;`                         | `const { x = defaultVal } = obj;` or default params   |
| Global assignments (`_global.CONST = val`)               | Explicit ES6 imports in all consumers                 |

> **Arrow function `this` caveat:** Arrow functions capture `this` lexically. Verify `this` usage before converting jQuery event handlers or prototype methods. If a function relies on dynamic `this`, keep it as a regular function or convert the whole module to a class first.

### Modernization Examples

The two most common non-obvious legacy patterns in this codebase:

**Object-literal singletons → static class:**

```javascript
// Before (found in FileManager, UIManager, NetworkManager, etc.)
const FileManager = {};
FileManager.remoteClient = '';
FileManager.load = function load(path) { /* ... */ };
export default FileManager;

// After
class FileManager {
    static remoteClient = '';
    static load(path) { /* ... */ }
}
export default FileManager;
```

### Globals Still in Use

- **ROConfig**: Runtime configuration object
- **SEEK_CUR, SEEK_SET, SEEK_END**: BinaryReader constants (prefer importing from `Utils/BinaryReader.js`)

## Config System

- **Config.js**: Auto-generated default settings (servers, features, preferences)
- **Config.local.js**: Optional local overrides (not committed)
- **ROConfig**: Merged configuration object loaded at runtime

Key options: `servers` (address/port/version), `packetDump` (packet logging), `skipIntro` (bypass intro), `remoteClient` (GRF asset server URL), `plugins`, `aura`, `autoLogin`.

## Build & Development

```bash
npm run dev             # Vite dev server
npm run build:online    # Build full client (Online.js)
npm run build:all       # Build all 7 apps
npm run build:pwa       # Build PWA (Online + Thread + manifest)
npm run electron        # Run Electron desktop app
npm run electron:dev    # Run Electron in dev mode (with DevTools)
npm run electron:build  # Package Electron app (output: dist/Desktop/)
npm run lint            # ESLint check
npm run lint:fix        # ESLint auto-fix
npm run format          # Prettier format
npm run ci              # lint + format:check (CI pipeline)
```

Full script list in `package.json`. `ThreadEventHandler.js` (Web Worker) is built separately via `npm run build:threadhandler`.

### Infrastructure Requirements

- **wsProxy**: WebSocket proxy (server-side) translating TCP ↔ WebSocket
- **Remote Client (PHP/JS)**: HTTP server extracting/serving GRF assets
- **Game Server**: rAthena or Hercules compatible
- **kRO Client Files**: GRF assets + executable for PACKETVER detection

## Common Development Tasks

### Adding a New UI Component

**New components should use GUIComponent (Shadow DOM):**

1. Create directory in `src/UI/Components/NewComponent/`
2. Create `NewComponent.js`, `NewComponent.html`, `NewComponent.css`
3. Use `new GUIComponent('Name', cssText)` with `render()` returning HTML
4. Use Custom Elements (`<ui-button>`, `<ui-text>`, `<ui-image>`) instead of `data-*` attributes
5. CSS: dimensions/position on `:host`, inner layout on `#Name`
6. Register with `UIManager.addComponent()`
7. Add keyboard shortcut in `src/Controls/ProcessCommand.js` if needed
    > Consult **L1** [`docs/UIComponent_to_GUIComponent.md`](docs/UIComponent_to_GUIComponent.md) first; fall back to **L0** [`_Scars.md`](docs/UIComponent_to_GUIComponent_Scars.md) for the full step-by-step checklist and pitfalls.  
    > See [`docs/CustomElements.md`](docs/CustomElements.md) for element reference
    > **Migrating an existing UIComponent:** Follow the step-by-step checklist in the migration guide. Key pitfalls: jQuery `.show()`/`.hide()` inside Shadow DOM, `$el.closest('body')` → `el.isConnected`, CSS `:host` dimensions.

### Implementing a New Packet Handler

1. Define packet structure in `src/Network/PacketStructure.js`
2. Register handler in `src/Network/PacketRegister.js`
3. Implement handler in appropriate engine (MapEngine, LoginEngine, etc.)
4. Handle version-specific differences via PacketVersions.js
5. Test with `packetDump` enabled in config

### Adding a New Packet Version

1. Create file in `src/Network/Packets/` (e.g., `packets2026_len_main.js`)
2. Define packet lengths for all known packets in that version
3. Update `PacketVerManager.js` to include the new date range
4. Test against server with matching PACKETVER

### Creating a New Renderer Effect

1. Create effect module in `src/Renderer/Effects/`
2. Implement with `update()` and `render()` methods
3. Register with EffectManager
4. Test performance impact and object pooling

### (Factory/Deduplication Task) Deduplicating JS code in multiple versions to unified shared factory pattern

**Before starting, you MUST read `docs/GUIComponent_Version_Dedup_Factory.md` — it contains invariants that cannot be violated.**

1. Create a ComponentNameCommon.js in Component Folder
2. Move all JS code from `V0` or `V1` to new Common Factory
3. Add each JS variation from other versions, creating a new entry on the factory function header

## Troubleshooting

- **Missing assets**: Verify GRF loading in Client.js, check Remote Client response
- **Packet errors**: Check PACKETVER matches server, verify PacketVersions.js, enable packetDump
- **Rendering issues**: Check WebGL capability (OpenGL ES 2.0 required), inspect Renderer.js context
- **Build errors**: Clear `dist/`, verify entry point exists in `src/App/`
- **Connection fails**: Check wsProxy configuration, firewall settings
- **Module not found**: Check path aliases in vite.config.js and file extensions

## Important Gotchas

- **PACKETVER mismatch**: Client and server packet definitions must align; debug by comparing hex packets
- **GRF paths**: Case-sensitive on Linux; relative to Remote Client or local file list
- **Texture limits**: WebGL has max texture dimensions; large sprite sheets can fail
- **Path aliases**: Always use aliases (e.g., `'Utils/BinaryReader.js'`) instead of relative paths
- **Global removal requires audit**: Before removing a global assignment (e.g., `SEEK_CUR`), verify all consumers including Web Workers have been updated to use ES6 imports

## Glossary

- **GID**: Global ID — unique identifier for game entities
- **GRF**: Game Resource File — container format for Ragnarok assets
- **PACKETVER**: Packet version — determines network protocol format based on kRO client date
- **wsProxy**: WebSocket proxy for TCP ↔ WebSocket translation
- **Entity**: Game object (player, NPC, monster) with rendering and state
- **Effect**: Visual effect (weather, auras, particles) managed by EffectManager
- **kRO**: Korean Ragnarok Online — reference client for packet versions

## Debugging

### Enabling Debug Output

- Set `development: true` in ROConfig to enable console output
- Set `enableConsole: true` for console without full dev mode
- ConsoleManager (`src/Utils/ConsoleManager.js`) silences ALL console  
  output in production — if you see no logs, check these flags first

### Network Debugging

- Set `packetDump: true` in ROConfig to log all packets
- Compare hex dumps against PacketStructure.js definitions
- Check PACKETVER detection: NetworkManager logs the detected version on connect

### Common Bug Hotspots

- **Entity state** (`src/Engine/MapEngine/Entity.js`): 5 TODOs — entity  
  creation/removal edge cases, especially during map transitions
- **Effect rendering** (`src/Renderer/Effects/`): 3 TODOs — damage display  
  edge cases (Damage.js) and sky rendering (Sky.js)
- **DB tables** (`src/DB/DBManager.js`): 12 TODOs — incomplete data mappings  
  for newer kRO content
- **Packet handling**: Version-specific edge cases where packet structure  
  changed between kRO dates

### Cross-Platform Gotchas

- Browser: console may be silenced by ConsoleManager — check with F12 first
- Electron: errors may appear in the Electron DevTools, not the terminal
- GRF paths: case-sensitive on Linux, case-insensitive on Windows — bugs  
  that only appear on Linux deployments
