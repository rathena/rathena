import { describe, it, expect } from 'vitest';

class MockFileReaderSync {
    readAsArrayBuffer(blob) {
        const copy = new Uint8Array(blob.length);
        copy.set(blob);
        return copy.buffer;
    }
}
globalThis.FileReaderSync = MockFileReaderSync;

import GRF from 'Loaders/GameFile.js';

function createMockFile(arrayBuffer) {
    const bytes = new Uint8Array(arrayBuffer);
    return {
        size: arrayBuffer.byteLength,
        slice(start, end) {
            return bytes.slice(start, end);
        }
    };
}

describe('GRF Loader', () => {
    it('rejects file too small for header', () => {
        const smallFile = { size: 10, slice: () => new Uint8Array(0) };
        expect(() => new GRF(smallFile)).toThrow('Not enough bytes');
    });

    it('rejects invalid signature', () => {
        const buf = new ArrayBuffer(GRF.struct_header.size);
        const file = createMockFile(buf);
        expect(() => new GRF(file)).toThrow('Incorrect header');
    });

    it('struct sizes are correct', () => {
        // header: 15+15+4+4+4+4 = 46
        expect(GRF.struct_header.size).toBe(46);
        // table: 4+4 = 8
        expect(GRF.struct_table.size).toBe(8);
        // entry: 4+4+4+1+4 = 17
        expect(GRF.struct_entry.size).toBe(17);
    });

    it('has expected constants', () => {
        expect(GRF.VERSION_200).toBe(0x200);
        expect(GRF.VERSION_300).toBe(0x300);
        expect(GRF.SIG_MAGIC).toBe('Master of Magic');
        expect(GRF.FILELIST_TYPE_FILE).toBe(0x01);
        expect(GRF.FILELIST_TYPE_ENCRYPT_MIXED).toBe(0x02);
        expect(GRF.FILELIST_TYPE_ENCRYPT_HEADER).toBe(0x04);
    });
});