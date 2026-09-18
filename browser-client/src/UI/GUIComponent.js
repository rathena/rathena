/**
 * UI/GUIComponent.js
 *
 * Modern base class for UI components using Shadow DOM.
 * Drop-in replacement for UIComponent — provides the same automation
 * (prepare, append, remove, focus, draggable, clone, scrollbar, etc.)
 * but uses native DOM + Shadow DOM instead of old UIComponent.
 *
 * @author AoShinHo
 */

import CommonCSS from './Common.css?raw';
import Mouse from 'Controls/MouseEventHandler.js';
import UIPreferences from 'Preferences/UI.js';
import Session from 'Engine/SessionStorage.js';
import Targa from 'Loaders/Targa.js';
import ClampToViewport from 'UI/ClampToViewport.js';

/**
 * Heavy modules loaded lazily to keep viewer bundles lightweight.
 * In Online/full-game context these resolve instantly (already loaded).
 * In viewer context they are never triggered.
 */
let _Cursor = null;
let _DB = null;
let _Client = null;
let _Renderer = null;
let _EntityManager = null;
let _ScrollBar = null;

let _depsPromise = null;

async function _loadHeavyDeps() {
	if (_Cursor) return;
	const [CursorMod, DBMod, ClientMod, RendererMod, EntityManagerMod, ScrollBarMod] = await Promise.all([
		import('UI/CursorManager.js'),
		import('DB/DBManager.js'),
		import('Core/Client.js'),
		import('Renderer/Renderer.js'),
		import('Renderer/EntityManager.js'),
		import('UI/Scrollbar.js')
	]);
	_Cursor = CursorMod.default;
	_DB = DBMod.default;
	_Client = ClientMod.default;
	_Renderer = RendererMod.default;
	_EntityManager = EntityManagerMod.default;
	_ScrollBar = ScrollBarMod.default;
}

function _ensureDeps() {
	if (!_depsPromise) {
		_depsPromise = _loadHeavyDeps();
	}
	return _depsPromise;
}

/**
 * Snap cache shared across all draggable instances (same as UIComponent)
 */
let _snapCache = [];

/**
 * Mouse interaction modes
 */
const MouseMode = Object.freeze({
	CROSS: 0, // Cross the UI and intersect with scene
	STOP: 1, // Block scene intersection when mouse is over the UI
	FREEZE: 2 // Block scene intersection while UI is alive
});

/**
 * CSS properties that are unitless (don't need 'px')
 */
const CSS_NUMBER = {
	zIndex: true,
	opacity: true,
	fontWeight: true,
	lineHeight: true,
	columnCount: true,
	fillOpacity: true,
	orphans: true,
	widows: true,
	zoom: true
};

class GUIComponent {
	/**
	 * @param {string} name       - Unique component name
	 * @param {string} [cssText]  - CSS content (injected into Shadow DOM)
	 */
	constructor(name, cssText) {
		this.name = name;
		this._cssText = cssText || null;
		this._host = null; // The outer DOM element
		this._shadow = null; // ShadowRoot
		this._container = null; // Inner wrapper inside shadow

		this.magnet = { TOP: false, BOTTOM: false, LEFT: false, RIGHT: false };
		this._isDraggable = false;

		this.mouseMode = MouseMode.STOP;
		this.needFocus = true;
		this.manager = null; // Set by UIManager.addComponent()

		this.__loaded = false;
		this.__preparing = false;
		this.__active = false;
		this.__scrollbarObserver = null;
		this.__mouseStopBlock = null;

		this._keyHandler = null;
		this._enterCount = 0;
		this._savedIntersect = false;

		// Compatibility proxy — lets UIManager and old UIComponent
		// instances call .ui.css(), .ui[0], .ui.offset(), etc.
		this.ui = null; // Created in prepare()
	}

	// ─── Static ────────────────────────────────────────────

	static MouseMode = MouseMode;

	// ─── Lifecycle: prepare ────────────────────────────────

	/**
	 * Get the root element of the component.
	 */

	getRoot() {
		return this._shadow || this._host;
	}

	/**
	 * Build the Shadow DOM, inject CSS, call init(), set up
	 * mouse intersection and touch handling.
	 * Equivalent to UIComponent.prototype.prepare().
	 */
	prepare() {
		if (this.__loaded || this.__preparing) return;

		// Guards against re-entrancy and leaves __loaded false when _prepare() throws
		this.__preparing = true;
		try {
			this._prepare();
			this.__loaded = true;
		} finally {
			this.__preparing = false;
		}
	}

	_prepare() {
		_ensureDeps();

		// Create host element
		this._host = document.createElement('div');
		this._host.id = this.name;
		this._host.style.zIndex = '50';
		this._host.style.position = 'absolute';

		// Attach Shadow DOM (open for devtools inspection)
		this._shadow = this._host.attachShadow({ mode: 'open' });

		// Inject Common CSS (shared, immutable)
		const commonStyle = document.createElement('style');
		commonStyle.textContent = CommonCSS;
		this._shadow.appendChild(commonStyle);

		// Inject component CSS (hot-reloadable)
		const compStyle = document.createElement('style');
		compStyle.setAttribute('data-component', this.name);
		compStyle.textContent = this._cssText || '';
		this._shadow.appendChild(compStyle);

		// Create inner container for component content
		this._container = document.createElement('div');
		this._container.classList.add('ui-component-root');
		this._shadow.appendChild(this._container);

		// Render component content (subclass provides this)
		if (this.render) {
			this._container.innerHTML = this.render();
		}

		// Process legacy data-* attributes (background, hover, down, text, etc.)
		this._processAllDataAttrs();

		// Create jQuery compatibility proxy
		this._createUIProxy();

		document.body.appendChild(this._host);

		// Initialize (subclass provides this)
		if (this.init) {
			this.init();
		}

		// Mouse intersection handling
		this._setupMouseMode();

		this._host.remove();
	}

	// ─── Lifecycle: append ─────────────────────────────────

	/**
	 * Add the component to the DOM.
	 * Equivalent to UIComponent.prototype.append().
	 *
	 * @param {HTMLElement|string} [target] - Target element. Defaults to document.body.
	 */
	append(target) {
		this.__active = true;

		if (!this.__loaded) {
			this.prepare();
			if (this.__active) this.append(target);
			return;
		}

		// Append host to target
		const parent = target ? (typeof target === 'string' ? document.querySelector(target) : target) : document.body;

		if (!parent) {
			console.error('[GUIComponent] Unable to find target element for appending UI.');
			return;
		}

		parent.appendChild(this._host);

		// Bind keydown
		if (this.onKeyDown) {
			this._bindKeyDown();
		}

		// Freeze mode
		if (this.mouseMode === MouseMode.FREEZE) {
			Mouse.intersect = false;
			Session.FreezeUI = true;
			_Cursor?.setType(_Cursor?.ACTION?.DEFAULT ?? 0);
		}

		// Hook
		if (this.onAppend) {
			this.onAppend();
		}

		// Scrollbars
		this._setupScrollbars();

		// Fix position overflow
		this._fixPositionOverflow();

		// Focus
		this.focus();
	}

	/**
	 * Fix component position after append or screen resize.
	 * Ensures the component stays within the visible viewport
	 * and respects magnet constraints.
	 *
	 * Skipped for non-draggable (fixed-position) components since their
	 * CSS position is intentional and should not be overridden.
	 */
	_fixPositionOverflow() {
		if (!this._host || !this._isDraggable) return;
		const WIDTH = _Renderer?.width ?? window.innerWidth;
		const HEIGHT = _Renderer?.height ?? window.innerHeight;
		ClampToViewport(this._host, WIDTH, HEIGHT, this.magnet);
	}

	// ─── Lifecycle: remove ─────────────────────────────────

	/**
	 * Remove the component from the DOM.
	 * Equivalent to UIComponent.prototype.remove().
	 */
	remove() {
		this.__active = false;

		if (this.__loaded && this._host && this._host.parentNode) {
			// Hook
			if (this.onRemove) {
				this.onRemove();
			}

			// Unbind keydown
			this._unbindKeyDown();

			// Fire x_remove event (used by mouse intersection cleanup)
			this._host.dispatchEvent(new Event('x_remove'));
			if (this._shadow) {
				this._shadow.querySelectorAll('*').forEach(node => {
					node.dispatchEvent(new Event('x_remove'));
				});
			}

			// Detach from DOM
			this._host.remove();

			// Freeze mode cleanup
			if (this.mouseMode === MouseMode.FREEZE) {
				Mouse.intersect = true;
				Session.FreezeUI = false;
			}

			// Scrollbar observer cleanup
			if (this.__scrollbarObserver) {
				this.__scrollbarObserver.disconnect();
				this.__scrollbarObserver = null;
			}
		}
	}

	// ─── Focus / zIndex management ─────────────────────────

	/**
	 * Focus the UI (bring to top of other components).
	 * Equivalent to UIComponent.prototype.focus().
	 */
	focus() {
		if (!this.manager || !this.needFocus) return;

		const components = this.manager.components;
		const list = [];

		// Collect zIndex values from other active components
		for (const name in components) {
			const other = components[name];
			if (other === this || !other.__active || !other.needFocus) continue;

			// Works with both UIComponent (.ui.css) and GUIComponent (._host.style)
			const z = parseInt(this._getZIndex(other), 10);
			list[z - 50] = z;
		}

		// Re-organize to remove gaps
		let j = 0;
		for (let i = 0; i < list.length; i++) {
			if (!list[i]) {
				j++;
				continue;
			}
			list[i] -= j;
		}

		// Apply new zIndex to all other components
		for (const name in components) {
			const other = components[name];
			if (other === this || !other.__active || !other.needFocus) continue;

			const z = parseInt(this._getZIndex(other), 10);
			this._setZIndex(other, list[z - 50]);
		}

		// Push ourselves to top
		this._setZIndex(this, list.length + 50 - j);
	}

	/**
	 * Place on top of all other components.
	 * Equivalent to UIComponent.prototype.placeOnTop().
	 */
	placeOnTop() {
		if (!this.manager) return;

		const components = this.manager.components;
		const zList = [];

		for (const name in components) {
			const other = components[name];
			if (other === this || !other.__active) continue;
			zList.push(parseInt(this._getZIndex(other), 10));
		}

		this._setZIndex(this, Math.max(50, ...zList) + 1);
	}

	/**
	 * Clone a component.
	 * Equivalent to UIComponent.prototype.clone().
	 *
	 * @param {string} name - New component name
	 * @param {boolean} [full] - Copy all properties
	 * @returns {GUIComponent}
	 */
	clone(name, full) {
		const cloned = new GUIComponent(name, this._cssText);

		// Copy render function so the clone produces the same HTML
		if (this.render) cloned.render = this.render;

		// Always inherit behavioral properties
		cloned.mouseMode = this.mouseMode;
		cloned.needFocus = this.needFocus;

		if (full) {
			for (const key of Object.keys(this)) {
				if (
					key === '_host' ||
					key === '_shadow' ||
					key === '_container' ||
					key === 'ui' ||
					key === '__loaded' ||
					key === '__preparing' ||
					key === '__scrollbarObserver'
				) {
					continue; // Don't copy DOM state
				}
				cloned[key] = this[key];
			}
		}

		return cloned;
	}

	// ─── Event binding ─────────────────────────────────────

	/**
	 * Enable event type. Only 'keydown' is supported (same as UIComponent).
	 * @param {string} type
	 */
	on(type) {
		if (type.toLowerCase() === 'keydown' && this.onKeyDown) {
			this._bindKeyDown();
		}
	}

	/**
	 * Disable event type.
	 * @param {string} type
	 */
	off(type) {
		if (type.toLowerCase() === 'keydown') {
			this._unbindKeyDown();
		}
	}

	// ─── Private: keydown helpers ──────────────────────────

	_bindKeyDown() {
		if (!this.onKeyDown) return;
		this._unbindKeyDown();
		const handler = this.onKeyDown.bind(this);
		this._keyHandler = event => {
			if (handler(event) === false) {
				event.preventDefault();
			}
		};
		const useCapture = !!this.captureKeyEvents;
		window.addEventListener('keydown', this._keyHandler, useCapture);
	}

	_unbindKeyDown() {
		if (this._keyHandler) {
			window.removeEventListener('keydown', this._keyHandler);
			window.removeEventListener('keydown', this._keyHandler, true); // capture phase
			this._keyHandler = null;
		}
	}

	/**
	 * Check if an editable element is focused.
	 * @returns {boolean}
	 */
	isEditableFocused() {
		const root = this.getRoot();
		const active = root && root.activeElement;
		if (!active || !active.tagName) return false;
		if (/input|textarea|select/i.test(active.tagName)) return true;
		else if (active.getAttribute('contenteditable') === 'true') return true;
		return false;
	}

	// ─── Private: zIndex helpers (cross-compatible) ────────

	/**
	 * Get zIndex from either a GUIComponent or a legacy UIComponent.
	 * @param {GUIComponent|UIComponent} comp
	 * @returns {string}
	 */
	_getZIndex(comp) {
		if (comp._host) {
			// GUIComponent
			return comp._host.style.zIndex || '50';
		}
		// Legacy UIComponent (jQuery)
		return comp.ui.css('zIndex') || '50';
	}

	/**
	 * Set zIndex on either a GUIComponent or a legacy UIComponent.
	 * @param {GUIComponent|UIComponent} comp
	 * @param {number} value
	 */
	_setZIndex(comp, value) {
		if (comp._host) {
			comp._host.style.zIndex = value;
		} else {
			comp.ui.css('zIndex', value);
		}
	}

	// ─── Draggable ─────────────────────────────────────────

	/**
	 * Make the component draggable.
	 * Equivalent to UIComponent.prototype.draggable().
	 *
	 * @param {string|HTMLElement} [handle] - CSS selector or element to use as drag handle.
	 *                                        Defaults to the host element.
	 * @returns {GUIComponent} this
	 */
	draggable(handle) {
		const host = this._host;
		const component = this;
		const SNAP_DISTANCE = 10;

		if (!host) return this;

		this._isDraggable = true;

		// Resolve handle element
		let handleEl;
		if (!handle) {
			handleEl = host;
		} else if (typeof handle === 'string') {
			handleEl = this._shadow.querySelector(handle) || this._container.querySelector(handle);
		} else if (handle instanceof HTMLElement) {
			handleEl = handle;
		} else {
			// jQuery object from legacy code — extract native element
			handleEl = handle[0] || handle;
		}

		if (!handleEl) return this;

		const onStart = event => {
			if (event.type === 'touchstart') {
				Mouse.screen.x = event.touches[0].pageX;
				Mouse.screen.y = event.touches[0].pageY;
			} else if (event.which !== 1) {
				return;
			}

			const x = host.offsetLeft - Mouse.screen.x;
			const y = host.offsetTop - Mouse.screen.y;
			const width = host.offsetWidth;
			const height = host.offsetHeight;

			// Build snap cache from other active components
			_snapCache = [];
			if (UIPreferences.windowmagnet && component.manager) {
				const hostParent = host.offsetParent;
				const components = component.manager.components;
				for (const name in components) {
					const other = components[name];
					if (
						!other ||
						other === component ||
						!other.__active ||
						!other.needFocus ||
						!other.ui ||
						!other.ui.is(':visible')
					)
						continue;

					const el = other._host || (other.ui && other.ui[0]);
					if (!el) continue;

					if (hostParent && el.offsetParent && el.offsetParent !== hostParent) {
						continue;
					}

					_snapCache.push({
						left: el.offsetLeft,
						top: el.offsetTop,
						right: el.offsetLeft + el.offsetWidth,
						bottom: el.offsetTop + el.offsetHeight
					});
				}
			}

			host.style.transition = '';
			// Force reflow to apply immediately
			host.offsetHeight; // eslint-disable-line no-unused-expressions

			let drag;
			let currentOpacity = 1.0;
			let lastMx = Mouse.screen.x;
			let lastMy = Mouse.screen.y;

			// Stop drag on mouseup / touchend
			const onEnd = ev => {
				if (ev.type === 'touchend' || ev.which === 1 || ev.isTrigger) {
					cancelAnimationFrame(drag);
					window.removeEventListener('mouseup', onEnd);
					window.removeEventListener('touchend', onEnd);
					_snapCache = [];

					if (component.gridSnap) {
						const gw = component.gridSnap.width;
						const gh = component.gridSnap.height;
						const padX = component.gridSnap.padX || 0;
						const padY = component.gridSnap.padY || 0;

						const curRect = host.getBoundingClientRect();
						const maxXI = Math.floor(((_Renderer?.width ?? window.innerWidth) - width - padX) / gw);
						const maxYI = Math.floor(((_Renderer?.height ?? window.innerHeight) - height - padY) / gh);

						let gxi = Math.round((curRect.left - padX) / gw);
						let gyi = Math.round((curRect.top - padY) / gh);
						gxi = Math.max(0, Math.min(gxi, maxXI));
						gyi = Math.max(0, Math.min(gyi, maxYI));

						const snappedX = gxi * gw + padX;
						const snappedY = gyi * gh + padY;

						host.style.transition = `left ${component.snapDuration || 150}ms, top ${component.snapDuration || 150}ms, opacity 150ms`;
						host.style.left = snappedX + 'px';
						host.style.top = snappedY + 'px';
						host.style.opacity = '1';

						const onTransEnd = () => {
							host.style.transition = '';
							host.removeEventListener('transitionend', onTransEnd);
							if (component.onDragEnd) component.onDragEnd();
						};
						host.addEventListener('transitionend', onTransEnd);
					} else {
						host.style.transition = 'opacity 150ms';
						host.style.opacity = '1';
						const onTransEnd = () => {
							host.style.transition = '';
							host.removeEventListener('transitionend', onTransEnd);
							if (component.onDragEnd) component.onDragEnd();
						};
						host.addEventListener('transitionend', onTransEnd);
					}
				}
			};

			window.addEventListener('mouseup', onEnd);
			window.addEventListener('touchend', onEnd);

			// Drag loop
			const dragging = () => {
				const mx = Mouse.screen.x;
				const my = Mouse.screen.y;

				// Skip frame if mouse hasn't moved
				if (mx === lastMx && my === lastMy) {
					drag = requestAnimationFrame(dragging);
					return;
				}
				lastMx = mx;
				lastMy = my;

				let x_ = mx + x;
				let y_ = my + y;
				currentOpacity = Math.max(currentOpacity - 0.02, 0.7);

				// Reset magnet
				if (component.magnet) {
					component.magnet.TOP =
						component.magnet.BOTTOM =
						component.magnet.LEFT =
						component.magnet.RIGHT =
							false;
				}

				// Border magnet
				if (Math.abs(x_) < SNAP_DISTANCE) {
					x_ = 0;
					if (component.magnet) component.magnet.LEFT = true;
				}
				if (Math.abs(y_) < SNAP_DISTANCE) {
					y_ = 0;
					if (component.magnet) component.magnet.TOP = true;
				}
				if (Math.abs(x_ + width - Mouse.screen.width) < SNAP_DISTANCE) {
					x_ = Mouse.screen.width - width;
					if (component.magnet) component.magnet.RIGHT = true;
				}
				if (Math.abs(y_ + height - Mouse.screen.height) < SNAP_DISTANCE) {
					y_ = Mouse.screen.height - height;
					if (component.magnet) component.magnet.BOTTOM = true;
				}

				// Window magnet (snap to other components)
				if (UIPreferences.windowmagnet && component.manager) {
					const lockX = component.magnet && (component.magnet.LEFT || component.magnet.RIGHT);
					const lockY = component.magnet && (component.magnet.TOP || component.magnet.BOTTOM);
					let snapX = null,
						snapY = null;
					let snapXD = SNAP_DISTANCE + 1,
						snapYD = SNAP_DISTANCE + 1;

					const checkX = val => {
						const d = Math.abs(val - x_);
						if (d < snapXD) {
							snapXD = d;
							snapX = val;
						}
					};
					const checkY = val => {
						const d = Math.abs(val - y_);
						if (d < snapYD) {
							snapYD = d;
							snapY = val;
						}
					};
					const isNear = (sA, eA, sB, eB) => !(eA + SNAP_DISTANCE < sB || eB + SNAP_DISTANCE < sA);

					for (let i = 0; i < _snapCache.length; i++) {
						const box = _snapCache[i];
						if (!lockX && isNear(y_, y_ + height, box.top, box.bottom)) {
							checkX(box.left);
							checkX(box.right);
							checkX(box.left - width);
							checkX(box.right - width);
						}
						if (!lockY && isNear(x_, x_ + width, box.left, box.right)) {
							checkY(box.top);
							checkY(box.bottom);
							checkY(box.top - height);
							checkY(box.bottom - height);
						}
					}

					if (!lockX && snapX !== null) x_ = snapX;
					if (!lockY && snapY !== null) y_ = snapY;
				}

				host.style.left = x_ + 'px';
				host.style.top = y_ + 'px';
				host.style.opacity = currentOpacity;
				drag = requestAnimationFrame(dragging);
			};
			drag = requestAnimationFrame(dragging);
		};
		handleEl.addEventListener('mousedown', onStart);
		handleEl.addEventListener('touchstart', onStart);
		return this;
	}

	// ─── Mouse intersection setup ──────────────────────────
	_setupShadowCursorEvents() {
		const container = this._container;
		if (!container) return;

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

		let _hovering = false;
		let _savedType = _Cursor?.ACTION?.DEFAULT ?? 0;

		container.addEventListener('mouseover', e => {
			const target = e.target;
			if (target.closest && target.closest(CLICKABLE_SELECTOR)) {
				if (!_hovering) {
					_savedType = _Cursor?.getActualType() ?? 0;
					_hovering = true;
				}
				if ((_Cursor?.getActualType() ?? 0) !== (_Cursor?.ACTION?.CLICK ?? 0)) {
					_Cursor?.setType(_Cursor?.ACTION?.CLICK ?? 0);
				}
			}
		});

		container.addEventListener('mouseout', e => {
			if (!_hovering) return;

			const related = e.relatedTarget;
			// Still over a clickable element inside this shadow? Do nothing.
			if (related && related.closest && related.closest(CLICKABLE_SELECTOR)) {
				return;
			}

			_hovering = false;
			_Cursor?.setType(_savedType);
		});

		container.addEventListener('mousedown', e => {
			const target = e.target;
			if (target.closest && target.closest(CLICKABLE_SELECTOR)) {
				if (!_hovering) {
					_savedType = _Cursor?.getActualType() ?? 0;
					_hovering = true;
				}
				_Cursor?.setType(_Cursor?.ACTION?.CLICK ?? 0, true, 1);
			}
		});

		container.addEventListener('mouseup', e => {
			const target = e.target;
			if (target.closest && target.closest(CLICKABLE_SELECTOR)) {
				if ((_Cursor?.getActualType() ?? 0) !== (_Cursor?.ACTION?.CLICK ?? 0)) {
					_Cursor?.setType(_Cursor?.ACTION?.CLICK ?? 0);
				}
				return;
			}
			if (_hovering) {
				_hovering = false;
				_Cursor?.setType(_savedType);
			}
		});
	}

	_setupMouseMode() {
		const element = this.__mouseStopBlock || this._host;
		if (this.mouseMode === GUIComponent.MouseMode.STOP) {
			let _intersect;
			let _enter = 0;

			element.addEventListener('mouseenter', () => {
				if (_enter === 0) {
					_intersect = Mouse.intersect;
					_enter++;
					if (_intersect) {
						Mouse.intersect = false;
						_Cursor?.setType(_Cursor?.ACTION?.DEFAULT ?? 0);
						_EntityManager?.setOverEntity(null);
					}
				}
			});

			element.addEventListener('mouseleave', () => {
				if (_enter > 0) {
					_enter--;
					if (_enter === 0 && _intersect) {
						if (!Session.FreezeUI) {
							Mouse.intersect = true;
						}
						_EntityManager?.setOverEntity(null);
					}
				}
			});

			// Firefox fix: mouseleave not fired on detach
			element.addEventListener('x_remove', () => {
				if (_enter > 0) {
					_enter = 0;
					if (_intersect) {
						Mouse.intersect = true;
						_EntityManager?.setOverEntity(null);
					}
				}
			});

			// Focus on mousedown
			element.addEventListener('mousedown', () => this.focus());
		}

		if (this.mouseMode !== GUIComponent.MouseMode.CROSS) {
			element.addEventListener('touchstart', e => e.stopImmediatePropagation());
		}
		// Shadow DOM cursor events (clickable hover detection)
		this._setupShadowCursorEvents();
	}

	// ─── Scrollbar setup ───────────────────────────────────

	_setupScrollbars() {
		const self = this;
		// Search inside the shadow container, not the host (light DOM)
		const root = this._container || this._host;
		// Observe the shadow root to detect changes inside Shadow DOM
		const observeTarget = this._shadow || this._host;

		setTimeout(() => {
			if (!this._host || !this._host.parentNode) return;

			const checkScrollbars = el => {
				// Check the element itself and all descendants
				const candidates = [el, ...el.querySelectorAll('*')];
				for (const node of candidates) {
					if (node.nodeType !== 1) continue;

					if (node._roScrollbarApplied) {
						_ScrollBar?.applyDOMScrollbar(node);
						continue;
					}

					const oy = window.getComputedStyle(node).overflowY;
					if (oy === 'auto' || oy === 'scroll') {
						_ScrollBar?.applyDOMScrollbar(node);
					}
				}
			};

			// Stagger checks to wait for CSS parsing
			checkScrollbars(root);
			setTimeout(() => checkScrollbars(root), 50);
			setTimeout(() => checkScrollbars(root), 150);
			setTimeout(() => checkScrollbars(root), 500);

			// Re-apply on visibility or content changes
			const observer = new MutationObserver(mutations => {
				let needsCheck = false;
				for (const mutation of mutations) {
					if (mutation.type === 'childList') {
						needsCheck = true;
					} else if (mutation.type === 'attributes') {
						if (mutation.attributeName === 'style') {
							const oldVal = mutation.oldValue || '';
							const wasHidden = oldVal.includes('display: none') || oldVal.includes('display:none');
							const isHidden = mutation.target.style.display === 'none';
							if (wasHidden && !isHidden) needsCheck = true;
						} else if (mutation.attributeName === 'class') {
							needsCheck = true;
						}
					}
				}
				if (needsCheck) checkScrollbars(root);
			});

			observer.observe(observeTarget, {
				childList: true,
				subtree: true,
				attributes: true,
				attributeOldValue: true,
				attributeFilter: ['style', 'class']
			});

			self.__scrollbarObserver = observer;
		}, 0);
	}

	// ─── Process data-* attributes (replaces parseHTML) ────

	/**
	 * Process legacy data-* attributes on a single DOM element.
	 * Handles: data-background, data-hover, data-down, data-active, data-text, data-preload.
	 * Static so it can be called from anywhere (e.g. dynamically created buttons).
	 */
	static processDataAttrs(node) {
		if (!_Client || !_DB) {
			_ensureDeps().then(() => GUIComponent.processDataAttrs(node));
			return;
		}
		const background = node.dataset.background;
		const hover = node.dataset.hover;
		const down = node.dataset.down;
		const active = node.dataset.active;
		const msgId = node.dataset.text;
		const preload = node.dataset.preload;

		let bgUri = null;
		let hoverUri = null;
		let downUri = null;
		let activeUri = null;

		const state = { hover: false, down: false, active: false };

		const updateBg = () => {
			if (state.down && downUri) {
				node.style.backgroundImage = `url(${downUri})`;
			} else if (state.active && activeUri) {
				node.style.backgroundImage = `url(${activeUri})`;
			} else if (state.hover && hoverUri) {
				node.style.backgroundImage = `url(${hoverUri})`;
			} else if (bgUri) {
				node.style.backgroundImage = `url(${bgUri})`;
			} else {
				node.style.backgroundImage = '';
			}
		};

		// Localized text
		if (msgId && _DB?.getMessage(msgId, '')) {
			node.textContent = _DB?.getMessage(msgId, '');
		}

		// Default background
		if (background) {
			_Client?.loadFile(_DB.INTERFACE_PATH + background, dataURI => {
				bgUri = dataURI;
				if (dataURI instanceof ArrayBuffer) {
					try {
						const tga = new Targa();
						tga.load(new Uint8Array(dataURI));
						bgUri = tga.getDataURL();
					} catch (e) {
						console.error(e.message);
					}
				}
				updateBg();
			});
		}

		// Active background
		if (active) {
			_Client?.loadFile(_DB.INTERFACE_PATH + active, dataURI => {
				activeUri = dataURI;
				if (node.classList.contains('active')) {
					state.active = true;
					updateBg();
				}
			});

			const observer = new MutationObserver(mutations => {
				for (const m of mutations) {
					if (m.attributeName === 'class') {
						const isActive = node.classList.contains('active');
						if (state.active !== isActive) {
							state.active = isActive;
							updateBg();
						}
					}
				}
			});

			observer.observe(node, { attributes: true, attributeFilter: ['class'] });

			node.addEventListener('x_remove', () => observer.disconnect(), { once: true });
			if (!node._roActiveObserver) {
				node._roActiveObserver = observer;
			}
		}

		// Hover background
		if (hover) {
			_Client?.loadFile(_DB.INTERFACE_PATH + hover, dataURI => {
				hoverUri = dataURI;
			});
			node.addEventListener('mouseover', () => {
				state.hover = true;
				updateBg();
			});
			node.addEventListener('mouseout', () => {
				state.hover = false;
				updateBg();
			});
		}

		// Down background
		if (down) {
			_Client?.loadFile(_DB.INTERFACE_PATH + down, dataURI => {
				downUri = dataURI;
			});
			node.addEventListener('mousedown', () => {
				state.down = true;
				updateBg();
			});
			node.addEventListener('mouseup', () => {
				state.down = false;
				updateBg();
			});
			if (!hover) {
				node.addEventListener('mouseout', () => {
					state.down = false;
					state.hover = false;
					updateBg();
				});
			}
		}

		// Preload images
		if (preload) {
			const files = preload.split(';').map(f => _DB.INTERFACE_PATH + f.trim());
			_Client?.loadFiles(files);
		}
	}

	/**
	 * Walk the shadow DOM and process all elements with data-* attributes.
	 * Called once during prepare().
	 */
	_processAllDataAttrs() {
		const root = this._shadow || this._host;
		if (!root) return;

		const selector = '[data-background],[data-hover],[data-down],[data-active],[data-text],[data-preload]';
		const nodes = root.querySelectorAll(selector);
		for (const node of nodes) {
			GUIComponent.processDataAttrs(node);
		}
	}

	/**
	 * Compatibility shim for UIComponent.parseHTML.
	 * When called via .call(element) or .each(parseHTML),
	 * `this` is the DOM element.
	 */
	get parseHTML() {
		return function () {
			GUIComponent.processDataAttrs(this);
		};
	}

	// ─── CSS hot-reload ────────────────────────────────────

	reloadCSS(newCssText) {
		if (this._shadow) {
			const existing = this._shadow.querySelector('style[data-component]');
			if (existing) {
				existing.textContent = newCssText;
			}
		}
	}

	/**
	 * Static version for external callers (e.g. HMR).
	 */
	static reloadCSS(name, newCssText) {
		// Fallback for components not yet migrated
		const id = 'hmr-' + name;
		let hotStyle = document.getElementById(id);
		if (!hotStyle) {
			hotStyle = document.createElement('style');
			hotStyle.id = id;
			hotStyle.type = 'text/css';
			document.head.appendChild(hotStyle);
		}
		hotStyle.textContent = newCssText;
	}

	// ─── Compatibility proxy ───────────────────────────────

	/**
	 * Create a minimal jQuery-like proxy for this.ui so that
	 * UIManager and old UIComponent.focus() can interoperate
	 * during the transition period.
	 */
	_createUIProxy() {
		const host = this._host;
		const component = this;
		const proxy = {
			0: host,
			length: 1,

			css(prop, value) {
				// Setter: .css({ top: 100, left: 200 })
				if (typeof prop === 'object') {
					for (const [k, v] of Object.entries(prop)) {
						host.style[k] = typeof v === 'number' && !CSS_NUMBER[k] ? v + 'px' : String(v);
					}
					return proxy;
				}
				if (value === undefined) {
					// Getter: .css('top')
					return (
						window.getComputedStyle(host).getPropertyValue(prop.replace(/([A-Z])/g, '-$1').toLowerCase()) ||
						host.style[prop]
					);
				}
				// Setter: .css('top', 100)
				host.style[prop] = typeof value === 'number' && !CSS_NUMBER[prop] ? value + 'px' : String(value);
				return proxy;
			},

			offset() {
				const rect = host.getBoundingClientRect();
				return { left: rect.left + window.scrollX, top: rect.top + window.scrollY };
			},

			offsetParent() {
				let el = host.offsetParent || document.documentElement;
				while (el && el !== document.documentElement && window.getComputedStyle(el).position === 'static') {
					el = el.offsetParent;
				}
				el = el || document.documentElement;
				return {
					length: el ? 1 : 0,
					0: el
				};
			},

			position() {
				return {
					left: host.offsetLeft,
					top: host.offsetTop
				};
			},

			width() {
				return host.getBoundingClientRect().width;
			},
			height() {
				return host.getBoundingClientRect().height;
			},

			is(selector) {
				if (selector === ':visible') {
					return host.style.display !== 'none' && host.offsetParent !== null;
				}
				return host.matches(selector);
			},

			show() {
				host.style.display = '';
				component._fixPositionOverflow();
				return proxy;
			},
			hide() {
				host.style.display = 'none';
				return proxy;
			},
			toggle() {
				if (host.style.display === 'none') {
					host.style.display = '';
					component._fixPositionOverflow();
				} else {
					host.style.display = 'none';
				}
				return proxy;
			},

			parent() {
				return {
					length: host.parentNode ? 1 : 0,
					append(child) {
						host.parentNode?.appendChild(child._host || child[0] || child);
					}
				};
			},

			detach() {
				if (host.parentNode) host.parentNode.removeChild(host);
				return proxy;
			},

			appendTo(target) {
				const t = typeof target === 'string' ? document.querySelector(target) : target[0] || target;
				if (t) t.appendChild(host);
				return proxy;
			},

			trigger(eventName) {
				host.dispatchEvent(new CustomEvent(eventName, { bubbles: true }));
				return proxy;
			},

			find(selector) {
				const root = host.shadowRoot || host;
				const results = root.querySelectorAll(selector);
				const wrapper = {
					length: results.length,
					0: results[0],
					each(fn) {
						results.forEach((el, i) => fn.call(el, i, el));
						return wrapper;
					},
					click(fn) {
						results.forEach(el => el.addEventListener('click', fn));
						return wrapper;
					},
					text(val) {
						if (val === undefined) return results[0]?.textContent || '';
						results.forEach(el => {
							el.textContent = val;
						});
						return wrapper;
					},
					show() {
						results.forEach(el => {
							el.style.display = '';
						});
						return wrapper;
					},
					hide() {
						results.forEach(el => {
							el.style.display = 'none';
						});
						return wrapper;
					},
					css(prop, value) {
						if (typeof prop === 'object') {
							results.forEach(el => {
								for (const [k, v] of Object.entries(prop)) {
									el.style[k] = typeof v === 'number' && !CSS_NUMBER[k] ? v + 'px' : String(v);
								}
							});
							return wrapper;
						}
						if (value === undefined) {
							return results[0]
								? window
										.getComputedStyle(results[0])
										.getPropertyValue(prop.replace(/([A-Z])/g, '-$1').toLowerCase())
								: '';
						}
						results.forEach(el => {
							el.style[prop] =
								typeof value === 'number' && !CSS_NUMBER[prop] ? value + 'px' : String(value);
						});
						return wrapper;
					},
					height() {
						return results[0] ? results[0].getBoundingClientRect().height : 0;
					},
					width() {
						return results[0] ? results[0].getBoundingClientRect().width : 0;
					}
				};
				return wrapper;
			}
		};
		this.ui = proxy;
	}
}

export default GUIComponent;
