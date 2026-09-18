import { describe, it, expect } from 'vitest';  
import HTMLEntity from 'Utils/HTMLEntity.js';  
  
describe('HTMLEntity', () => {  
    it('decodes basic HTML entities', () => {  
        expect(HTMLEntity.decodeHTMLEntities('&amp;')).toBe('&');  
        expect(HTMLEntity.decodeHTMLEntities('&lt;')).toBe('<');  
        expect(HTMLEntity.decodeHTMLEntities('&gt;')).toBe('>');  
    });  
  
    it('strips script tags', () => {  
        const result = HTMLEntity.decodeHTMLEntities('<script>alert(1)</script>hello');  
        expect(result).toBe('hello');  
    });  
  
    it('returns non-string input as-is', () => {  
        expect(HTMLEntity.decodeHTMLEntities(null)).toBe(null);  
        expect(HTMLEntity.decodeHTMLEntities(undefined)).toBe(undefined);  
    });  
  
    it('handles plain text', () => {  
        expect(HTMLEntity.decodeHTMLEntities('hello world')).toBe('hello world');  
    });  
});