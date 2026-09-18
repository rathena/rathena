const iconv = require("iconv-lite");

const mojibake = "À¯ÀúÀÎÅÍÆäÀÌ½º";

const bytes = Buffer.from(mojibake, "latin1");
const decoded = iconv.decode(bytes, "cp949");

console.log("Mojibake:", mojibake);
console.log("Bytes:", bytes.toString("hex"));
console.log("CP949:", decoded);
console.log("Esperado:", "유저인터페이스");
console.log("Match:", decoded === "유저인터페이스");

const doubleMojibake =
    "Ã€Â¯Ã€ÃºÃ€ÃŽÃ…ÃÃ†Ã¤Ã€ÃŒÂ½Âº";

const layer1 = iconv.decode(
    iconv.encode(doubleMojibake, "windows-1252"),
    "utf8"
);

console.log("\nDouble:", doubleMojibake);
console.log("Layer 1:", layer1);
console.log(
    "Layer 1 match:",
    layer1 === "À¯ÀúÀÎÅÍÆäÀÌ½º"
);

const layer2 = iconv.decode(
    Buffer.from(layer1, "latin1"),
    "cp949"
);

console.log("Layer 2:", layer2);
console.log(
    "Layer 2 match:",
    layer2 === "유저인터페이스"
);