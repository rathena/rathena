// 
// Athena style config parser
// (would be better to have "one" implementation instead of .. 4 :) 
// 
//
// Author: Florian Wilkemeyer <fw@f-ws.de>
//
// Copyright (c) RAthena Project (www.rathena.org) - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbasetypes.h"
#include "showmsg.h"
#include "db.h"
#include "malloc.h"

#include "raconf.h"

#define SECTION_LEN 32
#define VARNAME_LEN 64

struct raconf {
	DBMap *db;
};


struct conf_value{
	int64 intval;
	bool bval;
	double floatval;
	size_t strval_len; // not includung \0 
	char strval[16];
};



static struct conf_value *makeValue(const char *key, char *val, size_t val_len){
	struct conf_value *v;
/*	size_t sz;
		
	sz = sizeof(struct conf_value);
	if(val_len >=  sizeof(v->strval))
		sz += (val_len - sizeof(v->strval) +  1);*/
	
	v = (struct conf_value*)aCalloc(1, sizeof(struct conf_value));
	if(v == NULL){
		ShowFatalError("raconf: makeValue => Out of Memory while allocating new node.\n");
		return NULL;
	}
	
	memcpy(v->strval, val, val_len);
	v->strval[val_len+1] = '\0';
	v->strval_len = val_len;
	
	
	// Parse boolean value:
	if((val_len == 4)  &&  (strncmpi("true", val, 4) == 0))
		v->bval = true;
	else if((val_len == 3) && (strncmpi("yes", val, 3) == 0))
		v->bval = true;
	else if((val_len == 3) && (strncmpi("oui", val, 3) == 0))
		v->bval = true;
	else if((val_len == 2) && (strncmpi("si", val, 2) == 0))
		v->bval = true;
	else if((val_len == 2) && (strncmpi("ja", val, 2) == 0))
		v->bval = true;
	else if((val_len == 1) && (*val == '1'))
		v->bval = true;
	else if((val_len == 5) && (strncmpi("false", val, 5) == 0))
		v->bval = false;
	else if((val_len == 2) && (strncmpi("no", val, 2) == 0))
		v->bval = false;
	else if((val_len == 3) && (strncmpi("non", val, 3) == 0))
		v->bval = false;
	else if((val_len == 4) && (strncmpi("nein", val, 4) == 0))
		v->bval = false;
	else if((val_len == 1) && (*val == '0'))
		v->bval = false;		
	else
		v->bval = false; // assume false.
		
	// Parse number
	// Supported formats:
	// prefix: 0x hex . 
	// postix: h for hex
	//		   b for bin (dual)
	if( (val_len >= 1 && (val[val_len] == 'h')) || (val_len >= 2 && (val[0] == '0' && val[1] == 'x')) ){//HEX!
			if(val[val_len] == 'h'){
				val[val_len]= '\0';
				v->intval = strtoull(val, NULL, 16);
				val[val_len] = 'h';
			}else
				v->intval = strtoull(&val[2], NULL, 16);
	}else if( val_len >= 1 && (val[val_len] == 'b') ){	//BIN
		val[val_len] = '\0';
		v->intval = strtoull(val, NULL, 2);
		val[val_len] = 'b';
	}else if( *val >='0' && *val <= '9'){	// begins with normal digit, so assume its dec.
		// is it float?
		bool is_float = false;
		char *p;
		
		for(p = val; *p != '\0'; p++){
			if(*p == '.'){
				v->floatval = strtod(val, NULL);
				v->intval = (int64) v->floatval;
				is_float = true;
				break;
			}
		}
		
		if(is_float == false){
			v->intval = strtoull(val, NULL, 10);
			v->floatval = (double) v->intval;
		}
	}else{
		// Everything else: lets use boolean for fallback
		if(v->bval == true)
			v->intval = 1;
		else
			v->intval = 0;
	}
	
	return v;	
}//end: makeValue()


static bool configParse(raconf inst,  const char *fileName){
	FILE *fp;
	char line[4096];
	char currentSection[SECTION_LEN];
	char *p;
	char c;
	int linecnt;
	size_t linelen;
	size_t currentSection_len;
	
	fp = fopen(fileName, "r");
	if(fp == NULL){
		ShowError("configParse: cannot open '%s' for reading.\n", fileName);
		return false;
	}
	

	// Start with empty section:
	currentSection[0] = '\0';
	currentSection_len = 0;
	
	// 	
	linecnt = 0;
	while(1){
		linecnt++;
		
		if(fgets(line, sizeof(line), fp) != line)
			break;

		linelen = strlen(line);
		p = line;
		
		// Skip whitespaces from beginning (space and tab)
		_line_begin_skip_whities:
		c = *p;
		if(c == ' ' || c == '\t'){
			p++; 
			linelen--;
			goto _line_begin_skip_whities;
		}
		
		// Remove linebreaks as (cr or lf) and whitespaces from line end!
		_line_end_skip_whities_and_breaks:
		c = p[linelen-1];
		if(c == '\r' || c == '\n' || c == ' ' || c == '\t'){
			p[--linelen] = '\0';
			goto _line_end_skip_whities_and_breaks;
		}
		
		// Empty line? 
		// or line starts with comment (commented out)?
		if(linelen == 0 || (p[0] == '/' && p[1] == '/') || p[0] == ';')
			continue;
		
		// Variable names can contain:
		// A-Za-z-_.0-9
		//
		// Sections start with [ .. ] (INI Style)
		//
		c = *p;
		
		// check what we have.. :)
		if(c == '['){ // got section!
			// Got Section!
			// Search for ] 
			char *start = (p+1);
			
			while(1){
				++p;
				c = *p;
				
				if(c == '\0'){
					ShowError("Syntax Error: unterminated Section name in %s:%u (expected ']')\n", fileName, linecnt);
					fclose(fp);
					return false;
				}else if(c == ']'){ // closing backet (section name termination)
					if( (p - start + 1) > (sizeof(currentSection) ) ){
						ShowError("Syntax Error: Section name in %s:%u is too large (max Supported length: %u chars)\n", fileName, linecnt, sizeof(currentSection)-1);
						fclose(fp);
						return false;
					}
					
					// Set section!
					*p = '\0'; // add termination here.
					memcpy(currentSection, start, (p-start)+1 ); // we'll copy \0, too! (we replaced the ] backet with \0.)
					currentSection_len = (p-start);
					
					break;
					
				}else if( (c >= '0' && c <= '9') || (c == '-') || (c == ' ') || (c == '_') || (c == '.') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ){
					// skip .. (allowed char / specifier)
					continue;
				}else{
					ShowError("Syntax Error: Invalid Character '%c' in %s:%u (offset %u) for Section name.\n", c, fileName, linecnt, (p-line));
					fclose(fp);
					return false;
				}
				
			}//endwhile: parse section name 
			
		
		}else if( (c >= '0' && c <= '9') || (c == '-') || (c == '_') || (c == '.') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ){
			// Got variable! 
			// Search for '=' or ':' wich termiantes the name 
			char *start = p;
			char *valuestart = NULL;
			size_t start_len;
						
			while(1){
				++p;
				c = *p;
				
				if(c == '\0'){
					ShowError("Syntax Error: unterminated Variable name in %s:%u\n", fileName, linecnt);
					fclose(fp);
					return false;
				}else if( (c == '=') || (c == ':') ){
					// got name termination
					
					*p = '\0'; // Terminate it so  (start) will hold the pointer to the name.
					
					break;
					
				}else if( (c >= '0' && c <= '9') || (c == '-') || (c == '_') || (c == '.') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ){
					// skip .. allowed char 
					continue;
				}else{
					ShowError("Syntax Error: Invalid Character '%c' in %s:%u (offset %u) for Variable name.\n", c, fileName, linecnt, (p-line));
					fclose(fp);
					return false;
				}
				
			}//endwhile: parse var name			
			
			start_len = (p-start);
			if(start_len >= VARNAME_LEN){
				ShowError("%s:%u Variable length exceeds limit of %u Characters.\n", fileName, linecnt, VARNAME_LEN-1);
				fclose(fp);
				return false;
			}else if(start_len == 0){
				ShowError("%s:%u Empty Variable name is not allowed.\n", fileName, linecnt);
				fclose(fp);
				return false;
			}
			
			
			valuestart = (p+1); 
			
			
			// Skip whitespace from begin of value (tab and space)
			_skip_value_begin_whities:
			c = *valuestart; 
			if(c == ' ' || c == '\t'){
				valuestart++;
				goto _skip_value_begin_whities;
			}
			
			// Scan for value termination, 
			// wich can be \0  or  comment start (// or ; (INI) )
			//
			p = valuestart;
			while(1){
				c = *p;
				if(c == '\0'){
					// Terminated by line end.
					break;
				}else if(c == '/' && p[1] == '/'){
					// terminated by c++ style comment.
					*p = '\0';
					break;
				}else if(c == ';'){
					// terminated by ini style comment.
					*p = '\0';
					break;
				}
				
				p++;
			}//endwhile: search var value end.
						
			
			// Strip whitespaces from end of value.
			if(valuestart != p){ // not empty!
				p--;
				_strip_value_end_whities: 
				c = *p;
				if(c == ' ' || c == '\t'){
					*p = '\0';
					p--;
					goto _strip_value_end_whities;
				}
				p++;
			}					
			
			
			// Buildin Hook:
			if( stricmp(start, "import") == 0){
				if( configParse(inst, valuestart) != true){
					ShowError("%s:%u - Import of '%s' failed!\n", fileName, linecnt, valuestart);
				}
			}else{
				// put it to db.
				struct conf_value *v, *o;
				char key[ (SECTION_LEN+VARNAME_LEN+1+1) ]; //+1 for delimiter, +1 for termination.
				size_t section_len;
				
				if(*currentSection == '\0'){ // empty / none
					strncpy(key, "<unnamed>",9);
					section_len = 9;
				}else{
					strncpy(key, currentSection, currentSection_len);
					section_len = currentSection_len;
				}
				
				key[section_len] = '.'; // Delim
				
				strncpy(&key[section_len+1],  start, start_len);
				
				key[section_len + start_len + 1] = '\0'; 

				
				v = makeValue(key, valuestart, (p-valuestart) );				
				
				// Try to get the old one before
				o = strdb_get(inst->db, key);
				if(o != NULL){
					strdb_remove(inst->db, key);
					aFree(o); //			
				}
				
				strdb_put( inst->db, key,  v);
			}								
			
			
		}else{
			ShowError("Syntax Error: unexpected Character '%c' in %s:%u (offset %u)\n", c, fileName, linecnt, (p-line) );
			fclose(fp);
			return false;
		}
		
		
		
	}
	
	
	
	fclose(fp);
	return true;
}//end: configParse()


#define MAKEKEY(dest, section, key) { size_t section_len, key_len; \
										if(section == NULL || *section == '\0'){ \
											strncpy(dest, "<unnamed>", 9); \
											section_len = 9; \
										}else{ \
											section_len = strlen(section); \
											strncpy(dest, section, section_len); \
										} \
										\
										dest[section_len] = '.'; \
										\
										key_len = strlen(key); \
										strncpy(&dest[section_len+1],  key,  key_len); \
										dest[section_len + key_len + 1] = '\0'; \
									}
										

raconf  raconf_parse(const char *file_name){
	struct raconf *rc;
	
	rc = aCalloc(1, sizeof(struct raconf) );
	if(rc == NULL){
		ShowFatalError("raconf_parse: failed to allocate memory for new handle\n");
		return NULL;
	}

	rc->db = strdb_alloc(DB_OPT_BASE | DB_OPT_DUP_KEY, 98);	
	//
	
	if(configParse(rc, file_name) != true){
		ShowError("Failed to Parse Configuration file '%s'\n", file_name);
	}
	
	return rc;
}//end: raconf_parse()


void raconf_destroy(raconf rc){
	DBIterator *iter;
	struct conf_value *v;
	
	// Clear all entrys in db.
	iter = db_iterator(rc->db);
	for( v = (struct conf_value*)dbi_first(iter);  dbi_exists(iter);  v = (struct conf_value*)dbi_next(iter) ){
		aFree(v);
	}
	dbi_destroy(iter);

	db_destroy(rc->db);
	
	aFree(rc);	
	
}//end: raconf_destroy()

bool raconf_getbool(raconf rc, const char *section, const char *key,  bool _default){
	char keystr[SECTION_LEN + VARNAME_LEN + 1 + 1];
	struct conf_value *v;
	
	MAKEKEY(keystr, section, key);

	v = strdb_get(rc->db, keystr);
	if(v == NULL)
		return _default;
	else
		return v->bval;
}//end: raconf_getbool()


float raconf_getfloat(raconf rc,const char *section, const char *key, float _default){
	char keystr[SECTION_LEN + VARNAME_LEN + 1 + 1];
	struct conf_value *v;
	
	MAKEKEY(keystr, section, key);

	v = strdb_get(rc->db, keystr);
	if(v == NULL)
		return _default;
	else
		return (float)v->floatval;
}//end: raconf_getfloat()


int64 raconf_getint(raconf rc,  const char *section, const char *key, int64 _default){
	char keystr[SECTION_LEN + VARNAME_LEN + 1 + 1];
	struct conf_value *v;
	
	MAKEKEY(keystr, section, key);
	
	v = strdb_get(rc->db, keystr);
	if(v == NULL)
		return _default;
	else
		return v->intval;

}//end: raconf_getint()


const char* raconf_getstr(raconf rc,  const char *section, const char *key, const char *_default){
	char keystr[SECTION_LEN + VARNAME_LEN + 1 + 1];
	struct conf_value *v;

	MAKEKEY(keystr, section, key);	

	v = strdb_get(rc->db, keystr);
	if(v == NULL)
		return _default;
	else
		return v->strval;
}//end: raconf_getstr()


bool raconf_getboolEx(raconf rc, const char *section, const char *fallback_section, const char *key, bool _default){
	char keystr[SECTION_LEN + VARNAME_LEN + 1 + 1];
	struct conf_value *v;
	
	MAKEKEY(keystr, section, key);
	v = strdb_get(rc->db, keystr);
	if(v == NULL){
		
		MAKEKEY(keystr, fallback_section, key);
		v = strdb_get(rc->db, keystr);
		if(v == NULL){
			return _default;
		}else{
			return v->bval;
		}
		
	}else{
		return v->bval;
	}
}//end: raconf_getboolEx()


float raconf_getfloatEx(raconf rc,const char *section, const char *fallback_section, const char *key, float _default){
	char keystr[SECTION_LEN + VARNAME_LEN + 1 + 1];
	struct conf_value *v;
	
	MAKEKEY(keystr, section, key);
	v = strdb_get(rc->db, keystr);
	if(v == NULL){
		
		MAKEKEY(keystr, fallback_section, key);
		v = strdb_get(rc->db, keystr);
		if(v == NULL){
			return _default;
		}else{
			return (float)v->floatval;
		}
		
	}else{
		return (float)v->floatval;
	}
	
}//end: raconf_getfloatEx()


int64 raconf_getintEx(raconf rc,  const char *section, const char *fallback_section, const char *key, int64 _default){
	char keystr[SECTION_LEN + VARNAME_LEN + 1 + 1];
	struct conf_value *v;
	
	MAKEKEY(keystr, section, key);
	v = strdb_get(rc->db, keystr);
	if(v == NULL){
		
		MAKEKEY(keystr, fallback_section, key);
		v = strdb_get(rc->db, keystr);
		if(v == NULL){
			return _default;
		}else{
			return v->intval;
		}
		
	}else{
		return v->intval;
	}

}//end: raconf_getintEx()


const char* raconf_getstrEx(raconf rc,  const char *section, const char *fallback_section, const char *key, const char *_default){
	char keystr[SECTION_LEN + VARNAME_LEN + 1 + 1];
	struct conf_value *v;
	
	MAKEKEY(keystr, section, key);
	v = strdb_get(rc->db, keystr);
	if(v == NULL){
		
		MAKEKEY(keystr, fallback_section, key);
		v = strdb_get(rc->db, keystr);
		if(v == NULL){
			return _default;
		}else{
			return v->strval;
		}
		
	}else{
		return v->strval;
	}

}//end: raconf_getstrEx()
