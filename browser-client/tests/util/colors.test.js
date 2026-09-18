import { describe, it, expect } from 'vitest';  
import uint32ToRGB from 'Utils/colors.js';  
  
describe('uint32ToRGB', () => {  
    it('converts BGR uint32 to rgb() string', () => {  
        // BGR: R=0xFF (byte 0), G=0x00 (byte 1), B=0x00 (byte 2)  
        expect(uint32ToRGB(0x000000ff)).toBe('rgb(255,0,0)');   // red  
        expect(uint32ToRGB(0x0000ff00)).toBe('rgb(0,255,0)');   // green  
        expect(uint32ToRGB(0x00ff0000)).toBe('rgb(0,0,255)');   // blue  
    });  
  
    it('converts white', () => {  
        expect(uint32ToRGB(0x00ffffff)).toBe('rgb(255,255,255)');  
    });  
  
    it('converts black', () => {  
        expect(uint32ToRGB(0x00000000)).toBe('rgb(0,0,0)');  
    });  
});