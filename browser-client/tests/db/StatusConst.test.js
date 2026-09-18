import { describe, it, expect } from 'vitest';  
import SC from 'DB/Status/StatusConst.js';  
  
describe('StatusConst', () => {  
    it('BLANK is -1', () => {  
        expect(SC.BLANK).toBe(-1);  
    });  
  
    it('has correct buff IDs', () => {  
        expect(SC.PROVOKE).toBe(0);  
        expect(SC.ENDURE).toBe(1);  
        expect(SC.BLESSING).toBe(10);  
        expect(SC.INC_AGI).toBe(12);  
        expect(SC.KYRIE).toBe(19);  
        expect(SC.RIDING).toBe(27);  
    });  
  
    it('potion buffs are sequential', () => {  
        expect(SC.ATTHASTE_POTION1).toBe(37);  
        expect(SC.ATTHASTE_POTION2).toBe(38);  
        expect(SC.ATTHASTE_POTION3).toBe(39);  
    });  
});