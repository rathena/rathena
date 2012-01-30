/*
 * 2008 January 19
 *
 * The author disclaims copyright to this source code.  In place of
 * a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 *
 ********************************************************************
 *
 * DONE:
 * - Command line
 * - Windows version
 * - Exception
 * - Registers
 * - Stacktrace (frame number starting at 0)
 * + Functions:
 *   - address
 *   - name
 *   - offset in the function
 *   - line number and source file
 *   - source code of the line (external source)
 *   - function parameters
 *   - local function variables
 * + Variables/parameters:
 *   - variable name
 *   - variable type (char/wchar, integers, floats, enums, arrays, 
 *     pointers, structs, unions, ...)
 *   - readability of memory
 *   - value of char/wchar
 *   - value of integers
 *   - value of floats
 *   - value of enums (hex number)
 *   - values of arrays
 *   - address of pointers
 *   - value of simple pointers (not pointing to another pointer)
 *   - show (char*) and (wchar*) as nul-terminated strings
 *   - show chars/wchar escaped
 * + Modules:
 *   - base address
 *   - file
 *   - version
 *   - size
 *
 * TODO:
 * + Variables/parameters:
 *   - structure members
 *   - union members
 *   - globals
 * - Portability to MinGW
 *
 * $Id$
 */

#ifdef _WIN32


/////////////////////////////////////////////////////////////////////
// Include files 
//

#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define _NO_CVCONST_H
#include <dbghelp.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/////////////////////////////////////////////////////////////////////
// Types from Cvconst.h (DIA SDK) 
//

#ifdef _NO_CVCONST_H

typedef enum _BasicType
{ 
   btNoType   = 0,
   btVoid     = 1,
   btChar     = 2,
   btWChar    = 3,
   btInt      = 6,
   btUInt     = 7,
   btFloat    = 8,
   btBCD      = 9,
   btBool     = 10,
   btLong     = 13,
   btULong    = 14,
   btCurrency = 25,
   btDate     = 26,
   btVariant  = 27,
   btComplex  = 28,
   btBit      = 29,
   btBSTR     = 30,
   btHresult  = 31
} BasicType;

typedef enum _UdtKind
{
    UdtStruct,
    UdtClass,
    UdtUnion
} UdtKind;
/*
typedef enum _SymTag { 
   SymTagNull             = 0,
   SymTagExe              = 1,
   SymTagCompiland        = 2,
   SymTagCompilandDetails = 3,
   SymTagCompilandEnv     = 4,
   SymTagFunction         = 5,
   SymTagBlock            = 6,
   SymTagData             = 7,
   SymTagAnnotation       = 8,
   SymTagLabel            = 9,
   SymTagPublicSymbol     = 10,
   SymTagUDT              = 11,
   SymTagEnum             = 12,
   SymTagFunctionType     = 13,
   SymTagPointerType      = 14,
   SymTagArrayType        = 15,
   SymTagBaseType         = 16,
   SymTagTypedef          = 17,
   SymTagBaseClass        = 18,
   SymTagFriend           = 19,
   SymTagFunctionArgType  = 20,
   SymTagFuncDebugStart   = 21,
   SymTagFuncDebugEnd     = 22,
   SymTagUsingNamespace   = 23,
   SymTagVTableShape      = 24,
   SymTagVTable           = 25,
   SymTagCustom           = 26,
   SymTagThunk            = 27,
   SymTagCustomType       = 28,
   SymTagManagedType      = 29,
   SymTagDimension        = 30
} SymTag;
*/
#endif /* _NO_CVCONST_H */


/////////////////////////////////////////////////////////////////////
// dbghelp function prototypes
//

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(
	HANDLE hProcess,
	DWORD ProcessId,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
);
typedef BOOL (WINAPI *SYMINITIALIZE)(
	HANDLE  hProcess,
	PSTR    UserSearchPath,
	BOOL    fInvadeProcess
);
typedef DWORD (WINAPI *SYMSETOPTIONS)(
	DWORD   SymOptions
);
typedef DWORD (WINAPI *SYMGETOPTIONS)(
	VOID
);
typedef BOOL (WINAPI *SYMCLEANUP)(
	HANDLE  hProcess
);
typedef BOOL (WINAPI *SYMGETTYPEINFO)(
	HANDLE                      hProcess,
	DWORD64                     ModBase,
	ULONG                       TypeId,
	IMAGEHLP_SYMBOL_TYPE_INFO   GetType,
	PVOID                       pInfo
);
typedef BOOL (WINAPI *SYMGETLINEFROMADDR)(
	HANDLE          hProcess,
	DWORD           dwAddr,
	PDWORD          pdwDisplacement,
	PIMAGEHLP_LINE  Line
);
typedef BOOL (WINAPI *SYMENUMSYMBOLS)(
	HANDLE                          hProcess,
	ULONG64                         BaseOfDll,
	PCSTR                           Mask,
	PSYM_ENUMERATESYMBOLS_CALLBACK  EnumSymbolsCallback,
	PVOID                           UserContext
);
typedef BOOL (WINAPI *SYMSETCONTEXT)(
	HANDLE                  hProcess,
	PIMAGEHLP_STACK_FRAME   StackFrame,
	PIMAGEHLP_CONTEXT       Context
);
typedef BOOL (WINAPI *SYMFROMADDR)(
	HANDLE          hProcess,
	DWORD64         Address,
	PDWORD64        Displacement,
	PSYMBOL_INFO    Symbol
);
typedef BOOL (WINAPI *STACKWALK)(
	DWORD                           MachineType,
	HANDLE                          hProcess,
	HANDLE                          hThread,
	LPSTACKFRAME                    StackFrame,
	PVOID                           ContextRecord,
	PREAD_PROCESS_MEMORY_ROUTINE    ReadMemoryRoutine,
	PFUNCTION_TABLE_ACCESS_ROUTINE  FunctionTableAccessRoutine,
	PGET_MODULE_BASE_ROUTINE        GetModuleBaseRoutine,
	PTRANSLATE_ADDRESS_ROUTINE      TranslateAddress
);
typedef PVOID (WINAPI *SYMFUNCTIONTABLEACCESS)(
	HANDLE  hProcess,
	DWORD   AddrBase
);
typedef DWORD (WINAPI *SYMGETMODULEBASE)(
	HANDLE  hProcess,
	DWORD   dwAddr
);


/////////////////////////////////////////////////////////////////////
// Custom info

/// Internal structure used to pass some data around
typedef struct _InternalData {
	// PrintStacktrace
	FILE* log_file;
	STACKFRAME* pStackframe;
	HANDLE hProcess;
	DWORD nr_of_frame;

	// PrintFunctionDetail
	BOOL as_arg_list;
	BOOL log_params;
	BOOL log_locals;
	BOOL log_globals;
	DWORD nr_of_var;

	// PrintDataInfo
	ULONG64 modBase;
} InterData;

/// dbghelp dll filename
#define DBGHELP_DLL "dbghelp.dll"

// Default report filename, used when the module path is unavailable
#define DBG_DEFAULT_FILENAME "athena"

// Extended information printed in the console
#define DBG_EXTENDED_INFORMATION \
		"Please report the crash in the bug tracker:\n" \
		"http://www.eathena.ws/board/index.php?autocom=bugtracker\n"


/////////////////////////////////////////////////////////////////////
// Global variables

HANDLE dbghelp_dll = INVALID_HANDLE_VALUE;
MINIDUMPWRITEDUMP MiniDumpWriteDump_ = NULL;
SYMINITIALIZE SymInitialize_ = NULL;
SYMSETOPTIONS SymSetOptions_ = NULL;
SYMGETOPTIONS SymGetOptions_ = NULL;
SYMCLEANUP SymCleanup_ = NULL;
SYMGETTYPEINFO SymGetTypeInfo_ = NULL;
SYMGETLINEFROMADDR SymGetLineFromAddr_ = NULL;
SYMENUMSYMBOLS SymEnumSymbols_ = NULL;
SYMSETCONTEXT SymSetContext_ = NULL;
SYMFROMADDR SymFromAddr_ = NULL;
STACKWALK StackWalk_ = NULL;
SYMFUNCTIONTABLEACCESS SymFunctionTableAccess_ = NULL;
SYMGETMODULEBASE SymGetModuleBase_ = NULL;



/////////////////////////////////////////////////////////////////////
// Code


/// Writes the minidump to file. The callback function will at the 
/// same time write the list of modules to the log file.
///
/// @param file Filename of the minidump
/// @param ptrs Exception info
/// @param module_callback Callback for MiniDumpWriteDump
/// @param log_file Log file
static VOID
Dhp__WriteMinidumpFile(
	const char*                 file,
	PEXCEPTION_POINTERS         ptrs,
	MINIDUMP_CALLBACK_ROUTINE   module_callback,
	FILE*                       log_file)
{
	// open minidump file
	HANDLE minidump_file = CreateFileA(
		file,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if( minidump_file != INVALID_HANDLE_VALUE )
	{
		MINIDUMP_EXCEPTION_INFORMATION expt_info;
		MINIDUMP_CALLBACK_INFORMATION dump_cb_info;

		expt_info.ThreadId = GetCurrentThreadId();
		expt_info.ExceptionPointers = ptrs;
		expt_info.ClientPointers = FALSE;

		dump_cb_info.CallbackRoutine = module_callback;
		dump_cb_info.CallbackParam = (void*)log_file;

		if( module_callback != NULL && log_file != NULL )
			fprintf(log_file, "\n\nLoaded modules:\n");

		MiniDumpWriteDump_(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			minidump_file,
			MiniDumpNormal,
			ptrs ? &expt_info : NULL,
			NULL,
			&dump_cb_info
		);

		CloseHandle(minidump_file);
	}
}


/// Prints module information to the log file.
/// Used as a callback to MiniDumpWriteDump.
///
/// @param data Log file
/// @param callback_input
/// @param callback_output
/// @return
static BOOL CALLBACK
Dhp__PrintModuleInfoCallback(
	void*                           data,
	CONST PMINIDUMP_CALLBACK_INPUT  callback_input,
	PMINIDUMP_CALLBACK_OUTPUT       callback_output)
{
	if( data != NULL &&
		callback_input != NULL &&
		callback_input->CallbackType == ModuleCallback)
	{
		FILE* log_file = (FILE*)data;
		MINIDUMP_MODULE_CALLBACK module = callback_input->Module;

		fprintf(log_file, "0x%p", module.BaseOfImage);

		fprintf(log_file, "  %ws", module.FullPath, log_file);

		fprintf(log_file, "  (%d.%d.%d.%d, %d bytes)\n",
			HIWORD(module.VersionInfo.dwFileVersionMS),
			LOWORD(module.VersionInfo.dwFileVersionMS),
			HIWORD(module.VersionInfo.dwFileVersionLS),
			LOWORD(module.VersionInfo.dwFileVersionLS),
			module.SizeOfImage);
	}

	return TRUE;
}


/// Prints details about the current process, platform and exception 
/// information to the log file.
///
/// @param exception Exception info
/// @param context Exception context
/// @param log_file Log file
static VOID
Dhp__PrintProcessInfo(
	EXCEPTION_RECORD*   exception,
	CONTEXT*            context,
	FILE*               log_file)
{
	OSVERSIONINFOA oi;
	LPSTR cmd_line;

	fprintf(log_file,
		"\nProcess info:\n");

	// print the command line
	cmd_line = GetCommandLineA();
	if( cmd_line )
	fprintf(log_file,
		"Cmd line: %s\n",
		cmd_line);

	// print information about the OS
	oi.dwOSVersionInfoSize = sizeof(oi);
	GetVersionExA(&oi);
	fprintf(log_file,
		"Platform: Windows OS version %d.%d build %d %s\n",
		oi.dwMajorVersion, oi.dwMinorVersion, oi.dwBuildNumber, oi.szCSDVersion);

	// print the exception code
	if( exception )
	{
		fprintf(log_file,
			"\nException:\n"
			"0x%x",
			exception->ExceptionCode);
		switch( exception->ExceptionCode )
		{
#define PRINT(x) case x: fprintf(log_file, " "#x); break
		PRINT(EXCEPTION_ACCESS_VIOLATION);
		PRINT(EXCEPTION_DATATYPE_MISALIGNMENT);
		PRINT(EXCEPTION_BREAKPOINT);
		PRINT(EXCEPTION_SINGLE_STEP);
		PRINT(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
		PRINT(EXCEPTION_FLT_DENORMAL_OPERAND);
		PRINT(EXCEPTION_FLT_DIVIDE_BY_ZERO);
		PRINT(EXCEPTION_FLT_INEXACT_RESULT);
		PRINT(EXCEPTION_FLT_INVALID_OPERATION);
		PRINT(EXCEPTION_FLT_OVERFLOW);
		PRINT(EXCEPTION_FLT_STACK_CHECK);
		PRINT(EXCEPTION_FLT_UNDERFLOW);
		PRINT(EXCEPTION_INT_DIVIDE_BY_ZERO);
		PRINT(EXCEPTION_INT_OVERFLOW);
		PRINT(EXCEPTION_PRIV_INSTRUCTION);
		PRINT(EXCEPTION_IN_PAGE_ERROR);
		PRINT(EXCEPTION_ILLEGAL_INSTRUCTION);
		PRINT(EXCEPTION_NONCONTINUABLE_EXCEPTION);
		PRINT(EXCEPTION_STACK_OVERFLOW);
		PRINT(EXCEPTION_INVALID_DISPOSITION);
		PRINT(EXCEPTION_GUARD_PAGE);
		PRINT(EXCEPTION_INVALID_HANDLE);
#undef PRINT
		}

		// print where the fault occured
		fprintf(log_file, " at location 0x%p", exception->ExceptionAddress);

		// if the exception was an access violation, print additional information
		if( exception->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && exception->NumberParameters >= 2 )
			fprintf(log_file, " %s location 0x%p", exception->ExceptionInformation[0] ? "writing to" : "reading from", exception->ExceptionInformation[1]);

		fprintf(log_file, "\n");
	}

	// print the register info
	if( context )
	{
#if defined(_M_IX86)
		fprintf(log_file,
			"\nRegisters:\n");
		if( context->ContextFlags & CONTEXT_INTEGER )
		{
			fprintf(log_file,
				"eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x\n",
				context->Eax, context->Ebx, context->Ecx,
				context->Edx, context->Esi, context->Edi);
		}
		if( context->ContextFlags & CONTEXT_CONTROL )
		{
			fprintf(log_file,
				"eip=%08x esp=%08x ebp=%08x iopl=%1x %s %s %s %s %s %s %s %s %s %s\n",
				context->Eip, context->Esp, context->Ebp, 
				(context->EFlags >> 12) & 3,	//  IOPL level value
				context->EFlags & 0x00100000 ? "vip" : "   ",	//  VIP (virtual interrupt pending)
				context->EFlags & 0x00080000 ? "vif" : "   ",	//  VIF (virtual interrupt flag)
				context->EFlags & 0x00000800 ? "ov"  : "nv",	//  VIF (virtual interrupt flag)
				context->EFlags & 0x00000400 ? "dn"  : "up",	//  OF (overflow flag)
				context->EFlags & 0x00000200 ? "ei"  : "di",	//  IF (interrupt enable flag)
				context->EFlags & 0x00000080 ? "ng"  : "pl",	//  SF (sign flag)
				context->EFlags & 0x00000040 ? "zr"  : "nz",	//  ZF (zero flag)
				context->EFlags & 0x00000010 ? "ac"  : "na",	//  AF (aux carry flag)
				context->EFlags & 0x00000004 ? "po"  : "pe",	//  PF (parity flag)
				context->EFlags & 0x00000001 ? "cy"  : "nc");	//  CF (carry flag)
		}
		if( context->ContextFlags & CONTEXT_SEGMENTS )
		{
			fprintf(log_file,
				"cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x",
				context->SegCs,
				context->SegSs,
				context->SegDs,
				context->SegEs,
				context->SegFs,
				context->SegGs,
				context->EFlags);
			if( context->ContextFlags & CONTEXT_CONTROL )
				fprintf(log_file,
					"             efl=%08x",
					context->EFlags);
			fprintf(log_file, "\n");
		}
		else if( context->ContextFlags & CONTEXT_CONTROL )
			fprintf(log_file,
					"                                                                       efl=%08x\n",
					context->EFlags);
#else /* defined(_M_IX86) */
		//TODO add more processors
#endif
	}
}


/// Prints the typename of the symbol.
///
/// @param typeIndex Type index of the symbol
/// @param symtag Symbol tag
/// @param withParens If brackets are printed around the typename
/// @param interData Inter data
static VOID
Dhp__PrintTypeName(
	DWORD       typeIndex,
	DWORD       symtag,
	BOOL        withParens,
	InterData*  interData)
{
	// inter data
	FILE* log_file;
	HANDLE hProcess;
	ULONG64 modBase;
	//
	assert( interData != NULL );
	log_file = interData->log_file;
	hProcess = interData->hProcess;
	modBase  = interData->modBase;

	if( withParens )
		fprintf(log_file, "(");

	switch( symtag )
	{
	case SymTagEnum:
		{
			WCHAR* pwszTypeName;

			if( SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_SYMNAME, &pwszTypeName) )
			{
				fprintf(log_file, "enum %ls", pwszTypeName);
				LocalFree(pwszTypeName);
			}
			else
				fprintf(log_file, "enum <symname not found>");
		}
		break;
	case SymTagBaseType:
		{
			DWORD basetype;
			ULONG64 length = 0;

			SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_LENGTH, &length);
			if( !SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_BASETYPE, &basetype) )
			{
				fprintf(log_file, "<basetype not found>");
				break;
			}
			switch( basetype )
			{
			case btVoid: fprintf(log_file, "void"); break;
			case btChar: fprintf(log_file, "char"); break;
			case btWChar: fprintf(log_file, "wchar"); break;
			case btULong: fprintf(log_file, "unsigned ");	// next
			case btLong: fprintf(log_file, "long"); break;
			case btUInt: fprintf(log_file, "unsigned ");	// next
			case btInt:
				if( length == sizeof(char) ) fprintf(log_file, "char");
				else if( length == sizeof(short) ) fprintf(log_file, "short");
				else if( length == sizeof(int) ) fprintf(log_file, "int");
				else if( length == sizeof(long long) ) fprintf(log_file, "long long");
				else fprintf(log_file, "<int%d>", length*8);
				break;
			case btFloat:
				if( length == sizeof(float) ) fprintf(log_file, "float");
				else if( length == sizeof(double) ) fprintf(log_file, "double");
				else if( length == sizeof(long double) ) fprintf(log_file, "long double");
				else fprintf(log_file, "<float%d>", length*8);
				break;
			default: fprintf(log_file, "<TODO name of basetype %d %d>", basetype, length); break;
			}
		}
		break;
	case SymTagPointerType:
		{
			DWORD subtype;
			DWORD subtag;

			if( !SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_TYPE, &subtype) )
				fprintf(log_file, "<type not found>*");
			else if( !SymGetTypeInfo_(hProcess, modBase, subtype, TI_GET_SYMTAG, &subtag) )
				fprintf(log_file, "<symtag not found>*");
			else
			{
				Dhp__PrintTypeName(subtype, subtag, FALSE, interData);
				fprintf(log_file, "*");
			}
		}
		break;
	case SymTagArrayType:
		{
			DWORD childTypeIndex;
			DWORD childSymtag;

			// print root type
			childTypeIndex = typeIndex;
			childSymtag = symtag;
			for( ; ; )
			{
				if( !SymGetTypeInfo_( hProcess, modBase, childTypeIndex, TI_GET_TYPE, &childTypeIndex) )
				{
					fprintf(log_file, "<child type not found>");
					break;
				}
				if( !SymGetTypeInfo_( hProcess, modBase, childTypeIndex, TI_GET_SYMTAG, &childSymtag) )
				{
					fprintf(log_file, "<child symtag not found>");
					break;
				}
				if( childSymtag != SymTagArrayType )
				{
					Dhp__PrintTypeName(childTypeIndex, childSymtag, FALSE, interData);
					break;
				}
				// next dimension
			}
			// print dimensions
			childTypeIndex = typeIndex;
			childSymtag = symtag;
			for( ; childSymtag == SymTagArrayType ; )
			{
				DWORD childCount;
				if( !SymGetTypeInfo_( hProcess, modBase, childTypeIndex, TI_GET_COUNT, &childCount) )
					fprintf(log_file, "[<count not found>]");
				else
					fprintf(log_file, "[%u]", childCount);
				if( !SymGetTypeInfo_( hProcess, modBase, childTypeIndex, TI_GET_TYPE, &childTypeIndex) )
				{
					fprintf(log_file, "<child type not found>");
					break;
				}
				if( !SymGetTypeInfo_( hProcess, modBase, childTypeIndex, TI_GET_SYMTAG, &childSymtag) )
				{
					fprintf(log_file, "<child symtag not found>");
					break;
				}
				// next dimension
			}
		}
		break;
	default:
		{
			WCHAR* pSymname;
			DWORD udtkind;

			if( SymGetTypeInfo_( hProcess, modBase, typeIndex, TI_GET_UDTKIND, &udtkind) )
			{
				switch( (UdtKind)udtkind )
				{
				case UdtStruct: fprintf(log_file, "struct "); break;
				case UdtClass: fprintf(log_file, "class "); break;
				case UdtUnion: fprintf(log_file, "union "); break;
				default: fprintf(log_file, "<unknown udtkind %d> ", udtkind); break;
				}
			}
			if( SymGetTypeInfo_( hProcess, modBase, typeIndex, TI_GET_SYMNAME, &pSymname ) )
			{
				fprintf(log_file, "%ls", pSymname);
				LocalFree( pSymname );
			}
			else
				fprintf(log_file, "<TODO typename of symtag %d>", symtag); break;
		}
		break;
	}

	if( withParens )
		fprintf(log_file, ")");
}


/// Prints the bytes in the target location.
///
/// @param log_file Log file
/// @param p Pointer to the data
/// @param length Length of the data in bytes
static VOID
Dhp__PrintValueBytes(
	FILE*   log_file,
	BYTE*   p,
	ULONG64 length)
{
	ULONG64 i;

	fprintf(log_file, "<bytes:");
	for( i = 0; i < length; ++i )
	{
		fprintf(log_file, "%02X", p[i]);
	}
	fprintf(log_file, ">");
}


/// Prints a wide string/char value.
///
/// @param log_file Log file
/// @param p Pointer to the value
/// @param length Length of the value in bytes
static VOID
Dhp__PrintValueWideChars(
	FILE*   log_file,
	WCHAR*  wstr,
	ULONG64 length,
	BOOL    isString)
{
	ULONG64 i;
	char* buf;
	char delim;

	length /= sizeof(WCHAR);
	delim = ( isString || length > 1 ? '\"' : '\'' );
	fprintf(log_file, "%c", delim);
	buf = (char *)LocalAlloc(LMEM_FIXED, MB_CUR_MAX+1);
	buf[MB_CUR_MAX] = '\0';
	for( i = 0; i < length; ++i )
	{
		int n;
		switch( wstr[i] )
		{
		case L'\"': fprintf(log_file, "\\\""); break;
		case L'\'': fprintf(log_file, "\\\'"); break;
		case L'\\': fprintf(log_file, "\\\\"); break;
		case L'\a': fprintf(log_file, "\\a"); break;
		case L'\b': fprintf(log_file, "\\b"); break;
		case L'\f': fprintf(log_file, "\\f"); break;
		case L'\n': fprintf(log_file, "\\n"); break;
		case L'\r': fprintf(log_file, "\\r"); break;
		case L'\t': fprintf(log_file, "\\t"); break;
		case L'\v': fprintf(log_file, "\\v"); break;
		default:
			if( iswprint(wstr[i]) && (n=wctomb(buf, wstr[i])) > 0 )
			{
				buf[n] = '\0';
				fprintf(log_file, "%s", buf);
			}
			else fprintf(log_file, "\\u%04X", wstr[i]);
			break;
		}
	}
	LocalFree(buf);
	fprintf(log_file, "%c", delim);
}


/// Prints a string/char value.
///
/// @param log_file Log file
/// @param p Pointer to the value
/// @param length Length of the value in bytes
static VOID
Dhp__PrintValueChars(
	FILE*   log_file,
	char*   str,
	ULONG64 length,
	BOOL    isString)
{
	ULONG64 i;
	char delim;

	length /= sizeof(char);
	delim = ( isString || length > 1 ? '\"' : '\'' );
	fprintf(log_file, "%c", delim);
	for( i = 0; i < length; ++i )
	{
		switch( str[i] )
		{
		case '\"': fprintf(log_file, "\\\""); break;
		case '\'': fprintf(log_file, "\\\'"); break;
		case '\\': fprintf(log_file, "\\\\"); break;
		case '\a': fprintf(log_file, "\\a"); break;
		case '\b': fprintf(log_file, "\\b"); break;
		case '\f': fprintf(log_file, "\\f"); break;
		case '\n': fprintf(log_file, "\\n"); break;
		case '\r': fprintf(log_file, "\\r"); break;
		case '\t': fprintf(log_file, "\\t"); break;
		case '\v': fprintf(log_file, "\\v"); break;
		default:
			if( isprint((unsigned char)str[i]) ) fprintf(log_file, "%c", str[i]);
			else fprintf(log_file, "\\x%02X", (unsigned char)str[i]);
			break;
		}
	}
	fprintf(log_file, "%c", delim);
}


/// Prints a float value.
///
/// @param log_file Log file
/// @param p Pointer to the value
/// @param length Length of the value in bytes
static VOID
Dhp__PrintValueFloat(
	FILE*   log_file,
	VOID*   p,
	ULONG64 length)
{
	if( length == sizeof(float) ) fprintf(log_file, "%f", *(float*)p);
	else if( length == sizeof(double) ) fprintf(log_file, "%lf", *(double*)p);
	else if( length == sizeof(long double) ) fprintf(log_file, "%Lf", *(long double*)p);
	else
	{
		fprintf(log_file, "<unexpected length %I64u>", length);
		Dhp__PrintValueBytes(log_file, (BYTE*)p, length);
	}
}


/// Prints a hex value.
///
/// @param log_file Log file
/// @param p Pointer to the value
/// @param length Length of the value in bytes
static VOID
Dhp__PrintValueHex(
	FILE*   log_file,
	VOID*   p,
	ULONG64 length)
{
	if( length == sizeof(UINT32) ) fprintf(log_file, "0x%I32X", *(UINT32*)p);
	else if( length == sizeof(UINT64) ) fprintf(log_file, "0x%I64X", *(UINT64*)p);
	else if( length == sizeof(char) ) fprintf(log_file, "0x%X", *(unsigned char*)p);
	else if( length == sizeof(short) ) fprintf(log_file, "0x%X", *(unsigned short*)p);
	else if( length == sizeof(int) ) fprintf(log_file, "0x%x", *(unsigned int*)p);
	else if( length == sizeof(long) ) fprintf(log_file, "0x%lX", *(unsigned long*)p);
	else if( length == sizeof(long long) ) fprintf(log_file, "0x%llX", *(unsigned long long*)p);
	else
	{
		fprintf(log_file, "<unexpected length %I64u>", length);
		Dhp__PrintValueBytes(log_file, (BYTE*)p, length);
	}
}


/// Prints an unsigned integer value.
///
/// @param log_file Log file
/// @param p Pointer to the value
/// @param length Length of the value in bytes
static VOID
Dhp__PrintValueUnsigned(
	FILE*   log_file,
	VOID*   p,
	ULONG64 length)
{
	if( length == sizeof(INT32) ) fprintf(log_file, "%I32u", *(INT32*)p);
	else if( length == sizeof(INT64) ) fprintf(log_file, "%I64u", *(INT64*)p);
	else if( length == sizeof(char) ) fprintf(log_file, "%u", *(unsigned char*)p);
	else if( length == sizeof(short) ) fprintf(log_file, "%u", *(unsigned short*)p);
	else if( length == sizeof(int) ) fprintf(log_file, "%u", *(unsigned int*)p);
	else if( length == sizeof(long) ) fprintf(log_file, "%lu", *(unsigned long*)p);
	else if( length == sizeof(long long) ) fprintf(log_file, "%llu", *(unsigned long long*)p);
	else
	{
		fprintf(log_file, "<unexpected length %I64u>", length);
		Dhp__PrintValueBytes(log_file, (BYTE*)p, length);
	}
}


/// Prints a signed integer value.
///
/// @param log_file Log file
/// @param p Pointer to the value
/// @param length Length of the value in bytes
static VOID
Dhp__PrintValueSigned(
	FILE*   log_file,
	VOID*   p,
	ULONG64 length)
{
	if( length == sizeof(INT32) ) fprintf(log_file, "%I32d", *(INT32*)p);
	else if( length == sizeof(INT64) ) fprintf(log_file, "%I64d", *(INT64*)p);
	else if( length == sizeof(char) ) fprintf(log_file, "%d", *(signed char*)p);
	else if( length == sizeof(short) ) fprintf(log_file, "%d", *(signed short*)p);
	else if( length == sizeof(int) ) fprintf(log_file, "%d", *(signed int*)p);
	else if( length == sizeof(long) ) fprintf(log_file, "%ld", *(signed long*)p);
	else if( length == sizeof(long long) ) fprintf(log_file, "%lld", *(signed long long*)p);
	else
	{
		fprintf(log_file, "<unexpected length %I64u>", length);
		Dhp__PrintValueBytes(log_file, (BYTE*)p, length);
	}
}


/// Prints a nul-terminated wide string value.
/// Checks if the memory can be read from.
///
/// @param log_file Log file
/// @param str Target string
static VOID
Dhp__PrintValueCWideString(
	FILE*   log_file,
	WCHAR*   str)
{
	ULONG64 length = 0;

	// check if memory is readable
	__try
	{
		while( str[length] != L'\0' )
			++length;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		if( length ) Dhp__PrintValueWideChars(log_file, str, length*sizeof(WCHAR), TRUE);	// print readable part
		fprintf(log_file, "<invalid memory>");
		return;
	}

	// print string
	Dhp__PrintValueWideChars(log_file, str, length*sizeof(WCHAR), TRUE);
}


/// Prints a nul-terminated string value.
/// Checks if the memory can be read from.
///
/// @param log_file Log file
/// @param str Target string
static VOID
Dhp__PrintValueCString(
	FILE*   log_file,
	char*   str)
{
	ULONG64 length = 0;

	assert( log_file != NULL );

	// check if memory is readable
	__try
	{
		while( str[length] != '\0' )
			++length;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		if( length ) Dhp__PrintValueChars(log_file, str, length*sizeof(char), TRUE);	// print readable part
		fprintf(log_file, "<invalid memory>");
		return;
	}

	// print string
	Dhp__PrintValueChars(log_file, str, length*sizeof(char), TRUE);
}


// forward declaration of Dhp__PrintDataContents
static VOID Dhp__PrintDataContents(DWORD typeIndex, PVOID pVariable, InterData*  interData);


/// Prints the value of the data symbol.
/// Checks if the memory can be read from.
///
/// @param typeIndex Type index of the symbol
/// @param symtag Symbol tag
/// @param pVariable Address to the symbol contents
/// @param pInterData Inter data
static VOID
Dhp__PrintDataValue(
	DWORD       typeIndex,
	DWORD       symtag,
	PVOID       pVariable,
	InterData*  pInterData)
{
	// inter data
	FILE* log_file;
	DWORD64 modBase;
	HANDLE hProcess;
	//
	ULONG64 length = 0;
	DWORD basetype;
	BOOL isValid = TRUE;

	assert( pInterData != NULL );
	log_file = pInterData->log_file;
	modBase  = pInterData->modBase;
	hProcess = pInterData->hProcess;

	if( !SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_LENGTH, &length) )
	{
		fprintf(log_file, "<unknown data length>");
		return;
	}

	// check if memory is readable
	__try
	{
		BYTE* p = (BYTE*)pVariable;
		ULONG i;
		BYTE b = 0;
		for( i = 0; i < length; ++i )
			b += p[i];	// add to make sure it's not optimized out in release mode
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		fprintf(log_file, "<invalid memory>");
		return;
	}

	switch( symtag )
	{
	case SymTagBaseType:
		{
			if( !SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_BASETYPE, &basetype) )
			{
				fprintf(log_file, "<basetype not found>");
				Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
				break;
			}
			switch( basetype )
			{
			case btInt:
			case btLong:
				Dhp__PrintValueSigned(log_file, pVariable, length);
				break;
			case btUInt:
			case btULong:
				Dhp__PrintValueUnsigned(log_file, pVariable, length);
				break;
			case btFloat:
				Dhp__PrintValueFloat(log_file, pVariable, length);
				break;
			case btChar:
				{
					if( length == sizeof(char) ) fprintf(log_file, "%u ", *(unsigned char*)pVariable);
					Dhp__PrintValueChars(log_file, (char*)pVariable, length, FALSE);
				}
				break;
			case btWChar:
				{
					if( length == sizeof(WCHAR) ) fprintf(log_file, "%u ", *(WCHAR*)pVariable);
					Dhp__PrintValueWideChars(log_file, (WCHAR*)pVariable, length, FALSE);
				}
				break;
			case btVoid:
				if( length > 0 ) Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
				break;
			default:
				fprintf(log_file, "<TODO value of basetype %d>", basetype);
				Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
				break;
			}
		}
		break;
	case SymTagEnum:
		Dhp__PrintValueHex(log_file, pVariable, length);
		break;
	case SymTagPointerType:
		{
			DWORD childTypeIndex;
			DWORD childSymtag;

			fprintf(log_file, "0x%p", *(void**)pVariable);
			if( SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_TYPE, &childTypeIndex) &&
				SymGetTypeInfo_(hProcess, modBase, childTypeIndex, TI_GET_SYMTAG, &childSymtag) && 
				childSymtag != SymTagPointerType )
			{
				DWORD childBasetype;

				// child isn't a pointer, print the contents
				fprintf(log_file, " ");
				if( childSymtag == SymTagBaseType &&
					SymGetTypeInfo_(hProcess, modBase, childTypeIndex, TI_GET_BASETYPE, &childBasetype) &&
					(childBasetype == btChar || childBasetype == btWChar) )
				{
					// string or wide string
					if( childBasetype == btChar ) Dhp__PrintValueCString(log_file, *(char**)pVariable);
					else if( childBasetype == btWChar ) Dhp__PrintValueCWideString(log_file, *(WCHAR**)pVariable);
					else fprintf(log_file, "<unexpected child basetype %d>", childBasetype);
					break;
				}
				Dhp__PrintDataValue(childTypeIndex, childSymtag, *(PVOID*)pVariable, pInterData);
			}
		}
		break;
	case SymTagArrayType:
		{
			DWORD childTypeIndex;
			DWORD childSymtag;
			DWORD count;
			DWORD i;

			if( !SymGetTypeInfo_( hProcess, modBase, typeIndex, TI_GET_TYPE, &childTypeIndex) )
			{
				fprintf(log_file, "<child type not found>");
				Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
				break;
			}
			if( !SymGetTypeInfo_( hProcess, modBase, childTypeIndex, TI_GET_SYMTAG, &childSymtag) )
			{
				fprintf(log_file, "<child symtag not found>");
				Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
				break;
			}
			if( !SymGetTypeInfo_( hProcess, modBase, typeIndex, TI_GET_COUNT, &count) )
			{
				fprintf(log_file, "<count not found>");
				Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
				break;
			}
			// print values
			fprintf(log_file, "{");
			for( i = 0; i < count; ++i )
			{
				BYTE* pData = pVariable;
				pData += i*(length/count);
				if( i > 0 ) fprintf(log_file, ",");
				Dhp__PrintDataValue(childTypeIndex, childSymtag, pData, pInterData);
			}
			fprintf(log_file, "}");
		}
		break;
	default:
#if 0
		{//## TODO show children of structs/unions
			TI_FINDCHILDREN_PARAMS* children;
			DWORD childCount;
			DWORD i;

			// count children
			if( !SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_CHILDRENCOUNT, &childCount) )
			{
				fprintf(log_file, "<child count not found>");
				Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
				break;
			}

			// Prepare to get an array of "TypeIds", representing each of the children.
			// SymGetTypeInfo(TI_FINDCHILDREN) expects more memory than just a
			// TI_FINDCHILDREN_PARAMS struct has.  Use derivation to accomplish this.
			children = (TI_FINDCHILDREN_PARAMS*)LocalAlloc(LMEM_FIXED, sizeof(TI_FINDCHILDREN_PARAMS)+childCount*sizeof(ULONG));
			children->Count = childCount;
			children->Start= 0;

			// Get the array of TypeIds, one for each child type
			if( !SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_FINDCHILDREN, &children) )
			{
				fprintf(log_file, "<children not found>");
				Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
				LocalFree(children);
				return;
			}

			// Iterate through each of the children
			fprintf(log_file, "{");
			for( i = 0; i < childCount; ++i )
			{
				DWORD childOffset;
				DWORD childTypeid;
				WCHAR* childName;
				DWORD_PTR pData;

				if( i > 0 ) fprintf(log_file, ",");

				// Get the offset of the child member, relative to its parent
				if( !SymGetTypeInfo_(hProcess, modBase, children->ChildId[i], TI_GET_OFFSET, &childOffset) )
				{
					fprintf(log_file, "<child offset not found>");
					continue;
				}

				// Get the real "TypeId" of the child.
				if( !SymGetTypeInfo_(hProcess, modBase, children->ChildId[i], TI_GET_TYPEID, &childTypeid) )
				{
					fprintf(log_file, "<child typeid not found>");
					continue;
				}

				// Calculate the address of the member
				pData = (DWORD_PTR)pVariable;
				pData += childOffset;

				// print name of the child
				if( !SymGetTypeInfo_(hProcess, modBase, childTypeid, TI_GET_SYMNAME, &childName) )
				{
					fprintf(log_file, "<child symname not found>");
					continue;
				}
				fprintf(log_file, "%ws=", childName);
				LocalFree(childName);

				// print contents of the child
				Dhp__PrintDataContents(childTypeid, (PVOID)pData, interData);
			}
			fprintf(log_file, "}");

			LocalFree(children);
		}
#endif
		Dhp__PrintValueBytes(log_file, (BYTE*)pVariable, length);
		break;
	}
}


/// Prints the contents of the data symbol. (type and value)
///
/// @param typeIndex Type index of the symbol
/// @param pVariable Address of the symbol contents
/// @param pInterData Inter data
static VOID
Dhp__PrintDataContents(
	DWORD typeIndex,
	PVOID pVariable,
	InterData*  pInterData)
{
	// inter data
	FILE* log_file;
	HANDLE hProcess;
	DWORD64 modBase;
	//
	DWORD symtag;

	assert( pInterData != NULL );
	log_file = pInterData->log_file;
	hProcess = pInterData->hProcess;
	modBase  = pInterData->modBase;

	if( SymGetTypeInfo_(hProcess, modBase, typeIndex, TI_GET_SYMTAG, &symtag) )
	{
		// print type
		Dhp__PrintTypeName(typeIndex, symtag, TRUE, pInterData);
		// print value
		Dhp__PrintDataValue(typeIndex, symtag, pVariable, pInterData);
	}
	else
		fprintf(log_file, "<symtag not found>");
}


/// Prints information about the data symbol.
///
/// @param pSymInfo Symbol info
/// @param pInterData Inter data
static VOID
Dhp__PrintDataInfo(
	PSYMBOL_INFO    pSymInfo,
	InterData*      pInterData)
{
	// inter data
	FILE* log_file;
	STACKFRAME* pStackframe;
	BOOL as_arg_list;
	BOOL log_params;
	BOOL log_locals;
	BOOL log_globals;
	int nr_of_var;
	// my data
	DWORD_PTR pVariable = 0;
	enum{ UNKNOWN, PARAM, LOCAL, GLOBAL } scope = UNKNOWN;

	assert( pSymInfo != NULL );
	assert( pInterData != NULL );
	assert( pSymInfo->Tag == SymTagData );
	log_file    = pInterData->log_file;
	pStackframe = pInterData->pStackframe;
	as_arg_list = pInterData->as_arg_list;
	log_params  = pInterData->log_params;
	log_locals  = pInterData->log_locals;
	log_globals = pInterData->log_globals;
	nr_of_var   = pInterData->nr_of_var;

	// Determine the scope and address of the variable
    if( pSymInfo->Flags & SYMFLAG_REGREL )
	{
		pVariable = pStackframe->AddrFrame.Offset;
		pVariable += (DWORD_PTR)pSymInfo->Address;
		if( pSymInfo->Flags & SYMFLAG_PARAMETER )
			scope = PARAM;	// parameter
		else if( pSymInfo->Flags & SYMFLAG_LOCAL )
		{
			scope = LOCAL;	// local
#if defined(_M_IX86)
			if( (LONG64)pSymInfo->Address  > 0) scope = PARAM;	// parameter as local (bug in DBGHELP 5.1)
#endif
		}
	}
	else if( pSymInfo->Flags & SYMFLAG_REGISTER )
	{
		scope = ( pSymInfo->Flags & SYMFLAG_PARAMETER ? PARAM : LOCAL );	// register, optimized out(?)
	}
	else
	{
		pVariable = (DWORD_PTR)pSymInfo->Address;
		scope = GLOBAL;	// It must be a global variable
	}

	// check if we should to log the variable
	if( (scope == PARAM && log_params) ||
		(scope == LOCAL && log_locals) ||
		(scope == GLOBAL && log_globals) )
	{
		// print prefix and name
		if( as_arg_list )
			fprintf(log_file, "%s%s=", (nr_of_var ? ", " : ""), pSymInfo->Name);
		else
			fprintf(log_file, "\t%s = ", pSymInfo->Name);

		// print value
		if( !(pSymInfo->Flags & SYMFLAG_REGREL) && (pSymInfo->Flags & SYMF_REGISTER) )
			fprintf(log_file, "<value optimized out>");
		else
		{
			pInterData->modBase = pSymInfo->ModBase;
			Dhp__PrintDataContents(pSymInfo->TypeIndex, (PVOID)pVariable, pInterData);
		}

		// print postfix
		if( !as_arg_list )
			fprintf(log_file, "\n");
		pInterData->nr_of_var = ++nr_of_var;
	}
}


/// Prints information about the symbol.
///
/// @param pSymInfo Symbol info
/// @param pInterData Inter data
static VOID
Dhp__PrintSymbolInfo(
	PSYMBOL_INFO    pSymInfo,
	InterData*      pInterData)
{
	assert( pSymInfo != NULL );
	assert( pInterData != NULL );

	switch( pSymInfo->Tag ) 
	{
	case SymTagData: Dhp__PrintDataInfo( pSymInfo, pInterData ); break; 
	default: /*fprintf(pInterData->log_file, "<unsupported symtag %d>", pSymInfo->Tag);*/ break;
	}
}


/// Prints the details of one symbol to the log file.
/// Used as a callback for SymEnumSymbols.
///
/// @param pSymInfo Symbol info
/// @param symSize Size of the symbol info structure
/// @param pData Inter data
static BOOL WINAPI
Dhp__EnumSymbolsCallback(
	PSYMBOL_INFO    pSymInfo,
	ULONG           symSize,
	PVOID           pData)
{
	if( pSymInfo == NULL )
		return TRUE;	// try other symbols

	if( pData == NULL )
	{
		printf("Dhp__EnumSymbolsCallback: pData is NULL\n");
		return FALSE;
	}

	Dhp__PrintSymbolInfo(pSymInfo, (InterData*)pData);
	return TRUE;
}


/// Prints the source code of the target line.
/// Searches for the target file in the original path,
/// in the last src folder of the original path (relative)
/// and in the current directory.
///
/// @param filename Original source file
/// @param line Target line
/// @param log_file Log file
static VOID
Dhp__PrintSourceLine(
	FILE* log_file,
	char* filename,
	DWORD line)
{
	char path[MAX_PATH*3];
	char pathBuffer[MAX_PATH+1];
	char* p;

	assert( filename != NULL );
	assert( log_file != NULL );

	// generate search paths
	strcpy(path, filename);	// original path
	p = strrchr(path, '\\');
	if( p )
	{
		memcpy(p, ";\0", 2);
		p = strstr(filename, "\\src\\");
		if( p )
		{
			while( strstr(p+1, "\\src\\") )
				p = strstr(p+1, "\\src\\");
			strcat(path, p+1);	// last src folder path
			p = strrchr(path, '\\');
			memcpy(p, ";\0", 2);
		}
		filename = strrchr(filename, '\\')+1;
	}
	else
		*path = '\0';	// no path
	strcat(path, ".");	// current directoy

	// search for file and line
	if( SearchPathA(path, filename, NULL, MAX_PATH, pathBuffer, NULL) )
	{
		char code[1024+1];
		DWORD i = 1;
		FILE* fp;

		fp = fopen(pathBuffer, "rt");
		if( fp == NULL )
			return;

		code[1024] = '\0';
		while( fgets(code, 1024, fp) )
		{
			if( i == line )
			{// found line
				char* term = strchr(code, '\n');
				if( term && term != code && term[-1] == '\r' ) --term;
				if( term ) *term = '\0';
				fprintf(log_file, "%d\t%s\n", line, code);
				break;
			}
			if( strchr(code, '\n') )
				++i;
		}
		fclose(fp);
	}
}


/// Prints details of one function to the log file.
///
/// @param interData Inter data
static VOID
Dhp__PrintFunctionDetails(
	InterData*  pInterData)
{
	// inter data
	HANDLE hProcess;
	STACKFRAME* pStackframe;
	FILE* log_file;
	int nr_of_frame;
	//
	PSYMBOL_INFO pSymbolInfo;
	DWORD64 funcDisplacement=0;
	IMAGEHLP_STACK_FRAME imagehlpStackFrame;
	IMAGEHLP_LINE imagehlpLine;
	DWORD lineDisplacement=0;

	assert( pInterData != NULL );
	hProcess    = pInterData->hProcess;
	pStackframe = pInterData->pStackframe;
	log_file    = pInterData->log_file;
	nr_of_frame = pInterData->nr_of_frame;

	// frame info
	fprintf(log_file,
		"#%d  0x%p",
		nr_of_frame, (void*)(DWORD_PTR)pStackframe->AddrPC.Offset);

	// restrict symbol enumeration to this frame only
	ZeroMemory(&imagehlpStackFrame, sizeof(IMAGEHLP_STACK_FRAME));
	imagehlpStackFrame.InstructionOffset = pStackframe->AddrPC.Offset;
	SymSetContext_(hProcess, &imagehlpStackFrame, 0);

	// function name and displacement
	pSymbolInfo = (PSYMBOL_INFO)LocalAlloc(LMEM_FIXED, sizeof(SYMBOL_INFO)+1024);
	pSymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbolInfo->MaxNameLen = 1024;
	if( SymFromAddr_(hProcess, pStackframe->AddrPC.Offset, &funcDisplacement, pSymbolInfo) == TRUE )
	{
		fprintf(log_file,
			" in %.1024s+0x%I64X (",
			pSymbolInfo->Name, funcDisplacement);

		// log all function parameters
		pInterData->as_arg_list = TRUE;
		pInterData->log_params = TRUE;
		pInterData->log_locals = FALSE;
		pInterData->log_globals = FALSE;
		pInterData->nr_of_var = 0;
		SymEnumSymbols_(hProcess, 0, 0, Dhp__EnumSymbolsCallback, pInterData);

		fprintf(log_file,
			")");
	}
	else
		fprintf(log_file,
			"in <unknown function>");

	// find the source line for this function.
	imagehlpLine.SizeOfStruct = sizeof(IMAGEHLP_LINE);
	if( SymGetLineFromAddr_(hProcess, pStackframe->AddrPC.Offset, &lineDisplacement, &imagehlpLine) != 0 )
	{
		char* filename = imagehlpLine.FileName;
		DWORD line = imagehlpLine.LineNumber;

		fprintf(log_file,
			" at %s:%d\n",
			filename, line);

		Dhp__PrintSourceLine(log_file, filename, line);
	}
	else
		fprintf(log_file,
			"\n");

	// log all function local variables
	pInterData->as_arg_list = FALSE;
	pInterData->log_params = FALSE;
	pInterData->log_locals = TRUE;
	pInterData->log_globals = FALSE;
	pInterData->nr_of_var = 0;
	SymEnumSymbols_(hProcess, 0, 0, Dhp__EnumSymbolsCallback, pInterData);

	pInterData->nr_of_frame = ++nr_of_frame;
	LocalFree(pSymbolInfo);
}


/// Walks over the stack and prints all relevant information to the log file.
///
/// @param context Exception context
/// @param log_file Log file
static VOID
Dhp__PrintStacktrace(
	CONTEXT *context,
	FILE *log_file)
{
	HANDLE hProcess = GetCurrentProcess();
	STACKFRAME stackframe;
	DWORD machine;
	CONTEXT ctx;
	InterData interData;
	int skip = 0;
	int i;

	assert( log_file != NULL );

	fprintf(log_file,
		"\nStacktrace:\n");

	// Use thread information - if not supplied.
	if( context == NULL )
	{
		// If no context is supplied, skip 1 frame
		skip = 1;

		ctx.ContextFlags = CONTEXT_FULL;
		if( GetThreadContext(GetCurrentThread(), &ctx) )
			context = &ctx;
	}

	if( context == NULL )
		return;

	// Write the stack trace
	ZeroMemory(&stackframe, sizeof(STACKFRAME));
	stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrStack.Mode = AddrModeFlat;
	stackframe.AddrFrame.Mode = AddrModeFlat;
#if defined(_M_IX86)
	machine = IMAGE_FILE_MACHINE_I386;
	stackframe.AddrPC.Offset = context->Eip;
	stackframe.AddrStack.Offset = context->Esp;
	stackframe.AddrFrame.Offset = context->Ebp;
#else /* defined(_M_IX86) */
#error FIXME add more processors
some compilers don't stop on #error, this line makes sure it errors out
#endif

	interData.hProcess = hProcess;
	interData.log_file = log_file;
	interData.pStackframe = &stackframe;
	interData.nr_of_frame = 0;
	for( i = 0; ; ++i )
	{
		if( !StackWalk_(machine, hProcess, GetCurrentThread(),
			&stackframe, context, NULL, SymFunctionTableAccess_,
			SymGetModuleBase_, NULL))
		{
			break;
		}

		if( i >= skip )
		{
			// Check that the address is not zero.
			// Sometimes StackWalk returns TRUE with a frame of zero.
			if( stackframe.AddrPC.Offset != 0 )
				Dhp__PrintFunctionDetails(&interData);
		}
	}
}


typedef BOOL (WINAPI *ISDEBUGGERPRESENT)(void);
/// Checks if a debugger is attached to this process
///
/// @return TRUE is a debugger is present
static BOOL
Dhp__IsDebuggerPresent()
{
	HANDLE kernel32_dll;
	ISDEBUGGERPRESENT IsDebuggerPresent_;
	BOOL result;

	kernel32_dll = LoadLibraryA("kernel32.dll");
	if( kernel32_dll == NULL )
		return FALSE;

	IsDebuggerPresent_ = (ISDEBUGGERPRESENT)GetProcAddress(kernel32_dll, "IsDebuggerPresent");
	if( IsDebuggerPresent_ )
		result = IsDebuggerPresent_();
	else
		result = FALSE;

	FreeLibrary(kernel32_dll);

	return result;
}


/// Loads the dbghelp.dll library.
///
/// @return TRUE is sucessfull
static BOOL
Dhp__LoadDbghelpDll()
{
	dbghelp_dll = LoadLibraryA(DBGHELP_DLL);
	if( dbghelp_dll != INVALID_HANDLE_VALUE )
	{
		DWORD opts;

		// load the functions
		MiniDumpWriteDump_      =      (MINIDUMPWRITEDUMP)GetProcAddress(dbghelp_dll, "MiniDumpWriteDump");
		SymInitialize_          =          (SYMINITIALIZE)GetProcAddress(dbghelp_dll, "SymInitialize");
		SymSetOptions_          =          (SYMSETOPTIONS)GetProcAddress(dbghelp_dll, "SymSetOptions");
		SymGetOptions_          =          (SYMGETOPTIONS)GetProcAddress(dbghelp_dll, "SymGetOptions");
		SymCleanup_             =             (SYMCLEANUP)GetProcAddress(dbghelp_dll, "SymCleanup");
		SymGetTypeInfo_         =         (SYMGETTYPEINFO)GetProcAddress(dbghelp_dll, "SymGetTypeInfo");
		SymGetLineFromAddr_     =     (SYMGETLINEFROMADDR)GetProcAddress(dbghelp_dll, "SymGetLineFromAddr");
		SymEnumSymbols_         =         (SYMENUMSYMBOLS)GetProcAddress(dbghelp_dll, "SymEnumSymbols");
		SymSetContext_          =          (SYMSETCONTEXT)GetProcAddress(dbghelp_dll, "SymSetContext");
		SymFromAddr_            =            (SYMFROMADDR)GetProcAddress(dbghelp_dll, "SymFromAddr");
		StackWalk_              =              (STACKWALK)GetProcAddress(dbghelp_dll, "StackWalk");
		SymFunctionTableAccess_ = (SYMFUNCTIONTABLEACCESS)GetProcAddress(dbghelp_dll, "SymFunctionTableAccess");
		SymGetModuleBase_       =       (SYMGETMODULEBASE)GetProcAddress(dbghelp_dll, "SymGetModuleBase");

		if( MiniDumpWriteDump_ &&
			SymInitialize_  && SymSetOptions_  && SymGetOptions_ &&
			SymCleanup_     && SymGetTypeInfo_ && SymGetLineFromAddr_ &&
			SymEnumSymbols_ && SymSetContext_  && SymFromAddr_ && StackWalk_ &&
			SymFunctionTableAccess_ && SymGetModuleBase_ )
		{
			// initialize the symbol loading code
			opts = SymGetOptions_();

			// Set the 'load lines' option to retrieve line number information.
			// Set the 'deferred loads' option to map the debug info in memory only when needed.
			SymSetOptions_(opts | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);

			// Initialize the dbghelp DLL with the default path and automatic
			// module enumeration (and loading of symbol tables) for this process.
			SymInitialize_(GetCurrentProcess(), NULL, TRUE);

			return TRUE;
		}
	}

	if( dbghelp_dll )
	{
		FreeLibrary(dbghelp_dll);
		dbghelp_dll = NULL;
	}

	return FALSE;
}


/// Unloads the dbghelp.dll library.
static VOID
Dhp__UnloadDbghlpDll()
{
	SymCleanup_(GetCurrentProcess());

	FreeLibrary(dbghelp_dll);
	dbghelp_dll = NULL;
}


/// Creates the report and minidump files.
/// Puts the resulting pathnames in the arguments.
/// The buffers must be at least MAX_PATH+1 in size.
///
/// @param out_lpszLogFileName Buffer for the report filename
/// @param out_lpszDmpFileName Buffer for the minidump filename
/// @return TRUE if the files were created
static BOOL
Dhp__CreateFiles(
	char*   out_logFileName,
	char*   out_dmpFileName)
{
#define LEN_TIMESTAMP 14	// "YYYYMMDDhhmmss"
#define LEN_EXT 4	// ".rpt" or ".dmp"
	char baseFileName[MAX_PATH+1];
	char timestamp[LEN_TIMESTAMP+1];
	FILE* fp;
	time_t now;

	// Generate base filename for the report/minidump
	ZeroMemory(baseFileName, sizeof(baseFileName));
	if( GetModuleFileName(NULL, baseFileName, MAX_PATH-LEN_TIMESTAMP-LEN_EXT) )
	{
		char* pTerm = strrchr(baseFileName, '\\');
		if( pTerm == NULL ) pTerm = baseFileName;
		pTerm = strrchr(pTerm, '.');
		if( pTerm ) *pTerm = '\0';	// remove extension
	}
	else if( GetTempPathA(MAX_PATH-6-LEN_TIMESTAMP-LEN_EXT, baseFileName) )
	{// in temp folder
		strcat(baseFileName, DBG_DEFAULT_FILENAME);
	}
	else
	{// in current folder
		strcpy(baseFileName, DBG_DEFAULT_FILENAME);
	}

	time(&now);
#if 0
	szTimestamp[0] = '\0';
#else
	strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", localtime(&now));
#endif
	timestamp[LEN_TIMESTAMP] = '\0';
	
	sprintf(out_logFileName, "%s%s.rpt", baseFileName, timestamp);
	fp = fopen(out_logFileName, "w");
	if( fp == NULL )
		return FALSE;	// failed to create log file
	fclose(fp);

	sprintf(out_dmpFileName, "%s%s.dmp", baseFileName, timestamp);
	fp = fopen(out_dmpFileName, "w");
	if( fp == NULL)
		return FALSE;	// failed to create dump file
	fclose(fp);

	return TRUE;	// success
#undef LEN_EXT
#undef LEN_TIMESTAMP
}


/// Unhandled exception handler. Where the magic starts... ;D
///
/// @param ptrs Exception information
/// @return What to do with the exception
LONG WINAPI
Dhp__UnhandledExceptionFilter(PEXCEPTION_POINTERS ptrs)
{
	char szLogFileName[MAX_PATH+1];
	char szDmpFileName[MAX_PATH+1];
	FILE* log_file;

	// check if the crash handler was already loaded (crash while handling the crash)
	if( dbghelp_dll != INVALID_HANDLE_VALUE )
		return EXCEPTION_CONTINUE_SEARCH;

	// don't log anything if we're running inside a debugger ...
	if( Dhp__IsDebuggerPresent() == TRUE )
		return EXCEPTION_CONTINUE_SEARCH;

	// ... or if we can't load dbghelp.dll ...
	if( Dhp__LoadDbghelpDll() == FALSE )
		return EXCEPTION_CONTINUE_SEARCH;

	// ... or if we can't create the log files
	if( Dhp__CreateFiles(szLogFileName, szDmpFileName) == FALSE )
		return EXCEPTION_CONTINUE_SEARCH;

	// open log file
	log_file = fopen(szLogFileName, "wt");

	// print information about the process
	Dhp__PrintProcessInfo(
		ptrs ? ptrs->ExceptionRecord : NULL,
		ptrs ? ptrs->ContextRecord : NULL,
		log_file);

	// print the stacktrace
	Dhp__PrintStacktrace(
		ptrs ? ptrs->ContextRecord : NULL,
		log_file);

	// write the minidump file and use the callback to print the list of modules to the log file
	Dhp__WriteMinidumpFile(
		szDmpFileName,
		ptrs,
		Dhp__PrintModuleInfoCallback,
		log_file);

	fclose(log_file);

	Dhp__UnloadDbghlpDll();

	// inform the user
	fprintf(stderr,
		"\n"
		"This application has halted due to an unexpected error.\n"
		"A crash report and minidump file were saved to disk, you can find them here:\n"
		"%s\n"
		"%s\n"
		DBG_EXTENDED_INFORMATION
		"\n"
		"NOTE: The crash report and minidump files can contain sensitive information\n"
		"(filenames, partial file content, usernames and passwords etc.)\n",
		szLogFileName,
		szDmpFileName);

	// terminate the application
	return EXCEPTION_EXECUTE_HANDLER;
}



/////////////////////////////////////////////////////////////////////
// DLL stuff
#if !defined(DBG_EMBEDDED)


/// Previous exception filter.
static LPTOP_LEVEL_EXCEPTION_FILTER previousFilter;


#if defined(__GNUC__)
// GNU : define DLL load/unload functions
static void Dhp__OnStartup(void) __attribute__((constructor));
static void Dhp__OnExit(void) __attribute__((destructor));
#endif /* defined(__GNUC__) */


/// Installs as the unhandled exception handler.
void Dhp__OnStartup(void)
{
	// Install the unhandled exception filter function
	previousFilter = SetUnhandledExceptionFilter(Dhp__UnhandledExceptionFilter);
}


/// Uninstalls the handler.
void Dhp__OnExit(void)
{
	SetUnhandledExceptionFilter(previousFilter);
}


#if !defined(__GNUC__)
// Windows : invoke DLL load/unload functions
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch( dwReason )
	{
		case DLL_PROCESS_ATTACH: Dhp__OnStartup(); break;
		case DLL_PROCESS_DETACH: Dhp__OnExit(); break;
	}
	return TRUE;
}
#endif /* !defined(__GNUC__) */



#endif /* !defined(DBG_EMBEDDED) */



#endif /* _WIN32 */
