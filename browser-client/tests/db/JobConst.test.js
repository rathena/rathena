import { describe, it, expect } from 'vitest';  
import JobConst from 'DB/Jobs/JobConst.js';  
  
describe('JobConst', () => {  
    it('has correct 1st class IDs', () => {  
        expect(JobConst.NOVICE).toBe(0);  
        expect(JobConst.SWORDMAN).toBe(1);  
        expect(JobConst.MAGICIAN).toBe(2);  
        expect(JobConst.ARCHER).toBe(3);  
        expect(JobConst.ACOLYTE).toBe(4);  
        expect(JobConst.MERCHANT).toBe(5);  
        expect(JobConst.THIEF).toBe(6);  
    });  
  
    it('has correct 2nd class IDs', () => {  
        expect(JobConst.KNIGHT).toBe(7);  
        expect(JobConst.PRIEST).toBe(8);  
        expect(JobConst.WIZARD).toBe(9);  
        expect(JobConst.BLACKSMITH).toBe(10);  
        expect(JobConst.HUNTER).toBe(11);  
        expect(JobConst.ASSASSIN).toBe(12);  
    });  
  
    it('high novice starts at 4001', () => {  
        expect(JobConst.NOVICE_H).toBe(4001);  
        expect(JobConst.SWORDMAN_H).toBe(4002);  
    });  
  
    it('baby novice starts at 4023', () => {  
        expect(JobConst.NOVICE_B).toBe(4023);  
        expect(JobConst.SWORDMAN_B).toBe(4024);  
    });  
});