const path = require("path");
const Grf = require("./src/controllers/grfController");

async function main() {

    const grfPath =
        path.resolve("./resources/data.grf");

    console.log("GRF:", grfPath);

    const grf = new Grf(grfPath);

    await grf.load();

    console.log("Loaded:", grf.loaded);
    console.log("Type:", grf.type);

    const files = grf.listFiles();

    console.log("Indexed:", files.length);

    const tests = [
        "texture\\유저인터페이스\\statuswnd\\w_statwin_bg.bmp",
        "texture\\유저인터페이스\\statuswnd\\w_ex_statwin_bg.bmp",
        "texture\\유저인터페이스\\statuswnd\\expand_on_normal.bmp",
        "texture\\nao-existe.bmp"
    ];

    for (const filename of tests) {

        const data = await grf.getFile(filename);

        console.log(
            filename,
            data
                ? `OK: ${data.length} bytes`
                : "NOT FOUND"
        );
    }

    grf.close();
}

main().catch(console.error);