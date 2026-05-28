// Dialog-flow helpers — reusable building blocks for common NPC
// patterns. Import from `@lib/dialog`.
//
// Lib code is NOT processed by the auto-await build transform — use
// explicit `async` + `await` here. `awaitable(ctx)` re-types ctx so
// its dialog primitives have honest Promise-returning signatures.

import { awaitable } from "./internal";

/**
 * Render an array of dialog pages, one per "Next" click.
 *
 * @example
 * paginatedMes(ctx, [
 *     ["[Storyteller]", "Long ago, in a faraway kingdom…"],
 *     ["…a hero rose from the ashes.", "But that's a tale for another day."],
 * ]);
 */
export async function paginatedMes(ctx: NpcContext, pages: string[][]): Promise<void> {
    const a = awaitable(ctx);
    for (let i = 0; i < pages.length; i++) {
        for (const line of pages[i]) await a.mes(line);
        if (i < pages.length - 1) await a.next();
    }
}

/**
 * Show a yes/no confirmation. Returns `true` if the player picked
 * the first option (yes), `false` otherwise (including escape).
 *
 * @example
 * if (await confirm(ctx, "Are you sure you want to spend 100,000 zeny?")) {
 *     ctx.player!.zeny -= 100_000;
 * }
 */
export async function confirm(
    ctx: NpcContext,
    prompt: string,
    yesLabel = "Yes",
    noLabel = "No",
): Promise<boolean> {
    const a = awaitable(ctx);
    await a.mes(prompt);
    return (await a.select([yesLabel, noLabel])) === 0;
}

/**
 * Prompt for a number in `[min, max]`. Loops until the player enters
 * a valid value or cancels (returns `null`).
 *
 * @example
 * const qty = await numberInput(ctx, "How many?", 1, 100);
 * if (qty === null) return;  // player cancelled
 */
export async function numberInput(
    ctx: NpcContext,
    prompt: string,
    min: number,
    max: number,
): Promise<number | null> {
    const a = awaitable(ctx);
    while (true) {
        await a.mes(prompt);
        await a.mes(`Enter a number between ${min} and ${max}. Type 0 to cancel.`);
        const v = await a.input(min, max);
        if (v === 0) return null;
        if (v >= min && v <= max) return v;
        await a.mes("That's out of range. Please try again.");
    }
}

/**
 * Show a numbered menu where each option also displays a price.
 * Returns the chosen 0-based index or `-1` on cancel.
 *
 * @example
 * const pick = await pricedMenu(ctx, "What would you like to buy?", [
 *     { label: "Red Potion",    cost: 50 },
 *     { label: "Orange Potion", cost: 200 },
 * ]);
 */
export async function pricedMenu(
    ctx: NpcContext,
    prompt: string,
    options: Array<{ label: string; cost: number }>,
    suffix = "z",
): Promise<number> {
    const a = awaitable(ctx);
    await a.mes(prompt);
    return await a.select(
        options.map(o => `${o.label} — ${o.cost.toLocaleString()}${suffix}`),
    );
}
