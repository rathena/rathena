// src/controllers/grfController.js

const { GrfNode } = require("@chicowall/grf-loader");

const fs = require("fs");
const path = require("path");
const logger = require("../utils/logger");


function readGrfSignature(filePath) {
    const fd = fs.openSync(filePath, "r");

    try {
        const buffer = Buffer.alloc(16);
        fs.readSync(fd, buffer, 0, buffer.length, 0);

        const nullIndex = buffer.indexOf(0);

        return buffer
            .subarray(0, nullIndex >= 0 ? nullIndex : buffer.length)
            .toString("ascii");
    } finally {
        fs.closeSync(fd);
    }
}


class Grf {
    constructor(filePath) {
        this.fileName = path.basename(filePath);
        this.filePath = filePath;

        this.grf = null;
        this.loaded = false;
        this.type = null;
    }


    async load() {
        if (!fs.existsSync(this.filePath)) {
            logger.error(`GRF file not found: ${this.filePath}`);
            return;
        }

        try {
            const signature = readGrfSignature(this.filePath);

            logger.info(
                `Loading GRF ${this.fileName} - signature: "${signature}"`
            );

            if (signature === "Event Horizon") {
                const { EventHorizonGrf } = await import(
                    "../loaders/EventHorizonGrf.mjs"
                );

                this.grf = new EventHorizonGrf(this.filePath);
                this.type = "event-horizon";

                await this.grf.load();
            }

            else if (signature === "Master of Magic") {
                const fd = fs.openSync(this.filePath, "r");

                this.grf = new GrfNode(fd);
                this.type = "classic";

                await this.grf.load();
            }

            else {
                throw new Error(
                    `Unsupported GRF signature: "${signature}"`
                );
            }

            this.loaded = true;

            logger.info(
                `GRF loaded successfully: ${this.fileName} (${this.type})`
            );

        } catch (error) {
            this.loaded = false;
            this.grf = null;

            logger.error(
                `Error loading GRF file ${this.fileName}:`,
                error
            );
        }
    }


    async getFile(filename) {
        if (!this.loaded || !this.grf) {
            logger.error("GRF not loaded or not initialized");
            return null;
        }

        try {
            const { data, error } =
                await this.grf.getFile(filename);

            if (error) {
                return null;
            }

            return Buffer.from(data);

        } catch (error) {
            logger.error(
                `Error extracting file "${filename}": ${error}`
            );

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


    close() {
        if (this.grf && typeof this.grf.close === "function") {
            this.grf.close();
        }

        this.grf = null;
        this.loaded = false;
    }
}


module.exports = Grf;