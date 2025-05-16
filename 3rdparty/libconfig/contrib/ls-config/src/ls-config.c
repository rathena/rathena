#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <libintl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <libconfig.h>

#define PACKAGE    "LS bash config"
#define VERSION    "1.0.3"

// global flags
struct flags {
	int quiet; //quiet output
	int names; //set for printout config variables names
	int types; //set for printout config variables types
	int values; //set for printout config variables values
	int indexes; //set for printout config variables indexes
	int counter; //set for printout config varibales counting (for grout, list, array. in other cases it return 1)
	int unset; //unset valriable
	int boolstring; //set for output bool variable (0|1) as test (false|true)
	int mode; //1 - for setting variable, 0 - for get hist data
	int error; //error status handling
};

//take valur from input and comvert it to int
//TODO: Read long too
int getNumber() {
  char buf[1000];
  int test,val;
  unsigned int inp;
  fgets(buf, sizeof buf, stdin);
  test = sscanf(buf, "%u", &inp);
  val = (int) inp;
  if(val < 0) val *= -1;
  if(test > 0) return val;
  return (int) 0;
}

//printout help messsage
void printHelp() {
	printf(gettext("Configuration file handling\n"));
	printf("\n");
	printf(gettext("Usage: ls-config [OPTION]\n"));
	printf(gettext("Reading and writening data from configuration files\n"));
	printf(gettext("in libconfig9 format.\n"));
	printf("\n");
	printf(gettext("CAUTION: using without given config file are cause error!\n"));
	printf("\n");
	printf(gettext("Available options:\n"));
	printf(gettext("   -f, --file=FILE       Configuration file to handle.\n"));
	printf("\n");
	printf(gettext("   -s, --set=PATH        Set configuration variable of given path.\n"));
	printf(gettext("   -d, --data=DATA       Configuration variable value (only with -s)\n"));
	printf(gettext("   -p, --type=TYPE       Configuration value type\n"));
	printf("\n");
	printf(gettext("   -g, --get=PATH        Get configuration variable of given path.\n"));
	printf(gettext("   -n, --names           Printout variables names.\n"));
	printf(gettext("   -t, --types           Printout variables types.\n"));
	printf(gettext("   -v, --values          Printout variables values.\n"));
	printf(gettext("   -i, --indexes         Printout variables indexes.\n"));
	printf(gettext("   -c, --count           Printout elements count (only: array, list, group).\n"));
	printf(gettext("   -b, --bool-string     Printout boolean variables as text.\n"));
	printf("\n");
	printf(gettext("   -q, --quiet           Quiet output to use in scripts.\n"));
	printf(gettext("   -h, --help            Print this help message.\n"));
	printf("\n");
	printf(gettext("TYPE:    Variable types:\n"));
	printf(gettext("         group  - variables group,\n"));
	printf(gettext("         array  - array of variables,\n"));
	printf(gettext("         list   - list of variables,\n"));
	printf(gettext("         int    - integer number,\n"));
	printf(gettext("         int64  - 64bit integer number,\n"));
	printf(gettext("         float  - float point number,\n"));
	printf(gettext("         bool   - boolean value,\n"));
	printf(gettext("         string - character string.\n"));
	printf("\n");
	printf("(c) 2013 by LucaS web sutio - http://www.lucas.net.pl\n");
	printf("Author: Łukasz A. Grabowski\n");
   printf(gettext("Licence: "));
	printf("GPL v2.\n");
   exit(0);
};

//set configuration int value
int set_config_int(config_setting_t *setting, char *dataString, struct flags optflags) {
	long bufl; //int (long) to get from input data string
	int buf, scs; //config int, success status
	char *erp; //error output
	
	//convert input data to int
	errno = 0;
	bufl = strtol(dataString, &erp, 0);
	if(((errno == ERANGE && (bufl == LONG_MAX || bufl == LONG_MIN)) || (errno != 0 && bufl == 0)) || (erp == dataString) || bufl > INT_MAX || bufl < INT_MIN) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Incorrect data format.\n"));
  			return 12;
		};
	buf = (int)bufl;

	//set configuration variable
	scs = config_setting_set_int(setting, buf);
	if(scs == CONFIG_FALSE) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
		return 11;
	};	
	return 0;
};

//set configuration int64 value
int set_config_int64(config_setting_t *setting, char *dataString, struct flags optflags) {
	long bufl; //long to get from input data string
	int scs; //success status
	char *erp; //error output

	//convert input data to long
	errno = 0;
	bufl = strtol(dataString, &erp, 0);
	if(((errno == ERANGE && (bufl == LONG_MAX || bufl == LONG_MIN)) || (errno != 0 && bufl == 0)) || (erp == dataString)) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Incorrect data format.\n"));
			return 12;
		};

	//set configuration variable
	scs = config_setting_set_int64(setting, bufl);
	if(scs == CONFIG_FALSE) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
		return 11;
	};	
	return 0;
};

//set configuration float value
int set_config_float(config_setting_t *setting, char *dataString, struct flags optflags) {
	double buff; //double (float) to get from input data string
	int scs; //success status
	char *erp; //error output

	//convert input data to double
	errno = 0;
	buff = strtod(dataString, &erp);
   if(((errno == ERANGE && (buff == HUGE_VALF || buff == HUGE_VALL)) || (errno != 0 && buff == 0)) || (erp == dataString)) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Incorrect data format.\n"));
		return 12;
	}

	//set configuration variable
	scs = config_setting_set_float(setting, buff);
	if(scs == CONFIG_FALSE) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
		return 11;
	};	
	return 0;
};

//set configuration boolean value
int set_config_bool(config_setting_t *setting, char *dataString, struct flags optflags) {
	int scs, buf; //success status, input convert burrer

	//convert input data
	//chceck both 1/0 and true/false string
	buf = -1;
	if(!strcmp(dataString, "1") || !strcmp(dataString, "true") || !strcmp(dataString, "TRUE")) buf = 1;
	if(!strcmp(dataString, "0") || !strcmp(dataString, "false") || !strcmp(dataString, "FALSE")) buf = 0;
	if(buf < 0) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Incorrect data format.\n"));
		return 12;
	}

	//set configuration variable
	scs = config_setting_set_bool(setting, buf);
	if(scs == CONFIG_FALSE) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
		return 11;
	};	
	return 0;
};

//configuratnion variable path look like: foo.bar.car
//this fuction return string giving path to parent element (foo.bar)
char* path_parent(char *dataPath) {
	char *str_ptr, *last_ptr, *newpath, *dot="."; //tokenized buffer, last buffer, new parent path, separator
	newpath = malloc(1);
	memset(newpath, 0, 1);
	last_ptr = malloc(1);
	memset(last_ptr, 0, 1);
	
	//tokenize string and save last token
	str_ptr = strtok(dataPath, ".");
	last_ptr = (char*)realloc(last_ptr, (strlen(str_ptr)+1)*sizeof(char));
	strcpy(last_ptr, str_ptr);

	//loop overt path to build new path without last element
	while(str_ptr != NULL) {
		str_ptr = strtok(NULL, ".");
		if(str_ptr != NULL) {
			if(strlen(last_ptr) > 0 ) {
				newpath = (char*)realloc(newpath, (strlen(newpath)+strlen(last_ptr)+2)*sizeof(char));
				strcat(newpath, dot);
				strcat(newpath, last_ptr);
			};
			last_ptr = (char*)realloc(last_ptr, (strlen(str_ptr)+1)*sizeof(char));
			strcpy(last_ptr, str_ptr);
		} else {
			last_ptr = (char*)realloc(last_ptr, (1)*sizeof(char));
			memset(last_ptr, 0, 1);
		};
  	};
	free(dataPath);

	//if new path empty thren return null
	if(strlen(newpath) == 0) {
		free(newpath);
		newpath = NULL;
	};
	return newpath;
};

//get element name from configuration variable path
//e.g.: from foo.bar return bar
char* path_name(char *dataPath) {
	char *str_ptr, *name, *tk; //tokenized buffer, element name, copy of dataPath
	name = malloc(1);

	//make copy of dataPath
	tk = malloc((strlen(dataPath)+1)*sizeof(char));
	memset(name, 0, 1);
	strcpy(tk, dataPath);

	//tokenize dataPath
	str_ptr = strtok(tk, ".");
	
	//loop over tokenize pathh to get last element
	while(str_ptr != NULL) {
		name = (char*)realloc(name, (strlen(str_ptr)+1)*sizeof(char));
		strcpy(name, str_ptr);
		str_ptr = strtok(NULL, ".");
  	};
	free(tk);

	//if no element name then return null
	if(strlen(name) == 0) {
		free(name);
		name = NULL;
	};
	return name;
};

//set configuration path
//@return int success
//@param configFile - name (with path) of configuration fille
//@param dataPath - path of configuration variable (in config file)
//@param optflags - global options flags
//@param dataString - data to store in configuration variable in string format
//@param dataType - type of variable to save
int set_config(char *configFile, char *dataPath, struct flags optflags, char *dataString, char *dataType) {
	config_t cfg; //libcongig configuration handler
	config_setting_t *setting, *ss; //libconfig element handrer: mant, and subset (uset for multielement types)
	config_init(&cfg);
	int scs, dt, dattyp; //sucess statu, data type
	char *npath; // new variable configuration path path

	//open and read configuration file
	if(!config_read_file(&cfg, configFile)) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Can't read configuration file.\n"));
  		return 1;
 	};

	//if no data path or data string then cause error
	if(dataPath == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Conviguration variable path not given.\n"));
  		return 4;
	};
	if(dataString == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Configuration variable value not given.\n"));
  		return 9;
	};

 	//find configuration variable of given path
	setting = config_lookup(&cfg, dataPath);
	if(setting == NULL) {
		//if variable of given path not found get element name and partent path, 
		//then try to create it
		npath = path_name(dataPath);
		dataPath = path_parent(dataPath);
		if(dataPath == NULL) {		
			setting = config_root_setting(&cfg);
		} else {
			setting = config_lookup(&cfg, dataPath);
		};
		if(setting == NULL) {
			//if parent not exists exit with error
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("ERROR! Inavlid configuration variable path.\n"));
  			return 16;
		};
		//chceck type of parent element (named alement can be added only to group element)
		dt = config_setting_type(setting);
		if(dt != CONFIG_TYPE_GROUP) {
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("ERROR! New named configuration variable can be added only to group element.\n"));
  			return 17;
		};
		//check if new element type are given
		if(dataType == NULL) {
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("ERROR! Configuration variable type not given.\n"));
  			return 13;
		};
		//now get type based on his name
		if(!strcmp(dataType, "int")) {
			dattyp = CONFIG_TYPE_INT;
		} else if(!strcmp(dataType, "int64")) {
			dattyp = CONFIG_TYPE_INT64;
		} else if(!strcmp(dataType, "float")) {
			dattyp = CONFIG_TYPE_FLOAT;
		} else if(!strcmp(dataType, "string")) {
			dattyp = CONFIG_TYPE_STRING;
		} else if(!strcmp(dataType, "bool")) {
			dattyp = CONFIG_TYPE_BOOL;
		} else if(!strcmp(dataType, "array")) {
			dattyp = CONFIG_TYPE_ARRAY;
		} else if(!strcmp(dataType, "list")) {
			dattyp = CONFIG_TYPE_LIST;
		} else if(!strcmp(dataType, "group")) {
			dattyp = CONFIG_TYPE_GROUP;
		} else {
			//if given type no mutch eny then cause error and exit
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("ERROR! Inlegal data type.\n"));
  			return 14;
		};
		//add new element to configuration file
		ss = config_setting_add(setting, npath, dattyp);
		if(ss == NULL) {
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  			return 11;
		};
		scs = 0;
		//and based on new type set his value
		switch(dattyp) {
			case CONFIG_TYPE_INT:
				scs = set_config_int(ss, dataString, optflags);
				break;
			case CONFIG_TYPE_INT64:
				scs = set_config_int64(ss, dataString, optflags);
				break;
			case CONFIG_TYPE_FLOAT:
				scs = set_config_float(ss, dataString, optflags);
				break;
			case CONFIG_TYPE_STRING:
				scs = config_setting_set_string(ss, dataString);
				if(scs == CONFIG_FALSE) {
					if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  					scs = 11;
				} else scs = 0;
				break;
			case CONFIG_TYPE_BOOL:
				scs = set_config_bool(ss, dataString, optflags);
				break;
		};
		if(scs > 0) {
			//if occurs some error wihe setting variable value exit with error
			config_destroy(&cfg);
			return scs;
      };
	} else {
		//but if we found element of given path, try to set his value
		//first of all determinate type of value
		dt = config_setting_type(setting);
		switch(dt) {
			case CONFIG_TYPE_INT:
				if(dataType != NULL && strcmp(dataType, "int")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  					return 10;
				};	
				//then set value
				scs = set_config_int(setting, dataString, optflags);
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};	
				break;
			case CONFIG_TYPE_INT64:
				if(dataType != NULL && strcmp(dataType, "int64")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  					return 10;
				};	
				//then set value
				scs = set_config_int64(setting, dataString, optflags);
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};	
				break;
			case CONFIG_TYPE_FLOAT:
				if(dataType != NULL && strcmp(dataType, "float")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  					return 10;
				};	
				//then set value
				scs = set_config_float(setting, dataString, optflags);
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};	
				break;
			case CONFIG_TYPE_STRING:
				if(dataType != NULL && strcmp(dataType, "string")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  					return 10;
				};	
				//then set value
				scs = config_setting_set_string(setting, dataString);
				if(scs == CONFIG_FALSE) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  					return 11;
				};	
				break;
			case CONFIG_TYPE_BOOL:
				if(dataType != NULL && strcmp(dataType, "bool")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  					return 10;
				};
				//then set value
				scs = set_config_bool(setting, dataString, optflags);
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};	
				break;
			case CONFIG_TYPE_ARRAY:
				//if array are empty we can set alement of any scalar type
				if(config_setting_length(setting) == 0) {
					//but we must have his type
					if(dataType == NULL) {	
						config_destroy(&cfg);
						if(optflags.quiet == 0) printf(gettext("ERROR! Configuration variable type not given.\n"));
  						return 13;
					};
					if(!strcmp(dataType, "int")) {
						dattyp = CONFIG_TYPE_INT;
					} else if(!strcmp(dataType, "int64")) {
						dattyp = CONFIG_TYPE_INT64;
					} else if(!strcmp(dataType, "float")) {
						dattyp = CONFIG_TYPE_FLOAT;
					} else if(!strcmp(dataType, "string")) {
						dattyp = CONFIG_TYPE_STRING;
					} else if(!strcmp(dataType, "bool")) {
						dattyp = CONFIG_TYPE_BOOL;
					} else {
						//only scalar type availabe
						config_destroy(&cfg);
						if(optflags.quiet == 0) printf(gettext("ERROR! Prohibited data type.\n"));
  						return 18;
					};
					//first of all we must add new element to array
					ss = config_setting_add(setting, NULL, dattyp);
					if(ss == NULL) {
						config_destroy(&cfg);
						if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  						return 11;
					};
					//then based on his type set value
					switch(dattyp) {
						case CONFIG_TYPE_INT:
							scs = set_config_int(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
  								return scs;
							};	
							break;
						case CONFIG_TYPE_INT64:
							scs = set_config_int64(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
  								return scs;
							};	
							break;
						case CONFIG_TYPE_FLOAT:
							scs = set_config_float(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
  								return scs;
							};	
							break;
						case CONFIG_TYPE_STRING:
							scs = config_setting_set_string(ss, dataString);
							if(scs == CONFIG_FALSE) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  								return 11;
							};
							break;
						case CONFIG_TYPE_BOOL:
							scs = set_config_bool(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
  								return scs;
							};	
							break;
					};
				} else {
					//but if we have some element in array, we can add only element of same type
					//so, because all element in arry must be same type, we get type of first element
					//and based on it set new element
					dattyp = config_setting_type(config_setting_get_elem(setting, 0));
					switch(dattyp) {
						case CONFIG_TYPE_INT:
							if(dataType != NULL && strcmp(dataType, "int")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  								return 10;
							};
							//add new element
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  								return 11;
							};
							//then set his value
							scs = set_config_int(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
								return scs;
							};
							break;
						case CONFIG_TYPE_INT64:
							if(dataType != NULL && strcmp(dataType, "int64")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  								return 10;
							};
							//add new element
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  								return 11;
							};
							//then set his value
							scs = set_config_int64(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
								return scs;
							};
							break;
						case CONFIG_TYPE_FLOAT:
							if(dataType != NULL && strcmp(dataType, "float")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  								return 10;
							};
							//add new element
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  								return 11;
							};
							//then set his value
							scs = set_config_float(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
								return scs;
							};
							break;
						case CONFIG_TYPE_STRING:
							if(dataType != NULL && strcmp(dataType, "string")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  								return 10;
							};
							//add new element
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  								return 11;
							};
							//then set his value
							scs = config_setting_set_string(ss, dataString);
							if(scs == CONFIG_FALSE) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  								return 11;
							};
							break;
						case CONFIG_TYPE_BOOL:
							if(dataType != NULL && strcmp(dataType, "bool")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! inconsistent value type.\n"));
  								return 10;
							};
							//add new element
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  								return 11;
							};
							//then set his value
							scs = set_config_bool(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
								return scs;
							};
							break;
					};
				};
				break;
			case CONFIG_TYPE_LIST:
				//in case adding element to list, we can add any type of element
				if(dataType == NULL) {
					//but we must konwn his type
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! Configuration variable type not given.\n"));
  					return 13;
				};
				if(!strcmp(dataType, "int")) {
					dattyp = CONFIG_TYPE_INT;
				} else if(!strcmp(dataType, "int64")) {
					dattyp = CONFIG_TYPE_INT64;
				} else if(!strcmp(dataType, "float")) {
					dattyp = CONFIG_TYPE_FLOAT;
				} else if(!strcmp(dataType, "string")) {
					dattyp = CONFIG_TYPE_STRING;
				} else if(!strcmp(dataType, "bool")) {
					dattyp = CONFIG_TYPE_BOOL;
				} else if(!strcmp(dataType, "array")) {
					dattyp = CONFIG_TYPE_ARRAY;
				} else if(!strcmp(dataType, "list")) {
					dattyp = CONFIG_TYPE_LIST;
				} else if(!strcmp(dataType, "group")) {
					dattyp = CONFIG_TYPE_GROUP;
				} else {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! Inlegal data type.\n"));
  					return 14;
				};
				//add new element of given type
				ss = config_setting_add(setting, NULL, dattyp);
				if(ss == NULL) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  					return 11;
				};
				//now, based on type, set element value
				scs = 0;
				switch(dattyp) {
					case CONFIG_TYPE_INT:
						scs = set_config_int(ss, dataString, optflags);
						break;
					case CONFIG_TYPE_INT64:
						scs = set_config_int64(ss, dataString, optflags);
						break;
					case CONFIG_TYPE_FLOAT:
						scs = set_config_int64(ss, dataString, optflags);
						break;
					case CONFIG_TYPE_STRING:
						scs = config_setting_set_string(ss, dataString);
						if(scs == CONFIG_FALSE) {
							config_destroy(&cfg);
							if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  							return 11;
						};
						scs = 0;
						break;
					case CONFIG_TYPE_BOOL:
						scs = set_config_int64(ss, dataString, optflags);
						break;
				};
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};
				//finaly outpt index of new added element
				if(optflags.quiet == 0) {
					printf(gettext("Added element index: %d\n"), config_setting_index(ss));
				} else {
					printf("%d", config_setting_index(ss));
				};
				break;
			case CONFIG_TYPE_GROUP:
				//to group we can add any type of element, but we must have his name
				if(dataType == NULL) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! Configuration variable type not given.\n"));
  					return 13;
				};
				if(strlen(dataString) < 1) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! Bad name of configuration variable.\n"));
  					return 15;
				};
				//determinate type of new variable
				if(!strcmp(dataType, "int")) {
					dattyp = CONFIG_TYPE_INT;
				} else if(!strcmp(dataType, "int64")) {
					dattyp = CONFIG_TYPE_INT64;
				} else if(!strcmp(dataType, "float")) {
					dattyp = CONFIG_TYPE_FLOAT;
				} else if(!strcmp(dataType, "string")) {
					dattyp = CONFIG_TYPE_STRING;
				} else if(!strcmp(dataType, "bool")) {
					dattyp = CONFIG_TYPE_BOOL;
				} else if(!strcmp(dataType, "array")) {
					dattyp = CONFIG_TYPE_ARRAY;
				} else if(!strcmp(dataType, "list")) {
					dattyp = CONFIG_TYPE_LIST;
				} else if(!strcmp(dataType, "group")) {
					dattyp = CONFIG_TYPE_GROUP;
				} else {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! Inlegal data type.\n"));
  					return 14;
				};
				//then add new alement
				ss = config_setting_add(setting, dataString, dattyp);
				if(ss == NULL) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("ERROR! Variable set failed.\n"));
  					return 11;
				};
				//in case of adding new element to group we not set his value 
				//(value field of input are used to get variable name)
				//We only output index of new added element
				if(optflags.quiet == 0) {
					printf(gettext("Added element index: %d\n"), config_setting_index(ss));
				} else {
					printf("%d", config_setting_index(ss));
				};
				break;
		};
	}

	//Finaly write configuration file
	scs = config_write_file(&cfg, configFile);
	if(scs == CONFIG_FALSE) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Configuration file write failed.\n"));
  		return 8;
 	};
 	config_destroy(&cfg);
	return 0;
};

//unset configuration path
//(remove variable from configuration file)
//@return int success
//@param char* configFile - the name (with path) of configuration file
//@param char* configPath - path to configuration valriable to remove (unset)
//@param struct flags optflags - global flags
int unset_config(char *configFile, char *dataPath, struct flags optflags) {
	config_t cfg; //configuration file handler
	config_setting_t *setting, *par; //configuration valriale handler, and paren variable handler
	int idx, scs; //index of variable, sucess status
	//open configuration file
	config_init(&cfg);
	if(!config_read_file(&cfg, configFile)) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Can't read configuration file.\n"));
  		return 1;
 	};
	//chceck if data path given
	if(dataPath == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Conviguration variable path not given.\n"));
  		return 4;
	};
	//now find variable of given path
	setting = config_lookup(&cfg, dataPath);
	if(setting == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Given variable path not found.\n"));
  		return 3;
 	};
	//get element index
	idx = config_setting_index(setting);
	if(idx < 0) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Can't remove root element.\n"));
  		return 5;
 	};
	//now find parent element
	par = config_setting_parent(setting);
	if(par == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Can't find parent element.\n"));
  		return 6;
 	};
	//then remove element
	scs = config_setting_remove_elem(par, idx);
	if(scs == CONFIG_FALSE) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Variable unset failed.\n"));
  		return 7;
 	};
	//Finaly write configuration file
	scs = config_write_file(&cfg, configFile);
	if(scs == CONFIG_FALSE) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Configuration file write failed.\n"));
  		return 8;
 	};
 	config_destroy(&cfg);
	return 0;
};

//get configuratioin variable
//(read it from configuration file)
//@return char* variable value
//@param char* configFile - configuration file name (with path)
//@param cher* dataPath - configuration variable path (in file)
//@param struct flags optflags - global flags
int read_config(char *configFile, char *dataPath, struct flags optflags) {
	config_t cfg; //configuration file handler
	config_setting_t *setting, *ss; //configuration element handler, and helper handler (config element too)
	int comaset, varindex, varcounter; //helper flat for buid output strings, varibale index, counter
	unsigned int maxel, i; //max elements, and loop index
	char buffer[256]; //reading buffer
	const char *cbuffer;	
	const char *coma=";"; //output string variable separator
	int ibuffer, ssize; //value int buffer
	char *dataName, *dataTypeName, *dataValueString; //name of variable, type of variable, value of variable
	int dataType, st; //internale variable type
	//initialize values
	dataValueString = malloc(1);
	dataTypeName = malloc(1);
	memset(dataValueString, 0, 1);
	memset(dataTypeName, 0, 1);
	varindex = 0;
	varcounter = 0;
	//open and read configuration file
	config_init(&cfg);
	if(!config_read_file(&cfg, configFile)) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Can't read configuration file.\n"));
  		return 1;
 	};
	//now find variable element of given path
	if(dataPath == NULL) {
		//if path not givne load root element (default)
		setting = config_root_setting(&cfg);
	} else {
		setting = config_lookup(&cfg, dataPath);
	};
	if(setting == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("ERROR! Given variable path not found.\n"));
  		return 3;
 	};
	//read variable name
	dataName = config_setting_name(setting);
	if(dataName == NULL) dataName = "NULL"; //in case variable have no name convert to string representation
	//read variable type
	dataType = config_setting_type(setting);	
	//next conver type to human readable and read variable value based on his type
	//and in cases in type not scalar read index and coutn variables
	switch(dataType) {
		case CONFIG_TYPE_INT:
			dataTypeName = (char*)realloc(dataTypeName, 4*sizeof(char));
			strcpy(dataTypeName, "int");
			sprintf(buffer, "%d", config_setting_get_int(setting));
			dataValueString = (char*)realloc(dataValueString, (strlen(buffer)+1)*sizeof(char));
			strcpy(dataValueString, buffer);
			break;
		case CONFIG_TYPE_INT64:
			dataTypeName = (char*)realloc(dataTypeName, 6*sizeof(char));
			strcpy(dataTypeName, "int64");
			sprintf(buffer, "%lld", config_setting_get_int64(setting));
			dataValueString = (char*)realloc(dataValueString, (strlen(buffer)+1)*sizeof(char));
			strcpy(dataValueString, buffer);
			break;
		case CONFIG_TYPE_FLOAT:
			dataTypeName = (char*)realloc(dataTypeName, 9*sizeof(char));
			strcpy(dataTypeName, "float");
			sprintf(buffer, "%f", config_setting_get_float(setting));
			dataValueString = (char*)realloc(dataValueString, (strlen(buffer)+1)*sizeof(char));
			strcpy(dataValueString, buffer);
			break;
		case CONFIG_TYPE_STRING:
			dataTypeName = (char*)realloc(dataTypeName, 7*sizeof(char));
			strcpy(dataTypeName, "string");
			cbuffer = config_setting_get_string(setting);
			dataValueString = (char*)realloc(dataValueString, (strlen(cbuffer)+1)*sizeof(char));
			strcpy(dataValueString, cbuffer);
			break;
		case CONFIG_TYPE_BOOL:
			dataTypeName = (char*)realloc(dataTypeName, 5*sizeof(char));
			strcpy(dataTypeName, "bool");
			if(optflags.boolstring == 1) {
				//if expect bool as string, convert it to human readable
				ibuffer = config_setting_get_bool(setting); 
				if(ibuffer == CONFIG_TRUE) {
					dataValueString = (char*)realloc(dataValueString, 5*sizeof(char));
					strcpy(dataValueString, "true");
				} else {
					dataValueString = (char*)realloc(dataValueString, 6*sizeof(char));
					strcpy(dataValueString, "false");
				}
			} else {
					//else output as digit
					sprintf(buffer, "%d", config_setting_get_bool(setting));
					dataValueString = (char*)realloc(dataValueString, (strlen(buffer)+1)*sizeof(char));
					strcpy(dataValueString, buffer);
			};
			break;
		case CONFIG_TYPE_ARRAY:
			dataTypeName = (char*)realloc(dataTypeName, 6*sizeof(char));
			strcpy(dataTypeName, "array");
			//get element count 
			maxel = (unsigned int)config_setting_length(setting);
			comaset = 0;
			//and loop over all elements
			for(i = 0; i < maxel; i++) {
				//get element
				ss = config_setting_get_elem(setting, i);
				if(ss != NULL) {
					st = config_setting_type(ss);
					switch(st) {
						case CONFIG_TYPE_INT:
							sprintf(buffer, "%d", config_setting_get_int(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_INT64:
							sprintf(buffer, "%lld", config_setting_get_int64(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_FLOAT:
							sprintf(buffer, "%f", config_setting_get_float(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_STRING:
							ssize = (int)strlen(config_setting_get_string(ss));

							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+ssize+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, config_setting_get_string(ss));
						break;
						case CONFIG_TYPE_BOOL:
							if(optflags.boolstring == 1) {
								ibuffer = config_setting_get_bool(ss); 
								if(ibuffer == CONFIG_TRUE) {
									//if bool must be outputed as humen readable - convert it
									dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+4+2)*sizeof(char));
									if(comaset == 1) strcat(dataValueString, coma);
									strcat(dataValueString, "true");
								} else {
									dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+5+2)*sizeof(char));
									if(comaset == 1) strcat(dataValueString, coma);
									strcat(dataValueString, "false");
								}
							} else {
								//else output as digit
								sprintf(buffer, "%d", config_setting_get_bool(ss));
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, buffer);
							};
						break;
						case CONFIG_TYPE_ARRAY:
								//if array contains array output as kwyword ARRAY
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+7)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "ARRAY");
						break;
						case CONFIG_TYPE_LIST:
								//if array contains list output as keyword LIST
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+6)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "LIST");
						break;
						case CONFIG_TYPE_GROUP:
								//if array contains group output as keywort GROUP
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+7)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "GROUP");
						break;
					};	
					comaset = 1;
				};
			};
			break;
		case CONFIG_TYPE_LIST:
			dataTypeName = (char*)realloc(dataTypeName, 5*sizeof(char));
			strcpy(dataTypeName, "list");
			//get element count
			maxel = (unsigned int)config_setting_length(setting);
			//end loop over all elements
			comaset = 0;
			for(i = 0; i < maxel; i++) {
				ss = config_setting_get_elem(setting, i);
				if(ss != NULL) {
					st = config_setting_type(ss);
					switch(st) {
						case CONFIG_TYPE_INT:
							sprintf(buffer, "%d", config_setting_get_int(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_INT64:
							sprintf(buffer, "%lld", config_setting_get_int64(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_FLOAT:
							sprintf(buffer, "%f", config_setting_get_float(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_STRING:
							ssize = (int)strlen(config_setting_get_string(ss));

							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+ssize+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, config_setting_get_string(ss));
						break;
						case CONFIG_TYPE_BOOL:
							if(optflags.boolstring == 1) {
								ibuffer = config_setting_get_bool(ss); 
								if(ibuffer == CONFIG_TRUE) {
									//if bool must be printout as humanreadable - convert it
									dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+4+2)*sizeof(char));
									if(comaset == 1) strcat(dataValueString, coma);
									strcat(dataValueString, "true");
								} else {
									dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+5+2)*sizeof(char));
									if(comaset == 1) strcat(dataValueString, coma);
									strcat(dataValueString, "false");
								}
							} else {
								//else output as int
								sprintf(buffer, "%d", config_setting_get_bool(ss));
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, buffer);
							};
						break;
						case CONFIG_TYPE_ARRAY:
								//if list contain array output as keyword ARRAY
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+7)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "ARRAY");
						break;
						case CONFIG_TYPE_LIST:
								//if list contain list output as keyword LIST
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+6)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "LIST");
						break;
						case CONFIG_TYPE_GROUP:
								//if list contain group output as keyword GROUP
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+7)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "GROUP");
						break;
					};	
					comaset = 1;
				};
			};
			break;
		case CONFIG_TYPE_GROUP:
			dataTypeName = (char*)realloc(dataTypeName, 6*sizeof(char));
			strcpy(dataTypeName, "group");
			//get elementc count
			maxel = (unsigned int)config_setting_length(setting);
			//and loop over all elements
			//but in group case, we return inside variables names
			comaset = 0;
			for(i = 0; i < maxel; i++) {
				ss = config_setting_get_elem(setting, i);
				if(ss != NULL) {
					ssize = (int)strlen(config_setting_name(ss));
					dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+ssize+2)*sizeof(char));
					if(comaset == 1) strcat(dataValueString, coma);
					strcat(dataValueString, config_setting_name(ss));
					comaset = 1;
				};
			};
			break;
	};

	//last we get readed variable index, and element count
	varindex = config_setting_index(setting);
	varcounter = config_setting_length(setting);

	//and finaly output data
	if(optflags.names == 1 && optflags.quiet == 0) printf(gettext("Variable name:           %s\n"), dataName);
	if(optflags.names == 1 && optflags.quiet == 1) printf("%s", dataName);
	if((optflags.types == 1 && optflags.quiet == 1) && optflags.names == 1) printf(":");
	if(optflags.types == 1 && optflags.quiet == 0) printf(gettext("Variable type:           %s\n"), dataTypeName);
	if(optflags.types == 1 && optflags.quiet == 1) printf("%s", dataTypeName);
	if((optflags.values == 1 && optflags.quiet == 1) && (optflags.names == 1 || optflags.types == 1)) printf(":");
	if(optflags.values == 1 && optflags.quiet == 0) printf(gettext("Variable value:          %s\n"), dataValueString);
	if(optflags.values == 1 && optflags.quiet == 1) printf("%s", dataValueString);
	if((optflags.indexes == 1 && optflags.quiet == 1) && (optflags.names == 1 || optflags.types == 1 || optflags.values == 1)) printf(":");
	if(optflags.indexes == 1 && optflags.quiet == 0) printf(gettext("Variable index:          %d\n"), varindex);
	if(optflags.indexes == 1 && optflags.quiet == 1) printf("%d", varindex);
	if((optflags.counter == 1 && optflags.quiet == 1) && (optflags.names == 1 || optflags.types == 1 || optflags.values == 1 || optflags.indexes == 1)) printf(":");
	if(optflags.counter == 1 && optflags.quiet == 0) printf(gettext("Variable elements count: %d\n"), varcounter);
	if(optflags.counter == 1 && optflags.quiet == 1) printf("%d", varcounter);
	if(optflags.quiet == 1) printf("\n");
	
	config_destroy(&cfg);
	return 0;
}

int main(int argc, char **argv) {
	//firs set locale and domain to work with internationalization
	setlocale(LC_ALL, "");
	bindtextdomain("ls-config", "/usr/share/locale");
	textdomain("ls-config");

	//then declare and init values
   int opt,test; //used for read innput: option, and testing
	int fd; //file descriptor
	char *sinp, *dataPath=NULL, *dataString=NULL, *dataType=NULL; //string input, configuration variable path, input data, variable type
	char *configFile=NULL; //config file name (with path)
   struct flags optflags = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //global flags initialize
	int excode; //program exit code
	excode = 0;

	sinp = malloc(sizeof(char) * 256);

	//long options reading
	struct option long_options[] = {
		/* These options set a flag. */
		{"quiet",   no_argument, &optflags.quiet, 1},
		{"names",   no_argument, &optflags.names, 1},
		{"types",   no_argument, &optflags.types, 1},
		{"values",   no_argument, &optflags.values, 1},
		{"indexes",   no_argument, &optflags.indexes, 1},
		{"count",   no_argument, &optflags.counter, 1},
		{"unset",   no_argument, &optflags.unset, 1},
		{"bool-string",   no_argument, &optflags.boolstring, 1},
		/* These options don't set a flag.
		 We distinguish them by their indices. */
		{"help", no_argument, 0, 'h'},
		{"set",  required_argument, 0, 's'},
		{"get",  optional_argument, 0, 'g'},
		{"data", required_argument, 0, 'd'},
		{"type", required_argument, 0, 'p'},
		{"file", required_argument, 0, 'f'},
		{0, 0, 0, 0}
	};

	//next collect all input (given as options to program)
	while(1) {
		int option_index = 0;
		opt = getopt_long (argc, argv, "qntvicubs:g:d:p:hf:", long_options, &option_index);
		
		if(opt == -1) break;

		switch (opt) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				if(long_options[option_index].flag != 0) break;
				if(strcmp(long_options[option_index].name, "set") == 0 && optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataPath = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataPath, sinp);
					};
					optflags.mode = 1;
				};
				if(strcmp(long_options[option_index].name, "get") == 0 && optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataPath = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataPath, sinp);
					}; 
					optflags.mode = 0;
				};
				if(strcmp(long_options[option_index].name, "data") == 0 && optarg) {
					test = sscanf(optarg, "%[^\n]s", sinp);
					if(test > 0) {
						dataString = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataString, sinp);
					}; 
				};
				if(strcmp(long_options[option_index].name, "type") == 0 && optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataType = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataType, sinp);
					}; 
				};
				if(strcmp(long_options[option_index].name, "file") == 0 && optarg) {
					test = sscanf(optarg, "%[^\n]s", sinp);
					if(test > 0) {
						configFile = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(configFile, sinp);
					}; 
				};
				break;
			case 'q':
				optflags.quiet = 1;
				break;
			case 'n':
				optflags.names = 1;
				break;
			case 't':
				optflags.types = 1;
				break;
			case 'v':
				optflags.values = 1;
				break;
			case 'i':
				optflags.indexes = 1;
				break;
			case 'c':
				optflags.counter = 1;
				break;
			case 'u':
				optflags.unset = 1;
				break;
			case 'b':
				optflags.boolstring = 1;
				break;
			case 's':
				if(optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataPath = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataPath, sinp);
					}; 
					optflags.mode = 1;
				};
				break;
			case 'g':
				if(optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataPath = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataPath, sinp);
					};
				}; 
				optflags.mode = 0;
				break;
			case 'd':
				if(optarg) {
					test = sscanf(optarg, "%[^\n]s", sinp);
					if(test > 0) {
						dataString = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataString, sinp);
					};
				}; 
				break;
			case 'p':
				if(optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataType = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataType, sinp);
					};
				}; 
				break;
			case 'h':
				//free input buffer and printout help message 
				free(sinp);
				printHelp(); //this function contain exit from program
				break;
			case 'f':
				test = sscanf(optarg, "%[^\n]s", sinp);
				if(test > 0) {
					configFile = (char*)malloc((strlen(sinp)+1)*sizeof(char));
					strcpy(configFile, sinp);
				}; 
				break;
			case '?':
				break;
			default:
				break;
		}
	};

	//first of all we must ensure, then configuration file are available with right access mode
	if(optflags.mode == 0 && access(configFile, R_OK) < 0) optflags.error = 1;
	if(optflags.mode == 1) {
		fd = open(configFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if(fd < 0) {
			optflags.error = 1;
 		};
		close(fd);
   };
	if(optflags.error > 0) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Can't read configuration file.\n"));
		free(sinp);
		free(configFile);
		exit(1);
   };

	//now if we want to set variable, we must have his path
   if(optflags.mode == 1 && dataPath == NULL) {
		if(optflags.quiet == 0) printf(gettext("ERROR! Conviguration variable path not given.\n"));
		free(sinp);
		free(configFile);
		exit(4);
   };

	//if no output data requested, set to default output
	if(optflags.names == 0 && optflags.types == 0 && optflags.values == 0 && optflags.indexes == 0 && optflags.counter == 0) {
		optflags.names = 1;
		optflags.types = 1;
		optflags.values = 1;
	};

	//now we invode main work of this software based on request type (set, unset of get)
	if(optflags.mode == 0) excode = read_config(configFile, dataPath, optflags);
	if(optflags.mode == 1 && optflags.unset == 1) excode = unset_config(configFile, dataPath, optflags);
	if(optflags.mode == 1 && optflags.unset == 0) excode = set_config(configFile, dataPath, optflags, dataString, dataType);

	//then finalize free resources and exit returnig excode
	free(sinp);
	free(configFile);
	exit(excode);
}

