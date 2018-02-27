#ifndef _PARSE_PARAMETER_H
#define _PARSE_PARAMETER_H


//#include "global.h"
#define HTON2(x) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0x00FF)) 
#define NTOH2(x) HTON2(x)
#define HTON4(x) (   (( (x) << 24 ) & 0xFF000000) | (( (x) << 8 ) & 0x00FF0000) \
                   | (( (x) >> 8 ) & 0x0000FF00) | (( (x) >> 24 ) & 0x000000FF) )
#define NTOH4(x) HTON4(x)

/* Max bit/byte length of IPv6 address. */
#define IPV6_MAX_BYTELEN    16
#define IPV6_MAX_BITLEN    128


#define NULL_STRING(x)          ((NULL == x) || ('\0' == *x))
#define HAS_STRING(x)           ((NULL != x) && ('\0' != *x))
#define DIGIT_TO_CHAR(Digit)    ((char)(Digit - '0'))


#define MAX_UCHAR      ((unsigned char)((char)(-1)))
#define MIN_UCHAR      0
#define MAX_CHAR       (MAX_UCHAR >> 1)
#define MIN_CHAR       (~MAX_CHAR)

#define MAX_UNSIGNED   ((unsigned int)((int)(-1)))
#define MIN_UNSIGNED   0
#define MAX_INT        (MAX_UNSIGNED >> 1)
#define MIN_INT        (~MAX_INT)

#define MAX_USHORT     ((unsigned short)((short)(-1)))
#define MIN_USHORT     0
#define MAX_SHORT      (MAX_USHORT >> 1)
#define MIN_SHORT      (~MAX_SHORT)

#define MAX_ULONG      ((unsigned long)((long)(-1)))
#define MIN_ULONG      0
#define MAX_LONG       (MAX_ULONG >> 1)
#define MIN_LONG       (~MAX_LONG)



/* General purpose errors. */
#define GENERAL_ERROR                           -100
#define GENERAL_ERROR_NO_DATA                   ( GENERAL_ERROR - 1  )
#define GENERAL_ERROR_NOT_FOUND                 ( GENERAL_ERROR - 2  )
#define GENERAL_ERROR_ACCESS_DENIED             ( GENERAL_ERROR - 3  )
#define GENERAL_ERROR_NOT_EQUAL                 ( GENERAL_ERROR - 4  )
#define GENERAL_ERROR_ILLEGAL_VALUE             ( GENERAL_ERROR - 5  )
#define GENERAL_ERROR_CREATE_TASK            	( GENERAL_ERROR - 6  )
#define GENERAL_ERROR_NULL_POINTER            	( GENERAL_ERROR - 7  )
#define GENERAL_ERROR_DATA_AMBIG                ( GENERAL_ERROR - 8  )
#define GENERAL_ERROR_FILE_NOT_FOUND            ( GENERAL_ERROR - 9  )
#define GENERAL_ERROR_BUFFER_OVERRUN            ( GENERAL_ERROR - 10 )
#define GENERAL_ERROR_INVALID_RAPIDMARK         ( GENERAL_ERROR - 11 )
#define GENERAL_ERROR_OUT_OF_RANGE              ( GENERAL_ERROR - 12 )
#define GENERAL_ERROR_INVALID_PATH              ( GENERAL_ERROR - 13 )


/* Errors returned by the datatype conversion routines */
#define CONVERSION_ERROR_GENERAL                -400
#define CONVERSION_ERROR_INCORRECT_TYPE         ( CONVERSION_ERROR_GENERAL - 1 )
#define CONVERSION_ERROR_OVERFLOW               ( CONVERSION_ERROR_GENERAL - 2 )
#define CONVERSION_ERROR_UNDERFLOW              ( CONVERSION_ERROR_GENERAL - 3 )
#define CONVERSION_ERROR_TOO_LONG               ( CONVERSION_ERROR_GENERAL - 4 )

#define ERROR_CODE_BUFFER_SIZE 128

#define CHECK_HEX_FROMAT_STRING(para_string, address, name)

#define PARSE_NAME(str,num) (str##num)

#define PARSE_ULONG_STRING(para_string, value, name) \
do { \
  char *endptr = NULL; \
  errno = 0; \
  (value) = strtoul ((para_string), &endptr, 0); \
  if (*(para_string) == '-' || *endptr != '\0' || errno || value < 0) \
    { \
      vty_out (vty, "%% Invalid "#name" value%s", VTY_NEWLINE); \
      return CMD_SUCCESS; \
    } \
} while (0)


#define PARSE_USHORT_STRING(para_string, value, name) \
do { \
  char *endptr = NULL; \
  errno = 0; \
  (value) = strtoul ((para_string), &endptr, 0); \
  if (*(para_string) == '-' || *endptr != '\0' || errno ) \
    { \
      vty_out (vty, "%% Invalid "#name" value%s", VTY_NEWLINE); \
      return CMD_SUCCESS; \
    } \
} while (0)

#define PARSE_UCHAR_STRING(para_string, value, name) \
do { \
  char *endptr = NULL; \
  errno = 0; \
  (value) = strtoul ((para_string), &endptr, 0); \
  if (*(para_string) == '-' || *endptr != '\0' || errno) \
    { \
      vty_out (vty, "%% Invalid "#name" value%s", VTY_NEWLINE); \
      return CMD_SUCCESS; \
    } \
} while (0)


#define PARSE_CHAR_STRING(para_string, value, name) \
do { \
  char *endptr = NULL; \
  errno = 0; \
  (value) = strtoul ((para_string), &endptr, 0); \
  if (*(para_string) == '-' || *endptr != '\0' || errno ) \
    { \
      vty_out (vty, "%% Invalid "#name" value%s", VTY_NEWLINE); \
      return CMD_SUCCESS; \
    } \
} while (0)



#define PARSE_IP_STRING_COMMAND(__string_parameter__,__parameter1__,__parameter2__,__parameter3__,__parameter4__, __parameter_name__)\
	if(sscanf( __string_parameter__, "%hu.%hu.%hu.%hu", &__parameter1__,&__parameter2__,&__parameter3__,&__parameter4__) != 4 ) \
	{ \
		vty_out (vty, "  Error reading "#__parameter_name__" parameter from input, aborting%s", VTY_NEWLINE); \
		return (CMD_SUCCESS); \
	}


#define PARSE_IP_STRING(__string_parameter__,__ulong_parameter__, __parameter_name__)\
{\
    unsigned short parameter1,parameter2,parameter3,parameter4;\
    PARSE_IP_STRING_COMMAND(__string_parameter__,parameter1,parameter2,parameter3,parameter4,__parameter_name__)\
    INET_ADDR((unsigned char)parameter1,(unsigned char)parameter2,(unsigned char)parameter3,(unsigned char)parameter4,__ulong_parameter__)\
}

#define INET_ADDR(__parameter1__,__parameter2__,__parameter3__,__parameter4__,__result__)\
    __result__ = (__parameter1__ << 24) + (__parameter2__ << 16) + (__parameter3__ << 8) + (__parameter4__);





#define PARSE_MAC_STRING_COMMAND(__string_parameter__,__parameter__, __parameter_name__)\
	if(sscanf( __string_parameter__, "%02x:%02x:%02x:%02x:%02x:%02x", &__parameter__[0],&__parameter__[1],&__parameter__[2],&__parameter__[3],&__parameter__[4],&__parameter__[5]) != 6 ) \
	{ \
		vty_out (vty, "  Error, input"#__parameter_name__" parameter error, aborting%s", VTY_NEWLINE); \
		return (CMD_SUCCESS); \
	}


#define PARSE_MAC_STRING(__string_parameter__,__parameter__, __parameter_name__)\
do{\
    int para_mac[6];\
    PARSE_MAC_STRING_COMMAND(__string_parameter__,para_mac, __parameter_name__) \
    int mac_i;\
    for(mac_i = 0; mac_i< 6; mac_i++){\
        if(para_mac[mac_i]<0 || para_mac[mac_i]>0xff){\
    		vty_out (vty, "  Error, check input"#__parameter_name__" parameter value error, aborting%s", VTY_NEWLINE); \
    		return (CMD_SUCCESS); \
        }\
        __parameter__[mac_i] = para_mac[mac_i]; \
    }\
}while(0)

/*check whether the string is valid*/
/*They can't start with following characters, nor end with these ones
1)	The ASCII value from 0x00 to 0x20.
2)	The char "@" (ASCII value 0x40)
3)	The char "DEL" (ASCII value 0x7F)
4)	All the punctuation mark, including full stop ("." ASCII value 0x2E), comma ("," ASCII value 0x2C),
 question mark ("?" ASCII value 0x3F), exclamation mark ("!" ASCII value 0x21), semicolon (";" ASCII value 0x3B),
  colon (":" ASCII value0x3A ), caesura sign ("`" ASCII value 0x60), quotation mark (""" ASCII value 0x22), and so on

*/
#define LOID_PASSWD_CHECK(__string_parameter__,__parameter__, __parameter_name__)\
do{\
    if(__string_parameter__ == NULL) {\
        break;\
    }\
    UINT32    i=0,j=0;\
    UINT16    illegal_char_table[64];\
    UINT16    illegal_char_sum=0;\
    UINT16    string_len=0;\
    for (i=0; i< 0x21; i++){\
        illegal_char_table[i]=i;\
    }\
    illegal_char_table[i++]=0x40;\
    illegal_char_table[i++]=0x7f;\
    illegal_char_table[i++]=0x2e;\
    illegal_char_table[i++]=0x2c;\
    illegal_char_table[i++]=0x3f;\
    illegal_char_table[i++]=0x21;\
    illegal_char_table[i++]=0x3b;\
    illegal_char_table[i++]=0x3a;\
    illegal_char_table[i++]=0x60;\
    illegal_char_table[i]=0x22;\
    illegal_char_sum=i+1;\
    string_len=strlen(__string_parameter__);\
    for (i=0; i< string_len; i++ )  {\
        __parameter__[i] = __string_parameter__[i];\
        if ((i !=0) && (i!=(string_len-1)) ){\
            continue;\
        }\
        for (j=0;j< illegal_char_sum; j++ ){\
            if (illegal_char_table[j]==__string_parameter__[i]){\
                if (i==0){\
                    vty_out (vty, "  Error, check input"#__parameter_name__" parameter value error, aborting%s", VTY_NEWLINE); \
                    return (CMD_SUCCESS); \
                }\
                if (i==(string_len-1)) {\
                    vty_out (vty, "  Error, check input"#__parameter_name__" parameter value error, aborting%s", VTY_NEWLINE); \
                    return (CMD_SUCCESS); \
                }\
            }\
        }\
    }\
}while(0)

#define MACADDR_OCTETS  6

#define PARSE_TIME_STRING(__string_parameter__,v, __parameter_name__) \
do{\
    int n = 0;\
    n = sscanf(__string_parameter__, "%d:%d", &v.tm_hour, &v.tm_min);\
    if(0 == n){\
        vty_out(vty, "  Error, Please check "#__parameter_name__"!!! format: HH:MM%s", VTY_NEWLINE);\
        return CMD_SUCCESS;\
    }\
    if(1 == n){\
        vty_out(vty, "  Error, Please check "#__parameter_name__"!!! format: HH:MM%s", VTY_NEWLINE);\
        return CMD_SUCCESS;\
    }\
    if((v.tm_hour < 0 || v.tm_hour > 23)\
        || (v.tm_min < 0 || v.tm_min > 59)){\
        vty_out(vty, "  Error, "#__parameter_name__" invalid!%s", VTY_NEWLINE);\
        return CMD_SUCCESS;\
    }\
}while(0)

#define PARSE_DAY_STRING(__string_parameter__,v, __parameter_name__)\
do{\
    int n = 0;\
    n = sscanf(argv[2], "%d/%d/%d", &v.tm_year, &v.tm_mon, &v.tm_mday);\
    if(n != 3){\
        vty_out(vty, "  Error, "#__parameter_name__" invalid!%s", VTY_NEWLINE);\
        return CMD_SUCCESS;\
    }\
    if(v.tm_year<1970){\
        vty_out(vty, "  Error, You must input a future year!%s", VTY_NEWLINE);\
        return CMD_SUCCESS;\
    }\
    if((v.tm_mday < 0) || (v.tm_mday > 31)){\
        vty_out(vty, "  Error, "#__parameter_name__" day invalid!%s", VTY_NEWLINE);\
        return CMD_SUCCESS;\
    }\
    if((v.tm_mon < 0) || (v.tm_mon > 12)){\
        vty_out(vty, "  Error, "#__parameter_name__" month invalid!%s", VTY_NEWLINE);\
        return CMD_SUCCESS;\
    }\
}while(0)


int parseStrToByte(const char *pString, void *pChar, unsigned char convert);
int parseStrToUbyte(const char *pString, void *pUchar, unsigned char convert);
int parseStrToIP(const char *pString, void *pIPaddr, unsigned char convert);
char parseHex2Dec(char HexDigit);
char parseDec2Hex(char DecDigit) ;
unsigned char parseIsNumber(const char *pBuf);
int parseStrToMacAddr(const char *pString, void *pMacAddr, unsigned char convert);
int parseStrToInt(const char *pString, void *pInt, unsigned char convert);
int parseStrToUnsigned(const char *pString, void *pUnsigned, unsigned char convert);
int parseStrToShort(const char *pString, void *pShort, unsigned char convert);
int parseStrToUshort(const char *pString, void *pUshort, unsigned char convert);
int parseStrToLong(const char *pString, void *pLong, unsigned char convert);
int parsePrefixIpv6Addr (const char *str, struct in6_addr *addr, int *pLen);
int parseStrToUlong(const char *pString, void *pUlong, unsigned char convert);

char *parseParamErrorPrint(char *buf, int errcode);

char *convertNumToStr(int num, char *buf, int hex);

int parseStrToMacWildMask(const char *pString, void *pMacAddr, unsigned char convert);

#endif














