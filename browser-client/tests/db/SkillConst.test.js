import { describe, it, expect } from 'vitest';  
import SK from 'DB/Skills/SkillConst.js';  
  
describe('SkillConst', () => {  
    it('NV_BASIC is 1', () => {  
        expect(SK.NV_BASIC).toBe(1);  
    });  
  
    it('swordman skills are sequential', () => {  
        expect(SK.SM_SWORD).toBe(2);  
        expect(SK.SM_BASH).toBe(5);  
        expect(SK.SM_MAGNUM).toBe(7);  
        expect(SK.SM_ENDURE).toBe(8);  
    });  
  
    it('mage skills are sequential', () => {  
        expect(SK.MG_SRECOVERY).toBe(9);  
        expect(SK.MG_FIREBOLT).toBe(19);  
        expect(SK.MG_THUNDERSTORM).toBe(21);  
    });  
  
    it('acolyte heal is 28', () => {  
        expect(SK.AL_HEAL).toBe(28);  
    });  
  
    it('all values are positive numbers', () => {  
        Object.values(SK).forEach(v => {  
            expect(typeof v).toBe('number');  
            expect(v).toBeGreaterThan(0);  
        });  
    });  
});