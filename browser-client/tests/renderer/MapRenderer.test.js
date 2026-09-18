import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';

const mocks = vi.hoisted(() => ({
	soundManager: { stop: vi.fn() },
	bgm: { stop: vi.fn() },
	uiManager: { removeComponents: vi.fn() },
	background: {
		remove: vi.fn(callback => callback()),
		setLoading: vi.fn()
	},
	cursor: {
		ACTION: { DEFAULT: 0 },
		setType: vi.fn()
	},
	mouse: { intersect: true },
	renderer: {
		stop: vi.fn(),
		getContext: vi.fn(() => ({})),
		render: vi.fn()
	},
	entityManager: { free: vi.fn() },
	effectManager: { free: vi.fn() },
	sky: { setUpCloudData: vi.fn() },
	damage: { free: vi.fn() },
	joystickUI: { onRestore: vi.fn() }
}));

vi.mock('Core/Thread.js', () => ({ default: {} }));
vi.mock('Audio/SoundManager.js', () => ({ default: mocks.soundManager }));
vi.mock('Audio/BGM.js', () => ({ default: mocks.bgm }));
vi.mock('DB/DBManager.js', () => ({ default: {} }));
vi.mock('UI/UIManager.js', () => ({ default: mocks.uiManager }));
vi.mock('UI/Background.js', () => ({ default: mocks.background }));
vi.mock('UI/CursorManager.js', () => ({ default: mocks.cursor }));
vi.mock('Engine/SessionStorage.js', () => ({ default: {} }));
vi.mock('Core/MemoryManager.js', () => ({ default: {} }));
vi.mock('Controls/MouseEventHandler.js', () => ({ default: mocks.mouse }));
vi.mock('Renderer/Renderer.js', () => ({ default: mocks.renderer }));
vi.mock('Renderer/Camera.js', () => ({ default: {} }));
vi.mock('Renderer/EntityManager.js', () => ({ default: mocks.entityManager }));
vi.mock('Renderer/Map/GridSelector.js', () => ({ default: {} }));
vi.mock('Renderer/Map/Ground.js', () => ({ default: {} }));
vi.mock('Renderer/Map/Altitude.js', () => ({ default: {} }));
vi.mock('Renderer/Map/Water.js', () => ({ default: {} }));
vi.mock('Renderer/Map/Models.js', () => ({ default: {} }));
vi.mock('Renderer/Map/AnimatedModels.js', () => ({ default: {} }));
vi.mock('Renderer/Map/OccluderFade.js', () => ({ default: {} }));
vi.mock('Renderer/GR2/GR2ModelRenderer.js', () => ({ default: {} }));
vi.mock('Renderer/Map/Sounds.js', () => ({ default: {} }));
vi.mock('Renderer/Map/Effects.js', () => ({ default: {} }));
vi.mock('Renderer/SpriteRenderer.js', () => ({ default: {} }));
vi.mock('Renderer/EffectManager.js', () => ({ default: mocks.effectManager }));
vi.mock('Renderer/SignboardManager.js', () => ({ default: {} }));
vi.mock('Renderer/ScreenEffectManager.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Sky.js', () => ({ default: mocks.sky }));
vi.mock('Renderer/Effects/Damage.js', () => ({ default: mocks.damage }));
vi.mock('Preferences/Graphics.js', () => ({ default: {} }));
vi.mock('Preferences/Map.js', () => ({ default: { useFog: true } }));
vi.mock('Utils/gl-matrix.js', () => ({ default: { mat4: {} } }));
vi.mock('Network/PacketVerManager.js', () => ({ default: {} }));
vi.mock('UI/Components/JoystickUI/JoystickUI.js', () => ({ default: mocks.joystickUI }));
vi.mock('Renderer/Effects/PostProcess.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/Bloom.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/VerticalFlip.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/GaussianBlur.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/CAS.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/FXAA.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/Vibrance.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/Cartoon.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/Blind.js', () => ({ default: {} }));
vi.mock('Renderer/Effects/Shaders/Upsampling.js', () => ({ default: {} }));
vi.mock('Utils/WebGL.js', () => ({ default: {} }));

const { default: MapRenderer } = await import('Renderer/MapRenderer.js');

describe('MapRenderer joystick restoration', () => {
	let originalOnLoad;

	beforeEach(() => {
		vi.clearAllMocks();
		originalOnLoad = MapRenderer.onLoad;
		MapRenderer.loading = false;
		MapRenderer.currentMap = 'prontera.gat';
		MapRenderer.onLoad = vi.fn();
		mocks.mouse.intersect = true;
	});

	afterEach(() => {
		MapRenderer.onLoad = originalOnLoad;
	});

	it('restores joystick polling before re-appending UI after a same-map teleport', () => {
		MapRenderer.setMap('prontera.gat');

		expect(mocks.uiManager.removeComponents).toHaveBeenCalledOnce();
		expect(mocks.joystickUI.onRestore).toHaveBeenCalledOnce();
		expect(MapRenderer.onLoad).toHaveBeenCalledOnce();
		expect(mocks.joystickUI.onRestore.mock.invocationCallOrder[0]).toBeLessThan(
			MapRenderer.onLoad.mock.invocationCallOrder[0]
		);
	});

	it('leaves different-map restoration to the map completion lifecycle', () => {
		MapRenderer.setMap('geffen.gat');

		expect(mocks.background.setLoading).toHaveBeenCalledOnce();
		expect(mocks.joystickUI.onRestore).not.toHaveBeenCalled();
		expect(MapRenderer.onLoad).not.toHaveBeenCalled();
		expect(MapRenderer.loading).toBe(true);
		expect(MapRenderer.currentMap).toBe('geffen.gat');
	});
});
