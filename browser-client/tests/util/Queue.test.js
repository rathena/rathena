import { describe, it, expect } from 'vitest';  
import Queue from 'Utils/Queue.js';  
  
describe('Queue', () => {  
    it('executes callbacks in order', () => {  
        const order = [];  
        const q = new Queue();  
        q.add(() => { order.push(1); q.next(); });  
        q.add(() => { order.push(2); q.next(); });  
        q.add(() => { order.push(3); });  
        q.run();  
        expect(order).toEqual([1, 2, 3]);  
    });  
  
    it('stops when no next() is called', () => {  
        const order = [];  
        const q = new Queue();  
        q.add(() => { order.push(1); /* no next */ });  
        q.add(() => { order.push(2); });  
        q.run();  
        expect(order).toEqual([1]);  
    });  
  
    it('handles empty queue', () => {  
        const q = new Queue();  
        expect(() => q.run()).not.toThrow();  
    });  
});