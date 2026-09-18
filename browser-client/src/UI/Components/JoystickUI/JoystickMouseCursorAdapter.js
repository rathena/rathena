/**
 * UI/Components/JoystickUI/JoystickMouseCursorAdapter.js
 *
 * Adapts joystick input to emulate mouse behavior.
 * Handles cursor movement, coordinate projection from 3D world to 2D screen,
 * UI element navigation, and clicking events.
 *
 * @author AoShinHo
 */

import Renderer from 'Renderer/Renderer.js';
import Mouse from 'Controls/MouseEventHandler.js';
import glMatrix from 'Vendors/gl-matrix.js';
import Camera from 'Renderer/Camera.js';
import ControlsSettings from 'Preferences/Controls.js';
import Interaction from './JoystickInteractionService.js';

function move(dx, dy) {
	Mouse.screen.x = Math.max(0, Math.min(Renderer.width, Mouse.screen.x + dx * ControlsSettings.joySense));
	Mouse.screen.y = Math.max(0, Math.min(Renderer.height, Mouse.screen.y + dy * ControlsSettings.joySense));

	const cursor = document.querySelector('.cursor');
	if (cursor) {
		cursor.style.left = Mouse.screen.x + 'px';
		cursor.style.top = Mouse.screen.y + 'px';
	}
}

function moveMouseToEntity(entity) {
	if (!entity || !entity.position) {
		return;
	}

	const mat4 = glMatrix.mat4;
	const vec4 = glMatrix.vec4;

	const _matrix = mat4.create();
	const _vector = vec4.create();
	const _pos = vec4.create();

	// Transform entity position to screen coordinates
	_vector[0] = entity.position[0] + 0.5;
	_vector[1] = -entity.position[2];
	_vector[2] = entity.position[1] + 0.5;
	_vector[3] = 1.0;

	// Apply camera transformation
	mat4.translate(_matrix, Camera.modelView, _vector);

	// Set up billboard matrix (like in EntityRender)
	_matrix[0] = 1.0;
	_matrix[1] = 0.0;
	_matrix[2] = 0.0;
	_matrix[4] = 0.0;
	_matrix[5] = 1.0;
	_matrix[6] = 0.0;
	_matrix[8] = 0.0;
	_matrix[9] = 0.0;
	_matrix[10] = 1.0;

	// Project to screen
	mat4.multiply(_matrix, Camera.projection, _matrix);

	// Cast position for projection
	_pos[0] = 0.0;
	_pos[1] = 0.0;
	_pos[2] = 0.0;
	_pos[3] = 1.0;

	// Project point to scene
	vec4.transformMat4(_pos, _pos, _matrix);

	// Calculate screen position
	const z = _pos[3] === 0.0 ? 1.0 : 1.0 / _pos[3];
	const screenX = Renderer.width / 2 + Math.round((Renderer.width / 2) * (_pos[0] * z));
	let screenY = Renderer.height / 2 - Math.round((Renderer.height / 2) * (_pos[1] * z));

	screenY = screenY - 13;

	// Update mouse position
	Mouse.screen.x = screenX;
	Mouse.screen.y = screenY;

	// Update cursor visual position
	const _selector = document.querySelector('.cursor');
	if (_selector) {
		_selector.style.left = screenX + 'px';
		_selector.style.top = screenY + 'px';
	}
}

function leftClick(click = false) {
	const el = document.elementFromPoint(Mouse.screen.x, Mouse.screen.y);
	if (!el) {
		handleWorldLeftClick();
		return;
	}

	if (ControlsSettings.joyDisableVirtualMouse) {
		return;
	}

	const eventOptions = {
		bubbles: true,
		cancelable: true,
		view: window,
		clientX: Mouse.screen.x,
		clientY: Mouse.screen.y,
		which: 1
	};
	el.dispatchEvent(new MouseEvent('mousedown', eventOptions));
	setTimeout(function () {
		el.dispatchEvent(new MouseEvent('mouseup', eventOptions));
		if (click) {
			el.dispatchEvent(new MouseEvent('click', eventOptions));
		}
	}, 50);
}

function rightClick(holding = false) {
	const el = document.elementFromPoint(Mouse.screen.x, Mouse.screen.y);
	const isCanvas = el && el.tagName.toLowerCase() === 'canvas';
	if (!el || isCanvas) {
		handleWorldRightClick();
		return;
	}
	if (holding) {
		const draggableElement = el.closest('.item, .skill');
		if (draggableElement) {
			if (Interaction.openSelectionWindow(draggableElement)) {
				return;
			}
		}
	}

	if (ControlsSettings.joyDisableVirtualMouse) {
		return;
	}

	el.dispatchEvent(
		new MouseEvent('mousedown', {
			which: 3
		})
	);
	setTimeout(function () {
		el.dispatchEvent(
			new MouseEvent('mouseup', {
				which: 3
			})
		);
	}, 100);
}

function _dispatchMouseEvent(target, type, which) {
	target.dispatchEvent(
		new MouseEvent(type, {
			bubbles: true,
			cancelable: true,
			view: window,
			button: which === 3 ? 2 : 0,
			which: which
		})
	);
}

function handleWorldLeftClick() {
	if (!Mouse.intersect) {
		Mouse.intersect = true;
	}
	_dispatchMouseEvent(Renderer.canvas, 'mousedown', 1);

	setTimeout(function () {
		_dispatchMouseEvent(Renderer.canvas, 'mouseup', 1);
	}, 100);
}

function handleWorldRightClick() {
	if (!Mouse.intersect) {
		Mouse.intersect = true;
	}
	_dispatchMouseEvent(Renderer.canvas, 'mousedown', 3);

	setTimeout(function () {
		_dispatchMouseEvent(Renderer.canvas, 'mouseup', 3);
	}, 100);
}

function changeCameraAngle(angle) {
	Camera.angleFinal[1] += angle;
	Camera.updateState();
	Camera.save();
}

function changeCameraZoom(zoom) {
	Camera.setZoom(zoom);
}

function _dispatchKeyEvent(target, type, which) {
	target.dispatchEvent(
		new KeyboardEvent(type, {
			bubbles: true,
			cancelable: true,
			which: which,
			keyCode: which
		})
	);
}

function esc() {
	_dispatchKeyEvent(document, 'keydown', 27); // Esc
}

function enter() {
	_dispatchKeyEvent(document, 'keydown', 13); // Enter
}

function contextMenu() {
	const el = document.elementFromPoint(Mouse.screen.x, Mouse.screen.y);
	const draggableElement = el.closest('.item, .skill');

	if (el && draggableElement) {
		const contextMenuEvent = new MouseEvent('contextmenu', {
			bubbles: true,
			cancelable: true,
			view: window,
			clientX: Mouse.screen.x,
			clientY: Mouse.screen.y,
			which: 3
		});

		el.dispatchEvent(contextMenuEvent);
		return true;
	}
	return false;
}

function navigateDraggableItems(direction) {
	const el = document.elementFromPoint(Mouse.screen.x, Mouse.screen.y);

	const container = el.closest('.item, .skill');

	if (!container) {
		// Fall back to regular arrow key navigation
		let keyCode;
		switch (direction) {
			case 'up':
				keyCode = 38;
				break;
			case 'down':
				keyCode = 40;
				break;
			case 'left':
				keyCode = 37;
				break;
			case 'right':
				keyCode = 39;
				break;
		}
		_dispatchKeyEvent(document, 'keydown', keyCode);
		return;
	}

	let allDraggables = Array.from(container.querySelectorAll('.item, .skill')).filter(
		item => !item.matches('.tabs button, .tab-btn')
	);

	if (allDraggables.length === 0) {
		allDraggables = Array.from(document.querySelectorAll('.item, .skill')).filter(
			item => item.offsetParent !== null && !item.matches('.tabs button, .tab-btn')
		);
	}

	// Check if we're in a skill container by looking for skill-specific structure
	const isSkillContainer =
		(container.id && container.id.indexOf('positionSkills') === 0) ||
		container.querySelector(
			'#positionSkills1, #positionSkills2, #positionSkills3, #positionSkills4, #positionSkills5'
		) ||
		container.closest('.skillCol') !== null;

	const draggableElement = container.closest('.item, .skill');

	const currentIndex = allDraggables.indexOf(draggableElement);
	let newIndex = currentIndex;

	// Use fixed grid width based on container type
	let GRID_WIDTH;
	if (isSkillContainer) {
		// Skills always use fixed 7-column grid
		GRID_WIDTH = 7;
	} else {
		// Items: calculate based on container width and icon size
		const containerWidth = container.clientWidth || 200;
		const iconElement = draggableElement.querySelector('.icon');
		const iconWidth = (iconElement && iconElement.clientWidth) || 24; // .icon has fixed 24px width
		const iconMargin = 4; // margin from CSS: margin: 4px 4px 4px 4px
		const totalIconWidth = iconWidth + iconMargin * 2;
		GRID_WIDTH = Math.max(6, Math.min(8, Math.floor(containerWidth / totalIconWidth)));
	}

	switch (direction) {
		case 'up':
			newIndex = currentIndex - GRID_WIDTH;
			break;
		case 'down':
			newIndex = currentIndex + GRID_WIDTH;
			break;
		case 'left':
			newIndex = currentIndex - 1;
			break;
		case 'right':
			newIndex = currentIndex + 1;
			break;
	}

	// Bounds checking
	newIndex = Math.max(0, Math.min(allDraggables.length - 1, newIndex));

	if (newIndex !== currentIndex && newIndex < allDraggables.length) {
		const targetElement = allDraggables[newIndex];
		const targetRect = targetElement.getBoundingClientRect();

		if (targetRect) {
			const targetCenterX = targetRect.left + targetRect.width / 2;
			const targetCenterY = targetRect.top + targetRect.height / 2;

			// Move mouse to target element
			Mouse.screen.x = targetCenterX;
			Mouse.screen.y = targetCenterY;

			const _selector = document.querySelector('.cursor');
			if (_selector) {
				_selector.style.left = targetCenterX + 'px';
				_selector.style.top = targetCenterY + 'px';
			}
		}
	}
}

function quickCastClick() {
	setTimeout(function () {
		_dispatchMouseEvent(Renderer.canvas, 'mousedown', 1);
		setTimeout(function () {
			_dispatchMouseEvent(Renderer.canvas, 'mouseup', 1);
		}, 100);
	}, 100);
}
export default {
	quickCastClick: quickCastClick,
	moveMouseToEntity: moveMouseToEntity,
	navigateDraggableItems: navigateDraggableItems,
	contextMenu: contextMenu,
	esc: esc,
	enter: enter,
	changeCameraZoom: changeCameraZoom,
	changeCameraAngle: changeCameraAngle,
	move: move,
	leftClick: leftClick,
	rightClick: rightClick
};
