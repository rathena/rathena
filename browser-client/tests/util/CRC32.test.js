import { describe, it, expect } from 'vitest';  
import BinaryReader from 'Utils/BinaryReader.js';  
import 'Utils/CRC32.js'; // registra BinaryReader.prototype.CRC32  
  
describe('CRC32', () => {  
    it('computes CRC32 of known data', () => {  
        // "IEND" in PNG has known CRC32 = 0xAE426082  
        const data = new Uint8Array([0x49, 0x45, 0x4E, 0x44]);  
        const br = new BinaryReader(data.buffer);  
        const crc = br.CRC32(0, 4);  
        expect(crc).toBe(0xAE426082);  
    });  
  
    it('computes CRC32 of empty range', () => {  
        const data = new Uint8Array([0x00]);  
        const br = new BinaryReader(data.buffer);  
        const crc = br.CRC32(0, 0);  
        expect(crc).toBe(0); // empty range  
    });  
  
    it('computes CRC32 of "hello"', () => {  
        const str = 'hello';  
        const br = new BinaryReader(str);  
        // Known CRC32 of "hello" = 0x3610A686  
        expect(br.CRC32(0, 5)).toBe(0x3610A686);  
    });  
});