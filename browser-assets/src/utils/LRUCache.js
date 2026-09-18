/**
 * LRU Cache implementation for file content caching
 * Provides O(1) get/set operations with automatic eviction
 */
const crypto = require('crypto');

class LRUCache {
  constructor(maxSize = 5000, maxMemoryMB = 1024) {
    this.maxSize = maxSize;
    this.maxMemoryBytes = maxMemoryMB * 1024 * 1024;
    this.cache = new Map();
    this.currentMemory = 0;
    this.hits = 0;
    this.misses = 0;
  }

  /**
   * Get item from cache
   * @param {string} key - Cache key
   * @returns {{ data: Buffer, etag: string }|null} - Cached content with ETag or null
   */
  get(key) {
    if (!this.cache.has(key)) {
      this.misses++;
      return null;
    }

    // Move to end (most recently used)
    const item = this.cache.get(key);
    this.cache.delete(key);
    this.cache.set(key, item);
    this.hits++;
    return { data: item.data, etag: item.etag };
  }

  /**
   * Set item in cache
   * @param {string} key - Cache key
   * @param {Buffer} data - Content to cache
   */
  set(key, data) {
    if (!data || !Buffer.isBuffer(data)) return;

    const size = data.length;

    // Don't cache files larger than 10% of max memory
    if (size > this.maxMemoryBytes * 0.1) return;

    // Remove if exists
    if (this.cache.has(key)) {
      const old = this.cache.get(key);
      this.currentMemory -= old.size;
      this.cache.delete(key);
    }

    // Evict until we have space
    while (
      (this.cache.size >= this.maxSize || this.currentMemory + size > this.maxMemoryBytes) &&
      this.cache.size > 0
    ) {
      const firstKey = this.cache.keys().next().value;
      const firstItem = this.cache.get(firstKey);
      this.currentMemory -= firstItem.size;
      this.cache.delete(firstKey);
    }

    // Pre-compute ETag at cache time (avoids per-request MD5 hashing)
    const etag = crypto.createHash('md5').update(data).digest('hex').slice(0, 16);

    // Add new item
    this.cache.set(key, { data, size, etag });
    this.currentMemory += size;
  }

  /**
   * Check if key exists in cache
   * @param {string} key - Cache key
   * @returns {boolean}
   */
  has(key) {
    return this.cache.has(key);
  }

  /**
   * Clear the cache
   */
  clear() {
    this.cache.clear();
    this.currentMemory = 0;
  }

  /**
   * Get cache statistics
   * @returns {Object} - Cache stats
   */
  getStats() {
    const hitRate = this.hits + this.misses > 0
      ? ((this.hits / (this.hits + this.misses)) * 100).toFixed(2)
      : 0;

    return {
      size: this.cache.size,
      maxSize: this.maxSize,
      memoryUsedMB: (this.currentMemory / 1024 / 1024).toFixed(2),
      maxMemoryMB: (this.maxMemoryBytes / 1024 / 1024).toFixed(0),
      hits: this.hits,
      misses: this.misses,
      hitRate: `${hitRate}%`,
    };
  }
}

module.exports = LRUCache;
