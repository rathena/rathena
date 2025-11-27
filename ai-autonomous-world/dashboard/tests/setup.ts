/**
 * Jest Test Setup
 * Global test configuration and mocks
 */

import '@testing-library/jest-dom'

// Mock Next.js router
jest.mock('next/navigation', () => ({
  useRouter: () => ({
    push: jest.fn(),
    replace: jest.fn(),
    prefetch: jest.fn(),
    back: jest.fn()
  }),
  usePathname: () => '/',
  useSearchParams: () => new URLSearchParams()
}))

// Mock environment variables
process.env.NEXT_PUBLIC_API_URL = 'http://localhost:8000'
process.env.NEXT_PUBLIC_WS_URL = 'ws://localhost:8000/ws'

// Global test timeout
jest.setTimeout(10000)

// Suppress console errors in tests (optional)
global.console = {
  ...console,
  error: jest.fn(),
  warn: jest.fn(),
} as any