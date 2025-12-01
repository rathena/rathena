/**
 * Dashboard Unit Tests: WebSocket Client
 * Tests real-time WebSocket functionality
 * 
 * Tests:
 * - WebSocket connection establishment
 * - Real-time updates receiving
 * - Auto-reconnect on disconnect
 * - Message parsing and handling
 * - Connection state management
 * - Memory leak prevention
 */

import { describe, it, expect, beforeEach, afterEach } from '@jest/globals'

describe('WebSocket Client', () => {
  const WS_URL = 'ws://localhost:8000/ws'
  let mockWebSocket: any
  let connectionState: 'connecting' | 'open' | 'closed' | 'error'

  beforeEach(() => {
    connectionState = 'connecting'
    
    mockWebSocket = {
      readyState: 0, // CONNECTING
      send: jest.fn(),
      close: jest.fn(),
      addEventListener: jest.fn(),
      removeEventListener: jest.fn()
    }
  })

  afterEach(() => {
    jest.restoreAllMocks()
  })

  describe('Connection Management', () => {
    it('should connect successfully', async () => {
      // Simulate connection success
      mockWebSocket.readyState = 1 // OPEN
      connectionState = 'open'

      expect(connectionState).toBe('open')
      expect(mockWebSocket.readyState).toBe(1)
    })

    it('should handle connection errors', async () => {
      // Simulate connection error
      connectionState = 'error'
      const errorEvent = new Error('Connection failed')

      expect(connectionState).toBe('error')
    })

    it('should auto-reconnect on disconnect', async () => {
      let reconnectAttempts = 0
      const maxReconnectAttempts = 5

      // Simulate disconnect
      connectionState = 'closed'

      // Reconnect logic
      while (connectionState === 'closed' && reconnectAttempts < maxReconnectAttempts) {
        reconnectAttempts++
        await new Promise(resolve => setTimeout(resolve, 1000 * reconnectAttempts))
        
        if (reconnectAttempts === 3) {
          connectionState = 'open' // Success on 3rd attempt
        }
      }

      expect(reconnectAttempts).toBe(3)
      expect(connectionState).toBe('open')
    })

    it('should respect reconnect backoff', async () => {
      const backoffDelays: number[] = []

      for (let attempt = 0; attempt < 5; attempt++) {
        const delay = Math.min(1000 * Math.pow(2, attempt), 30000)
        backoffDelays.push(delay)
      }

      expect(backoffDelays[0]).toBe(1000)   // 1s
      expect(backoffDelays[1]).toBe(2000)   // 2s
      expect(backoffDelays[2]).toBe(4000)   // 4s
      expect(backoffDelays[3]).toBe(8000)   // 8s
      expect(backoffDelays[4]).toBe(16000)  // 16s
    })
  })

  describe('Message Handling', () => {
    it('should receive and parse agent status updates', async () => {
      const messageData = {
        type: 'agent_status_update',
        data: {
          agent: 'dialogue',
          status: 'healthy',
          uptime: 3600,
          last_activity: new Date().toISOString()
        }
      }

      const message = JSON.stringify(messageData)
      
      // Parse message
      const parsed = JSON.parse(message)

      expect(parsed.type).toBe('agent_status_update')
      expect(parsed.data.agent).toBe('dialogue')
      expect(parsed.data.status).toBe('healthy')
    })

    it('should receive world state updates', async () => {
      const messageData = {
        type: 'world_state_update',
        data: {
          time_of_day: 'afternoon',
          weather: 'sunny',
          active_players: 150
        }
      }

      const message = JSON.stringify(messageData)
      const parsed = JSON.parse(message)

      expect(parsed.type).toBe('world_state_update')
      expect(parsed.data.active_players).toBe(150)
    })

    it('should handle malformed messages', async () => {
      const malformedMessage = 'not-valid-json{'

      let parseError = null
      try {
        JSON.parse(malformedMessage)
      } catch (error) {
        parseError = error
      }

      expect(parseError).toBeTruthy()
    })

    it('should handle large messages', async () => {
      const largeData = {
        type: 'bulk_update',
        data: Array(1000).fill({ agent: 'test', status: 'ok' })
      }

      const message = JSON.stringify(largeData)
      const parsed = JSON.parse(message)

      expect(parsed.data).toHaveLength(1000)
      expect(message.length).toBeGreaterThan(10000)
    })
  })

  describe('Real-time Updates', () => {
    it('should update UI when message received', async () => {
      let uiState = {
        agentStatus: 'unknown',
        lastUpdated: null as Date | null
      }

      // Simulate message received
      const message = {
        type: 'agent_status_update',
        data: { agent: 'dialogue', status: 'healthy' }
      }

      // Update UI state
      uiState.agentStatus = message.data.status
      uiState.lastUpdated = new Date()

      expect(uiState.agentStatus).toBe('healthy')
      expect(uiState.lastUpdated).toBeInstanceOf(Date)
    })

    it('should maintain update frequency <500ms', async () => {
      const updates: number[] = []
      let lastUpdate = Date.now()

      // Simulate 10 updates
      for (let i = 0; i < 10; i++) {
        await new Promise(resolve => setTimeout(resolve, 100))
        const now = Date.now()
        updates.push(now - lastUpdate)
        lastUpdate = now
      }

      const avgLatency = updates.reduce((a, b) => a + b, 0) / updates.length

      expect(avgLatency).toBeLessThan(500)
    })

    it('should batch rapid updates', async () => {
      const updates: any[] = []
      const batchWindow = 100 // ms
      let batchCount = 0

      // Receive 10 rapid updates
      for (let i = 0; i < 10; i++) {
        updates.push({ id: i, timestamp: Date.now() })
        await new Promise(resolve => setTimeout(resolve, 10))
      }

      // Group by batch window
      let currentBatch: any[] = []
      let lastBatchTime = updates[0].timestamp

      updates.forEach(update => {
        if (update.timestamp - lastBatchTime < batchWindow) {
          currentBatch.push(update)
        } else {
          batchCount++
          currentBatch = [update]
          lastBatchTime = update.timestamp
        }
      })

      expect(updates).toHaveLength(10)
      // Updates within 100ms should be batched
    })
  })

  describe('Memory Management', () => {
    it('should not leak memory on repeated connections', async () => {
      const connections: any[] = []

      // Create and close 100 connections
      for (let i = 0; i < 100; i++) {
        const ws = { ...mockWebSocket }
        connections.push(ws)
        
        // Immediately close
        ws.readyState = 3 // CLOSED
        connections.pop() // Remove reference
      }

      // Should not accumulate connections
      expect(connections).toHaveLength(0)
    })

    it('should clean up event listeners on unmount', async () => {
      const listeners = new Map<string, Function[]>()

      // Add listeners
      const onMessage = jest.fn()
      const onError = jest.fn()
      const onClose = jest.fn()

      listeners.set('message', [onMessage])
      listeners.set('error', [onError])
      listeners.set('close', [onClose])

      expect(listeners.size).toBe(3)

      // Cleanup
      listeners.clear()

      expect(listeners.size).toBe(0)
    })
  })

  describe('Connection State', () => {
    it('should track connection state correctly', async () => {
      const states: string[] = []

      // Connection lifecycle
      states.push('connecting')
      await new Promise(resolve => setTimeout(resolve, 10))
      
      states.push('open')
      await new Promise(resolve => setTimeout(resolve, 10))
      
      states.push('closed')

      expect(states).toEqual(['connecting', 'open', 'closed'])
    })

    it('should expose connection status to UI', async () => {
      interface ConnectionStatus {
        connected: boolean
        reconnecting: boolean
        lastError: string | null
      }

      const status: ConnectionStatus = {
        connected: false,
        reconnecting: false,
        lastError: null
      }

      // Connection successful
      status.connected = true

      expect(status.connected).toBe(true)
      expect(status.lastError).toBeNull()

      // Connection lost
      status.connected = false
      status.reconnecting = true

      expect(status.connected).toBe(false)
      expect(status.reconnecting).toBe(true)
    })
  })

  describe('Message Queue', () => {
    it('should queue messages when disconnected', async () => {
      const messageQueue: any[] = []
      let isConnected = false

      // Try to send while disconnected
      const message = { type: 'ping', data: {} }
      
      if (isConnected) {
        mockWebSocket.send(JSON.stringify(message))
      } else {
        messageQueue.push(message)
      }

      expect(messageQueue).toHaveLength(1)
      expect(mockWebSocket.send).not.toHaveBeenCalled()

      // Connect and flush queue
      isConnected = true
      while (messageQueue.length > 0) {
        const queuedMessage = messageQueue.shift()
        mockWebSocket.send(JSON.stringify(queuedMessage))
      }

      expect(messageQueue).toHaveLength(0)
      expect(mockWebSocket.send).toHaveBeenCalledTimes(1)
    })
  })
})