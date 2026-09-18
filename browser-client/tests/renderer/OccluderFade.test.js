import { beforeEach, describe, expect, it, vi } from 'vitest';

const camera = {
	state: 1,
	states: { isometric: 0, third_person: 1, first_person: 2 },
	focus: new Float32Array([1, 2, 3])
};
const graphics = { occluderFade: 'dither', occluderFadeOpacity: 0.25, occluderFadeRadius: 5 };
const runWithDepth = vi.fn((_test, _mask, _corr, fn) => fn());

vi.mock('Renderer/Effects/Shaders/GLSL/OccluderFade.glsl?raw', () => ({
	default: 'uniform int uOccluderFadeMode;'
}));
vi.mock('Renderer/Camera.js', () => ({ default: camera }));
vi.mock('Preferences/Graphics.js', () => ({ default: graphics }));
vi.mock('Renderer/SpriteRenderer.js', () => ({ default: { runWithDepth } }));

const { default: OccluderFade } = await import('Renderer/Map/OccluderFade.js');

const uniform = {
	uOccluderFadeMode: 'mode',
	uOccluderFadeEye: 'eye',
	uOccluderFadeFocus: 'focus',
	uOccluderFadeRadius: 'radius',
	uOccluderFadeOpacity: 'opacity'
};

const IDENTITY = new Float32Array([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);

function makeGl(queryResult = true) {
	return {
		SRC_ALPHA: 770,
		ONE_MINUS_SRC_ALPHA: 771,
		QUERY_RESULT_AVAILABLE: 1,
		QUERY_RESULT: 2,
		ANY_SAMPLES_PASSED_CONSERVATIVE: 3,
		uniform1i: vi.fn(),
		uniform1f: vi.fn(),
		uniform3fv: vi.fn(),
		blendFunc: vi.fn(),
		colorMask: vi.fn(),
		createQuery: vi.fn(() => ({})),
		deleteQuery: vi.fn(),
		beginQuery: vi.fn(),
		endQuery: vi.fn(),
		getQueryParameter: vi.fn((_q, p) => (p === 1 ? true : queryResult))
	};
}

/**
 * Run the query pass for both slots and collect the result on the next frame
 */
function frame(gl, tick) {
	OccluderFade.beginFrame(gl, IDENTITY, tick);
	OccluderFade.renderQuery(gl, uniform, vi.fn(), OccluderFade.QUERY.MODELS);
	OccluderFade.renderQuery(gl, uniform, vi.fn(), OccluderFade.QUERY.ANIMATED);
}

describe('Renderer/Map/OccluderFade', () => {
	beforeEach(() => {
		camera.state = camera.states.third_person;
		graphics.occluderFade = 'dither';
		runWithDepth.mockClear();
		OccluderFade.free(makeGl());
	});

	it('splices the shared GLSL at the include marker', () => {
		const out = OccluderFade.injectShader('a\n// #include OccluderFade.glsl\nb');
		expect(out).toBe('a\nuniform int uOccluderFadeMode;\nb');
	});

	it('is active with the setting on, except in first person', () => {
		expect(OccluderFade.isActive()).toBe(true);

		camera.state = camera.states.isometric;
		expect(OccluderFade.isActive()).toBe(true);

		camera.state = camera.states.first_person;
		expect(OccluderFade.isActive()).toBe(false);

		camera.state = camera.states.third_person;
		graphics.occluderFade = 'off';
		expect(OccluderFade.isActive()).toBe(false);
	});

	it('picks the opaque shader mode from the setting once the view is blocked', () => {
		expect(OccluderFade.opaqueMode()).toBe(OccluderFade.MODE.OFF);

		const gl = makeGl(true);
		frame(gl, 1000);
		frame(gl, 1100);
		frame(gl, 1200);
		expect(OccluderFade.getStrength()).toBe(1);

		expect(OccluderFade.opaqueMode()).toBe(OccluderFade.MODE.DITHER);
		expect(OccluderFade.needsBlendPass()).toBe(false);

		graphics.occluderFade = 'alpha';
		expect(OccluderFade.opaqueMode()).toBe(OccluderFade.MODE.ALPHA_OPAQUE);
		expect(OccluderFade.needsBlendPass()).toBe(true);

		camera.state = camera.states.first_person;
		expect(OccluderFade.opaqueMode()).toBe(OccluderFade.MODE.OFF);
		expect(OccluderFade.needsBlendPass()).toBe(false);
	});

	it('eases the fade in while occluded and out once the view clears', () => {
		const blocked = makeGl(true);
		frame(blocked, 1000);
		expect(OccluderFade.getStrength()).toBe(0);
		frame(blocked, 1075);
		expect(OccluderFade.getStrength()).toBeCloseTo(0.5);
		frame(blocked, 1150);
		expect(OccluderFade.getStrength()).toBe(1);

		const clear = makeGl(false);
		frame(clear, 1250);
		expect(OccluderFade.getStrength()).toBeCloseTo(2 / 3);
		frame(clear, 1350);
		expect(OccluderFade.getStrength()).toBeCloseTo(1 / 3);
		frame(clear, 1450);
		expect(OccluderFade.getStrength()).toBeCloseTo(0);
	});

	it('runs the line of sight pass without color/depth writes and one query in flight per slot', () => {
		const gl = makeGl(true);
		const draw = vi.fn();

		OccluderFade.beginFrame(gl, IDENTITY, 1000);
		OccluderFade.renderQuery(gl, uniform, draw, OccluderFade.QUERY.MODELS);
		expect(gl.beginQuery).toHaveBeenCalledWith(3, expect.anything());
		expect(gl.colorMask).toHaveBeenNthCalledWith(1, false, false, false, false);
		expect(gl.colorMask).toHaveBeenLastCalledWith(true, true, true, true);
		expect(runWithDepth).toHaveBeenLastCalledWith(false, false, true, expect.any(Function));
		expect(gl.uniform1i).toHaveBeenLastCalledWith('mode', OccluderFade.MODE.QUERY);
		expect(draw).toHaveBeenCalledTimes(1);

		OccluderFade.renderQuery(gl, uniform, draw, OccluderFade.QUERY.MODELS);
		expect(draw).toHaveBeenCalledTimes(1);

		camera.state = camera.states.first_person;
		OccluderFade.beginFrame(gl, IDENTITY, 1100);
		OccluderFade.renderQuery(gl, uniform, draw, OccluderFade.QUERY.MODELS);
		expect(draw).toHaveBeenCalledTimes(1);
	});

	it('uploads only the mode when off', () => {
		const gl = makeGl();
		OccluderFade.setUniforms(gl, uniform, OccluderFade.MODE.OFF);
		expect(gl.uniform1i).toHaveBeenCalledWith('mode', 0);
		expect(gl.uniform3fv).not.toHaveBeenCalled();
		expect(gl.uniform1f).not.toHaveBeenCalled();
	});

	it('uploads eye from the inverted model-view and focus from the camera', () => {
		const gl = makeGl();
		// translation by (-4, -5, -6): the eye sits at (4, 5, 6)
		const modelView = new Float32Array([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -4, -5, -6, 1]);
		OccluderFade.beginFrame(gl, modelView, 1000);
		OccluderFade.setUniforms(gl, uniform, OccluderFade.MODE.DITHER);

		const eye = gl.uniform3fv.mock.calls.find((c) => c[0] === 'eye')[1];
		expect(Array.from(eye)).toEqual([4, 5, 6]);
		expect(gl.uniform3fv).toHaveBeenCalledWith('focus', camera.focus);
		expect(gl.uniform1f).toHaveBeenCalledWith('opacity', 0.25);
		expect(gl.uniform1f).toHaveBeenCalledWith('radius', 5);
	});

	it('draws the opaque pass with depth writes and the blend pass without', () => {
		const gl = makeGl(true);
		const draw = vi.fn();
		frame(gl, 1000);
		frame(gl, 1100);
		frame(gl, 1200);
		runWithDepth.mockClear();

		OccluderFade.renderOpaque(gl, uniform, draw);
		expect(runWithDepth).toHaveBeenLastCalledWith(true, true, true, expect.any(Function));
		expect(gl.uniform1i).toHaveBeenLastCalledWith('mode', OccluderFade.MODE.DITHER);

		OccluderFade.renderBlend(gl, uniform, draw);
		expect(runWithDepth).toHaveBeenLastCalledWith(true, false, true, expect.any(Function));
		expect(gl.uniform1i).toHaveBeenLastCalledWith('mode', OccluderFade.MODE.ALPHA_BLEND);
		expect(gl.blendFunc).toHaveBeenCalledWith(770, 771);
		expect(draw).toHaveBeenCalledTimes(2);
	});
});
