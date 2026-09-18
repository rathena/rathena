import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';

const mocks = vi.hoisted(() => ({
	renderer: {
		show: vi.fn(),
		hide: vi.fn()
	},
	buttonInput: { update: vi.fn(() => false) },
	axisInput: { update: vi.fn(() => false) },
	controls: { joyDeadline: 0.5 }
}));

vi.mock('UI/Components/JoystickUI/JoystickButtonInput.js', () => ({ default: mocks.buttonInput }));
vi.mock('UI/Components/JoystickUI/JoystickAxisInput.js', () => ({ default: mocks.axisInput }));
vi.mock('UI/Components/JoystickUI/JoystickUIRenderer.js', () => ({ default: mocks.renderer }));
vi.mock('Preferences/Controls.js', () => ({ default: mocks.controls }));

const { default: JoystickInputService } = await import('UI/Components/JoystickUI/JoystickInputService.js');

describe('JoystickInputService gamepad connection lifecycle', () => {
	beforeEach(() => {
		JoystickInputService.dispose();
		vi.clearAllMocks();
		JoystickInputService.active = false;
		JoystickInputService.buttonStates = {};
	});

	afterEach(() => {
		JoystickInputService.dispose();
	});

	it('shows the joystick again after a disconnect/reconnect cycle', () => {
		JoystickInputService.prepare();

		window.dispatchEvent(new Event('gamepadconnected'));
		expect(mocks.renderer.show).toHaveBeenCalledTimes(1);
		expect(JoystickInputService.active).toBe(true);

		window.dispatchEvent(new Event('gamepaddisconnected'));
		expect(mocks.renderer.hide).toHaveBeenCalledTimes(1);
		expect(JoystickInputService.active).toBe(false);

		// Reconnect: the connection listener must survive the disconnect.
		window.dispatchEvent(new Event('gamepadconnected'));
		expect(mocks.renderer.show).toHaveBeenCalledTimes(2);
		expect(JoystickInputService.active).toBe(true);
	});

	it('does not register duplicate listeners when prepare runs twice', () => {
		JoystickInputService.prepare();
		JoystickInputService.prepare();

		window.dispatchEvent(new Event('gamepadconnected'));

		expect(mocks.renderer.show).toHaveBeenCalledTimes(1);
	});

	it('clears the pending hide timer on dispose', () => {
		vi.useFakeTimers();
		const gamepad = { buttons: [], axes: [] };
		navigator.getGamepads = vi.fn(() => [gamepad]);

		try {
			JoystickInputService.prepare();

			// Activity shows the HUD.
			mocks.buttonInput.update.mockReturnValueOnce(true);
			JoystickInputService.update();
			expect(mocks.renderer.show).toHaveBeenCalledTimes(1);

			// No activity schedules the 30s hide.
			JoystickInputService.update();

			// Dispose must cancel that timer so it can't hide a freshly re-prepared HUD.
			JoystickInputService.dispose();
			vi.advanceTimersByTime(30000);

			expect(mocks.renderer.hide).not.toHaveBeenCalled();
		} finally {
			vi.useRealTimers();
			delete navigator.getGamepads;
		}
	});

	it('stops reacting to gamepad events after dispose', () => {
		JoystickInputService.prepare();
		JoystickInputService.dispose();

		window.dispatchEvent(new Event('gamepadconnected'));
		window.dispatchEvent(new Event('gamepaddisconnected'));

		expect(mocks.renderer.show).not.toHaveBeenCalled();
		expect(mocks.renderer.hide).not.toHaveBeenCalled();
	});
});
