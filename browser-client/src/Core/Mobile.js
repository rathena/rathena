/**
 * Core/Mobile.js
 *
 * Help to handle touch devices
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */
// TODO: resize event on mobile keyboard bug
// TODO: body overflow
// TODO: responsive design

/**
 * Import dependencies
 */
import Context from 'Core/Context.js';
import Events from 'Core/Events.js';
import Camera from 'Renderer/Camera.js';
import Session from 'Engine/SessionStorage.js';
import Mouse from 'Controls/MouseEventHandler.js';
import KEYS from 'Controls/KeyEventHandler.js';
import MobileUI from 'UI/Components/MobileUI/MobileUI.js';

/**
 * @var {boolean} is doing a gesture ?
 */
let _processGesture = false;

/**
 * @var {number} save angle and scale value
 */
let _scale, _angle, _touches, _intersect;

/**
 * Timer to detect delayed click
 */
let _timer = -1;

/**
 * @var {boolean} current touch sequence started on an interactive UI element
 * (owned by the UI until every finger is lifted)
 */
let _uiTouch = false;

/**
 * @var {boolean} the page itself is zoomed in (browser pinch or input focus zoom):
 * touches are left to the browser so the user can pinch the page back out
 */
let _pageZoomed = false;

/**
 * Viewport meta applied when the host page doesn't define one: `width=device-width` keeps
 * mobile browsers from laying the page out at desktop width (and zooming into focused
 * inputs to compensate), `maximum-scale=1` suppresses the input focus zoom on iOS WebKit.
 */
const VIEWPORT_META = 'width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no';

/**
 * Elements that must receive the tap themselves (as synthesized mouse
 * events) instead of being treated as a click on the map.
 */
const UI_TOUCH_SELECTOR =
	'input, textarea, select, button, a, label, [contenteditable], ui-button, [data-background], [data-hover], [data-down], .event_add_cursor, td.tab, .draggable';

/**
 * Does the touch land on an interactive UI element (walking through Shadow DOM) ?
 *
 * @param {TouchEvent} event
 * @return {boolean}
 */
function isUITouch(event) {
	const path = event.composedPath ? event.composedPath() : [event.target];

	for (const node of path) {
		if (!(node instanceof Element)) {
			continue;
		}

		if (node instanceof HTMLCanvasElement) {
			return false;
		}

		if (node.matches(UI_TOUCH_SELECTOR)) {
			return true;
		}
	}

	return false;
}

/**
 * Make sure the document has a viewport meta (see VIEWPORT_META).
 */
function ensureViewportMeta() {
	if (!document.head || document.head.querySelector('meta[name="viewport"]')) {
		return;
	}

	const meta = document.createElement('meta');
	meta.name = 'viewport';
	meta.content = VIEWPORT_META;
	document.head.appendChild(meta);
}

/**
 * Track the browser page zoom (visual viewport smaller than the layout viewport).
 * While zoomed, the canvas gets back its native touch handling (see `body.ro-page-zoomed`
 * in UI/Common.css) and our touch controls step aside.
 */
function onVisualViewportResize() {
	_pageZoomed = window.visualViewport.scale > 1.01;
	document.body.classList.toggle('ro-page-zoomed', _pageZoomed);
}

/**
 * @namespace Mobile
 */
class Mobile {
	/**
	 * Initialize
	 */
	static init() {}
}

/**
 * Return distance between touches
 *
 * @param {TouchList} touches
 * @return {number} distance
 */
function touchDistance(touches) {
	const x = touches[0].pageX - touches[1].pageX;
	const y = touches[0].pageY - touches[1].pageY;

	return Math.sqrt(x * x + y * y);
}

/**
 * Get angle from touches
 *
 * @param {TouchList} touches
 * @return {number} rotation angle
 */
function touchAngle(touches) {
	const x = touches[0].pageX - touches[1].pageX;
	const y = touches[0].pageY - touches[1].pageY;

	return (Math.atan2(y, x) * 180) / Math.PI;
}

/**
 * Get translation size (width)
 *
 * @param {TouchList} old touches
 * @param {TouchList} new touches
 */
function touchTranslationX(oldTouches, touches) {
	const x1 = touches[0].pageX - oldTouches[0].pageX;
	const x2 = touches[1].pageX - oldTouches[1].pageX;

	if (
		x1 &&
		x2 && // need a direction
		x1 < 0 === x2 < 0 && // same direction
		Math.abs(1 - x1 / x2) < 0.25 // need a coordinate movement
	) {
		return (x1 + x2) >> 1;
	}

	return 0;
}

/**
 * Get translation size (height)
 *
 * @param {TouchList} old touches
 * @param {TouchList} new touches
 */
function touchTranslationY(oldTouches, touches) {
	const y1 = touches[0].pageY - oldTouches[0].pageY;
	const y2 = touches[1].pageY - oldTouches[1].pageY;

	if (
		y1 &&
		y2 && // need a direction
		y1 < 0 === y2 < 0 && // same direction
		Math.abs(1 - y1 / y2) < 0.25 // need a coordinate movement
	) {
		return (y1 + y2) >> 1;
	}

	return 0;
}

/**
 * Delayed tap on the map: only dispatched when no gesture started meanwhile
 */
const delayedClick = () => {
	if (_processGesture) {
		return;
	}

	_timer = -1;

	if (Mobile.onTouchStart) {
		Mobile.onTouchStart();
	}

	if (!_intersect && Mobile.onTouchEnd) {
		Mobile.onTouchEnd();
	}

	Mouse.intersect = _intersect;
};

/**
 * Start touching the screen
 * Process gesture, or action
 */
const onTouchStart = event => {
	_touches = event.touches;

	// Let the browser deliver the tap to the UI element as mouse events
	// (mouseenter/mousedown/click), exactly like a mouse would do.
	// Extra fingers landing during a UI touch stay with the UI too.
	if (_pageZoomed) {
		_uiTouch = true;
	} else if (_touches.length === 1) {
		_uiTouch = isUITouch(event);
	}
	if (_uiTouch) {
		if (_timer > -1) {
			Events.clearTimeout(_timer);
			_timer = -1;
		}
		return;
	}

	event.preventDefault();
	event.stopImmediatePropagation();

	// Delayed click (to detect gesture)
	if (_timer > -1) {
		Events.clearTimeout(_timer);
		_timer = -1;
	}

	// Gesture
	if (_touches.length > 1) {
		_scale = touchDistance(_touches);
		_angle = touchAngle(_touches);
		_processGesture = true;
		return;
	}

	Mouse.screen.x = _touches[0].pageX;
	Mouse.screen.y = _touches[0].pageY;

	if (!Session.FreezeUI) {
		Mouse.intersect = true;
		_intersect = true;
	}

	_timer = Events.setTimeout(delayedClick, 200);
};

/**
 * Hook touch end to know when a gesture end
 * process OnMouseUp if no gesture detected
 */
function onTouchEnd(event) {
	if (_uiTouch) {
		if (event.touches.length === 0) {
			_uiTouch = false;
		}
		return;
	}

	if (_processGesture) {
		_processGesture = false;
		KEYS.SHIFT = false;
		Camera.rotate(false);
		return;
	}

	if (_timer > -1) {
		_intersect = false;
		return;
	}

	if (Mobile.onTouchEnd) {
		Mobile.onTouchEnd();
	}

	Mouse.intersect = false;
}

/**
 * The browser aborted the touch sequence: drop any pending tap or gesture
 * without acting on the map.
 */
function onTouchCancel(event) {
	// A cancelled gesture must end even if a finger remains on screen
	if (event.touches.length > 0 && !_processGesture) {
		return;
	}

	if (_uiTouch) {
		_uiTouch = false;
	} else if (_processGesture) {
		_processGesture = false;
		KEYS.SHIFT = false;
		Camera.rotate(false);
	} else if (_timer > -1) {
		Events.clearTimeout(_timer);
		_timer = -1;
	} else if (Mobile.onTouchEnd) {
		// Map press already dispatched: release it so walking stops
		Mobile.onTouchEnd();
	}

	_intersect = false;
	Mouse.intersect = false;
}

/**
 * Process gesture (scale, rotate)
 * Else move.
 */
function onTouchMove(event) {
	event.stopImmediatePropagation();

	const touches = event.touches;

	Mouse.screen.x = touches[0].pageX;
	Mouse.screen.y = touches[0].pageY;

	// Not in gesture, just process
	if (!_processGesture) {
		return;
	}

	const scale = touchDistance(touches) - _scale;
	//var angle = touchAngle(touches) / _angle;
	const x = Math.abs(touchTranslationX(_touches, touches));
	const y = Math.abs(touchTranslationY(_touches, touches));

	if (!Camera.action.active && (x > 10 || y > 10)) {
		KEYS.SHIFT = y > x;
		Camera.rotate(true);
		return;
	}

	// Process zoom
	if (Math.abs(scale) > 10) {
		Camera.zoomFinal -= scale * 0.1;
		Camera.zoomFinal = Math.min(
			Camera.zoomFinal,
			Math.abs(Camera.altitudeTo - Camera.altitudeFrom) * Camera.MAX_ZOOM
		);
		Camera.zoomFinal = Math.max(Camera.zoomFinal, 2.0);
	}
}

// Add full screen on mobile (sux to have the browser title bar)
if (Math.max(screen.availHeight, screen.availWidth) <= 800) {
	// Fullscreen on action
	window.addEventListener('touchstart', () => {
		if (!Context.isFullScreen()) {
			Context.requestFullScreen();
		}
	});
}

//Add mobile UI on touch
function touchDevice() {
	Session.isTouchDevice = true;

	if (Session.Playing) {
		//Already playing, don't wait for map change, just show it
		MobileUI.show();
	}
}
window.addEventListener('touchstart', touchDevice, { once: true });

ensureViewportMeta();

if (window.visualViewport) {
	window.visualViewport.addEventListener('resize', onVisualViewportResize);
	onVisualViewportResize();
}

// Touch controls
window.addEventListener('touchstart', onTouchStart, { passive: false });
window.addEventListener('touchend', onTouchEnd);
window.addEventListener('touchcancel', onTouchCancel);
window.addEventListener('touchmove', onTouchMove);

/**
 * Export
 */
export default Mobile;
