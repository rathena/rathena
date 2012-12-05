#ifndef _MD5CALC_H_
#define _MD5CALC_H_

void MD5_String(const char * string, char * output);
void MD5_Binary(const char * string, unsigned char * output);
void MD5_Salt(unsigned int len, char * output);

#endif /* _MD5CALC_H_ */
