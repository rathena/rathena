import {
    openGrf,
    extractFile,
    closeGrf
} from "./eventHorizonCore.mjs";

function normalizePath(value) {
    return String(value ?? "")
        .replace(/\//g, "\\")
        .replace(/^\\+/, "")
        .toLowerCase();
}

export class EventHorizonGrf {

    constructor(filePath) {
        this.filePath = filePath;

        this.grf = null;

        // Compatibilidade com GrfNode.
        // O grfController chama this.grf.files.keys().
        this.files = new Map();

        this._entries = new Map();
    }

    async load() {
        this.grf = openGrf(this.filePath);

        for (const entry of this.grf.files ?? []) {
            const originalName =
                entry?.name ??
                entry?.filename ??
                entry?.path;

            if (!originalName) {
                continue;
            }

            const normalized = normalizePath(originalName);

            /*
             * O extractor apresenta nomes começando por data\.
             * roBrowser normalmente procura:
             *
             * texture\...
             *
             * Portanto indexamos as duas formas.
             */
            this._entries.set(normalized, entry);
            this.files.set(originalName, entry);

            if (normalized.startsWith("data\\")) {
                const withoutData = normalized.substring(5);

                this._entries.set(withoutData, entry);

                const displayWithoutData =
                    originalName.replace(/^data[\\/]/i, "");

                this.files.set(displayWithoutData, entry);
            }
        }
    }

    async getFile(filename) {
        if (!this.grf) {
            return {
                data: null,
                error: new Error("Event Horizon GRF is not loaded")
            };
        }

        const normalized = normalizePath(filename);

        let entry = this._entries.get(normalized);

        // Aceita também chamadas explicitamente começando em data\
        if (!entry && normalized.startsWith("data\\")) {
            entry = this._entries.get(normalized.substring(5));
        }

        if (!entry) {
            return {
                data: null,
                error: new Error(`File not found in Event Horizon GRF: ${filename}`)
            };
        }

        try {
            const data = extractFile(this.grf, entry);

            return {
                data,
                error: null
            };
        } catch (error) {
            return {
                data: null,
                error
            };
        }
    }

    close() {
        if (this.grf) {
            closeGrf(this.grf);
            this.grf = null;
        }
    }
}