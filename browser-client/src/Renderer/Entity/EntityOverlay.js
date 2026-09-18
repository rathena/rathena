/**
 * Renderer/Entity/EntityOverlay.js
 *
 * Screen space layer hosting the entity DOM overlays
 * (nameplate, life bar, cast bar, guild emblem, chat bubble).
 *
 * Those canvases are placed from projected 3D coordinates, so they leave the viewport
 * as soon as their entity does. Appended to document.body they enlarge the document
 * scrollable area, which lets the browser scroll (or shrink to fit, on mobile) to
 * reveal them: the whole UI ends up scaled/offset and pointer coordinates no longer
 * match the scene. This layer is viewport sized and clips its content, so an overlay
 * outside the screen can never extend the document.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

class EntityOverlay {
	/**
	 * @var {HTMLElement} the layer, created on first use
	 */
	static layer = null;

	/**
	 * Get the layer, appending it to the document if needed
	 *
	 * @return {HTMLElement} layer
	 */
	static getLayer() {
		if (!this.layer) {
			this.layer = document.createElement('div');
			this.layer.className = 'entity-overlay';
			Object.assign(this.layer.style, {
				position: 'fixed',
				top: '0px',
				left: '0px',
				width: '100%',
				height: '100%',
				overflow: 'hidden',
				pointerEvents: 'none',
				// above the scene canvas (0), below the UI components (50+)
				zIndex: '2'
			});
		}

		if (!this.layer.parentNode) {
			document.body.appendChild(this.layer);
		}

		return this.layer;
	}

	/**
	 * Add an overlay to the layer
	 *
	 * @param {HTMLElement} element
	 */
	static append(element) {
		const layer = this.getLayer();

		if (element.parentNode !== layer) {
			layer.appendChild(element);
		}
	}
}

/**
 * Export
 */
export default EntityOverlay;
