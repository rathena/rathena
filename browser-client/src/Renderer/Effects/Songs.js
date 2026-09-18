import * as Tiles from 'Renderer/Effects/Tiles.js';
import FlatColorTile from 'Renderer/Effects/FlatColorTile.js';

export const DissonanceEffects = [
	FlatColorTile('dissonance', {
		r: 0xff / 255,
		g: 0xff / 255,
		b: 0xff / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/lens_w.bmp', 0.5, 0.7)
];

export const LullabyEffects = [
	FlatColorTile('lullaby', {
		r: 0xed / 255,
		g: 0x9e / 255,
		b: 0xff / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/zz.bmp', 0.5, 0.7)
];

export const MrKimEffects = [
	FlatColorTile('mrkim', {
		r: 0xfc / 255,
		g: 0xc7 / 255,
		b: 0xc7 / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/pocket.bmp', 0.5, 0.7)
];

export const EtChaosEffects = [
	FlatColorTile('etchaos', {
		r: 0x80 / 255,
		g: 0xff / 255,
		b: 0xc2 / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/lens_g.bmp', 0.5, 0.7)
];

export const DrumEffects = [
	FlatColorTile('drum', {
		r: 0xed / 255,
		g: 0x65 / 255,
		b: 0xfc / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/melody_b.bmp', 0.5, 0.7)
];

export const NibelungEffects = [
	FlatColorTile('nibelung', {
		r: 0x1c / 255,
		g: 0xec / 255,
		b: 0xff / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/twirl.bmp', 0.5, 0.7)
];

export const LokiEffects = [
	FlatColorTile('loki', {
		r: 0xdc / 255,
		g: 0x65 / 255,
		b: 0xfc / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/safeline.bmp', 0.5, 0.7)
];

export function AbyssEffects() {
	const gems = [
		'\xb7\xb9\xb5\xe5\xc1\xaa\xbd\xba\xc5\xe6', //red 레드젬스톤
		'\xbf\xbb\xb7\xce\xbf\xec\xc1\xaa\xbd\xba\xc5\xe6', //yellow 옐로우젬스톤
		'\xba\xed\xb7\xe7\xc1\xaa\xbd\xba\xc5\xe6'
	]; //blue 블루젬스톤

	return [
		FlatColorTile('abyss', {
			r: 0xff / 255,
			g: 0xff / 255,
			b: 0xff / 255,
			a: 0.05
		}),
		Tiles.HoveringTexture('data/texture/effect/' + gems[Math.round(Math.random() * 2)] + '.bmp', 0.5, 1)
	];
}

export const SiegfiedEffects = [
	FlatColorTile('siegfried', {
		r: 0x48 / 255,
		g: 0x3b / 255,
		b: 0xff / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/lens_b.bmp', 0.5, 0.7)
];

export const WhistleEffects = [
	FlatColorTile('whislte', {
		r: 0xff / 255,
		g: 0xc0 / 255,
		b: 0xcb / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/melody_b.bmp', 0.5, 0.7)
];

export const SinEffects = [
	FlatColorTile('sinsun', {
		r: 0xff / 255,
		g: 0.8,
		b: 0.85,
		a: 0.4
	}),
	Tiles.HoveringTexture('data/texture/effect/lens_r.bmp', 0.5, 0.7)
];

// We define random effects for Bragi at first execution
// this way is more efficient than randomize spells at each func call
const BragiRandNumbers = new Array(8 * 3); // 24 slots

for (let i = 0; i < BragiRandNumbers.length; ++i) {
	BragiRandNumbers[i] = Math.round(Math.random() * 7) + 1;
}

export function BragiSpellEffect(i) {
	const bragiSpell = Tiles.HoveringTexture(
		'data/texture/effect/spell_0' + BragiRandNumbers[i] + '.bmp',
		0.5,
		0.61,
		2,
		{ r: 0.61, g: 0.61, b: 1.0 }
	);

	return bragiSpell;
}

const BragiEffects = Array.from({ length: 24 }, (v, i) => BragiSpellEffect(i));

export const getBragiSpellNote = (function () {
	let i = 0;
	return function spellNote() {
		if (i >= BragiEffects.length) {
			i = 0;
		}
		return BragiEffects[i++];
	};
})();

export const AppleEffects = [
	FlatColorTile('apple', {
		r: 0xff / 255,
		g: 0xff / 255,
		b: 0x00 / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/idun_apple.bmp', 1, 0.7)
];

export const UglyEffects = [
	FlatColorTile('ugly', {
		r: 0xff / 255,
		g: 0xff / 255,
		b: 0xff / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/lens_w.bmp', 0.5, 0.7)
];

export const HummingEffects = [
	FlatColorTile('humming', {
		r: 0xe6 / 255,
		g: 0xd1 / 255,
		b: 0xd1 / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/melody_a.bmp', 0.5, 0.7)
];

export const ForgetEffects = [
	FlatColorTile('humming', {
		r: 0x1c / 255,
		g: 0xff / 255,
		b: 0x73 / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/lens_g.bmp', 0.5, 0.7)
];

export const FortuneEffects = [
	FlatColorTile('fortune', {
		r: 0xfc / 255,
		g: 0x6f / 255,
		b: 0x65 / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/heart_2.bmp', 0.5, 0.7)
];

export const ServiceEffects = [
	FlatColorTile('service', {
		r: 0xff / 255,
		g: 0x80 / 255,
		b: 0xb7 / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/safeline.bmp', 0.5, 0.7)
];

export const GospelEffects = [
	FlatColorTile('gospel', {
		r: 0xff / 255,
		g: 0xff / 255,
		b: 0xff / 255,
		a: 0.05
	}),
	Tiles.HoveringTexture('data/texture/effect/cross_old.bmp', 0.5, 0.7)
];

export const FogEffects = [
	FlatColorTile('fog', {
		r: 0xaa / 255,
		g: 0xaa / 255,
		b: 0xaa / 255,
		a: 0.6
	}),
	Tiles.HoveringTexture('data/texture/effect/lens_w.bmp', 0.5, 0.7)
];

export const GravityEffects = [
	FlatColorTile('gravity', {
		r: 0xff / 255,
		g: 0xff / 255,
		b: 0xff / 255,
		a: 0.2
	}),
	Tiles.HoveringTexture('data/texture/effect/lens_w.bmp', 0.5, 0.7)
];

export const EvillandEffects = [
	FlatColorTile('gray', {
		r: 0xa0 / 255,
		g: 0xa0 / 255,
		b: 0xa0 / 255,
		a: 0.2
	}),
	Tiles.HoveringTexture('data/texture/effect/curse.bmp', 1, 0.7)
];

export default {
	DissonanceEffects,
	LullabyEffects,
	MrKimEffects,
	EtChaosEffects,
	DrumEffects,
	NibelungEffects,
	LokiEffects,
	AbyssEffects,
	SiegfiedEffects,
	WhistleEffects,
	SinEffects,
	BragiSpellEffect,
	getBragiSpellNote,
	AppleEffects,
	UglyEffects,
	HummingEffects,
	ForgetEffects,
	FortuneEffects,
	ServiceEffects,
	GospelEffects,
	FogEffects,
	GravityEffects,
	EvillandEffects
};
