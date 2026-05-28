// Minimal NPC demonstrating the dialog primitives end-to-end. Spawned
// at Prontera (155, 180); click to drive a mes → select → close flow.
//
// This file is intentionally thin for the engine+registrars milestone.
// Later milestones expand it to use ctx.player snapshot data and
// shared helpers from @lib/*.

registerNpc({
    name: "ts_guide#prontera",
    map: "prontera",
    x: 155,
    y: 180,
    dir: 4,
    sprite: 757,
    async onClick(ctx) {
        ctx.mes("[TS Guide]");
        ctx.mes("Hi! I'm running entirely from TypeScript.");
        ctx.mes("Pick an option:");

        const choice = ctx.select([
            "What does this prove?",
            "Tell me a joke",
            "Nothing, bye",
        ]);

        if (choice === 0) {
            ctx.mes("[TS Guide]");
            ctx.mes("Click → V8 → JS function → packets back to you.");
            ctx.mes("No legacy .txt script involved.");
        } else if (choice === 1) {
            ctx.mes("[TS Guide]");
            ctx.mes("Why did the Poring cross the road?");
            ctx.mes("...");
            ctx.mes("To gel to the other side. (sorry)");
        } else {
            ctx.mes("[TS Guide]");
            ctx.mes("Take care!");
        }

        ctx.close();
    },
});
