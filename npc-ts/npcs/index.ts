// Aggregate the NPCs in this subtree. Each NPC file `export const`s
// a Registration record (no side-effect register*() call inside the
// file); this index imports them and makes a single varargs
// register*() call. Same idiomatic shape as Prometheus.

import { tsGuide } from "./kafra";
import { mailbox } from "./mailbox";

registerNpc(tsGuide, mailbox);
