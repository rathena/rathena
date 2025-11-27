'use client'

import { useEffect, useRef, useCallback } from 'react'
import { getDashboardWebSocket } from '@/lib/websocket'
import type { WebSocketMessage } from '@/lib/types'

export function useWebSocket() {
  const wsRef = useRef(getDashboardWebSocket())

  useEffect(() => {
    const ws = wsRef.current

    if (!ws.isConnected()) {
      ws.connect()
    }

    return () => {
      ws.disconnect()
    }
  }, [])

  const subscribe = useCallback((eventType: string, handler: (data: any) => void) => {
    return wsRef.current.on(eventType, handler)
  }, [])

  const send = useCallback((message: any) => {
    wsRef.current.send(message)
  }, [])

  return {
    subscribe,
    send,
    isConnected: wsRef.current.isConnected(),
  }
}

export function useWebSocketEvent<T = any>(
  eventType: string,
  handler: (data: T) => void,
  dependencies: any[] = []
) {
  const { subscribe } = useWebSocket()

  useEffect(() => {
    const unsubscribe = subscribe(eventType, handler)
    return unsubscribe
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [eventType, subscribe, ...dependencies])
}