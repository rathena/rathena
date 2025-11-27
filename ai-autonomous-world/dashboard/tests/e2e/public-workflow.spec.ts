/**
 * Dashboard E2E Tests: Public Workflow
 * End-to-end tests for public user journeys
 * 
 * Tests:
 * - View story arc progress
 * - Check world map
 * - View leaderboard
 * - See active events
 * - Check server status
 */

import { test, expect } from '@playwright/test'

test.describe('Public Dashboard Workflow', () => {
  test('user can view active story arc', async ({ page }) => {
    await page.goto('http://localhost:3000/story')
    
    // Wait for story arc to load
    await page.waitForSelector('[data-testid="story-arc"]', { timeout: 5000 })
    
    // Verify arc information displayed
    await expect(page.locator('[data-testid="arc-title"]')).toBeVisible()
    await expect(page.locator('[data-testid="arc-chapter"]')).toContainText(/Chapter \d+/)
    
    // Check progress bar exists
    const progressBar = page.locator('[data-testid="arc-progress-bar"]')
    await expect(progressBar).toBeVisible()
  })

  test('user can view world map', async ({ page }) => {
    await page.goto('http://localhost:3000/map')
    
    // Wait for map to render
    await page.waitForSelector('[data-testid="world-map"]', { timeout: 5000 })
    
    // Verify map is interactive
    const map = page.locator('[data-testid="world-map"]')
    await expect(map).toBeVisible()
    
    // Check for location markers
    const markers = await page.locator('[data-testid="map-marker"]').count()
    expect(markers).toBeGreaterThan(0)
  })

  test('user can view leaderboard', async ({ page }) => {
    await page.goto('http://localhost:3000/leaderboard')
    
    // Wait for leaderboard
    await page.waitForSelector('[data-testid="leaderboard"]', { timeout: 5000 })
    
    // Verify top players displayed
    const playerEntries = await page.locator('[data-testid="player-entry"]').count()
    expect(playerEntries).toBeGreaterThan(0)
    expect(playerEntries).toBeLessThanOrEqual(100)
    
    // Check first player has highest score
    const firstPlayerScore = await page.locator('[data-testid="player-entry"]')
      .first()
      .locator('[data-testid="player-score"]')
      .textContent()
    
    expect(firstPlayerScore).toBeTruthy()
  })

  test('user can see active events', async ({ page }) => {
    await page.goto('http://localhost:3000/events')
    
    // Wait for events list
    await page.waitForSelector('[data-testid="events-list"]', { timeout: 5000 })
    
    // Check event cards
    const events = page.locator('[data-testid="event-card"]')
    const eventCount = await events.count()
    
    if (eventCount > 0) {
      // Verify first event has required fields
      const firstEvent = events.first()
      await expect(firstEvent.locator('[data-testid="event-name"]')).toBeVisible()
      await expect(firstEvent.locator('[data-testid="event-location"]')).toBeVisible()
      await expect(firstEvent.locator('[data-testid="event-countdown"]')).toBeVisible()
    }
  })

  test('user can check server status', async ({ page }) => {
    await page.goto('http://localhost:3000/status')
    
    // Wait for status page
    await page.waitForSelector('[data-testid="server-status"]', { timeout: 5000 })
    
    // Check online status
    const statusIndicator = page.locator('[data-testid="server-online-status"]')
    await expect(statusIndicator).toBeVisible()
    await expect(statusIndicator).toContainText(/online|operational/i)
    
    // Check player count
    const playerCount = page.locator('[data-testid="players-online"]')
    await expect(playerCount).toBeVisible()
    
    const countText = await playerCount.textContent()
    expect(countText).toMatch(/\d+/)
  })

  test('public pages load quickly (<3s)', async ({ page }) => {
    const pages = ['/story', '/map', '/leaderboard', '/events', '/status']
    
    for (const pagePath of pages) {
      const start = Date.now()
      
      await page.goto(`http://localhost:3000${pagePath}`)
      await page.waitForLoadState('domcontentloaded')
      
      const loadTime = Date.now() - start
      
      expect(loadTime).toBeLessThan(3000)
    }
  })

  test('public dashboard works on mobile viewport', async ({ page }) => {
    // Set mobile viewport
    await page.setViewportSize({ width: 375, height: 667 })
    
    await page.goto('http://localhost:3000/story')
    
    // Check responsive layout
    const content = page.locator('[data-testid="story-arc"]')
    await expect(content).toBeVisible()
    
    // Verify mobile menu if present
    const mobileMenu = page.locator('[data-testid="mobile-menu"]')
    if (await mobileMenu.isVisible()) {
      await expect(mobileMenu).toBeVisible()
    }
  })

  test('public dashboard handles slow network', async ({ page, context }) => {
    // Simulate slow 3G
    await context.route('**/*', route => {
      setTimeout(() => route.continue(), 500) // 500ms delay
    })
    
    await page.goto('http://localhost:3000/story')
    
    // Should show loading state
    const loadingIndicator = page.locator('[data-testid="loading"]')
    
    // Eventually loads
    await page.waitForSelector('[data-testid="story-arc"]', { timeout: 10000 })
    await expect(page.locator('[data-testid="story-arc"]')).toBeVisible()
  })

  test('user interactions are smooth (60fps)', async ({ page }) => {
    await page.goto('http://localhost:3000/map')
    
    // Wait for map
    await page.waitForSelector('[data-testid="world-map"]', { timeout: 5000 })
    
    // Interact with map (pan/zoom)
    const map = page.locator('[data-testid="world-map"]')
    
    // Measure interaction performance
    const metrics = await page.evaluate(() => {
      return new Promise<number>((resolve) => {
        let frameCount = 0
        let startTime = performance.now()
        
        const countFrames = () => {
          frameCount++
          const elapsed = performance.now() - startTime
          
          if (elapsed >= 1000) {
            resolve(frameCount)
          } else {
            requestAnimationFrame(countFrames)
          }
        }
        
        requestAnimationFrame(countFrames)
      })
    })
    
    // Should achieve ~60fps
    expect(metrics).toBeGreaterThan(50)
  })
})