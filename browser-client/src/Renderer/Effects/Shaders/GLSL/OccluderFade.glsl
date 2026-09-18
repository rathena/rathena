// Fades world geometry lying between the camera and the followed entity
// (third person camera). Included by the model fragment shaders.
//
// uOccluderFadeMode:
//   0 - disabled
//   1 - dither (screen-door, single opaque pass)
//   2 - alpha, opaque pass: discard fragments inside the fade capsule
//   3 - alpha, blend pass: draw only fragments inside the fade capsule, translucent
//   4 - line of sight query: keep only fragments inside the (narrow) capsule,
//       used with an occlusion query to detect geometry covering the entity

uniform int   uOccluderFadeMode;
uniform vec3  uOccluderFadeEye;
uniform vec3  uOccluderFadeFocus;
uniform float uOccluderFadeRadius;
uniform float uOccluderFadeOpacity;
uniform float uOccluderFadeStrength;

const float OCCLUDER_FADE_BAYER[16] = float[16](
	 0.0,  8.0,  2.0, 10.0,
	12.0,  4.0, 14.0,  6.0,
	 3.0, 11.0,  1.0,  9.0,
	15.0,  7.0, 13.0,  5.0
);

// The faded region is a cylinder of constant radius from the eye to the focus,
// cut by a vertical plane through the focus (so a raised camera never fades
// anything behind the player); it closes over this distance (cells) before it.
const float OCCLUDER_FADE_END = 0.5;

// x: distance to the eye->focus axis,
// y: signed horizontal distance past the focus, along the view direction
vec2 occluderFadeCylinder(vec3 worldPos) {
	vec3 axis = uOccluderFadeFocus - uOccluderFadeEye;
	vec3 rel  = worldPos - uOccluderFadeEye;
	float len  = max(length(axis), 0.01);
	float t    = clamp(dot(rel, axis) / (len * len), 0.0, 1.0);
	// cut plane normal: horizontal view direction, tilting back to the view
	// axis as the camera gets steep (a vertical plane is meaningless top-down)
	vec3 level = vec3(axis.x, 0.0, axis.z);
	float flatness = length(level) / len;
	vec3 hdir  = level / max(length(level), 0.01);
	vec3 cut   = normalize(mix(axis / len, hdir, smoothstep(0.3, 0.6, flatness)));
	return vec2(length(rel - axis * t), dot(worldPos - uOccluderFadeFocus, cut));
}

// 0.0 = untouched, 1.0 = fully inside the cylinder between eye and focus
float occluderFadeAmount(vec3 worldPos) {
	vec2 c = occluderFadeCylinder(worldPos);
	float radial = 1.0 - smoothstep(uOccluderFadeRadius * 0.5, uOccluderFadeRadius, c.x);
	float along  = 1.0 - smoothstep(-OCCLUDER_FADE_END, 0.0, c.y);
	return radial * along * uOccluderFadeStrength;
}

// Applies the fade to the fragment alpha. Returns false when the fragment must be discarded.
bool occluderFade(vec3 worldPos, inout float alpha) {
	if (uOccluderFadeMode == 0) {
		return true;
	}

	if (uOccluderFadeMode == 4) {
		vec2 c = occluderFadeCylinder(worldPos);
		// stop short of the focus so the floor under the entity does not count
		return c.x < uOccluderFadeRadius && c.y < -0.5;
	}

	float fade = occluderFadeAmount(worldPos);
	float visibility = 1.0 - fade * (1.0 - uOccluderFadeOpacity);

	if (uOccluderFadeMode == 1) {
		ivec2 p = ivec2(gl_FragCoord.xy) & 3;
		float threshold = (OCCLUDER_FADE_BAYER[p.x + p.y * 4] + 0.5) / 16.0;
		return visibility > threshold;
	}

	if (uOccluderFadeMode == 2) {
		return fade < 0.01;
	}

	if (fade < 0.01) {
		return false;
	}
	alpha *= visibility;
	return true;
}
