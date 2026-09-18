import { describe, it, expect } from 'vitest';  
import Emotions from 'DB/Emotions.js';  
  
describe('Emotions', () => {  
    it('maps "!" command to ET_SURPRISE (index 0)', () => {  
        expect(Emotions.commands['!']).toBe(0);  
    });  
  
    it('maps "?" command to ET_QUESTION (index 1)', () => {  
        expect(Emotions.commands['?']).toBe(1);  
    });  
  
    it('maps "lv" command to ET_THROB (index 3)', () => {  
        expect(Emotions.commands['lv']).toBe(3);  
    });  
  
    it('maps "$" command to ET_MONEY (index 8)', () => {  
        expect(Emotions.commands['$']).toBe(8);  
    });  
  
    it('maps "gg" command to ET_KIK (index 29)', () => {  
        expect(Emotions.commands['gg']).toBe(29);  
    });  
  
    it('has sprite index for ET_SURPRISE', () => {  
        expect(Emotions.indexes[0]).toBe(0);  
    });  
  
    it('has sprite index for ET_BIGTHROB', () => {  
        expect(Emotions.indexes[14]).toBe(4);  
    });  
  
    it('has name for sprite index 0', () => {  
        expect(Emotions.names[0]).toBe('!');  
    });  
  
    it('has interface order mapping', () => {  
        // order maps interface_index → sprite_index  
        expect(Emotions.order[0]).toBe(0);  // interface 0 → sprite 0  
        expect(Emotions.order[4]).toBe(4);  // interface 4 → sprite 4 (bigthrob)  
    });  
  
    it('all commands resolve to valid emotion indices', () => {  
        for (const [cmd, idx] of Object.entries(Emotions.commands)) {  
            expect(typeof idx).toBe('number');  
            expect(idx).toBeGreaterThanOrEqual(0);  
        }  
    });  
});