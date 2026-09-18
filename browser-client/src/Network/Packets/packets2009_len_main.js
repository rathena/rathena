/**
 * Network/Packets/packets2009_len_main.js
 *
 * Network Packet Length
 * Manage packets length
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 * This file is based in a part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2018-2023 Hercules Dev Team
 * Copyright (C) 2018-2022 Andrei Karas (4144)
 *
 * @author Alison Serafim
 */

const length_list = new Array();

function init(packetver) {
	// Packet: 0x0064
	length_list[0x0064] = 55;

	// Packet: 0x0065
	length_list[0x0065] = 17;

	// Packet: 0x0066
	length_list[0x0066] = 3;

	// Packet: 0x0067
	length_list[0x0067] = 37;

	// Packet: 0x0068
	length_list[0x0068] = 46;

	// Packet: 0x0069
	length_list[0x0069] = -1;

	// Packet: 0x006a
	length_list[0x006a] = 23;

	// Packet: 0x006b
	length_list[0x006b] = -1;

	// Packet: 0x006c
	length_list[0x006c] = 3;

	// Packet: 0x006d
	if (packetver >= 20090617) {
		length_list[0x006d] = 114;
	} else if (packetver >= 20090107) {
		length_list[0x006d] = 110;
	}

	// Packet: 0x006e
	length_list[0x006e] = 3;

	// Packet: 0x006f
	length_list[0x006f] = 2;

	// Packet: 0x0070
	length_list[0x0070] = 3;

	// Packet: 0x0071
	length_list[0x0071] = 28;

	// Packet: 0x0072
	length_list[0x0072] = 22;

	// Packet: 0x0073
	if (packetver >= 20090406) {
		length_list[0x0073] = 11;
	} else if (packetver >= 20090401) {
		length_list[0x0073] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0073] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0073] = 282;
	} else if (packetver >= 20090218) {
		length_list[0x0073] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0073] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x0073] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0073] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0074
	length_list[0x0074] = 3;

	// Packet: 0x0075
	if (packetver >= 20090401) {
		length_list[0x0075] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0075] = 22;
	} else if (packetver >= 20090225) {
		length_list[0x0075] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0075] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x0075] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0075] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0076
	length_list[0x0076] = 9;

	// Packet: 0x0077
	if (packetver >= 20090406) {
		length_list[0x0077] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x0077] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0077] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0077] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0077] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x0077] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0077] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x0077] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x0077] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0077] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x0077] = -1;
	}

	// Packet: 0x0078
	if (packetver >= 20090406) {
		length_list[0x0078] = 55;
	} else if (packetver >= 20090318) {
		length_list[0x0078] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0078] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x0078] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0078] = 22;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0079
	if (packetver >= 20090406) {
		length_list[0x0079] = 53;
	} else if (packetver >= 20090318) {
		length_list[0x0079] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0079] = 15;
	} else if (packetver >= 20090225) {
		length_list[0x0079] = 20;
	} else if (packetver >= 20090218) {
		length_list[0x0079] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0079] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0079] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0079] = 21;
	} else if (packetver >= 20090107) {
		length_list[0x0079] = -1;
	}

	// Packet: 0x007a
	if (packetver >= 20090406) {
		length_list[0x007a] = 58;
	} else if (packetver >= 20090325) {
		length_list[0x007a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x007a] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x007a] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x007a] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x007a] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x007a] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x007a] = -1;
	}

	// Packet: 0x007b
	if (packetver >= 20090406) {
		length_list[0x007b] = 60;
	} else if (packetver >= 20090401) {
		length_list[0x007b] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x007b] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x007b] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x007b] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x007b] = 5;
	} else if (packetver >= 20090129) {
		length_list[0x007b] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x007b] = -1;
	}

	// Packet: 0x007c
	if (packetver >= 20090617) {
		length_list[0x007c] = 44;
	} else if (packetver >= 20090406) {
		length_list[0x007c] = 42;
	} else if (packetver >= 20090401) {
		length_list[0x007c] = 14;
	} else if (packetver >= 20090325) {
		length_list[0x007c] = 182;
	} else if (packetver >= 20090311) {
		length_list[0x007c] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x007c] = 19;
	} else if (packetver >= 20090218) {
		length_list[0x007c] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x007c] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x007c] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x007c] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x007c] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x007c] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x007d
	if (packetver >= 20090406) {
		length_list[0x007d] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x007d] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x007d] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x007d] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x007d] = 282;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x007e
	length_list[0x007e] = 105;

	// Packet: 0x007f
	if (packetver >= 20090406) {
		length_list[0x007f] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x007f] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x007f] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x007f] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x007f] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x007f] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x007f] = -1;
	}

	// Packet: 0x0080
	if (packetver >= 20090406) {
		length_list[0x0080] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x0080] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0080] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0080] = 36;
	} else if (packetver >= 20090107) {
		length_list[0x0080] = -1;
	}

	// Packet: 0x0081
	length_list[0x0081] = 3;

	// Packet: 0x0082
	length_list[0x0082] = 2;

	// Packet: 0x0083
	if (packetver >= 20090406) {
		length_list[0x0083] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x0083] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0083] = 65;
	} else if (packetver >= 20090311) {
		length_list[0x0083] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0083] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x0083] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0083] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0083] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0083] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0083] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0083] = 5;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0084
	length_list[0x0084] = 2;

	// Packet: 0x0085
	length_list[0x0085] = 10;

	// Packet: 0x0086
	if (packetver >= 20090406) {
		length_list[0x0086] = 16;
	} else if (packetver >= 20090325) {
		length_list[0x0086] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0086] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x0086] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0086] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x0086] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0086] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0086] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0086] = 17;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0087
	if (packetver >= 20090406) {
		length_list[0x0087] = 12;
	} else if (packetver >= 20090401) {
		length_list[0x0087] = 39;
	} else if (packetver >= 20090325) {
		length_list[0x0087] = 15;
	} else if (packetver >= 20090318) {
		length_list[0x0087] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x0087] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0087] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0087] = 21;
	} else if (packetver >= 20090211) {
		length_list[0x0087] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x0087] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0087] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x0087] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0087] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0088
	if (packetver >= 20090406) {
		length_list[0x0088] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0088] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0088] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x0088] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0088] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x0088] = 37;
	} else if (packetver >= 20090204) {
		length_list[0x0088] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0088] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0088] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0088] = 60;
	} else if (packetver >= 20090107) {
		length_list[0x0088] = 26;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0089
	length_list[0x0089] = 11;

	// Packet: 0x008a
	if (packetver >= 20090406) {
		length_list[0x008a] = 29;
	} else if (packetver >= 20090401) {
		length_list[0x008a] = 15;
	} else if (packetver >= 20090325) {
		length_list[0x008a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x008a] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x008a] = 21;
	} else if (packetver >= 20090218) {
		length_list[0x008a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x008a] = 282;
	} else if (packetver >= 20090204) {
		length_list[0x008a] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x008a] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x008a] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x008b
	length_list[0x008b] = 23;

	// Packet: 0x008c
	length_list[0x008c] = 14;

	// Packet: 0x008d
	if (packetver >= 20090311) {
		length_list[0x008d] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x008d] = 34;
	} else if (packetver >= 20090107) {
		length_list[0x008d] = -1;
	}

	// Packet: 0x008e
	if (packetver >= 20090401) {
		length_list[0x008e] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x008e] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x008e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x008e] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x008e] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x008e] = 12;
	} else if (packetver >= 20090107) {
		length_list[0x008e] = 60;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0090
	if (packetver >= 20090406) {
		length_list[0x0090] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x0090] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0090] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x0090] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0090] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0090] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x0090] = 44;
	} else if (packetver >= 20090129) {
		length_list[0x0090] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0090] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0090] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0090] = 36;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0091
	if (packetver >= 20090406) {
		length_list[0x0091] = 22;
	} else if (packetver >= 20090401) {
		length_list[0x0091] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0091] = 21;
	} else if (packetver >= 20090114) {
		length_list[0x0091] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0091] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0092
	if (packetver >= 20090406) {
		length_list[0x0092] = 28;
	} else if (packetver >= 20090401) {
		length_list[0x0092] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x0092] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0092] = 30;
	} else if (packetver >= 20090218) {
		length_list[0x0092] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0092] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0092] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0092] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x0092] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0092] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0092] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0093
	if (packetver >= 20090401) {
		length_list[0x0093] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x0093] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0093] = 14;
	} else if (packetver >= 20090211) {
		length_list[0x0093] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0093] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x0093] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x0093] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0093] = -1;
	}

	// Packet: 0x0094
	length_list[0x0094] = 19;

	// Packet: 0x0095
	if (packetver >= 20090406) {
		length_list[0x0095] = 30;
	} else if (packetver >= 20090401) {
		length_list[0x0095] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0095] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0095] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0095] = 33;
	} else if (packetver >= 20090218) {
		length_list[0x0095] = 60;
	} else if (packetver >= 20090211) {
		length_list[0x0095] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x0095] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0095] = 53;
	} else if (packetver >= 20090107) {
		length_list[0x0095] = -1;
	}

	// Packet: 0x0096
	if (packetver >= 20090406) {
		length_list[0x0096] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0096] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x0096] = 71;
	} else if (packetver >= 20090318) {
		length_list[0x0096] = 11;
	} else if (packetver >= 20090311) {
		length_list[0x0096] = 9;
	} else if (packetver >= 20090225) {
		length_list[0x0096] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0096] = 39;
	} else if (packetver >= 20090211) {
		length_list[0x0096] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0096] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0096] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x0096] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x0096] = -1;
	}

	// Packet: 0x0097
	if (packetver >= 20090406) {
		length_list[0x0097] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0097] = 30;
	} else if (packetver >= 20090325) {
		length_list[0x0097] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0097] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0097] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0097] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x0097] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0097] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x0097] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0097] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0097] = 37;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0098
	if (packetver >= 20090406) {
		length_list[0x0098] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x0098] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0098] = 26;
	} else if (packetver >= 20090318) {
		length_list[0x0098] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x0098] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0098] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x0098] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0098] = 20;
	} else if (packetver >= 20090114) {
		length_list[0x0098] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x0098] = 9;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0099
	if (packetver >= 20090318) {
		length_list[0x0099] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0099] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0099] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0099] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x0099] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0099] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0099] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0099] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0099] = 31;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x009a
	if (packetver >= 20090311) {
		length_list[0x009a] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x009a] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x009a] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x009a] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x009a] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x009a] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x009a] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x009a] = 65;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x009b
	length_list[0x009b] = 34;

	// Packet: 0x009c
	if (packetver >= 20090406) {
		length_list[0x009c] = 9;
	} else if (packetver >= 20090401) {
		length_list[0x009c] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x009c] = 79;
	} else if (packetver >= 20090318) {
		length_list[0x009c] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x009c] = 29;
	} else if (packetver >= 20090225) {
		length_list[0x009c] = 65;
	} else if (packetver >= 20090107) {
		length_list[0x009c] = -1;
	}

	// Packet: 0x009d
	if (packetver >= 20090406) {
		length_list[0x009d] = 17;
	} else if (packetver >= 20090401) {
		length_list[0x009d] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x009d] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x009d] = 54;
	} else if (packetver >= 20090225) {
		length_list[0x009d] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x009d] = 68;
	} else if (packetver >= 20090211) {
		length_list[0x009d] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x009d] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x009d] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x009d] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x009d] = -1;
	}

	// Packet: 0x009e
	if (packetver >= 20090406) {
		length_list[0x009e] = 17;
	} else if (packetver >= 20090401) {
		length_list[0x009e] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x009e] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x009e] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x009e] = 114;
	} else if (packetver >= 20090225) {
		length_list[0x009e] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x009e] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x009e] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x009e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x009e] = 7;
	} else if (packetver >= 20090120) {
		length_list[0x009e] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x009e] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x009e] = -1;
	}

	// Packet: 0x009f
	length_list[0x009f] = 20;

	// Packet: 0x00a0
	if (packetver >= 20090406) {
		length_list[0x00a0] = 23;
	} else if (packetver >= 20090325) {
		length_list[0x00a0] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00a0] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x00a0] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00a0] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00a0] = -1;
	}

	// Packet: 0x00a1
	if (packetver >= 20090406) {
		length_list[0x00a1] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00a1] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x00a1] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00a1] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x00a1] = 27;
	} else if (packetver >= 20090225) {
		length_list[0x00a1] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00a1] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x00a1] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x00a1] = -1;
	}

	// Packet: 0x00a2
	length_list[0x00a2] = 14;

	// Packet: 0x00a3
	if (packetver >= 20090406) {
		length_list[0x00a3] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00a3] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x00a3] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00a3] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x00a3] = -1;
	}

	// Packet: 0x00a4
	if (packetver >= 20090401) {
		length_list[0x00a4] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00a4] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x00a4] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00a4] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x00a4] = 31;
	} else if (packetver >= 20090114) {
		length_list[0x00a4] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00a4] = 54;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00a5
	if (packetver >= 20090406) {
		length_list[0x00a5] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00a5] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x00a5] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00a5] = 12;
	} else if (packetver >= 20090129) {
		length_list[0x00a5] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00a5] = 54;
	} else if (packetver >= 20090114) {
		length_list[0x00a5] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00a5] = 24;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00a6
	if (packetver >= 20090406) {
		length_list[0x00a6] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00a6] = 59;
	} else if (packetver >= 20090325) {
		length_list[0x00a6] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x00a6] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00a6] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x00a6] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00a6] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x00a6] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00a6] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x00a6] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00a6] = 18;
	} else if (packetver >= 20090107) {
		length_list[0x00a6] = 20;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00a7
	length_list[0x00a7] = 9;

	// Packet: 0x00a8
	if (packetver >= 20090406) {
		length_list[0x00a8] = 7;
	} else if (packetver >= 20090401) {
		length_list[0x00a8] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00a8] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x00a8] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00a8] = 30;
	} else if (packetver >= 20090218) {
		length_list[0x00a8] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00a8] = 20;
	} else if (packetver >= 20090204) {
		length_list[0x00a8] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00a8] = 54;
	} else if (packetver >= 20090120) {
		length_list[0x00a8] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00a8] = 27;
	} else if (packetver >= 20090107) {
		length_list[0x00a8] = 7;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00a9
	if (packetver >= 20090406) {
		length_list[0x00a9] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00a9] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00a9] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x00a9] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00a9] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00a9] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00aa
	if (packetver >= 20090406) {
		length_list[0x00aa] = 7;
	} else if (packetver >= 20090325) {
		length_list[0x00aa] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00aa] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x00aa] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00aa] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x00aa] = 5;
	} else if (packetver >= 20090211) {
		length_list[0x00aa] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00aa] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x00aa] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00aa] = 53;
	} else if (packetver >= 20090114) {
		length_list[0x00aa] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x00aa] = -1;
	}

	// Packet: 0x00ab
	if (packetver >= 20090406) {
		length_list[0x00ab] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x00ab] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00ab] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x00ab] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00ab] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x00ab] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00ab] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x00ab] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00ac
	if (packetver >= 20090325) {
		length_list[0x00ac] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x00ac] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00ac] = 14;
	} else if (packetver >= 20090211) {
		length_list[0x00ac] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x00ac] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00ac] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00ae
	length_list[0x00ae] = -1;

	// Packet: 0x00af
	if (packetver >= 20090406) {
		length_list[0x00af] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00af] = 54;
	} else if (packetver >= 20090325) {
		length_list[0x00af] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00af] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x00af] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00af] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x00af] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00af] = 14;
	} else if (packetver >= 20090129) {
		length_list[0x00af] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x00af] = -1;
	}

	// Packet: 0x00b0
	if (packetver >= 20090406) {
		length_list[0x00b0] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x00b0] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x00b0] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00b0] = 57;
	} else if (packetver >= 20090114) {
		length_list[0x00b0] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00b0] = 30;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00b1
	if (packetver >= 20090406) {
		length_list[0x00b1] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x00b1] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x00b1] = 27;
	} else if (packetver >= 20090311) {
		length_list[0x00b1] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00b1] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x00b1] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00b1] = 29;
	} else if (packetver >= 20090107) {
		length_list[0x00b1] = 11;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00b2
	if (packetver >= 20090406) {
		length_list[0x00b2] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00b2] = 42;
	} else if (packetver >= 20090325) {
		length_list[0x00b2] = 32;
	} else if (packetver >= 20090318) {
		length_list[0x00b2] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00b2] = 186;
	} else if (packetver >= 20090225) {
		length_list[0x00b2] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x00b2] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00b2] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x00b2] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00b2] = 97;
	} else if (packetver >= 20090107) {
		length_list[0x00b2] = 23;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00b3
	if (packetver >= 20090406) {
		length_list[0x00b3] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00b3] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x00b3] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00b3] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x00b3] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00b3] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x00b3] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00b3] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00b4
	if (packetver >= 20090325) {
		length_list[0x00b4] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00b4] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x00b4] = 37;
	} else if (packetver >= 20090225) {
		length_list[0x00b4] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x00b4] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00b4] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x00b4] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00b4] = -1;
	}

	// Packet: 0x00b5
	if (packetver >= 20090406) {
		length_list[0x00b5] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x00b5] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00b5] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x00b5] = 32;
	} else if (packetver >= 20090211) {
		length_list[0x00b5] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00b5] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x00b5] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00b5] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00b5] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00b6
	if (packetver >= 20090406) {
		length_list[0x00b6] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x00b6] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00b6] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x00b6] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00b6] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x00b6] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00b6] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x00b6] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00b6] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00b6] = 26;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00b7
	if (packetver >= 20090406) {
		length_list[0x00b7] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00b7] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x00b7] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00b7] = 29;
	} else if (packetver >= 20090311) {
		length_list[0x00b7] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00b7] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x00b7] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00b7] = 54;
	} else if (packetver >= 20090204) {
		length_list[0x00b7] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00b7] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00b7] = -1;
	}

	// Packet: 0x00b8
	if (packetver >= 20090406) {
		length_list[0x00b8] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x00b8] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00b8] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x00b8] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00b8] = 33;
	} else if (packetver >= 20090129) {
		length_list[0x00b8] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00b8] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x00b8] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00b8] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00b9
	if (packetver >= 20090406) {
		length_list[0x00b9] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00b9] = 9;
	} else if (packetver >= 20090311) {
		length_list[0x00b9] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00b9] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x00b9] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00b9] = 31;
	} else if (packetver >= 20090114) {
		length_list[0x00b9] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00b9] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00ba
	length_list[0x00ba] = 2;

	// Packet: 0x00bb
	if (packetver >= 20090406) {
		length_list[0x00bb] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x00bb] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00bb] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x00bb] = 43;
	} else if (packetver >= 20090211) {
		length_list[0x00bb] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00bb] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x00bb] = 9;
	} else if (packetver >= 20090120) {
		length_list[0x00bb] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00bb] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x00bb] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00bc
	if (packetver >= 20090406) {
		length_list[0x00bc] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00bc] = 65;
	} else if (packetver >= 20090325) {
		length_list[0x00bc] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x00bc] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x00bc] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00bc] = 27;
	} else if (packetver >= 20090218) {
		length_list[0x00bc] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00bc] = 81;
	} else if (packetver >= 20090204) {
		length_list[0x00bc] = 15;
	} else if (packetver >= 20090129) {
		length_list[0x00bc] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x00bc] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00bd
	if (packetver >= 20090406) {
		length_list[0x00bd] = 44;
	} else if (packetver >= 20090225) {
		length_list[0x00bd] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00bd] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x00bd] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00bd] = 60;
	} else if (packetver >= 20090120) {
		length_list[0x00bd] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00bd] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x00bd] = 14;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00be
	if (packetver >= 20090406) {
		length_list[0x00be] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x00be] = 30;
	} else if (packetver >= 20090325) {
		length_list[0x00be] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x00be] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x00be] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x00be] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00be] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x00be] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00be] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x00be] = 5;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00bf
	if (packetver >= 20090406) {
		length_list[0x00bf] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x00bf] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00bf] = 18;
	} else if (packetver >= 20090311) {
		length_list[0x00bf] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00bf] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x00bf] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00bf] = 28;
	} else if (packetver >= 20090129) {
		length_list[0x00bf] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00bf] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00bf] = 86;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00c0
	if (packetver >= 20090406) {
		length_list[0x00c0] = 7;
	} else if (packetver >= 20090401) {
		length_list[0x00c0] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x00c0] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00c0] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x00c0] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00c0] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x00c0] = 31;
	} else if (packetver >= 20090107) {
		length_list[0x00c0] = 54;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00c1
	if (packetver >= 20090406) {
		length_list[0x00c1] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x00c1] = 67;
	} else if (packetver >= 20090318) {
		length_list[0x00c1] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00c1] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x00c1] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00c1] = 54;
	} else if (packetver >= 20090211) {
		length_list[0x00c1] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x00c1] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00c1] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x00c1] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00c1] = -1;
	}

	// Packet: 0x00c2
	if (packetver >= 20090406) {
		length_list[0x00c2] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00c2] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x00c2] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00c2] = 12;
	} else if (packetver >= 20090225) {
		length_list[0x00c2] = 9;
	} else if (packetver >= 20090218) {
		length_list[0x00c2] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x00c2] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00c2] = 60;
	} else if (packetver >= 20090120) {
		length_list[0x00c2] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x00c2] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x00c2] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00c3
	if (packetver >= 20090406) {
		length_list[0x00c3] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x00c3] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x00c3] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00c3] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00c3] = 5;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00c4
	if (packetver >= 20090406) {
		length_list[0x00c4] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00c4] = 9;
	} else if (packetver >= 20090218) {
		length_list[0x00c4] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00c4] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x00c4] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00c4] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00c5
	if (packetver >= 20090406) {
		length_list[0x00c5] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x00c5] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00c5] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x00c5] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x00c5] = 32;
	} else if (packetver >= 20090129) {
		length_list[0x00c5] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00c5] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x00c5] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00c5] = -1;
	}

	// Packet: 0x00c6
	if (packetver >= 20090325) {
		length_list[0x00c6] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00c6] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x00c6] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00c6] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x00c6] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x00c6] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00c6] = 57;
	} else if (packetver >= 20090120) {
		length_list[0x00c6] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x00c6] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00c6] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00c7
	if (packetver >= 20090406) {
		length_list[0x00c7] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00c7] = 5;
	} else if (packetver >= 20090325) {
		length_list[0x00c7] = 53;
	} else if (packetver >= 20090318) {
		length_list[0x00c7] = 54;
	} else if (packetver >= 20090311) {
		length_list[0x00c7] = 27;
	} else if (packetver >= 20090225) {
		length_list[0x00c7] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00c7] = 23;
	} else if (packetver >= 20090211) {
		length_list[0x00c7] = 57;
	} else if (packetver >= 20090129) {
		length_list[0x00c7] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00c7] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00c7] = -1;
	}

	// Packet: 0x00c8
	if (packetver >= 20090401) {
		length_list[0x00c8] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00c8] = 58;
	} else if (packetver >= 20090318) {
		length_list[0x00c8] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00c8] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x00c8] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00c8] = 9;
	} else if (packetver >= 20090204) {
		length_list[0x00c8] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00c8] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x00c8] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x00c8] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00c8] = 42;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00c9
	if (packetver >= 20090401) {
		length_list[0x00c9] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00c9] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x00c9] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x00c9] = 29;
	} else if (packetver >= 20090218) {
		length_list[0x00c9] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00c9] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x00c9] = 22;
	} else if (packetver >= 20090120) {
		length_list[0x00c9] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00c9] = 15;
	} else if (packetver >= 20090107) {
		length_list[0x00c9] = -1;
	}

	// Packet: 0x00ca
	if (packetver >= 20090406) {
		length_list[0x00ca] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00ca] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00ca] = 282;
	} else if (packetver >= 20090318) {
		length_list[0x00ca] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x00ca] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x00ca] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00ca] = 22;
	} else if (packetver >= 20090211) {
		length_list[0x00ca] = 21;
	} else if (packetver >= 20090204) {
		length_list[0x00ca] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x00ca] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00ca] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00cb
	length_list[0x00cb] = 3;

	// Packet: 0x00cc
	if (packetver >= 20090406) {
		length_list[0x00cc] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00cc] = 7;
	} else if (packetver >= 20090325) {
		length_list[0x00cc] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x00cc] = 9;
	} else if (packetver >= 20090204) {
		length_list[0x00cc] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00cc] = 12;
	} else if (packetver >= 20090120) {
		length_list[0x00cc] = 39;
	} else if (packetver >= 20090114) {
		length_list[0x00cc] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x00cc] = 5;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00cd
	length_list[0x00cd] = 3;

	// Packet: 0x00ce
	if (packetver >= 20090406) {
		length_list[0x00ce] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x00ce] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00ce] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x00ce] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00ce] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x00ce] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00ce] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x00ce] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00cf
	if (packetver >= 20090406) {
		length_list[0x00cf] = 27;
	} else if (packetver >= 20090401) {
		length_list[0x00cf] = 16;
	} else if (packetver >= 20090325) {
		length_list[0x00cf] = 282;
	} else if (packetver >= 20090318) {
		length_list[0x00cf] = 44;
	} else if (packetver >= 20090311) {
		length_list[0x00cf] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x00cf] = 79;
	} else if (packetver >= 20090211) {
		length_list[0x00cf] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00cf] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x00cf] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x00cf] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00cf] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00cf] = 14;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00d0
	if (packetver >= 20090406) {
		length_list[0x00d0] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00d0] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x00d0] = 37;
	} else if (packetver >= 20090318) {
		length_list[0x00d0] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x00d0] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00d0] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00d0] = -1;
	}

	// Packet: 0x00d1
	if (packetver >= 20090406) {
		length_list[0x00d1] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x00d1] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00d1] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x00d1] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x00d1] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00d1] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x00d1] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00d1] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00d1] = 16;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00d2
	if (packetver >= 20090406) {
		length_list[0x00d2] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x00d2] = 30;
	} else if (packetver >= 20090325) {
		length_list[0x00d2] = 5;
	} else if (packetver >= 20090311) {
		length_list[0x00d2] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00d2] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x00d2] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00d2] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00d2] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00d3
	if (packetver >= 20090401) {
		length_list[0x00d3] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x00d3] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x00d3] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00d3] = 12;
	} else if (packetver >= 20090225) {
		length_list[0x00d3] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x00d3] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x00d3] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00d3] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x00d3] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x00d3] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00d4
	length_list[0x00d4] = -1;

	// Packet: 0x00d5
	if (packetver >= 20090401) {
		length_list[0x00d5] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00d5] = 12;
	} else if (packetver >= 20090318) {
		length_list[0x00d5] = 26;
	} else if (packetver >= 20090311) {
		length_list[0x00d5] = 30;
	} else if (packetver >= 20090114) {
		length_list[0x00d5] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00d5] = 58;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00d6
	if (packetver >= 20090406) {
		length_list[0x00d6] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00d6] = 11;
	} else if (packetver >= 20090318) {
		length_list[0x00d6] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00d6] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x00d6] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x00d6] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00d6] = 59;
	} else if (packetver >= 20090204) {
		length_list[0x00d6] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x00d6] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00d6] = 18;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00d7
	if (packetver >= 20090406) {
		length_list[0x00d7] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00d7] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x00d7] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x00d7] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x00d7] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00d7] = 26;
	} else if (packetver >= 20090211) {
		length_list[0x00d7] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x00d7] = 23;
	} else if (packetver >= 20090129) {
		length_list[0x00d7] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00d7] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00d7] = -1;
	}

	// Packet: 0x00d8
	if (packetver >= 20090406) {
		length_list[0x00d8] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00d8] = 14;
	} else if (packetver >= 20090325) {
		length_list[0x00d8] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00d8] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x00d8] = 97;
	} else if (packetver >= 20090218) {
		length_list[0x00d8] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00d8] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x00d8] = 42;
	} else if (packetver >= 20090129) {
		length_list[0x00d8] = 15;
	} else if (packetver >= 20090120) {
		length_list[0x00d8] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x00d8] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00d8] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00d9
	if (packetver >= 20090406) {
		length_list[0x00d9] = 14;
	} else if (packetver >= 20090401) {
		length_list[0x00d9] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x00d9] = 26;
	} else if (packetver >= 20090318) {
		length_list[0x00d9] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x00d9] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x00d9] = 58;
	} else if (packetver >= 20090218) {
		length_list[0x00d9] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x00d9] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x00d9] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x00d9] = 55;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00da
	if (packetver >= 20090406) {
		length_list[0x00da] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x00da] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00da] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x00da] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00da] = 19;
	} else if (packetver >= 20090204) {
		length_list[0x00da] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00da] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x00da] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00da] = 31;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00db
	if (packetver >= 20090401) {
		length_list[0x00db] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00db] = 9;
	} else if (packetver >= 20090311) {
		length_list[0x00db] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00db] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x00db] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x00db] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00db] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x00db] = -1;
	}

	// Packet: 0x00dc
	if (packetver >= 20090406) {
		length_list[0x00dc] = 28;
	} else if (packetver >= 20090401) {
		length_list[0x00dc] = 282;
	} else if (packetver >= 20090318) {
		length_list[0x00dc] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x00dc] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00dc] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x00dc] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00dc] = 67;
	} else if (packetver >= 20090129) {
		length_list[0x00dc] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00dc] = 26;
	} else if (packetver >= 20090114) {
		length_list[0x00dc] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x00dc] = -1;
	}

	// Packet: 0x00dd
	if (packetver >= 20090406) {
		length_list[0x00dd] = 29;
	} else if (packetver >= 20090401) {
		length_list[0x00dd] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00dd] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x00dd] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00dd] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x00dd] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00dd] = 11;
	} else if (packetver >= 20090114) {
		length_list[0x00dd] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00dd] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00de
	if (packetver >= 20090406) {
		length_list[0x00de] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00de] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x00de] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x00de] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00de] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x00de] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x00de] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00de] = 32;
	} else if (packetver >= 20090129) {
		length_list[0x00de] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x00de] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00de] = 282;
	} else if (packetver >= 20090107) {
		length_list[0x00de] = 26;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00df
	if (packetver >= 20090406) {
		length_list[0x00df] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00df] = 33;
	} else if (packetver >= 20090325) {
		length_list[0x00df] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x00df] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00df] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x00df] = 27;
	} else if (packetver >= 20090211) {
		length_list[0x00df] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x00df] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00df] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x00df] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00df] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x00df] = 22;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00e0
	if (packetver >= 20090406) {
		length_list[0x00e0] = 30;
	} else if (packetver >= 20090401) {
		length_list[0x00e0] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00e0] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x00e0] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00e0] = 11;
	} else if (packetver >= 20090204) {
		length_list[0x00e0] = 33;
	} else if (packetver >= 20090114) {
		length_list[0x00e0] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00e0] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00e1
	if (packetver >= 20090406) {
		length_list[0x00e1] = 30;
	} else if (packetver >= 20090401) {
		length_list[0x00e1] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x00e1] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x00e1] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00e1] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x00e1] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00e1] = 23;
	} else if (packetver >= 20090120) {
		length_list[0x00e1] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00e1] = 28;
	} else if (packetver >= 20090107) {
		length_list[0x00e1] = -1;
	}

	// Packet: 0x00e2
	if (packetver >= 20090406) {
		length_list[0x00e2] = 26;
	} else if (packetver >= 20090401) {
		length_list[0x00e2] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x00e2] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00e2] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x00e2] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00e2] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x00e2] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00e3
	if (packetver >= 20090406) {
		length_list[0x00e3] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x00e3] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00e3] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x00e3] = 55;
	} else if (packetver >= 20090311) {
		length_list[0x00e3] = 15;
	} else if (packetver >= 20090225) {
		length_list[0x00e3] = 7;
	} else if (packetver >= 20090218) {
		length_list[0x00e3] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00e3] = 21;
	} else if (packetver >= 20090129) {
		length_list[0x00e3] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00e3] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x00e3] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00e3] = 26;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00e4
	if (packetver >= 20090401) {
		length_list[0x00e4] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x00e4] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x00e4] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00e4] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x00e4] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00e4] = 20;
	} else if (packetver >= 20090107) {
		length_list[0x00e4] = 7;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00e5
	if (packetver >= 20090406) {
		length_list[0x00e5] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x00e5] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00e5] = 68;
	} else if (packetver >= 20090311) {
		length_list[0x00e5] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x00e5] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00e5] = 37;
	} else if (packetver >= 20090211) {
		length_list[0x00e5] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x00e5] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00e5] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x00e5] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x00e5] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00e6
	if (packetver >= 20090406) {
		length_list[0x00e6] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00e6] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00e6] = 86;
	} else if (packetver >= 20090204) {
		length_list[0x00e6] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00e6] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x00e6] = 58;
	} else if (packetver >= 20090114) {
		length_list[0x00e6] = 57;
	} else if (packetver >= 20090107) {
		length_list[0x00e6] = -1;
	}

	// Packet: 0x00e7
	if (packetver >= 20090406) {
		length_list[0x00e7] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00e7] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00e7] = 55;
	} else if (packetver >= 20090218) {
		length_list[0x00e7] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00e7] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x00e7] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00e7] = 68;
	} else if (packetver >= 20090114) {
		length_list[0x00e7] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00e7] = 42;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00e8
	if (packetver >= 20090406) {
		length_list[0x00e8] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x00e8] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00e8] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x00e8] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00e8] = 58;
	} else if (packetver >= 20090129) {
		length_list[0x00e8] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x00e8] = -1;
	}

	// Packet: 0x00e9
	if (packetver >= 20090406) {
		length_list[0x00e9] = 19;
	} else if (packetver >= 20090325) {
		length_list[0x00e9] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00e9] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x00e9] = 22;
	} else if (packetver >= 20090225) {
		length_list[0x00e9] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00e9] = 5;
	} else if (packetver >= 20090211) {
		length_list[0x00e9] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x00e9] = 33;
	} else if (packetver >= 20090129) {
		length_list[0x00e9] = 32;
	} else if (packetver >= 20090120) {
		length_list[0x00e9] = 54;
	} else if (packetver >= 20090107) {
		length_list[0x00e9] = -1;
	}

	// Packet: 0x00ea
	if (packetver >= 20090406) {
		length_list[0x00ea] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x00ea] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00ea] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x00ea] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x00ea] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00ea] = 12;
	} else if (packetver >= 20090218) {
		length_list[0x00ea] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00ea] = 282;
	} else if (packetver >= 20090129) {
		length_list[0x00ea] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00ea] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x00ea] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00ea] = -1;
	}

	// Packet: 0x00eb
	if (packetver >= 20090406) {
		length_list[0x00eb] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x00eb] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00eb] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x00eb] = 15;
	} else if (packetver >= 20090311) {
		length_list[0x00eb] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x00eb] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x00eb] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x00eb] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x00eb] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00eb] = 71;
	} else if (packetver >= 20090114) {
		length_list[0x00eb] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00eb] = 14;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00ec
	if (packetver >= 20090406) {
		length_list[0x00ec] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00ec] = 60;
	} else if (packetver >= 20090325) {
		length_list[0x00ec] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00ec] = 27;
	} else if (packetver >= 20090311) {
		length_list[0x00ec] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00ec] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x00ec] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00ec] = 15;
	} else if (packetver >= 20090204) {
		length_list[0x00ec] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00ec] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x00ec] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x00ec] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x00ec] = 18;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00ed
	if (packetver >= 20090406) {
		length_list[0x00ed] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x00ed] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00ed] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x00ed] = 22;
	} else if (packetver >= 20090204) {
		length_list[0x00ed] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x00ed] = 282;
	} else if (packetver >= 20090114) {
		length_list[0x00ed] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x00ed] = 7;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00ee
	if (packetver >= 20090401) {
		length_list[0x00ee] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x00ee] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x00ee] = 282;
	} else if (packetver >= 20090311) {
		length_list[0x00ee] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00ee] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x00ee] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00ee] = 24;
	} else if (packetver >= 20090120) {
		length_list[0x00ee] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x00ee] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00ee] = 16;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00ef
	if (packetver >= 20090406) {
		length_list[0x00ef] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x00ef] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x00ef] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x00ef] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00ef] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x00ef] = 16;
	} else if (packetver >= 20090211) {
		length_list[0x00ef] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00ef] = 43;
	} else if (packetver >= 20090129) {
		length_list[0x00ef] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x00ef] = 81;
	} else if (packetver >= 20090114) {
		length_list[0x00ef] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00ef] = 39;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00f0
	if (packetver >= 20090406) {
		length_list[0x00f0] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x00f0] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x00f0] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00f0] = 34;
	} else if (packetver >= 20090211) {
		length_list[0x00f0] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00f0] = 31;
	} else if (packetver >= 20090129) {
		length_list[0x00f0] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x00f0] = 26;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00f1
	length_list[0x00f1] = 2;

	// Packet: 0x00f2
	if (packetver >= 20090406) {
		length_list[0x00f2] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00f2] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x00f2] = 32;
	} else if (packetver >= 20090318) {
		length_list[0x00f2] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x00f2] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x00f2] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x00f2] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x00f2] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x00f2] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x00f2] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00f2] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x00f2] = -1;
	}

	// Packet: 0x00f3
	length_list[0x00f3] = -1;

	// Packet: 0x00f4
	if (packetver >= 20090406) {
		length_list[0x00f4] = 21;
	} else if (packetver >= 20090401) {
		length_list[0x00f4] = 18;
	} else if (packetver >= 20090311) {
		length_list[0x00f4] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00f4] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x00f4] = 14;
	} else if (packetver >= 20090211) {
		length_list[0x00f4] = 53;
	} else if (packetver >= 20090129) {
		length_list[0x00f4] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00f4] = 43;
	} else if (packetver >= 20090114) {
		length_list[0x00f4] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x00f4] = 14;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00f5
	length_list[0x00f5] = 11;

	// Packet: 0x00f6
	if (packetver >= 20090406) {
		length_list[0x00f6] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x00f6] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x00f6] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00f6] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x00f6] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x00f6] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x00f6] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x00f6] = 21;
	} else if (packetver >= 20090114) {
		length_list[0x00f6] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00f6] = 26;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00f7
	length_list[0x00f7] = 17;

	// Packet: 0x00f8
	if (packetver >= 20090401) {
		length_list[0x00f8] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x00f8] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00f8] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x00f8] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x00f8] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00f8] = 65;
	} else if (packetver >= 20090204) {
		length_list[0x00f8] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00f8] = 60;
	} else if (packetver >= 20090107) {
		length_list[0x00f8] = -1;
	}

	// Packet: 0x00f9
	if (packetver >= 20090406) {
		length_list[0x00f9] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x00f9] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00f9] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x00f9] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00f9] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x00f9] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00f9] = 39;
	} else if (packetver >= 20090129) {
		length_list[0x00f9] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x00f9] = 12;
	} else if (packetver >= 20090107) {
		length_list[0x00f9] = -1;
	}

	// Packet: 0x00fa
	if (packetver >= 20090406) {
		length_list[0x00fa] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x00fa] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x00fa] = 20;
	} else if (packetver >= 20090311) {
		length_list[0x00fa] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x00fa] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x00fa] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00fa] = 114;
	} else if (packetver >= 20090107) {
		length_list[0x00fa] = -1;
	}

	// Packet: 0x00fb
	if (packetver >= 20090406) {
		length_list[0x00fb] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x00fb] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x00fb] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x00fb] = 36;
	} else if (packetver >= 20090225) {
		length_list[0x00fb] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x00fb] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x00fb] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x00fb] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x00fb] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x00fb] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00fb] = 16;
	} else if (packetver >= 20090107) {
		length_list[0x00fb] = 282;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00fc
	if (packetver >= 20090406) {
		length_list[0x00fc] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x00fc] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00fc] = 22;
	} else if (packetver >= 20090120) {
		length_list[0x00fc] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00fc] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x00fc] = -1;
	}

	// Packet: 0x00fd
	if (packetver >= 20090406) {
		length_list[0x00fd] = 27;
	} else if (packetver >= 20090218) {
		length_list[0x00fd] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x00fd] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x00fd] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00fd] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x00fd] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00fd] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x00fe
	if (packetver >= 20090406) {
		length_list[0x00fe] = 30;
	} else if (packetver >= 20090401) {
		length_list[0x00fe] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00fe] = 67;
	} else if (packetver >= 20090211) {
		length_list[0x00fe] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x00fe] = 29;
	} else if (packetver >= 20090129) {
		length_list[0x00fe] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x00fe] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x00fe] = 21;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x00ff
	if (packetver >= 20090406) {
		length_list[0x00ff] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x00ff] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x00ff] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x00ff] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x00ff] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x00ff] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x00ff] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x00ff] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x00ff] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x00ff] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x00ff] = 5;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0100
	if (packetver >= 20090406) {
		length_list[0x0100] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x0100] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x0100] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0100] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0100] = 81;
	} else if (packetver >= 20090211) {
		length_list[0x0100] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0100] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x0100] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0100] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0101
	if (packetver >= 20090406) {
		length_list[0x0101] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0101] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0101] = 20;
	} else if (packetver >= 20090120) {
		length_list[0x0101] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0101] = 20;
	} else if (packetver >= 20090107) {
		length_list[0x0101] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0102
	if (packetver >= 20090406) {
		length_list[0x0102] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x0102] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0102] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x0102] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0102] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x0102] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x0102] = 282;
	} else if (packetver >= 20090204) {
		length_list[0x0102] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0102] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0102] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x0102] = 9;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0103
	if (packetver >= 20090406) {
		length_list[0x0103] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x0103] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0103] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x0103] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0103] = 54;
	} else if (packetver >= 20090204) {
		length_list[0x0103] = 28;
	} else if (packetver >= 20090129) {
		length_list[0x0103] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0103] = 66;
	} else if (packetver >= 20090114) {
		length_list[0x0103] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x0103] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0104
	if (packetver >= 20090406) {
		length_list[0x0104] = 79;
	} else if (packetver >= 20090325) {
		length_list[0x0104] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0104] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x0104] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0104] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x0104] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0104] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x0104] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0104] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0104] = -1;
	}

	// Packet: 0x0105
	if (packetver >= 20090406) {
		length_list[0x0105] = 31;
	} else if (packetver >= 20090401) {
		length_list[0x0105] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x0105] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0105] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0105] = 14;
	} else if (packetver >= 20090114) {
		length_list[0x0105] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0105] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0106
	if (packetver >= 20090406) {
		length_list[0x0106] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x0106] = 24;
	} else if (packetver >= 20090325) {
		length_list[0x0106] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0106] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0106] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0106] = 9;
	} else if (packetver >= 20090129) {
		length_list[0x0106] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0106] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x0106] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0106] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0107
	if (packetver >= 20090406) {
		length_list[0x0107] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x0107] = 16;
	} else if (packetver >= 20090325) {
		length_list[0x0107] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x0107] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0107] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x0107] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0107] = 23;
	} else if (packetver >= 20090107) {
		length_list[0x0107] = -1;
	}

	// Packet: 0x0108
	if (packetver >= 20090325) {
		length_list[0x0108] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0108] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x0108] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0108] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0108] = -1;
	}

	// Packet: 0x0109
	if (packetver >= 20090325) {
		length_list[0x0109] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0109] = 81;
	} else if (packetver >= 20090225) {
		length_list[0x0109] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0109] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x0109] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0109] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0109] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x0109] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0109] = 11;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x010a
	if (packetver >= 20090406) {
		length_list[0x010a] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x010a] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x010a] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x010a] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x010a] = 22;
	} else if (packetver >= 20090107) {
		length_list[0x010a] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x010b
	if (packetver >= 20090401) {
		length_list[0x010b] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x010b] = 17;
	} else if (packetver >= 20090211) {
		length_list[0x010b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x010b] = 14;
	} else if (packetver >= 20090129) {
		length_list[0x010b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x010b] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x010b] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x010b] = 23;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x010c
	if (packetver >= 20090406) {
		length_list[0x010c] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x010c] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x010c] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x010c] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x010c] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x010c] = 27;
	} else if (packetver >= 20090120) {
		length_list[0x010c] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x010c] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x010c] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x010d
	length_list[0x010d] = 2;

	// Packet: 0x010e
	if (packetver >= 20090406) {
		length_list[0x010e] = 11;
	} else if (packetver >= 20090325) {
		length_list[0x010e] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x010e] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x010e] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x010e] = 86;
	} else if (packetver >= 20090204) {
		length_list[0x010e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x010e] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x010e] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x010f
	if (packetver >= 20090325) {
		length_list[0x010f] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x010f] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x010f] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x010f] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x010f] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x010f] = 43;
	} else if (packetver >= 20090107) {
		length_list[0x010f] = 81;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0110
	if (packetver >= 20090406) {
		length_list[0x0110] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0110] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0110] = 9;
	} else if (packetver >= 20090225) {
		length_list[0x0110] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0110] = 29;
	} else if (packetver >= 20090129) {
		length_list[0x0110] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0110] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x0110] = -1;
	}

	// Packet: 0x0111
	if (packetver >= 20090406) {
		length_list[0x0111] = 39;
	} else if (packetver >= 20090325) {
		length_list[0x0111] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0111] = 20;
	} else if (packetver >= 20090218) {
		length_list[0x0111] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0111] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0111] = -1;
	}

	// Packet: 0x0112
	if (packetver >= 20090406) {
		length_list[0x0112] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x0112] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x0112] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0112] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0112] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0112] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0112] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0112] = 29;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0113
	length_list[0x0113] = 25;

	// Packet: 0x0114
	if (packetver >= 20090406) {
		length_list[0x0114] = 31;
	} else if (packetver >= 20090401) {
		length_list[0x0114] = 28;
	} else if (packetver >= 20090325) {
		length_list[0x0114] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0114] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0114] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0114] = 17;
	} else if (packetver >= 20090129) {
		length_list[0x0114] = 182;
	} else if (packetver >= 20090120) {
		length_list[0x0114] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0114] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0114] = 60;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0115
	length_list[0x0115] = 35;

	// Packet: 0x0116
	length_list[0x0116] = 17;

	// Packet: 0x0117
	if (packetver >= 20090406) {
		length_list[0x0117] = 18;
	} else if (packetver >= 20090325) {
		length_list[0x0117] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0117] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x0117] = 282;
	} else if (packetver >= 20090129) {
		length_list[0x0117] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0117] = 30;
	} else if (packetver >= 20090114) {
		length_list[0x0117] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x0117] = -1;
	}

	// Packet: 0x0118
	if (packetver >= 20090406) {
		length_list[0x0118] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x0118] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0118] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0118] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0118] = 27;
	} else if (packetver >= 20090225) {
		length_list[0x0118] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0118] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x0118] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0118] = 33;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0119
	if (packetver >= 20090406) {
		length_list[0x0119] = 13;
	} else if (packetver >= 20090325) {
		length_list[0x0119] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0119] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0119] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0119] = 97;
	} else if (packetver >= 20090114) {
		length_list[0x0119] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0119] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x011a
	if (packetver >= 20090406) {
		length_list[0x011a] = 15;
	} else if (packetver >= 20090401) {
		length_list[0x011a] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x011a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x011a] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x011a] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x011a] = 9;
	} else if (packetver >= 20090218) {
		length_list[0x011a] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x011a] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x011a] = 282;
	} else if (packetver >= 20090120) {
		length_list[0x011a] = 9;
	} else if (packetver >= 20090114) {
		length_list[0x011a] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x011a] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x011b
	if (packetver >= 20090406) {
		length_list[0x011b] = 20;
	} else if (packetver >= 20090401) {
		length_list[0x011b] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x011b] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x011b] = 24;
	} else if (packetver >= 20090311) {
		length_list[0x011b] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x011b] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x011b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x011b] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x011b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x011b] = 32;
	} else if (packetver >= 20090107) {
		length_list[0x011b] = -1;
	}

	// Packet: 0x011c
	if (packetver >= 20090406) {
		length_list[0x011c] = 68;
	} else if (packetver >= 20090325) {
		length_list[0x011c] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x011c] = 282;
	} else if (packetver >= 20090311) {
		length_list[0x011c] = 58;
	} else if (packetver >= 20090225) {
		length_list[0x011c] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x011c] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x011c] = -1;
	}

	// Packet: 0x011d
	if (packetver >= 20090406) {
		length_list[0x011d] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x011d] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x011d] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x011d] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x011d] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x011d] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x011d] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x011d] = 32;
	} else if (packetver >= 20090129) {
		length_list[0x011d] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x011d] = 182;
	} else if (packetver >= 20090107) {
		length_list[0x011d] = -1;
	}

	// Packet: 0x011e
	if (packetver >= 20090406) {
		length_list[0x011e] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x011e] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x011e] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x011e] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x011e] = 60;
	} else if (packetver >= 20090211) {
		length_list[0x011e] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x011e] = 22;
	} else if (packetver >= 20090120) {
		length_list[0x011e] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x011e] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x011e] = -1;
	}

	// Packet: 0x011f
	if (packetver >= 20090406) {
		length_list[0x011f] = 16;
	} else if (packetver >= 20090401) {
		length_list[0x011f] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x011f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x011f] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x011f] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x011f] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x011f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x011f] = 282;
	} else if (packetver >= 20090107) {
		length_list[0x011f] = -1;
	}

	// Packet: 0x0120
	if (packetver >= 20090406) {
		length_list[0x0120] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x0120] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0120] = 9;
	} else if (packetver >= 20090311) {
		length_list[0x0120] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0120] = 15;
	} else if (packetver >= 20090218) {
		length_list[0x0120] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0120] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x0120] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x0120] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0120] = 12;
	} else if (packetver >= 20090107) {
		length_list[0x0120] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0121
	if (packetver >= 20090406) {
		length_list[0x0121] = 14;
	} else if (packetver >= 20090318) {
		length_list[0x0121] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0121] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x0121] = 31;
	} else if (packetver >= 20090211) {
		length_list[0x0121] = 12;
	} else if (packetver >= 20090204) {
		length_list[0x0121] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0121] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0121] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x0121] = 67;
	} else if (packetver >= 20090107) {
		length_list[0x0121] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0122
	if (packetver >= 20090204) {
		length_list[0x0122] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0122] = 20;
	} else if (packetver >= 20090120) {
		length_list[0x0122] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0122] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0122] = 9;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0123
	if (packetver >= 20090401) {
		length_list[0x0123] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0123] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0123] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0123] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0123] = 22;
	} else if (packetver >= 20090114) {
		length_list[0x0123] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0123] = 26;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0124
	if (packetver >= 20090406) {
		length_list[0x0124] = 21;
	} else if (packetver >= 20090325) {
		length_list[0x0124] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0124] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x0124] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0124] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x0124] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0124] = 28;
	} else if (packetver >= 20090114) {
		length_list[0x0124] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0124] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0125
	if (packetver >= 20090406) {
		length_list[0x0125] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x0125] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0125] = 282;
	} else if (packetver >= 20090225) {
		length_list[0x0125] = 7;
	} else if (packetver >= 20090218) {
		length_list[0x0125] = 28;
	} else if (packetver >= 20090211) {
		length_list[0x0125] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x0125] = 20;
	} else if (packetver >= 20090129) {
		length_list[0x0125] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0125] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0125] = -1;
	}

	// Packet: 0x0126
	if (packetver >= 20090406) {
		length_list[0x0126] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x0126] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x0126] = 282;
	} else if (packetver >= 20090318) {
		length_list[0x0126] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x0126] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0126] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x0126] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0126] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0126] = 14;
	} else if (packetver >= 20090129) {
		length_list[0x0126] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0126] = 14;
	} else if (packetver >= 20090114) {
		length_list[0x0126] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0126] = 60;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0127
	if (packetver >= 20090406) {
		length_list[0x0127] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x0127] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0127] = 66;
	} else if (packetver >= 20090318) {
		length_list[0x0127] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0127] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x0127] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0127] = 44;
	} else if (packetver >= 20090211) {
		length_list[0x0127] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0127] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0127] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x0127] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0127] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0127] = 18;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0128
	if (packetver >= 20090406) {
		length_list[0x0128] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x0128] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x0128] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x0128] = 12;
	} else if (packetver >= 20090311) {
		length_list[0x0128] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0128] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0128] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0128] = 29;
	} else if (packetver >= 20090107) {
		length_list[0x0128] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0129
	if (packetver >= 20090406) {
		length_list[0x0129] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x0129] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0129] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0129] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x0129] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0129] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0129] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0129] = 22;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x012a
	if (packetver >= 20090406) {
		length_list[0x012a] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x012a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x012a] = 13;
	} else if (packetver >= 20090311) {
		length_list[0x012a] = 33;
	} else if (packetver >= 20090225) {
		length_list[0x012a] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x012a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x012a] = 34;
	} else if (packetver >= 20090129) {
		length_list[0x012a] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x012a] = 29;
	} else if (packetver >= 20090114) {
		length_list[0x012a] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x012a] = 11;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x012b
	length_list[0x012b] = 2;

	// Packet: 0x012c
	if (packetver >= 20090406) {
		length_list[0x012c] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x012c] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x012c] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x012c] = 13;
	} else if (packetver >= 20090129) {
		length_list[0x012c] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x012c] = 282;
	} else if (packetver >= 20090107) {
		length_list[0x012c] = 79;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x012d
	if (packetver >= 20090406) {
		length_list[0x012d] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x012d] = 43;
	} else if (packetver >= 20090325) {
		length_list[0x012d] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x012d] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x012d] = 22;
	} else if (packetver >= 20090204) {
		length_list[0x012d] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x012d] = 79;
	} else if (packetver >= 20090120) {
		length_list[0x012d] = 11;
	} else if (packetver >= 20090114) {
		length_list[0x012d] = 13;
	} else if (packetver >= 20090107) {
		length_list[0x012d] = -1;
	}

	// Packet: 0x012e
	if (packetver >= 20090406) {
		length_list[0x012e] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x012e] = 14;
	} else if (packetver >= 20090311) {
		length_list[0x012e] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x012e] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x012e] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x012e] = 28;
	} else if (packetver >= 20090114) {
		length_list[0x012e] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x012e] = 65;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x012f
	length_list[0x012f] = -1;

	// Packet: 0x0130
	if (packetver >= 20090406) {
		length_list[0x0130] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x0130] = 7;
	} else if (packetver >= 20090325) {
		length_list[0x0130] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0130] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x0130] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0130] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x0130] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0130] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x0130] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x0130] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0130] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x0130] = -1;
	}

	// Packet: 0x0131
	if (packetver >= 20090406) {
		length_list[0x0131] = 86;
	} else if (packetver >= 20090401) {
		length_list[0x0131] = 37;
	} else if (packetver >= 20090325) {
		length_list[0x0131] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0131] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0131] = 17;
	} else if (packetver >= 20090218) {
		length_list[0x0131] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0131] = 7;
	} else if (packetver >= 20090120) {
		length_list[0x0131] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0131] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x0131] = -1;
	}

	// Packet: 0x0132
	if (packetver >= 20090406) {
		length_list[0x0132] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x0132] = 11;
	} else if (packetver >= 20090318) {
		length_list[0x0132] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0132] = 28;
	} else if (packetver >= 20090225) {
		length_list[0x0132] = 39;
	} else if (packetver >= 20090218) {
		length_list[0x0132] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0132] = 27;
	} else if (packetver >= 20090204) {
		length_list[0x0132] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x0132] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0132] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0132] = -1;
	}

	// Packet: 0x0133
	if (packetver >= 20090325) {
		length_list[0x0133] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0133] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0133] = 60;
	} else if (packetver >= 20090225) {
		length_list[0x0133] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x0133] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0133] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x0133] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0133] = 68;
	} else if (packetver >= 20090107) {
		length_list[0x0133] = -1;
	}

	// Packet: 0x0134
	if (packetver >= 20090406) {
		length_list[0x0134] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0134] = 29;
	} else if (packetver >= 20090325) {
		length_list[0x0134] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x0134] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0134] = 58;
	} else if (packetver >= 20090225) {
		length_list[0x0134] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0134] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0134] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x0134] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0134] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0134] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0134] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0134] = -1;
	}

	// Packet: 0x0135
	if (packetver >= 20090406) {
		length_list[0x0135] = 7;
	} else if (packetver >= 20090401) {
		length_list[0x0135] = 97;
	} else if (packetver >= 20090311) {
		length_list[0x0135] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0135] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x0135] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0135] = 44;
	} else if (packetver >= 20090204) {
		length_list[0x0135] = 282;
	} else if (packetver >= 20090129) {
		length_list[0x0135] = 59;
	} else if (packetver >= 20090107) {
		length_list[0x0135] = -1;
	}

	// Packet: 0x0136
	if (packetver >= 20090401) {
		length_list[0x0136] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0136] = 53;
	} else if (packetver >= 20090318) {
		length_list[0x0136] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0136] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x0136] = 30;
	} else if (packetver >= 20090218) {
		length_list[0x0136] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x0136] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0136] = 16;
	} else if (packetver >= 20090114) {
		length_list[0x0136] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0136] = 26;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0137
	if (packetver >= 20090406) {
		length_list[0x0137] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x0137] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0137] = 16;
	} else if (packetver >= 20090311) {
		length_list[0x0137] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0137] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0137] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x0137] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0137] = 68;
	} else if (packetver >= 20090129) {
		length_list[0x0137] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0137] = -1;
	}

	// Packet: 0x0138
	length_list[0x0138] = 3;

	// Packet: 0x0139
	if (packetver >= 20090406) {
		length_list[0x0139] = 16;
	} else if (packetver >= 20090211) {
		length_list[0x0139] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0139] = 21;
	} else if (packetver >= 20090129) {
		length_list[0x0139] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0139] = 54;
	} else if (packetver >= 20090107) {
		length_list[0x0139] = -1;
	}

	// Packet: 0x013a
	if (packetver >= 20090406) {
		length_list[0x013a] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x013a] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x013a] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x013a] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x013a] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x013a] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x013a] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x013a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x013a] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x013a] = -1;
	}

	// Packet: 0x013b
	if (packetver >= 20090406) {
		length_list[0x013b] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x013b] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x013b] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x013b] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x013b] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x013b] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x013b] = 18;
	} else if (packetver >= 20090204) {
		length_list[0x013b] = 5;
	} else if (packetver >= 20090129) {
		length_list[0x013b] = 32;
	} else if (packetver >= 20090120) {
		length_list[0x013b] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x013b] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x013b] = 7;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x013c
	if (packetver >= 20090406) {
		length_list[0x013c] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x013c] = 34;
	} else if (packetver >= 20090325) {
		length_list[0x013c] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x013c] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x013c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x013c] = 28;
	} else if (packetver >= 20090211) {
		length_list[0x013c] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x013c] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x013c] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x013c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x013c] = 5;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x013d
	if (packetver >= 20090406) {
		length_list[0x013d] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x013d] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x013d] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x013d] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x013d] = 53;
	} else if (packetver >= 20090120) {
		length_list[0x013d] = 36;
	} else if (packetver >= 20090114) {
		length_list[0x013d] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x013d] = -1;
	}

	// Packet: 0x013e
	if (packetver >= 20090406) {
		length_list[0x013e] = 24;
	} else if (packetver >= 20090318) {
		length_list[0x013e] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x013e] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x013e] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x013e] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x013e] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x013e] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x013e] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x013e] = 23;
	} else if (packetver >= 20090114) {
		length_list[0x013e] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x013e] = 182;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x013f
	if (packetver >= 20090406) {
		length_list[0x013f] = 26;
	} else if (packetver >= 20090401) {
		length_list[0x013f] = 14;
	} else if (packetver >= 20090325) {
		length_list[0x013f] = 29;
	} else if (packetver >= 20090318) {
		length_list[0x013f] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x013f] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x013f] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x013f] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x013f] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x013f] = 53;
	} else if (packetver >= 20090129) {
		length_list[0x013f] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x013f] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x013f] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x013f] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0140
	if (packetver >= 20090406) {
		length_list[0x0140] = 22;
	} else if (packetver >= 20090318) {
		length_list[0x0140] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0140] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0140] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0140] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x0140] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0140] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0140] = -1;
	}

	// Packet: 0x0141
	if (packetver >= 20090406) {
		length_list[0x0141] = 14;
	} else if (packetver >= 20090325) {
		length_list[0x0141] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0141] = 11;
	} else if (packetver >= 20090311) {
		length_list[0x0141] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0141] = 17;
	} else if (packetver >= 20090218) {
		length_list[0x0141] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0141] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x0141] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0141] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0141] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0141] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0142
	if (packetver >= 20090406) {
		length_list[0x0142] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x0142] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x0142] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0142] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0142] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x0142] = 60;
	} else if (packetver >= 20090107) {
		length_list[0x0142] = -1;
	}

	// Packet: 0x0143
	if (packetver >= 20090406) {
		length_list[0x0143] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x0143] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x0143] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0143] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x0143] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0143] = 22;
	} else if (packetver >= 20090107) {
		length_list[0x0143] = -1;
	}

	// Packet: 0x0144
	if (packetver >= 20090406) {
		length_list[0x0144] = 23;
	} else if (packetver >= 20090218) {
		length_list[0x0144] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0144] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x0144] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0144] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0144] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0144] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0145
	length_list[0x0145] = 19;

	// Packet: 0x0146
	if (packetver >= 20090406) {
		length_list[0x0146] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x0146] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0146] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0146] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0146] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x0146] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0146] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0146] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0146] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0147
	if (packetver >= 20090406) {
		length_list[0x0147] = 39;
	} else if (packetver >= 20090401) {
		length_list[0x0147] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x0147] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0147] = 114;
	} else if (packetver >= 20090129) {
		length_list[0x0147] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x0147] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x0147] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0148
	if (packetver >= 20090406) {
		length_list[0x0148] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x0148] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0148] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x0148] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x0148] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0148] = 22;
	} else if (packetver >= 20090204) {
		length_list[0x0148] = 30;
	} else if (packetver >= 20090114) {
		length_list[0x0148] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0148] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0149
	if (packetver >= 20090406) {
		length_list[0x0149] = 9;
	} else if (packetver >= 20090401) {
		length_list[0x0149] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0149] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0149] = 22;
	} else if (packetver >= 20090311) {
		length_list[0x0149] = 29;
	} else if (packetver >= 20090225) {
		length_list[0x0149] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0149] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0149] = -1;
	}

	// Packet: 0x014a
	length_list[0x014a] = 6;

	// Packet: 0x014b
	length_list[0x014b] = 27;

	// Packet: 0x014c
	if (packetver >= 20090406) {
		length_list[0x014c] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x014c] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x014c] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x014c] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x014c] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x014c] = 57;
	} else if (packetver >= 20090204) {
		length_list[0x014c] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x014c] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x014c] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x014c] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x014c] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x014d
	if (packetver >= 20090406) {
		length_list[0x014d] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x014d] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x014d] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x014d] = 39;
	} else if (packetver >= 20090218) {
		length_list[0x014d] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x014d] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x014d] = 13;
	} else if (packetver >= 20090129) {
		length_list[0x014d] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x014d] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x014d] = 282;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x014e
	length_list[0x014e] = 6;

	// Packet: 0x014f
	if (packetver >= 20090406) {
		length_list[0x014f] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x014f] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x014f] = 16;
	} else if (packetver >= 20090318) {
		length_list[0x014f] = 58;
	} else if (packetver >= 20090311) {
		length_list[0x014f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x014f] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x014f] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x014f] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x014f] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x014f] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x014f] = 182;
	} else if (packetver >= 20090107) {
		length_list[0x014f] = -1;
	}

	// Packet: 0x0150
	length_list[0x0150] = 110;

	// Packet: 0x0151
	length_list[0x0151] = 6;

	// Packet: 0x0152
	length_list[0x0152] = -1;

	// Packet: 0x0153
	length_list[0x0153] = -1;

	// Packet: 0x0154
	if (packetver >= 20090318) {
		length_list[0x0154] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0154] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x0154] = 17;
	} else if (packetver >= 20090218) {
		length_list[0x0154] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0154] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x0154] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0154] = 21;
	} else if (packetver >= 20090107) {
		length_list[0x0154] = -1;
	}

	// Packet: 0x0155
	if (packetver >= 20090318) {
		length_list[0x0155] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0155] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x0155] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0155] = 12;
	} else if (packetver >= 20090204) {
		length_list[0x0155] = 26;
	} else if (packetver >= 20090114) {
		length_list[0x0155] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0155] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0156
	length_list[0x0156] = -1;

	// Packet: 0x0157
	length_list[0x0157] = 6;

	// Packet: 0x0158
	length_list[0x0158] = -1;

	// Packet: 0x0159
	if (packetver >= 20090406) {
		length_list[0x0159] = 54;
	} else if (packetver >= 20090401) {
		length_list[0x0159] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0159] = 34;
	} else if (packetver >= 20090318) {
		length_list[0x0159] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x0159] = 18;
	} else if (packetver >= 20090218) {
		length_list[0x0159] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0159] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x0159] = -1;
	}

	// Packet: 0x015a
	if (packetver >= 20090406) {
		length_list[0x015a] = 66;
	} else if (packetver >= 20090401) {
		length_list[0x015a] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x015a] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x015a] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x015a] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x015a] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x015a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x015a] = 14;
	} else if (packetver >= 20090129) {
		length_list[0x015a] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x015a] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x015a] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x015b
	if (packetver >= 20090406) {
		length_list[0x015b] = 54;
	} else if (packetver >= 20090401) {
		length_list[0x015b] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x015b] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x015b] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x015b] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x015b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x015b] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x015b] = 11;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x015c
	if (packetver >= 20090406) {
		length_list[0x015c] = 90;
	} else if (packetver >= 20090401) {
		length_list[0x015c] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x015c] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x015c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x015c] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x015c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x015c] = 14;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x015d
	length_list[0x015d] = 42;

	// Packet: 0x015e
	if (packetver >= 20090406) {
		length_list[0x015e] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x015e] = 26;
	} else if (packetver >= 20090311) {
		length_list[0x015e] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x015e] = 21;
	} else if (packetver >= 20090218) {
		length_list[0x015e] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x015e] = -1;
	}

	// Packet: 0x015f
	length_list[0x015f] = 42;

	// Packet: 0x0160
	if (packetver >= 20090406) {
		length_list[0x0160] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0160] = 17;
	} else if (packetver >= 20090325) {
		length_list[0x0160] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0160] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0160] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0160] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0160] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0160] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0160] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0160] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0160] = 37;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0161
	if (packetver >= 20090406) {
		length_list[0x0161] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0161] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x0161] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0161] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x0161] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x0161] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0161] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0161] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0161] = 54;
	} else if (packetver >= 20090120) {
		length_list[0x0161] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0161] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x0161] = 4;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0162
	if (packetver >= 20090406) {
		length_list[0x0162] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0162] = 54;
	} else if (packetver >= 20090225) {
		length_list[0x0162] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0162] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x0162] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0162] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0162] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0163
	if (packetver >= 20090401) {
		length_list[0x0163] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0163] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x0163] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0163] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0163] = 27;
	} else if (packetver >= 20090211) {
		length_list[0x0163] = 22;
	} else if (packetver >= 20090204) {
		length_list[0x0163] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0163] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x0163] = -1;
	}

	// Packet: 0x0164
	length_list[0x0164] = -1;

	// Packet: 0x0165
	if (packetver >= 20090406) {
		length_list[0x0165] = 30;
	} else if (packetver >= 20090401) {
		length_list[0x0165] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x0165] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0165] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x0165] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0165] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0165] = -1;
	}

	// Packet: 0x0166
	if (packetver >= 20090401) {
		length_list[0x0166] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0166] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x0166] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0166] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x0166] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0166] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0166] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0166] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0166] = -1;
	}

	// Packet: 0x0167
	if (packetver >= 20090406) {
		length_list[0x0167] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x0167] = 53;
	} else if (packetver >= 20090325) {
		length_list[0x0167] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x0167] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0167] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x0167] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0167] = 11;
	} else if (packetver >= 20090120) {
		length_list[0x0167] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0167] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x0167] = 26;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0168
	if (packetver >= 20090406) {
		length_list[0x0168] = 14;
	} else if (packetver >= 20090401) {
		length_list[0x0168] = 23;
	} else if (packetver >= 20090204) {
		length_list[0x0168] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0168] = 27;
	} else if (packetver >= 20090114) {
		length_list[0x0168] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0168] = 11;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0169
	if (packetver >= 20090406) {
		length_list[0x0169] = 3;
	} else if (packetver >= 20090401) {
		length_list[0x0169] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0169] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x0169] = 29;
	} else if (packetver >= 20090311) {
		length_list[0x0169] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0169] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x0169] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0169] = 17;
	} else if (packetver >= 20090204) {
		length_list[0x0169] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0169] = 11;
	} else if (packetver >= 20090114) {
		length_list[0x0169] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0169] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x016a
	if (packetver >= 20090406) {
		length_list[0x016a] = 30;
	} else if (packetver >= 20090401) {
		length_list[0x016a] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x016a] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x016a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x016a] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x016a] = 7;
	} else if (packetver >= 20090120) {
		length_list[0x016a] = 34;
	} else if (packetver >= 20090107) {
		length_list[0x016a] = 5;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x016b
	if (packetver >= 20090406) {
		length_list[0x016b] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x016b] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x016b] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x016b] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x016b] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x016b] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x016b] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x016b] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x016b] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x016b] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x016b] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x016b] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x016c
	if (packetver >= 20090406) {
		length_list[0x016c] = 43;
	} else if (packetver >= 20090225) {
		length_list[0x016c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x016c] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x016c] = 16;
	} else if (packetver >= 20090114) {
		length_list[0x016c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x016c] = 53;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x016d
	if (packetver >= 20090406) {
		length_list[0x016d] = 14;
	} else if (packetver >= 20090401) {
		length_list[0x016d] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x016d] = 9;
	} else if (packetver >= 20090318) {
		length_list[0x016d] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x016d] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x016d] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x016d] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x016d] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x016d] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x016d] = 11;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x016e
	if (packetver >= 20090406) {
		length_list[0x016e] = 186;
	} else if (packetver >= 20090311) {
		length_list[0x016e] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x016e] = 186;
	} else if (packetver >= 20090218) {
		length_list[0x016e] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x016e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x016e] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x016e] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x016e] = 97;
	} else if (packetver >= 20090107) {
		length_list[0x016e] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x016f
	if (packetver >= 20090406) {
		length_list[0x016f] = 182;
	} else if (packetver >= 20090401) {
		length_list[0x016f] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x016f] = 67;
	} else if (packetver >= 20090318) {
		length_list[0x016f] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x016f] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x016f] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x016f] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x016f] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x016f] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x016f] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x016f] = -1;
	}

	// Packet: 0x0170
	if (packetver >= 20090406) {
		length_list[0x0170] = 14;
	} else if (packetver >= 20090401) {
		length_list[0x0170] = 36;
	} else if (packetver >= 20090318) {
		length_list[0x0170] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0170] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0170] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0170] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0170] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x0170] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0170] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0170] = 79;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0171
	if (packetver >= 20090406) {
		length_list[0x0171] = 30;
	} else if (packetver >= 20090401) {
		length_list[0x0171] = 67;
	} else if (packetver >= 20090325) {
		length_list[0x0171] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0171] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0171] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x0171] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0171] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x0171] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0171] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0171] = 43;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0172
	if (packetver >= 20090406) {
		length_list[0x0172] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x0172] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x0172] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x0172] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0172] = 55;
	} else if (packetver >= 20090211) {
		length_list[0x0172] = 11;
	} else if (packetver >= 20090204) {
		length_list[0x0172] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0172] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0172] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0172] = -1;
	}

	// Packet: 0x0173
	length_list[0x0173] = 3;

	// Packet: 0x0174
	length_list[0x0174] = -1;

	// Packet: 0x0175
	length_list[0x0175] = 6;

	// Packet: 0x0176
	length_list[0x0176] = 106;

	// Packet: 0x0177
	if (packetver >= 20090406) {
		length_list[0x0177] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0177] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x0177] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0177] = 5;
	} else if (packetver >= 20090225) {
		length_list[0x0177] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0177] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0177] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0177] = 22;
	} else if (packetver >= 20090120) {
		length_list[0x0177] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x0177] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0177] = 4;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0178
	if (packetver >= 20090406) {
		length_list[0x0178] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x0178] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0178] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x0178] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0178] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0178] = 68;
	} else if (packetver >= 20090120) {
		length_list[0x0178] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0178] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0178] = -1;
	}

	// Packet: 0x0179
	if (packetver >= 20090406) {
		length_list[0x0179] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x0179] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x0179] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0179] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x0179] = 32;
	} else if (packetver >= 20090225) {
		length_list[0x0179] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0179] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0179] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x0179] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0179] = 33;
	} else if (packetver >= 20090107) {
		length_list[0x0179] = 53;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x017a
	if (packetver >= 20090406) {
		length_list[0x017a] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x017a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x017a] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x017a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x017a] = 29;
	} else if (packetver >= 20090129) {
		length_list[0x017a] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x017a] = 11;
	} else if (packetver >= 20090114) {
		length_list[0x017a] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x017a] = 30;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x017b
	if (packetver >= 20090401) {
		length_list[0x017b] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x017b] = 13;
	} else if (packetver >= 20090225) {
		length_list[0x017b] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x017b] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x017b] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x017b] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x017b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x017b] = 17;
	} else if (packetver >= 20090114) {
		length_list[0x017b] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x017b] = 11;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x017c
	if (packetver >= 20090406) {
		length_list[0x017c] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x017c] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x017c] = 282;
	} else if (packetver >= 20090311) {
		length_list[0x017c] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x017c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x017c] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x017c] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x017c] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x017c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x017c] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x017d
	if (packetver >= 20090406) {
		length_list[0x017d] = 7;
	} else if (packetver >= 20090401) {
		length_list[0x017d] = 53;
	} else if (packetver >= 20090325) {
		length_list[0x017d] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x017d] = 28;
	} else if (packetver >= 20090311) {
		length_list[0x017d] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x017d] = 31;
	} else if (packetver >= 20090218) {
		length_list[0x017d] = 9;
	} else if (packetver >= 20090211) {
		length_list[0x017d] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x017d] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x017d] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x017e
	if (packetver >= 20090325) {
		length_list[0x017e] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x017e] = 26;
	} else if (packetver >= 20090311) {
		length_list[0x017e] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x017e] = 30;
	} else if (packetver >= 20090218) {
		length_list[0x017e] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x017e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x017e] = 14;
	} else if (packetver >= 20090120) {
		length_list[0x017e] = 186;
	} else if (packetver >= 20090114) {
		length_list[0x017e] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x017e] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x017f
	if (packetver >= 20090311) {
		length_list[0x017f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x017f] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x017f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x017f] = 33;
	} else if (packetver >= 20090107) {
		length_list[0x017f] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0180
	length_list[0x0180] = 6;

	// Packet: 0x0181
	length_list[0x0181] = 3;

	// Packet: 0x0182
	length_list[0x0182] = 106;

	// Packet: 0x0183
	length_list[0x0183] = 10;

	// Packet: 0x0184
	length_list[0x0184] = 10;

	// Packet: 0x0185
	length_list[0x0185] = 34;

	// Packet: 0x0187
	length_list[0x0187] = 6;

	// Packet: 0x0188
	if (packetver >= 20090406) {
		length_list[0x0188] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x0188] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0188] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x0188] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0188] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x0188] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0188] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0189
	if (packetver >= 20090406) {
		length_list[0x0189] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x0189] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0189] = 90;
	} else if (packetver >= 20090204) {
		length_list[0x0189] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0189] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x0189] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x0189] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0189] = -1;
	}

	// Packet: 0x018a
	if (packetver >= 20090406) {
		length_list[0x018a] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x018a] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x018a] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x018a] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x018a] = 71;
	} else if (packetver >= 20090204) {
		length_list[0x018a] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x018a] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x018a] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x018a] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x018b
	if (packetver >= 20090401) {
		length_list[0x018b] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x018b] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x018b] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x018b] = 21;
	} else if (packetver >= 20090129) {
		length_list[0x018b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x018b] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x018b] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x018b] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x018c
	if (packetver >= 20090406) {
		length_list[0x018c] = 29;
	} else if (packetver >= 20090401) {
		length_list[0x018c] = 11;
	} else if (packetver >= 20090318) {
		length_list[0x018c] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x018c] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x018c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x018c] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x018c] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x018c] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x018c] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x018c] = 30;
	} else if (packetver >= 20090114) {
		length_list[0x018c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x018c] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x018d
	if (packetver >= 20090401) {
		length_list[0x018d] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x018d] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x018d] = 14;
	} else if (packetver >= 20090311) {
		length_list[0x018d] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x018d] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x018d] = 28;
	} else if (packetver >= 20090204) {
		length_list[0x018d] = 18;
	} else if (packetver >= 20090129) {
		length_list[0x018d] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x018d] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x018d] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x018d] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x018e
	if (packetver >= 20090406) {
		length_list[0x018e] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x018e] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x018e] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x018e] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x018e] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x018e] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x018e] = -1;
	}

	// Packet: 0x018f
	if (packetver >= 20090406) {
		length_list[0x018f] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x018f] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x018f] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x018f] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x018f] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x018f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x018f] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x018f] = -1;
	}

	// Packet: 0x0190
	length_list[0x0190] = 23;

	// Packet: 0x0191
	length_list[0x0191] = 86;

	// Packet: 0x0192
	length_list[0x0192] = 24;

	// Packet: 0x0193
	length_list[0x0193] = 2;

	// Packet: 0x0194
	if (packetver >= 20090406) {
		length_list[0x0194] = 30;
	} else if (packetver >= 20090401) {
		length_list[0x0194] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0194] = 28;
	} else if (packetver >= 20090318) {
		length_list[0x0194] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0194] = 54;
	} else if (packetver >= 20090225) {
		length_list[0x0194] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0194] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x0194] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0194] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0194] = 12;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0195
	if (packetver >= 20090406) {
		length_list[0x0195] = 102;
	} else if (packetver >= 20090401) {
		length_list[0x0195] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0195] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x0195] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0195] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0195] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x0195] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0195] = 29;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0196
	if (packetver >= 20090406) {
		length_list[0x0196] = 9;
	} else if (packetver >= 20090311) {
		length_list[0x0196] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0196] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x0196] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0196] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0196] = -1;
	}

	// Packet: 0x0197
	length_list[0x0197] = 4;

	// Packet: 0x0198
	length_list[0x0198] = 8;

	// Packet: 0x0199
	if (packetver >= 20090406) {
		length_list[0x0199] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x0199] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0199] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0199] = 67;
	} else if (packetver >= 20090218) {
		length_list[0x0199] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0199] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0199] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x0199] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0199] = 282;
	} else if (packetver >= 20090107) {
		length_list[0x0199] = -1;
	}

	// Packet: 0x019a
	if (packetver >= 20090406) {
		length_list[0x019a] = 14;
	} else if (packetver >= 20090401) {
		length_list[0x019a] = 20;
	} else if (packetver >= 20090325) {
		length_list[0x019a] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x019a] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x019a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x019a] = 33;
	} else if (packetver >= 20090204) {
		length_list[0x019a] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x019a] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x019a] = 17;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x019b
	if (packetver >= 20090406) {
		length_list[0x019b] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x019b] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x019b] = 65;
	} else if (packetver >= 20090311) {
		length_list[0x019b] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x019b] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x019b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x019b] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x019b] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x019b] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x019b] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x019c
	if (packetver >= 20090406) {
		length_list[0x019c] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x019c] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x019c] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x019c] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x019c] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x019c] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x019c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x019c] = 15;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x019d
	if (packetver >= 20090406) {
		length_list[0x019d] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x019d] = 282;
	} else if (packetver >= 20090325) {
		length_list[0x019d] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x019d] = 13;
	} else if (packetver >= 20090311) {
		length_list[0x019d] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x019d] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x019d] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x019d] = 60;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x019e
	length_list[0x019e] = 2;

	// Packet: 0x019f
	if (packetver >= 20090406) {
		length_list[0x019f] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x019f] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x019f] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x019f] = 53;
	} else if (packetver >= 20090311) {
		length_list[0x019f] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x019f] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x019f] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x019f] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x019f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x019f] = 15;
	} else if (packetver >= 20090114) {
		length_list[0x019f] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x019f] = 26;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01a0
	if (packetver >= 20090406) {
		length_list[0x01a0] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x01a0] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01a0] = 79;
	} else if (packetver >= 20090114) {
		length_list[0x01a0] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01a0] = 114;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01a1
	if (packetver >= 20090406) {
		length_list[0x01a1] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x01a1] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01a1] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x01a1] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x01a1] = -1;
	}

	// Packet: 0x01a2
	if (packetver >= 20090406) {
		length_list[0x01a2] = 37;
	} else if (packetver >= 20090401) {
		length_list[0x01a2] = 21;
	} else if (packetver >= 20090325) {
		length_list[0x01a2] = 31;
	} else if (packetver >= 20090225) {
		length_list[0x01a2] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01a2] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x01a2] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x01a2] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01a2] = 59;
	} else if (packetver >= 20090107) {
		length_list[0x01a2] = -1;
	}

	// Packet: 0x01a3
	if (packetver >= 20090406) {
		length_list[0x01a3] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x01a3] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01a3] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x01a3] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01a3] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x01a3] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01a3] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x01a3] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x01a3] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01a4
	if (packetver >= 20090406) {
		length_list[0x01a4] = 11;
	} else if (packetver >= 20090325) {
		length_list[0x01a4] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x01a4] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01a4] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x01a4] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01a4] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x01a4] = 27;
	} else if (packetver >= 20090114) {
		length_list[0x01a4] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01a4] = 20;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01a5
	length_list[0x01a5] = 26;

	// Packet: 0x01a6
	if (packetver >= 20090401) {
		length_list[0x01a6] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01a6] = 23;
	} else if (packetver >= 20090318) {
		length_list[0x01a6] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x01a6] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01a6] = 182;
	} else if (packetver >= 20090204) {
		length_list[0x01a6] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01a6] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x01a6] = 58;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01a7
	if (packetver >= 20090406) {
		length_list[0x01a7] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x01a7] = 27;
	} else if (packetver >= 20090325) {
		length_list[0x01a7] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x01a7] = 60;
	} else if (packetver >= 20090311) {
		length_list[0x01a7] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x01a7] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01a7] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x01a7] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01a7] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x01a7] = -1;
	}

	// Packet: 0x01a8
	length_list[0x01a8] = 4;

	// Packet: 0x01a9
	length_list[0x01a9] = 6;

	// Packet: 0x01aa
	if (packetver >= 20090406) {
		length_list[0x01aa] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x01aa] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01aa] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x01aa] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01aa] = 86;
	} else if (packetver >= 20090211) {
		length_list[0x01aa] = 79;
	} else if (packetver >= 20090107) {
		length_list[0x01aa] = -1;
	}

	// Packet: 0x01ab
	if (packetver >= 20090406) {
		length_list[0x01ab] = 12;
	} else if (packetver >= 20090401) {
		length_list[0x01ab] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x01ab] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01ab] = 23;
	} else if (packetver >= 20090311) {
		length_list[0x01ab] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x01ab] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x01ab] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01ab] = 57;
	} else if (packetver >= 20090129) {
		length_list[0x01ab] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01ab] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x01ab] = 5;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01ac
	if (packetver >= 20090406) {
		length_list[0x01ac] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x01ac] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01ac] = 28;
	} else if (packetver >= 20090318) {
		length_list[0x01ac] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01ac] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x01ac] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01ac] = 13;
	} else if (packetver >= 20090211) {
		length_list[0x01ac] = 68;
	} else if (packetver >= 20090120) {
		length_list[0x01ac] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01ac] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x01ac] = -1;
	}

	// Packet: 0x01ad
	if (packetver >= 20090401) {
		length_list[0x01ad] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01ad] = 24;
	} else if (packetver >= 20090225) {
		length_list[0x01ad] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01ad] = 90;
	} else if (packetver >= 20090211) {
		length_list[0x01ad] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01ad] = 27;
	} else if (packetver >= 20090129) {
		length_list[0x01ad] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x01ad] = 60;
	} else if (packetver >= 20090114) {
		length_list[0x01ad] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x01ad] = -1;
	}

	// Packet: 0x01ae
	if (packetver >= 20090406) {
		length_list[0x01ae] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x01ae] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01ae] = 18;
	} else if (packetver >= 20090311) {
		length_list[0x01ae] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x01ae] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x01ae] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x01ae] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x01ae] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01ae] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x01ae] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01af
	length_list[0x01af] = 4;

	// Packet: 0x01b0
	if (packetver >= 20090406) {
		length_list[0x01b0] = 11;
	} else if (packetver >= 20090325) {
		length_list[0x01b0] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01b0] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x01b0] = -1;
	}

	// Packet: 0x01b1
	length_list[0x01b1] = 7;

	// Packet: 0x01b2
	if (packetver >= 20090406) {
		length_list[0x01b2] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x01b2] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x01b2] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x01b2] = 44;
	} else if (packetver >= 20090225) {
		length_list[0x01b2] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01b2] = 58;
	} else if (packetver >= 20090204) {
		length_list[0x01b2] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01b2] = 7;
	} else if (packetver >= 20090120) {
		length_list[0x01b2] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01b2] = 60;
	} else if (packetver >= 20090107) {
		length_list[0x01b2] = -1;
	}

	// Packet: 0x01b3
	if (packetver >= 20090406) {
		length_list[0x01b3] = 67;
	} else if (packetver >= 20090401) {
		length_list[0x01b3] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01b3] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x01b3] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x01b3] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01b3] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x01b3] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01b3] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01b4
	length_list[0x01b4] = 12;

	// Packet: 0x01b5
	length_list[0x01b5] = 18;

	// Packet: 0x01b6
	if (packetver >= 20090406) {
		length_list[0x01b6] = 114;
	} else if (packetver >= 20090401) {
		length_list[0x01b6] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x01b6] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x01b6] = 33;
	} else if (packetver >= 20090311) {
		length_list[0x01b6] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x01b6] = 18;
	} else if (packetver >= 20090218) {
		length_list[0x01b6] = 27;
	} else if (packetver >= 20090211) {
		length_list[0x01b6] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01b6] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x01b6] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01b6] = 13;
	} else if (packetver >= 20090107) {
		length_list[0x01b6] = 16;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01b7
	length_list[0x01b7] = 6;

	// Packet: 0x01b8
	length_list[0x01b8] = 3;

	// Packet: 0x01b9
	if (packetver >= 20090406) {
		length_list[0x01b9] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x01b9] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01b9] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x01b9] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x01b9] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x01b9] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01b9] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x01b9] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x01b9] = -1;
	}

	// Packet: 0x01ba
	if (packetver >= 20090406) {
		length_list[0x01ba] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x01ba] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01ba] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x01ba] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01ba] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x01ba] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x01ba] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x01ba] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01ba] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x01ba] = 15;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01bb
	if (packetver >= 20090406) {
		length_list[0x01bb] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x01bb] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01bb] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x01bb] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01bb] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x01bb] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x01bb] = 12;
	} else if (packetver >= 20090129) {
		length_list[0x01bb] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01bb] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x01bb] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01bb] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01bc
	if (packetver >= 20090406) {
		length_list[0x01bc] = 26;
	} else if (packetver >= 20090401) {
		length_list[0x01bc] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x01bc] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01bc] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x01bc] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01bc] = 14;
	} else if (packetver >= 20090129) {
		length_list[0x01bc] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01bc] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x01bc] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01bd
	if (packetver >= 20090406) {
		length_list[0x01bd] = 26;
	} else if (packetver >= 20090401) {
		length_list[0x01bd] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01bd] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x01bd] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01bd] = 22;
	} else if (packetver >= 20090211) {
		length_list[0x01bd] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01bd] = 65;
	} else if (packetver >= 20090129) {
		length_list[0x01bd] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x01bd] = -1;
	}

	// Packet: 0x01be
	length_list[0x01be] = 2;

	// Packet: 0x01bf
	length_list[0x01bf] = 3;

	// Packet: 0x01c0
	length_list[0x01c0] = 2;

	// Packet: 0x01c1
	length_list[0x01c1] = 14;

	// Packet: 0x01c2
	length_list[0x01c2] = 10;

	// Packet: 0x01c3
	if (packetver >= 20090318) {
		length_list[0x01c3] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01c3] = 15;
	} else if (packetver >= 20090218) {
		length_list[0x01c3] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01c3] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x01c3] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01c3] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x01c3] = 15;
	} else if (packetver >= 20090107) {
		length_list[0x01c3] = -1;
	}

	// Packet: 0x01c4
	if (packetver >= 20090406) {
		length_list[0x01c4] = 22;
	} else if (packetver >= 20090401) {
		length_list[0x01c4] = 81;
	} else if (packetver >= 20090325) {
		length_list[0x01c4] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x01c4] = 16;
	} else if (packetver >= 20090311) {
		length_list[0x01c4] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x01c4] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x01c4] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01c4] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x01c4] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01c4] = 9;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01c5
	if (packetver >= 20090406) {
		length_list[0x01c5] = 22;
	} else if (packetver >= 20090401) {
		length_list[0x01c5] = 32;
	} else if (packetver >= 20090325) {
		length_list[0x01c5] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01c5] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x01c5] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x01c5] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01c5] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x01c5] = -1;
	}

	// Packet: 0x01c6
	length_list[0x01c6] = 4;

	// Packet: 0x01c7
	length_list[0x01c7] = 2;

	// Packet: 0x01c8
	if (packetver >= 20090406) {
		length_list[0x01c8] = 13;
	} else if (packetver >= 20090325) {
		length_list[0x01c8] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01c8] = 43;
	} else if (packetver >= 20090311) {
		length_list[0x01c8] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x01c8] = 53;
	} else if (packetver >= 20090211) {
		length_list[0x01c8] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01c8] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x01c8] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01c8] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x01c8] = -1;
	}

	// Packet: 0x01c9
	if (packetver >= 20090406) {
		length_list[0x01c9] = 97;
	} else if (packetver >= 20090401) {
		length_list[0x01c9] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01c9] = 28;
	} else if (packetver >= 20090318) {
		length_list[0x01c9] = 54;
	} else if (packetver >= 20090311) {
		length_list[0x01c9] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x01c9] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x01c9] = 282;
	} else if (packetver >= 20090211) {
		length_list[0x01c9] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x01c9] = 14;
	} else if (packetver >= 20090129) {
		length_list[0x01c9] = 102;
	} else if (packetver >= 20090120) {
		length_list[0x01c9] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x01c9] = 79;
	} else if (packetver >= 20090107) {
		length_list[0x01c9] = 32;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01ca
	length_list[0x01ca] = 3;

	// Packet: 0x01cb
	length_list[0x01cb] = 9;

	// Packet: 0x01cc
	length_list[0x01cc] = 9;

	// Packet: 0x01cd
	if (packetver >= 20090406) {
		length_list[0x01cd] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x01cd] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01cd] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x01cd] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01cd] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x01cd] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x01cd] = 18;
	} else if (packetver >= 20090114) {
		length_list[0x01cd] = 33;
	} else if (packetver >= 20090107) {
		length_list[0x01cd] = 20;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01ce
	if (packetver >= 20090406) {
		length_list[0x01ce] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x01ce] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01ce] = 26;
	} else if (packetver >= 20090211) {
		length_list[0x01ce] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01ce] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x01ce] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x01ce] = -1;
	}

	// Packet: 0x01cf
	if (packetver >= 20090406) {
		length_list[0x01cf] = 28;
	} else if (packetver >= 20090225) {
		length_list[0x01cf] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01cf] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x01cf] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01cf] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01d0
	if (packetver >= 20090406) {
		length_list[0x01d0] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x01d0] = 23;
	} else if (packetver >= 20090325) {
		length_list[0x01d0] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01d0] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x01d0] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01d0] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x01d0] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x01d0] = 36;
	} else if (packetver >= 20090129) {
		length_list[0x01d0] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x01d0] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01d0] = 30;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01d1
	length_list[0x01d1] = 14;

	// Packet: 0x01d2
	if (packetver >= 20090406) {
		length_list[0x01d2] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x01d2] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01d2] = 54;
	} else if (packetver >= 20090318) {
		length_list[0x01d2] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01d2] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x01d2] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x01d2] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x01d2] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01d2] = 22;
	} else if (packetver >= 20090107) {
		length_list[0x01d2] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01d3
	length_list[0x01d3] = 35;

	// Packet: 0x01d4
	if (packetver >= 20090406) {
		length_list[0x01d4] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x01d4] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01d4] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x01d4] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x01d4] = 53;
	} else if (packetver >= 20090204) {
		length_list[0x01d4] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01d4] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x01d4] = -1;
	}

	// Packet: 0x01d5
	if (packetver >= 20090401) {
		length_list[0x01d5] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01d5] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x01d5] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01d5] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x01d5] = -1;
	}

	// Packet: 0x01d6
	if (packetver >= 20090406) {
		length_list[0x01d6] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x01d6] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x01d6] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x01d6] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x01d6] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x01d6] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01d6] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x01d6] = 26;
	} else if (packetver >= 20090114) {
		length_list[0x01d6] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01d6] = 11;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01d7
	if (packetver >= 20090406) {
		length_list[0x01d7] = 11;
	} else if (packetver >= 20090401) {
		length_list[0x01d7] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01d7] = 14;
	} else if (packetver >= 20090311) {
		length_list[0x01d7] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x01d7] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x01d7] = 59;
	} else if (packetver >= 20090204) {
		length_list[0x01d7] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01d7] = 16;
	} else if (packetver >= 20090107) {
		length_list[0x01d7] = 114;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01d8
	if (packetver >= 20090406) {
		length_list[0x01d8] = 54;
	} else if (packetver >= 20090401) {
		length_list[0x01d8] = 19;
	} else if (packetver >= 20090325) {
		length_list[0x01d8] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01d8] = 57;
	} else if (packetver >= 20090129) {
		length_list[0x01d8] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01d8] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x01d8] = 39;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01d9
	if (packetver >= 20090406) {
		length_list[0x01d9] = 53;
	} else if (packetver >= 20090325) {
		length_list[0x01d9] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01d9] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x01d9] = 31;
	} else if (packetver >= 20090218) {
		length_list[0x01d9] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01d9] = 11;
	} else if (packetver >= 20090204) {
		length_list[0x01d9] = 22;
	} else if (packetver >= 20090129) {
		length_list[0x01d9] = 37;
	} else if (packetver >= 20090120) {
		length_list[0x01d9] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01d9] = 54;
	} else if (packetver >= 20090107) {
		length_list[0x01d9] = 28;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01da
	if (packetver >= 20090406) {
		length_list[0x01da] = 60;
	} else if (packetver >= 20090401) {
		length_list[0x01da] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01da] = 11;
	} else if (packetver >= 20090204) {
		length_list[0x01da] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01da] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x01da] = 22;
	} else if (packetver >= 20090114) {
		length_list[0x01da] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x01da] = -1;
	}

	// Packet: 0x01db
	length_list[0x01db] = 2;

	// Packet: 0x01dc
	length_list[0x01dc] = -1;

	// Packet: 0x01dd
	length_list[0x01dd] = 47;

	// Packet: 0x01de
	if (packetver >= 20090406) {
		length_list[0x01de] = 33;
	} else if (packetver >= 20090401) {
		length_list[0x01de] = 18;
	} else if (packetver >= 20090318) {
		length_list[0x01de] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01de] = 67;
	} else if (packetver >= 20090225) {
		length_list[0x01de] = 59;
	} else if (packetver >= 20090218) {
		length_list[0x01de] = 11;
	} else if (packetver >= 20090204) {
		length_list[0x01de] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01de] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x01de] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01de] = 282;
	} else if (packetver >= 20090107) {
		length_list[0x01de] = -1;
	}

	// Packet: 0x01df
	length_list[0x01df] = 6;

	// Packet: 0x01e0
	length_list[0x01e0] = 30;

	// Packet: 0x01e1
	if (packetver >= 20090406) {
		length_list[0x01e1] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x01e1] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01e1] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x01e1] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01e1] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x01e1] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01e1] = 26;
	} else if (packetver >= 20090114) {
		length_list[0x01e1] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x01e1] = -1;
	}

	// Packet: 0x01e2
	length_list[0x01e2] = 34;

	// Packet: 0x01e3
	length_list[0x01e3] = 14;

	// Packet: 0x01e4
	length_list[0x01e4] = 2;

	// Packet: 0x01e5
	length_list[0x01e5] = 6;

	// Packet: 0x01e6
	length_list[0x01e6] = 26;

	// Packet: 0x01e7
	if (packetver >= 20090406) {
		length_list[0x01e7] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x01e7] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x01e7] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01e7] = 86;
	} else if (packetver >= 20090311) {
		length_list[0x01e7] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x01e7] = 7;
	} else if (packetver >= 20090218) {
		length_list[0x01e7] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01e7] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x01e7] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01e7] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x01e7] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01e7] = 15;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01e8
	if (packetver >= 20090406) {
		length_list[0x01e8] = 28;
	} else if (packetver >= 20090401) {
		length_list[0x01e8] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x01e8] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x01e8] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01e8] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x01e8] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01e8] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x01e8] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01e8] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x01e8] = -1;
	}

	// Packet: 0x01e9
	if (packetver >= 20090406) {
		length_list[0x01e9] = 81;
	} else if (packetver >= 20090325) {
		length_list[0x01e9] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01e9] = 28;
	} else if (packetver >= 20090211) {
		length_list[0x01e9] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01e9] = 79;
	} else if (packetver >= 20090129) {
		length_list[0x01e9] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x01e9] = 27;
	} else if (packetver >= 20090114) {
		length_list[0x01e9] = 55;
	} else if (packetver >= 20090107) {
		length_list[0x01e9] = 17;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01ea
	if (packetver >= 20090406) {
		length_list[0x01ea] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x01ea] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01ea] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x01ea] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x01ea] = 59;
	} else if (packetver >= 20090225) {
		length_list[0x01ea] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01ea] = 66;
	} else if (packetver >= 20090211) {
		length_list[0x01ea] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01ea] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x01ea] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01ea] = 90;
	} else if (packetver >= 20090114) {
		length_list[0x01ea] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01ea] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01eb
	if (packetver >= 20090406) {
		length_list[0x01eb] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x01eb] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x01eb] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x01eb] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01eb] = 33;
	} else if (packetver >= 20090211) {
		length_list[0x01eb] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x01eb] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01eb] = 23;
	} else if (packetver >= 20090120) {
		length_list[0x01eb] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01eb] = 59;
	} else if (packetver >= 20090107) {
		length_list[0x01eb] = -1;
	}

	// Packet: 0x01ec
	length_list[0x01ec] = 26;

	// Packet: 0x01ed
	if (packetver >= 20090406) {
		length_list[0x01ed] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x01ed] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01ed] = 5;
	} else if (packetver >= 20090311) {
		length_list[0x01ed] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01ed] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x01ed] = 67;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01ee
	if (packetver >= 20090401) {
		length_list[0x01ee] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01ee] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x01ee] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x01ee] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x01ee] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01ee] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x01ee] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01ee] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x01ee] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x01ee] = -1;
	}

	// Packet: 0x01ef
	if (packetver >= 20090325) {
		length_list[0x01ef] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01ef] = 9;
	} else if (packetver >= 20090225) {
		length_list[0x01ef] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x01ef] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01ef] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x01ef] = 18;
	} else if (packetver >= 20090129) {
		length_list[0x01ef] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01ef] = 14;
	} else if (packetver >= 20090114) {
		length_list[0x01ef] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x01ef] = 14;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01f0
	if (packetver >= 20090401) {
		length_list[0x01f0] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01f0] = 60;
	} else if (packetver >= 20090318) {
		length_list[0x01f0] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x01f0] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x01f0] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x01f0] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x01f0] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x01f0] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01f1
	length_list[0x01f1] = -1;

	// Packet: 0x01f2
	if (packetver >= 20090406) {
		length_list[0x01f2] = 20;
	} else if (packetver >= 20090325) {
		length_list[0x01f2] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01f2] = 11;
	} else if (packetver >= 20090311) {
		length_list[0x01f2] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x01f2] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01f2] = 18;
	} else if (packetver >= 20090211) {
		length_list[0x01f2] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01f2] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x01f2] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01f2] = 12;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01f3
	length_list[0x01f3] = 10;

	// Packet: 0x01f4
	if (packetver >= 20090406) {
		length_list[0x01f4] = 32;
	} else if (packetver >= 20090401) {
		length_list[0x01f4] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01f4] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x01f4] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01f4] = 20;
	} else if (packetver >= 20090225) {
		length_list[0x01f4] = 37;
	} else if (packetver >= 20090120) {
		length_list[0x01f4] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01f4] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x01f4] = 12;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01f5
	if (packetver >= 20090406) {
		length_list[0x01f5] = 9;
	} else if (packetver >= 20090225) {
		length_list[0x01f5] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x01f5] = 26;
	} else if (packetver >= 20090211) {
		length_list[0x01f5] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x01f5] = 54;
	} else if (packetver >= 20090129) {
		length_list[0x01f5] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x01f5] = 11;
	} else if (packetver >= 20090114) {
		length_list[0x01f5] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01f5] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x01f6
	length_list[0x01f6] = 34;

	// Packet: 0x01f7
	length_list[0x01f7] = 14;

	// Packet: 0x01f8
	length_list[0x01f8] = 2;

	// Packet: 0x01f9
	length_list[0x01f9] = 6;

	// Packet: 0x01fa
	length_list[0x01fa] = 48;

	// Packet: 0x01fb
	length_list[0x01fb] = 56;

	// Packet: 0x01fc
	if (packetver >= 20090325) {
		length_list[0x01fc] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x01fc] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x01fc] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x01fc] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x01fc] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x01fc] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x01fc] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x01fc] = 11;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01fd
	if (packetver >= 20090406) {
		length_list[0x01fd] = 15;
	} else if (packetver >= 20090401) {
		length_list[0x01fd] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x01fd] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x01fd] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x01fd] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x01fd] = 29;
	} else if (packetver >= 20090218) {
		length_list[0x01fd] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x01fd] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01fd] = 53;
	} else if (packetver >= 20090107) {
		length_list[0x01fd] = -1;
	}

	// Packet: 0x01fe
	if (packetver >= 20090406) {
		length_list[0x01fe] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x01fe] = 28;
	} else if (packetver >= 20090325) {
		length_list[0x01fe] = 44;
	} else if (packetver >= 20090120) {
		length_list[0x01fe] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x01fe] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x01fe] = 15;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x01ff
	length_list[0x01ff] = 10;

	// Packet: 0x0200
	length_list[0x0200] = 26;

	// Packet: 0x0201
	if (packetver >= 20090406) {
		length_list[0x0201] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0201] = 58;
	} else if (packetver >= 20090225) {
		length_list[0x0201] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0201] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x0201] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0201] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0201] = -1;
	}

	// Packet: 0x0202
	if (packetver >= 20090406) {
		length_list[0x0202] = 26;
	} else if (packetver >= 20090401) {
		length_list[0x0202] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0202] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0202] = 27;
	} else if (packetver >= 20090204) {
		length_list[0x0202] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0202] = 13;
	} else if (packetver >= 20090114) {
		length_list[0x0202] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0202] = 11;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0203
	if (packetver >= 20090406) {
		length_list[0x0203] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x0203] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x0203] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x0203] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0203] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x0203] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x0203] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0203] = 29;
	} else if (packetver >= 20090204) {
		length_list[0x0203] = 67;
	} else if (packetver >= 20090129) {
		length_list[0x0203] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x0203] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0203] = -1;
	}

	// Packet: 0x0204
	length_list[0x0204] = 18;

	// Packet: 0x0205
	length_list[0x0205] = 26;

	// Packet: 0x0206
	if (packetver >= 20090406) {
		length_list[0x0206] = 11;
	} else if (packetver >= 20090401) {
		length_list[0x0206] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0206] = 18;
	} else if (packetver >= 20090318) {
		length_list[0x0206] = 71;
	} else if (packetver >= 20090311) {
		length_list[0x0206] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0206] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x0206] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x0206] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0206] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x0206] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0206] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0206] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0207
	if (packetver >= 20090406) {
		length_list[0x0207] = 34;
	} else if (packetver >= 20090401) {
		length_list[0x0207] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0207] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x0207] = 9;
	} else if (packetver >= 20090311) {
		length_list[0x0207] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0207] = 7;
	} else if (packetver >= 20090218) {
		length_list[0x0207] = 9;
	} else if (packetver >= 20090211) {
		length_list[0x0207] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0207] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x0207] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x0207] = -1;
	}

	// Packet: 0x0208
	if (packetver >= 20090406) {
		length_list[0x0208] = 14;
	} else if (packetver >= 20090401) {
		length_list[0x0208] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0208] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x0208] = 31;
	} else if (packetver >= 20090311) {
		length_list[0x0208] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x0208] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0208] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x0208] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0208] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x0208] = -1;
	}

	// Packet: 0x0209
	if (packetver >= 20090406) {
		length_list[0x0209] = 36;
	} else if (packetver >= 20090401) {
		length_list[0x0209] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0209] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x0209] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0209] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x0209] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0209] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0209] = 86;
	} else if (packetver >= 20090120) {
		length_list[0x0209] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0209] = -1;
	}

	// Packet: 0x020a
	if (packetver >= 20090406) {
		length_list[0x020a] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x020a] = 31;
	} else if (packetver >= 20090311) {
		length_list[0x020a] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x020a] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x020a] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x020a] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x020a] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x020a] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x020d
	length_list[0x020d] = -1;

	// Packet: 0x020e
	if (packetver >= 20090406) {
		length_list[0x020e] = 32;
	} else if (packetver >= 20090318) {
		length_list[0x020e] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x020e] = 12;
	} else if (packetver >= 20090225) {
		length_list[0x020e] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x020e] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x020e] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x020e] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x020e] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x020e] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x020e] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0212
	length_list[0x0212] = 26;

	// Packet: 0x0213
	length_list[0x0213] = 26;

	// Packet: 0x0214
	length_list[0x0214] = 42;

	// Packet: 0x0215
	length_list[0x0215] = 6;

	// Packet: 0x0216
	length_list[0x0216] = 6;

	// Packet: 0x0217
	if (packetver >= 20090406) {
		length_list[0x0217] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x0217] = 102;
	} else if (packetver >= 20090325) {
		length_list[0x0217] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x0217] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0217] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x0217] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0217] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x0217] = 24;
	} else if (packetver >= 20090129) {
		length_list[0x0217] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x0217] = 14;
	} else if (packetver >= 20090114) {
		length_list[0x0217] = 44;
	} else if (packetver >= 20090107) {
		length_list[0x0217] = -1;
	}

	// Packet: 0x0218
	if (packetver >= 20090406) {
		length_list[0x0218] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x0218] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0218] = 28;
	} else if (packetver >= 20090225) {
		length_list[0x0218] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0218] = 9;
	} else if (packetver >= 20090211) {
		length_list[0x0218] = 66;
	} else if (packetver >= 20090204) {
		length_list[0x0218] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0218] = -1;
	}

	// Packet: 0x0219
	if (packetver >= 20090406) {
		length_list[0x0219] = 282;
	} else if (packetver >= 20090311) {
		length_list[0x0219] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0219] = 28;
	} else if (packetver >= 20090218) {
		length_list[0x0219] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0219] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0219] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0219] = 58;
	} else if (packetver >= 20090120) {
		length_list[0x0219] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0219] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0219] = 282;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x021a
	if (packetver >= 20090406) {
		length_list[0x021a] = 282;
	} else if (packetver >= 20090325) {
		length_list[0x021a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x021a] = 23;
	} else if (packetver >= 20090107) {
		length_list[0x021a] = -1;
	}

	// Packet: 0x021b
	if (packetver >= 20090406) {
		length_list[0x021b] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x021b] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x021b] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x021b] = 15;
	} else if (packetver >= 20090204) {
		length_list[0x021b] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x021b] = 55;
	} else if (packetver >= 20090107) {
		length_list[0x021b] = 28;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x021c
	if (packetver >= 20090406) {
		length_list[0x021c] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x021c] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x021c] = 11;
	} else if (packetver >= 20090311) {
		length_list[0x021c] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x021c] = 9;
	} else if (packetver >= 20090218) {
		length_list[0x021c] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x021c] = 11;
	} else if (packetver >= 20090204) {
		length_list[0x021c] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x021c] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x021c] = 79;
	} else if (packetver >= 20090107) {
		length_list[0x021c] = -1;
	}

	// Packet: 0x021d
	length_list[0x021d] = 6;

	// Packet: 0x021e
	length_list[0x021e] = 6;

	// Packet: 0x021f
	length_list[0x021f] = 66;

	// Packet: 0x0220
	length_list[0x0220] = 10;

	// Packet: 0x0221
	length_list[0x0221] = -1;

	// Packet: 0x0222
	length_list[0x0222] = 6;

	// Packet: 0x0223
	length_list[0x0223] = 8;

	// Packet: 0x0224
	if (packetver >= 20090406) {
		length_list[0x0224] = 10;
	} else if (packetver >= 20090401) {
		length_list[0x0224] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0224] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x0224] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0224] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0224] = 19;
	} else if (packetver >= 20090211) {
		length_list[0x0224] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x0224] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0224] = 16;
	} else if (packetver >= 20090120) {
		length_list[0x0224] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x0224] = 60;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0225
	if (packetver >= 20090406) {
		length_list[0x0225] = 2;
	} else if (packetver >= 20090401) {
		length_list[0x0225] = 66;
	} else if (packetver >= 20090325) {
		length_list[0x0225] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0225] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x0225] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x0225] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0225] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0225] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0225] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0225] = 44;
	} else if (packetver >= 20090107) {
		length_list[0x0225] = -1;
	}

	// Packet: 0x0226
	if (packetver >= 20090406) {
		length_list[0x0226] = 282;
	} else if (packetver >= 20090401) {
		length_list[0x0226] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0226] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x0226] = 53;
	} else if (packetver >= 20090311) {
		length_list[0x0226] = 55;
	} else if (packetver >= 20090218) {
		length_list[0x0226] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0226] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0226] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0226] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0226] = 102;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0227
	if (packetver >= 20090406) {
		length_list[0x0227] = 18;
	} else if (packetver >= 20090401) {
		length_list[0x0227] = 22;
	} else if (packetver >= 20090311) {
		length_list[0x0227] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0227] = 86;
	} else if (packetver >= 20090218) {
		length_list[0x0227] = 17;
	} else if (packetver >= 20090211) {
		length_list[0x0227] = 20;
	} else if (packetver >= 20090204) {
		length_list[0x0227] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0227] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x0227] = 29;
	} else if (packetver >= 20090114) {
		length_list[0x0227] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0227] = 9;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0228
	length_list[0x0228] = 18;

	// Packet: 0x0229
	if (packetver >= 20090406) {
		length_list[0x0229] = 15;
	} else if (packetver >= 20090401) {
		length_list[0x0229] = 33;
	} else if (packetver >= 20090325) {
		length_list[0x0229] = 39;
	} else if (packetver >= 20090318) {
		length_list[0x0229] = 27;
	} else if (packetver >= 20090129) {
		length_list[0x0229] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0229] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x0229] = -1;
	}

	// Packet: 0x022a
	if (packetver >= 20090406) {
		length_list[0x022a] = 58;
	} else if (packetver >= 20090401) {
		length_list[0x022a] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x022a] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x022a] = 26;
	} else if (packetver >= 20090311) {
		length_list[0x022a] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x022a] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x022a] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x022a] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x022a] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x022a] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x022a] = 282;
	} else if (packetver >= 20090114) {
		length_list[0x022a] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x022a] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x022b
	if (packetver >= 20090406) {
		length_list[0x022b] = 57;
	} else if (packetver >= 20090401) {
		length_list[0x022b] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x022b] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x022b] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x022b] = 12;
	} else if (packetver >= 20090211) {
		length_list[0x022b] = 16;
	} else if (packetver >= 20090129) {
		length_list[0x022b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x022b] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x022b] = -1;
	}

	// Packet: 0x022c
	if (packetver >= 20090406) {
		length_list[0x022c] = 65;
	} else if (packetver >= 20090401) {
		length_list[0x022c] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x022c] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x022c] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x022c] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x022c] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x022c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x022c] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x022d
	if (packetver >= 20090406) {
		length_list[0x022d] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x022d] = 27;
	} else if (packetver >= 20090325) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20090318) {
		length_list[0x022d] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x022d] = 28;
	} else if (packetver >= 20090225) {
		length_list[0x022d] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x022d] = 67;
	} else if (packetver >= 20090211) {
		length_list[0x022d] = 31;
	} else if (packetver >= 20090204) {
		length_list[0x022d] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x022d] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x022d] = 14;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x022e
	if (packetver >= 20090406) {
		length_list[0x022e] = 71;
	} else if (packetver >= 20090401) {
		length_list[0x022e] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x022e] = 16;
	} else if (packetver >= 20090311) {
		length_list[0x022e] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x022e] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x022e] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x022e] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x022e] = 186;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x022f
	if (packetver >= 20090406) {
		length_list[0x022f] = 5;
	} else if (packetver >= 20090401) {
		length_list[0x022f] = 17;
	} else if (packetver >= 20090325) {
		length_list[0x022f] = 39;
	} else if (packetver >= 20090318) {
		length_list[0x022f] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x022f] = 23;
	} else if (packetver >= 20090225) {
		length_list[0x022f] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x022f] = 12;
	} else if (packetver >= 20090211) {
		length_list[0x022f] = 5;
	} else if (packetver >= 20090129) {
		length_list[0x022f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x022f] = 31;
	} else if (packetver >= 20090114) {
		length_list[0x022f] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x022f] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0230
	if (packetver >= 20090406) {
		length_list[0x0230] = 12;
	} else if (packetver >= 20090311) {
		length_list[0x0230] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0230] = 30;
	} else if (packetver >= 20090218) {
		length_list[0x0230] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0230] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x0230] = 15;
	} else if (packetver >= 20090129) {
		length_list[0x0230] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0230] = 15;
	} else if (packetver >= 20090114) {
		length_list[0x0230] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x0230] = -1;
	}

	// Packet: 0x0231
	length_list[0x0231] = 26;

	// Packet: 0x0232
	if (packetver >= 20090406) {
		length_list[0x0232] = 9;
	} else if (packetver >= 20090325) {
		length_list[0x0232] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0232] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0232] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0232] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x0232] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0232] = 3;
	}

	// Packet: 0x0233
	if (packetver >= 20090406) {
		length_list[0x0233] = 11;
	} else if (packetver >= 20090401) {
		length_list[0x0233] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x0233] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0233] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0233] = 60;
	} else if (packetver >= 20090218) {
		length_list[0x0233] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x0233] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0233] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0233] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0233] = 7;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0234
	if (packetver >= 20090406) {
		length_list[0x0234] = 6;
	} else if (packetver >= 20090401) {
		length_list[0x0234] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x0234] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0234] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0234] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x0234] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0234] = 16;
	} else if (packetver >= 20090129) {
		length_list[0x0234] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0234] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0234] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0235
	if (packetver >= 20090311) {
		length_list[0x0235] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0235] = 22;
	} else if (packetver >= 20090218) {
		length_list[0x0235] = 79;
	} else if (packetver >= 20090211) {
		length_list[0x0235] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0235] = 21;
	} else if (packetver >= 20090107) {
		length_list[0x0235] = -1;
	}

	// Packet: 0x0236
	length_list[0x0236] = 10;

	// Packet: 0x0237
	if (packetver >= 20090401) {
		length_list[0x0237] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x0237] = 43;
	} else if (packetver >= 20090211) {
		length_list[0x0237] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0237] = 19;
	} else if (packetver >= 20090129) {
		length_list[0x0237] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0237] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0237] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0237] = -1;
	}

	// Packet: 0x0238
	if (packetver >= 20090406) {
		length_list[0x0238] = 282;
	} else if (packetver >= 20090401) {
		length_list[0x0238] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x0238] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0238] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0238] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0238] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0238] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x0238] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0238] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0238] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0238] = 12;
	} else if (packetver >= 20090107) {
		length_list[0x0238] = 22;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0239
	if (packetver >= 20090406) {
		length_list[0x0239] = 11;
	} else if (packetver >= 20090401) {
		length_list[0x0239] = 12;
	} else if (packetver >= 20090325) {
		length_list[0x0239] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0239] = 97;
	} else if (packetver >= 20090218) {
		length_list[0x0239] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0239] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x0239] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0239] = 66;
	} else if (packetver >= 20090120) {
		length_list[0x0239] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0239] = 71;
	} else if (packetver >= 20090107) {
		length_list[0x0239] = 7;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x023a
	if (packetver >= 20090406) {
		length_list[0x023a] = 4;
	} else if (packetver >= 20090401) {
		length_list[0x023a] = 15;
	} else if (packetver >= 20090318) {
		length_list[0x023a] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x023a] = 79;
	} else if (packetver >= 20090225) {
		length_list[0x023a] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x023a] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x023a] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x023a] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x023a] = 22;
	} else if (packetver >= 20090107) {
		length_list[0x023a] = 90;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x023b
	length_list[0x023b] = 36;

	// Packet: 0x023c
	if (packetver >= 20090406) {
		length_list[0x023c] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x023c] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x023c] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x023c] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x023c] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x023c] = -1;
	}

	// Packet: 0x023d
	length_list[0x023d] = 6;

	// Packet: 0x023e
	length_list[0x023e] = 8;

	// Packet: 0x023f
	length_list[0x023f] = 2;

	// Packet: 0x0240
	length_list[0x0240] = -1;

	// Packet: 0x0241
	length_list[0x0241] = 6;

	// Packet: 0x0242
	length_list[0x0242] = -1;

	// Packet: 0x0243
	length_list[0x0243] = 6;

	// Packet: 0x0244
	length_list[0x0244] = 6;

	// Packet: 0x0245
	length_list[0x0245] = 3;

	// Packet: 0x0246
	length_list[0x0246] = 4;

	// Packet: 0x0247
	length_list[0x0247] = 8;

	// Packet: 0x0248
	length_list[0x0248] = -1;

	// Packet: 0x0249
	length_list[0x0249] = 3;

	// Packet: 0x024a
	length_list[0x024a] = 70;

	// Packet: 0x024b
	length_list[0x024b] = 4;

	// Packet: 0x024c
	length_list[0x024c] = 8;

	// Packet: 0x024d
	length_list[0x024d] = 12;

	// Packet: 0x024e
	length_list[0x024e] = 6;

	// Packet: 0x024f
	length_list[0x024f] = 10;

	// Packet: 0x0250
	length_list[0x0250] = 3;

	// Packet: 0x0251
	length_list[0x0251] = 34;

	// Packet: 0x0252
	length_list[0x0252] = -1;

	// Packet: 0x0253
	length_list[0x0253] = 3;

	// Packet: 0x0254
	length_list[0x0254] = 3;

	// Packet: 0x0255
	length_list[0x0255] = 5;

	// Packet: 0x0256
	length_list[0x0256] = 5;

	// Packet: 0x0257
	length_list[0x0257] = 8;

	// Packet: 0x0258
	length_list[0x0258] = 2;

	// Packet: 0x0259
	length_list[0x0259] = 3;

	// Packet: 0x025a
	length_list[0x025a] = -1;

	// Packet: 0x025b
	length_list[0x025b] = 6;

	// Packet: 0x025c
	length_list[0x025c] = 4;

	// Packet: 0x025d
	length_list[0x025d] = 6;

	// Packet: 0x025e
	length_list[0x025e] = 4;

	// Packet: 0x025f
	length_list[0x025f] = 6;

	// Packet: 0x0260
	length_list[0x0260] = 6;

	// Packet: 0x0261
	length_list[0x0261] = 11;

	// Packet: 0x0262
	length_list[0x0262] = 11;

	// Packet: 0x0263
	length_list[0x0263] = 11;

	// Packet: 0x0264
	length_list[0x0264] = 20;

	// Packet: 0x0265
	length_list[0x0265] = 20;

	// Packet: 0x0266
	length_list[0x0266] = 30;

	// Packet: 0x0267
	length_list[0x0267] = 4;

	// Packet: 0x0268
	length_list[0x0268] = 4;

	// Packet: 0x0269
	length_list[0x0269] = 4;

	// Packet: 0x026a
	length_list[0x026a] = 4;

	// Packet: 0x026b
	length_list[0x026b] = 4;

	// Packet: 0x026c
	length_list[0x026c] = 4;

	// Packet: 0x026d
	length_list[0x026d] = 4;

	// Packet: 0x026f
	length_list[0x026f] = 2;

	// Packet: 0x0270
	length_list[0x0270] = 2;

	// Packet: 0x0271
	length_list[0x0271] = 40;

	// Packet: 0x0272
	length_list[0x0272] = 44;

	// Packet: 0x0273
	length_list[0x0273] = 30;

	// Packet: 0x0274
	if (packetver >= 20090406) {
		length_list[0x0274] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x0274] = 15;
	} else if (packetver >= 20090311) {
		length_list[0x0274] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0274] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0274] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0274] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0274] = -1;
	}

	// Packet: 0x0275
	if (packetver >= 20090701) {
		length_list[0x0275] = 37;
	}

	// Packet: 0x0276
	if (packetver >= 20090701) {
		length_list[0x0276] = -1;
	}

	// Packet: 0x0277
	length_list[0x0277] = 84;

	// Packet: 0x0278
	length_list[0x0278] = 2;

	// Packet: 0x0279
	length_list[0x0279] = 2;

	// Packet: 0x027a
	length_list[0x027a] = -1;

	// Packet: 0x027b
	length_list[0x027b] = 14;

	// Packet: 0x027c
	length_list[0x027c] = 60;

	// Packet: 0x027d
	length_list[0x027d] = 62;

	// Packet: 0x027e
	length_list[0x027e] = -1;

	// Packet: 0x027f
	length_list[0x027f] = 8;

	// Packet: 0x0280
	length_list[0x0280] = 12;

	// Packet: 0x0281
	length_list[0x0281] = 4;

	// Packet: 0x0282
	length_list[0x0282] = 284;

	// Packet: 0x0283
	length_list[0x0283] = 6;

	// Packet: 0x0284
	length_list[0x0284] = 14;

	// Packet: 0x0285
	length_list[0x0285] = 6;

	// Packet: 0x0286
	length_list[0x0286] = 4;

	// Packet: 0x0287
	if (packetver >= 20090406) {
		length_list[0x0287] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0287] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x0287] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0287] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x0287] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0287] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x0287] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0287] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x0287] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0287] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0287] = 29;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0288
	length_list[0x0288] = 10;

	// Packet: 0x0289
	length_list[0x0289] = 12;

	// Packet: 0x028a
	length_list[0x028a] = 18;

	// Packet: 0x028b
	length_list[0x028b] = -1;

	// Packet: 0x028c
	length_list[0x028c] = 46;

	// Packet: 0x028d
	length_list[0x028d] = 34;

	// Packet: 0x028e
	length_list[0x028e] = 4;

	// Packet: 0x028f
	length_list[0x028f] = 6;

	// Packet: 0x0290
	length_list[0x0290] = 4;

	// Packet: 0x0291
	length_list[0x0291] = 4;

	// Packet: 0x0292
	length_list[0x0292] = 2;

	// Packet: 0x0293
	length_list[0x0293] = 70;

	// Packet: 0x0294
	length_list[0x0294] = 10;

	// Packet: 0x0295
	if (packetver >= 20090318) {
		length_list[0x0295] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0295] = 81;
	} else if (packetver >= 20090225) {
		length_list[0x0295] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x0295] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0295] = -1;
	}

	// Packet: 0x0296
	if (packetver >= 20090406) {
		length_list[0x0296] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0296] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0296] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0296] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0296] = 36;
	} else if (packetver >= 20090204) {
		length_list[0x0296] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0296] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0296] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0296] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0296] = -1;
	}

	// Packet: 0x0297
	if (packetver >= 20090211) {
		length_list[0x0297] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0297] = 59;
	} else if (packetver >= 20090129) {
		length_list[0x0297] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0297] = 58;
	} else if (packetver >= 20090107) {
		length_list[0x0297] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0298
	length_list[0x0298] = 8;

	// Packet: 0x0299
	length_list[0x0299] = 6;

	// Packet: 0x029a
	if (packetver >= 20090406) {
		length_list[0x029a] = 27;
	} else if (packetver >= 20090401) {
		length_list[0x029a] = 9;
	} else if (packetver >= 20090325) {
		length_list[0x029a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x029a] = 15;
	} else if (packetver >= 20090311) {
		length_list[0x029a] = 32;
	} else if (packetver >= 20090218) {
		length_list[0x029a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x029a] = 102;
	} else if (packetver >= 20090204) {
		length_list[0x029a] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x029a] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x029a] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x029a] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x029b
	length_list[0x029b] = 80;

	// Packet: 0x029c
	length_list[0x029c] = 66;

	// Packet: 0x029d
	length_list[0x029d] = -1;

	// Packet: 0x029e
	length_list[0x029e] = 11;

	// Packet: 0x029f
	length_list[0x029f] = 3;

	// Packet: 0x02a2
	length_list[0x02a2] = 8;

	// Packet: 0x02a5
	length_list[0x02a5] = 8;

	// Packet: 0x02a6
	if (packetver >= 20090408) {
		length_list[0x02a6] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x02a6] = 404;
	}

	// Packet: 0x02a7
	if (packetver >= 20090408) {
		length_list[0x02a7] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x02a7] = 404;
	}

	// Packet: 0x02aa
	length_list[0x02aa] = 4;

	// Packet: 0x02ab
	length_list[0x02ab] = 36;

	// Packet: 0x02ac
	length_list[0x02ac] = 6;

	// Packet: 0x02ad
	length_list[0x02ad] = 8;

	// Packet: 0x02b0
	length_list[0x02b0] = 85;

	// Packet: 0x02b1
	length_list[0x02b1] = -1;

	// Packet: 0x02b2
	length_list[0x02b2] = -1;

	// Packet: 0x02b3
	length_list[0x02b3] = 107;

	// Packet: 0x02b4
	length_list[0x02b4] = 6;

	// Packet: 0x02b5
	length_list[0x02b5] = -1;

	// Packet: 0x02b6
	length_list[0x02b6] = 7;

	// Packet: 0x02b7
	length_list[0x02b7] = 7;

	// Packet: 0x02b8
	length_list[0x02b8] = 22;

	// Packet: 0x02b9
	length_list[0x02b9] = 191;

	// Packet: 0x02ba
	length_list[0x02ba] = 11;

	// Packet: 0x02bb
	length_list[0x02bb] = 8;

	// Packet: 0x02bc
	length_list[0x02bc] = 6;

	// Packet: 0x02c1
	length_list[0x02c1] = -1;

	// Packet: 0x02c2
	length_list[0x02c2] = -1;

	// Packet: 0x02c4
	length_list[0x02c4] = 26;

	// Packet: 0x02c5
	length_list[0x02c5] = 30;

	// Packet: 0x02c6
	length_list[0x02c6] = 30;

	// Packet: 0x02c7
	length_list[0x02c7] = 7;

	// Packet: 0x02c8
	length_list[0x02c8] = 3;

	// Packet: 0x02c9
	length_list[0x02c9] = 3;

	// Packet: 0x02ca
	length_list[0x02ca] = 3;

	// Packet: 0x02cb
	length_list[0x02cb] = 65;

	// Packet: 0x02cc
	length_list[0x02cc] = 4;

	// Packet: 0x02cd
	length_list[0x02cd] = 71;

	// Packet: 0x02ce
	length_list[0x02ce] = 10;

	// Packet: 0x02cf
	length_list[0x02cf] = 6;

	// Packet: 0x02d0
	length_list[0x02d0] = -1;

	// Packet: 0x02d1
	length_list[0x02d1] = -1;

	// Packet: 0x02d2
	length_list[0x02d2] = -1;

	// Packet: 0x02d3
	length_list[0x02d3] = 4;

	// Packet: 0x02d4
	length_list[0x02d4] = 29;

	// Packet: 0x02d5
	length_list[0x02d5] = 2;

	// Packet: 0x02d6
	length_list[0x02d6] = 6;

	// Packet: 0x02d7
	length_list[0x02d7] = -1;

	// Packet: 0x02d8
	length_list[0x02d8] = 10;

	// Packet: 0x02d9
	length_list[0x02d9] = 10;

	// Packet: 0x02da
	length_list[0x02da] = 3;

	// Packet: 0x02db
	length_list[0x02db] = -1;

	// Packet: 0x02dc
	length_list[0x02dc] = -1;

	// Packet: 0x02dd
	length_list[0x02dd] = 32;

	// Packet: 0x02de
	length_list[0x02de] = 6;

	// Packet: 0x02df
	length_list[0x02df] = 36;

	// Packet: 0x02e0
	length_list[0x02e0] = 34;

	// Packet: 0x02e1
	if (packetver >= 20090406) {
		length_list[0x02e1] = 33;
	} else if (packetver >= 20090325) {
		length_list[0x02e1] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x02e1] = 114;
	} else if (packetver >= 20090311) {
		length_list[0x02e1] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x02e1] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x02e1] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x02e1] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x02e1] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x02e1] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x02e1] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x02e1] = 60;
	} else if (packetver >= 20090107) {
		length_list[0x02e1] = 7;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x02e2
	length_list[0x02e2] = 20;

	// Packet: 0x02e3
	length_list[0x02e3] = 22;

	// Packet: 0x02e4
	length_list[0x02e4] = 11;

	// Packet: 0x02e5
	length_list[0x02e5] = 9;

	// Packet: 0x02e6
	length_list[0x02e6] = 6;

	// Packet: 0x02e7
	length_list[0x02e7] = -1;

	// Packet: 0x02e8
	length_list[0x02e8] = -1;

	// Packet: 0x02e9
	length_list[0x02e9] = -1;

	// Packet: 0x02ea
	length_list[0x02ea] = -1;

	// Packet: 0x02eb
	length_list[0x02eb] = 13;

	// Packet: 0x02ec
	if (packetver >= 20090406) {
		length_list[0x02ec] = 67;
	} else if (packetver >= 20090318) {
		length_list[0x02ec] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x02ec] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x02ec] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x02ec] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x02ec] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x02ec] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x02ec] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x02ec] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x02ec] = -1;
	}

	// Packet: 0x02ed
	if (packetver >= 20090406) {
		length_list[0x02ed] = 59;
	} else if (packetver >= 20090401) {
		length_list[0x02ed] = 28;
	} else if (packetver >= 20090325) {
		length_list[0x02ed] = 15;
	} else if (packetver >= 20090318) {
		length_list[0x02ed] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x02ed] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x02ed] = 29;
	} else if (packetver >= 20090211) {
		length_list[0x02ed] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x02ed] = 23;
	} else if (packetver >= 20090129) {
		length_list[0x02ed] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x02ed] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x02ed] = -1;
	}

	// Packet: 0x02ee
	if (packetver >= 20090406) {
		length_list[0x02ee] = 60;
	} else if (packetver >= 20090401) {
		length_list[0x02ee] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x02ee] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x02ee] = 5;
	} else if (packetver >= 20090311) {
		length_list[0x02ee] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x02ee] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x02ee] = 20;
	} else if (packetver >= 20090129) {
		length_list[0x02ee] = 13;
	} else if (packetver >= 20090120) {
		length_list[0x02ee] = 21;
	} else if (packetver >= 20090107) {
		length_list[0x02ee] = -1;
	}

	// Packet: 0x02ef
	length_list[0x02ef] = 8;

	// Packet: 0x02f0
	length_list[0x02f0] = 10;

	// Packet: 0x02f1
	length_list[0x02f1] = 2;

	// Packet: 0x02f2
	length_list[0x02f2] = 2;

	// Packet: 0x02f3
	if (packetver >= 20090406) {
		length_list[0x02f3] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x02f3] = 58;
	} else if (packetver >= 20090318) {
		length_list[0x02f3] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x02f3] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x02f3] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x02f3] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x02f3] = 27;
	} else if (packetver >= 20090129) {
		length_list[0x02f3] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x02f3] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x02f3] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x02f4
	if (packetver >= 20090401) {
		length_list[0x02f4] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x02f4] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x02f4] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x02f4] = 39;
	} else if (packetver >= 20090225) {
		length_list[0x02f4] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x02f4] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x02f4] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x02f4] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x02f4] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x02f4] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x02f4] = -1;
	}

	// Packet: 0x02f5
	if (packetver >= 20090325) {
		length_list[0x02f5] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x02f5] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x02f5] = 60;
	} else if (packetver >= 20090225) {
		length_list[0x02f5] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x02f5] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x02f5] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x02f5] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x02f5] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x02f6
	if (packetver >= 20090401) {
		length_list[0x02f6] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x02f6] = 26;
	} else if (packetver >= 20090318) {
		length_list[0x02f6] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x02f6] = 65;
	} else if (packetver >= 20090225) {
		length_list[0x02f6] = 19;
	} else if (packetver >= 20090218) {
		length_list[0x02f6] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x02f6] = 5;
	} else if (packetver >= 20090129) {
		length_list[0x02f6] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x02f6] = 18;
	} else if (packetver >= 20090114) {
		length_list[0x02f6] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x02f6] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x02f7
	if (packetver >= 20090406) {
		length_list[0x02f7] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x02f7] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x02f7] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x02f7] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x02f7] = 24;
	} else if (packetver >= 20090204) {
		length_list[0x02f7] = 282;
	} else if (packetver >= 20090114) {
		length_list[0x02f7] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x02f7] = 22;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x02f8
	if (packetver >= 20090406) {
		length_list[0x02f8] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x02f8] = 5;
	} else if (packetver >= 20090325) {
		length_list[0x02f8] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x02f8] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x02f8] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x02f8] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x02f8] = 15;
	} else if (packetver >= 20090129) {
		length_list[0x02f8] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x02f8] = 14;
	} else if (packetver >= 20090114) {
		length_list[0x02f8] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x02f8] = 5;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x02f9
	if (packetver >= 20090318) {
		length_list[0x02f9] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x02f9] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x02f9] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x02f9] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x02f9] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x02f9] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x02f9] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x02f9] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x02fa
	if (packetver >= 20090225) {
		length_list[0x02fa] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x02fa] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x02fa] = 14;
	} else if (packetver >= 20090129) {
		length_list[0x02fa] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x02fa] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x02fa] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x02fb
	if (packetver >= 20090311) {
		length_list[0x02fb] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x02fb] = 58;
	} else if (packetver >= 20090218) {
		length_list[0x02fb] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x02fb] = 42;
	} else if (packetver >= 20090204) {
		length_list[0x02fb] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x02fb] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x02fb] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x02fb] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x02fc
	if (packetver >= 20090325) {
		length_list[0x02fc] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x02fc] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x02fc] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x02fc] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x02fc] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x02fc] = 39;
	} else if (packetver >= 20090129) {
		length_list[0x02fc] = 54;
	} else if (packetver >= 20090107) {
		length_list[0x02fc] = -1;
	}

	// Packet: 0x02fd
	if (packetver >= 20090406) {
		length_list[0x02fd] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x02fd] = 5;
	} else if (packetver >= 20090325) {
		length_list[0x02fd] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x02fd] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x02fd] = 42;
	} else if (packetver >= 20090218) {
		length_list[0x02fd] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x02fd] = 182;
	} else if (packetver >= 20090129) {
		length_list[0x02fd] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x02fd] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x02fd] = 102;
	} else if (packetver >= 20090107) {
		length_list[0x02fd] = 11;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x02fe
	if (packetver >= 20090325) {
		length_list[0x02fe] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x02fe] = 5;
	} else if (packetver >= 20090311) {
		length_list[0x02fe] = 19;
	} else if (packetver >= 20090218) {
		length_list[0x02fe] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x02fe] = 9;
	} else if (packetver >= 20090204) {
		length_list[0x02fe] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x02fe] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x02ff
	if (packetver >= 20090406) {
		length_list[0x02ff] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x02ff] = 27;
	} else if (packetver >= 20090325) {
		length_list[0x02ff] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x02ff] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x02ff] = 54;
	} else if (packetver >= 20090204) {
		length_list[0x02ff] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x02ff] = 9;
	} else if (packetver >= 20090120) {
		length_list[0x02ff] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x02ff] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x02ff] = -1;
	}

	// Packet: 0x0300
	if (packetver >= 20090318) {
		length_list[0x0300] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0300] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x0300] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0300] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0300] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0300] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0300] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0301
	if (packetver >= 20090406) {
		length_list[0x0301] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0301] = 22;
	} else if (packetver >= 20090325) {
		length_list[0x0301] = 36;
	} else if (packetver >= 20090218) {
		length_list[0x0301] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0301] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x0301] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x0301] = 28;
	} else if (packetver >= 20090120) {
		length_list[0x0301] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0301] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0301] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0302
	if (packetver >= 20090325) {
		length_list[0x0302] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0302] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0302] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0302] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x0302] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0302] = 81;
	} else if (packetver >= 20090114) {
		length_list[0x0302] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0302] = 55;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0303
	if (packetver >= 20090318) {
		length_list[0x0303] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0303] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x0303] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0303] = 53;
	} else if (packetver >= 20090211) {
		length_list[0x0303] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0303] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0303] = 39;
	} else if (packetver >= 20090120) {
		length_list[0x0303] = 26;
	} else if (packetver >= 20090114) {
		length_list[0x0303] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0303] = -1;
	}

	// Packet: 0x0304
	if (packetver >= 20090406) {
		length_list[0x0304] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0304] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x0304] = 29;
	} else if (packetver >= 20090318) {
		length_list[0x0304] = 19;
	} else if (packetver >= 20090225) {
		length_list[0x0304] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0304] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x0304] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0304] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0304] = 26;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0305
	if (packetver >= 20090318) {
		length_list[0x0305] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0305] = 16;
	} else if (packetver >= 20090120) {
		length_list[0x0305] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0305] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0305] = 282;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0306
	if (packetver >= 20090406) {
		length_list[0x0306] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0306] = 12;
	} else if (packetver >= 20090318) {
		length_list[0x0306] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0306] = 282;
	} else if (packetver >= 20090225) {
		length_list[0x0306] = 24;
	} else if (packetver >= 20090218) {
		length_list[0x0306] = 33;
	} else if (packetver >= 20090120) {
		length_list[0x0306] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0306] = 42;
	} else if (packetver >= 20090107) {
		length_list[0x0306] = 33;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0307
	if (packetver >= 20090225) {
		length_list[0x0307] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0307] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0307] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0307] = -1;
	}

	// Packet: 0x0308
	if (packetver >= 20090406) {
		length_list[0x0308] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0308] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x0308] = 16;
	} else if (packetver >= 20090311) {
		length_list[0x0308] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0308] = 16;
	} else if (packetver >= 20090218) {
		length_list[0x0308] = 53;
	} else if (packetver >= 20090211) {
		length_list[0x0308] = 15;
	} else if (packetver >= 20090204) {
		length_list[0x0308] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0308] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0308] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0309
	if (packetver >= 20090406) {
		length_list[0x0309] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0309] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x0309] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0309] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x0309] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0309] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0309] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x0309] = 32;
	} else if (packetver >= 20090107) {
		length_list[0x0309] = 14;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x030a
	if (packetver >= 20090325) {
		length_list[0x030a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x030a] = 9;
	} else if (packetver >= 20090311) {
		length_list[0x030a] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x030a] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x030a] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x030a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x030a] = 182;
	} else if (packetver >= 20090129) {
		length_list[0x030a] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x030a] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x030a] = 32;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x030b
	if (packetver >= 20090325) {
		length_list[0x030b] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x030b] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x030b] = 9;
	} else if (packetver >= 20090225) {
		length_list[0x030b] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x030b] = 43;
	} else if (packetver >= 20090211) {
		length_list[0x030b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x030b] = 282;
	} else if (packetver >= 20090129) {
		length_list[0x030b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x030b] = 19;
	} else if (packetver >= 20090114) {
		length_list[0x030b] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x030b] = 4;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x030c
	if (packetver >= 20090325) {
		length_list[0x030c] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x030c] = 5;
	} else if (packetver >= 20090225) {
		length_list[0x030c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x030c] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x030c] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x030c] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x030c] = -1;
	}

	// Packet: 0x030d
	if (packetver >= 20090406) {
		length_list[0x030d] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x030d] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x030d] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x030d] = 67;
	} else if (packetver >= 20090218) {
		length_list[0x030d] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x030d] = 27;
	} else if (packetver >= 20090129) {
		length_list[0x030d] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x030d] = 33;
	} else if (packetver >= 20090114) {
		length_list[0x030d] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x030d] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x030e
	if (packetver >= 20090406) {
		length_list[0x030e] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x030e] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x030e] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x030e] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x030e] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x030e] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x030e] = 65;
	} else if (packetver >= 20090107) {
		length_list[0x030e] = 5;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x030f
	if (packetver >= 20090401) {
		length_list[0x030f] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x030f] = 57;
	} else if (packetver >= 20090225) {
		length_list[0x030f] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x030f] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x030f] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x030f] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x030f] = 26;
	} else if (packetver >= 20090114) {
		length_list[0x030f] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x030f] = 53;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0310
	if (packetver >= 20090218) {
		length_list[0x0310] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0310] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0310] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x0310] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x0310] = 15;
	} else if (packetver >= 20090114) {
		length_list[0x0310] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0310] = -1;
	}

	// Packet: 0x0311
	if (packetver >= 20090406) {
		length_list[0x0311] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0311] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x0311] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x0311] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0311] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0311] = 27;
	} else if (packetver >= 20090114) {
		length_list[0x0311] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0311] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0312
	if (packetver >= 20090325) {
		length_list[0x0312] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0312] = 5;
	} else if (packetver >= 20090311) {
		length_list[0x0312] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0312] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0312] = 5;
	} else if (packetver >= 20090211) {
		length_list[0x0312] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x0312] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0312] = 97;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0313
	if (packetver >= 20090325) {
		length_list[0x0313] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0313] = 5;
	} else if (packetver >= 20090311) {
		length_list[0x0313] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0313] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0313] = 37;
	} else if (packetver >= 20090129) {
		length_list[0x0313] = 11;
	} else if (packetver >= 20090120) {
		length_list[0x0313] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0313] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0313] = -1;
	}

	// Packet: 0x0314
	if (packetver >= 20090406) {
		length_list[0x0314] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0314] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0314] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0314] = 60;
	} else if (packetver >= 20090129) {
		length_list[0x0314] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0314] = 102;
	} else if (packetver >= 20090114) {
		length_list[0x0314] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0314] = 11;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0315
	if (packetver >= 20090401) {
		length_list[0x0315] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0315] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x0315] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0315] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0315] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x0315] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0315] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0315] = 20;
	} else if (packetver >= 20090114) {
		length_list[0x0315] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0315] = 30;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0316
	if (packetver >= 20090406) {
		length_list[0x0316] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0316] = 13;
	} else if (packetver >= 20090325) {
		length_list[0x0316] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x0316] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0316] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0316] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x0316] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0316] = 11;
	} else if (packetver >= 20090120) {
		length_list[0x0316] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0316] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0316] = 13;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0317
	if (packetver >= 20090406) {
		length_list[0x0317] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0317] = 16;
	} else if (packetver >= 20090318) {
		length_list[0x0317] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0317] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0317] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0317] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x0317] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0317] = 21;
	} else if (packetver >= 20090120) {
		length_list[0x0317] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0317] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x0317] = -1;
	}

	// Packet: 0x0318
	if (packetver >= 20090401) {
		length_list[0x0318] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0318] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0318] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0318] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0318] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0318] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0318] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0318] = 59;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0319
	if (packetver >= 20090401) {
		length_list[0x0319] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0319] = 33;
	} else if (packetver >= 20090318) {
		length_list[0x0319] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0319] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0319] = 60;
	} else if (packetver >= 20090218) {
		length_list[0x0319] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x0319] = -1;
	}

	// Packet: 0x031a
	if (packetver >= 20090406) {
		length_list[0x031a] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x031a] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x031a] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x031a] = 33;
	} else if (packetver >= 20090225) {
		length_list[0x031a] = 282;
	} else if (packetver >= 20090218) {
		length_list[0x031a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x031a] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x031a] = 7;
	} else if (packetver >= 20090120) {
		length_list[0x031a] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x031a] = 23;
	} else if (packetver >= 20090107) {
		length_list[0x031a] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x031b
	if (packetver >= 20090218) {
		length_list[0x031b] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x031b] = 19;
	} else if (packetver >= 20090129) {
		length_list[0x031b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x031b] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x031b] = 282;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x031c
	if (packetver >= 20090401) {
		length_list[0x031c] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x031c] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x031c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x031c] = 20;
	} else if (packetver >= 20090211) {
		length_list[0x031c] = 186;
	} else if (packetver >= 20090204) {
		length_list[0x031c] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x031c] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x031c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x031c] = 30;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x031d
	if (packetver >= 20090401) {
		length_list[0x031d] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x031d] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x031d] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x031d] = 28;
	} else if (packetver >= 20090204) {
		length_list[0x031d] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x031d] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x031d] = -1;
	}

	// Packet: 0x031e
	if (packetver >= 20090311) {
		length_list[0x031e] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x031e] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x031e] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x031e] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x031e] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x031e] = 54;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x031f
	if (packetver >= 20090401) {
		length_list[0x031f] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x031f] = 114;
	} else if (packetver >= 20090318) {
		length_list[0x031f] = 17;
	} else if (packetver >= 20090311) {
		length_list[0x031f] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x031f] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x031f] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x031f] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x031f] = 71;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0320
	if (packetver >= 20090204) {
		length_list[0x0320] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0320] = 12;
	} else if (packetver >= 20090120) {
		length_list[0x0320] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0320] = 16;
	} else if (packetver >= 20090107) {
		length_list[0x0320] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0321
	if (packetver >= 20090401) {
		length_list[0x0321] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0321] = 22;
	} else if (packetver >= 20090218) {
		length_list[0x0321] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0321] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0321] = 186;
	} else if (packetver >= 20090107) {
		length_list[0x0321] = -1;
	}

	// Packet: 0x0322
	if (packetver >= 20090401) {
		length_list[0x0322] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0322] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0322] = 9;
	} else if (packetver >= 20090218) {
		length_list[0x0322] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0322] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x0322] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0322] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x0322] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x0322] = 27;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0323
	if (packetver >= 20090406) {
		length_list[0x0323] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0323] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0323] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0323] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x0323] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0323] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x0323] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0323] = 12;
	} else if (packetver >= 20090120) {
		length_list[0x0323] = 37;
	} else if (packetver >= 20090114) {
		length_list[0x0323] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x0323] = 7;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0324
	if (packetver >= 20090406) {
		length_list[0x0324] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0324] = 5;
	} else if (packetver >= 20090325) {
		length_list[0x0324] = 33;
	} else if (packetver >= 20090311) {
		length_list[0x0324] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0324] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0324] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0324] = 12;
	} else if (packetver >= 20090204) {
		length_list[0x0324] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0324] = 97;
	} else if (packetver >= 20090120) {
		length_list[0x0324] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x0324] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0324] = 10;
	}

	// Packet: 0x0325
	if (packetver >= 20090311) {
		length_list[0x0325] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0325] = 27;
	} else if (packetver >= 20090129) {
		length_list[0x0325] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0325] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0325] = 32;
	} else if (packetver >= 20090107) {
		length_list[0x0325] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0326
	if (packetver >= 20090401) {
		length_list[0x0326] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0326] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0326] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0326] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0326] = 17;
	} else if (packetver >= 20090204) {
		length_list[0x0326] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0326] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0326] = 14;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0327
	if (packetver >= 20090406) {
		length_list[0x0327] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0327] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0327] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0327] = 28;
	} else if (packetver >= 20090211) {
		length_list[0x0327] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0327] = 9;
	} else if (packetver >= 20090129) {
		length_list[0x0327] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0327] = 13;
	} else if (packetver >= 20090107) {
		length_list[0x0327] = -1;
	}

	// Packet: 0x0328
	if (packetver >= 20090318) {
		length_list[0x0328] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0328] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x0328] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0328] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0328] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0328] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x0328] = 14;
	} else if (packetver >= 20090120) {
		length_list[0x0328] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x0328] = 26;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0329
	if (packetver >= 20090406) {
		length_list[0x0329] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0329] = 60;
	} else if (packetver >= 20090318) {
		length_list[0x0329] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0329] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0329] = 54;
	} else if (packetver >= 20090218) {
		length_list[0x0329] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0329] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x0329] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x0329] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0329] = 114;
	} else if (packetver >= 20090107) {
		length_list[0x0329] = -1;
	}

	// Packet: 0x032a
	if (packetver >= 20090318) {
		length_list[0x032a] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x032a] = 66;
	} else if (packetver >= 20090204) {
		length_list[0x032a] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x032a] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x032a] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x032a] = 7;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x032b
	if (packetver >= 20090406) {
		length_list[0x032b] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x032b] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x032b] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x032b] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x032b] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x032b] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x032b] = 17;
	} else if (packetver >= 20090211) {
		length_list[0x032b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x032b] = 55;
	} else if (packetver >= 20090107) {
		length_list[0x032b] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x032c
	if (packetver >= 20090401) {
		length_list[0x032c] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x032c] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x032c] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x032c] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x032c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x032c] = 58;
	} else if (packetver >= 20090211) {
		length_list[0x032c] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x032c] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x032c] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x032c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x032c] = 19;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x032d
	if (packetver >= 20090401) {
		length_list[0x032d] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x032d] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x032d] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x032d] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x032d] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x032e
	if (packetver >= 20090406) {
		length_list[0x032e] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x032e] = 68;
	} else if (packetver >= 20090318) {
		length_list[0x032e] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x032e] = 17;
	} else if (packetver >= 20090225) {
		length_list[0x032e] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x032e] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x032e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x032e] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x032e] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x032e] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x032f
	if (packetver >= 20090218) {
		length_list[0x032f] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x032f] = 90;
	} else if (packetver >= 20090204) {
		length_list[0x032f] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x032f] = -1;
	}

	// Packet: 0x0330
	if (packetver >= 20090318) {
		length_list[0x0330] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0330] = 7;
	} else if (packetver >= 20090218) {
		length_list[0x0330] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0330] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0330] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0330] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0330] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0330] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0330] = -1;
	}

	// Packet: 0x0331
	if (packetver >= 20090318) {
		length_list[0x0331] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0331] = 5;
	} else if (packetver >= 20090211) {
		length_list[0x0331] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0331] = 71;
	} else if (packetver >= 20090129) {
		length_list[0x0331] = 29;
	} else if (packetver >= 20090107) {
		length_list[0x0331] = -1;
	}

	// Packet: 0x0332
	if (packetver >= 20090401) {
		length_list[0x0332] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0332] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x0332] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0332] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x0332] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0332] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0332] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0333
	if (packetver >= 20090406) {
		length_list[0x0333] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0333] = 282;
	} else if (packetver >= 20090325) {
		length_list[0x0333] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0333] = 102;
	} else if (packetver >= 20090311) {
		length_list[0x0333] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0333] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0333] = 14;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0334
	if (packetver >= 20090406) {
		length_list[0x0334] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0334] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x0334] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0334] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x0334] = 97;
	} else if (packetver >= 20090225) {
		length_list[0x0334] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x0334] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0334] = 31;
	} else if (packetver >= 20090120) {
		length_list[0x0334] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0334] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0334] = 27;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0335
	if (packetver >= 20090401) {
		length_list[0x0335] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0335] = 11;
	} else if (packetver >= 20090318) {
		length_list[0x0335] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0335] = 71;
	} else if (packetver >= 20090225) {
		length_list[0x0335] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0335] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0335] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0335] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0335] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0335] = -1;
	}

	// Packet: 0x0336
	if (packetver >= 20090325) {
		length_list[0x0336] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0336] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x0336] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0336] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x0336] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0336] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x0336] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0336] = 65;
	} else if (packetver >= 20090120) {
		length_list[0x0336] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0336] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x0336] = 4;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0337
	if (packetver >= 20090401) {
		length_list[0x0337] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0337] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x0337] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0337] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x0337] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0337] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x0337] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0337] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x0337] = 27;
	} else if (packetver >= 20090107) {
		length_list[0x0337] = 6;
	}

	// Packet: 0x0338
	if (packetver >= 20090211) {
		length_list[0x0338] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0338] = 12;
	} else if (packetver >= 20090114) {
		length_list[0x0338] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0338] = 4;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0339
	if (packetver >= 20090311) {
		length_list[0x0339] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0339] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x0339] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x0339] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0339] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0339] = -1;
	}

	// Packet: 0x033a
	if (packetver >= 20090401) {
		length_list[0x033a] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x033a] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x033a] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x033a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x033a] = 39;
	} else if (packetver >= 20090204) {
		length_list[0x033a] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x033a] = 114;
	} else if (packetver >= 20090120) {
		length_list[0x033a] = 16;
	} else if (packetver >= 20090107) {
		length_list[0x033a] = -1;
	}

	// Packet: 0x033b
	if (packetver >= 20090225) {
		length_list[0x033b] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x033b] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x033b] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x033b] = 17;
	} else if (packetver >= 20090120) {
		length_list[0x033b] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x033b] = 97;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x033c
	if (packetver >= 20090401) {
		length_list[0x033c] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x033c] = 27;
	} else if (packetver >= 20090311) {
		length_list[0x033c] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x033c] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x033c] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x033c] = 282;
	} else if (packetver >= 20090129) {
		length_list[0x033c] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x033c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x033c] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x033d
	if (packetver >= 20090406) {
		length_list[0x033d] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x033d] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x033d] = 11;
	} else if (packetver >= 20090318) {
		length_list[0x033d] = 39;
	} else if (packetver >= 20090311) {
		length_list[0x033d] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x033d] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x033d] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x033d] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x033d] = 43;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x033e
	if (packetver >= 20090225) {
		length_list[0x033e] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x033e] = 114;
	} else if (packetver >= 20090204) {
		length_list[0x033e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x033e] = 11;
	} else if (packetver >= 20090120) {
		length_list[0x033e] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x033e] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x033e] = 4;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x033f
	if (packetver >= 20090406) {
		length_list[0x033f] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x033f] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x033f] = 42;
	} else if (packetver >= 20090311) {
		length_list[0x033f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x033f] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x033f] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x033f] = 28;
	} else if (packetver >= 20090114) {
		length_list[0x033f] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x033f] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0340
	if (packetver >= 20090406) {
		length_list[0x0340] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0340] = 71;
	} else if (packetver >= 20090325) {
		length_list[0x0340] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x0340] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0340] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x0340] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0340] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x0340] = 68;
	} else if (packetver >= 20090107) {
		length_list[0x0340] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0341
	if (packetver >= 20090406) {
		length_list[0x0341] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0341] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0341] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0341] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x0341] = 39;
	} else if (packetver >= 20090218) {
		length_list[0x0341] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x0341] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0341] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x0341] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0341] = 5;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0342
	if (packetver >= 20090401) {
		length_list[0x0342] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0342] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x0342] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0342] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0342] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0342] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0342] = 36;
	} else if (packetver >= 20090120) {
		length_list[0x0342] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0342] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0342] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0343
	if (packetver >= 20090325) {
		length_list[0x0343] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0343] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x0343] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0343] = 32;
	} else if (packetver >= 20090218) {
		length_list[0x0343] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0343] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0343] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0343] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x0343] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x0343] = 27;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0344
	if (packetver >= 20090401) {
		length_list[0x0344] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0344] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x0344] = 14;
	} else if (packetver >= 20090311) {
		length_list[0x0344] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x0344] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0344] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0344] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x0344] = -1;
	}

	// Packet: 0x0345
	if (packetver >= 20090401) {
		length_list[0x0345] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0345] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0345] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0345] = 19;
	} else if (packetver >= 20090107) {
		length_list[0x0345] = -1;
	}

	// Packet: 0x0346
	if (packetver >= 20090406) {
		length_list[0x0346] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0346] = 13;
	} else if (packetver >= 20090325) {
		length_list[0x0346] = 59;
	} else if (packetver >= 20090318) {
		length_list[0x0346] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0346] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0346] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x0346] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0346] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0346] = 22;
	} else if (packetver >= 20090114) {
		length_list[0x0346] = 19;
	} else if (packetver >= 20090107) {
		length_list[0x0346] = 13;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0347
	if (packetver >= 20090401) {
		length_list[0x0347] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0347] = 11;
	} else if (packetver >= 20090318) {
		length_list[0x0347] = 15;
	} else if (packetver >= 20090311) {
		length_list[0x0347] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x0347] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x0347] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0347] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0347] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0347] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x0347] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0347] = 23;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0348
	if (packetver >= 20090406) {
		length_list[0x0348] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0348] = 7;
	} else if (packetver >= 20090325) {
		length_list[0x0348] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x0348] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0348] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0348] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0348] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0348] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0348] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0349
	if (packetver >= 20090406) {
		length_list[0x0349] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0349] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x0349] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0349] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0349] = 31;
	} else if (packetver >= 20090129) {
		length_list[0x0349] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0349] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0349] = 22;
	} else if (packetver >= 20090107) {
		length_list[0x0349] = 11;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x034a
	if (packetver >= 20090406) {
		length_list[0x034a] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x034a] = 9;
	} else if (packetver >= 20090325) {
		length_list[0x034a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x034a] = 31;
	} else if (packetver >= 20090311) {
		length_list[0x034a] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x034a] = 22;
	} else if (packetver >= 20090218) {
		length_list[0x034a] = 19;
	} else if (packetver >= 20090211) {
		length_list[0x034a] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x034a] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x034a] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x034a] = 36;
	} else if (packetver >= 20090107) {
		length_list[0x034a] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x034b
	if (packetver >= 20090225) {
		length_list[0x034b] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x034b] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x034b] = 23;
	} else if (packetver >= 20090204) {
		length_list[0x034b] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x034b] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x034b] = 54;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x034c
	if (packetver >= 20090225) {
		length_list[0x034c] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x034c] = 54;
	} else if (packetver >= 20090211) {
		length_list[0x034c] = 28;
	} else if (packetver >= 20090204) {
		length_list[0x034c] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x034c] = 282;
	} else if (packetver >= 20090120) {
		length_list[0x034c] = 65;
	} else if (packetver >= 20090114) {
		length_list[0x034c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x034c] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x034d
	if (packetver >= 20090406) {
		length_list[0x034d] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x034d] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x034d] = 5;
	} else if (packetver >= 20090318) {
		length_list[0x034d] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x034d] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x034d] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x034d] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x034d] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x034d] = 23;
	} else if (packetver >= 20090114) {
		length_list[0x034d] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x034d] = 60;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x034e
	if (packetver >= 20090401) {
		length_list[0x034e] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x034e] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x034e] = 7;
	} else if (packetver >= 20090218) {
		length_list[0x034e] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x034e] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x034e] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x034e] = 31;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x034f
	if (packetver >= 20090406) {
		length_list[0x034f] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x034f] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x034f] = 19;
	} else if (packetver >= 20090311) {
		length_list[0x034f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x034f] = 66;
	} else if (packetver >= 20090218) {
		length_list[0x034f] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x034f] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x034f] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x034f] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x034f] = 27;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0350
	if (packetver >= 20090406) {
		length_list[0x0350] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0350] = 14;
	} else if (packetver >= 20090325) {
		length_list[0x0350] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x0350] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0350] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x0350] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0350] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x0350] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0350] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0350] = 28;
	} else if (packetver >= 20090107) {
		length_list[0x0350] = -1;
	}

	// Packet: 0x0351
	if (packetver >= 20090401) {
		length_list[0x0351] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0351] = 12;
	} else if (packetver >= 20090311) {
		length_list[0x0351] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x0351] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0351] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0351] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0352
	if (packetver >= 20090406) {
		length_list[0x0352] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0352] = 14;
	} else if (packetver >= 20090325) {
		length_list[0x0352] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x0352] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x0352] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0352] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0352] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x0352] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0352] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0352] = 14;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0353
	if (packetver >= 20090406) {
		length_list[0x0353] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0353] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x0353] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0353] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0353] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x0353] = 54;
	} else if (packetver >= 20090218) {
		length_list[0x0353] = 39;
	} else if (packetver >= 20090120) {
		length_list[0x0353] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0353] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0353] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0354
	if (packetver >= 20090311) {
		length_list[0x0354] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0354] = 23;
	} else if (packetver >= 20090218) {
		length_list[0x0354] = 16;
	} else if (packetver >= 20090211) {
		length_list[0x0354] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0354] = 97;
	} else if (packetver >= 20090129) {
		length_list[0x0354] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0354] = 14;
	} else if (packetver >= 20090114) {
		length_list[0x0354] = 29;
	} else if (packetver >= 20090107) {
		length_list[0x0354] = 20;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0355
	if (packetver >= 20090401) {
		length_list[0x0355] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0355] = 97;
	} else if (packetver >= 20090318) {
		length_list[0x0355] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0355] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0355] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0355] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0355] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0355] = 16;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0356
	if (packetver >= 20090406) {
		length_list[0x0356] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0356] = 28;
	} else if (packetver >= 20090325) {
		length_list[0x0356] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0356] = 37;
	} else if (packetver >= 20090311) {
		length_list[0x0356] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0356] = 81;
	} else if (packetver >= 20090211) {
		length_list[0x0356] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0356] = 5;
	} else if (packetver >= 20090129) {
		length_list[0x0356] = 11;
	} else if (packetver >= 20090120) {
		length_list[0x0356] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0356] = 28;
	} else if (packetver >= 20090107) {
		length_list[0x0356] = -1;
	}

	// Packet: 0x0357
	if (packetver >= 20090406) {
		length_list[0x0357] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0357] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x0357] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0357] = 20;
	} else if (packetver >= 20090225) {
		length_list[0x0357] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0357] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x0357] = 13;
	} else if (packetver >= 20090204) {
		length_list[0x0357] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0357] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x0357] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0357] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0357] = -1;
	}

	// Packet: 0x0358
	if (packetver >= 20090325) {
		length_list[0x0358] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0358] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0358] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x0358] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0358] = 32;
	} else if (packetver >= 20090120) {
		length_list[0x0358] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0358] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x0358] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0359
	if (packetver >= 20090218) {
		length_list[0x0359] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0359] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x0359] = 29;
	} else if (packetver >= 20090129) {
		length_list[0x0359] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0359] = -1;
	}

	// Packet: 0x035a
	if (packetver >= 20090406) {
		length_list[0x035a] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x035a] = 11;
	} else if (packetver >= 20090311) {
		length_list[0x035a] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x035a] = 29;
	} else if (packetver >= 20090218) {
		length_list[0x035a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x035a] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x035a] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x035a] = 12;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x035b
	if (packetver >= 20090325) {
		length_list[0x035b] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x035b] = 26;
	} else if (packetver >= 20090311) {
		length_list[0x035b] = 14;
	} else if (packetver >= 20090120) {
		length_list[0x035b] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x035b] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x035b] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x035c
	length_list[0x035c] = 2;

	// Packet: 0x035d
	length_list[0x035d] = -1;

	// Packet: 0x035e
	length_list[0x035e] = 2;

	// Packet: 0x035f
	if (packetver >= 20090406) {
		length_list[0x035f] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x035f] = 11;
	} else if (packetver >= 20090325) {
		length_list[0x035f] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x035f] = 67;
	} else if (packetver >= 20090311) {
		length_list[0x035f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x035f] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x035f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x035f] = -1;
	}

	// Packet: 0x0360
	if (packetver >= 20090401) {
		length_list[0x0360] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0360] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0360] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0360] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x0360] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0360] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x0360] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0360] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x0360] = 29;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0361
	if (packetver >= 20090406) {
		length_list[0x0361] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0361] = 8;
	} else if (packetver >= 20090325) {
		length_list[0x0361] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0361] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0361] = 22;
	} else if (packetver >= 20090211) {
		length_list[0x0361] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0361] = 86;
	} else if (packetver >= 20090120) {
		length_list[0x0361] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0361] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0361] = -1;
	}

	// Packet: 0x0362
	if (packetver >= 20090318) {
		length_list[0x0362] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0362] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x0362] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x0362] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0362] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x0362] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0362] = 15;
	} else if (packetver >= 20090107) {
		length_list[0x0362] = -1;
	}

	// Packet: 0x0363
	if (packetver >= 20090401) {
		length_list[0x0363] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0363] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0363] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0363] = 13;
	} else if (packetver >= 20090211) {
		length_list[0x0363] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0363] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x0363] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0363] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0363] = 34;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0364
	if (packetver >= 20090406) {
		length_list[0x0364] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0364] = 30;
	} else if (packetver >= 20090325) {
		length_list[0x0364] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0364] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x0364] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0364] = 30;
	} else if (packetver >= 20090218) {
		length_list[0x0364] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0364] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0364] = 7;
	} else if (packetver >= 20090120) {
		length_list[0x0364] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x0364] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x0364] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0365
	if (packetver >= 20090318) {
		length_list[0x0365] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0365] = 60;
	} else if (packetver >= 20090211) {
		length_list[0x0365] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0365] = 54;
	} else if (packetver >= 20090120) {
		length_list[0x0365] = 67;
	} else if (packetver >= 20090107) {
		length_list[0x0365] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0366
	if (packetver >= 20090406) {
		length_list[0x0366] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0366] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x0366] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0366] = 16;
	} else if (packetver >= 20090107) {
		length_list[0x0366] = -1;
	}

	// Packet: 0x0367
	if (packetver >= 20090406) {
		length_list[0x0367] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0367] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x0367] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0367] = 30;
	} else if (packetver >= 20090218) {
		length_list[0x0367] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0367] = 15;
	} else if (packetver >= 20090204) {
		length_list[0x0367] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0367] = 29;
	} else if (packetver >= 20090107) {
		length_list[0x0367] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0368
	if (packetver >= 20090204) {
		length_list[0x0368] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0368] = 71;
	} else if (packetver >= 20090114) {
		length_list[0x0368] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x0368] = 53;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0369
	if (packetver >= 20090318) {
		length_list[0x0369] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0369] = 5;
	} else if (packetver >= 20090225) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0369] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0369] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0369] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0369] = 19;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x036a
	if (packetver >= 20090311) {
		length_list[0x036a] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x036a] = 44;
	} else if (packetver >= 20090218) {
		length_list[0x036a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x036a] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x036a] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x036a] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x036a] = 24;
	} else if (packetver >= 20090114) {
		length_list[0x036a] = 282;
	} else if (packetver >= 20090107) {
		length_list[0x036a] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x036b
	if (packetver >= 20090311) {
		length_list[0x036b] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x036b] = 9;
	} else if (packetver >= 20090218) {
		length_list[0x036b] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x036b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x036b] = 19;
	} else if (packetver >= 20090120) {
		length_list[0x036b] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x036b] = 53;
	} else if (packetver >= 20090107) {
		length_list[0x036b] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x036c
	if (packetver >= 20090401) {
		length_list[0x036c] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x036c] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x036c] = 186;
	} else if (packetver >= 20090311) {
		length_list[0x036c] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x036c] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x036c] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x036c] = 26;
	} else if (packetver >= 20090114) {
		length_list[0x036c] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x036c] = 5;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x036d
	if (packetver >= 20090406) {
		length_list[0x036d] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x036d] = 5;
	} else if (packetver >= 20090311) {
		length_list[0x036d] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x036d] = 18;
	} else if (packetver >= 20090218) {
		length_list[0x036d] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x036d] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x036d] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x036d] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x036e
	if (packetver >= 20090406) {
		length_list[0x036e] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x036e] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x036e] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x036e] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x036e] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x036e] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x036e] = -1;
	}

	// Packet: 0x036f
	if (packetver >= 20090325) {
		length_list[0x036f] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x036f] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x036f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x036f] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x036f] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x036f] = 29;
	} else if (packetver >= 20090204) {
		length_list[0x036f] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x036f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x036f] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x036f] = 28;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0370
	if (packetver >= 20090406) {
		length_list[0x0370] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0370] = 60;
	} else if (packetver >= 20090218) {
		length_list[0x0370] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0370] = 9;
	} else if (packetver >= 20090204) {
		length_list[0x0370] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0370] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0370] = 28;
	} else if (packetver >= 20090107) {
		length_list[0x0370] = -1;
	}

	// Packet: 0x0371
	if (packetver >= 20090406) {
		length_list[0x0371] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0371] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x0371] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0371] = 26;
	} else if (packetver >= 20090225) {
		length_list[0x0371] = 22;
	} else if (packetver >= 20090218) {
		length_list[0x0371] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0371] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x0371] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0371] = 86;
	} else if (packetver >= 20090107) {
		length_list[0x0371] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0372
	if (packetver >= 20090318) {
		length_list[0x0372] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0372] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0372] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0372] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x0372] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0372] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x0372] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0372] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x0372] = -1;
	}

	// Packet: 0x0373
	if (packetver >= 20090406) {
		length_list[0x0373] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0373] = 7;
	} else if (packetver >= 20090325) {
		length_list[0x0373] = 14;
	} else if (packetver >= 20090318) {
		length_list[0x0373] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0373] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x0373] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0373] = 67;
	} else if (packetver >= 20090204) {
		length_list[0x0373] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0373] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0373] = -1;
	}

	// Packet: 0x0374
	if (packetver >= 20090406) {
		length_list[0x0374] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0374] = 29;
	} else if (packetver >= 20090325) {
		length_list[0x0374] = 68;
	} else if (packetver >= 20090318) {
		length_list[0x0374] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0374] = 13;
	} else if (packetver >= 20090225) {
		length_list[0x0374] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x0374] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0374] = 60;
	} else if (packetver >= 20090204) {
		length_list[0x0374] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0374] = 19;
	} else if (packetver >= 20090120) {
		length_list[0x0374] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x0374] = 36;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0375
	if (packetver >= 20090129) {
		length_list[0x0375] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0375] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x0375] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0375] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0376
	if (packetver >= 20090406) {
		length_list[0x0376] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0376] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x0376] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0376] = 60;
	} else if (packetver >= 20090204) {
		length_list[0x0376] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0376] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x0376] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0376] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x0376] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0377
	if (packetver >= 20090406) {
		length_list[0x0377] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0377] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x0377] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0377] = 12;
	} else if (packetver >= 20090107) {
		length_list[0x0377] = -1;
	}

	// Packet: 0x0378
	if (packetver >= 20090325) {
		length_list[0x0378] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0378] = 14;
	} else if (packetver >= 20090311) {
		length_list[0x0378] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0378] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x0378] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0378] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0378] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0379
	if (packetver >= 20090406) {
		length_list[0x0379] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0379] = 26;
	} else if (packetver >= 20090318) {
		length_list[0x0379] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0379] = 22;
	} else if (packetver >= 20090225) {
		length_list[0x0379] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x0379] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0379] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0379] = 9;
	} else if (packetver >= 20090120) {
		length_list[0x0379] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0379] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0379] = -1;
	}

	// Packet: 0x037a
	if (packetver >= 20090401) {
		length_list[0x037a] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x037a] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x037a] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x037a] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x037a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x037a] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x037a] = 19;
	} else if (packetver >= 20090107) {
		length_list[0x037a] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x037b
	if (packetver >= 20090318) {
		length_list[0x037b] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x037b] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x037b] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x037b] = 11;
	} else if (packetver >= 20090204) {
		length_list[0x037b] = 66;
	} else if (packetver >= 20090129) {
		length_list[0x037b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x037b] = 55;
	} else if (packetver >= 20090114) {
		length_list[0x037b] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x037b] = 5;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x037c
	if (packetver >= 20090406) {
		length_list[0x037c] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x037c] = 22;
	} else if (packetver >= 20090318) {
		length_list[0x037c] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x037c] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x037c] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x037c] = 282;
	} else if (packetver >= 20090211) {
		length_list[0x037c] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x037c] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x037c] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x037c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x037c] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x037d
	if (packetver >= 20090406) {
		length_list[0x037d] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x037d] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x037d] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x037d] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x037d] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x037d] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x037d] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x037d] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x037d] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x037e
	if (packetver >= 20090401) {
		length_list[0x037e] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x037e] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x037e] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x037e] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x037e] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x037e] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x037e] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x037e] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x037e] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x037f
	if (packetver >= 20090311) {
		length_list[0x037f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x037f] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x037f] = 22;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0380
	if (packetver >= 20090406) {
		length_list[0x0380] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0380] = 55;
	} else if (packetver >= 20090325) {
		length_list[0x0380] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0380] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x0380] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0380] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0380] = -1;
	}

	// Packet: 0x0381
	if (packetver >= 20090406) {
		length_list[0x0381] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0381] = 90;
	} else if (packetver >= 20090325) {
		length_list[0x0381] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0381] = 282;
	} else if (packetver >= 20090218) {
		length_list[0x0381] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0381] = 16;
	} else if (packetver >= 20090204) {
		length_list[0x0381] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0381] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0381] = 26;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0382
	if (packetver >= 20090325) {
		length_list[0x0382] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0382] = 32;
	} else if (packetver >= 20090225) {
		length_list[0x0382] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0382] = 5;
	} else if (packetver >= 20090129) {
		length_list[0x0382] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0382] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0382] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0382] = 2;
	}

	// Packet: 0x0383
	if (packetver >= 20090325) {
		length_list[0x0383] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0383] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x0383] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0383] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x0383] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x0383] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0383] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x0383] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0384
	if (packetver >= 20090406) {
		length_list[0x0384] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0384] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x0384] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0384] = 24;
	} else if (packetver >= 20090225) {
		length_list[0x0384] = 16;
	} else if (packetver >= 20090218) {
		length_list[0x0384] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0384] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x0384] = 5;
	} else if (packetver >= 20090129) {
		length_list[0x0384] = 186;
	} else if (packetver >= 20090120) {
		length_list[0x0384] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0384] = -1;
	}

	// Packet: 0x0385
	if (packetver >= 20090325) {
		length_list[0x0385] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0385] = 11;
	} else if (packetver >= 20090311) {
		length_list[0x0385] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0385] = 90;
	} else if (packetver >= 20090218) {
		length_list[0x0385] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0385] = 28;
	} else if (packetver >= 20090129) {
		length_list[0x0385] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0385] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0385] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0386
	if (packetver >= 20090406) {
		length_list[0x0386] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0386] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x0386] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x0386] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0386] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0386] = 27;
	} else if (packetver >= 20090129) {
		length_list[0x0386] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0386] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0386] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0387
	if (packetver >= 20090318) {
		length_list[0x0387] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0387] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0387] = 54;
	} else if (packetver >= 20090218) {
		length_list[0x0387] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x0387] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0387] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x0387] = 22;
	} else if (packetver >= 20090107) {
		length_list[0x0387] = 71;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0388
	if (packetver >= 20090318) {
		length_list[0x0388] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0388] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0388] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0388] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x0388] = 22;
	} else if (packetver >= 20090129) {
		length_list[0x0388] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0388] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x0388] = -1;
	}

	// Packet: 0x0389
	if (packetver >= 20090406) {
		length_list[0x0389] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0389] = 9;
	} else if (packetver >= 20090318) {
		length_list[0x0389] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0389] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0389] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0389] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x0389] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0389] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x0389] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0389] = 66;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x038a
	if (packetver >= 20090406) {
		length_list[0x038a] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x038a] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x038a] = 9;
	} else if (packetver >= 20090318) {
		length_list[0x038a] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x038a] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x038a] = 27;
	} else if (packetver >= 20090218) {
		length_list[0x038a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x038a] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x038a] = -1;
	}

	// Packet: 0x038b
	if (packetver >= 20090401) {
		length_list[0x038b] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x038b] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x038b] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x038b] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x038b] = 53;
	} else if (packetver >= 20090218) {
		length_list[0x038b] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x038b] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x038b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x038b] = 114;
	} else if (packetver >= 20090114) {
		length_list[0x038b] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x038b] = 27;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x038c
	if (packetver >= 20090114) {
		length_list[0x038c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x038c] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x038d
	if (packetver >= 20090318) {
		length_list[0x038d] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x038d] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x038d] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x038d] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x038d] = 43;
	} else if (packetver >= 20090204) {
		length_list[0x038d] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x038d] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x038d] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x038d] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x038d] = 32;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x038e
	if (packetver >= 20090401) {
		length_list[0x038e] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x038e] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x038e] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x038e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x038e] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x038e] = 17;
	} else if (packetver >= 20090107) {
		length_list[0x038e] = -1;
	}

	// Packet: 0x038f
	if (packetver >= 20090311) {
		length_list[0x038f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x038f] = 182;
	} else if (packetver >= 20090218) {
		length_list[0x038f] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x038f] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x038f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x038f] = 9;
	} else if (packetver >= 20090114) {
		length_list[0x038f] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x038f] = 39;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0390
	if (packetver >= 20090318) {
		length_list[0x0390] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0390] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x0390] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0390] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0390] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0390] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x0390] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0391
	if (packetver >= 20090225) {
		length_list[0x0391] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0391] = 23;
	} else if (packetver >= 20090211) {
		length_list[0x0391] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0391] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0391] = 11;
	} else if (packetver >= 20090120) {
		length_list[0x0391] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x0391] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x0391] = -1;
	}

	// Packet: 0x0392
	if (packetver >= 20090318) {
		length_list[0x0392] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0392] = 9;
	} else if (packetver >= 20090211) {
		length_list[0x0392] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0392] = 28;
	} else if (packetver >= 20090129) {
		length_list[0x0392] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0392] = 12;
	} else if (packetver >= 20090114) {
		length_list[0x0392] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x0392] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0393
	if (packetver >= 20090401) {
		length_list[0x0393] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0393] = 102;
	} else if (packetver >= 20090218) {
		length_list[0x0393] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0393] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x0393] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0393] = 16;
	} else if (packetver >= 20090120) {
		length_list[0x0393] = 12;
	} else if (packetver >= 20090107) {
		length_list[0x0393] = -1;
	}

	// Packet: 0x0394
	if (packetver >= 20090406) {
		length_list[0x0394] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0394] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x0394] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0394] = 32;
	} else if (packetver >= 20090107) {
		length_list[0x0394] = -1;
	}

	// Packet: 0x0395
	if (packetver >= 20090406) {
		length_list[0x0395] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0395] = 22;
	} else if (packetver >= 20090225) {
		length_list[0x0395] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0395] = 9;
	} else if (packetver >= 20090211) {
		length_list[0x0395] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0395] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x0395] = 31;
	} else if (packetver >= 20090114) {
		length_list[0x0395] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0395] = -1;
	}

	// Packet: 0x0396
	if (packetver >= 20090401) {
		length_list[0x0396] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0396] = 5;
	} else if (packetver >= 20090318) {
		length_list[0x0396] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0396] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0396] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0396] = 24;
	} else if (packetver >= 20090211) {
		length_list[0x0396] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0396] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x0396] = -1;
	}

	// Packet: 0x0397
	if (packetver >= 20090401) {
		length_list[0x0397] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0397] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0397] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0397] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x0397] = 55;
	} else if (packetver >= 20090114) {
		length_list[0x0397] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0397] = 9;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0398
	if (packetver >= 20090401) {
		length_list[0x0398] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0398] = 9;
	} else if (packetver >= 20090225) {
		length_list[0x0398] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0398] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0398] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0398] = 12;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0399
	if (packetver >= 20090406) {
		length_list[0x0399] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0399] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x0399] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0399] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x0399] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x0399] = 282;
	} else if (packetver >= 20090218) {
		length_list[0x0399] = 186;
	} else if (packetver >= 20090211) {
		length_list[0x0399] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x0399] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0399] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x0399] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0399] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x0399] = 9;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x039a
	if (packetver >= 20090318) {
		length_list[0x039a] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x039a] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x039a] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x039a] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x039a] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x039a] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x039a] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x039a] = 3;
	}

	// Packet: 0x039b
	if (packetver >= 20090325) {
		length_list[0x039b] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x039b] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x039b] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x039b] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x039b] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x039b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x039b] = 90;
	} else if (packetver >= 20090129) {
		length_list[0x039b] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x039b] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x039c
	if (packetver >= 20090406) {
		length_list[0x039c] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x039c] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x039c] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x039c] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x039c] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x039c] = 12;
	} else if (packetver >= 20090218) {
		length_list[0x039c] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x039c] = 39;
	} else if (packetver >= 20090204) {
		length_list[0x039c] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x039c] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x039c] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x039c] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x039c] = 86;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x039d
	if (packetver >= 20090406) {
		length_list[0x039d] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x039d] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x039d] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x039d] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x039d] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x039d] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x039d] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x039d] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x039e
	if (packetver >= 20090406) {
		length_list[0x039e] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x039e] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x039e] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x039e] = 5;
	} else if (packetver >= 20090225) {
		length_list[0x039e] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x039e] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x039e] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x039e] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x039e] = -1;
	}

	// Packet: 0x039f
	if (packetver >= 20090406) {
		length_list[0x039f] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x039f] = 29;
	} else if (packetver >= 20090325) {
		length_list[0x039f] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x039f] = 60;
	} else if (packetver >= 20090204) {
		length_list[0x039f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x039f] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x039f] = 18;
	} else if (packetver >= 20090107) {
		length_list[0x039f] = 30;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03a0
	if (packetver >= 20090406) {
		length_list[0x03a0] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03a0] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x03a0] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03a0] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x03a0] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03a0] = 14;
	} else if (packetver >= 20090120) {
		length_list[0x03a0] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03a0] = 54;
	} else if (packetver >= 20090107) {
		length_list[0x03a0] = -1;
	}

	// Packet: 0x03a1
	if (packetver >= 20090406) {
		length_list[0x03a1] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03a1] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x03a1] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x03a1] = 14;
	} else if (packetver >= 20090311) {
		length_list[0x03a1] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03a1] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x03a1] = 57;
	} else if (packetver >= 20090211) {
		length_list[0x03a1] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x03a1] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x03a1] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03a1] = 39;
	} else if (packetver >= 20090107) {
		length_list[0x03a1] = 44;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03a2
	if (packetver >= 20090401) {
		length_list[0x03a2] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03a2] = 23;
	} else if (packetver >= 20090225) {
		length_list[0x03a2] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03a2] = 28;
	} else if (packetver >= 20090120) {
		length_list[0x03a2] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03a2] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x03a2] = 282;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03a3
	if (packetver >= 20090401) {
		length_list[0x03a3] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03a3] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x03a3] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x03a3] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03a3] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x03a3] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03a3] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x03a3] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03a4
	if (packetver >= 20090406) {
		length_list[0x03a4] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03a4] = 31;
	} else if (packetver >= 20090325) {
		length_list[0x03a4] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x03a4] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x03a4] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03a4] = 14;
	} else if (packetver >= 20090211) {
		length_list[0x03a4] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x03a4] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x03a4] = 44;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03a5
	if (packetver >= 20090325) {
		length_list[0x03a5] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03a5] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x03a5] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03a5] = 28;
	} else if (packetver >= 20090114) {
		length_list[0x03a5] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03a5] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03a6
	if (packetver >= 20090401) {
		length_list[0x03a6] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03a6] = 15;
	} else if (packetver >= 20090225) {
		length_list[0x03a6] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03a6] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x03a6] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x03a6] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03a6] = 17;
	} else if (packetver >= 20090114) {
		length_list[0x03a6] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03a6] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03a7
	if (packetver >= 20090406) {
		length_list[0x03a7] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03a7] = 14;
	} else if (packetver >= 20090311) {
		length_list[0x03a7] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03a7] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x03a7] = 13;
	} else if (packetver >= 20090211) {
		length_list[0x03a7] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x03a7] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03a7] = 30;
	} else if (packetver >= 20090114) {
		length_list[0x03a7] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x03a7] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03a8
	if (packetver >= 20090325) {
		length_list[0x03a8] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03a8] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x03a8] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x03a8] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03a8] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x03a8] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x03a8] = 26;
	} else if (packetver >= 20090120) {
		length_list[0x03a8] = 86;
	} else if (packetver >= 20090114) {
		length_list[0x03a8] = 19;
	} else if (packetver >= 20090107) {
		length_list[0x03a8] = -1;
	}

	// Packet: 0x03a9
	if (packetver >= 20090406) {
		length_list[0x03a9] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03a9] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x03a9] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x03a9] = 27;
	} else if (packetver >= 20090311) {
		length_list[0x03a9] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x03a9] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x03a9] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03a9] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x03a9] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03a9] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03a9] = 15;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03aa
	if (packetver >= 20090406) {
		length_list[0x03aa] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03aa] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x03aa] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03aa] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x03aa] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x03aa] = 18;
	} else if (packetver >= 20090114) {
		length_list[0x03aa] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03aa] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03ab
	if (packetver >= 20090401) {
		length_list[0x03ab] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03ab] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x03ab] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03ab] = 9;
	} else if (packetver >= 20090225) {
		length_list[0x03ab] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03ab] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x03ab] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03ab] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x03ab] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03ab] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x03ab] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03ac
	if (packetver >= 20090325) {
		length_list[0x03ac] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03ac] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x03ac] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03ac] = 71;
	} else if (packetver >= 20090129) {
		length_list[0x03ac] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03ac] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x03ac] = 54;
	} else if (packetver >= 20090107) {
		length_list[0x03ac] = -1;
	}

	// Packet: 0x03ad
	if (packetver >= 20090325) {
		length_list[0x03ad] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03ad] = 14;
	} else if (packetver >= 20090311) {
		length_list[0x03ad] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03ad] = 15;
	} else if (packetver >= 20090218) {
		length_list[0x03ad] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03ad] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x03ad] = -1;
	}

	// Packet: 0x03ae
	if (packetver >= 20090325) {
		length_list[0x03ae] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03ae] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x03ae] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x03ae] = 282;
	} else if (packetver >= 20090218) {
		length_list[0x03ae] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03ae] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x03ae] = -1;
	}

	// Packet: 0x03af
	if (packetver >= 20090406) {
		length_list[0x03af] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03af] = 5;
	} else if (packetver >= 20090325) {
		length_list[0x03af] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03af] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x03af] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x03af] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03af] = 14;
	} else if (packetver >= 20090211) {
		length_list[0x03af] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03af] = 54;
	} else if (packetver >= 20090114) {
		length_list[0x03af] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03af] = 23;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03b0
	if (packetver >= 20090406) {
		length_list[0x03b0] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03b0] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x03b0] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03b0] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x03b0] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03b0] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x03b0] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x03b0] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03b0] = 26;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03b1
	if (packetver >= 20090406) {
		length_list[0x03b1] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03b1] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x03b1] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03b1] = 18;
	} else if (packetver >= 20090225) {
		length_list[0x03b1] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03b1] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x03b1] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03b1] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x03b1] = 26;
	} else if (packetver >= 20090114) {
		length_list[0x03b1] = 17;
	} else if (packetver >= 20090107) {
		length_list[0x03b1] = 22;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03b2
	if (packetver >= 20090311) {
		length_list[0x03b2] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03b2] = 5;
	} else if (packetver >= 20090211) {
		length_list[0x03b2] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03b2] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x03b2] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x03b2] = -1;
	}

	// Packet: 0x03b3
	if (packetver >= 20090406) {
		length_list[0x03b3] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03b3] = 79;
	} else if (packetver >= 20090325) {
		length_list[0x03b3] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x03b3] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03b3] = 53;
	} else if (packetver >= 20090218) {
		length_list[0x03b3] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03b3] = 31;
	} else if (packetver >= 20090114) {
		length_list[0x03b3] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03b3] = 39;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03b4
	if (packetver >= 20090406) {
		length_list[0x03b4] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03b4] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x03b4] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03b4] = 31;
	} else if (packetver >= 20090211) {
		length_list[0x03b4] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03b4] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x03b4] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x03b4] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x03b4] = -1;
	}

	// Packet: 0x03b5
	if (packetver >= 20090311) {
		length_list[0x03b5] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03b5] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x03b5] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03b5] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x03b5] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03b5] = 37;
	} else if (packetver >= 20090107) {
		length_list[0x03b5] = -1;
	}

	// Packet: 0x03b6
	if (packetver >= 20090406) {
		length_list[0x03b6] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03b6] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x03b6] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03b6] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x03b6] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03b6] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x03b6] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x03b6] = 60;
	} else if (packetver >= 20090114) {
		length_list[0x03b6] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x03b6] = 22;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03b7
	if (packetver >= 20090401) {
		length_list[0x03b7] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03b7] = 31;
	} else if (packetver >= 20090311) {
		length_list[0x03b7] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x03b7] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03b7] = 32;
	} else if (packetver >= 20090211) {
		length_list[0x03b7] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x03b7] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03b7] = 9;
	} else if (packetver >= 20090120) {
		length_list[0x03b7] = 4;
	} else if (packetver >= 20090114) {
		length_list[0x03b7] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03b7] = 29;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03b8
	if (packetver >= 20090401) {
		length_list[0x03b8] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03b8] = 17;
	} else if (packetver >= 20090318) {
		length_list[0x03b8] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x03b8] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03b8] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x03b8] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x03b8] = -1;
	}

	// Packet: 0x03b9
	if (packetver >= 20090401) {
		length_list[0x03b9] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03b9] = 26;
	} else if (packetver >= 20090318) {
		length_list[0x03b9] = 58;
	} else if (packetver >= 20090218) {
		length_list[0x03b9] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03b9] = 22;
	} else if (packetver >= 20090204) {
		length_list[0x03b9] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03b9] = 28;
	} else if (packetver >= 20090120) {
		length_list[0x03b9] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03b9] = 20;
	} else if (packetver >= 20090107) {
		length_list[0x03b9] = 18;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03ba
	if (packetver >= 20090406) {
		length_list[0x03ba] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03ba] = 12;
	} else if (packetver >= 20090325) {
		length_list[0x03ba] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03ba] = 22;
	} else if (packetver >= 20090311) {
		length_list[0x03ba] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03ba] = 23;
	} else if (packetver >= 20090218) {
		length_list[0x03ba] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03ba] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x03ba] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x03ba] = 42;
	} else if (packetver >= 20090114) {
		length_list[0x03ba] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03ba] = 67;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03bb
	if (packetver >= 20090406) {
		length_list[0x03bb] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03bb] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x03bb] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03bb] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x03bb] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03bb] = 34;
	} else if (packetver >= 20090107) {
		length_list[0x03bb] = -1;
	}

	// Packet: 0x03bc
	if (packetver >= 20090325) {
		length_list[0x03bc] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03bc] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x03bc] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x03bc] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03bc] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x03bc] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x03bc] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03bc] = 53;
	} else if (packetver >= 20090120) {
		length_list[0x03bc] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03bc] = 186;
	} else if (packetver >= 20090107) {
		length_list[0x03bc] = -1;
	}

	// Packet: 0x03bd
	if (packetver >= 20090401) {
		length_list[0x03bd] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03bd] = 20;
	} else if (packetver >= 20090311) {
		length_list[0x03bd] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03bd] = 67;
	} else if (packetver >= 20090211) {
		length_list[0x03bd] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03bd] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x03bd] = 28;
	} else if (packetver >= 20090107) {
		length_list[0x03bd] = -1;
	}

	// Packet: 0x03be
	if (packetver >= 20090406) {
		length_list[0x03be] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03be] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x03be] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x03be] = 42;
	} else if (packetver >= 20090311) {
		length_list[0x03be] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x03be] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03be] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03bf
	if (packetver >= 20090406) {
		length_list[0x03bf] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03bf] = 11;
	} else if (packetver >= 20090325) {
		length_list[0x03bf] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x03bf] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x03bf] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03bf] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x03bf] = 12;
	} else if (packetver >= 20090211) {
		length_list[0x03bf] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03bf] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x03bf] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03bf] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03bf] = 28;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03c0
	if (packetver >= 20090325) {
		length_list[0x03c0] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03c0] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x03c0] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x03c0] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03c0] = 102;
	} else if (packetver >= 20090204) {
		length_list[0x03c0] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03c0] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x03c0] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03c0] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03c0] = -1;
	}

	// Packet: 0x03c1
	if (packetver >= 20090401) {
		length_list[0x03c1] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03c1] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x03c1] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x03c1] = 13;
	} else if (packetver >= 20090225) {
		length_list[0x03c1] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03c1] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x03c1] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x03c1] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x03c1] = 18;
	} else if (packetver >= 20090107) {
		length_list[0x03c1] = -1;
	}

	// Packet: 0x03c2
	if (packetver >= 20090406) {
		length_list[0x03c2] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03c2] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x03c2] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x03c2] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03c2] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x03c2] = 55;
	} else if (packetver >= 20090204) {
		length_list[0x03c2] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03c2] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x03c2] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x03c2] = -1;
	}

	// Packet: 0x03c3
	if (packetver >= 20090406) {
		length_list[0x03c3] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03c3] = 114;
	} else if (packetver >= 20090325) {
		length_list[0x03c3] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03c3] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x03c3] = 18;
	} else if (packetver >= 20090225) {
		length_list[0x03c3] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03c3] = 15;
	} else if (packetver >= 20090120) {
		length_list[0x03c3] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03c3] = 3;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03c4
	if (packetver >= 20090406) {
		length_list[0x03c4] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03c4] = 39;
	} else if (packetver >= 20090325) {
		length_list[0x03c4] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x03c4] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03c4] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x03c4] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03c4] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x03c4] = -1;
	}

	// Packet: 0x03c5
	if (packetver >= 20090325) {
		length_list[0x03c5] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03c5] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x03c5] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03c5] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x03c5] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x03c5] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03c5] = 90;
	} else if (packetver >= 20090107) {
		length_list[0x03c5] = 12;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03c6
	if (packetver >= 20090406) {
		length_list[0x03c6] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03c6] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x03c6] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03c6] = 21;
	} else if (packetver >= 20090311) {
		length_list[0x03c6] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03c6] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x03c6] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03c6] = 58;
	} else if (packetver >= 20090204) {
		length_list[0x03c6] = 14;
	} else if (packetver >= 20090120) {
		length_list[0x03c6] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03c6] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x03c6] = 7;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03c7
	if (packetver >= 20090406) {
		length_list[0x03c7] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03c7] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x03c7] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03c7] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x03c7] = 282;
	} else if (packetver >= 20090211) {
		length_list[0x03c7] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03c7] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x03c7] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x03c7] = 28;
	} else if (packetver >= 20090107) {
		length_list[0x03c7] = 9;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03c8
	if (packetver >= 20090325) {
		length_list[0x03c8] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03c8] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x03c8] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03c8] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x03c8] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03c8] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03c8] = -1;
	}

	// Packet: 0x03c9
	if (packetver >= 20090406) {
		length_list[0x03c9] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03c9] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x03c9] = 26;
	} else if (packetver >= 20090311) {
		length_list[0x03c9] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03c9] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x03c9] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x03c9] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03c9] = 30;
	} else if (packetver >= 20090129) {
		length_list[0x03c9] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x03c9] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03c9] = 68;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03ca
	if (packetver >= 20090406) {
		length_list[0x03ca] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03ca] = 57;
	} else if (packetver >= 20090225) {
		length_list[0x03ca] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03ca] = 18;
	} else if (packetver >= 20090211) {
		length_list[0x03ca] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x03ca] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03ca] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x03ca] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03ca] = 34;
	} else if (packetver >= 20090107) {
		length_list[0x03ca] = -1;
	}

	// Packet: 0x03cb
	if (packetver >= 20090225) {
		length_list[0x03cb] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03cb] = 60;
	} else if (packetver >= 20090211) {
		length_list[0x03cb] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x03cb] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03cb] = 33;
	} else if (packetver >= 20090107) {
		length_list[0x03cb] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03cc
	if (packetver >= 20090401) {
		length_list[0x03cc] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03cc] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x03cc] = 29;
	} else if (packetver >= 20090225) {
		length_list[0x03cc] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03cc] = 11;
	} else if (packetver >= 20090211) {
		length_list[0x03cc] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x03cc] = -1;
	}

	// Packet: 0x03cd
	if (packetver >= 20090406) {
		length_list[0x03cd] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03cd] = 20;
	} else if (packetver >= 20090325) {
		length_list[0x03cd] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x03cd] = 66;
	} else if (packetver >= 20090311) {
		length_list[0x03cd] = 7;
	} else if (packetver >= 20090218) {
		length_list[0x03cd] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03cd] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x03cd] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03cd] = 26;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03ce
	if (packetver >= 20090225) {
		length_list[0x03ce] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03ce] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x03ce] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03ce] = 7;
	} else if (packetver >= 20090120) {
		length_list[0x03ce] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03ce] = 22;
	} else if (packetver >= 20090107) {
		length_list[0x03ce] = 7;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03cf
	if (packetver >= 20090225) {
		length_list[0x03cf] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03cf] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x03cf] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03cf] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x03cf] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03cf] = 14;
	} else if (packetver >= 20090114) {
		length_list[0x03cf] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03cf] = 30;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03d0
	if (packetver >= 20090406) {
		length_list[0x03d0] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03d0] = 14;
	} else if (packetver >= 20090211) {
		length_list[0x03d0] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03d0] = 7;
	} else if (packetver >= 20090129) {
		length_list[0x03d0] = 18;
	} else if (packetver >= 20090120) {
		length_list[0x03d0] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03d0] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03d0] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03d1
	if (packetver >= 20090406) {
		length_list[0x03d1] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03d1] = 54;
	} else if (packetver >= 20090325) {
		length_list[0x03d1] = 26;
	} else if (packetver >= 20090311) {
		length_list[0x03d1] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03d1] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x03d1] = 36;
	} else if (packetver >= 20090211) {
		length_list[0x03d1] = 28;
	} else if (packetver >= 20090114) {
		length_list[0x03d1] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03d1] = 28;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03d2
	if (packetver >= 20090406) {
		length_list[0x03d2] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03d2] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x03d2] = 29;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03d3
	if (packetver >= 20090401) {
		length_list[0x03d3] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03d3] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x03d3] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03d3] = 26;
	} else if (packetver >= 20090211) {
		length_list[0x03d3] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03d3] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x03d3] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03d3] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03d4
	if (packetver >= 20090406) {
		length_list[0x03d4] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03d4] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x03d4] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03d4] = 15;
	} else if (packetver >= 20090218) {
		length_list[0x03d4] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x03d4] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x03d4] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03d4] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x03d4] = 282;
	} else if (packetver >= 20090114) {
		length_list[0x03d4] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x03d4] = 21;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03d5
	if (packetver >= 20090325) {
		length_list[0x03d5] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03d5] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x03d5] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03d5] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x03d5] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03d5] = 60;
	} else if (packetver >= 20090114) {
		length_list[0x03d5] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03d5] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03d6
	if (packetver >= 20090406) {
		length_list[0x03d6] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03d6] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x03d6] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03d6] = 32;
	} else if (packetver >= 20090311) {
		length_list[0x03d6] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03d6] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x03d6] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x03d6] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03d6] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x03d6] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03d6] = 4;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03d7
	if (packetver >= 20090406) {
		length_list[0x03d7] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03d7] = 182;
	} else if (packetver >= 20090318) {
		length_list[0x03d7] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03d7] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x03d7] = 42;
	} else if (packetver >= 20090218) {
		length_list[0x03d7] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03d7] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x03d7] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03d7] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x03d7] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03d7] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x03d7] = 27;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03d8
	if (packetver >= 20090318) {
		length_list[0x03d8] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03d8] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x03d8] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03d8] = 32;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03d9
	if (packetver >= 20090406) {
		length_list[0x03d9] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03d9] = 11;
	} else if (packetver >= 20090325) {
		length_list[0x03d9] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03d9] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x03d9] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03d9] = 5;
	} else if (packetver >= 20090129) {
		length_list[0x03d9] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03d9] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x03d9] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03da
	if (packetver >= 20090204) {
		length_list[0x03da] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03da] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x03da] = 44;
	} else if (packetver >= 20090114) {
		length_list[0x03da] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03da] = 102;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03db
	if (packetver >= 20090325) {
		length_list[0x03db] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03db] = 5;
	} else if (packetver >= 20090211) {
		length_list[0x03db] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03db] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x03db] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03db] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x03db] = -1;
	}

	// Packet: 0x03dc
	if (packetver >= 20090211) {
		length_list[0x03dc] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03dc] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x03dc] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03dc] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x03dc] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x03dc] = 90;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03dd
	length_list[0x03dd] = 18;

	// Packet: 0x03de
	length_list[0x03de] = 18;

	// Packet: 0x03e2
	if (packetver >= 20090401) {
		length_list[0x03e2] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03e2] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x03e2] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03e2] = 29;
	} else if (packetver >= 20090204) {
		length_list[0x03e2] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03e2] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x03e2] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x03e2] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x03e2] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03e3
	if (packetver >= 20090401) {
		length_list[0x03e3] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03e3] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x03e3] = 54;
	} else if (packetver >= 20090311) {
		length_list[0x03e3] = 67;
	} else if (packetver >= 20090129) {
		length_list[0x03e3] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03e3] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x03e3] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x03e3] = -1;
	}

	// Packet: 0x03e4
	if (packetver >= 20090401) {
		length_list[0x03e4] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03e4] = 58;
	} else if (packetver >= 20090318) {
		length_list[0x03e4] = 12;
	} else if (packetver >= 20090311) {
		length_list[0x03e4] = 19;
	} else if (packetver >= 20090211) {
		length_list[0x03e4] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03e4] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x03e4] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03e4] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x03e4] = -1;
	}

	// Packet: 0x03e5
	if (packetver >= 20090211) {
		length_list[0x03e5] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03e5] = 9;
	} else if (packetver >= 20090120) {
		length_list[0x03e5] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03e5] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x03e5] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03e6
	if (packetver >= 20090406) {
		length_list[0x03e6] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03e6] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x03e6] = 54;
	} else if (packetver >= 20090318) {
		length_list[0x03e6] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03e6] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x03e6] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03e6] = 5;
	} else if (packetver >= 20090211) {
		length_list[0x03e6] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x03e6] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x03e6] = 58;
	} else if (packetver >= 20090107) {
		length_list[0x03e6] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03e7
	if (packetver >= 20090225) {
		length_list[0x03e7] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03e7] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x03e7] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03e7] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x03e7] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03e7] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x03e7] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x03e7] = 66;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03e8
	if (packetver >= 20090401) {
		length_list[0x03e8] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03e8] = 14;
	} else if (packetver >= 20090318) {
		length_list[0x03e8] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03e8] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x03e8] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03e8] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x03e8] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x03e8] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x03e8] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x03e8] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03e8] = 81;
	} else if (packetver >= 20090107) {
		length_list[0x03e8] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03e9
	if (packetver >= 20090406) {
		length_list[0x03e9] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03e9] = 10;
	} else if (packetver >= 20090325) {
		length_list[0x03e9] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x03e9] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03e9] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x03e9] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x03e9] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03e9] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x03e9] = 53;
	} else if (packetver >= 20090129) {
		length_list[0x03e9] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x03e9] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03e9] = 58;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03ea
	if (packetver >= 20090406) {
		length_list[0x03ea] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03ea] = 44;
	} else if (packetver >= 20090325) {
		length_list[0x03ea] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03ea] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x03ea] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03ea] = 97;
	} else if (packetver >= 20090107) {
		length_list[0x03ea] = -1;
	}

	// Packet: 0x03eb
	if (packetver >= 20090406) {
		length_list[0x03eb] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03eb] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x03eb] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03eb] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x03eb] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03eb] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x03eb] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x03eb] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x03eb] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x03eb] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03eb] = 39;
	} else if (packetver >= 20090107) {
		length_list[0x03eb] = -1;
	}

	// Packet: 0x03ec
	if (packetver >= 20090318) {
		length_list[0x03ec] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03ec] = 68;
	} else if (packetver >= 20090225) {
		length_list[0x03ec] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x03ec] = 282;
	} else if (packetver >= 20090204) {
		length_list[0x03ec] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03ec] = 34;
	} else if (packetver >= 20090114) {
		length_list[0x03ec] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03ec] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03ed
	if (packetver >= 20090406) {
		length_list[0x03ed] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03ed] = 32;
	} else if (packetver >= 20090325) {
		length_list[0x03ed] = 186;
	} else if (packetver >= 20090311) {
		length_list[0x03ed] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03ed] = 13;
	} else if (packetver >= 20090218) {
		length_list[0x03ed] = 9;
	} else if (packetver >= 20090211) {
		length_list[0x03ed] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03ed] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x03ed] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x03ed] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03ed] = 33;
	} else if (packetver >= 20090107) {
		length_list[0x03ed] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03ee
	if (packetver >= 20090401) {
		length_list[0x03ee] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03ee] = 7;
	} else if (packetver >= 20090318) {
		length_list[0x03ee] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03ee] = 39;
	} else if (packetver >= 20090225) {
		length_list[0x03ee] = 43;
	} else if (packetver >= 20090211) {
		length_list[0x03ee] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03ee] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x03ee] = 60;
	} else if (packetver >= 20090107) {
		length_list[0x03ee] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03ef
	if (packetver >= 20090325) {
		length_list[0x03ef] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03ef] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x03ef] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03ef] = 28;
	} else if (packetver >= 20090129) {
		length_list[0x03ef] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03ef] = 16;
	} else if (packetver >= 20090114) {
		length_list[0x03ef] = 27;
	} else if (packetver >= 20090107) {
		length_list[0x03ef] = 17;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03f0
	if (packetver >= 20090406) {
		length_list[0x03f0] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03f0] = 186;
	} else if (packetver >= 20090318) {
		length_list[0x03f0] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03f0] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x03f0] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x03f0] = 21;
	} else if (packetver >= 20090211) {
		length_list[0x03f0] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x03f0] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03f0] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x03f0] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x03f0] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03f0] = -1;
	}

	// Packet: 0x03f1
	if (packetver >= 20090401) {
		length_list[0x03f1] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03f1] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x03f1] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03f1] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x03f1] = 30;
	} else if (packetver >= 20090218) {
		length_list[0x03f1] = 65;
	} else if (packetver >= 20090211) {
		length_list[0x03f1] = 14;
	} else if (packetver >= 20090204) {
		length_list[0x03f1] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03f1] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x03f1] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03f1] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03f2
	if (packetver >= 20090311) {
		length_list[0x03f2] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03f2] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x03f2] = 26;
	} else if (packetver >= 20090211) {
		length_list[0x03f2] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03f2] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x03f2] = 282;
	} else if (packetver >= 20090120) {
		length_list[0x03f2] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x03f2] = -1;
	}

	// Packet: 0x03f3
	if (packetver >= 20090401) {
		length_list[0x03f3] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03f3] = 2;
	} else if (packetver >= 20090318) {
		length_list[0x03f3] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03f3] = 5;
	} else if (packetver >= 20090225) {
		length_list[0x03f3] = 7;
	} else if (packetver >= 20090211) {
		length_list[0x03f3] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03f3] = 9;
	} else if (packetver >= 20090129) {
		length_list[0x03f3] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x03f3] = 67;
	} else if (packetver >= 20090114) {
		length_list[0x03f3] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x03f3] = -1;
	}

	// Packet: 0x03f4
	if (packetver >= 20090406) {
		length_list[0x03f4] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03f4] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x03f4] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03f4] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x03f4] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x03f4] = 32;
	} else if (packetver >= 20090129) {
		length_list[0x03f4] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03f4] = 13;
	} else if (packetver >= 20090114) {
		length_list[0x03f4] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03f4] = 5;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03f5
	if (packetver >= 20090318) {
		length_list[0x03f5] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03f5] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x03f5] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03f5] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x03f5] = -1;
	}

	// Packet: 0x03f6
	if (packetver >= 20090406) {
		length_list[0x03f6] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03f6] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x03f6] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03f6] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x03f6] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03f6] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x03f6] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03f6] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x03f6] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x03f6] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x03f7
	if (packetver >= 20090401) {
		length_list[0x03f7] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03f7] = 54;
	} else if (packetver >= 20090318) {
		length_list[0x03f7] = 16;
	} else if (packetver >= 20090311) {
		length_list[0x03f7] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03f7] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x03f7] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03f7] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x03f7] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x03f7] = 7;
	} else if (packetver >= 20090114) {
		length_list[0x03f7] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x03f7] = -1;
	}

	// Packet: 0x03f8
	if (packetver >= 20090406) {
		length_list[0x03f8] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03f8] = 3;
	} else if (packetver >= 20090318) {
		length_list[0x03f8] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x03f8] = 21;
	} else if (packetver >= 20090211) {
		length_list[0x03f8] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03f8] = 17;
	} else if (packetver >= 20090129) {
		length_list[0x03f8] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03f8] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03f8] = -1;
	}

	// Packet: 0x03f9
	if (packetver >= 20090225) {
		length_list[0x03f9] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03f9] = 14;
	} else if (packetver >= 20090211) {
		length_list[0x03f9] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03f9] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x03f9] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x03f9] = 11;
	} else if (packetver >= 20090114) {
		length_list[0x03f9] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x03f9] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03fa
	if (packetver >= 20090406) {
		length_list[0x03fa] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03fa] = 282;
	} else if (packetver >= 20090325) {
		length_list[0x03fa] = 90;
	} else if (packetver >= 20090318) {
		length_list[0x03fa] = 33;
	} else if (packetver >= 20090311) {
		length_list[0x03fa] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x03fa] = 71;
	} else if (packetver >= 20090218) {
		length_list[0x03fa] = 5;
	} else if (packetver >= 20090211) {
		length_list[0x03fa] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03fa] = 8;
	} else if (packetver >= 20090120) {
		length_list[0x03fa] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x03fa] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x03fa] = 57;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03fb
	if (packetver >= 20090406) {
		length_list[0x03fb] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03fb] = 19;
	} else if (packetver >= 20090325) {
		length_list[0x03fb] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x03fb] = 60;
	} else if (packetver >= 20090311) {
		length_list[0x03fb] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x03fb] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03fb] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x03fb] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x03fb] = 68;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03fc
	if (packetver >= 20090406) {
		length_list[0x03fc] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x03fc] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x03fc] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x03fc] = 16;
	} else if (packetver >= 20090211) {
		length_list[0x03fc] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03fc] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x03fc] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03fc] = 16;
	} else if (packetver >= 20090107) {
		length_list[0x03fc] = 182;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x03fd
	if (packetver >= 20090211) {
		length_list[0x03fd] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x03fd] = 16;
	} else if (packetver >= 20090107) {
		length_list[0x03fd] = -1;
	}

	// Packet: 0x03fe
	if (packetver >= 20090204) {
		length_list[0x03fe] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x03fe] = 22;
	} else if (packetver >= 20090114) {
		length_list[0x03fe] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x03fe] = -1;
	}

	// Packet: 0x03ff
	if (packetver >= 20090401) {
		length_list[0x03ff] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x03ff] = 13;
	} else if (packetver >= 20090120) {
		length_list[0x03ff] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x03ff] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x03ff] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0400
	if (packetver >= 20090401) {
		length_list[0x0400] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0400] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0400] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x0400] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0400] = 8;
	} else if (packetver >= 20090211) {
		length_list[0x0400] = 67;
	} else if (packetver >= 20090107) {
		length_list[0x0400] = -1;
	}

	// Packet: 0x0401
	if (packetver >= 20090401) {
		length_list[0x0401] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0401] = 60;
	} else if (packetver >= 20090318) {
		length_list[0x0401] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x0401] = 4;
	} else if (packetver >= 20090225) {
		length_list[0x0401] = 11;
	} else if (packetver >= 20090218) {
		length_list[0x0401] = 29;
	} else if (packetver >= 20090204) {
		length_list[0x0401] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0401] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0401] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0401] = 2;
	}

	// Packet: 0x0402
	if (packetver >= 20090225) {
		length_list[0x0402] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0402] = 42;
	} else if (packetver >= 20090211) {
		length_list[0x0402] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0402] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0402] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0402] = 27;
	} else if (packetver >= 20090114) {
		length_list[0x0402] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x0402] = 14;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0403
	if (packetver >= 20090401) {
		length_list[0x0403] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0403] = 27;
	} else if (packetver >= 20090318) {
		length_list[0x0403] = 4;
	} else if (packetver >= 20090311) {
		length_list[0x0403] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0403] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0403] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x0403] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0403] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0403] = 9;
	} else if (packetver >= 20090120) {
		length_list[0x0403] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0403] = 58;
	} else if (packetver >= 20090107) {
		length_list[0x0403] = -1;
	}

	// Packet: 0x0404
	if (packetver >= 20090406) {
		length_list[0x0404] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0404] = 7;
	} else if (packetver >= 20090325) {
		length_list[0x0404] = 282;
	} else if (packetver >= 20090318) {
		length_list[0x0404] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0404] = 3;
	} else if (packetver >= 20090225) {
		length_list[0x0404] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x0404] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0404] = 11;
	} else if (packetver >= 20090204) {
		length_list[0x0404] = 6;
	} else if (packetver >= 20090120) {
		length_list[0x0404] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0404] = 7;
	} else if (packetver >= 20090107) {
		length_list[0x0404] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0405
	if (packetver >= 20090406) {
		length_list[0x0405] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0405] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x0405] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0405] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0405] = 22;
	} else if (packetver >= 20090211) {
		length_list[0x0405] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0405] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0405] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0406
	if (packetver >= 20090225) {
		length_list[0x0406] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0406] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x0406] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0406] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0406] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0406] = 16;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0407
	if (packetver >= 20090318) {
		length_list[0x0407] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0407] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0407] = -1;
	}

	// Packet: 0x0408
	if (packetver >= 20090401) {
		length_list[0x0408] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0408] = 5;
	} else if (packetver >= 20090318) {
		length_list[0x0408] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x0408] = 5;
	} else if (packetver >= 20090225) {
		length_list[0x0408] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0408] = 8;
	} else if (packetver >= 20090204) {
		length_list[0x0408] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0408] = 22;
	} else if (packetver >= 20090120) {
		length_list[0x0408] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0408] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0408] = 2;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0409
	if (packetver >= 20090401) {
		length_list[0x0409] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0409] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x0409] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0409] = 30;
	} else if (packetver >= 20090114) {
		length_list[0x0409] = 67;
	} else if (packetver >= 20090107) {
		length_list[0x0409] = 7;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x040a
	if (packetver >= 20090406) {
		length_list[0x040a] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x040a] = 6;
	} else if (packetver >= 20090325) {
		length_list[0x040a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x040a] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x040a] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x040a] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x040a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x040a] = 58;
	} else if (packetver >= 20090204) {
		length_list[0x040a] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x040a] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x040a] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x040a] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x040a] = -1;
	}

	// Packet: 0x040b
	if (packetver >= 20090406) {
		length_list[0x040b] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x040b] = 21;
	} else if (packetver >= 20090318) {
		length_list[0x040b] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x040b] = 8;
	} else if (packetver >= 20090225) {
		length_list[0x040b] = 102;
	} else if (packetver >= 20090218) {
		length_list[0x040b] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x040b] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x040b] = 14;
	} else if (packetver >= 20090120) {
		length_list[0x040b] = 8;
	} else if (packetver >= 20090107) {
		length_list[0x040b] = -1;
	}

	// Packet: 0x040c
	if (packetver >= 20090401) {
		length_list[0x040c] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x040c] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x040c] = 21;
	} else if (packetver >= 20090311) {
		length_list[0x040c] = 182;
	} else if (packetver >= 20090218) {
		length_list[0x040c] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x040c] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x040c] = 60;
	} else if (packetver >= 20090114) {
		length_list[0x040c] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x040c] = 67;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x040d
	if (packetver >= 20090401) {
		length_list[0x040d] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x040d] = 22;
	} else if (packetver >= 20090225) {
		length_list[0x040d] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x040d] = 28;
	} else if (packetver >= 20090211) {
		length_list[0x040d] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x040d] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x040d] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x040d] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x040d] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x040d] = 282;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x040e
	if (packetver >= 20090406) {
		length_list[0x040e] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x040e] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x040e] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x040e] = 26;
	} else if (packetver >= 20090311) {
		length_list[0x040e] = 11;
	} else if (packetver >= 20090225) {
		length_list[0x040e] = 14;
	} else if (packetver >= 20090218) {
		length_list[0x040e] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x040e] = 4;
	} else if (packetver >= 20090204) {
		length_list[0x040e] = 58;
	} else if (packetver >= 20090129) {
		length_list[0x040e] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x040e] = 5;
	} else if (packetver >= 20090114) {
		length_list[0x040e] = 66;
	} else if (packetver >= 20090107) {
		length_list[0x040e] = -1;
	}

	// Packet: 0x040f
	if (packetver >= 20090401) {
		length_list[0x040f] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x040f] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x040f] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x040f] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x040f] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x040f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x040f] = 29;
	} else if (packetver >= 20090114) {
		length_list[0x040f] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x040f] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0410
	if (packetver >= 20090406) {
		length_list[0x0410] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0410] = 3;
	} else if (packetver >= 20090325) {
		length_list[0x0410] = 21;
	} else if (packetver >= 20090211) {
		length_list[0x0410] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0410] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0410] = 11;
	} else if (packetver >= 20090114) {
		length_list[0x0410] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0410] = 26;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0411
	if (packetver >= 20090318) {
		length_list[0x0411] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0411] = 14;
	} else if (packetver >= 20090211) {
		length_list[0x0411] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0411] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x0411] = 81;
	} else if (packetver >= 20090120) {
		length_list[0x0411] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0411] = 9;
	} else if (packetver >= 20090107) {
		length_list[0x0411] = 186;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0412
	if (packetver >= 20090401) {
		length_list[0x0412] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0412] = 81;
	} else if (packetver >= 20090318) {
		length_list[0x0412] = 22;
	} else if (packetver >= 20090218) {
		length_list[0x0412] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0412] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0412] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0412] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0412] = 2;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0413
	if (packetver >= 20090325) {
		length_list[0x0413] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0413] = 7;
	} else if (packetver >= 20090311) {
		length_list[0x0413] = 14;
	} else if (packetver >= 20090225) {
		length_list[0x0413] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x0413] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0413] = 282;
	} else if (packetver >= 20090204) {
		length_list[0x0413] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0413] = 5;
	} else if (packetver >= 20090120) {
		length_list[0x0413] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x0413] = 2;
	} else if (packetver >= 20090107) {
		length_list[0x0413] = -1;
	}

	// Packet: 0x0414
	if (packetver >= 20090225) {
		length_list[0x0414] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0414] = 34;
	} else if (packetver >= 20090211) {
		length_list[0x0414] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0414] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x0414] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0414] = -1;
	}

	// Packet: 0x0415
	if (packetver >= 20090406) {
		length_list[0x0415] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0415] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x0415] = 29;
	} else if (packetver >= 20090318) {
		length_list[0x0415] = 17;
	} else if (packetver >= 20090311) {
		length_list[0x0415] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x0415] = 114;
	} else if (packetver >= 20090218) {
		length_list[0x0415] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0415] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0415] = 2;
	} else if (packetver >= 20090120) {
		length_list[0x0415] = 30;
	} else if (packetver >= 20090114) {
		length_list[0x0415] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0415] = 8;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0416
	if (packetver >= 20090401) {
		length_list[0x0416] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0416] = 28;
	} else if (packetver >= 20090318) {
		length_list[0x0416] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x0416] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0416] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x0416] = 15;
	} else if (packetver >= 20090114) {
		length_list[0x0416] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0416] = 8;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0417
	if (packetver >= 20090325) {
		length_list[0x0417] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0417] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x0417] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0417] = 26;
	} else if (packetver >= 20090218) {
		length_list[0x0417] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0417] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x0417] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0417] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x0417] = 59;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0418
	if (packetver >= 20090401) {
		length_list[0x0418] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0418] = 60;
	} else if (packetver >= 20090318) {
		length_list[0x0418] = 182;
	} else if (packetver >= 20090311) {
		length_list[0x0418] = 23;
	} else if (packetver >= 20090225) {
		length_list[0x0418] = 33;
	} else if (packetver >= 20090218) {
		length_list[0x0418] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0418] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0418] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0418] = 15;
	} else if (packetver >= 20090120) {
		length_list[0x0418] = 42;
	} else if (packetver >= 20090114) {
		length_list[0x0418] = 21;
	} else if (packetver >= 20090107) {
		length_list[0x0418] = -1;
	}

	// Packet: 0x0419
	if (packetver >= 20090401) {
		length_list[0x0419] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0419] = 20;
	} else if (packetver >= 20090311) {
		length_list[0x0419] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0419] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0419] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0419] = 14;
	} else if (packetver >= 20090129) {
		length_list[0x0419] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0419] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0419] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0419] = -1;
	}

	// Packet: 0x041a
	if (packetver >= 20090325) {
		length_list[0x041a] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x041a] = 3;
	} else if (packetver >= 20090311) {
		length_list[0x041a] = 102;
	} else if (packetver >= 20090225) {
		length_list[0x041a] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x041a] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x041a] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x041a] = -1;
	}

	// Packet: 0x041b
	if (packetver >= 20090318) {
		length_list[0x041b] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x041b] = 86;
	} else if (packetver >= 20090225) {
		length_list[0x041b] = 4;
	} else if (packetver >= 20090211) {
		length_list[0x041b] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x041b] = 2;
	} else if (packetver >= 20090129) {
		length_list[0x041b] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x041b] = 11;
	} else if (packetver >= 20090114) {
		length_list[0x041b] = 30;
	} else if (packetver >= 20090107) {
		length_list[0x041b] = 54;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x041c
	if (packetver >= 20090406) {
		length_list[0x041c] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x041c] = 26;
	} else if (packetver >= 20090325) {
		length_list[0x041c] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x041c] = 30;
	} else if (packetver >= 20090311) {
		length_list[0x041c] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x041c] = 20;
	} else if (packetver >= 20090218) {
		length_list[0x041c] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x041c] = 4;
	} else if (packetver >= 20090129) {
		length_list[0x041c] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x041c] = 28;
	} else if (packetver >= 20090114) {
		length_list[0x041c] = 17;
	} else if (packetver >= 20090107) {
		length_list[0x041c] = 34;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x041d
	if (packetver >= 20090318) {
		length_list[0x041d] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x041d] = 282;
	} else if (packetver >= 20090225) {
		length_list[0x041d] = 7;
	} else if (packetver >= 20090218) {
		length_list[0x041d] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x041d] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x041d] = 90;
	} else if (packetver >= 20090107) {
		length_list[0x041d] = 67;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x041e
	if (packetver >= 20090406) {
		length_list[0x041e] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x041e] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x041e] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x041e] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x041e] = 30;
	} else if (packetver >= 20090211) {
		length_list[0x041e] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x041e] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x041e] = -1;
	}

	// Packet: 0x041f
	if (packetver >= 20090401) {
		length_list[0x041f] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x041f] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x041f] = 34;
	} else if (packetver >= 20090311) {
		length_list[0x041f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x041f] = 3;
	} else if (packetver >= 20090218) {
		length_list[0x041f] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x041f] = 26;
	} else if (packetver >= 20090204) {
		length_list[0x041f] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x041f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x041f] = 8;
	} else if (packetver >= 20090114) {
		length_list[0x041f] = 5;
	} else if (packetver >= 20090107) {
		length_list[0x041f] = -1;
	}

	// Packet: 0x0420
	if (packetver >= 20090406) {
		length_list[0x0420] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0420] = 30;
	} else if (packetver >= 20090318) {
		length_list[0x0420] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x0420] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0420] = 10;
	} else if (packetver >= 20090204) {
		length_list[0x0420] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0420] = 30;
	} else if (packetver >= 20090120) {
		length_list[0x0420] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0420] = 23;
	} else if (packetver >= 20090107) {
		length_list[0x0420] = 14;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0421
	if (packetver >= 20090401) {
		length_list[0x0421] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0421] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x0421] = 2;
	} else if (packetver >= 20090218) {
		length_list[0x0421] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0421] = 7;
	} else if (packetver >= 20090204) {
		length_list[0x0421] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0421] = 4;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0422
	if (packetver >= 20090406) {
		length_list[0x0422] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0422] = 5;
	} else if (packetver >= 20090318) {
		length_list[0x0422] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0422] = 22;
	} else if (packetver >= 20090225) {
		length_list[0x0422] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0422] = 3;
	} else if (packetver >= 20090120) {
		length_list[0x0422] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0422] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0422] = 28;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0423
	if (packetver >= 20090406) {
		length_list[0x0423] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0423] = 86;
	} else if (packetver >= 20090325) {
		length_list[0x0423] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0423] = 6;
	} else if (packetver >= 20090311) {
		length_list[0x0423] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x0423] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0423] = 10;
	} else if (packetver >= 20090120) {
		length_list[0x0423] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0423] = 58;
	} else if (packetver >= 20090107) {
		length_list[0x0423] = 30;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0424
	if (packetver >= 20090225) {
		length_list[0x0424] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0424] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0424] = 9;
	} else if (packetver >= 20090204) {
		length_list[0x0424] = 8;
	} else if (packetver >= 20090129) {
		length_list[0x0424] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0424] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x0424] = 26;
	} else if (packetver >= 20090107) {
		length_list[0x0424] = -1;
	}

	// Packet: 0x0425
	if (packetver >= 20090318) {
		length_list[0x0425] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0425] = 4;
	} else if (packetver >= 20090218) {
		length_list[0x0425] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0425] = 60;
	} else if (packetver >= 20090107) {
		length_list[0x0425] = 57;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0426
	if (packetver >= 20090225) {
		length_list[0x0426] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0426] = 6;
	} else if (packetver >= 20090211) {
		length_list[0x0426] = 53;
	} else if (packetver >= 20090204) {
		length_list[0x0426] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x0426] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0426] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0426] = -1;
	}

	// Packet: 0x0427
	if (packetver >= 20090325) {
		length_list[0x0427] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0427] = 22;
	} else if (packetver >= 20090311) {
		length_list[0x0427] = 16;
	} else if (packetver >= 20090218) {
		length_list[0x0427] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0427] = 13;
	} else if (packetver >= 20090129) {
		length_list[0x0427] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0427] = 2;
	} else if (packetver >= 20090114) {
		length_list[0x0427] = 10;
	} else if (packetver >= 20090107) {
		length_list[0x0427] = 28;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0428
	if (packetver >= 20090401) {
		length_list[0x0428] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0428] = 5;
	} else if (packetver >= 20090318) {
		length_list[0x0428] = 2;
	} else if (packetver >= 20090225) {
		length_list[0x0428] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x0428] = 2;
	} else if (packetver >= 20090211) {
		length_list[0x0428] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x0428] = 102;
	} else if (packetver >= 20090129) {
		length_list[0x0428] = 7;
	} else if (packetver >= 20090120) {
		length_list[0x0428] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x0428] = 19;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0429
	if (packetver >= 20090406) {
		length_list[0x0429] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0429] = 2;
	} else if (packetver >= 20090325) {
		length_list[0x0429] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0429] = 11;
	} else if (packetver >= 20090129) {
		length_list[0x0429] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0429] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0429] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x0429] = -1;
	}

	// Packet: 0x042a
	if (packetver >= 20090406) {
		length_list[0x042a] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x042a] = 5;
	} else if (packetver >= 20090318) {
		length_list[0x042a] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x042a] = 3;
	} else if (packetver >= 20090129) {
		length_list[0x042a] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x042a] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x042a] = -1;
	}

	// Packet: 0x042b
	if (packetver >= 20090406) {
		length_list[0x042b] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x042b] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x042b] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x042b] = 10;
	} else if (packetver >= 20090218) {
		length_list[0x042b] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x042b] = 30;
	} else if (packetver >= 20090204) {
		length_list[0x042b] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x042b] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x042b] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x042b] = 58;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x042c
	if (packetver >= 20090311) {
		length_list[0x042c] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x042c] = 8;
	} else if (packetver >= 20090218) {
		length_list[0x042c] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x042c] = 2;
	} else if (packetver >= 20090204) {
		length_list[0x042c] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x042c] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x042c] = 29;
	} else if (packetver >= 20090107) {
		length_list[0x042c] = 31;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x042d
	if (packetver >= 20090401) {
		length_list[0x042d] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x042d] = 5;
	} else if (packetver >= 20090204) {
		length_list[0x042d] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x042d] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x042d] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x042d] = 24;
	} else if (packetver >= 20090107) {
		length_list[0x042d] = -1;
	}

	// Packet: 0x042e
	if (packetver >= 20090401) {
		length_list[0x042e] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x042e] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x042e] = 6;
	} else if (packetver >= 20090225) {
		length_list[0x042e] = -1;
	} else if (packetver >= 20090218) {
		length_list[0x042e] = 10;
	} else if (packetver >= 20090211) {
		length_list[0x042e] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x042e] = 67;
	} else if (packetver >= 20090120) {
		length_list[0x042e] = 6;
	} else if (packetver >= 20090114) {
		length_list[0x042e] = 4;
	} else if (packetver >= 20090107) {
		length_list[0x042e] = 6;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x042f
	if (packetver >= 20090325) {
		length_list[0x042f] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x042f] = 28;
	} else if (packetver >= 20090311) {
		length_list[0x042f] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x042f] = 3;
	} else if (packetver >= 20090211) {
		length_list[0x042f] = -1;
	} else if (packetver >= 20090204) {
		length_list[0x042f] = 10;
	} else if (packetver >= 20090129) {
		length_list[0x042f] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x042f] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x042f] = 11;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0430
	if (packetver >= 20090325) {
		length_list[0x0430] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0430] = 2;
	} else if (packetver >= 20090311) {
		length_list[0x0430] = 7;
	} else if (packetver >= 20090225) {
		length_list[0x0430] = 28;
	} else if (packetver >= 20090218) {
		length_list[0x0430] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0430] = 29;
	} else if (packetver >= 20090204) {
		length_list[0x0430] = 16;
	} else if (packetver >= 20090129) {
		length_list[0x0430] = 6;
	} else if (packetver >= 20090107) {
		length_list[0x0430] = -1;
	}

	// Packet: 0x0431
	if (packetver >= 20090406) {
		length_list[0x0431] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0431] = 4;
	} else if (packetver >= 20090325) {
		length_list[0x0431] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0431] = 10;
	} else if (packetver >= 20090311) {
		length_list[0x0431] = -1;
	} else if (packetver >= 20090225) {
		length_list[0x0431] = 6;
	} else if (packetver >= 20090204) {
		length_list[0x0431] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0431] = 43;
	} else if (packetver >= 20090120) {
		length_list[0x0431] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0431] = 7;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0432
	if (packetver >= 20090218) {
		length_list[0x0432] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0432] = 3;
	} else if (packetver >= 20090114) {
		length_list[0x0432] = -1;
	} else if (packetver >= 20090107) {
		length_list[0x0432] = 10;
		// ignored packet from 2009-01-07aRagexe
	}

	// Packet: 0x0433
	if (packetver >= 20090406) {
		length_list[0x0433] = -1;
	} else if (packetver >= 20090401) {
		length_list[0x0433] = 8;
	} else if (packetver >= 20090318) {
		length_list[0x0433] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0433] = 54;
	} else if (packetver >= 20090225) {
		length_list[0x0433] = 6;
	} else if (packetver >= 20090218) {
		length_list[0x0433] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0433] = 26;
	} else if (packetver >= 20090129) {
		length_list[0x0433] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0433] = 14;
	} else if (packetver >= 20090114) {
		length_list[0x0433] = 31;
	} else if (packetver >= 20090107) {
		length_list[0x0433] = 7;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0434
	if (packetver >= 20090401) {
		length_list[0x0434] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0434] = 6;
	} else if (packetver >= 20090318) {
		length_list[0x0434] = 59;
	} else if (packetver >= 20090311) {
		length_list[0x0434] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x0434] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0434] = 67;
	} else if (packetver >= 20090120) {
		length_list[0x0434] = 39;
	} else if (packetver >= 20090107) {
		length_list[0x0434] = 6;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0435
	if (packetver >= 20090325) {
		length_list[0x0435] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0435] = 36;
	} else if (packetver >= 20090311) {
		length_list[0x0435] = 90;
	} else if (packetver >= 20090204) {
		length_list[0x0435] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0435] = 15;
	} else if (packetver >= 20090120) {
		length_list[0x0435] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0435] = 11;
	} else if (packetver >= 20090107) {
		length_list[0x0435] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0436
	if (packetver >= 20090406) {
		length_list[0x0436] = 19;
	} else if (packetver >= 20090325) {
		length_list[0x0436] = -1;
	} else if (packetver >= 20090318) {
		length_list[0x0436] = 8;
	} else if (packetver >= 20090311) {
		length_list[0x0436] = 10;
	} else if (packetver >= 20090225) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20090120) {
		length_list[0x0436] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0436] = 14;
	} else if (packetver >= 20090107) {
		length_list[0x0436] = -1;
	}

	// Packet: 0x0437
	if (packetver >= 20090406) {
		length_list[0x0437] = 7;
	} else if (packetver >= 20090401) {
		length_list[0x0437] = -1;
	} else if (packetver >= 20090325) {
		length_list[0x0437] = 4;
	} else if (packetver >= 20090318) {
		length_list[0x0437] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0437] = 30;
	} else if (packetver >= 20090225) {
		length_list[0x0437] = 16;
	} else if (packetver >= 20090218) {
		length_list[0x0437] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0437] = 54;
	} else if (packetver >= 20090204) {
		length_list[0x0437] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0437] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0437] = 57;
	} else if (packetver >= 20090107) {
		length_list[0x0437] = -1;
	}

	// Packet: 0x0438
	if (packetver >= 20090406) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20090318) {
		length_list[0x0438] = -1;
	} else if (packetver >= 20090311) {
		length_list[0x0438] = 5;
	} else if (packetver >= 20090225) {
		length_list[0x0438] = 3;
	} else if (packetver >= 20090204) {
		length_list[0x0438] = -1;
	} else if (packetver >= 20090129) {
		length_list[0x0438] = 39;
	} else if (packetver >= 20090120) {
		length_list[0x0438] = -1;
	} else if (packetver >= 20090114) {
		length_list[0x0438] = 3;
	} else if (packetver >= 20090107) {
		length_list[0x0438] = 10;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x0439
	if (packetver >= 20090406) {
		length_list[0x0439] = 8;
	} else if (packetver >= 20090401) {
		length_list[0x0439] = 5;
	} else if (packetver >= 20090218) {
		length_list[0x0439] = -1;
	} else if (packetver >= 20090211) {
		length_list[0x0439] = 6;
	} else if (packetver >= 20090129) {
		length_list[0x0439] = -1;
	} else if (packetver >= 20090120) {
		length_list[0x0439] = 10;
	} else if (packetver >= 20090114) {
		length_list[0x0439] = 28;
	} else if (packetver >= 20090107) {
		length_list[0x0439] = 3;
		// ignored packet from 2009-01-07bRagexe
	}

	// Packet: 0x043d
	length_list[0x043d] = 8;

	// Packet: 0x043e
	length_list[0x043e] = -1;

	// Packet: 0x043f
	if (packetver >= 20090114) {
		length_list[0x043f] = 25;
	} else if (packetver >= 20090107) {
		length_list[0x043f] = 8;
	}

	// Packet: 0x0440
	length_list[0x0440] = 10;

	// Packet: 0x0441
	length_list[0x0441] = 4;

	// Packet: 0x0442
	length_list[0x0442] = 8;

	// Packet: 0x0443
	length_list[0x0443] = 8;

	// Packet: 0x0444
	length_list[0x0444] = -1;

	// Packet: 0x0445
	length_list[0x0445] = 10;

	// Packet: 0x0446
	if (packetver >= 20090218) {
		length_list[0x0446] = 14;
	}

	// Packet: 0x0447
	if (packetver >= 20090520) {
		length_list[0x0447] = 2;
	}

	// Packet: 0x0448
	if (packetver >= 20090225) {
		length_list[0x0448] = -1;
	}

	// Packet: 0x0449
	if (packetver >= 20090401) {
		length_list[0x0449] = 4;
	}

	// Packet: 0x044a
	if (packetver >= 20090406) {
		length_list[0x044a] = 6;
	}

	// Packet: 0x044b
	if (packetver >= 20090514) {
		length_list[0x044b] = 2;
	}

	// Packet: 0x07d0
	if (packetver >= 20090520) {
		length_list[0x07d0] = 6;
	}

	// Packet: 0x07d1
	if (packetver >= 20090520) {
		length_list[0x07d1] = 2;
	}

	// Packet: 0x07d2
	if (packetver >= 20090520) {
		length_list[0x07d2] = -1;
	}

	// Packet: 0x07d3
	if (packetver >= 20090520) {
		length_list[0x07d3] = 4;
	}

	// Packet: 0x07d4
	if (packetver >= 20090520) {
		length_list[0x07d4] = 4;
	}

	// Packet: 0x07d5
	if (packetver >= 20090520) {
		length_list[0x07d5] = 4;
	}

	// Packet: 0x07d6
	if (packetver >= 20090520) {
		length_list[0x07d6] = 4;
	}

	// Packet: 0x07d7
	if (packetver >= 20090603) {
		length_list[0x07d7] = 8;
	}

	// Packet: 0x07d8
	if (packetver >= 20090603) {
		length_list[0x07d8] = 8;
	}

	// Packet: 0x07d9
	if (packetver >= 20090617) {
		length_list[0x07d9] = 268;
	} else if (packetver >= 20090603) {
		length_list[0x07d9] = 254;
	}

	// Packet: 0x07da
	if (packetver >= 20090603) {
		length_list[0x07da] = 6;
	}

	// Packet: 0x07db
	if (packetver >= 20090610) {
		length_list[0x07db] = 8;
	}

	// Packet: 0x07dc
	if (packetver >= 20090617) {
		length_list[0x07dc] = 6;
	}

	// Packet: 0x07dd
	if (packetver >= 20090617) {
		length_list[0x07dd] = 54;
	}

	// Packet: 0x07de
	if (packetver >= 20090617) {
		length_list[0x07de] = 30;
	}

	// Packet: 0x07df
	if (packetver >= 20090617) {
		length_list[0x07df] = 54;
	}

	// Packet: 0x07e0
	if (packetver >= 20090708) {
		length_list[0x07e0] = 58;
	}

	// Packet: 0x07e1
	if (packetver >= 20090715) {
		length_list[0x07e1] = 15;
	}

	// Packet: 0x07e2
	if (packetver >= 20090805) {
		length_list[0x07e2] = 8;
	}

	// Packet: 0x07e3
	if (packetver >= 20090818) {
		length_list[0x07e3] = 6;
	}

	// Packet: 0x07e4
	if (packetver >= 20090818) {
		length_list[0x07e4] = -1;
	}

	// Packet: 0x07e5
	if (packetver >= 20090922) {
		length_list[0x07e5] = 8;
	} else if (packetver >= 20090825) {
		length_list[0x07e5] = -1;
	}

	// Packet: 0x07e6
	if (packetver >= 20090818) {
		length_list[0x07e6] = 8;
	}

	// Packet: 0x07e7
	if (packetver >= 20090922) {
		length_list[0x07e7] = 32;
	} else if (packetver >= 20090825) {
		length_list[0x07e7] = 5;
	}

	// Packet: 0x07e8
	if (packetver >= 20090922) {
		length_list[0x07e8] = -1;
	}

	// Packet: 0x07e9
	if (packetver >= 20090922) {
		length_list[0x07e9] = 5;
	}

	// Packet: 0x07ea
	if (packetver >= 20090929) {
		length_list[0x07ea] = 2;
	}

	// Packet: 0x07eb
	if (packetver >= 20090929) {
		length_list[0x07eb] = -1;
	}

	// Packet: 0x07ec
	if (packetver >= 20091006) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20090929) {
		length_list[0x07ec] = 6;
	}

	// Packet: 0x07ed
	if (packetver >= 20091006) {
		length_list[0x07ed] = 10;
	} else if (packetver >= 20090929) {
		length_list[0x07ed] = 8;
	}

	// Packet: 0x07ee
	if (packetver >= 20090929) {
		length_list[0x07ee] = 6;
	}

	// Packet: 0x07ef
	if (packetver >= 20090929) {
		length_list[0x07ef] = 8;
	}

	// Packet: 0x07f0
	if (packetver >= 20090929) {
		length_list[0x07f0] = 8;
	}

	// Packet: 0x07f1
	if (packetver >= 20090929) {
		length_list[0x07f1] = 15;
	}

	// Packet: 0x07f2
	if (packetver >= 20091006) {
		length_list[0x07f2] = 6;
	} else if (packetver >= 20090929) {
		length_list[0x07f2] = 4;
	}

	// Packet: 0x07f3
	if (packetver >= 20091006) {
		length_list[0x07f3] = 4;
	} else if (packetver >= 20090929) {
		length_list[0x07f3] = 3;
	}

	// Packet: 0x07f4
	if (packetver >= 20091006) {
		length_list[0x07f4] = 3;
	}

	// Packet: 0x07f5
	if (packetver >= 20091027) {
		length_list[0x07f5] = 6;
	}

	// Packet: 0x07f6
	if (packetver >= 20091027) {
		length_list[0x07f6] = 14;
	}

	// Packet: 0x07f7
	if (packetver >= 20091103) {
		length_list[0x07f7] = -1;
	}

	// Packet: 0x07f8
	if (packetver >= 20091103) {
		length_list[0x07f8] = -1;
	}

	// Packet: 0x07f9
	if (packetver >= 20091103) {
		length_list[0x07f9] = -1;
	}

	// Packet: 0x07fa
	if (packetver >= 20091117) {
		length_list[0x07fa] = 8;
	}

	// Packet: 0x07fb
	if (packetver >= 20091124) {
		length_list[0x07fb] = 25;
	}

	// Packet: 0x07fc
	if (packetver >= 20091201) {
		length_list[0x07fc] = 10;
	}

	// Packet: 0x07fd
	if (packetver >= 20091201) {
		length_list[0x07fd] = -1;
	}

	// Packet: 0x07fe
	if (packetver >= 20091201) {
		length_list[0x07fe] = 26;
	}

	// Packet: 0x07ff
	if (packetver >= 20091201) {
		length_list[0x07ff] = -1;
	}

	// Packet: 0x0800
	if (packetver >= 20091215) {
		length_list[0x0800] = -1;
	}

	// Packet: 0x0801
	if (packetver >= 20091215) {
		length_list[0x0801] = -1;
	}

	// Packet: 0x0802
	if (packetver >= 20091222) {
		length_list[0x0802] = 18;
	}

	// Packet: 0x0803
	if (packetver >= 20091222) {
		length_list[0x0803] = 4;
	}

	// Packet: 0x0804
	if (packetver >= 20091228) {
		length_list[0x0804] = 14;
	} else if (packetver >= 20091222) {
		length_list[0x0804] = 8;
	}

	// Packet: 0x0805
	if (packetver >= 20091222) {
		length_list[0x0805] = -1;
	}

	// Packet: 0x0806
	if (packetver >= 20091228) {
		length_list[0x0806] = 2;
	} else if (packetver >= 20091222) {
		length_list[0x0806] = 4;
	}

	// Packet: 0x0807
	if (packetver >= 20091228) {
		length_list[0x0807] = 4;
	} else if (packetver >= 20091222) {
		length_list[0x0807] = 2;
	}

	// Packet: 0x0808
	if (packetver >= 20091228) {
		length_list[0x0808] = 14;
	} else if (packetver >= 20091222) {
		length_list[0x0808] = 4;
	}

	// Packet: 0x0809
	if (packetver >= 20091228) {
		length_list[0x0809] = 50;
	} else if (packetver >= 20091222) {
		length_list[0x0809] = 14;
	}

	// Packet: 0x080a
	if (packetver >= 20091228) {
		length_list[0x080a] = 18;
	} else if (packetver >= 20091222) {
		length_list[0x080a] = 50;
	}

	// Packet: 0x080b
	if (packetver >= 20091228) {
		length_list[0x080b] = 6;
	} else if (packetver >= 20091222) {
		length_list[0x080b] = 18;
	}

	// Packet: 0x080c
	if (packetver >= 20091228) {
		// removed
	} else if (packetver >= 20091222) {
		length_list[0x080c] = 6;
	}

	return length_list;
}

/**
 * Export
 */
export default {
	init: init
};
