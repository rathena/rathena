import { describe, it, expect } from 'vitest';  
import EL from 'DB/Items/EquipmentLocation.js';  
  
describe('EquipmentLocation', () => {  
    it('uses correct bit flags (powers of 2)', () => {  
        expect(EL.HEAD_BOTTOM).toBe(1);  
        expect(EL.WEAPON).toBe(2);  
        expect(EL.GARMENT).toBe(4);  
        expect(EL.ACCESSORY1).toBe(8);  
        expect(EL.ARMOR).toBe(16);  
        expect(EL.SHIELD).toBe(32);  
        expect(EL.SHOES).toBe(64);  
        expect(EL.ACCESSORY2).toBe(128);  
        expect(EL.HEAD_TOP).toBe(256);  
        expect(EL.HEAD_MID).toBe(512);  
    });  
  
    it('all values are unique powers of 2', () => {  
        const values = Object.values(EL);  
        values.forEach(v => {  
            expect(v & (v - 1)).toBe(0); // power of 2 check  
            expect(v).toBeGreaterThan(0);  
        });  
        expect(new Set(values).size).toBe(values.length);  
    });  
  
    it('supports bitwise OR for multi-slot equips', () => {  
        const headgear = EL.HEAD_TOP | EL.HEAD_MID | EL.HEAD_BOTTOM;  
        expect(headgear & EL.HEAD_TOP).toBeTruthy();  
        expect(headgear & EL.HEAD_MID).toBeTruthy();  
        expect(headgear & EL.HEAD_BOTTOM).toBeTruthy();  
        expect(headgear & EL.WEAPON).toBeFalsy();  
    });  
});