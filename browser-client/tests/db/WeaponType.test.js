import { describe, it, expect } from 'vitest';  
import WeaponType from 'DB/Items/WeaponType.js';  
  
describe('WeaponType', () => {  
    it('base weapon types are sequential 0-24', () => {  
        expect(WeaponType.NONE).toBe(0);  
        expect(WeaponType.SHORTSWORD).toBe(1);  
        expect(WeaponType.SWORD).toBe(2);  
        expect(WeaponType.BOW).toBe(11);  
        expect(WeaponType.KATAR).toBe(16);  
        expect(WeaponType.LAST).toBe(24);  
    });  
  
    it('dual wield types start at 25', () => {  
        expect(WeaponType.SHORTSWORD_SHORTSWORD).toBe(25);  
        expect(WeaponType.SWORD_SWORD).toBe(26);  
        expect(WeaponType.SWORD_AXE).toBe(30);  
    });  
  
    it('MAX is 103', () => {  
        expect(WeaponType.MAX).toBe(103);  
    });  
  
    it('all values are non-negative numbers', () => {  
        Object.values(WeaponType).forEach(v => {  
            expect(typeof v).toBe('number');  
            expect(v).toBeGreaterThanOrEqual(0);  
        });  
    });  
});