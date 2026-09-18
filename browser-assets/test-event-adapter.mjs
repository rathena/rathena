import { EventHorizonGrf } from "./src/loaders/EventHorizonGrf.mjs";

const grf = new EventHorizonGrf(
    "C:\\Gravity\\Ragnarok\\data.grf"
);

await grf.load();

console.log("Indexed paths:", grf.files.size);

const tests = [
    "texture\\유저인터페이스\\statuswnd\\w_statwin_bg.bmp",
    "data\\texture\\유저인터페이스\\statuswnd\\w_statwin_bg.bmp",

    "texture\\유저인터페이스\\statuswnd\\w_ex_statwin_bg.bmp",

    "texture\\유저인터페이스\\statuswnd\\expand_on_normal.bmp",

    // propositalmente inexistente
    "texture\\nao-existe.bmp"
];

for (const filename of tests) {

    const result = await grf.getFile(filename);

    console.log(
        filename,
        result.error
            ? `ERROR: ${result.error.message}`
            : `OK: ${result.data.length} bytes`
    );
}

grf.close();