/**
 * UI/Components/Navigation/PathfindingWorker.js
 *
 * Web Worker for handling pathfinding calculations
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

// Priority queue implementation for A* pathfinding
class PriorityQueue {
	constructor() {
		this.values = [];
	}

	enqueue(val, priority) {
		this.values.push({ val, priority });
		this.sort();
	}

	dequeue() {
		return this.values.shift();
	}

	sort() {
		this.values.sort((a, b) => a.priority - b.priority);
	}
}

// Helper function to get node key
function getKey(x, y) {
	return x + ',' + y;
}

// Helper function to get heuristic distance using Chebyshev Distance
function heuristic(x1, y1, x2, y2) {
	const dx = Math.abs(x1 - x2);
	const dy = Math.abs(y1 - y2);
	// Chebyshev distance: max(dx, dy)
	// Optimal for grid-based maps where diagonal movement costs the same as cardinal movement
	return Math.max(dx, dy);
}

// Helper function to get neighbors
function getNeighbors(x, y, mapData, warps) {
	const neighbors = [];
	const directions = [
		[0, 1], // down
		[1, 0], // right
		[0, -1], // up
		[-1, 0] // left
	];

	// Add normal walkable neighbors
	for (let i = 0; i < directions.length; i++) {
		const newX = x + directions[i][0];
		const newY = y + directions[i][1];

		// Check bounds
		if (newX < 0 || newX >= mapData.width || newY < 0 || newY >= mapData.height) {
			continue;
		}

		// Check if walkable using cell types array
		if (mapData.cellTypes[newX + newY * mapData.width] & mapData.walkableType) {
			neighbors.push({
				x: newX,
				y: newY,
				isWarp: false,
				cost: 1
			});
		}
	}

	// Add warp neighbors if we're close to a warp
	if (warps) {
		for (const warp of warps) {
			// Check if we're close enough to the warp source
			const distanceToWarp = heuristic(x, y, warp.srcX, warp.srcY);
			if (distanceToWarp <= 1) {
				// If we're adjacent to or on the warp
				// Add the warp destination as a neighbor
				neighbors.push({
					x: warp.destX,
					y: warp.destY,
					isWarp: true,
					warpId: warp.id,
					cost: 1 // Cost of using a warp
				});
			}
		}
	}

	return neighbors;
}

// A* pathfinding implementation with warp support
function findPath(startX, startY, endX, endY, mapData, existingPath) {
	startX = Math.floor(startX);
	startY = Math.floor(startY);
	endX = Math.floor(endX);
	endY = Math.floor(endY);

	// If start and end are the same
	if (startX === endX && startY === endY) {
		return [];
	}

	// Check if end point is walkable
	if (!(mapData.cellTypes[endX + endY * mapData.width] & mapData.walkableType)) {
		return [];
	}

	// If we have an existing path, check if start point is close to the existing path start point
	if (existingPath && existingPath.length > 0) {
		const existingStartX = existingPath[0].x;
		const existingStartY = existingPath[0].y;
		const distanceToExistingStart = heuristic(startX, startY, existingStartX, existingStartY);

		// If start point is very close to the existing path start, just return the existing path
		const CLOSE_THRESHOLD = 3; // Consider points within 3 cells as "close"
		if (distanceToExistingStart <= CLOSE_THRESHOLD) {
			return existingPath;
		}
	}

	// If we have an existing path, create a lookup for faster checking
	const existingPathLookup = new Map();
	if (existingPath && existingPath.length > 0) {
		for (let i = 0; i < existingPath.length; i++) {
			const point = existingPath[i];
			existingPathLookup.set(getKey(point.x, point.y), i);
		}
	}

	const openSet = new PriorityQueue();
	const closedSet = new Set();
	const cameFrom = new Map();
	const gScore = new Map();
	const fScore = new Map();

	// Initialize start node
	const startKey = getKey(startX, startY);
	openSet.enqueue([startX, startY], 0);
	gScore.set(startKey, 0);
	fScore.set(startKey, heuristic(startX, startY, endX, endY));

	// Get warps for the current map if available
	const warps = mapData.warps || [];

	// Minimum percentage of the path that must be recalculated
	const MIN_RECALCULATION_PERCENTAGE = 0.25;
	const minRecalculationPoints =
		existingPath && existingPath.length > 0 ? Math.floor(existingPath.length * MIN_RECALCULATION_PERCENTAGE) : 0;

	while (openSet.values.length > 0) {
		const current = openSet.dequeue().val;
		const currentX = current[0];
		const currentY = current[1];
		const currentKey = getKey(currentX, currentY);

		// If we've reached the destination
		if (currentX === endX && currentY === endY) {
			// Reconstruct path
			const path = [];
			let key = currentKey;
			while (cameFrom.has(key)) {
				const [x, y, isWarp, warpId] = cameFrom.get(key);
				path.unshift({
					x: parseInt(x),
					y: parseInt(y),
					isWarp: isWarp,
					warpId: warpId
				});
				key = getKey(x, y);
			}
			path.unshift({ x: startX, y: startY });

			return path;
		}

		// Check if current point is on the existing path and we've recalculated
		// at least the minimum percentage of the path
		if (existingPathLookup.has(currentKey) && existingPathLookup.get(currentKey) >= minRecalculationPoints) {
			// We found a point on the existing path, so we can use the rest of the existing path
			const pathIndex = existingPathLookup.get(currentKey);

			// Reconstruct path up to this point
			const newPath = [];
			let key = currentKey;
			while (cameFrom.has(key)) {
				const [x, y, isWarp, warpId] = cameFrom.get(key);
				newPath.unshift({
					x: parseInt(x),
					y: parseInt(y),
					isWarp: isWarp,
					warpId: warpId
				});
				key = getKey(x, y);
			}
			newPath.unshift({ x: startX, y: startY });

			// Append the rest of the existing path from this point onward
			if (pathIndex < existingPath.length - 1) {
				for (let i = pathIndex + 1; i < existingPath.length; i++) {
					newPath.push(existingPath[i]);
				}
			}

			return newPath;
		}

		closedSet.add(currentKey);

		// Get neighbors including warps
		const neighbors = getNeighbors(currentX, currentY, mapData, warps);
		for (const neighbor of neighbors) {
			const neighborKey = getKey(neighbor.x, neighbor.y);

			if (closedSet.has(neighborKey)) {
				continue;
			}

			const tentativeGScore = gScore.get(currentKey) + neighbor.cost;

			if (!openSet.values.some(node => getKey(node.val[0], node.val[1]) === neighborKey)) {
				openSet.enqueue(
					[neighbor.x, neighbor.y],
					tentativeGScore + heuristic(neighbor.x, neighbor.y, endX, endY)
				);
			} else if (tentativeGScore >= gScore.get(neighborKey)) {
				continue;
			}

			// Store warp information in cameFrom if this neighbor is a warp
			cameFrom.set(neighborKey, [currentX, currentY, neighbor.isWarp, neighbor.warpId]);
			gScore.set(neighborKey, tentativeGScore);
			fScore.set(neighborKey, tentativeGScore + heuristic(neighbor.x, neighbor.y, endX, endY));
		}
	}

	return [];
}

// Handle messages from the main thread
self.onmessage = function (e) {
	const data = e.data;

	switch (data.type) {
		case 'findPath': {
			const path = findPath(data.startX, data.startY, data.endX, data.endY, data.mapData, data.existingPath);
			self.postMessage({ type: 'pathResult', path: path, workerId: data.workerId });
			break;
		}
	}
};
