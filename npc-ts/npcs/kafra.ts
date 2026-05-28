// Exercises ctx.player — pulls character name / level / stats out of
// the host snapshot and dialogs them back. Spawned at Prontera
// (155, 180); click to drive the flow.

registerNpc({
    name: "ts_guide#prontera",
    map: "prontera",
    x: 155,
    y: 180,
    dir: 4,
    sprite: 757,
    async onClick(ctx) {
        if (!ctx.player) return;
        const p = ctx.player;

        ctx.mes("[TS Guide]");
        ctx.mes(`Hi, ${p.name}!`);
        ctx.mes(`You are level ${p.baseLevel} / job ${p.jobLevel}.`);
        ctx.mes(`Class id: ${p.classId}, sex: ${p.sex === 1 ? "M" : "F"}.`);
        ctx.mes(`You're standing at ${p.mapName} (${p.x}, ${p.y}).`);

        const choice = ctx.select([
            "Show my stats",
            "Show my vitals",
            "Show my wallet",
            "Nothing, bye",
        ]);

        ctx.mes("[TS Guide]");
        if (choice === 0) {
            ctx.mes(`STR ${p.str}   AGI ${p.agi}   VIT ${p.vit}`);
            ctx.mes(`INT ${p.int}   DEX ${p.dex}   LUK ${p.luk}`);
            ctx.mes(`Status points: ${p.statusPoint}`);
            ctx.mes(`Skill points: ${p.skillPoint}`);
        } else if (choice === 1) {
            ctx.mes(`HP: ${p.hp} / ${p.maxHp}`);
            ctx.mes(`SP: ${p.sp} / ${p.maxSp}`);
            ctx.mes(`Weight: ${p.weight} / ${p.maxWeight}`);
        } else if (choice === 2) {
            ctx.mes(`Zeny: ${p.zeny} z`);
            ctx.mes(`Party id: ${p.partyId || "(none)"}`);
            ctx.mes(`Guild id: ${p.guildId || "(none)"}`);
        } else {
            ctx.mes("Take care!");
        }

        ctx.close();
    },
});
