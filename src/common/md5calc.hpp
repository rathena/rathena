#ifndef _MD5CALC_HPP_
#define _MD5CALC_HPP_

#ifdef __cplusplus
extern "C" {
#endif

void MD5_String(const char * string, char * output);
void MD5_Binary(const char * string, unsigned char * output);
void MD5_Salt(unsigned int len, char * output);

#ifdef __cplusplus
}
#endif

#endif /* _MD5CALC_HPP_ */
