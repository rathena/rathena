# Migration Guide: UIComponent → GUIComponent

# L0 = Historical Archive

WARNING

L0 is not operational memory.

L0 contains:

- scars
- historical migrations
- postmortems
- examples
- forensic investigations

Do not use L0 to derive default behavior.
Use L1 first.

## Architecture Overview

### UIComponent (legacy) — `src/UI/UIComponent.js`

- jQuery-based: DOM manipulation, events, CSS, animations all use jQuery
- CSS injected into a global `<style>` tag in `<head>` via `jQuery('style:first').append(...)`
- HTML parsed via `jQuery(htmlText)`, interactive attributes handled by `parseHTML()` using `data-background`, `data-hover`, `data-down`, `data-active`, `data-text`, `data-preload`
- Lives in the Light DOM (direct child of `document.body`)
- `this.ui` is a jQuery object wrapping the root element

### GUIComponent (new) — `src/UI/GUIComponent.js`

- Native DOM + Shadow DOM (`attachShadow({ mode: 'open' })`)
- CSS injected inside the Shadow DOM via a `<style>` element (Common.css + component CSS)
- HTML returned by `render()` method as a string, inserted into `this._container.innerHTML`
- Uses Custom Elements (`<ui-button>`, `<ui-text>`, `<ui-image>`) instead of `data-*` attributes (see docs/CustomElements.md) [OPTIONAL]
- `this.ui` is a jQuery-compatible proxy that exists **only** for interoperability with `UIManager` and legacy `UIComponent` instances — **new code inside a GUIComponent should always use native DOM and Shadow DOM APIs directly**

### DOM Structure

```
document.body
└── div#ComponentName          ← this._host (position: absolute, z-index: 50)
    └── #shadow-root (open)    ← this._shadow
        ├── <style>            ← Common.css + component CSS
        └── div.ui-component-root  ← this._container
            └── <div id="ComponentName">  ← component HTML from render()
                └── ...content...
```

> **Note**: `this._host` receives `position: absolute` and `z-index: 50` via JavaScript during `prepare()`. You do **not** need to declare these in your `:host` CSS.

---

## Lifecycle Hooks

GUIComponent has three lifecycle hooks. Understanding when each runs is critical to avoid bugs like duplicate event bindings.

```
prepare()
  ├── build Shadow DOM, inject CSS, render HTML
  ├── _processAllDataAttrs()
  ├── _createUIProxy()
  ├── appendChild(host) to document.body   ← host IS in the DOM
  ├── init()                               ← runs ONCE
  ├── _setupMouseMode()
  └── host.remove()                        ← host removed from DOM

append()
  ├── appendChild(host) to target          ← host IS in the DOM
  ├── onAppend()                           ← runs EVERY TIME append() is called
  ├── _setupScrollbars()
  ├── _fixPositionOverflow()
  └── focus()

remove()
  ├── onRemove()                           ← runs EVERY TIME remove() is called
  ├── unbind keydown
  ├── dispatch 'x_remove' event
  └── host.remove()                        ← host removed from DOM
```

| Hook         | When it runs                                              | Use for                                                                                  |
| ------------ | --------------------------------------------------------- | ---------------------------------------------------------------------------------------- |
| `init()`     | Once, during `prepare()`. Host is temporarily in the DOM. | One-time setup: `draggable()`, event binding, initial hide (toggle-style only — see §19) |
| `onAppend()` | Every time `append()` is called. Host is in the DOM.      | Position restore, anything that must run each time the component appears                 |
| `onRemove()` | Every time `remove()` is called, before detach.           | Save preferences, cleanup                                                                |

**RULE**: Bind events in `init()` (runs once). Restore position/state in `onAppend()` (runs every time). Save state in `onRemove()`.

---

## Mouse Modes

Each GUIComponent has a `mouseMode` property that controls how mouse events interact with the 3D scene behind the UI:

```javascript
const MouseMode = Object.freeze({
	CROSS: 0, // Mouse crosses the UI and still intersects with the scene
	STOP: 1, // Blocks scene intersection when mouse is over the UI
	FREEZE: 2 // Blocks scene intersection while the UI is alive (modal)
});
```

| Mode     | Use case                                 | Example                         |
| -------- | ---------------------------------------- | ------------------------------- |
| `CROSS`  | Transparent overlays, HUD elements       | Minimap, chat bubbles           |
| `STOP`   | Standard windows (default)               | Clan, Inventory, Equipment      |
| `FREEZE` | Modal dialogs that block all interaction | NPC dialog, confirmation popups |

Set it on the component instance:

```javascript
Clan.mouseMode = GUIComponent.MouseMode.STOP;
```

---

## Comprehensive Migration Checklist

Use this checklist to ensure nothing is missed during a migration. Each item references the detailed section (§) where the full explanation lives.

### Phase 1: Pre-Migration Analysis

Read the original component (JS, CSS, HTML) and answer these questions:

- [ ] **Mouse mode?** — CROSS (transparent overlay), STOP (standard window), FREEZE (modal dialog). See [Mouse Modes](#mouse-modes).
- [ ] **Toggle-style or on-demand?** — Toggle = appended at startup, shown/hidden via `toggle()`. On-demand = appended only when needed (e.g., NPC dialog). On-demand components must NOT hide in `init()` (§24).
- [ ] **Has text inputs (`<input>`, `<select>`, `<textarea>`)?** → Must set `captureKeyEvents = true` and guard `onKeyDown` with `shadowRoot.activeElement` check (§8).
- [ ] **Displays game text with `^rrggbb` color codes?** → Must use `DB.formatMsgToHtml()` + `innerHTML`, not `.textContent` (§13, §30).
- [ ] **Has right-click handling?** → Must add explicit `contextmenu` listener with `preventDefault()` (§26).
- [ ] **Fixed-size or dynamic-size?** → Affects `:host` CSS and inner element positioning strategy (§4a, §4b, §25).
- [ ] **Full-viewport overlay?** → `:host { width: 100%; height: 100% }` (§16).
- [ ] **Has multiple draggable sub-windows?** → `draggable()` only works on `_host`; need custom `_makeDraggable()` helper (§32).
- [ ] **Has toggleable background images (checkboxes, state indicators)?** → Do NOT use `<ui-button>` — use `<button>` or `<div>` instead (§33).
- [ ] **Needs hand cursor on hover?** → Element must match GUIComponent's `CLICKABLE_SELECTOR` list (§34).
- [ ] **FREEZE mode covering full viewport?** → Do NOT use `pointer-events: none` on `:host` (§35).
- [ ] **Uses `<ui-image>` in HTML template?** → Aware of lifecycle race; `attributeChangedCallback` fires before `connectedCallback` (§36).
- [ ] **Full-viewport overlay (CROSS mode)?** → Inner div must use `position: absolute`, not `position: relative` to bypass `.ui-component-root` (§37, §38).
- [ ] **Has tiling background textures (window frames)?** → Do NOT add `background-repeat: no-repeat` to center/body elements (§39).
- [ ] **Mobile-facing component?** → Use `vmin` units for sizes, `%` for positions (§40).
- [ ] **Embeds another UIComponent?** → Must migrate inner component too; UIComponent cannot query inside Shadow DOM (§42).

### Phase 2: Create Files

- [ ] Create `ComponentName.js`, `ComponentName.html`, `ComponentName.css`
- [ ] JS: `new GUIComponent('Name', cssText)`, `render = () => htmlText`
- [ ] JS: `import 'UI/Elements/Elements.js'` if using Custom Elements
- [ ] JS: Set `mouseMode` (CROSS/STOP/FREEZE)

### Phase 3: Convert JS

- [ ] Replace jQuery DOM queries with `this._shadow.querySelector()` / `querySelectorAll()` (§5)
- [ ] Use `ComponentName.getRoot()` / `this.getRoot()` helper for module-level singletons (§5)
- [ ] Replace jQuery event binding with `addEventListener` (§6)
- [ ] Bind events in `init()`, restore state in `onAppend()`, save in `onRemove()` — never bind in `onAppend()`
- [ ] Replace `.text(value)` → `.textContent` (plain text) or `DB.formatMsgToHtml()` + `.innerHTML` (game text with `^rrggbb`) (§13, §30)
- [ ] Replace `.show()`/`.hide()` → `style.display = 'block'` / `'none'` (§1, §10, §18)
- [ ] Replace `$.extend()` → `Object.assign()` / spread operator
- [ ] Replace `jQuery.escape()` → local `_escapeHTML()` helper
- [ ] Replace `return false` in event handlers → explicit `stopImmediatePropagation()` + `preventDefault()` (§12)
- [ ] Remove `event.isTrigger` checks (§21)
- [ ] Keep `function()` (not arrow) for callbacks relying on dynamic `this` via `.call()`/`.apply()` (§9)
- [ ] Null-guard `querySelector` results for dynamic indices from server packets (§31)
- [ ] Use `querySelectorAll` + `forEach` when selector may match multiple elements (§28)
- [ ] Call `element._roScrollbarRestart()` after programmatic `scrollTop` changes (§29)
- [ ] Add `contextmenu` listener with `e.preventDefault()` on right-click areas (§26)
- [ ] Use `getBoundingClientRect()` relative to root for overlay/tooltip positioning (§27)
- [ ] For elements moved outside shadow root: apply inline styles BEFORE `remove()` (§19)
- [ ] Guard `onKeyDown` with `shadowRoot.activeElement` check if component has text inputs (§8)
- [ ] Use capture-phase listeners to replace jQuery event queue reordering (§12)

### Phase 4: Convert HTML

- [ ] Replace `data-background`/`data-hover`/`data-down` → `<ui-button bg="..." hover="..." down="...">` (optional — `data-*` still works)
- [ ] Replace `data-text` → `<ui-text msg="...">` (optional)
- [ ] Replace `data-background` (non-button) → `<ui-image src="...">` (optional)
- [ ] For clickable elements needing hand cursor: use `<button>`, `<ui-button>`, `<a>`, `<input>`, `<label>`, or `.item-link`/`.draggable` class — plain `<div>` is NOT recognized by cursor system (§34)
- [ ] Do NOT use `<ui-button>` for elements whose `backgroundImage` changes at runtime (checkboxes, toggles) — use `<button>` instead (§33)

### Phase 5: Convert CSS

- [ ] Move `top`/`left`/`width`/`height` to `:host` (§4a)
- [ ] Do NOT set `position: absolute` or `z-index` on `:host` (set by JS in `prepare()`)
- [ ] Inner element: `position: absolute` (fixed-size with `:host` dimensions) or `position: relative` (auto-size without `:host` dimensions) (§4a, §25)
- [ ] Dynamic-size components: no `position: absolute` on inner div (§4b)
- [ ] FREEZE mode full-viewport: `:host { width: 100%; height: 100% }` WITHOUT `pointer-events: none` (§35)
- [ ] CROSS mode overlay: `:host { pointer-events: none }` + `pointer-events: auto` on interactive children (§11)
- [ ] Remove `body {}` rules — apply via JS in `onAppend()` (§15)
- [ ] Use explicit `display: 'block'`/`'none'` when CSS declares `display: none` on toggled elements (§10)
- [ ] Add `height: 100%` to inner element if `:host` has `overflow: hidden` with `bottom`-anchored children (§23)
- [ ] Escape CSS selectors starting with digit: `.3d` → `.\\33 d` (§14)
- [ ] Avoid duplicate dimensions on `:host` and inner element — causes scrollbars (§20)
- [ ] Do NOT add `background-repeat: no-repeat` to tiling texture elements (window frame centers) (§39)
- [ ] Use `vmin` for sizes on mobile-facing components, not fixed `px` (§40)

### Phase 6: Post-Migration Verification

- [ ] ESLint: 0 errors
- [ ] Prettier: all files pass
- [ ] Visual: no unexpected scrollbars (§20)
- [ ] Visual: sprite cursor shows hand on clickable elements (§34)
- [ ] Visual: toggle/checkbox elements maintain visual state after click (§33)
- [ ] Functional: FREEZE mode blocks all clicks to game world (§35)
- [ ] Functional: text with `^rrggbb` renders as colored spans
- [ ] Functional: keyboard input works in text fields (not stolen by global handlers)
- [ ] Functional: drag-and-drop works (sub-windows, items)
- [ ] Functional: right-click does NOT show browser context menu (§26)
- [ ] Functional: ESC / Enter / arrow keys work as expected
- [ ] Functional: scrollbars sync with mouse wheel (§29)

### Pitfall Classification

Not all pitfalls require action from the migrator. Some are already handled by the GUIComponent framework.

**Automatic (no action needed):**

- §2: Global CSS penetration → Common.css `:host-context` rule
- §3: Scrollbar CSS injection → `applyDOMScrollbar()` auto-detects shadow
- §4 (closest body): `isConnected` fix in Scrollbar.js
- §5 (mouse events): `_setupShadowCursorEvents()` handles retargeting
- §6 (offsetParent): `ui` proxy mimics jQuery behavior
- §7 (css proxy): Both proxy syntaxes work

**Manual (you must handle):**

- §1: jQuery `.show()` → explicit display values
- §8: `captureKeyEvents` + `onKeyDown` guard for text inputs
- §9: Keep `function()` for dynamic `this` callbacks
- §10: CSS `display: none` fallback trap
- §11: `pointer-events: none` inheritance from `:host`
- §12: Capture-phase events replacing jQuery queue reordering
- §13/§30: RO text `^rrggbb` color codes via `DB.formatMsgToHtml()`
- §14: CSS selectors starting with digit
- §15: Body styles in shadow CSS
- §16: Full-viewport `:host` sizing
- §17: Asset path constants → use `DB.INTERFACE_PATH`
- §18: jQuery `.show()` on child elements
- §19: Elements moved outside shadow lose CSS
- §20: Duplicate `:host` / inner dimensions causing scrollbars
- §21: Remove `event.isTrigger` checks
- §22: `getComputedStyle()` for visible element detection
- §23: Inner element `height: 100%` for `bottom`-anchored children
- §24: Don't hide on-demand components in `init()`
- §25: `position: relative` for auto-sizing inner root
- §26: Explicit `contextmenu` prevention
- §27: `getBoundingClientRect()` for overlay positioning
- §28: `querySelectorAll` for multi-match updates
- §29: Scrollbar sync after programmatic `scrollTop`
- §31: Null-guard `querySelector` for server packet indices
- §32: Sub-window dragging via custom `_makeDraggable()` helper
- §33: `<ui-button>` overrides `backgroundImage` — don't use for toggleable elements
- §34: Sprite cursor `CLICKABLE_SELECTOR` — use recognized elements for hand cursor
- §35: FREEZE mode + `pointer-events: none` conflict on `:host`
- §36: `<ui-image>` lifecycle race (already fixed in UIImage.js, but aware when creating new Custom Elements)
- §37: `.ui-component-root` has no dimensions — full-viewport inner div must use `position: absolute`
- §38: CROSS-mode selective `pointer-events` pattern
- §39: `background-repeat` defaults for tiling window frame textures
- §40: Use `vmin` units for mobile-responsive sizing
- §41: `touch-action: manipulation` must target `:host` (already applied in Common.css)
- §42: UIComponent cannot be embedded inside GUIComponent Shadow DOM

---

## Step-by-Step Migration Checklist

### 1. Create the component files

Each GUIComponent needs three files:

- `ComponentName.js` — Logic
- `ComponentName.html` — Template (raw HTML string)
- `ComponentName.css` — Styles

### 2. Convert the JS file

**Before (UIComponent):**

```javascript
import UIComponent from 'UI/UIComponent.js';
import UIManager from 'UI/UIManager.js';
import htmlText from './Clan.html?raw';
import cssText from './Clan.css?raw';

const Clan = new UIComponent('Clan', htmlText, cssText);
```

**After (GUIComponent):**

```javascript
import GUIComponent from 'UI/GUIComponent.js';
import UIManager from 'UI/UIManager.js';
import 'UI/Elements/Elements.js'; // ← REQUIRED: registers <ui-button>, <ui-text>, <ui-image>
import htmlText from './Clan.html?raw';
import cssText from './Clan.css?raw';

const Clan = new GUIComponent('Clan', cssText); // ← only CSS, not HTML

Clan.render = () => htmlText; // ← HTML goes here
```

Key differences:

- `GUIComponent` constructor takes `(name, cssText)` — no HTML argument
- HTML is returned by `render()` method
- Must `import 'UI/Elements/Elements.js'` to register custom elements [OPTIONAL]

#### Components with `null` HTML (dynamic DOM)

Some legacy UIComponents pass `null` for HTML and build their root element dynamically in `init()`:

```javascript
// Before (UIComponent) — no HTML template, root created at runtime
const StatusIcons = new UIComponent('StatusIcons', null, cssText);

StatusIcons.init = function init() {
	this.ui = jQuery('<div/>').attr('id', 'StatusIcons');
	// ... dynamically create child elements later
};
```

For GUIComponent, you still need a minimal HTML template — the `render()` method must return something. Create a `.html` file with a minimal container:

```html
<!-- StatusIcons.html -->
<div id="StatusIcons"></div>
```

```javascript
// After (GUIComponent)
const StatusIcons = new GUIComponent('StatusIcons', cssText);
StatusIcons.render = () => htmlText; // ← returns the minimal <div> wrapper
// Dynamic child elements are appended to this container at runtime
```

### 3. Convert the HTML file [OPTIONAL]

Replace `data-*` attributes with Custom Elements:

| UIComponent (data-\*)                                                                      | GUIComponent (Custom Element)                                          |
| ------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------- |
| `<button data-background="btn_ok.bmp" data-hover="btn_ok_a.bmp" data-down="btn_ok_b.bmp">` | `<ui-button bg="btn_ok.bmp" hover="btn_ok_a.bmp" down="btn_ok_b.bmp">` |
| `<span data-text="2355">Fallback</span>`                                                   | `<ui-text msg="2355">Fallback</ui-text>`                               |
| `<div data-background="image.bmp">`                                                        | `<ui-image src="image.bmp">`                                           |

Elements that still use `data-background`, `data-hover`, `data-down`, `data-active`, `data-text`, or `data-preload` will be processed by `GUIComponent._processAllDataAttrs()` during `prepare()`. Both approaches work; Custom Elements are preferred for new code but is optional. Create new custom elements if conversion demands it (see docs/CustomElements.md).

### 4. Convert the CSS file — CRITICAL

This is where most bugs occur. The CSS must be restructured for Shadow DOM.

#### 4a. Move dimensions and position to `:host`

The host element (`this._host`) is what the outside world sees. Its dimensions determine:

- `getBoundingClientRect()` used by magnetic snap
- `_fixPositionOverflow()` used by screen boundary checks
- `UIManager.fixResizeOverflow()` used by window resize

**RULE**: The `:host` selector MUST define `width`, `height`, `top`, and `left`. The inner root element (`#ComponentName`) MUST NOT have `top` or `left` (set them to 0 or omit them).

**Before:**

```css
#Clan {
	position: absolute;
	top: 150px;
	left: 150px;
	width: 400px;
	height: 317px;
}
```

**After:**

```css
:host {
	width: 400px;
	height: 317px;
	top: 150px;
	left: 150px;
}

#Clan {
	position: absolute;
	width: 400px;
	height: 317px;
}
```

> **Note**: You do NOT need `position: absolute` or `z-index` on `:host` — these are set via JavaScript in `prepare()`.

**WHY**: If `top`/`left` remain on the inner element, it offsets the content inside the host. The host's `getBoundingClientRect()` returns the host's position, not the inner element's visual position. This causes magnetic snap to align to wrong edges (especially bottom/right).

**WHY `position: absolute` on inner element**: The inner `#Clan` div needs `position: absolute` (or `relative`) to serve as a containing block for its children that use `position: absolute`. Without it, absolutely-positioned children would escape to the next positioned ancestor.

#### 4b. Dynamic-size components (no fixed width/height)

Some components (e.g., StatusIcons) have no fixed dimensions — they grow dynamically as children are added. In the original UIComponent, the single `#ComponentName` div was both the positioned element AND the container. In GUIComponent, `_host` is the positioned element and the inner `#ComponentName` div from `render()` is a separate element inside the shadow DOM.

**RULE for dynamic-size components**: Do NOT add `position: absolute` to the inner `#ComponentName` div. This creates an extra 0×0 containing block inside the shadow DOM that breaks layout for absolutely-positioned children. Let `.state` (or similar child) elements use `_host` as their containing block instead.

**Fixed-size components** (Clan, Inventory, etc.):

```css
:host {
	width: 400px;
	height: 317px;
	top: 150px;
	left: 150px;
}

#Clan {
	position: absolute; /* ← NEEDED: serves as containing block for children */
	width: 400px;
	height: 317px;
}
```

**Dynamic-size components** (StatusIcons, etc.):

```css
:host {
	top: 166px;
	right: 20px;
	overflow: visible; /* ← ensures children aren't clipped beyond 0×0 host bounds */
}

#StatusIcons {
	display: block; /* ← NO position: absolute — children position relative to _host */
}
```

**WHY**: In the original UIComponent, `position: absolute` on the root div was necessary because it was the top-level element appended to `document.body`. In GUIComponent, `_host` already has `position: absolute` (set by `prepare()`). Adding it again on the inner div creates a redundant 0×0 positioned container inside the shadow DOM. For fixed-size components this is harmless (both have the same explicit dimensions), but for dynamic-size components it breaks the containing block chain — absolutely-positioned children end up positioned relative to a 0×0 box instead of the properly-positioned `_host`.

### 5. Convert DOM queries

**Before (jQuery):**

```javascript
this.ui.find('.content.info .name .value').text(clan.name);
this.ui.find('.close').click(function() { ... });
```

**After (native DOM):**

```javascript
const root = ComponentName.getRoot();
root.querySelector('.content.info .name .value').textContent = clan.name;

const closeBtn = root.querySelector('.close');
closeBtn.addEventListener('click', () => { ... });
```

**RULE**: Always query from `ComponentName.getRoot()` / `this.getRoot()`, never from `document`. Elements inside Shadow DOM are invisible to `document.querySelector()`.

#### Helper: `ComponentName.getRoot()` / `this.getRoot()` for module-level components

For components defined as module-level singletons (not using `this` inside methods) use `ComponentName.getRoot()` / `this.getRoot()` helper to avoid repeating the shadow root lookup:

```javascript
// Usage in any function:
function resetPositions() {
	const root = ComponentName.getRoot();
	const elements = root.querySelectorAll('.state');
	// ...
}
```

This is preferred over `this._shadow || this._host` when the component's methods are plain functions (not on `this`), which is common for overlay/HUD components like StatusIcons, MiniMap, etc.

### 6. Convert event handlers

**Before:**

```javascript
Guild.init = function init() {
	this.ui.find('.close').mousedown(stopPropagation).click(Guild.toggle.bind(this));
	this.draggable(this.ui.find('.titlebar'));
	this.ui.hide();
};
```

**After:**

```javascript
Clan.init = function init() {
	this.draggable('.titlebar');
	const root = Clan.getRoot();
	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		closeBtn.addEventListener('click', () => Clan.toggle());
	}
	this._host.style.display = 'none';
};
```

Key changes:

- `draggable()` accepts a CSS selector string (resolved inside shadow)
- Position restore goes in `onAppend()`
- Uses native DOM `addEventListener` instead of jQuery `.click()`/`.mousedown()`

### 7. The `this.ui` proxy — interop only

`this.ui` is a jQuery-compatible proxy that exists so `UIManager` and legacy `UIComponent` code can interact with GUIComponents without changes. **New code inside a GUIComponent should always use native DOM and Shadow DOM APIs.**

| Proxy method                           | Native DOM equivalent (preferred)                                                |
| -------------------------------------- | -------------------------------------------------------------------------------- |
| `this.ui.css('top', '100px')`          | `this._host.style.top = '100px'`                                                 |
| `this.ui.css({ top: 100, left: 200 })` | `Object.assign(this._host.style, { top: '100px', left: '200px' })`               |
| `this.ui.show()`                       | `this._host.style.display = ''` (+ call `this._fixPositionOverflow()` if needed) |
| `this.ui.hide()`                       | `this._host.style.display = 'none'`                                              |
| `this.ui.is(':visible')`               | `this._host.style.display !== 'none'`                                            |
| `this.ui.find('.foo')`                 | `this._shadow.querySelector('.foo')` or `this._shadow.querySelectorAll('.foo')`  |
| `this.ui.width()`                      | `this._host.getBoundingClientRect().width`                                       |
| `this.ui.height()`                     | `this._host.getBoundingClientRect().height`                                      |
| `this.ui.offset()`                     | `this._host.getBoundingClientRect()`                                             |
| `this.ui.position()`                   | `{ left: this._host.offsetLeft, top: this._host.offsetTop }`                     |
| `this.ui.parent()`                     | `this._host.parentNode`                                                          |
| `this.ui.trigger('event')`             | `this._host.dispatchEvent(new CustomEvent('event', { bubbles: true }))`          |
| `this.ui.detach()`                     | `this._host.remove()`                                                            |
| `this.ui.appendTo(target)`             | `target.appendChild(this._host)`                                                 |

> **Caveat**: `this.ui.show()` automatically calls `this._fixPositionOverflow()` to ensure the component stays within screen bounds. When using native DOM `this._host.style.display = ''`, call `this._fixPositionOverflow()` manually if the component may be near screen edges.

**When to use `this.ui`**: Only when passing the component to external code that expects a jQuery-like interface (e.g., `UIManager` internals, legacy components calling methods on other components).

**When to use native DOM**: Always, inside the component's own code (`init`, `onAppend`, `onRemove`, event handlers, etc.).

### 8. Convert preferences (save/restore position)

**Before:**

```javascript
Guild.onRemove = function onRemove() {
	_preferences.x = parseInt(this.ui.css('left'), 10);
	_preferences.y = parseInt(this.ui.css('top'), 10);
	_preferences.save();
};
```

**After:**

```javascript
Guild.onRemove = function onRemove() {
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.save();
};
```

**NEVER use for preferences saving:**

```javascript
// BAD — returns 0,0 when hidden
const rect = this._host.getBoundingClientRect();
_preferences.x = Math.round(rect.left);
```

```javascript
// BAD — position 0 becomes 100
_preferences.x = parseInt(this._host.style.left, 10) || 100;
```

```javascript
// BAD — proxy
_preferences.x = parseInt(this.ui.css('left'), 10);
```

### 9. Register with UIManager

Same as before:

```javascript
export default UIManager.addComponent(Clan);
```

`UIManager.addComponent()` accepts both `UIComponent` and `GUIComponent` instances.

---

## Additional Features

### Clone

Deep-clone a GUIComponent instance:

```javascript
const clone = Clan.clone();
```

---

## Known Shadow DOM Pitfalls (already fixed)

These issues were encountered during the first migration (Clan) and are documented here to explain the fixes and prevent regressions.

### 1. jQuery `.show()` sets `display: block`, breaking flex/grid layouts

**Bug**: jQuery's `.show()` sets `element.style.display = 'block'` regardless of the element's CSS `display` value. For elements that use `display: flex` (like the scrollbar), this breaks the layout.

**Fix applied in `src/UI/Scrollbar.js`** (commit `7ce87bf9`):

```javascript
// WRONG
$scrollbar.hide();
$scrollbar.show();

// CORRECT
$scrollbar[0].style.display = 'none';
$scrollbar[0].style.display = ''; // ← resets to CSS value (flex)
```

**RULE**: Never use jQuery `.show()`/`.hide()` on elements that may have non-block display. Use `element.style.display = ''` (empty string resets to the CSS-declared value) and `element.style.display = 'none'`.

### 2. Global CSS does not penetrate Shadow DOM

**Bug**: CSS rules in the global `<style>` tag (e.g., `.custom-cursor * { cursor: none !important; }` from CursorManager) do not affect elements inside Shadow DOM.

**Fix applied in `src/UI/Common.css`** (commit `f4183351`):

```css
:host-context(.custom-cursor) * {
	cursor: none !important;
}
```

This rule is inside `Common.css` which is injected into every Shadow DOM. `:host-context(.custom-cursor)` checks if any ancestor of the shadow host has the class `custom-cursor` (set on `document.body` by CursorManager).

**RULE**: Any global CSS that needs to affect Shadow DOM content must be added to `Common.css`.

### 3. Scrollbar CSS injection into Shadow DOM

**Bug**: The scrollbar CSS (`.ro-custom-scrollbar` styles) is injected into the global `<style>` tag by `Scrollbar.js setupStyles()`. These styles don't reach elements inside Shadow DOM.

**Fix applied in `src/UI/Scrollbar.js`** (commit `b9d21673`):

```javascript
const shadowRoot = element.getRootNode();
if (shadowRoot instanceof ShadowRoot && !shadowRoot.querySelector('style[data-scrollbar]')) {
	const scrollbarStyle = document.createElement('style');
	scrollbarStyle.setAttribute('data-scrollbar', '');
	scrollbarStyle.textContent = ScrollBar._cssText;
	shadowRoot.appendChild(scrollbarStyle);
}
```

`ScrollBar.applyDOMScrollbar()` now auto-detects if the target element is inside a Shadow DOM and injects the scrollbar CSS into that shadow root. No action needed from the component author.

### 4. `$element.closest('body')` fails inside Shadow DOM

**Bug**: jQuery's `.closest('body')` traverses up the DOM tree but stops at the shadow boundary. For elements inside Shadow DOM, it always returns an empty set, causing the scrollbar update poller to stop immediately.

**Fix applied in `src/UI/Scrollbar.js`** (commit `7ce87bf9`):

```javascript
// WRONG
if (!$element.closest('body').length) { ... }

// CORRECT
if (!$element[0].isConnected) { ... }
```

`Element.isConnected` is a native property that returns `true` if the element is connected to the document, including through Shadow DOM.

**RULE**: Never use jQuery `.closest('body')` or `jQuery.contains(document, element)` to check if an element is in the DOM. Use `element.isConnected` instead.

### 5. Mouse events are retargeted across Shadow DOM boundaries

**Bug**: When a `mouseover` event fires on an element inside Shadow DOM and bubbles to `document.body`, `e.target` is retargeted to the shadow host element. The CursorManager's `findClickableTarget(e.target)` never finds clickable elements (like `.ui-custom-scrollbar`, `ui-button`) inside the shadow.

**Fix applied in `src/UI/GUIComponent.js`** (commit `f4183351`):

```javascript
_setupShadowCursorEvents() {
    const container = this._container;
    // Register mouseover/mouseout/mousedown/mouseup directly on the container
    // INSIDE the shadow DOM, where e.target is the real element (no retargeting)
    container.addEventListener('mouseover', e => {
        if (e.target.closest && e.target.closest(CLICKABLE_SELECTOR)) {
            // Change cursor to hand
        }
    });
    // ... mouseout, mousedown, mouseup handlers
}
```

This method is called automatically by `_setupMouseMode()`. No action needed from the component author.

**RULE**: Event listeners that need to identify specific elements inside Shadow DOM must be registered INSIDE the shadow tree (on `this._container` or `this._shadow`), not on `document.body`.

### 6. `offsetParent()` mismatch between jQuery and native DOM

**Bug**: Legacy UIComponent's `draggable()` builds a snap cache and skips components whose `offsetParent` differs from the dragged component's `offsetParent`. jQuery's `.offsetParent()` walks up the tree and returns `document.documentElement` for elements whose parent chain is all `position: static`. Native `element.offsetParent` returns `document.body`. This mismatch causes legacy UIs to skip GUIComponent instances when building the snap cache.

**Fix applied in `src/UI/GUIComponent.js`** (commit `1813e446`):

```javascript
offsetParent() {
    let el = host.offsetParent || document.documentElement;
    while (el && el !== document.documentElement && window.getComputedStyle(el).position === 'static') {
        el = el.offsetParent;
    }
    el = el || document.documentElement;
    return { length: el ? 1 : 0, 0: el };
},
```

This mimics jQuery's `.offsetParent()` behavior. No action needed from the component author.

### ~~7. `this.ui.css()` object syntax~~ (FIXED)

Previously broken, now fixed. Both proxy syntaxes work:

```javascript
this.ui.css({ top: 100, left: 200 });
this.ui.css('top', 100);
```

Prefer native DOM:

```javascript
this._host.style.top = '100px';
this._host.style.left = '200px';
```

### ~~8. Keyboard input stolen from `<input>` / `<textarea>` inside Shadow DOM~~ (FIXED)

**Bug**: When a GUIComponent contains `<input>`, `<select>`, or `<textarea>` elements inside its Shadow DOM, users cannot type in them — keystrokes are intercepted by other global `keydown` handlers (ChatBox battle mode, shortcut system, etc.).

**Root cause**: Shadow DOM encapsulates focus. When an `<input>` inside a shadow root is focused:

- `document.activeElement` returns the **shadow host** (`<div>`) — not the actual `<input>`
- `event.target` on bubbled events is **retargeted** to the shadow host

Other global handlers (e.g., ChatBox line 816–826, line 928–941) check `document.activeElement.tagName` or `event.target.tagName` to detect focused inputs. They see `DIV` instead of `INPUT`, so they consume the keystroke instead of letting it through.

```
document.activeElement          → <div#ChatRoomCreate>  (host)
shadowRoot.activeElement        → <input name="title">  (real element)
event.target                    → <div#ChatRoomCreate>  (retargeted)
event.composedPath()[0]         → <input name="title">  (real element)
```

**Fix — two parts:**

**Part 1: Register `onKeyDown` in the capture phase**

The component's `onKeyDown` handler must run **before** all other `window.keydown` handlers. Set `captureKeyEvents = true` on the component instance. This makes `_bindKeyDown()` register the handler with `useCapture = true`, so it fires during the capture phase (before the bubble phase where other handlers listen).

```javascript
// In _bindKeyDown() — GUIComponent.js
_bindKeyDown() {
    if (!this.onKeyDown) return;
    this._unbindKeyDown();
    const handler = this.onKeyDown.bind(this);
    this._keyHandler = event => {
        if (handler(event) === false) {
            event.preventDefault();
        }
    };
    var useCapture = !!this.captureKeyEvents;
    window.addEventListener('keydown', this._keyHandler, useCapture);
}
```

**Part 2: Guard the `onKeyDown` handler**

Inside the component's `onKeyDown`, check if an input element inside the shadow is focused. If so, call `stopImmediatePropagation()` to block all other handlers, and return `true` (NOT `false`) so the wrapper does not call `preventDefault()` — the browser must execute the default action (typing the character).

Use `shadowRoot.activeElement` (not `document.activeElement`) to get the real focused element inside the shadow.

```javascript
ChatRoomCreate.onKeyDown = function onKeyDown(event) {
	var shadow = this._shadow || this._host;
	var focused = shadow.activeElement;

	// If an input inside the shadow is focused, let the browser handle the keystroke
	if (ChatRoomCreate.isEditableFocused()) {
		// Still handle Enter/Escape for form submission/close
		if (event.which === KEYS.ENTER) {
			submitForm.call(this);
			event.stopImmediatePropagation();
			return false;
		}
		if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
			this.hide();
			event.stopImmediatePropagation();
			return false;
		}
		// Block other handlers from consuming the keystroke, but let the browser type it
		event.stopImmediatePropagation();
		return true; // ← CRITICAL: true, not false
	}

	// Normal key handling when no input is focused
	// ...
	return true;
};
```

**Component setup:**

```javascript
ChatRoomCreate.captureKeyEvents = true; // ← capture phase
```

**Why `return true` is critical**: The `_bindKeyDown` wrapper calls `event.preventDefault()` when the handler returns `false`. For text inputs, `preventDefault()` on `keydown` blocks the character from being inserted. Returning `true` (or any non-`false` value) skips `preventDefault()`, allowing normal typing.

**Why only some keys worked without the fix**: Keys like `z`, `x`, `c`, `v`, `b`, `n`, `m` are not mapped to any shortcut in the default `ShortCutControls.js`. All other keys (`a`–`y`, `0`–`9`, `F1`–`F12`) are consumed by the ChatBox battle mode handler (line 928–941) or the shortcut system before reaching the input.

**RULE**: Any GUIComponent with `<input>`, `<select>`, or `<textarea>` inside its Shadow DOM **must** set `captureKeyEvents = true` and guard its `onKeyDown` with a `shadowRoot.activeElement` check. Without this, users will not be able to type in those fields.

**Affected components**: ChatRoomCreate, ChatRoom, and any future GUIComponent with text input fields.

And also update `_unbindKeyDown()` in `src/UI/GUIComponent.js` (line 424-430) — it already removes both normal and capture listeners, which is correct. But `_bindKeyDown()` (line 412-422) needs to be updated to check `this.captureKeyEvents`:

```javascript
_bindKeyDown() {
    if (!this.onKeyDown) return;
    this._unbindKeyDown();
    const handler = this.onKeyDown.bind(this);
    this._keyHandler = event => {
        if (handler(event) === false) {
            event.preventDefault();
        }
    };
    var useCapture = !!this.captureKeyEvents;
    window.addEventListener('keydown', this._keyHandler, useCapture);
}
```

### 9. Arrow functions break `this`-dependent callbacks (e.g., `Texture.load`)

When converting callbacks from `function()` to arrow functions, check whether the callback relies on dynamic `this` binding via `.call()` or `.apply()`.

**Example**: `Texture.load()` in `src/Utils/Texture.js` calls `oncomplete.apply(canvas, args)`, setting `this` to the loaded canvas element. If you convert the callback to an arrow function, `this` captures the outer lexical scope instead of the canvas.

```javascript
// WRONG — arrow function ignores .apply(canvas), `this` is outer scope
Client.loadFile(path, data => {
	Texture.load(data, () => {
		addResizedStatusIcon(this, index); // `this` is NOT the canvas!
	});
});

// CORRECT — regular function receives `this` = canvas from .apply()
Client.loadFile(path, data => {
	Texture.load(data, function () {
		addResizedStatusIcon(this, index); // `this` is the canvas ✓
	});
});
```

**RULE**: Keep callbacks as regular `function()` when the caller uses `.call()`, `.apply()`, or sets `this` dynamically. This pattern appears in `Texture.load`, `Client.loadFile` completions, and some event handlers. The outer callback (e.g., `Client.loadFile`'s callback) can safely be an arrow function if it doesn't use `this`.

### 10. CSS `display: none` fallback trap when toggling visibility

**Bug**: Elements inside Shadow DOM styled with `display: none` in CSS cannot be shown by setting `element.style.display = ''`. The empty string removes the inline style, causing the element to fall back to the CSS-declared `display: none` — so it stays hidden.

This is different from Light DOM components where jQuery's `.show()` / `.hide()` set explicit inline values. In Shadow DOM with scoped CSS, the CSS `display: none` always wins when the inline style is cleared.

```css
/* WRONG — CSS declares display: none, inline '' just removes the override */
.skill-level {
	position: fixed;
	display: none; /* ← elements start hidden */
}
```

```javascript
// WRONG — '' removes inline style, falls back to CSS display: none
element.style.display = ''; // Still hidden!

// CORRECT — use explicit 'block' to override CSS
element.style.display = 'block'; // Visible ✓
element.style.display = 'none'; // Hidden ✓
```

**Better approach**: Don't use CSS `display: none` for elements whose visibility is toggled at runtime. Set the initial `display: none` via inline style in `init()`, then toggle with explicit values:

```css
/* CORRECT — no display in CSS */
.skill-level {
	position: fixed;
}
```

```javascript
// init() — hide initially via inline style
element.style.display = 'none';

// onAppend() — show
element.style.display = 'block';

// onRemove() — hide
element.style.display = 'none';
```

**RULE**: For elements toggled at runtime, manage `display` entirely through inline styles. Either use explicit values (`'block'`/`'none'`) or remove `display: none` from CSS and set the initial state in `init()`.

### 11. `pointer-events: none` on `:host` is inherited by Shadow DOM children

**Bug**: When `:host` has `pointer-events: none` (common for overlay/HUD components in CROSS mouse mode), all children inside the Shadow DOM inherit `pointer-events: none`. Unlike most CSS encapsulation in Shadow DOM, **inheritable CSS properties DO cross the shadow boundary** from host to children.

This means children (e.g., canvas elements, interactive overlays) silently become invisible to mouse events. They won't receive `click`, `mousedown`, `mouseover`, etc.

```css
/* Host is transparent to mouse — correct for overlays */
:host {
	pointer-events: none;
}

/* WRONG — canvas inherits pointer-events: none from :host */
.skill-level {
	position: fixed;
	/* no pointer-events override → inherits none → invisible to mouse */
}

/* CORRECT — explicitly opt children back in */
.skill-level {
	position: fixed;
	pointer-events: auto; /* ← overrides inherited none */
}
```

**When to use `pointer-events: auto`**: Add it to any child element that needs to participate in mouse event hit testing. For purely visual elements (like the skill level number following the cursor), `pointer-events: auto` is optional since event listeners on `window` still fire regardless. But if any system relies on `document.elementFromPoint()` or if the element itself has event listeners, it needs `pointer-events: auto`.

**Other inheritable properties that cross Shadow DOM**: `color`, `font-*`, `visibility`, `cursor`, `direction`, `text-align`, `line-height`, `letter-spacing`, `word-spacing`, `white-space`, `user-select`. Check for unintended inheritance when the host sets any of these.

**RULE**: When `:host` sets `pointer-events: none`, add `pointer-events: auto` to any child that needs mouse interaction. Review other inheritable properties that might leak from host to shadow children.

### 12. jQuery event priority (`events.unshift`) → capture phase + explicit propagation control

**Bug**: Some legacy UIComponents manipulate jQuery's internal event queue to ensure their handlers fire first:

```javascript
// BEFORE (UIComponent) — move handler to front of jQuery's internal queue
jQuery(window).one('mousedown.targetselection', intersectEntities);
events = jQuery._data(window, 'events').mousedown;
events.unshift(events.pop());
```

This pattern has no native DOM equivalent. `addEventListener` order is determined by registration order, and you cannot reorder handlers.

**Fix**: Use the **capture phase** (`addEventListener(event, handler, true)`) to guarantee the handler fires before all bubble-phase handlers. Capture phase runs top-down (window → document → ... → target) before the bubble phase runs bottom-up (target → ... → window). A capture handler on `window` fires before ANY other handler anywhere in the document.

```javascript
// AFTER (GUIComponent) — capture phase fires before all bubble handlers
_mousedownHandler = event => {
	intersectEntities(event);
};
window.addEventListener('mousedown', _mousedownHandler, true);

// Cleanup must also specify capture: true
window.removeEventListener('mousedown', _mousedownHandler, true);
```

**Also convert jQuery's `return false`**: In jQuery, returning `false` from an event handler calls both `event.stopPropagation()` and `event.preventDefault()`. In native DOM, the return value of an `addEventListener` callback is **ignored**. You must call these methods explicitly:

```javascript
// BEFORE (jQuery handler) — return false = stopPropagation + preventDefault
function intersectEntities(event) {
	event.stopImmediatePropagation();
	// ... process ...
	return false; // ← jQuery calls stopPropagation + preventDefault
}

// AFTER (native handler) — must be explicit
function intersectEntities(event) {
	event.stopImmediatePropagation();
	event.preventDefault(); // ← must call explicitly, return value is ignored
	// ... process ...
}
```

**RULE**: Replace jQuery event queue manipulation (`events.unshift(events.pop())`) with capture-phase listeners. Replace `return false` with explicit `stopImmediatePropagation()` + `preventDefault()` calls.

### 13. jQuery `.text()` override is lost — RO-style `^rrggbb` color codes rendered as literal text

**Bug**: The legacy codebase overrides jQuery's `.text()` method in `Utils/jquery.js` to add RO-specific text processing. When a UIComponent calls `this.ui.find('.content').text(value)`, it is NOT using standard jQuery `.text()` — the override:

1. **Sanitizes HTML** (whitelist: `<font>`, `<i>`, `<b>` — all other tags stripped)
2. **Converts `^rrggbb` color codes** to `<span style="color:#rrggbb">`
3. **Substitutes `^nItemID^NNN`** with the item's display name from DB
4. **Converts `\n` to `<br/>`**
5. Sets the result via `.html()` (innerHTML), not `.textContent`

When migrating to GUIComponent, the natural replacement for `.text()` is `.textContent`. This strips ALL processing — color codes appear as literal `^FF0000` text, newlines are ignored, and HTML tags are escaped.

**Affected components**: Any component that displays user-facing game text with `^rrggbb` color codes. Common examples: SkillDescription, NpcBox, Quest, ItemInfo, and any component using `DB.getSkillDescription()`, `DB.getMessage()`, or similar DB text that may contain color codes.

**Fix**: Replicate the jQuery `.text()` override logic natively. Create a local `_formatROText()` helper:

```javascript
const _allowedTags = new Set(['font', 'i', 'b']);

function _formatROText(value) {
	const tmp = document.createElement('div');
	tmp.innerHTML = String(value);

	tmp.querySelectorAll('*').forEach(el => {
		if (!_allowedTags.has(el.tagName.toLowerCase())) {
			el.replaceWith(...el.childNodes);
		}
	});

	let txt = tmp.innerHTML;

	let result;
	const colorReg = /\^([a-fA-F0-9]{6})/;
	while ((result = colorReg.exec(txt))) {
		txt = txt.replace(result[0], `<span style="color:#${result[1]}">`) + '</span>';
	}

	const itemReg = /\^nItemID\^(\d+)/g;
	while ((result = itemReg.exec(txt))) {
		txt = txt.replace(result[0], DB.getItemInfo(result[1]).identifiedDisplayName);
	}

	txt = txt.replace(/\n/g, '<br/>');

	return txt;
}
```

Then use `.innerHTML` instead of `.textContent`:

```javascript
// WRONG — loses color codes, newlines, item substitution
content.textContent = DB.getSkillDescription(id);

// CORRECT — preserves RO text formatting
content.innerHTML = _formatROText(DB.getSkillDescription(id));
```

**How to detect**: Search for `.text(` calls in the legacy component. If the argument could contain `^rrggbb` codes (skill descriptions, NPC dialog, item info, quest text), you need `_formatROText()` + `.innerHTML`. If the argument is always plain text (e.g., a numeric value or a simple label), `.textContent` is fine.

**RULE**: When replacing jQuery `.text(value)` with native DOM, check whether the value may contain RO-style formatting (`^rrggbb`, `^nItemID`, newlines). If it does, use `_formatROText()` + `.innerHTML`. If it's plain text, use `.textContent`.

### 14. CSS selectors starting with a digit are invalid

**Bug**: `element.closest('.3d')` throws `SyntaxError: '.3d' is not a valid selector`. CSS selectors cannot begin with a digit — this is a CSS specification rule, not specific to Shadow DOM, but it surfaces during migration when converting jQuery selectors (which are more lenient) to native `querySelector`/`closest`.

**Fix**: Use the CSS Unicode escape sequence. The digit `3` is Unicode code point `U+0033`, so `.3d` becomes `.\33 d` (hex escape followed by a mandatory space delimiter):

```javascript
// WRONG — invalid CSS selector, throws SyntaxError
el.closest('.3d');

// CORRECT — CSS escape sequence for '3'
el.closest('.\\33 d');
```

In source code, the backslash must be doubled (`\\33`) because JavaScript string literals consume one level of escaping. The space after `33` is part of the CSS escape syntax (it terminates the hex sequence) and is NOT part of the class name.

**RULE**: When converting jQuery selectors to native `querySelector`/`closest`, check for class names starting with a digit. Use `\\xx ` (escaped hex code + space) to make them valid CSS selectors. Common in this codebase: `.3d` → `.\\33 d`.

### 15. Body styles in component CSS are ignored inside Shadow DOM

**Bug**: UIComponent injects CSS into a global `<style>` tag in `<head>`, so rules targeting `body` work:

```css
/* UIComponent CSS — works because it's in the global stylesheet */
body {
	background-color: #45484d;
	margin: 0;
	overflow: hidden;
}
```

In GUIComponent, CSS is scoped inside the shadow root. A `body` rule inside Shadow DOM has no effect on `document.body` — it is silently ignored.

**Fix**: Apply body styles programmatically in `onAppend()`:

```javascript
Viewer.onAppend = function onAppend() {
	document.body.style.backgroundColor = '#45484d';
	document.body.style.margin = '0';
	document.body.style.overflow = 'hidden';
};
```

Remove the `body { ... }` block from the component's CSS file — it does nothing inside Shadow DOM and is misleading.

**RULE**: Any CSS rule targeting elements outside the shadow tree (`body`, `html`, or global classes) must be applied via JavaScript in `onAppend()` or moved to a global stylesheet. Shadow DOM CSS can only style elements within the shadow root.

### 16. `:host` sizing for full-viewport components (viewers)

**Bug**: The migration guide documents fixed-size components (`:host { width: 400px; height: 317px; }`). Full-viewport components like viewers need percentage-based sizing. Without `:host` dimensions, the shadow host is 0×0 — content renders but may not be visible or interactive.

**Fix**: Use percentage dimensions with explicit positioning:

```css
:host {
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
}
```

The inner container should also fill the host:

```css
#ViewerName {
	position: absolute;
	width: 100%;
	height: 100%;
}
```

**RULE**: Full-viewport components must set `:host { width: 100%; height: 100%; top: 0; left: 0; }`. Do not assume the host will auto-expand to fit its content — Shadow DOM hosts have no intrinsic size.

### 17. Don't duplicate asset path constants across component base classes

**Bug**: During the GUIComponent implementation, `INTERFACE_PATH` (the Korean-encoded texture path prefix) was copied from `UIComponent.js` into `GUIComponent.js` as a local constant. This creates a maintenance risk — if the path changes, both copies must be updated.

**Fix**: Use `DBManager.INTERFACE_PATH` (the canonical source) instead of a local constant:

```javascript
// WRONG — duplicated constant in component base class
const INTERFACE_PATH = 'data/texture/\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba/';
_Client?.loadFile(INTERFACE_PATH + background, ...);

// CORRECT — reference the single source of truth in DBManager
_Client?.loadFile(_DB.INTERFACE_PATH + background, ...);
```

**RULE**: Asset-related path constants belong in `DBManager`. When porting functionality that uses asset paths, import from `DBManager` rather than copying constants.

### 18. jQuery `.show()` → native DOM for queried child elements

**Bug**: UIComponent code like `Viewer.ui.find('.head').show()` uses jQuery's `.show()` to make child elements visible. When migrating, it's easy to miss these and leave dead jQuery calls or incorrectly replace them.

**Fix**: Use `querySelector` on the shadow root and set `style.display` directly:

```javascript
// BEFORE (UIComponent — jQuery)
Viewer.ui.find('.head').show();

// AFTER (GUIComponent — native DOM)
const root = Viewer.getRoot();
root.querySelector('.head').style.display = 'block';
```

Use `'block'` (or the appropriate display value) to override CSS `display: none`. Use `''` (empty string) only if the element's CSS does NOT declare `display: none` (see pitfall #10).

**RULE**: When converting jQuery `.show()`/`.hide()` on child elements found via `.find()`, replace with `querySelector` + explicit `style.display` assignment. Review what the element's CSS `display` value is to choose the correct override.

---

## Quick Reference: What NOT to do

| Don't                                                                      | Do instead                                                                                                  | Why                                                                                                                   |
| -------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------- |
| `jQuery(element).show()` inside shadow                                     | `element.style.display = ''`                                                                                | jQuery sets `display:block`, kills flex/grid                                                                          |
| `$el.closest('body').length`                                               | `el.isConnected`                                                                                            | `.closest()` can't cross shadow boundary                                                                              |
| `document.querySelector('.my-shadow-element')`                             | `this._shadow.querySelector(...)`                                                                           | Global queries can't see shadow content                                                                               |
| `this.ui.find('.foo')`                                                     | `this._shadow.querySelector('.foo')`                                                                        | Proxy is for interop only                                                                                             |
| `this.ui.css('top', '100px')`                                              | `this._host.style.top = '100px'`                                                                            | Proxy is for interop only                                                                                             |
| `this.ui.show()` / `this.ui.hide()`                                        | `this._host.style.display = ''` / `= 'none'`                                                                | Proxy is for interop only                                                                                             |
| `this.ui.is(':visible')`                                                   | `this._host.style.display !== 'none'`                                                                       | Proxy is for interop only                                                                                             |
| Put `top`/`left` on inner element                                          | Put on `:host`                                                                                              | Breaks magnetic snap positioning                                                                                      |
| Omit `:host { width; height }`                                             | Always declare dimensions on `:host`                                                                        | Host collapses to 0×0, snap/overflow broken                                                                           |
| Register click handlers on `document.body` expecting shadow targets        | Register inside `this._container`                                                                           | Event retargeting hides real target                                                                                   |
| Bind events in `onAppend()`                                                | Bind in `init()`, restore state in `onAppend()`                                                             | `onAppend()` runs every time — duplicates bindings                                                                    |
| Set `position`/`z-index` on `:host` in CSS                                 | Omit — set automatically by JS                                                                              | Redundant, may conflict                                                                                               |
| `onKeyDown` without `shadowRoot.activeElement` guard                       | Check `(this._shadow \|\| this._host).activeElement.tagName`                                                | `document.activeElement` returns host, not the real input inside shadow                                               |
| `position: absolute` on inner div of dynamic-size component                | `display: block` (no positioning)                                                                           | Creates 0×0 containing block that breaks child positioning (see §4b)                                                  |
| `content.textContent = DB.getSkillDescription(id)`                         | `content.innerHTML = _formatROText(...)`                                                                    | jQuery `.text()` is overridden to process `^rrggbb` colors, `^nItemID`, newlines (see §13)                            |
| Convert all callbacks to arrow functions blindly                           | Keep `function()` when caller uses `.call()`/`.apply()`                                                     | Arrow functions ignore dynamic `this` binding (see §9)                                                                |
| CSS `display: none` + `element.style.display = ''` to show                 | Use explicit `'block'`/`'none'`, or omit CSS `display: none`                                                | Empty string removes inline style, falls back to CSS `none` (see §10)                                                 |
| Assume children inherit only scoped styles from `:host`                    | Add `pointer-events: auto` on children if `:host` is `none`                                                 | Inheritable CSS crosses shadow boundary: `pointer-events`, `color`, `cursor`, etc. (see §11)                          |
| `jQuery._data(window,'events').unshift(events.pop())`                      | `addEventListener(event, handler, true)` (capture phase)                                                    | Native DOM has no queue reordering; capture phase guarantees priority (see §12)                                       |
| `return false` from native event handler                                   | Explicit `stopImmediatePropagation()` + `preventDefault()`                                                  | Native handler return values are ignored; only jQuery interprets `return false` (see §12)                             |
| Move element to `document.body` without inline styles                      | Apply inline styles before `element.remove()` (see §24)                                                     | Elements outside shadow root lose all scoped CSS                                                                      |
| Duplicate `width`/`height` on both `:host` and inner element               | Put dimensions on inner element only when content overflows (§15)                                           | Conflicting size constraints create scrollbars in components with complex content                                     |
| Check `event.isTrigger` in migrated handler                                | Remove the check entirely (see §21)                                                                         | `isTrigger` is jQuery-only; native DOM events never set this property                                                 |
| Inner element without explicit height when host uses `overflow: hidden`    | Add `height: 100%` to inner element                                                                         | `bottom`-anchored children (resize handles, footers) position relative to inner element, not clipped host (see §23)   |
| `this._host.style.display = 'none'` in `init()` for on-demand components   | Do not hide — hide only in toggle-style components                                                          | `append()` does not reset `display`; host stays permanently hidden (see §24)                                          |
| `element.style.display` to find visible element                            | Also check `getComputedStyle(element).display` (see §22)                                                    | Inline style may be empty while CSS sets `display: none`                                                              |
| `position: absolute` on inner root when host should auto-size              | Use `position: relative` instead (see §25)                                                                  | Absolute inner root collapses host width; relative provides position context while keeping normal flow                |
| Omit `contextmenu` handler on right-click areas                            | Add `contextmenu` listener with `e.preventDefault()` (see §26)                                              | UIComponent's `return false` auto-prevented; native handlers must suppress explicitly                                 |
| `this.offsetTop` / `this.offsetLeft` for element position                  | Use `getBoundingClientRect()` relative to root (see §27)                                                    | Shadow DOM `offsetTop` is relative to intermediate positioned parent (scrollbar), not component root                  |
| `root.querySelector(sel)` when multiple elements may match                 | Use `root.querySelectorAll(sel)` + `forEach` (see §28)                                                      | `querySelector` returns only the first match; multi-slot items only update one slot                                   |
| Setting `scrollTop` without syncing custom scrollbar                       | Call `element._roScrollbarRestart()` after (see §29)                                                        | Scrollbar poller runs on 300ms interval; thumb jumps until it catches up                                              |
| Custom `_formatROText()` when `DB.formatMsgToHtml()` exists                | Use `DB.formatMsgToHtml(text)` (see §30)                                                                    | Centralized utility already handles `^rrggbb`, item substitution, newlines                                            |
| `querySelector` on slot index without null check                           | Always null-guard: `if (!el) return;` (see §31)                                                             | Server packets may reference slot indices beyond what the HTML template provides                                      |
| `<ui-button>` for toggleable elements (checkboxes, state indicators)       | Use `<button>` or `<div>` and manage `backgroundImage` yourself                                             | `<ui-button>` resets `backgroundImage` on `mouseup` to the `bg` attribute value (see §33)                             |
| `pointer-events: none` on `:host` for FREEZE mode full-viewport            | Omit `pointer-events: none` — FREEZE mode needs the host to block                                           | Clicks pass through to the game canvas behind the UI (see §35)                                                        |
| `draggable()` for sub-windows inside a component                           | Write a custom `_makeDraggable(element, handle)` helper (see §32)                                           | `draggable()` operates on `_host` only — cannot drag sub-elements independently                                       |
| Plain `<div>` for clickable elements expecting hand cursor                 | Use `<button>`, `<ui-button>`, `<a>`, or `.item-link` class                                                 | Game's sprite cursor only shows hand for elements in `CLICKABLE_SELECTOR` (see §34)                                   |
| Assume `<ui-button>` supplies `background-repeat`/`border` or centers text | Restore `background-repeat:no-repeat;border:0` + flex (plain text) / `text-align` (`<ui-text>` child) (§43) | `<ui-button>` is `extends HTMLElement` — only sets `backgroundImage`, no UA button styling → tiled bg, top-left label |

---

### 19. Elements moved outside Shadow DOM lose all scoped CSS

**Bug**: Some components remove an element from the shadow root and append it to `document.body` (e.g., a "level up" notification button that must float above all other UI). Once outside the shadow root, the element loses access to all scoped CSS defined in the component's `<style>` tag.

**Example**: The SkillList level up button (`#lvlup_job`) is styled inside the Shadow DOM with `z-index`, `position`, `width`, `height`, `border`, `background-color`, `background-repeat`. When removed from the shadow root and appended to `document.body`, all these styles vanish — the button becomes an unstyled, invisible element.

**Fix**: Apply all required CSS properties as inline styles BEFORE calling `element.remove()`:

```javascript
// In init(), before detaching the button from shadow root:
const lvlupBtn = root.querySelector('#lvlup_job');
if (lvlupBtn) {
	_btnLevelUp = lvlupBtn;
	// Copy scoped CSS properties to inline styles BEFORE removing
	_btnLevelUp.style.zIndex = '51';
	_btnLevelUp.style.position = 'absolute';
	_btnLevelUp.style.right = '0px';
	_btnLevelUp.style.bottom = '0px';
	_btnLevelUp.style.width = '43px';
	_btnLevelUp.style.height = '43px';
	_btnLevelUp.style.border = 'none';
	_btnLevelUp.style.backgroundColor = 'transparent';
	_btnLevelUp.style.backgroundRepeat = 'no-repeat';
	_btnLevelUp.remove(); // Now safe — inline styles travel with the element
}
```

**How to detect**: Search for patterns where an element is queried inside the shadow root and later appended to `document.body`, `document.documentElement`, or any element outside the shadow tree. Common indicators:

- `element.remove()` followed by `document.body.appendChild(element)`
- Elements that need to float above all UI (notification buttons, modal overlays, tooltips anchored to screen position)

**RULE**: When moving an element from inside Shadow DOM to outside (e.g., `document.body`), copy ALL required CSS properties to inline styles before calling `remove()`. Inline styles travel with the element; scoped CSS does not.

### 20. `:host` dimension conflicts causing scrollbars

**Bug**: When both `:host` and the inner element (e.g., `#Guild`) declare the same `width` and `height`, the dual constraint can cause scrollbars. This happens when the inner element's content (including borders, padding, or child elements) extends even slightly beyond the host bounds.

**Example**: Guild component had:

```css
/* CAUSED SCROLLBAR */
:host {
	top: 100px;
	left: 100px;
	width: 400px;
	height: 317px;
}

#Guild {
	position: absolute;
	width: 400px;
	height: 317px;
}
```

The inner `#Guild` with its borders and tab content slightly exceeded the host's 400×317 bounds, creating a scrollbar on the host element.

**Fix**: For components where the inner element's content can vary (tab systems, dynamic forms, etc.), omit `width`/`height` from `:host` and let only the inner element define them:

```css
/* FIXED — no scrollbar */
:host {
	top: 100px;
	left: 100px;
}

#Guild {
	position: absolute;
	width: 400px;
	height: 317px;
}
```

**When to apply this vs §4a**: §4a says `:host` MUST define `width`/`height` for `getBoundingClientRect()`, magnetic snap, and overflow checks. This is correct for most components. However, if you observe scrollbars after migration:

1. Check if both `:host` and the inner element have identical dimensions
2. If so, try removing dimensions from `:host` and test magnetic snap / overflow
3. If snap/overflow still works correctly (the inner `position: absolute` element sizes the host implicitly), keep dimensions off `:host`
4. If snap/overflow breaks, add `overflow: hidden` to `:host` instead

**Affected components**: Guild (6 tabs with varying content), and potentially any component with borders, padding, or dynamic content that can slightly exceed nominal dimensions.

**RULE**: After migrating, visually test the component. If scrollbars appear inside the window, check for duplicate dimensions between `:host` and the inner element. Remove `:host` dimensions or add `overflow: hidden` as needed.

### 21. jQuery `event.isTrigger` has no native DOM equivalent

**Bug**: Some legacy UIComponent event handlers check `event.isTrigger` to distinguish between real user events and programmatically triggered events (via jQuery's `.trigger()`). In native DOM, `event.isTrigger` is always `undefined`, so guards like `if (!event.isTrigger)` always evaluate to `true`.

```javascript
// BEFORE (UIComponent) — jQuery sets isTrigger on programmatic events
tabButton.addEventListener('click', function (event) {
	if (!event.isTrigger) {
		// Only run for real clicks, not .trigger('click')
		requestTabData();
	}
});

// AFTER (GUIComponent) — isTrigger is always undefined, guard always passes
// Remove the guard entirely:
tabButton.addEventListener('click', event => {
	requestTabData();
});
```

**How to detect**: Search for `isTrigger` in the legacy component code. If found, evaluate whether the guard is needed:

- If the code uses `jQuery.trigger()` to programmatically fire events, consider whether the native equivalent (`dispatchEvent`) needs similar discrimination. If so, use a custom property on the event: `new CustomEvent('click', { detail: { programmatic: true } })`.
- If the guard was just defensive and the handler works fine without it, remove it.

**RULE**: Remove `event.isTrigger` checks during migration. If programmatic vs real event distinction is genuinely needed, use `CustomEvent` with a `detail` property instead.

### 22. Finding visible elements requires `getComputedStyle()`, not just inline style checks

**Bug**: When using §10's pattern (explicit `display: 'block'` / `'none'` for toggling), code that searches for the "currently visible" element by checking `element.style.display` may miss elements whose visibility is controlled by CSS rules rather than inline styles.

**Example**: Guild's `onValidate()` needs to find which tab content is currently visible. After tab switching sets `style.display = 'block'` on the active tab and `style.display = 'none'` on others, the first tab (shown on initial load) may not have an inline `display` style at all — its visibility comes from CSS.

```javascript
// WRONG — misses elements visible via CSS (no inline style set)
const visible = Array.from(root.querySelectorAll('.content')).find(el => el.style.display !== 'none');

// CORRECT — check both inline style AND computed style
const visible = Array.from(root.querySelectorAll('.content')).find(el => {
	const d = el.style.display;
	return d !== 'none' && getComputedStyle(el).display !== 'none';
});
```

**RULE**: When searching for visible/hidden elements in Shadow DOM, use `getComputedStyle(element).display` as a fallback. Inline `style.display` only reflects explicitly set values, not CSS-declared ones.

### 23. Inner element without `height: 100%` breaks `bottom`-anchored children when host clips with `overflow: hidden`

**Bug**: In UIComponent, the root element (e.g., `#ShortCut`) had its height set directly via `this.ui.css('height', ...)`. Children using `position: absolute; bottom: 1px` were positioned relative to that same element's bottom. In GUIComponent, the host controls height via `this._host.style.height`, and `overflow: hidden` on `:host` clips content. But the inner element (e.g., `#ShortCut`) has no height constraint — it expands to fit all its children. A child anchored with `bottom: 1px` is positioned relative to the inner element's full height, not the host's clipped height, so it appears only when all content is visible.

**Example**: ShortCut's resize handle uses `position: absolute; bottom: 1px; right: 1px` inside `#ShortCut`. The host height is set dynamically (e.g., `34px` for 1 row, `136px` for all 4). Without a height constraint on `#ShortCut`, the resize handle sits at the bottom of all 4 rows (~136px), invisible when the host clips to 1–3 rows.

```css
/* BROKEN — resize button invisible when fewer than 4 rows shown */
:host {
	width: 280px;
	overflow: hidden; /* clips to host height */
}
#ShortCut {
	position: absolute;
	width: 280px;
	/* no height — expands to all 4 rows */
}

/* FIXED — inner element tracks host height */
:host {
	width: 280px;
	overflow: hidden;
}
#ShortCut {
	position: absolute;
	width: 280px;
	height: 100%; /* matches host height, bottom-anchored children stay visible */
}
```

**How to detect**: Search for children with `bottom: Npx` (resize handles, footers, status bars) inside the inner element. If the host uses `overflow: hidden` for dynamic height clipping, the inner element needs `height: 100%` to keep those children in view.

**RULE**: When `:host` uses `overflow: hidden` to clip content and the inner element contains `bottom`-anchored children, add `height: 100%` to the inner element so it tracks the host's dynamic height.

---

### 24. `init()` hiding breaks on-demand components (toggle-style vs on-demand)

**Bug**: Components that are appended on demand (e.g., Announce — appended only when a server announcement arrives) should NOT set `this._host.style.display = 'none'` in `init()`. GUIComponent's `append()` method does `parent.appendChild(this._host)` but does **not** reset `display`. The host stays permanently hidden even after `append()` is called.

This differs from toggle-style components (e.g., Clan, Inventory) that start hidden and are shown via `toggle()` or `this.ui.show()`. Those components correctly hide in `init()` because `toggle()`/`show()` explicitly sets `display = ''`.

```javascript
// WRONG — on-demand component hidden permanently
Announce.init = function init() {
	const root = Announce.getRoot();
	this.canvas = root.querySelector('canvas');
	this.ctx = this.canvas.getContext('2d');
	this._host.style.display = 'none'; // ← BUG: append() won't undo this
};

// CORRECT — no hiding for on-demand components
Announce.init = function init() {
	const root = Announce.getRoot();
	this.canvas = root.querySelector('canvas');
	this.ctx = this.canvas.getContext('2d');
};
```

**How to detect**: Check how the component is used in the engine code. If the pattern is `Component.append()` → `Component.set(data)` (the component is added to the DOM only when needed), it is on-demand. Do not hide it in `init()`. If the pattern is `Component.append()` at startup and then `Component.toggle()` / `Component.show()` / `Component.hide()` to control visibility, it is toggle-style and hiding in `init()` is correct.

**Also watch for the inner div**: If the inner `#ComponentName` div has `position: absolute` in CSS but the component has no fixed dimensions on `:host`, the host element collapses to 0×0 (since the absolutely-positioned inner content is out of flow). For on-demand overlay components without fixed dimensions, remove `position: absolute` from the inner div and let the shadow content size the host naturally.

**RULE**: Only hide in `init()` for toggle-style components whose `show()`/`toggle()` explicitly sets `display`. On-demand components (appended only when needed, never toggled) must NOT hide in `init()`.

### 25. Inner root `position: relative` vs `position: absolute` for host auto-sizing

**Bug**: The migration guide (§4a) recommends `position: absolute` on the inner root element for fixed-size components. However, when the host element should derive its width from the inner content (i.e., no explicit `width` on `:host`), `position: absolute` takes the inner element out of normal flow — the host collapses to 0×0 and the component appears too narrow.

**Example**: Inventory has no fixed `width` on `:host` because it can be resized. The inner `#InventoryV0` needs to provide a positioning context for its absolute children (tabs, resize handle, overlays), but should also size the host:

```css
/* BROKEN — host collapses to 0 width, inventory appears as a narrow sliver */
:host {
	top: 100px;
	left: 100px;
}
#InventoryV0 {
	position: absolute; /* out of flow → host collapses */
	width: 280px;
	height: 317px;
}

/* FIXED — host auto-sizes from inner content, children still positioned correctly */
:host {
	top: 100px;
	left: 100px;
}
#InventoryV0 {
	position: relative; /* in flow → sizes host; still a containing block */
	width: 280px;
	height: 317px;
}
```

**When to use which**:

- `position: absolute` — when `:host` has explicit `width`/`height` (the host sizes itself; inner element fills it)
- `position: relative` — when `:host` has NO explicit dimensions and must auto-size from inner content, AND the inner element has absolute children that need a positioning context

**Affected components**: Inventory V0-V3, Equipment V0-V4, CartItems, ItemInfo — all use `position: relative` on the inner root because the host derives its size from the inner content.

**RULE**: If `:host` omits `width`/`height`, use `position: relative` (not `absolute`) on the inner root. This keeps the inner element in normal flow (sizing the host) while still serving as a containing block for absolutely-positioned children.

### 26. `contextmenu` event must be explicitly prevented in GUIComponent

**Bug**: In UIComponent, jQuery's `.mousedown()` handlers that `return false` automatically call both `stopPropagation()` and `preventDefault()`, which suppresses the browser's native right-click context menu. After migrating to GUIComponent with native `addEventListener('mousedown', ...)`, the return value is ignored — the browser context menu appears on right-click in areas like Inventory, Equipment, and CartItems.

**Fix**: Add an explicit `contextmenu` event listener with `preventDefault()` on any element that handles right-click:

```javascript
// UIComponent (jQuery) — return false handled everything
this.ui.find('.items').on('contextmenu', '.item', function () {
	showItemOptions(this);
	return false; // ← jQuery: stopPropagation + preventDefault
});

// GUIComponent (native) — must prevent contextmenu explicitly
root.addEventListener('contextmenu', e => {
	const item = e.target.closest('.item');
	if (item) {
		e.preventDefault(); // ← suppress browser context menu
		e.stopImmediatePropagation();
		showItemOptions(item);
	}
});
```

**How to detect**: Search for right-click handling in the legacy component: `.mousedown()` handlers that check `event.which === 3`, or `.on('contextmenu', ...)`. All of these need corresponding `contextmenu` listeners with `preventDefault()` in the migrated version.

**RULE**: Any GUIComponent that handles right-click must add a `contextmenu` event listener with `e.preventDefault()`. Native DOM does not interpret `return false` as jQuery does.

### 27. `offsetTop`/`offsetLeft` → `getBoundingClientRect()` for overlay positioning

**Bug**: Legacy UIComponent code uses jQuery's `.position()` to calculate element coordinates relative to the component root (e.g., for hover labels, tooltips, overlays). The natural native replacement — `element.offsetTop` / `element.offsetLeft` — gives coordinates relative to the element's `offsetParent`, which in Shadow DOM may be an intermediate positioned ancestor (e.g., the scrollbar widget sets `position: relative` on the scroll container). This causes overlays to appear in the wrong position.

**Example**: Equipment hover label should appear above the hovered slot button. Using `offsetTop` gives position relative to the scroll container, not the component root:

```javascript
// WRONG — offsetTop relative to intermediate positioned parent (scrollbar container)
overlay.style.top = `${button.offsetTop - 22}px`;
overlay.style.left = `${button.offsetLeft - 22}px`;

// CORRECT — getBoundingClientRect relative to component root
const btnRect = button.getBoundingClientRect();
const rootRect = rootEl.getBoundingClientRect();
overlay.style.top = `${btnRect.top - rootRect.top - 22}px`;
overlay.style.left = `${btnRect.left - rootRect.left - 22}px`;
```

**Why this differs from UIComponent**: jQuery's `.position()` internally uses `getBoundingClientRect()` and computes coordinates relative to the offset parent correctly, accounting for the parent chain. Native `offsetTop`/`offsetLeft` only look at the immediate `offsetParent`, which can be a scrollbar wrapper inserted by the custom scrollbar system.

**RULE**: When positioning overlays, tooltips, or labels relative to another element inside Shadow DOM, use `getBoundingClientRect()` on both the target element and the reference root, then subtract. Do not use `offsetTop`/`offsetLeft` — the offset parent chain in Shadow DOM may include intermediate positioned elements.

### 28. `querySelectorAll` for multi-match element updates

**Bug**: When updating elements that can have multiple instances matching the same selector (e.g., multi-slot equipment items where the same `data-index` appears in multiple slots), `querySelector` returns only the first match. Only one slot gets updated.

**Example**: A headgear occupying Head_Top + Head_Mid + Head_Bottom creates HTML in all three slots with `data-index="1"`. The async `Client.loadFile` callback uses `querySelector` to set the icon — only the first slot gets the image:

```javascript
// WRONG — only updates first matching slot
Client.loadFile(DB.INTERFACE_PATH + 'item/' + it.identifiedResourceName + '.bmp', data => {
	const btn = root.querySelector(`.item[data-index="${item.index}"] button`);
	if (btn) btn.style.backgroundImage = `url(${data})`;
});

// CORRECT — updates ALL matching slots
Client.loadFile(DB.INTERFACE_PATH + 'item/' + it.identifiedResourceName + '.bmp', data => {
	const btns = root.querySelectorAll(`.item[data-index="${item.index}"] button`);
	btns.forEach(btn => {
		btn.style.backgroundImage = `url(${data})`;
	});
});
```

Apply the same pattern for item name and grade icon updates.

**How to detect**: Any `Client.loadFile` callback (or other async callback) that updates DOM elements by a `data-index` or similar attribute where the same value may appear in multiple places. Common in Equipment components with multi-slot items.

**RULE**: When an async callback updates DOM elements by attribute selector, use `querySelectorAll` + `forEach` instead of `querySelector` if the same attribute value can appear on multiple elements.

### 29. Custom scrollbar thumb jumps when `scrollTop` is set programmatically

**Bug**: CartItems (and similar components with custom scrollbars) implements a `wheel` event handler that sets `scrollTop` directly. The custom scrollbar's position poller runs on a 300ms interval — between polls, the thumb position and the actual scroll position are out of sync, causing the thumb to visually jump.

**Fix**: After programmatically changing `scrollTop`, call `element._roScrollbarRestart()` to force the scrollbar thumb to sync immediately:

```javascript
// WRONG — scrollbar thumb jumps for up to 300ms
container.addEventListener('wheel', e => {
	e.preventDefault();
	container.scrollTop += e.deltaY > 0 ? 32 : -32;
});

// CORRECT — scrollbar syncs immediately
container.addEventListener('wheel', e => {
	e.preventDefault();
	container.scrollTop += e.deltaY > 0 ? 32 : -32;
	if (container._roScrollbarRestart) {
		container._roScrollbarRestart();
	}
});
```

**When to use**: Any time you programmatically modify `scrollTop` or `scrollLeft` on an element that has a custom scrollbar (added by `_setupScrollbars()` / `applyDOMScrollbar()`).

**RULE**: After programmatic scroll changes, call `element._roScrollbarRestart()` to sync the custom scrollbar thumb. Check that the method exists before calling (defensive guard).

### 30. Use `DB.formatMsgToHtml()` instead of local `_formatROText()`

**Update to §13**: The migration guide (§13) describes creating a local `_formatROText()` helper for RO color code processing. Since then, `DB.formatMsgToHtml()` has been identified as a centralized utility in `DBManager` that performs the same processing (HTML sanitization, `^rrggbb` color codes, item ID substitution, newline conversion).

**Preferred approach**: Import and use `DB.formatMsgToHtml()` instead of duplicating the logic:

```javascript
import DB from 'DB/DBManager.js';

// Instead of a local _formatROText() helper:
content.innerHTML = DB.formatMsgToHtml(DB.getItemDescription(itemId));
```

**When to still use a local helper**: If the component needs custom text processing beyond what `DB.formatMsgToHtml()` provides (e.g., additional tag whitelist, custom substitutions), create a local helper that wraps or extends it.

**RULE**: Prefer `DB.formatMsgToHtml()` for RO text formatting. Only create a local `_formatROText()` if additional processing is needed beyond what the centralized utility provides.

### 31. Null-guard slot queries for server packet mismatches

**Bug**: Server packets may reference slot indices that exceed the number of HTML containers defined in the component template. For example, the ShortCut component renders 36 containers (4 rows × 9 slots), but the server may send 38 slot entries. A `querySelector` for index 37 returns `null`, and setting `.innerHTML` on it crashes.

```javascript
// WRONG — crashes when index exceeds HTML template slots
const ui = root.querySelector(`.container[data-index="${index}"]`);
ui.innerHTML = ''; // TypeError: Cannot set properties of null

// CORRECT — guard against missing containers
const ui = root.querySelector(`.container[data-index="${index}"]`);
if (!ui) return;
ui.innerHTML = '';
```

**How to detect**: Any function that receives indices from server packets (shortcut slots, equipment slots, inventory positions) and queries the DOM by those indices. The server's slot count may differ from the client's HTML template.

**RULE**: Always null-check `querySelector` results when the query selector includes dynamic values from server data. Add an early `return` or `continue` if the element is not found.

### 32. Sub-window dragging requires a custom helper — `draggable()` only operates on `_host`

**Bug**: GUIComponent's `draggable(handleSelector)` method makes the entire `_host` element draggable by a handle inside the shadow. For components with multiple independently draggable sub-windows (e.g., NpcStore has InputWindow, OutputWindow, AvailableItemsWindow, PurchaseResult), `draggable()` cannot be used — it always moves `_host`.

**Fix**: Write a local `_makeDraggable(element, handle)` helper that attaches mousedown/mousemove/mouseup listeners to implement dragging on arbitrary elements inside the shadow:

```javascript
function _makeDraggable(element, handle) {
	handle.addEventListener('mousedown', e => {
		if (e.which !== 1) return;
		const offsetX = element.offsetLeft - e.pageX;
		const offsetY = element.offsetTop - e.pageY;

		const onMove = ev => {
			element.style.left = `${ev.pageX + offsetX}px`;
			element.style.top = `${ev.pageY + offsetY}px`;
		};
		const onUp = ev => {
			if (ev.which === 1) {
				document.removeEventListener('mousemove', onMove);
				document.removeEventListener('mouseup', onUp);
			}
		};
		document.addEventListener('mousemove', onMove);
		document.addEventListener('mouseup', onUp);
		e.stopImmediatePropagation();
	});
}

// Usage in init():
_makeDraggable(InputWindow, InputWindow.querySelector('.titlebar'));
_makeDraggable(OutputWindow, OutputWindow.querySelector('.titlebar'));
```

**How to detect**: Any component where the original UIComponent uses multiple jQuery UI `.draggable()` calls on different child elements, or implements custom drag logic on sub-windows.

**RULE**: `GUIComponent.draggable()` only moves `_host`. For independently draggable sub-windows, write a local `_makeDraggable()` helper. Attach `mousemove`/`mouseup` to `document` (not the shadow root) so dragging works when the cursor leaves the element bounds.

### 33. `<ui-button>` overrides `backgroundImage` — don't use for toggleable elements

**Bug**: The `<ui-button>` Custom Element (defined in `src/UI/Elements/UIButton.js`) manages `style.backgroundImage` internally. On `connectedCallback`, it stores the `bg`, `hover`, and `down` attribute values as URIs. Its `mouseup` and `mouseout` handlers call `update()` which resets `style.backgroundImage` back to the original `bgUri` value.

If you use `<ui-button>` for a checkbox/toggle and set `style.backgroundImage` in your toggle handler (e.g., switching between `checkbox_0.bmp` and `checkbox_1.bmp`), the button's `mouseup` handler fires immediately after and overwrites your change back to the original `bg` attribute value. The checkbox appears to flicker — the checked image appears briefly then reverts to unchecked.

```javascript
// WRONG — ui-button's mouseup resets backgroundImage after your handler runs
// <ui-button class="selectall" bg="checkbox_0.bmp"></ui-button>
selectAll.style.backgroundImage = `url(${checkedImageData})`;
// → mouseup fires → backgroundImage reset to checkbox_0.bmp

// CORRECT — use <button> (native element, no background management)
// <button class="selectall"></button>
selectAll.style.backgroundImage = `url(${checkedImageData})`;
// → stays as set
```

**When to use `<ui-button>`**: Standard buttons where the background should follow mouse states (normal → hover → pressed → normal). The button manages these transitions automatically.

**When NOT to use `<ui-button>`**: Any element where your code needs to control `backgroundImage` at runtime (checkboxes, toggles, state indicators, progress elements). Use a native `<button>` or `<div>` instead and manage the background yourself.

**CSS for replacement element**: When replacing `<ui-button>` with `<button>`, add these CSS properties to replicate the visual behavior:

```css
.selectall {
	display: inline-block;
	border: none;
	background-color: transparent;
	background-repeat: no-repeat;
	cursor: pointer;
}
```

**RULE**: Never use `<ui-button>` for elements whose `backgroundImage` is set dynamically at runtime. Use `<button>` or `<div>` and manage the background image in your own handler.

### 34. Sprite cursor `CLICKABLE_SELECTOR` — use recognized elements for hand cursor

**Bug**: The game uses a custom sprite cursor (not CSS `cursor`). GUIComponent's `_setupShadowCursorEvents()` registers `mouseover`/`mouseout` listeners inside the shadow that detect when the mouse is over a "clickable" element and switch the sprite cursor to the hand/click state via `_Cursor.setType(CLICK)`.

The detection uses a hardcoded `CLICKABLE_SELECTOR`:

```javascript
const CLICKABLE_SELECTOR = [
	'a',
	'button',
	'ui-button',
	'input',
	'label',
	'select',
	'textarea',
	'.item-link',
	'.draggable',
	'.ro-custom-scrollbar',
	'.ro-custom-scrollbar *'
].join(',');
```

A plain `<div>` is NOT in this list. If you replace `<ui-button>` with `<div>` for a checkbox (§33), the sprite cursor will not show the hand on hover. Use `<button>` instead, which is in the selector list.

**Elements recognized for hand cursor**: `a`, `button`, `ui-button`, `input`, `label`, `select`, `textarea`, `.item-link`, `.draggable`, `.ro-custom-scrollbar`.

**RULE**: When choosing a replacement element for `<ui-button>`, prefer `<button>` over `<div>` so the game's sprite cursor shows the hand on hover. If `<div>` is required, add one of the recognized classes (e.g., `.item-link`) or consider extending `CLICKABLE_SELECTOR` in GUIComponent.js.

### 35. FREEZE mode + `pointer-events: none` on `:host` = clicks pass through to game

**Bug**: §11 explains that `:host { pointer-events: none }` is used for CROSS mode overlays, with `pointer-events: auto` on interactive children. This is correct for CROSS mode. However, applying the same pattern to FREEZE mode full-viewport components (like NpcStore) breaks modal behavior — clicks pass through the transparent areas of `:host` to the game canvas behind it, allowing the player to move or interact with NPCs while the store window is open.

FREEZE mode sets `Mouse.intersect = false` at the engine level, but `pointer-events: none` on `:host` means the browser delivers click events directly to elements behind the host (the game canvas), bypassing the shadow DOM entirely. The engine's mouse intersection flag doesn't prevent the browser from dispatching native DOM events.

```css
/* WRONG — FREEZE mode with pointer-events: none on :host */
/* Clicks pass through to game canvas */
:host {
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
	pointer-events: none; /* ← BUG: defeats FREEZE */
}

/* CORRECT — FREEZE mode without pointer-events: none */
/* Host blocks all clicks, sub-windows are interactive */
:host {
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
}
```

**When to use `pointer-events: none` on `:host`**: Only for CROSS mode components (transparent overlays, HUD elements) where mouse events should pass through to the game.

**When NOT to use**: STOP and FREEZE mode components. STOP components have fixed dimensions (host naturally blocks only its own area). FREEZE components cover the full viewport and must block ALL clicks.

**RULE**: Never use `pointer-events: none` on `:host` for FREEZE mode components. The full-viewport host must block clicks to implement modal behavior. Only CROSS mode components should use `pointer-events: none` on `:host`.

### 36. `<ui-image>` lifecycle race: `attributeChangedCallback` fires before `connectedCallback`

**Bug**: When a GUIComponent's `render()` returns HTML containing `<ui-image src="...">`, the browser parses it via `innerHTML`. During parsing, `attributeChangedCallback('src', ...)` fires immediately when the parser encounters the `src` attribute — but `connectedCallback` has not run yet, so `this.parentElement` is `null`. The image load silently fails because there is no parent to apply the background to.

```
innerHTML parsing timeline:
  1. <ui-image> element created
  2. attributeChangedCallback('src', null, 'path.bmp')  ← parentElement is null!
  3. Element inserted into DOM tree
  4. connectedCallback()                                  ← parentElement is now available
```

**Fix applied in `src/UI/Elements/UIImage.js`**: Defer the initial image load to `connectedCallback`. Skip `attributeChangedCallback` until `_initialized = true` (set in `connectedCallback`). If `parentElement` is still null in `connectedCallback` (possible during complex DOM operations), retry with `requestAnimationFrame`.

```javascript
class UIImage extends HTMLElement {
	connectedCallback() {
		if (this._initialized) return;
		this._initialized = true;
		this.style.display = 'none';
		this._applyBackground();
	}

	attributeChangedCallback(name, oldVal, newVal) {
		// Only reload on dynamic src changes AFTER initial setup.
		if (name === 'src' && this._initialized) {
			this._applyBackground();
		}
	}

	_applyBackground() {
		const target = this.parentElement;
		if (!target) {
			requestAnimationFrame(() => {
				if (this.parentElement) this._loadImage(path, this.parentElement);
			});
			return;
		}
		this._loadImage(path, target);
	}
}
```

**RULE**: Custom Elements that need `parentElement` must defer work to `connectedCallback`, not `attributeChangedCallback`. This applies to `<ui-image>` and any future Custom Elements that modify their parent.

### 37. `.ui-component-root` wrapper has no dimensions — full-viewport inner divs must use `position: absolute`

**Bug**: GUIComponent wraps all `render()` output in a `<div class="ui-component-root">` (see DOM Structure above). This wrapper has no CSS dimensions — it is an unstyled block element that collapses to its content height. For full-viewport overlay components (like MobileUI), setting `position: relative; width: 100%; height: 100%` on the inner `#ComponentName` div does NOT work because `100%` resolves relative to the parent `.ui-component-root`, which has zero explicit height.

```
<host>  (position: absolute; width: 100%; height: 100%)
  #shadow-root
    <div class="ui-component-root">      ← NO dimensions! Collapses to content height.
      <div id="MobileUI"                 ← position: relative; height: 100% → 100% of 0 = 0
          style="width:100%; height:100%">
        <button style="position:absolute; bottom:10%">  ← positioned in 0-height box!
```

**Fix**: Use `position: absolute` on the inner div instead of `position: relative`. This takes it out of normal flow and positions it relative to `:host` (the nearest positioned ancestor), bypassing the unstyled `.ui-component-root` wrapper entirely.

```css
/* WRONG — collapses to zero because .ui-component-root has no height */
#MobileUI {
	position: relative;
	width: 100%;
	height: 100%;
}

/* CORRECT — positions relative to :host, ignoring .ui-component-root */
#MobileUI {
	position: absolute;
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
}
```

**RULE**: For full-viewport overlays, use `position: absolute; top: 0; left: 0; width: 100%; height: 100%` on the inner `#ComponentName` div. Do NOT use `position: relative` — it makes the element's size depend on `.ui-component-root`, which has no explicit dimensions.

### 38. CROSS-mode full-viewport overlay: selective `pointer-events` pattern

For CROSS-mode components that cover the full viewport (like MobileUI), you need transparent areas to pass clicks through to the game while keeping interactive elements clickable. This combines §11 and §16 into a specific pattern:

```css
/* :host covers entire viewport, passes all clicks through */
:host {
	width: 100%;
	height: 100%;
	pointer-events: none;
}

/* Inner container also passes clicks through */
#MobileUI {
	position: absolute;
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
	pointer-events: none;
}

/* Only interactive elements receive clicks */
#MobileUI button,
#MobileUI .joystick-base {
	pointer-events: auto;
}
```

**Key difference from FREEZE mode (§35)**: FREEZE components block ALL clicks by omitting `pointer-events: none` on `:host`. CROSS overlays must explicitly opt-in interactive children with `pointer-events: auto`.

**Also set `mouseMode = GUIComponent.MouseMode.CROSS`** — this tells the engine not to block 3D scene mouse intersection when the cursor is over the host.

### 39. `background-repeat` defaults matter for asset-driven window frames

**Bug**: When migrating components with tiled background textures (window frame headers, footers, content areas), do NOT add `background-repeat: no-repeat` or `background-color: transparent` to center/body elements. The original UIComponent CSS often had no explicit `background-repeat`, relying on the browser default (`repeat`) so that center bar textures tile horizontally to fill the window width.

Window frames use a 9-patch (3×3 slice) pattern:

- **Corner pieces** (e.g., `titlebar_left.bmp`, `titlebar_right.bmp`): fixed size, sized so only one tile is visible — repeating doesn't matter
- **Center pieces** (e.g., `titlebar_mid.bmp`, `btnbar_mid2.bmp`): variable width, designed to tile with `repeat-x` to fill the window

```css
/* WRONG — center texture shows only once, leaving empty space */
.header .center {
	background-repeat: no-repeat;
	background-color: transparent;
}

/* CORRECT — omit background-repeat so center tiles horizontally (browser default: repeat) */
.header .center {
	/* No background-repeat rule — defaults to repeat */
}
```

**RULE**: When migrating, do not add `background-repeat: no-repeat` unless the original CSS explicitly had it. For window frames, center/body sections must tile. Corner sections can be either way since they're sized to show exactly one tile.

### 40. Use viewport units (`vmin`) for mobile-responsive component sizing

Components intended for mobile use (like MobileUI) should use viewport-relative units instead of fixed `px` for element sizes. Fixed pixel sizes don't scale with screen dimensions, making buttons too small on high-DPI mobile devices or too large on small screens.

```css
/* WRONG — fixed sizes don't adapt to screen */
.primary {
	width: 45px;
	height: 45px;
}
.joystick-container {
	width: 100px;
	height: 100px;
}

/* CORRECT — scales proportionally with viewport */
.primary {
	width: 11vmin;
	height: 11vmin;
}
.joystick-container {
	width: 25vmin;
	height: 25vmin;
}
```

**Unit choice**: `vmin` (the smaller of `vw` and `vh`) keeps elements at a consistent physical proportion regardless of landscape/portrait orientation. Use `%` for positions (already common) and `vmin` for sizes.

**Don't set pixel-based dimensions in `onAppend`** for viewport-sized components. Let CSS `width: 100%; height: 100%` handle it — pixel dimensions set once in `onAppend` become stale when the screen resizes or rotates.

### 41. `touch-action: manipulation` must target `:host` explicitly inside Shadow DOM

**Bug**: `Common.css` contains `html, body { touch-action: manipulation; }` to prevent double-tap zoom. Inside Shadow DOM, there is no `html` or `body` element — this rule matches nothing. Mobile browsers will still trigger double-tap zoom and input-focus zoom on elements inside Shadow DOM.

**Fix applied in `Common.css`**: Add explicit rules targeting Shadow DOM elements:

```css
:host {
	touch-action: manipulation;
}

input,
textarea,
select {
	touch-action: manipulation;
}
```

**Note**: `touch-action: manipulation` prevents double-tap zoom but does NOT prevent iOS Safari's input-focus zoom (which triggers when `font-size < 16px`). The only CSS-only fix for input-focus zoom is setting `font-size: 16px` on inputs — but this may break the asset-driven UI layout. As a mitigation, the Renderer listens to `visualViewport` resize events to keep the canvas in sync if the browser does zoom.

### 42. UIComponent inside GUIComponent Shadow DOM — use `ComponentName.getRoot()` helper

**Bug**: If a legacy `UIComponent` is appended inside a `GUIComponent`'s Shadow DOM (e.g., SwitchEquip inside EquipmentV4), it breaks because `UIComponent` uses `document.getElementById()` and `document.querySelector()` to find its elements. These global DOM queries cannot see into Shadow DOM.

```javascript
// UIComponent.parseHTML() uses:
document.getElementById('SwitchEquip'); // → null (element is inside shadow DOM)
```

**Fix**: Migrate the inner component to GUIComponent as well, using `ComponentName.getRoot()` / `this.getRoot()` to query within the correct shadow root. If migration is impractical, ensure the UIComponent's root element is appended to `document.body` (not inside another component's shadow DOM).

**RULE**: Do not embed a `UIComponent` inside a `GUIComponent`'s Shadow DOM. Either migrate the inner component to GUIComponent, or keep it as a sibling in `document.body`.

---

### 43. `<ui-button>` carries no button styling — restore background-repeat/border + text alignment

**Bug**: `<ui-button>` (`src/UI/Elements/UIButton.js`) is an autonomous custom element (`extends HTMLElement`), so it has the default `display: inline` and **none** of the user-agent `<button>` styling. `connectedCallback` only sets `style.backgroundImage` — it never sets `background-repeat`, `background-color`, `border`, or any text alignment. Migrations that removed `border: 0; background-repeat: no-repeat; background-color: transparent` from the old `button` CSS selectors (assuming `<ui-button>` would supply them) regress two ways:

1. **Background tiles** when the CSS box is larger than the BMP (default `background-repeat: repeat`) — e.g. a close button sized 42×20 over a smaller image duplicates the asset.
2. **Injected text is not centered** — it lands top-left because there is no UA centering.

**Fix** — two text patterns depending on how the label is provided:

- **Plain text** set via `.textContent` directly on the `<ui-button>` → flex centering works (anonymous text is blockified):

```css
#charcreate_v4 .btns ui-button {
	background-repeat: no-repeat;
	background-color: transparent;
	border: 0;
	display: flex;
	align-items: center;
	justify-content: center;
}
```

- **`<ui-text>` child** (`src/UI/Elements/UIText.js`, also `display: inline`) → flex does NOT reliably center the inline child; use `text-align` (+ `line-height` for vertical):

```css
#CharSelectV4 .btn.delete,
#CharSelectV4 .btn.canceldelete,
#CharSelectV4 .btn.finaldelete {
	background-repeat: no-repeat;
	background-color: transparent;
	border: 0;
	text-align: center;
	line-height: 24px; /* = button height → vertical center */
}
```

Do NOT style the `<ui-text>` itself (`display: block; width: 100%`) — it shifts the label and misaligns. Keep alignment on the button. For tall image buttons with top-anchored text (e.g. `bt_gamestart` with `padding-top`), use `text-align: center` only, omit `line-height`.

**Distinct from §33**: §33 is about `<ui-button>` overwriting `backgroundImage` at runtime (use `<button>` for toggles). §36 is about `<ui-button>` lacking static button styling (restore it in component CSS).

**How to detect**: After migration, a `<ui-button>` whose CSS box differs from the BMP size shows a tiled background; any `<ui-button>` containing text (`.textContent` or `<ui-text>`) shows the label top-left.

**RULE**: When a migrated `<ui-button>` shows a tiled background or misaligned label, restore `background-repeat: no-repeat; background-color: transparent; border: 0` on the button selector. Center plain `.textContent` with `display: flex; align-items: center; justify-content: center`; center a `<ui-text>` child with `text-align: center` (+ `line-height` = button height). Never align by styling the inner `<ui-text>`.

---

## Migrated Component Reference

Compact reference for all migrated components. Each row lists the component, its CSS strategy, mouse mode, and which pitfalls are most relevant.

| Component                   | CSS Strategy                                                                     | Mouse Mode | Key Pitfalls       | Notes                                                                                             |
| --------------------------- | -------------------------------------------------------------------------------- | ---------- | ------------------ | ------------------------------------------------------------------------------------------------- |
| **Clan**                    | Fixed-size: `:host { width; height }`, inner `position: absolute`                | STOP       | §4a, §20           | First migration; still uses some `this.ui` proxy calls                                            |
| **StatusIcons**             | Dynamic-size: no dims on `:host`, `overflow: visible`, inner `display: block`    | CROSS      | §4b, §9, §11       | `StatusIcons.getRoot()` helper; mixed arrow/regular callbacks for `Texture.load`                  |
| **SkillTargetSelection**    | Overlay: `:host { pointer-events: none }`, children `position: fixed`            | CROSS      | §10, §11, §12      | Capture-phase mousedown; `needFocus = false`                                                      |
| **SkillDescription**        | Tooltip: no fixed dims, dynamic positioning                                      | STOP       | §13, §30           | `DB.formatMsgToHtml()` for skill descriptions                                                     |
| **Guild**                   | No dims on `:host` (avoids scrollbar §20), inner `position: absolute`            | STOP       | §10, §20, §21, §22 | 6 tabs; `getComputedStyle()` for visible tab detection                                            |
| **SkillList / SkillListV0** | No dims on `:host`, inner `position: absolute`                                   | STOP       | §10, §19           | Level up button moved outside shadow with inline styles                                           |
| **SkillListMH**             | No dims on `:host`, class selector `.SkillListMH` (not ID)                       | STOP       | —                  | Factory pattern: `createSkillListMH(type)` creates two instances                                  |
| **ShortCut**                | `:host { overflow: hidden }`, inner `height: 100%`                               | STOP       | §23, §31           | Dynamic row clipping; null-guard for server slots                                                 |
| **ShortCuts**               | Position only on `:host`, dims on inner                                          | STOP       | §8, §20            | `captureKeyEvents` for macro text inputs                                                          |
| **Announce**                | No dims on `:host`, inner no `position: absolute`                                | CROSS      | §24                | On-demand; do NOT hide in `init()`                                                                |
| **Inventory V0–V3**         | No dims on `:host`, inner `position: relative`                                   | STOP       | §25, §26, §27      | `contextmenu` prevention; `getBoundingClientRect()` for hover labels                              |
| **Equipment V0–V4**         | No dims on `:host`, inner `position: relative`                                   | STOP       | §25, §26, §27, §28 | `querySelectorAll` for multi-slot items                                                           |
| **CartItems**               | No dims on `:host`, inner `position: relative`                                   | STOP       | §25, §26, §29      | Scrollbar sync after wheel scroll                                                                 |
| **ItemInfo**                | No dims on `:host`, inner `position: relative`                                   | STOP       | §25, §30           | `DB.formatMsgToHtml()` for item descriptions                                                      |
| **BasicInfo V0–V5**         | Fixed-size: `:host { width; height }`, inner `position: absolute`                | STOP       | —                  | UIVersionManager routing; small/large toggle via `classList`                                      |
| **NpcBox**                  | Fixed-size: `:host { width; height }`                                            | FREEZE     | §13, §30           | `DB.formatMsgToHtml()` for NPC dialog with color codes                                            |
| **NpcMenu**                 | Dynamic-size                                                                     | FREEZE     | §13, §30, §8       | `DB.formatMsgToHtml()` for menu items; keyboard nav (UP/DOWN/ENTER/ESC)                           |
| **NpcStore**                | Full-viewport: `:host { width: 100%; height: 100% }`                             | FREEZE     | §32, §33, §34, §35 | Custom `_makeDraggable()` for sub-windows; `<button>` for checkbox (not `<ui-button>`)            |
| **InputBox**                | Fixed-size                                                                       | FREEZE     | §8                 | `captureKeyEvents` for text input; modal overlay                                                  |
| **CharCreate V0–V4**        | Fixed-size: `:host { width; height }`, inner `position: absolute`                | STOP       | §36, §39           | `<ui-image>` for backgrounds; center textures must tile (`repeat-x`); `data-background` on inputs |
| **CharSelect V1–V4**        | Fixed-size: `:host { width; height }`, inner `position: absolute`                | STOP       | §36, §39           | `<ui-image>` for `box_select` border; page ball click handlers; animated backgrounds (V4)         |
| **MobileUI**                | Full-viewport CROSS: `:host { width: 100%; height: 100%; pointer-events: none }` | CROSS      | §37, §38, §40, §41 | `position: absolute` on inner div; `vmin` sizing; selective `pointer-events: auto`                |
| **SwitchEquip**             | Fixed-size                                                                       | STOP       | §42                | Migrated from UIComponent because it was embedded inside EquipmentV4 Shadow DOM                   |
| **Error**                   | Full-page fatal error                                                            | —          | —                  | Not a GUIComponent (standalone; uses native DOM only)                                             |

### CSS Pattern Decision Guide

```
Does :host have explicit width/height?
├── YES (fixed-size) → inner: position: absolute; width: X; height: Y
│   Example: Clan, BasicInfo, InputBox
│
└── NO (auto-size / dynamic)
    ├── Does inner element need to be a positioning context for absolute children?
    │   ├── YES → inner: position: relative
    │   │   Example: Inventory, Equipment, CartItems, ItemInfo
    │   └── NO → inner: display: block
    │       Example: StatusIcons, Announce
    │
    └── Is it a full-viewport overlay?
        ├── FREEZE mode → :host { width: 100%; height: 100% } (NO pointer-events: none)
        │   Example: NpcStore
        └── CROSS mode
            ├── Fixed children (no absolute positioning needed)
            │   → :host { pointer-events: none; overflow: visible }
            │   Example: SkillTargetSelection, StatusIcons
            └── Absolute children filling viewport (§37, §38)
                → :host { width: 100%; height: 100%; pointer-events: none }
                → inner: position: absolute; top:0; left:0; width:100%; height:100%
                → buttons/interactive: pointer-events: auto
                Example: MobileUI
```
