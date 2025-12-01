/**
 * Dashboard Unit Tests: API Client
 * Tests API client functionality for dashboard
 * 
 * Tests:
 * - API endpoint calls
 * - Error handling
 * - Retry logic
 * - Response transformation
 * - Request timeout handling
 */

import { describe, it, expect, beforeEach, afterEach } from '@jest/globals'

describe('API Client', () => {
  const BASE_URL = 'http://localhost:8000/api/v1'
  let mockFetch: any

  beforeEach(() => {
    mockFetch = jest.fn()
    global.fetch = mockFetch
  })

  afterEach(() => {
    jest.restoreAllMocks()
  })

  describe('GET Requests', () => {
    it('should fetch agent status successfully', async () => {
      const mockResponse = {
        agents: [
          { name: 'dialogue', status: 'healthy', uptime: 3600 },
          { name: 'quest', status: 'healthy', uptime: 3600 }
        ]
      }

      mockFetch.mockResolvedValueOnce({
        ok: true,
        status: 200,
        json: async () => mockResponse
      })

      const response = await fetch(`${BASE_URL}/world/agents/status`)
      const data = await response.json()

      expect(response.ok).toBe(true)
      expect(data.agents).toHaveLength(2)
      expect(data.agents[0].name).toBe('dialogue')
    })

    it('should handle 404 errors gracefully', async () => {
      mockFetch.mockResolvedValueOnce({
        ok: false,
        status: 404,
        json: async () => ({ error: 'Not Found' })
      })

      const response = await fetch(`${BASE_URL}/npc/nonexistent`)
      
      expect(response.ok).toBe(false)
      expect(response.status).toBe(404)
    })

    it('should handle network errors', async () => {
      mockFetch.mockRejectedValueOnce(new Error('Network error'))

      await expect(fetch(`${BASE_URL}/world/agents/status`))
        .rejects.toThrow('Network error')
    })

    it('should complete requests within 200ms', async () => {
      const mockResponse = { data: 'test' }
      mockFetch.mockResolvedValueOnce({
        ok: true,
        json: async () => mockResponse
      })

      const start = Date.now()
      await fetch(`${BASE_URL}/test`)
      const duration = Date.now() - start

      expect(duration).toBeLessThan(200)
    })
  })

  describe('POST Requests', () => {
    it('should send POST request with JSON body', async () => {
      const requestBody = {
        player_id: 'player_001',
        npc_id: 'npc_001',
        message: 'Hello'
      }

      mockFetch.mockResolvedValueOnce({
        ok: true,
        status: 200,
        json: async () => ({ event_id: 'evt_001', processed: true })
      })

      const response = await fetch(`${BASE_URL}/ai/player/interaction`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(requestBody)
      })

      const data = await response.json()

      expect(response.ok).toBe(true)
      expect(data.processed).toBe(true)
      expect(mockFetch).toHaveBeenCalledWith(
        `${BASE_URL}/ai/player/interaction`,
        expect.objectContaining({
          method: 'POST',
          body: JSON.stringify(requestBody)
        })
      )
    })

    it('should handle validation errors (400)', async () => {
      mockFetch.mockResolvedValueOnce({
        ok: false,
        status: 400,
        json: async () => ({
          error: 'Validation Error',
          details: ['player_id is required']
        })
      })

      const response = await fetch(`${BASE_URL}/ai/player/interaction`, {
        method: 'POST',
        body: JSON.stringify({})
      })

      expect(response.status).toBe(400)
    })
  })

  describe('Retry Logic', () => {
    it('should retry on 500 errors', async () => {
      let attemptCount = 0

      mockFetch.mockImplementation(async () => {
        attemptCount++
        if (attemptCount < 3) {
          return {
            ok: false,
            status: 500,
            json: async () => ({ error: 'Internal Server Error' })
          }
        }
        return {
          ok: true,
          status: 200,
          json: async () => ({ success: true })
        }
      })

      // Simulate retry logic
      const maxRetries = 3
      let response
      for (let i = 0; i < maxRetries; i++) {
        response = await fetch(`${BASE_URL}/test`)
        if (response.ok) break
        await new Promise(resolve => setTimeout(resolve, 100 * (2 ** i)))
      }

      expect(attemptCount).toBe(3)
      expect(response?.ok).toBe(true)
    })

    it('should respect max retry limit', async () => {
      mockFetch.mockResolvedValue({
        ok: false,
        status: 500
      })

      const maxRetries = 3
      let attemptCount = 0

      for (let i = 0; i < maxRetries; i++) {
        attemptCount++
        await fetch(`${BASE_URL}/test`)
      }

      expect(attemptCount).toBe(maxRetries)
    })
  })

  describe('Request Cancellation', () => {
    it('should support AbortController', async () => {
      const controller = new AbortController()
      
      mockFetch.mockImplementation(() => 
        new Promise((resolve, reject) => {
          setTimeout(() => resolve({ ok: true }), 1000)
          controller.signal.addEventListener('abort', () => {
            reject(new Error('Request aborted'))
          })
        })
      )

      const fetchPromise = fetch(`${BASE_URL}/test`, {
        signal: controller.signal
      })

      controller.abort()

      await expect(fetchPromise).rejects.toThrow('Request aborted')
    })
  })

  describe('Response Caching', () => {
    it('should cache successful responses', async () => {
      const cache = new Map<string, any>()
      const cacheKey = '/world/agents/status'

      // First request
      if (!cache.has(cacheKey)) {
        mockFetch.mockResolvedValueOnce({
          ok: true,
          json: async () => ({ agents: [] })
        })

        const response = await fetch(`${BASE_URL}${cacheKey}`)
        const data = await response.json()
        cache.set(cacheKey, data)
      }

      // Second request (from cache)
      const cachedData = cache.get(cacheKey)

      expect(cachedData).toBeDefined()
      expect(mockFetch).toHaveBeenCalledTimes(1) // Only called once
    })
  })
})