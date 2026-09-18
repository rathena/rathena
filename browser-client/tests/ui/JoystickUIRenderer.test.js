import { afterEach, describe, expect, it, vi } from 'vitest';

const mocks = vi.hoisted(() => ({
	controls: {
		joyAutoHide: true
	},
	inputService: {
		active: true
	}
}));

vi.mock('UI/Components/ShortCut/ShortCut.js', () => ({ default: {} }));
vi.mock('UI/Components/Inventory/Inventory.js', () => ({ default: {} }));
vi.mock('UI/Components/JoystickUI/JoystickSetManager.js', () => ({ default: {} }));
vi.mock('DB/DBManager.js', () => ({ default: {} }));
vi.mock('Core/Client.js', () => ({ default: {} }));
vi.mock('Preferences/Controls.js', () => ({ default: mocks.controls }));
vi.mock('DB/Items/ItemType.js', () => ({ default: {} }));
vi.mock('UI/Components/JoystickUI/JoystickShortcutMapper.js', () => ({ default: {} }));
vi.mock('UI/Components/JoystickUI/JoystickInputService.js', () => ({ default: mocks.inputService }));
vi.mock('DB/Skills/SkillInfo.js', () => ({ default: {} }));

const { default: JoystickUIRenderer } = await import('UI/Components/JoystickUI/JoystickUIRenderer.js');

function createUI() {
	const host = document.createElement('div');
	host.style.display = 'block';

	return {
		0: host,
		show: vi.fn(() => {
			host.style.display = 'block';
		}),
		hide: vi.fn(() => {
			host.style.display = 'none';
		})
	};
}

describe('JoystickUIRenderer mouse auto-hide', () => {
	afterEach(() => {
		JoystickUIRenderer.dispose();
	});

	it('hides on physical mouse movement after attach and reattach', () => {
		const firstUI = createUI();
		JoystickUIRenderer.attach(firstUI);
		mocks.inputService.active = true;

		document.dispatchEvent(new MouseEvent('mousemove', { clientX: 10, clientY: 0 }));

		expect(firstUI.hide).toHaveBeenCalledOnce();
		expect(mocks.inputService.active).toBe(false);

		JoystickUIRenderer.dispose();

		const restoredUI = createUI();
		JoystickUIRenderer.attach(restoredUI);
		mocks.inputService.active = true;

		document.dispatchEvent(new MouseEvent('mousemove', { clientX: 0, clientY: 10 }));

		expect(restoredUI.hide).toHaveBeenCalledOnce();
		expect(mocks.inputService.active).toBe(false);
	});

	it('ignores mousemove and show/hide after dispose', () => {
		const firstUI = createUI();
		JoystickUIRenderer.attach(firstUI);

		JoystickUIRenderer.dispose();
		firstUI.show.mockClear();
		firstUI.hide.mockClear();

		document.dispatchEvent(new MouseEvent('mousemove', { clientX: 10, clientY: 10 }));
		JoystickUIRenderer.show();
		JoystickUIRenderer.hide();

		expect(firstUI.show).not.toHaveBeenCalled();
		expect(firstUI.hide).not.toHaveBeenCalled();
	});
});
