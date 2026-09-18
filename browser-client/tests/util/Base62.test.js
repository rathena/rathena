import { describe, it, expect } from 'vitest';  
import Base62 from 'Utils/Base62.js';  
  
describe('Base62', () => {  
    it('encode/decode roundtrip', () => {  
        expect(Base62.decode(Base62.encode(12345))).toBe(12345);  
    });  
  
    it('encode 0 returns empty string', () => {  
        expect(Base62.encode(0)).toBe('');  
    });  
  
    it('decode known value', () => {  
        expect(Base62.decode('1')).toBe(1);  
        expect(Base62.decode('a')).toBe(10);  
        expect(Base62.decode('A')).toBe(36);  
    });  
});