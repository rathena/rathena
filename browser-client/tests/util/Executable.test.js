import { describe, it, expect } from 'vitest';  
import Executable from 'Utils/Executable.js';  
  
describe('Executable', () => {  
    describe('isROExec', () => {  
        it('rejects non-exe files', () => {  
            expect(Executable.isROExec({ name: 'test.txt', size: 4 * 1024 * 1024 })).toBe(false);  
            expect(Executable.isROExec({ name: 'test.grf', size: 4 * 1024 * 1024 })).toBe(false);  
        });  
  
        it('rejects exe files that are too small', () => {  
            expect(Executable.isROExec({ name: 'ragexe.exe', size: 1024 })).toBe(false);  
            expect(Executable.isROExec({ name: 'ragexe.exe', size: 2 * 1024 * 1024 })).toBe(false);  
        });  
  
        it('rejects exe files that are too large', () => {  
            expect(Executable.isROExec({ name: 'ragexe.exe', size: 8 * 1024 * 1024 })).toBe(false);  
        });  
  
        it('accepts exe files in valid size range (3-7 MB)', () => {  
            expect(Executable.isROExec({ name: 'ragexe.exe', size: 4 * 1024 * 1024 })).toBe(true);  
            expect(Executable.isROExec({ name: 'Ragexe.EXE', size: 5 * 1024 * 1024 })).toBe(true);  
        });  
    });  
});