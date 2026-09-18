// src/controllers/grfController.js
const { GrfNode } = require("@chicowall/grf-loader");

const fs = require("fs");
const path = require("path");
const logger = require("../utils/logger");
class Grf {
	constructor(filePath) {
		this.fileName = path.basename(filePath);
		this.filePath = filePath;
		this.grf = null;
		this.loaded = false;
	}

	async load() {
		if (!fs.existsSync(this.filePath)) {
			logger.error(`GRF file not found: ${this.filePath}`);
			return;
		}

		try {
			const fd = fs.openSync(this.filePath, "r");
			this.grf = new GrfNode(fd);
			await this.grf.load();
			this.loaded = true;
		} catch (error) {
			logger.error("Error loading GRF file:", error);
		}
	}

	async getFile(filename) {
		if (!this.loaded || !this.grf) {
			logger.error("GRF not loaded or not initialized");
			return null;
		}
		try {
			const { data, error } = await this.grf.getFile(filename);
			if (error) {
				return null;
			}
			return Buffer.from(data);
		} catch (error) {
			logger.error(`Error extracting file: ${error}`);
			return null;
		}
	}

	listFiles() {
		if (!this.loaded || !this.grf) {
			logger.error("GRF not loaded or not initialized");
			return [];
		}

		return Array.from(this.grf.files.keys());
	}
}

module.exports = Grf;
