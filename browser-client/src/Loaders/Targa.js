/**
 * Loaders/Targa.js
 *
 * Loaders for .tga image file (Targa Truevision)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * TGA Namespace
 * @constructor
 */
class Targa {
	constructor() {}

	/**
	 * @var {object} TGA type constants
	 */
	static Type = {
		NO_DATA: 0,
		INDEXED: 1,
		RGB: 2,
		GREY: 3,
		RLE_INDEXED: 9,
		RLE_RGB: 10,
		RLE_GREY: 11
	};

	/**
	 * @var {object} TGA origin constants
	 */
	static Origin = {
		BOTTOM_LEFT: 0x00,
		BOTTOM_RIGHT: 0x01,
		TOP_LEFT: 0x02,
		TOP_RIGHT: 0x03,
		SHIFT: 0x04,
		MASK: 0x30
	};

	/**
	 * Open a targa file using XHR, be aware with Cross Domain files...
	 *
	 * @param {string} path - Path of the filename to load
	 * @param {function} callback - callback to trigger when the file is loaded
	 */
	open(path, callback) {
		const tga = this;
		const req = new XMLHttpRequest();
		req.responseType = 'arraybuffer';
		req.open('GET', path, true);
		req.onload = function () {
			if (this.status === 200) {
				tga.load(new Uint8Array(req.response));
				if (callback) {
					callback.call(tga);
				}
			}
		};
		req.send(null);
	}

	/**
	 * Load and parse a TGA file
	 *
	 * @param {Uint8Array} data - TGA file buffer array
	 */
	load(data) {
		let offset = 0;

		// Not enough data to contain header ?
		if (data.length < 0x12) {
			throw new Error('Targa::load() - Not enough data to contain header');
		}

		// Read TgaHeader
		this.header = {
			/* 0x00  BYTE */ idLength: data[offset++],
			/* 0x01  BYTE */ colorMapType: data[offset++],
			/* 0x02  BYTE */ imageType: data[offset++],
			/* 0x03  WORD */ colorMapIndex: data[offset++] | (data[offset++] << 8),
			/* 0x05  WORD */ colorMapLength: data[offset++] | (data[offset++] << 8),
			/* 0x07  BYTE */ colorMapDepth: data[offset++],
			/* 0x08  WORD */ offsetX: data[offset++] | (data[offset++] << 8),
			/* 0x0a  WORD */ offsetY: data[offset++] | (data[offset++] << 8),
			/* 0x0c  WORD */ width: data[offset++] | (data[offset++] << 8),
			/* 0x0e  WORD */ height: data[offset++] | (data[offset++] << 8),
			/* 0x10  BYTE */ pixelDepth: data[offset++],
			/* 0x11  BYTE */ flags: data[offset++]
		};

		// Set shortcut
		this.header.hasEncoding =
			this.header.imageType === Targa.Type.RLE_INDEXED ||
			this.header.imageType === Targa.Type.RLE_RGB ||
			this.header.imageType === Targa.Type.RLE_GREY;
		this.header.hasColorMap =
			this.header.imageType === Targa.Type.RLE_INDEXED || this.header.imageType === Targa.Type.INDEXED;
		this.header.isGreyColor =
			this.header.imageType === Targa.Type.RLE_GREY || this.header.imageType === Targa.Type.GREY;

		// Check if a valid TGA file (or if we can load it)
		checkHeader(this.header);

		// Move to data
		offset += this.header.idLength;
		if (offset >= data.length) {
			throw new Error('Targa::load() - No data');
		}

		// Read palette
		if (this.header.hasColorMap) {
			const colorMapSize = this.header.colorMapLength * (this.header.colorMapDepth >> 3);
			this.palette = data.subarray(offset, offset + colorMapSize);
			offset += colorMapSize;
		}

		const pixelSize = this.header.pixelDepth >> 3;
		const imageSize = this.header.width * this.header.height;
		const pixelTotal = imageSize * pixelSize;

		// RLE encoded
		if (this.header.hasEncoding) {
			this.imageData = decodeRLE(data, offset, pixelSize, pixelTotal);
		}

		// RAW pixels
		else {
			this.imageData = data.subarray(offset, offset + (this.header.hasColorMap ? imageSize : pixelTotal));
		}
	}

	/**
	 * Return a ImageData object from a TGA file
	 *
	 * @param {object} imageData - Optional ImageData to work with
	 * @returns {object} imageData
	 */
	getImageData(imageData) {
		const width = this.header.width;
		const height = this.header.height;
		// Create an imageData
		if (!imageData) {
			imageData = { width: width, height: height, data: new Uint8ClampedArray(width * height * 4) };
		}
		const origin = (this.header.flags & Targa.Origin.MASK) >> Targa.Origin.SHIFT;
		let x_start, x_step, x_end, y_start, y_step, y_end;

		if (origin === Targa.Origin.TOP_LEFT || origin === Targa.Origin.TOP_RIGHT) {
			y_start = 0;
			y_step = 1;
			y_end = height;
		} else {
			y_start = height - 1;
			y_step = -1;
			y_end = -1;
		}

		if (origin === Targa.Origin.TOP_LEFT || origin === Targa.Origin.BOTTOM_LEFT) {
			x_start = 0;
			x_step = 1;
			x_end = width;
		} else {
			x_start = width - 1;
			x_step = -1;
			x_end = -1;
		}

		const data = imageData.data;
		const input = this.imageData;
		const buffer32 = new Uint32Array(data.buffer);
		let i = 0;

		switch (this.header.pixelDepth) {
			case 8:
				if (this.header.isGreyColor) {
					// 8-bit grayscale
					for (let y = y_start; y !== y_end; y += y_step) {
						const rowOffset = y * width;
						for (let x = x_start; x !== x_end; x += x_step) {
							const v = input[i++];
							buffer32[rowOffset + x] = (255 << 24) | (v << 16) | (v << 8) | v;
						}
					}
				} else {
					// 8-bit indexed (paletted)
					const colormap = this.palette;
					for (let y = y_start; y !== y_end; y += y_step) {
						const rowOffset = y * width;
						for (let x = x_start; x !== x_end; x += x_step) {
							const idx = input[i++] * 3;
							const b = colormap[idx + 0];
							const g = colormap[idx + 1];
							const r = colormap[idx + 2];
							buffer32[rowOffset + x] = (255 << 24) | (b << 16) | (g << 8) | r;
						}
					}
				}
				break;

			case 16:
				if (this.header.isGreyColor) {
					// 16-bit grayscale (intensity + alpha)
					for (let y = y_start; y !== y_end; y += y_step) {
						const rowOffset = y * width;
						for (let x = x_start; x !== x_end; x += x_step) {
							const intensity = input[i++];
							const alpha = input[i++];
							buffer32[rowOffset + x] = (alpha << 24) | (intensity << 16) | (intensity << 8) | intensity;
						}
					}
				} else {
					// 16-bit 5-5-5-1
					for (let y = y_start; y !== y_end; y += y_step) {
						const rowOffset = y * width;
						for (let x = x_start; x !== x_end; x += x_step) {
							const color = input[i] | (input[i + 1] << 8);
							i += 2;

							const r = (color & 0x7c00) >> 7;
							const g = (color & 0x03e0) >> 2;
							const b = (color & 0x001f) << 3;
							const a = color & 0x8000 ? 255 : 0;

							buffer32[rowOffset + x] = (a << 24) | (b << 16) | (g << 8) | r;
						}
					}
				}
				break;
			case 24:
				for (let y = y_start; y !== y_end; y += y_step) {
					const rowOffset = y * width;
					for (let x = x_start; x !== x_end; x += x_step) {
						const b = input[i++];
						const g = input[i++];
						const r = input[i++];
						buffer32[rowOffset + x] = (255 << 24) | (b << 16) | (g << 8) | r;
					}
				}
				break;

			case 32: {
				const MAGENTA_MASK = 0x00ff00ff;
				const PIXEL_MASK = 0x00ffffff;
				for (let y = y_start; y !== y_end; y += y_step) {
					const rowOffset = y * width;
					for (let x = x_start; x !== x_end; x += x_step) {
						// BGRA to RGBA (or ABGR depending on endianness)
						// TGA 32 is usually BGRA
						// read as Uint32 (BGRA format input[i]=B, input[i+1]=G, input[i+2]=R, input[i+3]=A).
						// Writing (A<<24 | B<<16 | G<<8 | R) produces LE memory bytes [R, G, B, A], which matches Canvas ImageData (RGBA).
						const pixel = (input[i + 3] << 24) | (input[i] << 16) | (input[i + 1] << 8) | input[i + 2];
						i += 4;

						buffer32[rowOffset + x] = (pixel & PIXEL_MASK) === MAGENTA_MASK ? 0 : pixel;
					}
				}
				break;
			}
		}
		return imageData;
	}

	/**
	 * Return a canvas with the TGA render on it
	 *
	 * @returns {object} CanvasElement
	 */
	getCanvas() {
		const canvas = document.createElement('canvas');
		const ctx = canvas.getContext('2d');
		const imageData = ctx.createImageData(this.header.width, this.header.height);

		canvas.width = this.header.width;
		canvas.height = this.header.height;

		ctx.putImageData(this.getImageData(imageData), 0, 0);

		return canvas;
	}

	/**
	 * Return a dataURI of the TGA file
	 *
	 * @param {string} type - Optional image content-type to output (default: image/png)
	 * @returns {string} url
	 */
	getDataURL(type) {
		return this.getCanvas().toDataURL(type || 'image/png');
	}
}

/**
 * Check the header of TGA file to detect errors
 *
 * @param {object} tga header structure
 * @throws Error
 */
function checkHeader(header) {
	// What the need of a file without data ?
	if (header.imageType === Targa.Type.NO_DATA) {
		throw new Error('Targa::checkHeader() - No data');
	}

	// Indexed type
	if (header.hasColorMap) {
		if (header.colorMapLength > 256 || header.colorMapDepth !== 24 || header.colorMapType !== 1) {
			throw new Error('Targa::checkHeader() - Invalid colormap for indexed type');
		}
	} else {
		if (header.colorMapType) {
			throw new Error('Targa::checkHeader() - Why does the image contain a palette ?');
		}
	}

	// Check image size
	if (header.width <= 0 || header.height <= 0) {
		throw new Error('Targa::checkHeader() - Invalid image size');
	}

	// Check pixel size
	if (header.pixelDepth !== 8 && header.pixelDepth !== 16 && header.pixelDepth !== 24 && header.pixelDepth !== 32) {
		throw new Error(`Targa::checkHeader() - Invalid pixel size "${header.pixelDepth}"`);
	}
}

/**
 * Decode RLE compression
 *
 * @param {Uint8Array} data
 * @param {number} offset in data to start loading RLE
 * @param {number} pixel count
 * @param {number} output buffer size
 */
function decodeRLE(data, offset, pixelSize, outputSize) {
	let pos, c, count, i;

	const output = new Uint8Array(outputSize);
	const pixels = new Uint8Array(pixelSize);
	pos = 0;

	while (pos < outputSize) {
		c = data[offset++];
		count = (c & 0x7f) + 1;

		// RLE pixels.
		if (c & 0x80) {
			// Bind pixel tmp array
			for (i = 0; i < pixelSize; ++i) {
				pixels[i] = data[offset++];
			}

			// Copy pixel array
			for (i = 0; i < count; ++i) {
				output.set(pixels, pos);
				pos += pixelSize;
			}
		}

		// Raw pixels.
		else {
			count *= pixelSize;
			for (i = 0; i < count; ++i) {
				output[pos++] = data[offset++];
			}
		}
	}

	return output;
}

/**
 * Export
 */
export default Targa;
