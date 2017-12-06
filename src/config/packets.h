// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _CONFIG_PACKETS_H_
#define _CONFIG_PACKETS_H_

/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/config/
 **/

#ifndef PACKETVER
	/// Do NOT edit this line! To set your client version, please do this instead:
	/// In Windows: Add this line in your src\custom\defines_pre.h file: #define PACKETVER YYYYMMDD
	/// In Linux: The same as above or run the following command: ./configure --enable-packetver=YYYYMMDD
	#define PACKETVER 20151104
#endif

#ifndef PACKETVER_RE
	/// From this point on only kRO RE clients are supported
	#if PACKETVER > 20151104
		#define PACKETVER_RE
	#endif
#endif

#if PACKETVER >= 20110817
	/// Comment to disable the official packet obfuscation support.
	/// This requires PACKETVER 2011-08-17 or newer.
	#ifndef PACKET_OBFUSCATION
		#define PACKET_OBFUSCATION

		// Define these inside src/custom/defines_pre.h or src/custom/defines_post.h
		//#define PACKET_OBFUSCATION_KEY1 <key1>
		//#define PACKET_OBFUSCATION_KEY2 <key2>
		//#define PACKET_OBFUSCATION_KEY3 <key3>

		/// Comment this to disable warnings for missing client side encryption
		#define PACKET_OBFUSCATION_WARN
	#endif
#else
	#if defined(PACKET_OBFUSCATION)
		#error You enabled packet obfuscation for a version which is too old. Minimum supported client is 2011-08-17.
	#endif
#endif

#ifndef DUMP_UNKNOWN_PACKET
	//#define DUMP_UNKNOWN_PACKET
#endif

#ifndef DUMP_INVALID_PACKET
	//#define DUMP_INVALID_PACKET
#endif

/**
 * No settings past this point
 **/

/// Check if the specified packetversion supports the pincode system
#define PACKETVER_SUPPORTS_PINCODE PACKETVER >= 20110309

/// Check if the client needs delete_date as remaining time and not the actual delete_date (actually it was tested for clients since 2013)
#define PACKETVER_CHAR_DELETEDATE (PACKETVER > 20130000 && PACKETVER <= 20141022) || PACKETVER >= 20150513

/// Check if the specified packetvresion supports the cashshop sale system
#define PACKETVER_SUPPORTS_SALES PACKETVER >= 20131223

#endif // _CONFIG_PACKETS_H_
