'use client'

import { useState, useEffect } from 'react'
import { DashboardAPI } from '@/lib/api'
import { useWebSocketEvent } from './useWebSocket'
import type { Agent } from '@/lib/types'

export function useAgentStatus() {
  const [agents, setAgents] = useState<Agent[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    async function fetchAgents() {
      try {
        const data = await DashboardAPI.getAllAgents()
        setAgents(data)
        setError(null)
      } catch (err) {
        setError(err instanceof Error ? err.message : 'Failed to fetch agents')
      } finally {
        setLoading(false)
      }
    }

    fetchAgents()
  }, [])

  // Subscribe to real-time agent updates
  useWebSocketEvent('agent_update', (updatedAgent: Agent) => {
    setAgents(prev => 
      prev.map(agent => 
        agent.id === updatedAgent.id ? updatedAgent : agent
      )
    )
  })

  const getAgentsByCategory = (category: string) => {
    return agents.filter(agent => agent.type === category)
  }

  const getActiveAgents = () => {
    return agents.filter(agent => agent.status === 'active')
  }

  const getErrorAgents = () => {
    return agents.filter(agent => agent.status === 'error')
  }

  return {
    agents,
    loading,
    error,
    getAgentsByCategory,
    getActiveAgents,
    getErrorAgents,
  }
}

export function useSingleAgent(agentId: string) {
  const [agent, setAgent] = useState<Agent | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    async function fetchAgent() {
      try {
        const data = await DashboardAPI.getAgentDetails(agentId)
        setAgent(data)
        setError(null)
      } catch (err) {
        setError(err instanceof Error ? err.message : 'Failed to fetch agent')
      } finally {
        setLoading(false)
      }
    }

    fetchAgent()
  }, [agentId])

  // Subscribe to updates for this specific agent
  useWebSocketEvent('agent_update', (updatedAgent: Agent) => {
    if (updatedAgent.id === agentId) {
      setAgent(updatedAgent)
    }
  })

  return { agent, loading, error }
}