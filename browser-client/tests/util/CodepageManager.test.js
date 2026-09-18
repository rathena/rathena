import { describe, it, expect, beforeEach, vi } from 'vitest';  
import TextEncoding from 'Utils/CodepageManager.js';  
  
describe('CodepageManager', () => {  
  
    // Reset charset before each test to avoid state leaking  
    beforeEach(() => {  
        TextEncoding.warned = false;  
        TextEncoding.setCharset('windows-1252');  
    });  
  
    // ─── detectEncodingByLangtype ────────────────────────────  
  
    describe('detectEncodingByLangtype', () => {  
        const detect = TextEncoding.detectEncodingByLangtype;  
  
        it('SERVICETYPE_KOREA (0) → windows-949', () => {  
            expect(detect(0x00)).toBe('windows-949');  
        });  
  
        it('SERVICETYPE_AMERICA (1) → windows-1252', () => {  
            expect(detect(0x01)).toBe('windows-1252');  
        });  
  
        it('SERVICETYPE_JAPAN (2) → shift-jis', () => {  
            expect(detect(0x02)).toBe('shift-jis');  
        });  
  
        it('SERVICETYPE_CHINA (3) → gbk', () => {  
            expect(detect(0x03)).toBe('gbk');  
        });  
  
        it('SERVICETYPE_TAIWAN (4) → big5', () => {  
            expect(detect(0x04)).toBe('big5');  
        });  
  
        it('SERVICETYPE_THAI (5) → windows-874', () => {  
            expect(detect(0x05)).toBe('windows-874');  
        });  
  
        it('SERVICETYPE_BRAZIL (12) → windows-1252', () => {  
            expect(detect(0x0c)).toBe('windows-1252');  
        });  
  
        it('SERVICETYPE_RUSSIA (14) → windows-1251', () => {  
            expect(detect(0x0e)).toBe('windows-1251');  
        });  
  
        it('SERVICETYPE_VIETNAM (15) → windows-1258', () => {  
            expect(detect(0x0f)).toBe('windows-1258');  
        });  
  
        it('SERVICETYPE_FRANCE (18) → windows-1252', () => {  
            expect(detect(0x12)).toBe('windows-1252');  
        });  
  
        it('SERVICETYPE_UAE (19) → windows-1256', () => {  
            expect(detect(0x13)).toBe('windows-1256');  
        });  
  
        it('custom Central European (160) → windows-1250', () => {  
            expect(detect(0xa0)).toBe('windows-1250');  
        });  
  
        it('custom Greek (161) → windows-1253', () => {  
            expect(detect(0xa1)).toBe('windows-1253');  
        });  
  
        it('custom Turkish (162) → windows-1254', () => {  
            expect(detect(0xa2)).toBe('windows-1254');  
        });  
  
        it('custom Hebrew (163) → windows-1255', () => {  
            expect(detect(0xa3)).toBe('windows-1255');  
        });  
  
        it('custom Baltic (164) → windows-1257', () => {  
            expect(detect(0xa4)).toBe('windows-1257');  
        });  
  
        it('custom UTF-8 (240) → utf-8', () => {  
            expect(detect(0xf0)).toBe('utf-8');  
        });  
  
        it('custom UTF-16LE (241) → utf-16le', () => {  
            expect(detect(0xf1)).toBe('utf-16le');  
        });  
  
        it('custom UTF-16BE (242) → utf-16be', () => {  
            expect(detect(0xf2)).toBe('utf-16be');  
        });  
  
        it('unknown langtype defaults to windows-1252', () => {  
            expect(detect(0xff)).toBe('windows-1252');  
            expect(detect(999)).toBe('windows-1252');  
        });  
  
        it('disableKorean forces windows-1252 regardless of langtype', () => {  
            expect(detect(0x00, true)).toBe('windows-1252'); // Korea → forced 1252  
            expect(detect(0x02, true)).toBe('windows-1252'); // Japan → forced 1252  
            expect(detect(0x0e, true)).toBe('windows-1252'); // Russia → forced 1252  
        });  
  
        // All latin-based services share windows-1252  
        it('Indonesia through Australia all map to windows-1252', () => {  
            for (let lt = 0x06; lt <= 0x0d; lt++) {  
                expect(detect(lt)).toBe('windows-1252');  
            }  
        });  
    });  
  
    // ─── setCharset ──────────────────────────────────────────  
  
    describe('setCharset', () => {  
        it('sets userCharset for supported charsets', () => {  
            TextEncoding.setCharset('windows-949');  
            expect(TextEncoding.userCharset).toBe('windows-949');  
        });  
  
        it('sets userCharset for non-default but valid charsets', () => {  
            TextEncoding.setCharset('shift-jis');  
            expect(TextEncoding.userCharset).toBe('shift-jis');  
        });  
    });  
  
    // ─── encode / decode roundtrip ───────────────────────────  
  
    describe('encode and decode', () => {  
        it('roundtrip ASCII with windows-1252', () => {  
            TextEncoding.setCharset('windows-1252');  
            const encoded = TextEncoding.encode('Olá Mundo');  
            const decoded = TextEncoding.decode(encoded);  
            expect(decoded).toBe('Olá Mundo');  
        });  
  
        it('roundtrip Korean text with windows-949', () => {  
            TextEncoding.setCharset('windows-949');  
            const text = '포링';  // "Poring" in Korean  
            const encoded = TextEncoding.encode(text);  
            expect(encoded).toBeInstanceOf(Uint8Array);  
            expect(encoded.length).toBeGreaterThan(0);  
            const decoded = TextEncoding.decode(encoded);  
            expect(decoded).toBe(text);  
        });  
  
        it('roundtrip Japanese text with shift-jis', () => {  
            TextEncoding.setCharset('shift-jis');  
            const text = 'ポリン';  // "Poring" in Japanese  
            const encoded = TextEncoding.encode(text);  
            const decoded = TextEncoding.decode(encoded);  
            expect(decoded).toBe(text);  
        });  
  
        it('roundtrip Russian text with windows-1251', () => {  
            TextEncoding.setCharset('windows-1251');  
            const text = 'Привет';  // "Hello" in Russian  
            const encoded = TextEncoding.encode(text);  
            const decoded = TextEncoding.decode(encoded);  
            expect(decoded).toBe(text);  
        });  
  
        it('roundtrip Chinese text with gbk', () => {  
            TextEncoding.setCharset('gbk');  
            const text = '波利';  // "Poring" in Chinese  
            const encoded = TextEncoding.encode(text);  
            const decoded = TextEncoding.decode(encoded);  
            expect(decoded).toBe(text);  
        });  
  
        it('roundtrip Thai text with windows-874', () => {  
            TextEncoding.setCharset('windows-874');  
            const text = 'สวัสดี';  // "Hello" in Thai  
            const encoded = TextEncoding.encode(text);  
            const decoded = TextEncoding.decode(encoded);  
            expect(decoded).toBe(text);  
        });  
  
        it('encode with explicit charset parameter', () => {  
            TextEncoding.setCharset('windows-1252');  
            const encoded = TextEncoding.encode('포링', 'windows-949');  
            const decoded = TextEncoding.decode(encoded, 'windows-949');  
            expect(decoded).toBe('포링');  
        });  
    });  
  
    // ─── decode with charset parameter ───────────────────────  
  
describe('decode', () => {  
    it('returns empty string for non-Uint8Array input', () => {  
        const spy = vi.spyOn(console, 'error').mockImplementation(() => {});  
        expect(TextEncoding.decode('not a uint8array')).toBe('');  
        expect(TextEncoding.decode(123)).toBe('');  
        expect(spy).toHaveBeenCalledTimes(2);  
        spy.mockRestore();  
    });  
  
    it('returns empty string for invalid charset', () => {  
        const spy = vi.spyOn(console, 'error').mockImplementation(() => {});  
        const data = new Uint8Array([72, 101, 108, 108, 111]);  
        expect(TextEncoding.decode(data, 'invalid-charset-xyz')).toBe('');  
        expect(spy).toHaveBeenCalledTimes(1);  
        spy.mockRestore();  
    });  
});  
  
    // ─── encode edge cases ───────────────────────────────────  
    
describe('encode edge cases', () => {  
    it('returns empty Uint8Array for non-string input', () => {  
        const spy = vi.spyOn(console, 'error').mockImplementation(() => {});  
        const result = TextEncoding.encode(123);  
        expect(result).toBeInstanceOf(Uint8Array);  
        expect(result.length).toBe(0);  
        expect(spy).toHaveBeenCalledTimes(1);  
        spy.mockRestore();  
    });  
  
    it('returns empty Uint8Array for invalid charset', () => {  
        const spy = vi.spyOn(console, 'error').mockImplementation(() => {});  
        const result = TextEncoding.encode('test', 'invalid-charset-xyz');  
        expect(result).toBeInstanceOf(Uint8Array);  
        expect(result.length).toBe(0);  
        expect(spy).toHaveBeenCalledTimes(1);  
        spy.mockRestore();  
    });  
});
    // ─── smartDecode (via charset='utf-8') ───────────────────  
  
    describe('smartDecode (utf-8 fallback)', () => {  
        it('decodes valid UTF-8 as UTF-8', () => {  
            TextEncoding.setCharset('windows-949');  
            // Valid UTF-8 for "Hello"  
            const bytes = new Uint8Array([0x48, 0x65, 0x6c, 0x6c, 0x6f]);  
            const result = TextEncoding.decode(bytes, 'utf-8');  
            expect(result).toBe('Hello');  
        });  
  
        it('falls back to userCharset when UTF-8 produces replacement chars', () => {  
            TextEncoding.setCharset('windows-949');  
            // Encode Korean text in windows-949 (not valid UTF-8)  
            const koreanBytes = TextEncoding.encode('포링', 'windows-949');  
            // Decode with utf-8 charset → should fallback to windows-949  
            const result = TextEncoding.decode(koreanBytes, 'utf-8');  
            expect(result).toBe('포링');  
        });  
  
        it('falls back to windows-1251 for Russian bytes', () => {  
            TextEncoding.setCharset('windows-1251');  
            const russianBytes = TextEncoding.encode('Привет', 'windows-1251');  
            const result = TextEncoding.decode(russianBytes, 'utf-8');  
            expect(result).toBe('Привет');  
        });  
    });  
  
    // ─── decodeString ────────────────────────────────────────  
  
    describe('decodeString', () => {  
        it('returns empty string for falsy input', () => {  
            expect(TextEncoding.decodeString('')).toBe('');  
            expect(TextEncoding.decodeString(null)).toBe('');  
            expect(TextEncoding.decodeString(undefined)).toBe('');  
        });  
  
        it('decodes raw byte string using current charset', () => {  
            TextEncoding.setCharset('windows-1252');  
            // Latin chars that are the same in windows-1252  
            const result = TextEncoding.decodeString('Hello');  
            expect(result).toBe('Hello');  
        });  
  
        it('decodes Korean escaped string with windows-949', () => {  
            TextEncoding.setCharset('windows-949');  
            // \xbb\xe7\xb0\xfa is "사과" (apple) in EUC-KR/windows-949  
            const result = TextEncoding.decodeString('\xbb\xe7\xb0\xfa');  
            expect(result).toBe('사과');  
        });  
    });  
});