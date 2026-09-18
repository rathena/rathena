# Custom Scrollbar Usage Guide

The custom scrollbar system in RO Browser provides a way to have stylized scrollbars that match the RO interface while solving common issues like layout jitter and browser inconsistencies.

## 1. Basic Usage

The system automatically applies a scrollbar to any element inside a `UIComponent` that has:

- `overflow-y: auto;`
- `overflow-y: scroll;`

### Dynamic Padding

The scrollbar **automatically** manages `padding-right`. It will only add the space (e.g., 13px) when the content is long enough to require scrolling. If the content fits, the padding is removed to maximize your display area.

## 2. Using Custom Skins

To use a non-default skin (like the **ChatBox** skin), add the `data-scrollbar-skin` attribute to your HTML element:

```html
<!-- Inside your HTML template or JS creation -->
<div class="content" data-scrollbar-skin="chatbox">...</div>
```

## 3. Adding a New Skin

To add a new skin, modify [ScrollBar.js](../src/UI/Scrollbar.js).

### Step 1: Update `skinsToLoad`

Add a new object to the `skinsToLoad` array in `ScrollBar.init`:

```javascript
{
    name: 'myskin',
    files: [
        'path/to/down.bmp',  // [0] Down Button
        'path/to/mid.bmp',   // [1] Track (Repeated Y)
        'path/to/up.bmp',    // [2] Up Button
        'path/to/bdown.bmp', // [3] Thumb Bottom Part
        'path/to/bmid.bmp',  // [4] Thumb Middle Part
        'path/to/bup.bmp'    // [5] Thumb Top Part
    ],
    colors: {
        track: '#000000', // Optional: Used if mid file is null
        thumb: '#9a9a9a'  // Optional: Used if thumb files are null
    },
    width: 13,      // Total width of the scrollbar area
    btnHeight: 12,  // Height of the up/down buttons
    btnWidth: 12,   // Width of the up/down buttons
    trackWidth: 13  // Width of the track and thumb
}
```

### Step 2: Component Tagging

Once the skin is added to `ScrollBar.js`, tag your component's element with `data-scrollbar-skin="myskin"`.

## 4. Manual Updates

If your component's content changes significantly but is not detected by the automatic `MutationObserver`, you can manually trigger a refresh:

```javascript
var ScrollBar = require('UI/Scrollbar');
ScrollBar.applyDOMScrollbar(myElement);
```
