import { describe, it, expect } from 'vitest';  
import EC from 'DB/EmotionsConst.js';  
  
describe('EmotionsConst', () => {  
    it('has correct base emotion IDs', () => {  
        expect(EC.ET_SURPRISE).toBe(0);  
        expect(EC.ET_QUESTION).toBe(1);  
        expect(EC.ET_DELIGHT).toBe(2);  
        expect(EC.ET_THROB).toBe(3);  
        expect(EC.ET_BIGTHROB).toBe(4);  
        expect(EC.ET_SWEAT).toBe(5);  
    });  
  
    it('has correct extended emotion IDs', () => {  
        expect(EC.ET_OK).toBe(33);  
        expect(EC.ET_WHISP).toBe(79);  
    });  
  
    it('all values are unique numbers', () => {  
        const values = Object.values(EC);  
        const unique = new Set(values);  
        expect(unique.size).toBe(values.length);  
        values.forEach(v => expect(typeof v).toBe('number'));  
    });  
});