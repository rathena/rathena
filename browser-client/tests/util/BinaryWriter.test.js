import { describe, it, expect } from 'vitest';  
import BinaryReader from 'Utils/BinaryReader.js';  
import BinaryWriter from 'Utils/BinaryWriter.js';  
  
describe('BinaryWriter', () => {  
    it('roundtrip Int32', () => {  
        const bw = new BinaryWriter(4);  
        bw.writeInt(-12345);  
        const br = new BinaryReader(bw.buffer);  
        expect(br.readInt()).toBe(-12345);  
    });  
  
    it('roundtrip UInt64', () => {  
        const bw = new BinaryWriter(8);  
        bw.writeUInt64(123456789);  
        const br = new BinaryReader(bw.buffer);  
        expect(br.readUInt64()).toBe(123456789);  
    });  
  
    it('roundtrip position', () => {  
        const bw = new BinaryWriter(3);  
        bw.writePos([150, 250]);  
        // verify bytes were written (3 bytes for pos)  
        expect(bw.offset).toBe(3);  
    });  
});