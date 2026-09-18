import { describe, it, expect } from 'vitest';  
import TownInfo from 'DB/TownInfo.js';  
  
describe('TownInfo', () => {  
    it('has Prontera data', () => {  
        expect(TownInfo['prontera']).toBeDefined();  
        expect(TownInfo['prontera'].length).toBeGreaterThan(0);  
    });  
  
    it('has Payon data', () => {  
        expect(TownInfo['payon']).toBeDefined();  
        const kafra = TownInfo['payon'].find(n => n.Name === 'Kafra Employee');  
        expect(kafra).toBeDefined();  
        expect(kafra.Type).toBe(6);  
    });  
  
    it('einbech has correct NPC positions', () => {  
        const einbech = TownInfo['einbech'];  
        expect(einbech).toBeDefined();  
        expect(einbech[0].Name).toBe('Kafra Employee');  
        expect(einbech[0].X).toBe(181);  
        expect(einbech[0].Y).toBe(132);  
        expect(einbech[0].Type).toBe(6);  
    });  
  
    it('all entries have required fields', () => {  
        for (const [map, npcs] of Object.entries(TownInfo)) {  
            for (const npc of npcs) {  
                expect(npc).toHaveProperty('Name');  
                expect(npc).toHaveProperty('X');  
                expect(npc).toHaveProperty('Y');  
                expect(npc).toHaveProperty('Type');  
                expect(typeof npc.X).toBe('number');  
                expect(typeof npc.Y).toBe('number');  
            }  
        }  
    });  
});