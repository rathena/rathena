import { describe, it, expect } from 'vitest';  
import Inflate from 'Utils/Inflate.js';  
  
describe('Inflate', () => {  
it('decompresses a stored (uncompressed) deflate block', () => {  
    const compressed = new Uint8Array([  
        0x78, 0x01,  
        0x01, 0x05, 0x00, 0xFA, 0xFF,  
        0x48, 0x65, 0x6C, 0x6C, 0x6F,  
        0x05, 0x8D, 0x01, 0xF5  
    ]);  
    const inflater = new Inflate(compressed);  
    const output = new Uint8Array(256);  
    const len = inflater.getBytes(output);  
    const str = String.fromCharCode(...output.slice(0, len));  
    expect(str).toBe('Hello');  
});  
  
it('decompresses fixed Huffman block', () => {  
    const compressed = new Uint8Array([0x78, 0x9c, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0x07, 0x00, 0x05, 0x8c, 0x01, 0xf5]);  
    const inflater = new Inflate(compressed);  
    const output = new Uint8Array(256);  
    const len = inflater.getBytes(output);  
    const str = String.fromCharCode(...output.slice(0, len));  
    expect(str).toBe('Hello');  
});
    it('rejects invalid compression method', () => {  
        const bad = new Uint8Array([0x01, 0x00]);  
        expect(() => new Inflate(bad)).toThrow('Unknown compression method');  
    });  
  
    it('rejects bad FCHECK', () => {  
        // CMF=0x78 is valid, but FLG=0x00 fails: (0x78 << 8) + 0x00 = 0x7800, 0x7800 % 31 = 8 ≠ 0  
        const bad = new Uint8Array([0x78, 0x00]);  
        expect(() => new Inflate(bad)).toThrow('Bad FCHECK');  
    });  
});