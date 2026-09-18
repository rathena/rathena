import { describe, it, expect } from 'vitest';  
import partyColor from 'Utils/partyColors.js';  
  
describe('partyColors', () => {  
    it('returns deterministic color for same AID', () => {  
        const a = partyColor(12345);  
        const b = partyColor(12345);  
        expect(a.red).toBe(b.red);  
        expect(a.green).toBe(b.green);  
        expect(a.blue).toBe(b.blue);  
    });  
  
    it('returns rgb style string', () => {  
        const c = partyColor(1);  
        expect(c.style).toMatch(/^rgb\(\d+, \d+, \d+\)$/);  
    });  
  
    it('returns values in 0-255 range', () => {  
        const c = partyColor(99999);  
        expect(c.red).toBeGreaterThanOrEqual(0);  
        expect(c.red).toBeLessThanOrEqual(255);  
        expect(c.green).toBeGreaterThanOrEqual(0);  
        expect(c.green).toBeLessThanOrEqual(255);  
    });  
  
    it('different AIDs produce different colors', () => {  
        const a = partyColor(1);  
        const b = partyColor(2);  
        const same = a.red === b.red && a.green === b.green && a.blue === b.blue;  
        expect(same).toBe(false);  
    });  
});