import { describe, it, expect } from 'vitest';  
import Struct from 'Utils/Struct.js';  
  
describe('Struct', () => {  
    it('parses simple struct definition', () => {  
        const s = new Struct('int value', 'float speed');  
        expect(s.size).toBe(8);  
        expect(s._list.value.func).toBe('getInt32');  
        expect(s._list.speed.func).toBe('getFloat32');  
    });  
  
    it('parses unsigned types', () => {  
        const s = new Struct('unsigned short id');  
        expect(s._list.id.func).toBe('getUint16');  
        expect(s.size).toBe(2);  
    });  
  
    it('parses arrays', () => {  
        const s = new Struct('unsigned char name[24]');  
        expect(s._list.name.count).toBe(24);  
        expect(s.size).toBe(24);  
    });  
  
    it('calculates total size with mixed types', () => {  
        const s = new Struct(  
            'unsigned char username[24]',  
            'unsigned char password[24]',  
            'bool stay_connect',  
            'float version',  
            'int tick'  
        );  
        expect(s.size).toBe(24 + 24 + 1 + 4 + 4);  
    });  
  
    it('throws on unknown type', () => {  
        expect(() => new Struct('string name')).toThrow();  
    });
  
    it('handles double type', () => {  
        const s = new Struct('double value');  
        expect(s.size).toBe(8);  
        expect(s._list.value.func).toBe('getFloat64');  
    });  
});