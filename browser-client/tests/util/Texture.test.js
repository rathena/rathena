import { describe, it, expect, vi, beforeEach } from 'vitest';  
  
// We need to mock canvas before Texture.js loads,  
// because it creates procCanvas/procCtx at module level.  
  
let capturedImageData = null;  
let putImageDataCalls = [];  
  
const mockCtx = {  
    clearRect: vi.fn(),  
    drawImage: vi.fn(),  
    getImageData: vi.fn((x, y, w, h) => {  
        // Return the capturedImageData set by each test  
        return capturedImageData;  
    }),  
    putImageData: vi.fn((imageData) => {  
        putImageDataCalls.push(imageData);  
    }),  
    createImageData: vi.fn((w, h) => ({  
        width: w,  
        height: h,  
        data: new Uint8ClampedArray(w * h * 4)  
    }))  
};  
  
// Override document.createElement to return mock canvas  
const originalCreateElement = document.createElement.bind(document);  
document.createElement = vi.fn((tag) => {  
    if (tag === 'canvas') {  
        return {  
            width: 0,  
            height: 0,  
            getContext: vi.fn(() => mockCtx),  
            toDataURL: vi.fn(() => 'data:image/png;base64,mock')  
        };  
    }  
    return originalCreateElement(tag);  
});  
  
// Now import Texture — it will get our mocked canvas  
const { default: Texture } = await import('Utils/Texture.js');  
  
describe('Texture.removeMagenta', () => {  
    beforeEach(() => {  
        capturedImageData = null;  
        putImageDataCalls = [];  
        mockCtx.clearRect.mockClear();  
        mockCtx.drawImage.mockClear();  
        mockCtx.getImageData.mockClear();  
        mockCtx.putImageData.mockClear();  
    });  
  
    function makeImageData(width, height, pixels) {  
        // pixels is array of [R, G, B, A] tuples  
        const data = new Uint8ClampedArray(width * height * 4);  
        for (let i = 0; i < pixels.length; i++) {  
            data[i * 4 + 0] = pixels[i][0]; // R  
            data[i * 4 + 1] = pixels[i][1]; // G  
            data[i * 4 + 2] = pixels[i][2]; // B  
            data[i * 4 + 3] = pixels[i][3]; // A  
        }  
        return { width, height, data };  
    }  
  
    function mockCanvas(w, h) {  
        return {  
            width: w,  
            height: h,  
            getContext: vi.fn(() => mockCtx)  
        };  
    }  
  
    it('removes pure magenta pixels (255, 0, 255)', () => {  
        const imgData = makeImageData(2, 1, [  
            [255, 0, 255, 255],   // pure magenta → should be zeroed  
            [100, 200, 50, 255],  // normal pixel → should stay  
        ]);  
        capturedImageData = imgData;  
  
        const canvas = mockCanvas(2, 1);  
        Texture.removeMagenta(canvas);  
  
        // Magenta pixel should be fully transparent  
        expect(imgData.data[0]).toBe(0);  // R  
        expect(imgData.data[1]).toBe(0);  // G  
        expect(imgData.data[2]).toBe(0);  // B  
        expect(imgData.data[3]).toBe(0);  // A  
  
        // Normal pixel should be untouched  
        expect(imgData.data[4]).toBe(100);  
        expect(imgData.data[5]).toBe(200);  
        expect(imgData.data[6]).toBe(50);  
        expect(imgData.data[7]).toBe(255);  
    });  
  
    it('removes near-magenta pixels (R>230, G<20, B>230)', () => {  
        const imgData = makeImageData(1, 3, [  
            [231, 19, 231, 255],  // just above threshold → removed  
            [240, 0, 250, 200],   // magenta with partial alpha → removed  
            [255, 0, 255, 128],   // magenta with half alpha → removed  
        ]);  
        capturedImageData = imgData;  
  
        Texture.removeMagenta(mockCanvas(1, 3));  
  
        // All three should be zeroed  
        for (let i = 0; i < 3; i++) {  
            expect(imgData.data[i * 4 + 0]).toBe(0);  
            expect(imgData.data[i * 4 + 1]).toBe(0);  
            expect(imgData.data[i * 4 + 2]).toBe(0);  
            expect(imgData.data[i * 4 + 3]).toBe(0);  
        }  
    });  
  
    it('preserves pixels at threshold boundary', () => {  
        const imgData = makeImageData(1, 4, [  
            [230, 0, 255, 255],   // R=230 (not > 230) → kept  
            [255, 20, 255, 255],  // G=20 (not < 20) → kept  
            [255, 0, 230, 255],   // B=230 (not > 230) → kept  
            [200, 0, 200, 255],   // below threshold → kept  
        ]);  
        capturedImageData = imgData;  
  
        Texture.removeMagenta(mockCanvas(1, 4));  
  
        // All should be preserved  
        expect(imgData.data[0]).toBe(230);   // R=230 kept  
        expect(imgData.data[4 + 1]).toBe(20); // G=20 kept  
        expect(imgData.data[8 + 2]).toBe(230); // B=230 kept  
        expect(imgData.data[12]).toBe(200);   // R=200 kept  
    });  
  
    it('handles all-magenta image', () => {  
        const pixels = Array(4).fill([255, 0, 255, 255]);  
        const imgData = makeImageData(2, 2, pixels);  
        capturedImageData = imgData;  
  
        Texture.removeMagenta(mockCanvas(2, 2));  
  
        for (let i = 0; i < imgData.data.length; i++) {  
            expect(imgData.data[i]).toBe(0);  
        }  
    });  
  
    it('handles image with no magenta', () => {  
        const imgData = makeImageData(2, 1, [  
            [128, 128, 128, 255],  
            [0, 0, 0, 255],  
        ]);  
        capturedImageData = imgData;  
        const original = new Uint8ClampedArray(imgData.data);  
  
        Texture.removeMagenta(mockCanvas(2, 1));  
  
        expect(imgData.data).toEqual(original);  
    });  
  
    it('resizes procCanvas when dimensions differ', () => {  
        const imgData = makeImageData(1, 1, [[0, 0, 0, 255]]);  
        capturedImageData = imgData;  
  
        Texture.removeMagenta(mockCanvas(64, 64));  
  
        expect(mockCtx.clearRect).toHaveBeenCalledWith(0, 0, 64, 64);  
        expect(mockCtx.getImageData).toHaveBeenCalledWith(0, 0, 64, 64);  
    });  
  
    it('calls putImageData on the input canvas context', () => {  
        const imgData = makeImageData(1, 1, [[255, 0, 255, 255]]);  
        capturedImageData = imgData;  
  
        const canvas = mockCanvas(1, 1);  
        Texture.removeMagenta(canvas);  
  
        // putImageData should be called (on the canvas's context)  
        expect(mockCtx.putImageData).toHaveBeenCalledWith(imgData, 0, 0);  
    });  
});