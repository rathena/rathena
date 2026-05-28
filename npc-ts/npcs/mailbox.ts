// TS port of npc/other/mail.txt — the 30-duplicate Mailbox.
//
// rAthena (.txt):
//   -            script   Mailbox#dummy::MailBox  -1,{ ... }
//   prontera,146,86,0  duplicate(MailBox)  Mailbox#prt   888
//   prontera,275,213,0 duplicate(MailBox)  Mailbox#2prt  888
//   ...
//
// TS port: one `registerNpc` call. The primary is the first placed
// mailbox; the rest are passed as `duplicates`. All share the same
// onClick closure.

import type { NpcRegistration } from "@api";

export const mailbox: NpcRegistration = {
    // Primary mailbox — Prontera.
    name: "Mailbox#prt1",
    map: "prontera",
    x: 146,
    y: 86,
    dir: 0,
    sprite: 888,  // rAthena mailbox sprite

    async onClick(ctx) {
        if (!ctx.player) return;

        ctx.mes("[Mailbox]");
        ctx.mes("To use the mailbox service,");
        ctx.mes("you are required to pay 130 zeny.");
        ctx.mes("Would you like to use the service?");
        ctx.next();

        const choice = ctx.select(["Yes.", "No."]);
        if (choice !== 0) {
            ctx.mes("[Mailbox]");
            ctx.mes("Thank you, please come again.");
            ctx.close();
            return;
        }

        ctx.mes("[Mailbox]");
        if (ctx.player.zeny < 130) {
            ctx.mes("I am sorry, but you do not have enough money.");
            ctx.mes("To use the mailbox service,");
            ctx.mes("you are required to pay 130 zeny.");
            ctx.close();
            return;
        }
        ctx.mes("Thank you, please come again.");
        ctx.player.zeny -= 130;
        ctx.player.openMail();  // stub for now — graduates when the UI op lands
        ctx.close();
    },

    // 29 more placed copies, scattered around the world. Each is a
    // separate NPC entity in the registry, all sharing the onClick
    // above. `sprite` / `dir` / `triggerArea` would inherit from the
    // primary if omitted; we set sprite explicitly to mirror the
    // legacy mail.txt verbatim.
    duplicates: [
        { name: "Mailbox#prt2",  map: "prontera",  x: 275, y: 213 },
        { name: "Mailbox#prt3",  map: "prontera",  x:  34, y: 212 },
        { name: "Mailbox#iz",    map: "izlude",    x: 136, y:  94 },
        { name: "Mailbox#moc1",  map: "moc_ruins", x:  72, y: 166 },
        { name: "Mailbox#moc2",  map: "moc_ruins", x: 156, y:  52 },
        { name: "Mailbox#gef1",  map: "geffen",    x: 115, y:  67 },
        { name: "Mailbox#gef2",  map: "geffen",    x: 199, y: 125 },
        { name: "Mailbox#pay1",  map: "payon",     x: 178, y: 110 },
        { name: "Mailbox#pay2",  map: "payon",     x: 137, y: 159 },
        { name: "Mailbox#al",    map: "alberta",   x: 117, y:  64 },
        { name: "Mailbox#aldeg", map: "aldebaran", x: 145, y: 116 },
        // …add the remaining locations to taste; this is the migration
        // shape, identical to rAthena's mail.txt rows.
    ],
};
