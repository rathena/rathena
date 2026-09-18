/**
 * Loaders/Model.js
 *
 * Loaders for Gravity .rsm file (Resource Model)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import BinaryReader from 'Utils/BinaryReader.js';
import glMatrix from 'Utils/gl-matrix.js';

/**
 * Import
 */
const vec3 = glMatrix.vec3;
const mat3 = glMatrix.mat3;
const mat4 = glMatrix.mat4;

class Box {
	constructor() {
		this.max = vec3.fromValues(-Infinity, -Infinity, -Infinity);
		this.min = vec3.fromValues(Infinity, Infinity, Infinity);
		this.offset = vec3.create();
		this.range = vec3.create();
		this.center = vec3.create();
	}
}

class Node {
	/**
	 * Node Constructor
	 *
	 * @param {object} rsm
	 * @param {object} fp BinaryReader
	 * @param {boolean} only
	 */
	constructor(rsm, fp, only) {
		let i, j, k, count;
		const version = rsm.version;
		let posKeyframes, scaleKeyFrames, textureKeyFrameGroup;
		let textures = [];
		// Read initialised
		this.main = rsm;
		this.is_only = only;

		// Read name and parent name
		if (version >= 2.2) {
			this.name = fp.readBinaryString(fp.readLong());
			this.parentname = fp.readBinaryString(fp.readLong());
		} else {
			this.name = fp.readBinaryString(40);
			this.parentname = fp.readBinaryString(40);
		}

		// Read textures
		count = fp.readLong();
		textures = new Array(count);

		for (i = 0; i < count; i++) {
			textures[i] = version >= 2.3 ? fp.readBinaryString(fp.readLong()) : fp.readLong();
		}

		// Read options
		this.mat3 = [
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat()
		];
		this.offset = [fp.readFloat(), fp.readFloat(), fp.readFloat()];

		// Read position, rotation angle, rotation axis, and scale
		if (version >= 2.2) {
			this.pos = [0, 0, 0];
			this.rotangle = 0;
			this.rotaxis = [0, 0, 0];
			this.scale = [1, 1, 1];
			this.flip = [1, -1, 1];
		} else {
			this.pos = [fp.readFloat(), fp.readFloat(), fp.readFloat()];
			this.rotangle = fp.readFloat();
			this.rotaxis = [fp.readFloat(), fp.readFloat(), fp.readFloat()];
			this.scale = [fp.readFloat(), fp.readFloat(), fp.readFloat()];
			this.flip = [1, 1, 1];
		}

		// Read vertices
		count = fp.readLong();
		const vertices = new Array(count);

		for (i = 0; i < count; ++i) {
			vertices[i] = [fp.readFloat(), fp.readFloat(), fp.readFloat()];
		}

		// Read texture vertices
		count = fp.readLong();
		const tvertices = new Float32Array(count * 6);

		for (i = 0, j = 0; i < count; ++i, j += 6) {
			if (version >= 1.2) {
				tvertices[j + 0] = fp.readUByte() / 255;
				tvertices[j + 1] = fp.readUByte() / 255;
				tvertices[j + 2] = fp.readUByte() / 255;
				tvertices[j + 3] = fp.readUByte() / 255;
			}
			tvertices[j + 4] = fp.readFloat() * 0.98 + 0.01;
			tvertices[j + 5] = fp.readFloat() * 0.98 + 0.01;
		}

		// Read faces
		count = fp.readLong();
		const faces = new Array(count);

		for (i = 0; i < count; ++i) {
			let len = -1;
			if (version >= 2.2) {
				len = fp.readLong();
			}
			faces[i] = {
				vertidx: [fp.readUShort(), fp.readUShort(), fp.readUShort()],
				tvertidx: [fp.readUShort(), fp.readUShort(), fp.readUShort()],
				texid: fp.readUShort(),
				padding: fp.readUShort(),
				twoSide: fp.readLong()
			};

			if (version >= 1.2) {
				faces[i].smoothGroup = fp.readLong();
				if (len > 24) {
					faces[i].smoothGroup_1 = fp.readLong();
				}
				if (len > 28) {
					faces[i].smoothGroup_2 = fp.readLong();
				}
				if (len > 32) {
					fp.seek(len - 32, SEEK_CUR);
				}
			}
		}

		// Read scaleKeyFrame
		if (version >= 1.6) {
			count = fp.readLong();
			scaleKeyFrames = new Array(count);

			for (i = 0; i < count; i++) {
				scaleKeyFrames[i] = {
					Frame: fp.readLong(),
					Scale: [fp.readFloat(), fp.readFloat(), fp.readFloat()],
					Data: fp.readFloat()
				};
			}
		}

		// Read rotkeyframes
		count = fp.readLong();
		const rotKeyframes = new Array(count);

		for (i = 0; i < count; ++i) {
			rotKeyframes[i] = {
				frame: fp.readLong(),
				q: [fp.readFloat(), fp.readFloat(), fp.readFloat(), fp.readFloat()]
			};
		}

		// Read poskeyframes
		if (version >= 2.2) {
			count = fp.readLong();
			posKeyframes = new Array(count);

			for (i = 0; i < count; ++i) {
				posKeyframes[i] = {
					frame: fp.readLong(),
					px: fp.readFloat(),
					py: fp.readFloat(),
					pz: fp.readFloat(),
					Data: fp.readLong()
				};
			}
		}

		// Additional version 2.3 changes
		if (version >= 2.3) {
			count = fp.readLong();
			textureKeyFrameGroup = new Array(count);

			for (i = 0; i < count; ++i) {
				const textureId = fp.readLong();
				const amountTextureAnimations = fp.readLong();

				// Initialize textureKeyFrameGroup[i] if it doesn't exist
				if (!textureKeyFrameGroup[i]) {
					textureKeyFrameGroup[i] = [];
				}

				// Initialize textureKeyFrameGroup[i][textureId] if it doesn't exist
				if (!textureKeyFrameGroup[i][textureId]) {
					textureKeyFrameGroup[i][textureId] = [];
				}

				for (j = 0; j < amountTextureAnimations; ++j) {
					const type = fp.readLong();
					const amountFrames = fp.readLong();

					// Initialize textureKeyFrameGroup[i][textureId][type] if it doesn't exist
					if (!textureKeyFrameGroup[i][textureId][type]) {
						textureKeyFrameGroup[i][textureId][type] = [];
					}

					for (k = 0; k < amountFrames; ++k) {
						textureKeyFrameGroup[i][textureId][type].push({
							frame: fp.readLong(),
							offset: fp.readFloat()
						});
					}
				}
			}
		}

		this.box = new RSM.Box();
		this.matrix = mat4.create();
		this.textures = textures;
		this.vertices = vertices;
		this.tvertices = tvertices;
		this.faces = faces;
		this.rotKeyframes = rotKeyframes;
		this.posKeyframes = posKeyframes;
		this.scaleKeyFrames = scaleKeyFrames;
		this.textureKeyFrameGroup = textureKeyFrameGroup;
	}

	/**
	 * Calculate node bounding box
	 *
	 * @param {mat4} _matrix
	 */
	calcBoundingBox(_matrix) {
		// Define variables
		let i, j, count;
		const v = vec3.create();
		const box = this.box;
		const nodes = this.main.nodes;
		const matrix = mat4.create();
		const vertices = this.vertices;
		const max = Math.max,
			min = Math.min;
		let x, y, z;

		// Find position
		mat4.copy(this.matrix, _matrix);
		mat4.translate(this.matrix, this.matrix, this.pos);

		// Dynamic or static model
		if (!this.rotKeyframes.length) {
			mat4.rotate(this.matrix, this.matrix, this.rotangle, this.rotaxis);
		} else {
			mat4.rotateQuat(this.matrix, this.matrix, this.rotKeyframes[0].q);
		}

		mat4.scale(this.matrix, this.matrix, this.scale);

		// Strat from here just modify a local matrix
		mat4.copy(matrix, this.matrix);

		if (!this.is_only) {
			mat4.translate(matrix, matrix, this.offset);
		}

		mat4.multiply(matrix, matrix, mat3.toMat4(this.mat3));

		for (i = 0, count = vertices.length; i < count; ++i) {
			//mat4.multiplyVec3( matrix, matrix, vertices[i], v );
			x = vertices[i][0];
			y = vertices[i][1];
			z = vertices[i][2];

			v[0] = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12];
			v[1] = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13];
			v[2] = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];

			for (j = 0; j < 3; j++) {
				box.min[j] = min(v[j], box.min[j]);
				box.max[j] = max(v[j], box.max[j]);
			}
		}

		for (i = 0; i < 3; i++) {
			box.offset[i] = (box.max[i] + box.min[i]) / 2.0;
			box.range[i] = (box.max[i] - box.min[i]) / 2.0;
			box.center[i] = box.min[i] + box.range[i];
		}

		for (i = 0, count = nodes.length; i < count; ++i) {
			if (nodes[i].parentname === this.name && this.name !== this.parentname) {
				nodes[i].calcBoundingBox(this.matrix);
			}
		}
	}

	/**
	 * Compile Node
	 *
	 * @param {mat4} instance_matrix
	 */
	compile(instance_matrix) {
		const modelViewMat = mat4.create();
		const normalMat = mat4.create();

		const textures = this.textures;
		const faces = this.faces;
		const vertices = this.vertices;

		const mesh = {};
		const mesh_size = [];

		const shadeGroup = new Array(32);
		const shadeGroupUsed = new Array(32);
		let i, x, y, z, count;

		// Calculate matrix
		const matrix = mat4.create();
		mat4.identity(matrix);
		mat4.translate(matrix, matrix, [-this.main.box.center[0], -this.main.box.max[1], -this.main.box.center[2]]);
		mat4.multiply(matrix, matrix, this.matrix);

		if (!this.is_only) {
			mat4.translate(matrix, matrix, this.offset);
		}

		mat4.multiply(matrix, matrix, mat3.toMat4(this.mat3));

		// Multiply with instance matrix (position/rotation/...)
		// Generate normal matrix
		mat4.multiply(modelViewMat, instance_matrix, matrix);
		mat4.extractRotation(normalMat, modelViewMat);

		// Generate new vertices
		count = vertices.length;
		const vert = new Float32Array(count * 3);
		for (i = 0; i < count; ++i) {
			x = vertices[i][0];
			y = vertices[i][1];
			z = vertices[i][2];

			//(vec3)vert = (mat4)modelViewMat * (vec3)vertices[i];
			vert[i * 3 + 0] = modelViewMat[0] * x + modelViewMat[4] * y + modelViewMat[8] * z + modelViewMat[12];
			vert[i * 3 + 1] = modelViewMat[1] * x + modelViewMat[5] * y + modelViewMat[9] * z + modelViewMat[13];
			vert[i * 3 + 2] = modelViewMat[2] * x + modelViewMat[6] * y + modelViewMat[10] * z + modelViewMat[14];
		}

		// Generate face normals
		const face_normal = new Float32Array(faces.length * 3);

		// Setup mesh slot array
		for (i = 0, count = textures.length; i < count; ++i) {
			mesh_size[textures[i]] = 0;
		}

		// Find mesh max face
		for (i = 0, count = faces.length; i < count; ++i) {
			mesh_size[textures[faces[i].texid]]++;
		}

		// Initialize buffer
		for (i = 0, count = textures.length; i < count; ++i) {
			mesh[textures[i]] = new Float32Array(mesh_size[textures[i]] * 9 * 3);
		}

		switch (this.main.shadeType) {
			default:

			case RSM.SHADING.NONE:
				this.calcNormal_NONE(face_normal);
				this.generate_mesh_FLAT(vert, face_normal, mesh);
				break;

			case RSM.SHADING.FLAT:
				this.calcNormal_FLAT(face_normal, normalMat, shadeGroupUsed);
				this.generate_mesh_FLAT(vert, face_normal, mesh);
				break;

			case RSM.SHADING.SMOOTH:
				this.calcNormal_FLAT(face_normal, normalMat, shadeGroupUsed);
				this.calcNormal_SMOOTH(face_normal, shadeGroupUsed, shadeGroup);
				this.generate_mesh_SMOOTH(vert, shadeGroup, mesh);
				break;
		}

		return mesh;
	}

	/**
	 * Compile Node at a specific animation frame
	 *
	 * @param {mat4} instance_matrix
	 * @param {number} frame - Animation frame
	 * @param {number} animLen - Total animation length
	 */
	compileAtFrame(instance_matrix, frame, animLen) {
		const modelViewMat = mat4.create();
		const normalMat = mat4.create();

		const textures = this.textures;
		const faces = this.faces;
		const vertices = this.vertices;

		const mesh = {};
		const mesh_size = [];

		const shadeGroup = new Array(32);
		const shadeGroupUsed = new Array(32);
		let i, x, y, z, count;

		// Calculate animated matrix
		const matrix = mat4.create();
		mat4.identity(matrix);
		mat4.translate(matrix, matrix, [-this.main.box.center[0], -this.main.box.max[1], -this.main.box.center[2]]);

		// Calculate node transform matrix with animation
		const nodeMatrix = mat4.create();
		mat4.identity(nodeMatrix);

		// Position animation
		const animPos = getPositionAtFrame(this.posKeyframes, frame, animLen);
		if (animPos) {
			mat4.translate(nodeMatrix, nodeMatrix, animPos);
		} else {
			mat4.translate(nodeMatrix, nodeMatrix, this.pos);
		}

		// Rotation animation
		const animRot = getRotationAtFrame(this.rotKeyframes, frame, animLen);
		if (animRot) {
			mat4.rotateQuat(nodeMatrix, nodeMatrix, animRot);
		} else if (this.rotKeyframes && this.rotKeyframes.length > 0) {
			mat4.rotateQuat(nodeMatrix, nodeMatrix, this.rotKeyframes[0].q);
		} else {
			mat4.rotate(nodeMatrix, nodeMatrix, this.rotangle, this.rotaxis);
		}

		// Scale animation
		const animScale = getScaleAtFrame(this.scaleKeyFrames, frame, animLen);
		if (animScale) {
			mat4.scale(nodeMatrix, nodeMatrix, animScale);
		} else {
			mat4.scale(nodeMatrix, nodeMatrix, this.scale);
		}

		mat4.multiply(matrix, matrix, nodeMatrix);

		if (!this.is_only) {
			mat4.translate(matrix, matrix, this.offset);
		}

		mat4.multiply(matrix, matrix, mat3.toMat4(this.mat3));

		// Multiply with instance matrix (position/rotation/...)
		// Generate normal matrix
		mat4.multiply(modelViewMat, instance_matrix, matrix);
		mat4.extractRotation(normalMat, modelViewMat);

		// Generate new vertices
		count = vertices.length;
		const vert = new Float32Array(count * 3);
		for (i = 0; i < count; ++i) {
			x = vertices[i][0];
			y = vertices[i][1];
			z = vertices[i][2];

			//(vec3)vert = (mat4)modelViewMat * (vec3)vertices[i];
			vert[i * 3 + 0] = modelViewMat[0] * x + modelViewMat[4] * y + modelViewMat[8] * z + modelViewMat[12];
			vert[i * 3 + 1] = modelViewMat[1] * x + modelViewMat[5] * y + modelViewMat[9] * z + modelViewMat[13];
			vert[i * 3 + 2] = modelViewMat[2] * x + modelViewMat[6] * y + modelViewMat[10] * z + modelViewMat[14];
		}

		// Generate face normals
		const face_normal = new Float32Array(faces.length * 3);

		// Setup mesh slot array
		for (i = 0, count = textures.length; i < count; ++i) {
			mesh_size[textures[i]] = 0;
		}

		// Find mesh max face
		for (i = 0, count = faces.length; i < count; ++i) {
			mesh_size[textures[faces[i].texid]]++;
		}

		// Initialize buffer
		for (i = 0, count = textures.length; i < count; ++i) {
			mesh[textures[i]] = new Float32Array(mesh_size[textures[i]] * 9 * 3);
		}

		switch (this.main.shadeType) {
			default:

			case RSM.SHADING.NONE:
				this.calcNormal_NONE(face_normal);
				this.generate_mesh_FLAT(vert, face_normal, mesh);
				break;

			case RSM.SHADING.FLAT:
				this.calcNormal_FLAT(face_normal, normalMat, shadeGroupUsed);
				this.generate_mesh_FLAT(vert, face_normal, mesh);
				break;

			case RSM.SHADING.SMOOTH:
				this.calcNormal_FLAT(face_normal, normalMat, shadeGroupUsed);
				this.calcNormal_SMOOTH(face_normal, shadeGroupUsed, shadeGroup);
				this.generate_mesh_SMOOTH(vert, shadeGroup, mesh);
				break;
		}

		return mesh;
	}
	/**
	 * Generate default normals
	 *
	 * @param {Float32Array[]} out
	 */
	calcNormal_NONE(out) {
		let i, count;
		for (i = 1, count = out.length; i < count; i += 3) {
			out[i] = -1;
		}
	}

	/**
	 * Generate FLAT normals
	 *
	 * @param {Float32Array[]} out
	 * @param {mat4} normalMat
	 * @param {Array} groupUsed
	 */
	calcNormal_FLAT(out, normalMat, groupUsed) {
		let i, j, count;
		let face;
		const temp_vec = vec3.create();
		const faces = this.faces;
		const vertices = this.vertices;

		for (i = 0, j = 0, count = faces.length; i < count; ++i, j += 3) {
			face = faces[i];

			vec3.calcNormal(vertices[face.vertidx[0]], vertices[face.vertidx[1]], vertices[face.vertidx[2]], temp_vec);

			// (vec3)out = (mat4)normalMat * (vec3)temp_vec:
			out[j] =
				normalMat[0] * temp_vec[0] + normalMat[4] * temp_vec[1] + normalMat[8] * temp_vec[2] + normalMat[12];
			out[j + 1] =
				normalMat[1] * temp_vec[0] + normalMat[5] * temp_vec[1] + normalMat[9] * temp_vec[2] + normalMat[13];
			out[j + 2] =
				normalMat[2] * temp_vec[0] + normalMat[6] * temp_vec[1] + normalMat[10] * temp_vec[2] + normalMat[14];

			groupUsed[face.smoothGroup] = true;
		}
	}

	/**
	 * Generate smooth normals
	 *
	 * @param {Float32Array[]} normal
	 * @param {Array} groupUsed
	 * @param {Array} group
	 */
	calcNormal_SMOOTH(normal, groupUsed, group) {
		let i, j, k, l, v, x, y, z, len;
		const size = this.vertices.length;
		const faces = this.faces;
		let face, norm;
		const count = faces.length;

		for (j = 0; j < 32; ++j) {
			// Group not used, skip it
			if (!groupUsed[j]) {
				continue;
			}

			group[j] = new Float32Array(size * 3);
			norm = group[j];

			for (v = 0, l = 0; v < size; ++v, l += 3) {
				x = 0;
				y = 0;
				z = 0;

				for (i = 0, k = 0; i < count; ++i, k += 3) {
					face = faces[i];
					if (
						face.smoothGroup === j &&
						(face.vertidx[0] === v || face.vertidx[1] === v || face.vertidx[2] === v)
					) {
						x += normal[k];
						y += normal[k + 1];
						z += normal[k + 2];
					}
				}

				// (vec3)norm = normalize( vec3(x,y,z) );
				len = 1 / Math.sqrt(x * x + y * y + z * z);
				norm[l] = x * len;
				norm[l + 1] = y * len;
				norm[l + 2] = z * len;
			}
		}
	}

	/**
	 * Generate Mesh (with normals type FLAT)
	 *
	 * @param {Float32Array[]} vert
	 * @param {Float32Array[]} norm
	 * @param {Array} mesh
	 */
	generate_mesh_FLAT(vert, norm, mesh) {
		let a, b, o, i, j, k, t, count;
		const faces = this.faces;
		const textures = this.textures;
		const tver = this.tvertices;
		const alpha = this.main.alpha;
		const offset = [];
		let face, idx, tidx, out;

		// Setup mesh slot array
		for (i = 0, count = textures.length; i < count; ++i) {
			offset[textures[i]] = 0;
		}

		for (i = 0, o = 0, k = 0, count = faces.length; i < count; ++i, k += 3) {
			face = faces[i];
			idx = face.vertidx;
			tidx = face.tvertidx;
			t = textures[face.texid];
			out = mesh[t];
			o = offset[t];

			for (j = 0; j < 3; j++, o += 9) {
				a = idx[j] * 3;
				b = tidx[j] * 6;
				/* vec3 positions  */ out[o + 0] = vert[a + 0];
				out[o + 1] = vert[a + 1];
				out[o + 2] = vert[a + 2];
				/* vec3 normals    */ out[o + 3] = norm[k + 0];
				out[o + 4] = norm[k + 1];
				out[o + 5] = norm[k + 2];
				/* vec2 textCoords */ out[o + 6] = tver[b + 4];
				out[o + 7] = tver[b + 5];
				/* float alpha     */ out[o + 8] = alpha;
			}

			offset[t] = o;
		}
	}

	/**
	 * Generate Mesh (with normals type SMOOTH)
	 *
	 * @param {Float32Array[]} vert
	 * @param {Array} shadeGroup
	 * @param {Array} mesh
	 */
	generate_mesh_SMOOTH(vert, shadeGroup, mesh) {
		let a, b, o, i, j, t, count;
		const faces = this.faces;
		const textures = this.textures;
		const tver = this.tvertices;
		const alpha = this.main.alpha;
		const offset = [];
		let norm, face, idx, tidx, out;

		// Setup mesh slot array
		for (i = 0, count = textures.length; i < count; ++i) {
			offset[textures[i]] = 0;
		}

		for (i = 0, o = 0, count = faces.length; i < count; ++i) {
			face = faces[i];
			norm = shadeGroup[face.smoothGroup];
			idx = face.vertidx;
			tidx = face.tvertidx;

			t = textures[face.texid];
			out = mesh[t];
			o = offset[t];

			for (j = 0; j < 3; j++, o += 9) {
				a = idx[j] * 3;
				b = tidx[j] * 6;
				/* vec3 positions  */ out[o + 0] = vert[a + 0];
				out[o + 1] = vert[a + 1];
				out[o + 2] = vert[a + 2];
				/* vec3 normals    */ out[o + 3] = norm[a + 0];
				out[o + 4] = norm[a + 1];
				out[o + 5] = norm[a + 2];
				/* vec2 textCoords */ out[o + 6] = tver[b + 4];
				out[o + 7] = tver[b + 5];
				/* float alpha     */ out[o + 8] = alpha;
			}

			offset[t] = o;
		}
	}
}

/**
 * Model class loader
 *
 * @param {ArrayBuffer} data - optional
 */
class RSM {
	constructor(data) {
		if (data) {
			this.load(data);
		}
	}

	/**
	 * Model Shading type
	 */
	static SHADING = {
		NONE: 0,
		FLAT: 1,
		SMOOTH: 2
	};

	/**
	 * Bounding Box
	 */
	static Box = Box;

	/**
	 * Loading RSM file
	 *
	 * @param {ArrayBuffer} data
	 */
	load(data) {
		let i, count;
		let posKeyframes;
		let mainNodeName = null;
		const textures = [];
		const additionalTextures = [];

		// Read header.
		const fp = new BinaryReader(data);
		const header = fp.readBinaryString(4);

		if (header !== 'GRSM' && header !== 'GRSX') {
			throw new Error(`RSM::load() - Incorrect header "${header}", must be "GRSM"`);
		}

		// Read infos
		this.version = fp.readByte() + fp.readByte() / 10;
		this.animLen = fp.readLong();
		this.shadeType = fp.readLong();
		this.main_node = null;

		this.alpha = this.version >= 1.4 ? fp.readUByte() / 255.0 : 1.0;

		// Read data based on version
		if (this.version >= 2.3) {
			this.frameRatePerSecond = fp.readFloat();
			count = fp.readLong();

			for (i = 0; i < count; i++) {
				textures.push(fp.readBinaryString(fp.readLong()));
			}
		} else if (this.version >= 2.2) {
			this.frameRatePerSecond = fp.readFloat();
			count = fp.readLong();
			for (i = 0; i < count; ++i) {
				additionalTextures.push(fp.readBinaryString(fp.readLong()));
			}

			count = fp.readLong();

			for (i = 0; i < count; i++) {
				textures.push(fp.readBinaryString(fp.readLong()));
			}
		} else {
			fp.seek(16, SEEK_CUR); // reserved
			count = fp.readLong();
			for (i = 0; i < count; ++i) {
				additionalTextures.push(fp.readBinaryString(40));
			}
			mainNodeName = fp.readBinaryString(40);
			textures.push(mainNodeName);
		}

		count = fp.readLong();
		const nodes = new Array(count);
		for (i = 0; i < count; ++i) {
			nodes[i] = new RSM.Node(this, fp, count === 1);
			if (mainNodeName && nodes[i].name === mainNodeName) {
				this.main_node = nodes[i];
			}
		}

		// In some custom models, the default name don't match nodes name.
		// So by default, assume the main node is the first one.
		if (this.main_node === null) {
			this.main_node = nodes[0];
		}

		// Read poskeyframes
		if (this.version < 1.6) {
			count = fp.readLong();
			posKeyframes = new Array(count);

			for (i = 0; i < count; ++i) {
				posKeyframes[i] = {
					frame: fp.readLong(),
					px: fp.readFloat(),
					py: fp.readFloat(),
					pz: fp.readFloat(),
					data: fp.readFloat()
				};
			}

			this.posKeyframes = posKeyframes;
		} else {
			this.posKeyframes = [];
		}

		// read Volume box
		count = fp.offset >= fp.length ? 0 : fp.readLong();
		const volumebox = new Array(count);

		for (i = 0; i < count; ++i) {
			volumebox[i] = {
				size: [fp.readFloat(), fp.readFloat(), fp.readFloat()],
				pos: [fp.readFloat(), fp.readFloat(), fp.readFloat()],
				rot: [fp.readFloat(), fp.readFloat(), fp.readFloat()],
				flag: this.version >= 1.3 ? fp.readLong() : 0
			};
		}

		this.textures = additionalTextures;
		this.nodes = nodes;
		if (this.version >= 2.3) {
			for (i = 0; i < this.main_node.textures.length; i++) {
				if (!this.textures.includes(this.main_node.textures[i])) {
					const texture = this.main_node.textures[i];
					this.textures.push(texture);
					this.main_node.textures[i] = this.textures.indexOf(texture);
				}
			}
			this.nodes.forEach(node => {
				for (i = 0; i < node.textures.length; i++) {
					if (typeof node.textures[i] !== 'number') {
						const texture = node.textures[i];
						if (!this.textures.includes(texture)) {
							this.textures.push(texture);
						}
						node.textures[i] = this.textures.indexOf(texture);
					}
				}
			});
		}
		this.volumebox = volumebox;
		this.instances = [];
		this.box = new RSM.Box();
		this.calcBoundingBox();
	}

	/**
	 * Create a model instance
	 *
	 * @param {object} model
	 * @param {number} width
	 * @param {number} height
	 */
	createInstance(model, width, height) {
		const matrix = mat4.create();

		mat4.identity(matrix);
		mat4.translate(matrix, matrix, [model.position[0] + width, model.position[1], model.position[2] + height]);
		mat4.rotateZ(matrix, matrix, (model.rotation[2] / 180) * Math.PI);
		mat4.rotateX(matrix, matrix, (model.rotation[0] / 180) * Math.PI);
		mat4.rotateY(matrix, matrix, (model.rotation[1] / 180) * Math.PI);
		mat4.scale(matrix, matrix, model.scale);

		if (this.main_node.main.version >= 2.2) {
			// Apply scaling based on the main node's flip attribute
			mat4.scale(matrix, matrix, this.main_node.flip);

			// Apply translation based on the main node's offset attribute
			mat4.translate(matrix, matrix, this.main_node.offset);

			// Apply translation upwards by the range value in the Y axis
			mat4.translate(matrix, matrix, [0.0, this.box.range[1], 0.0]);

			// Apply translation based on the box's offset attribute
			mat4.translate(matrix, matrix, this.box.offset);
		}

		this.instances.push(matrix);
	}

	/**
	 * Calculate model bounding box
	 */
	calcBoundingBox() {
		let i, j;
		const box = this.box;
		const matrix = mat4.create();
		const nodes = this.nodes;
		const min = Math.min,
			max = Math.max;
		const count = nodes.length;

		mat4.identity(matrix);
		this.main_node.calcBoundingBox(matrix);

		for (i = 0; i < 3; ++i) {
			for (j = 0; j < count; ++j) {
				box.max[i] = max(box.max[i], nodes[j].box.max[i]);
				box.min[i] = min(box.min[i], nodes[j].box.min[i]);
			}
			box.offset[i] = (box.max[i] + box.min[i]) / 2.0;
			box.range[i] = (box.max[i] - box.min[i]) / 2.0;
			box.center[i] = box.min[i] + box.range[i];
		}
	}

	/**
	 * Compile Model
	 */
	compile() {
		const nodes = this.nodes;
		const instances = this.instances;

		const node_count = nodes.length;
		const instance_count = instances.length;
		let i, j, k;

		const meshes = new Array(node_count * instance_count);

		// Generate Mesh
		for (i = 0, k = 0; i < node_count; ++i) {
			for (j = 0; j < instance_count; ++j, k++) {
				meshes[k] = nodes[i].compile(instances[j]);
			}
		}

		return {
			meshes: meshes,
			textures: this.textures
		};
	}

	/**
	 * Check if this model has animation keyframes
	 *
	 * @return {boolean}
	 */
	hasAnimation() {
		if (this._hasanimation) {
			return true;
		}

		for (let i = 0; i < this.nodes.length; i++) {
			const node = this.nodes[i];
			if (
				(node.rotKeyframes && node.rotKeyframes.length > 0) ||
				(node.posKeyframes && node.posKeyframes.length > 0) ||
				(node.scaleKeyFrames && node.scaleKeyFrames.length > 0)
			) {
				this._hasanimation = true;
				return true;
			}
		}
		return false;
	}

	static Node = Node;

	/**
	 * Compile Model at a specific animation frame
	 *
	 * @param {number} frame - Animation frame
	 */
	compileAtFrame(frame) {
		const nodes = this.nodes;
		const instances = this.instances;
		const animLen = this.animLen || 1;

		const node_count = nodes.length;
		const instance_count = instances.length;
		let i, j, k;

		const meshes = new Array(node_count * instance_count);

		// Generate Mesh at frame
		for (i = 0, k = 0; i < node_count; ++i) {
			for (j = 0; j < instance_count; ++j, k++) {
				meshes[k] = nodes[i].compileAtFrame(instances[j], frame, animLen);
			}
		}

		return {
			meshes: meshes,
			textures: this.textures
		};
	}
}
/**
 * Interpolate rotation keyframes using SLERP
 *
 * @param {Array} keyframes
 * @param {number} frame
 * @param {number} animLen
 * @returns {Array|null} quaternion or null
 */
function getRotationAtFrame(keyframes, frame, animLen) {
	if (!keyframes || keyframes.length === 0) {
		return null;
	}

	if (keyframes.length === 1) {
		return keyframes[0].q;
	}

	// Find surrounding keyframes
	let prev = keyframes[0];
	let next = null;

	for (let i = 0; i < keyframes.length; i++) {
		if (keyframes[i].frame > frame) {
			next = keyframes[i];
			break;
		}
		prev = keyframes[i];
	}

	if (!next) {
		return prev.q;
	}

	// Calculate interpolation factor
	const frameDiff = next.frame - prev.frame;
	if (frameDiff === 0) {
		return prev.q;
	}

	const t = (frame - prev.frame) / frameDiff;

	// SLERP between quaternions
	return slerpQuat(prev.q, next.q, t);
}

/**
 * SLERP quaternion interpolation
 */
function slerpQuat(q1, q2, t) {
	const result = new Float32Array(4);

	let dot = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];

	let q2Sign = 1;
	if (dot < 0) {
		dot = -dot;
		q2Sign = -1;
	}

	let scale0, scale1;
	if (dot > 0.9995) {
		scale0 = 1.0 - t;
		scale1 = t * q2Sign;
	} else {
		const theta = Math.acos(dot);
		const sinTheta = Math.sin(theta);
		scale0 = Math.sin((1.0 - t) * theta) / sinTheta;
		scale1 = (Math.sin(t * theta) / sinTheta) * q2Sign;
	}

	result[0] = scale0 * q1[0] + scale1 * q2[0];
	result[1] = scale0 * q1[1] + scale1 * q2[1];
	result[2] = scale0 * q1[2] + scale1 * q2[2];
	result[3] = scale0 * q1[3] + scale1 * q2[3];

	return result;
}

/**
 * Interpolate position keyframes
 *
 * @param {Array} keyframes
 * @param {number} frame
 * @param {number} animLen
 * @returns {Array|null} position or null
 */
function getPositionAtFrame(keyframes, frame, animLen) {
	if (!keyframes || keyframes.length === 0) {
		return null;
	}

	if (keyframes.length === 1) {
		return [keyframes[0].px, keyframes[0].py, keyframes[0].pz];
	}

	// Find surrounding keyframes
	let prev = keyframes[0];
	let next = null;

	for (let i = 0; i < keyframes.length; i++) {
		if (keyframes[i].frame > frame) {
			next = keyframes[i];
			break;
		}
		prev = keyframes[i];
	}

	if (!next) {
		return [prev.px, prev.py, prev.pz];
	}

	// Calculate interpolation factor
	const frameDiff = next.frame - prev.frame;
	if (frameDiff === 0) {
		return [prev.px, prev.py, prev.pz];
	}

	const t = (frame - prev.frame) / frameDiff;

	// Linear interpolation
	return [prev.px + (next.px - prev.px) * t, prev.py + (next.py - prev.py) * t, prev.pz + (next.pz - prev.pz) * t];
}

/**
 * Interpolate scale keyframes
 *
 * @param {Array} keyframes
 * @param {number} frame
 * @param {number} animLen
 * @returns {Array|null} scale or null
 */
function getScaleAtFrame(keyframes, frame, animLen) {
	if (!keyframes || keyframes.length === 0) {
		return null;
	}

	if (keyframes.length === 1) {
		return keyframes[0].Scale;
	}

	// Find surrounding keyframes
	let prev = keyframes[0];
	let next = null;

	for (let i = 0; i < keyframes.length; i++) {
		if (keyframes[i].Frame > frame) {
			next = keyframes[i];
			break;
		}
		prev = keyframes[i];
	}

	if (!next) {
		return prev.Scale;
	}

	// Calculate interpolation factor
	const frameDiff = next.Frame - prev.Frame;
	if (frameDiff === 0) {
		return prev.Scale;
	}

	const t = (frame - prev.Frame) / frameDiff;

	// Linear interpolation
	return [
		prev.Scale[0] + (next.Scale[0] - prev.Scale[0]) * t,
		prev.Scale[1] + (next.Scale[1] - prev.Scale[1]) * t,
		prev.Scale[2] + (next.Scale[2] - prev.Scale[2]) * t
	];
}

/**
 * Export
 */
export default RSM;
