import { describe, it, expect } from 'vitest';  
import BinaryReader from 'Utils/BinaryReader.js';  
  
describe('BinaryReader', () => {  
    it('should read Int32 little-endian', () => {  
        const buf = new ArrayBuffer(4);  
        new DataView(buf).setInt32(0, 42, true); // LE  
        const br = new BinaryReader(buf);  
        expect(br.readLong()).toBe(42);  
    });  
  
    it('should read Float32', () => {  
        const buf = new ArrayBuffer(4);  
        new DataView(buf).setFloat32(0, 3.14, true);  
        const br = new BinaryReader(buf);  
        expect(br.readFloat()).toBeCloseTo(3.14, 2);  
    });  
  
    it('should seek correctly', () => {  
        const buf = new ArrayBuffer(8);  
        const br = new BinaryReader(buf);  
        br.seek(4);  
        expect(br.tell()).toBe(4);  
    });  
  
    it('should construct from string', () => {  
        let br = new BinaryReader('113e195e6c051bb1cfb12a644bb084c5');  
        expect(br.readBinaryString(32)).toBe('113e195e6c051bb1cfb12a644bb084c5'); //getBinaryString  
        br = new BinaryReader('cb1ea78023d337c38e8ba5124e2338ae');  
        expect(br.readBinaryString(32)).toBe('cb1ea78023d337c38e8ba5124e2338ae'); //getBinaryString 
    });  
  
    it('should read position (getPos)', () => {  
        // Encode known x=100, y=200, dir=5  
        const buf = new ArrayBuffer(3);  
        const x = 100, y = 200, dir = 5;  
        const packed = (x << 14) | (y << 4) | dir;  
        const view = new DataView(buf);  
        view.setUint8(0, (packed >> 16) & 0xff);  
        view.setUint8(1, (packed >> 8) & 0xff);  
        view.setUint8(2, packed & 0xff);  
        const br = new BinaryReader(buf);  
        const [rx, ry, rd] = br.getPos();  
        expect(rx).toBe(100);  
        expect(ry).toBe(200);  
        expect(rd).toBe(5);  
    });  
});