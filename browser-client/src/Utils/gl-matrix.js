/**
 * Utils/gl-matrix.js
 *
 * Extend gl-matrix
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix, { vec2, vec3, vec4, mat2, mat2d, mat3, mat4, quat } from 'Vendors/gl-matrix.js';

	/**
	 * Calculate a normal from the three givens vectors
	 *
	 * @param {vec3} a
	 * @param {vec3} b
	 * @param {vec3} c
	 * @param {vec3} out
	 *
	 * @return {vec3} out
	 */
	glMatrix.vec3.calcNormal = function( a, b, c, out ) {
		let x1, y1, z1, x2, y2, z2, x3, y3, z3, len;

		// vec3.sub( c, b );
		x1 = c[0] - b[0];
		y1 = c[1] - b[1];
		z1 = c[2] - b[2];

		// vec3.sub( a, b );
		x2 = a[0] - b[0];
		y2 = a[1] - b[1];
		z2 = a[2] - b[2];

		// vec3.cross()
		x3 = y1 * z2 - z1 * y2;
		y3 = z1 * x2 - x1 * z2;
		z3 = x1 * y2 - y1 * x2;

		// vec3.normalize()
		const dist = x3*x3 + y3*y3 + z3*z3;
		if (dist > 0) {
			len = 1 / Math.sqrt(dist);
			out[0] = x3 * len;
			out[1] = y3 * len;
			out[2] = z3 * len;
		} else {
			out[0] = 0; out[1] = 0; out[2] = 0;
		}

		return out;
	};


	/**
	 * Create a normal with the four givens vector
	 *
	 * @param {vec3} a
	 * @param {vec3} b
	 * @param {vec3} c
	 * @param {vec3} d
	 * @param {vec3} out
	 *
	 * @returns {vec3} out
	 */
	glMatrix.vec4.calcNormal = function( a, b, c, d, out ) {
		let x,  y,  z,
			x1, y1, z1,
			x2, y2, z2,
			x3, y3, z3,
			len;

		//vec3.calcNormal( a, b, c, out );
			//vec3.subtract( c, b, v1 );
			x1  = c[0] - b[0];  y1 = c[1] - b[1];  z1 = c[2] - b[2];
			//vec3.subtract( a, b, v2 );
			x2  = a[0] - b[0];  y2 = a[1] - b[1];  z2 = a[2] - b[2];
			//vec3.cross( v1, v2 );
			x3  = y1 * z2 - z1 * y2;
			y3  = z1 * x2 - x1 * z2;
			z3  = x1 * y2 - y1 * x2;
			//vec3.normalize(v1);
			const dist1 = x3*x3 + y3*y3 + z3*z3;
			if (dist1 > 0) {
				len = 1 / Math.sqrt(dist1);
				x   = x3 * len;
				y   = y3 * len;
				z   = z3 * len;
			} else {
				x = 0; y = 0; z = 0;
			}

		//vec3.calcNormal( c, d, a, tmp );
			//vec3.subtract( a, d, v1 );
			x1  = a[0] - d[0];  y1 = a[1] - d[1];  z1 = a[2] - d[2];
			//vec3.subtract( c, d, v2 );
			x2  = c[0] - d[0];  y2 = c[1] - d[1];  z2 = c[2] - d[2];
			//vec3.cross( v1, v2 );
			x3  = y1 * z2 - z1 * y2;
			y3  = z1 * x2 - x1 * z2;
			z3  = x1 * y2 - y1 * x2;
			//vec3.normalize(v1);
			const dist2 = x3*x3 + y3*y3 + z3*z3;
			if (dist2 > 0) {
				len = 1 / Math.sqrt(dist2);
				x  += x3 * len;
				y  += y3 * len;
				z  += z3 * len;
			}

		//vec3.normalize(out);
		const distOut = x*x + y*y + z*z;
		if (distOut > 0) {
			len    = 1 / Math.sqrt(distOut);
			out[0] = x * len;
			out[1] = y * len;
			out[2] = z * len;
		} else {
			out[0] = 0; out[1] = 0; out[2] = 0;
		}

		return out;
	};


	/**
	 * Translates a matrix by the given Z property
	 *
	 * @param {mat4} mat mat4 to translate
	 * @param {float} z float z translation
	 * @param {mat4} [dest] mat4 receiving operation result. If not specified result is written to mat
	 *
	 * @returns {mat4} dest if specified, mat otherwise
	 */
	glMatrix.mat4.translateZ = function (mat, z, dest) {
		let a00, a01, a02, a03,
			a10, a11, a12, a13,
			a20, a21, a22, a23;

		if (!dest || mat === dest) {
			mat[12] += mat[8]  * z;
			mat[13] += mat[9]  * z;
			mat[14] += mat[10] * z;
			mat[15] += mat[11] * z;
			return mat;
		}

		a00 = mat[0]; a01 = mat[1]; a02 = mat[2]; a03 = mat[3];
		a10 = mat[4]; a11 = mat[5]; a12 = mat[6]; a13 = mat[7];
		a20 = mat[8]; a21 = mat[9]; a22 = mat[10]; a23 = mat[11];

		dest[0] = a00; dest[1] = a01; dest[2] = a02; dest[3] = a03;
		dest[4] = a10; dest[5] = a11; dest[6] = a12; dest[7] = a13;
		dest[8] = a20; dest[9] = a21; dest[10] = a22; dest[11] = a23;

		dest[12] += a20 * z;
		dest[13] += a21 * z;
		dest[14] += a22 * z;
		dest[15] += a23 * z;
		return dest;
	};


	/**
	 * Do a quaternon rotation
	 *
	 * @param {mat4} mat mat4 to rotate
	 * @param {Array} w
	 *
	 * @returns {mat4} dest if specified, mat otherwise
	 */
	glMatrix.mat4.rotateQuat = function(out, mat, w) {
		let a,b,c,d;
		a = w[0];
		b = w[1];
		c = w[2];
		d = w[3];

		let norm = Math.sqrt(a*a+b*b+c*c+d*d);
		a /= norm;
		b /= norm;
		c /= norm;
		d /= norm;

		return glMatrix.mat4.multiply( out, mat,[
			1.0 - 2.0 * ( b * b + c * c ),     2.0 * (a * b + c * d),            2.0 * (a * c - b * d),           0.0,
			2.0 * ( a * b - c * d ),           1.0 - 2.0 * ( a * a + c * c ),    2.0 * (c * b + a * d ),          0.0,
			2.0 * ( a * c + b * d ),           2.0 * ( b * c - a * d ),          1.0 - 2.0 * ( a * a + b * b ),   0.0,
			0.0,                               0.0,                              0.0,                             1.0
		]);
	};


	/**
	 * Extract rotation matrix
	 *
	 * @param {mat4} mat
	 * @param {mat4} dest
	 *
	 * @returns {mat4} dest
	 */
	glMatrix.mat4.extractRotation = function( out, mat ) {

		let scale_x = 1.0 / glMatrix.vec3.length([ mat[0], mat[1], mat[2]  ]);
		let scale_y = 1.0 / glMatrix.vec3.length([ mat[4], mat[5], mat[6]  ]);
		let scale_z = 1.0 / glMatrix.vec3.length([ mat[8], mat[9], mat[10] ]);

		out[0]  = mat[0]  * scale_x;
		out[1]  = mat[1]  * scale_x;
		out[2]  = mat[2]  * scale_x;
		out[4]  = mat[4]  * scale_y;
		out[5]  = mat[5]  * scale_y;
		out[6]  = mat[6]  * scale_y;
		out[8]  = mat[8]  * scale_z;
		out[9]  = mat[9]  * scale_z;
		out[10] = mat[10] * scale_z;

		return out;
	};


	/**
	 * Copies the elements of a mat3 into the upper 3x3 elements of a mat4
	 *
	 * @param {mat3} mat mat3 containing values to copy
	 * @param {mat4} [dest] mat4 receiving copied values
	 *
	 * @returns {mat4} dest if specified, a new mat4 otherwise
	 */
	glMatrix.mat3.toMat4 = function (mat, dest) {
		if (!dest) { dest = glMatrix.mat4.create(); }

		dest[15] = 1;
		dest[14] = 0;
		dest[13] = 0;
		dest[12] = 0;

		dest[11] = 0;
		dest[10] = mat[8];
		dest[9] = mat[7];
		dest[8] = mat[6];

		dest[7] = 0;
		dest[6] = mat[5];
		dest[5] = mat[4];
		dest[4] = mat[3];

		dest[3] = 0;
		dest[2] = mat[2];
		dest[1] = mat[1];
		dest[0] = mat[0];

		return dest;
	};


	/**
	 * Copies the upper-left 3x3 values of a mat4 into a mat3
	 *
	 * @param {mat3} out the receiving matrix
	 * @param {mat4} mat the source 4x4 matrix
	 *
	 * @returns {mat3} out
	 */
	glMatrix.mat3.fromMat4 = function (out, mat) {
		out[0] = mat[0];
		out[1] = mat[1];
		out[2] = mat[2];
		out[3] = mat[4];
		out[4] = mat[5];
		out[5] = mat[6];
		out[6] = mat[8];
		out[7] = mat[9];
		out[8] = mat[10];

		return out;
	};


	/**
	 * Perspective matrix (Legacy RoBrowser version)
	 *
	 * @param {number} fovy
	 * @param {number} aspect
	 * @param {number} near
	 * @param {number} far
	 * @param {mat4} [dest]
	 *
	 * @returns {mat4} dest
	 */
	glMatrix.mat4.perspective = function (fovy, aspect, near, far, dest) {
		if (!dest) { dest = glMatrix.mat4.create(); }
		let top = near * Math.tan(fovy * Math.PI / 360.0),
			right = top * aspect;
		return glMatrix.mat4.frustum(dest, -right, right, -top, top, near, far);
	};


	/**
	 * Multiplies a vec3 by a mat4
	 * The last component of the vec3 is assumed to be 1.0
	 * 
	 * @param {vec3} vec 3 position vector
	 * @param {mat4} mat 4x4 matrix
	 * 
	 * @returns {vec3} resulting vector
	 */
	glMatrix.mat4.multiplyVec3 = function(vec, mat) {
		const out = new Float32Array(3);
		const x = vec[0], y = vec[1], z = vec[2];

		out[0] = mat[0] * x + mat[4] * y + mat[8] * z + mat[12];
		out[1] = mat[1] * x + mat[5] * y + mat[9] * z + mat[13];
		out[2] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14];
		return out;
	};


	/**
	 * Calculates the inverse of the upper 3x3 elements of a mat4 and copies the result into a mat3
	 * The resulting matrix is useful for calculating transformed normals
	 *
	 * Params:
	 * @param {mat4} mat mat4 containing values to invert and copy
	 * @param {mat3} [dest] mat3 receiving values
	 *
	 * @returns {mat3} dest is specified, a new mat3 otherwise, null if the matrix cannot be inverted
	 */
	glMatrix.mat4.toInverseMat3 = function (mat, dest) {
		// Cache the matrix values (makes for huge speed increases!)
		let a00 = mat[0], a01 = mat[1], a02 = mat[2],
			a10 = mat[4], a11 = mat[5], a12 = mat[6],
			a20 = mat[8], a21 = mat[9], a22 = mat[10],

			b01 = a22 * a11 - a12 * a21,
			b11 = -a22 * a10 + a12 * a20,
			b21 = a21 * a10 - a11 * a20,

			d = a00 * b01 + a01 * b11 + a02 * b21,
			id;

		if (!d) { return null; }
		id = 1 / d;

		if (!dest) { dest = glMatrix.mat3.create(); }

		dest[0] = b01 * id;
		dest[1] = (-a22 * a01 + a02 * a21) * id;
		dest[2] = (a12 * a01 - a02 * a11) * id;
		dest[3] = b11 * id;
		dest[4] = (a22 * a00 - a02 * a20) * id;
		dest[5] = (-a12 * a00 + a02 * a10) * id;
		dest[6] = b21 * id;
		dest[7] = (-a21 * a00 + a01 * a20) * id;
		dest[8] = (a11 * a00 - a01 * a10) * id;

		return dest;
	};

export { glMatrix, vec2, vec3, vec4, mat2, mat2d, mat3, mat4, quat };
export default glMatrix;
