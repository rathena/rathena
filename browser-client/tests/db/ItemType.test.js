import { describe, it, expect } from 'vitest';  
import ItemType from 'DB/Items/ItemType.js';  
  
describe('ItemType', () => {  
    it('has correct item type values', () => {  
        expect(ItemType.HEALING).toBe(0);  
        expect(ItemType.USABLE).toBe(2);  
        expect(ItemType.ETC).toBe(3);  
        expect(ItemType.ARMOR).toBe(4);  
        expect(ItemType.WEAPON).toBe(5);  
        expect(ItemType.CARD).toBe(6);  
        expect(ItemType.PETEGG).toBe(7);  
        expect(ItemType.AMMO).toBe(10);  
    });  
  
    it('SEARCH is 99 (UI filter)', () => {  
        expect(ItemType.SEARCH).toBe(99);  
    });  
});