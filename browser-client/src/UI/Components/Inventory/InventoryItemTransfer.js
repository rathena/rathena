/**
 * Priorities for windows that receive full inventory stacks.
 * Modal interaction windows take precedence over secondary inventory windows.
 */
export const InventoryItemTransferPriority = Object.freeze({
	TRADE: 300,
	NPC_STORE: 400
});

/**
 * Transfer an inventory item stack to the active receivers, highest priority first.
 * A receiver that declines the item lets the next one (or the caller's fallback) handle it.
 *
 * @param {object} item Inventory item
 * @param {object} components Registered UI components
 * @returns {boolean} Whether a receiver handled the request
 */
export function transferInventoryItemStack(item, components) {
	if (!item || !components) {
		return false;
	}

	const receivers = Object.values(components)
		.filter(component => {
			return (
				component?.__active &&
				component._host?.isConnected &&
				component._host.style.display !== 'none' &&
				typeof component.receiveInventoryItemStack === 'function'
			);
		})
		.sort((a, b) => (b.inventoryTransferPriority || 0) - (a.inventoryTransferPriority || 0));

	return receivers.some(receiver => receiver.receiveInventoryItemStack(item) === true);
}
