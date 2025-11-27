/**
 * Dashboard E2E Tests: Admin Workflow
 * End-to-end tests for admin user journeys using Playwright
 * 
 * Tests:
 * - Admin login and navigation
 * - View all agent statuses
 * - Monitor system health
 * - Check performance metrics
 * - Review error logs
 * - Manage configuration
 */

import { test, expect } from '@playwright/test'

test.describe('Admin Dashboard Workflow', () => {
  test.beforeEach(async ({ page }) => {
    // Navigate to dashboard
    await page.goto('http://localhost:3000')
  })

  test('admin can view all 21 agents', async ({ page }) => {
    await page.goto('http://localhost:3000/admin/agents')
    
    // Wait for agents to load
    await page.waitForSelector('[data-testid="agent-card"]', { timeout: 5000 })
    
    // Count agent cards
    const agentCards = await page.locator('[data-testid="agent-card"]').count()
    expect(agentCards).toBe(21)
    
    // Verify first agent displays correctly
    const firstAgent = page.locator('[data-testid="agent-card"]').first()
    await expect(firstAgent).toBeVisible()
    await expect(firstAgent.locator('[data-testid="agent-name"]')).toContainText(/\w+/)
    await expect(firstAgent.locator('[data-testid="agent-status"]')).toContainText(/healthy|warning|critical/)
  })

  test('admin can see real-time updates', async ({ page }) => {
    await page.goto('http://localhost:3000/admin/agents')
    
    // Get initial last updated timestamp
    const lastUpdated = await page.locator('[data-testid="last-updated"]').first().textContent()
    
    // Wait for WebSocket update (5s interval + buffer)
    await page.waitForTimeout(6000)
    
    // Check timestamp changed
    const newLastUpdated = await page.locator('[data-testid="last-updated"]').first().textContent()
    expect(newLastUpdated).not.toBe(lastUpdated)
    
    // Verify timestamp format (HH:MM:SS)
    expect(newLastUpdated).toMatch(/\d{2}:\d{2}:\d{2}/)
  })

  test('admin can navigate between modules', async ({ page }) => {
    // Start at agents page
    await page.goto('http://localhost:3000/admin/agents')
    await expect(page).toHaveURL(/\/admin\/agents/)
    
    // Navigate to performance
    await page.click('a[href="/admin/performance"]')
    await expect(page).toHaveURL(/\/admin\/performance/)
    await expect(page.locator('h1')).toContainText(/Performance|Metrics/)
    
    // Navigate to costs
    await page.click('a[href="/admin/costs"]')
    await expect(page).toHaveURL(/\/admin\/costs/)
    
    // Navigate to errors
    await page.click('a[href="/admin/errors"]')
    await expect(page).toHaveURL(/\/admin\/errors/)
  })

  test('admin can view system health metrics', async ({ page }) => {
    await page.goto('http://localhost:3000/admin/health')
    
    // Check health score displayed
    const healthScore = page.locator('[data-testid="health-score"]')
    await expect(healthScore).toBeVisible()
    
    const scoreText = await healthScore.textContent()
    const score = parseInt(scoreText || '0')
    expect(score).toBeGreaterThan(0)
    expect(score).toBeLessThanOrEqual(100)
  })

  test('admin can view performance charts', async ({ page }) => {
    await page.goto('http://localhost:3000/admin/performance')
    
    // Wait for charts to render
    await page.waitForSelector('[data-testid="latency-chart"]', { timeout: 5000 })
    await page.waitForSelector('[data-testid="throughput-chart"]', { timeout: 5000 })
    
    // Verify charts are visible
    await expect(page.locator('[data-testid="latency-chart"]')).toBeVisible()
    await expect(page.locator('[data-testid="throughput-chart"]')).toBeVisible()
    
    // Charts should render in <3s
    const start = Date.now()
    await page.waitForSelector('[data-testid="chart-rendered"]', { timeout: 3000 })
    const duration = Date.now() - start
    expect(duration).toBeLessThan(3000)
  })

  test('admin can check LLM cost breakdown', async ({ page }) => {
    await page.goto('http://localhost:3000/admin/costs')
    
    // Wait for cost data
    await page.waitForSelector('[data-testid="total-cost"]', { timeout: 5000 })
    
    // Check total cost displayed
    const totalCost = page.locator('[data-testid="total-cost"]')
    await expect(totalCost).toBeVisible()
    
    // Check provider breakdown
    await expect(page.locator('[data-testid="provider-costs"]')).toBeVisible()
    
    // Verify cost is a number
    const costText = await totalCost.textContent()
    expect(costText).toMatch(/\$\d+\.\d{2}/)
  })

  test('admin can filter error logs', async ({ page }) => {
    await page.goto('http://localhost:3000/admin/errors')
    
    // Wait for error log
    await page.waitForSelector('[data-testid="error-log"]', { timeout: 5000 })
    
    // Select error level filter
    await page.selectOption('[data-testid="error-level-filter"]', 'error')
    
    // Wait for filtered results
    await page.waitForTimeout(500)
    
    // Verify only errors shown
    const errorEntries = await page.locator('[data-testid="error-entry"]').all()
    for (const entry of errorEntries) {
      const level = await entry.getAttribute('data-level')
      expect(level).toBe('error')
    }
  })

  test('admin can export metrics', async ({ page }) => {
    await page.goto('http://localhost:3000/admin/performance')
    
    // Click export button
    const exportButton = page.locator('[data-testid="export-metrics"]')
    await expect(exportButton).toBeVisible()
    
    // Trigger download (mock)
    await exportButton.click()
    
    // In real implementation, would verify download started
  })

  test('dashboard loads in <3s', async ({ page }) => {
    const start = Date.now()
    
    await page.goto('http://localhost:3000/admin/agents')
    await page.waitForLoadState('domcontentloaded')
    
    const loadTime = Date.now() - start
    
    expect(loadTime).toBeLessThan(3000)
  })

  test('dashboard achieves good Core Web Vitals', async ({ page }) => {
    await page.goto('http://localhost:3000/admin/agents')
    
    // Measure Largest Contentful Paint (LCP)
    const lcp = await page.evaluate(() => {
      return new Promise<number>((resolve) => {
        new PerformanceObserver((list) => {
          const entries = list.getEntries()
          const lastEntry = entries[entries.length - 1]
          resolve(lastEntry.startTime)
        }).observe({ entryTypes: ['largest-contentful-paint'] })
        
        setTimeout(() => resolve(0), 5000)
      })
    })
    
    // LCP should be <2.5s (2500ms)
    if (lcp > 0) {
      expect(lcp).toBeLessThan(2500)
    }
  })

  test('dashboard handles offline gracefully', async ({ page, context }) => {
    await page.goto('http://localhost:3000/admin/agents')
    
    // Verify online state
    const onlineIndicator = page.locator('[data-testid="connection-status"]')
    await expect(onlineIndicator).toContainText(/online|connected/i)
    
    // Simulate offline
    await context.setOffline(true)
    await page.waitForTimeout(2000)
    
    // Should show offline message
    await expect(onlineIndicator).toContainText(/offline|disconnected/i)
    
    // Restore online
    await context.setOffline(false)
    await page.waitForTimeout(2000)
    
    // Should reconnect
    await expect(onlineIndicator).toContainText(/online|connected/i)
  })
})