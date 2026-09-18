function pastel_colour(seed) {
	const baseRed = 0;
	const baseGreen = 0;
	const baseBlue = 0;

	const rand_1 = Math.abs(Math.sin(seed++) * 10000) % 256;
	const rand_2 = Math.abs(Math.sin(seed++) * 10000) % 256;
	const rand_3 = Math.abs(Math.sin(seed++) * 10000) % 256;

	//build colour
	const red = Math.round((rand_1 + baseRed) / 2);
	const green = Math.round((rand_2 + baseGreen) / 2);
	const blue = Math.round((rand_3 + baseBlue) / 2);

	return { red: red, green: green, blue: blue };
}
export default function (aid) {
	const rv = pastel_colour(aid);

	rv.style = 'rgb(' + rv.red + ', ' + rv.green + ', ' + rv.blue + ')';
	return rv;
}
