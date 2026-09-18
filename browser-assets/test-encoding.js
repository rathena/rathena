const variants = [
    "Ã€Â¯Ã€ÃºÃ€ÃŽÃ…ÃÃ†Ã¤Ã€ÃŒÂ½Âº",
    "À¯ÀúÀÎÅÍÆäÀÌ½º",
    "유저인터페이스"
];

for (const s of variants) {
    console.log("\nSTRING:", s);
    console.log("length:", s.length);

    console.log(
        "codepoints:",
        [...s]
            .map(c => c.codePointAt(0).toString(16))
            .join(" ")
    );

    console.log(
        "utf8 hex:",
        Buffer.from(s, "utf8").toString("hex")
    );

    console.log(
        "latin1 hex:",
        Buffer.from(s, "latin1").toString("hex")
    );
}