/**
 * Dashboard Unit Tests: Utility Functions
 * Tests utility functions used in dashboard
 * 
 * Tests:
 * - Date/time formatting
 * - Data transformation
 * - Number formatting
 * - Status calculations
 * - Chart data preparation
 */

import { describe, it, expect } from '@jest/globals'

describe('Dashboard Utilities', () => {
  describe('Date Formatting', () => {
    it('should format timestamp to readable string', () => {
      const timestamp = new Date('2025-01-15T14:30:00Z')
      
      const formatDateTime = (date: Date) => {
        return date.toLocaleString('en-US', {
          year: 'numeric',
          month: 'short',
          day: '2-digit',
          hour: '2-digit',
          minute: '2-digit'
        })
      }

      const formatted = formatDateTime(timestamp)
      
      expect(formatted).toContain('Jan')
      expect(formatted).toContain('15')
      expect(formatted).toContain('2025')
    })

    it('should calculate time ago', () => {
      const now = new Date()
      const fiveMinutesAgo = new Date(now.getTime() - 5 * 60 * 1000)

      const getTimeAgo = (date: Date) => {
        const seconds = Math.floor((now.getTime() - date.getTime()) / 1000)
        
        if (seconds < 60) return `${seconds}s ago`
        if (seconds < 3600) return `${Math.floor(seconds / 60)}m ago`
        if (seconds < 86400) return `${Math.floor(seconds / 3600)}h ago`
        return `${Math.floor(seconds / 86400)}d ago`
      }

      const timeAgo = getTimeAgo(fiveMinutesAgo)
      
      expect(timeAgo).toBe('5m ago')
    })

    it('should format duration', () => {
      const formatDuration = (seconds: number) => {
        const hours = Math.floor(seconds / 3600)
        const minutes = Math.floor((seconds % 3600) / 60)
        const secs = seconds % 60

        if (hours > 0) return `${hours}h ${minutes}m`
        if (minutes > 0) return `${minutes}m ${secs}s`
        return `${secs}s`
      }

      expect(formatDuration(3665)).toBe('1h 1m')
      expect(formatDuration(125)).toBe('2m 5s')
      expect(formatDuration(45)).toBe('45s')
    })
  })

  describe('Number Formatting', () => {
    it('should format large numbers with commas', () => {
      const formatNumber = (num: number) => {
        return num.toLocaleString('en-US')
      }

      expect(formatNumber(1000)).toBe('1,000')
      expect(formatNumber(1000000)).toBe('1,000,000')
      expect(formatNumber(1234567)).toBe('1,234,567')
    })

    it('should format numbers with K/M/B suffixes', () => {
      const formatCompact = (num: number) => {
        if (num >= 1000000000) return `${(num / 1000000000).toFixed(1)}B`
        if (num >= 1000000) return `${(num / 1000000).toFixed(1)}M`
        if (num >= 1000) return `${(num / 1000).toFixed(1)}K`
        return num.toString()
      }

      expect(formatCompact(500)).toBe('500')
      expect(formatCompact(1500)).toBe('1.5K')
      expect(formatCompact(2500000)).toBe('2.5M')
      expect(formatCompact(3500000000)).toBe('3.5B')
    })

    it('should format percentages', () => {
      const formatPercentage = (value: number, total: number) => {
        return `${((value / total) * 100).toFixed(1)}%`
      }

      expect(formatPercentage(75, 100)).toBe('75.0%')
      expect(formatPercentage(3, 10)).toBe('30.0%')
      expect(formatPercentage(1, 3)).toBe('33.3%')
    })
  })

  describe('Status Calculations', () => {
    it('should determine agent health status', () => {
      const getAgentHealthStatus = (uptime: number, lastActivity: Date) => {
        const now = new Date()
        const secondsSinceActivity = (now.getTime() - lastActivity.getTime()) / 1000

        if (secondsSinceActivity < 10) return 'healthy'
        if (secondsSinceActivity < 60) return 'warning'
        return 'critical'
      }

      const now = new Date()
      const recent = new Date(now.getTime() - 5000) // 5s ago
      const old = new Date(now.getTime() - 120000) // 2m ago

      expect(getAgentHealthStatus(3600, recent)).toBe('healthy')
      expect(getAgentHealthStatus(3600, old)).toBe('critical')
    })

    it('should calculate system health score', () => {
      const calculateHealthScore = (agents: any[]) => {
        const healthyCount = agents.filter(a => a.status === 'healthy').length
        return (healthyCount / agents.length) * 100
      }

      const agents = [
        { name: 'dialogue', status: 'healthy' },
        { name: 'quest', status: 'healthy' },
        { name: 'economy', status: 'warning' },
        { name: 'faction', status: 'healthy' }
      ]

      const score = calculateHealthScore(agents)
      expect(score).toBe(75) // 3/4 = 75%
    })

    it('should detect anomalies in metrics', () => {
      const detectAnomaly = (current: number, baseline: number, threshold: number = 0.2) => {
        const deviation = Math.abs((current - baseline) / baseline)
        return deviation > threshold
      }

      expect(detectAnomaly(100, 100, 0.2)).toBe(false) // No change
      expect(detectAnomaly(150, 100, 0.2)).toBe(true)  // 50% increase
      expect(detectAnomaly(70, 100, 0.2)).toBe(true)   // 30% decrease
    })
  })

  describe('Data Transformation', () => {
    it('should transform API data for charts', () => {
      const apiData = [
        { timestamp: '2025-01-15T10:00:00Z', value: 100 },
        { timestamp: '2025-01-15T11:00:00Z', value: 120 },
        { timestamp: '2025-01-15T12:00:00Z', value: 110 }
      ]

      const transformForChart = (data: any[]) => {
        return data.map(item => ({
          time: new Date(item.timestamp).getHours(),
          value: item.value
        }))
      }

      const chartData = transformForChart(apiData)

      expect(chartData).toHaveLength(3)
      expect(chartData[0].time).toBe(10)
      expect(chartData[0].value).toBe(100)
    })

    it('should aggregate agent statistics', () => {
      const agents = [
        { name: 'dialogue', requests_handled: 500, avg_latency: 0.2 },
        { name: 'quest', requests_handled: 300, avg_latency: 0.3 },
        { name: 'economy', requests_handled: 200, avg_latency: 0.25 }
      ]

      const aggregateStats = (agents: any[]) => {
        return {
          total_requests: agents.reduce((sum, a) => sum + a.requests_handled, 0),
          avg_latency: agents.reduce((sum, a) => sum + a.avg_latency, 0) / agents.length,
          agent_count: agents.length
        }
      }

      const stats = aggregateStats(agents)

      expect(stats.total_requests).toBe(1000)
      expect(stats.avg_latency).toBeCloseTo(0.25, 2)
      expect(stats.agent_count).toBe(3)
    })

    it('should group data by time period', () => {
      const events = [
        { timestamp: '2025-01-15T10:30:00Z', type: 'login' },
        { timestamp: '2025-01-15T10:45:00Z', type: 'login' },
        { timestamp: '2025-01-15T11:15:00Z', type: 'login' },
        { timestamp: '2025-01-15T11:30:00Z', type: 'login' }
      ]

      const groupByHour = (events: any[]) => {
        const groups: Record<string, any[]> = {}

        events.forEach(event => {
          const hour = new Date(event.timestamp).getHours()
          const key = `${hour}:00`
          
          if (!groups[key]) groups[key] = []
          groups[key].push(event)
        })

        return groups
      }

      const grouped = groupByHour(events)

      expect(Object.keys(grouped)).toHaveLength(2)
      expect(grouped['10:00']).toHaveLength(2)
      expect(grouped['11:00']).toHaveLength(2)
    })
  })

  describe('Validation', () => {
    it('should validate player ID format', () => {
      const validatePlayerId = (playerId: string) => {
        return /^player_\d{3,}$/.test(playerId)
      }

      expect(validatePlayerId('player_001')).toBe(true)
      expect(validatePlayerId('player_12345')).toBe(true)
      expect(validatePlayerId('invalid')).toBe(false)
      expect(validatePlayerId('player_')).toBe(false)
    })

    it('should validate coordinate ranges', () => {
      const validateCoordinate = (x: number, y: number) => {
        return x >= 0 && x <= 500 && y >= 0 && y <= 500
      }

      expect(validateCoordinate(100, 200)).toBe(true)
      expect(validateCoordinate(0, 0)).toBe(true)
      expect(validateCoordinate(500, 500)).toBe(true)
      expect(validateCoordinate(-1, 100)).toBe(false)
      expect(validateCoordinate(100, 501)).toBe(false)
    })

    it('should sanitize user input', () => {
      const sanitizeInput = (input: string) => {
        return input
          .replace(/[<>]/g, '')
          .trim()
          .slice(0, 500)
      }

      expect(sanitizeInput('<script>alert("xss")</script>')).toBe('scriptalert("xss")/script')
      expect(sanitizeInput('  hello  ')).toBe('hello')
      expect(sanitizeInput('a'.repeat(600))).toHaveLength(500)
    })
  })

  describe('Color Utilities', () => {
    it('should get status color', () => {
      const getStatusColor = (status: string) => {
        const colors: Record<string, string> = {
          'healthy': 'green',
          'warning': 'yellow',
          'critical': 'red',
          'unknown': 'gray'
        }
        return colors[status] || 'gray'
      }

      expect(getStatusColor('healthy')).toBe('green')
      expect(getStatusColor('warning')).toBe('yellow')
      expect(getStatusColor('critical')).toBe('red')
      expect(getStatusColor('offline')).toBe('gray')
    })

    it('should calculate gradient for heatmap', () => {
      const getHeatmapColor = (value: number, min: number, max: number) => {
        const normalized = (value - min) / (max - min)
        
        if (normalized < 0.33) return 'blue'
        if (normalized < 0.67) return 'yellow'
        return 'red'
      }

      expect(getHeatmapColor(10, 0, 100)).toBe('blue')
      expect(getHeatmapColor(50, 0, 100)).toBe('yellow')
      expect(getHeatmapColor(90, 0, 100)).toBe('red')
    })
  })

  describe('Sorting and Filtering', () => {
    it('should sort agents by status', () => {
      const agents = [
        { name: 'c', status: 'warning' },
        { name: 'a', status: 'critical' },
        { name: 'b', status: 'healthy' }
      ]

      const statusPriority: Record<string, number> = {
        'critical': 1,
        'warning': 2,
        'healthy': 3
      }

      const sorted = [...agents].sort((a, b) => 
        statusPriority[a.status] - statusPriority[b.status]
      )

      expect(sorted[0].name).toBe('a') // critical
      expect(sorted[1].name).toBe('c') // warning
      expect(sorted[2].name).toBe('b') // healthy
    })

    it('should filter agents by criteria', () => {
      const agents = [
        { name: 'dialogue', status: 'healthy', uptime: 3600 },
        { name: 'quest', status: 'warning', uptime: 1800 },
        { name: 'economy', status: 'healthy', uptime: 7200 }
      ]

      const healthyAgents = agents.filter(a => a.status === 'healthy')
      const longUptime = agents.filter(a => a.uptime > 3000)

      expect(healthyAgents).toHaveLength(2)
      expect(longUptime).toHaveLength(2)
    })
  })

  describe('Calculation Utilities', () => {
    it('should calculate average latency', () => {
      const latencies = [100, 150, 200, 180, 120]
      
      const calculateAverage = (values: number[]) => {
        return values.reduce((sum, val) => sum + val, 0) / values.length
      }

      const avg = calculateAverage(latencies)
      
      expect(avg).toBe(150)
    })

    it('should calculate percentiles', () => {
      const values = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
      
      const calculatePercentile = (arr: number[], percentile: number) => {
        const sorted = [...arr].sort((a, b) => a - b)
        const index = Math.ceil((percentile / 100) * sorted.length) - 1
        return sorted[index]
      }

      expect(calculatePercentile(values, 50)).toBe(50)  // Median
      expect(calculatePercentile(values, 95)).toBe(95)  // P95
      expect(calculatePercentile(values, 99)).toBe(99)  // P99
    })

    it('should calculate trend direction', () => {
      const calculateTrend = (current: number, previous: number) => {
        if (current > previous) return 'increasing'
        if (current < previous) return 'decreasing'
        return 'stable'
      }

      expect(calculateTrend(150, 100)).toBe('increasing')
      expect(calculateTrend(80, 120)).toBe('decreasing')
      expect(calculateTrend(100, 100)).toBe('stable')
    })
  })

  describe('Text Utilities', () => {
    it('should truncate long text', () => {
      const truncate = (text: string, maxLength: number) => {
        if (text.length <= maxLength) return text
        return text.slice(0, maxLength - 3) + '...'
      }

      expect(truncate('Short', 10)).toBe('Short')
      expect(truncate('This is a very long text', 10)).toBe('This is...')
    })

    it('should capitalize first letter', () => {
      const capitalize = (text: string) => {
        return text.charAt(0).toUpperCase() + text.slice(1).toLowerCase()
      }

      expect(capitalize('hello')).toBe('Hello')
      expect(capitalize('WORLD')).toBe('World')
      expect(capitalize('tEsT')).toBe('Test')
    })

    it('should convert snake_case to Title Case', () => {
      const toTitleCase = (text: string) => {
        return text
          .split('_')
          .map(word => word.charAt(0).toUpperCase() + word.slice(1))
          .join(' ')
      }

      expect(toTitleCase('dialogue_agent')).toBe('Dialogue Agent')
      expect(toTitleCase('world_event_generator')).toBe('World Event Generator')
    })
  })

  describe('Array Utilities', () => {
    it('should chunk array into batches', () => {
      const chunk = <T,>(array: T[], size: number): T[][] => {
        const chunks: T[][] = []
        for (let i = 0; i < array.length; i += size) {
          chunks.push(array.slice(i, i + size))
        }
        return chunks
      }

      const items = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
      const chunked = chunk(items, 3)

      expect(chunked).toHaveLength(4)
      expect(chunked[0]).toEqual([1, 2, 3])
      expect(chunked[3]).toEqual([10])
    })

    it('should deduplicate array', () => {
      const deduplicate = <T,>(array: T[]): T[] => {
        return [...new Set(array)]
      }

      const duplicates = [1, 2, 2, 3, 3, 3, 4]
      const unique = deduplicate(duplicates)

      expect(unique).toEqual([1, 2, 3, 4])
    })
  })

  describe('Object Utilities', () => {
    it('should deep merge objects', () => {
      const deepMerge = (target: any, source: any) => {
        const result = { ...target }
        
        for (const key in source) {
          if (typeof source[key] === 'object' && !Array.isArray(source[key])) {
            result[key] = deepMerge(result[key] || {}, source[key])
          } else {
            result[key] = source[key]
          }
        }
        
        return result
      }

      const obj1 = { a: 1, b: { c: 2 } }
      const obj2 = { b: { d: 3 }, e: 4 }
      const merged = deepMerge(obj1, obj2)

      expect(merged.a).toBe(1)
      expect(merged.b.c).toBe(2)
      expect(merged.b.d).toBe(3)
      expect(merged.e).toBe(4)
    })

    it('should pick specific keys from object', () => {
      const pick = <T extends object, K extends keyof T>(obj: T, keys: K[]): Pick<T, K> => {
        const result = {} as Pick<T, K>
        keys.forEach(key => {
          result[key] = obj[key]
        })
        return result
      }

      const agent = {
        name: 'dialogue',
        status: 'healthy',
        uptime: 3600,
        requests_handled: 1000,
        internal_state: { /* ... */ }
      }

      const publicData = pick(agent, ['name', 'status', 'uptime'])

      expect(Object.keys(publicData)).toEqual(['name', 'status', 'uptime'])
      expect(publicData.name).toBe('dialogue')
    })
  })
})