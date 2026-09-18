/**
 * UI/clampToViewport.js
 *
 * Shared viewport clamp + magnet math for UI components.
 * Used by both UIManager.fixResizeOverflow() (batch, on resize)
 * and GUIComponent._fixPositionOverflow() (single, on append).
 *
 * Pure: takes a resolved element + dimensions + magnet, mutates style only.
 */
/**
 * Keep an element inside the viewport and apply magnet constraints.
 *
 * @param {HTMLElement} el      - The element to reposition (host / ui[0]).
 * @param {number} WIDTH        - Viewport width.
 * @param {number} HEIGHT       - Viewport height.
 * @param {{TOP?:boolean,BOTTOM?:boolean,LEFT?:boolean,RIGHT?:boolean}} [magnet]
 */
export default function UIClamp(el, WIDTH, HEIGHT, magnet) {
	if (!el) return;
	const rect = el.getBoundingClientRect();
	const x = rect.left;
	const y = rect.top;
	const width = rect.width;
	const height = rect.height;
	// Overflow bottom
	if (y + height > HEIGHT) {
		el.style.top = HEIGHT - Math.min(height, HEIGHT) + 'px';
	}
	// Overflow top
	if (y < 0) {
		el.style.top = '0px';
	}
	// Overflow right
	if (x + width > WIDTH) {
		el.style.left = WIDTH - Math.min(width, WIDTH) + 'px';
	}
	// Overflow left
	if (x < 0) {
		el.style.left = '0px';
	}
	// Magnet constraints (TOP/LEFT already at 0, no adjustment needed)
	if (magnet) {
		if (magnet.BOTTOM) {
			el.style.top = HEIGHT - height + 'px';
		}
		if (magnet.RIGHT) {
			el.style.left = WIDTH - width + 'px';
		}
	}
}
