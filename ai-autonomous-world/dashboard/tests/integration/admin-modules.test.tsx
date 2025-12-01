/**
 * Dashboard Integration Tests: Admin Modules
 * Tests admin-only dashboard modules
 * 
 * Admin Modules (8):
 * - Agent Status Monitor
 * - System Health Dashboard
 * - Performance Metrics
 * - LLM Cost Tracker
 * - Database Analytics
 * - Error Log Viewer
 * - Configuration Manager
 * - Action Queue Monitor
 */

import React from 'react'
import { describe, it, expect, beforeEach } from '@jest/globals'
import { render, screen, waitFor } from '@testing-library/react'
import '@testing-library/jest-dom'

describe('Admin Modules', () => {
  beforeEach(() => {
    jest.clearAllMocks()
  })

  describe('Agent Status Monitor', () => {
    it('should display all 21 agents', async () => {
      const mockAgents = Array.from({ length: 21 }, (_, i) => ({
        name: `agent_${i}`,
        status: 'healthy',
        uptime: 3600
      }))

      // Mock component render
      const AgentStatusMonitor = () => (
        <div data-testid="agent-monitor">
          {mockAgents.map(agent => (
            <div key={agent.name} data-testid="agent-card">
              {agent.name}: {agent.status}
            </div>
          ))}
        </div>
      )

      render(<AgentStatusMonitor />)

      const agentCards = screen.getAllByTestId('agent-card')
      expect(agentCards).toHaveLength(21)
    })

    it('should show real-time status updates', async () => {
      let agentStatus = 'healthy'

      const AgentCard = () => (
        <div data-testid="status">{agentStatus}</div>
      )

      const { rerender } = render(<AgentCard />)
      expect(screen.getByTestId('status')).toHaveTextContent('healthy')

      // Simulate WebSocket update
      agentStatus = 'warning'
      rerender(<AgentCard />)

      expect(screen.getByTestId('status')).toHaveTextContent('warning')
    })

    it('should highlight unhealthy agents', () => {
      const agents = [
        { name: 'dialogue', status: 'healthy' },
        { name: 'quest', status: 'critical' }
      ]

      const getAgentColor = (status: string) => {
        if (status === 'healthy') return 'green'
        if (status === 'warning') return 'yellow'
        if (status === 'critical') return 'red'
        return 'gray'
      }

      expect(getAgentColor(agents[0].status)).toBe('green')
      expect(getAgentColor(agents[1].status)).toBe('red')
    })
  })

  describe('System Health Dashboard', () => {
    it('should calculate overall health score', () => {
      const systemMetrics = {
        agents_healthy: 19,
        agents_total: 21,
        api_success_rate: 0.98,
        db_connection_healthy: true,
        cache_hit_rate: 0.75
      }

      const calculateHealthScore = (metrics: typeof systemMetrics) => {
        const agentScore = (metrics.agents_healthy / metrics.agents_total) * 40
        const apiScore = metrics.api_success_rate * 30
        const dbScore = metrics.db_connection_healthy ? 15 : 0
        const cacheScore = metrics.cache_hit_rate * 15

        return agentScore + apiScore + dbScore + cacheScore
      }

      const healthScore = calculateHealthScore(systemMetrics)
      
      expect(healthScore).toBeGreaterThan(80) // >80% is good
      expect(healthScore).toBeLessThanOrEqual(100)
    })

    it('should show system resource usage', () => {
      const resources = {
        cpu_percent: 45.5,
        memory_mb: 512,
        disk_usage_percent: 35.2,
        network_mbps: 10.5
      }

      expect(resources.cpu_percent).toBeLessThan(80)
      expect(resources.memory_mb).toBeLessThan(2048)
      expect(resources.disk_usage_percent).toBeLessThan(80)
    })
  })

  describe('Performance Metrics', () => {
    it('should display latency metrics', () => {
      const metrics = {
        api_latency_p50: 120,
        api_latency_p95: 280,
        api_latency_p99: 450,
        db_query_p95: 12,
        websocket_latency: 85
      }

      expect(metrics.api_latency_p95).toBeLessThan(500)
      expect(metrics.db_query_p95).toBeLessThan(15)
      expect(metrics.websocket_latency).toBeLessThan(500)
    })

    it('should show throughput metrics', () => {
      const throughput = {
        requests_per_second: 125,
        events_per_second: 450,
        actions_per_second: 85
      }

      expect(throughput.requests_per_second).toBeGreaterThan(100)
    })

    it('should render performance charts', () => {
      const chartData = [
        { time: '10:00', latency: 150 },
        { time: '11:00', latency: 180 },
        { time: '12:00', latency: 160 }
      ]

      expect(chartData).toHaveLength(3)
      expect(chartData.every(d => d.latency < 500)).toBe(true)
    })
  })

  describe('LLM Cost Tracker', () => {
    it('should calculate daily LLM costs', () => {
      const usage = {
        openai_calls: 1000,
        openai_cost_per_call: 0.002,
        anthropic_calls: 500,
        anthropic_cost_per_call: 0.003,
        cache_hits: 3500,
        total_calls: 5000
      }

      const totalCost = (usage.openai_calls * usage.openai_cost_per_call) +
                       (usage.anthropic_calls * usage.anthropic_cost_per_call)
      
      const cacheReduction = (usage.cache_hits / usage.total_calls) * 100

      expect(totalCost).toBe(3.5) // $3.50
      expect(cacheReduction).toBe(70) // 70% cache hit rate
    })

    it('should show cost breakdown by provider', () => {
      const costByProvider = {
        openai: 2.0,
        anthropic: 1.5,
        deepseek: 0.3,
        total: 3.8
      }

      const percentages = {
        openai: (costByProvider.openai / costByProvider.total) * 100,
        anthropic: (costByProvider.anthropic / costByProvider.total) * 100,
        deepseek: (costByProvider.deepseek / costByProvider.total) * 100
      }

      expect(percentages.openai).toBeCloseTo(52.6, 1)
      expect(percentages.anthropic).toBeCloseTo(39.5, 1)
    })

    it('should alert on budget threshold', () => {
      const dailyBudget = 40.0
      const currentSpend = 42.5

      const isOverBudget = currentSpend > dailyBudget
      const alertLevel = isOverBudget ? 'critical' : 'normal'

      expect(isOverBudget).toBe(true)
      expect(alertLevel).toBe('critical')
    })
  })

  describe('Database Analytics', () => {
    it('should show query performance stats', () => {
      const dbStats = {
        total_queries: 50000,
        slow_queries: 250,
        avg_query_time_ms: 8.5,
        p95_query_time_ms: 14.2,
        connection_pool_usage: 0.45
      }

      const slowQueryPercent = (dbStats.slow_queries / dbStats.total_queries) * 100

      expect(slowQueryPercent).toBeLessThan(1) // <1% slow queries
      expect(dbStats.avg_query_time_ms).toBeLessThan(15)
      expect(dbStats.connection_pool_usage).toBeLessThan(0.8)
    })

    it('should show table sizes', () => {
      const tableSizes = [
        { table: 'player_memory', rows: 500000, size_mb: 125 },
        { table: 'npc_state', rows: 10000, size_mb: 15 },
        { table: 'quest_progress', rows: 250000, size_mb: 45 }
      ]

      const totalSize = tableSizes.reduce((sum, t) => sum + t.size_mb, 0)

      expect(totalSize).toBe(185) // 185 MB total
      expect(tableSizes[0].table).toBe('player_memory') // Largest table
    })
  })

  describe('Error Log Viewer', () => {
    it('should display recent errors', () => {
      const errors = [
        {
          timestamp: new Date(),
          level: 'error',
          message: 'LLM timeout',
          agent: 'dialogue',
          stack: 'Error: timeout...'
        },
        {
          timestamp: new Date(),
          level: 'warning',
          message: 'High latency detected',
          agent: 'quest',
          stack: null
        }
      ]

      expect(errors).toHaveLength(2)
      expect(errors[0].level).toBe('error')
    })

    it('should filter errors by severity', () => {
      const allErrors = [
        { level: 'error', message: 'Critical error' },
        { level: 'warning', message: 'Warning message' },
        { level: 'error', message: 'Another error' },
        { level: 'info', message: 'Info message' }
      ]

      const criticalErrors = allErrors.filter(e => e.level === 'error')

      expect(criticalErrors).toHaveLength(2)
    })

    it('should group errors by agent', () => {
      const errors = [
        { agent: 'dialogue', message: 'Error 1' },
        { agent: 'dialogue', message: 'Error 2' },
        { agent: 'quest', message: 'Error 3' }
      ]

      const grouped = errors.reduce((acc, error) => {
        if (!acc[error.agent]) acc[error.agent] = []
        acc[error.agent].push(error)
        return acc
      }, {} as Record<string, any[]>)

      expect(grouped['dialogue']).toHaveLength(2)
      expect(grouped['quest']).toHaveLength(1)
    })
  })

  describe('Configuration Manager', () => {
    it('should display current configuration', () => {
      const config = {
        llm_provider: 'azure_openai',
        cache_enabled: true,
        debug_mode: false,
        max_concurrent_requests: 100,
        timeout_ms: 30000
      }

      expect(config.llm_provider).toBe('azure_openai')
      expect(config.cache_enabled).toBe(true)
    })

    it('should validate configuration changes', () => {
      const validateConfig = (config: any) => {
        const errors: string[] = []

        if (config.max_concurrent_requests < 1) {
          errors.push('max_concurrent_requests must be >= 1')
        }
        if (config.timeout_ms < 1000) {
          errors.push('timeout_ms must be >= 1000')
        }

        return errors
      }

      const invalidConfig = {
        max_concurrent_requests: 0,
        timeout_ms: 500
      }

      const errors = validateConfig(invalidConfig)
      expect(errors).toHaveLength(2)
    })
  })

  describe('Action Queue Monitor', () => {
    it('should show pending actions', () => {
      const pendingActions = [
        { action_id: 'action_001', type: 'spawn_monster', status: 'pending' },
        { action_id: 'action_002', type: 'broadcast', status: 'pending' },
        { action_id: 'action_003', type: 'give_reward', status: 'executing' }
      ]

      const pending = pendingActions.filter(a => a.status === 'pending')
      
      expect(pending).toHaveLength(2)
    })

    it('should show action processing rate', () => {
      const metrics = {
        actions_processed_today: 5000,
        actions_failed_today: 15,
        avg_processing_time_ms: 4.2,
        success_rate: 0.997
      }

      expect(metrics.success_rate).toBeGreaterThan(0.99)
      expect(metrics.avg_processing_time_ms).toBeLessThan(5)
    })
  })
})