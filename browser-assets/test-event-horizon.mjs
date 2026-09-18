import {
    openGrf,
    extractFile,
    closeGrf
} from "./src/loaders/eventHorizonCore.mjs";

const grfPath = "C:\\Gravity\\Ragnarok\\data.grf";

const grf = openGrf(grfPath);

console.log("Version:", grf.version);
console.log("Files type:", grf.files?.constructor?.name);
console.log("Files count:", grf.files?.size ?? grf.files?.length);
console.log("GRF keys:", Object.keys(grf));

const wanted = [
    "w_statwin_bg.bmp",
    "w_ex_statwin_bg.bmp",
    "expand_on_normal.bmp",
    "expand_on_press.bmp"
];

// Aceita Map ou Array
const entries =
    grf.files instanceof Map
        ? Array.from(grf.files.values())
        : Array.from(grf.files ?? []);

for (const name of wanted) {

    const entry = entries.find(e => {
        const entryName =
            e?.name ??
            e?.filename ??
            e?.path ??
            "";

        return entryName
            .toLowerCase()
            .endsWith(name.toLowerCase());
    });

    if (!entry) {
        console.log("NOT FOUND:", name);
        continue;
    }

    const entryName =
        entry.name ??
        entry.filename ??
        entry.path ??
        "(unknown)";

    const data = extractFile(grf, entry);

    console.log(
        "FOUND:",
        entryName,
        "| bytes:",
        data.length
    );
}

closeGrf(grf);