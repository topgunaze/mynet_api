/*
    stephen.liu add
    for parse command parameters
    20151102
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <vty.h>
#include "global.h"
#include "parse_parameter.h"


char ToUpper(char C)
{
    if (('a' <= C) && ('z' >= C))
        return ((C - ((char)'a')) + ((char)'A'));

    return C;
}

extern unsigned char
ISDIGIT ( int c )
{
    return (('0' <= c) && (c <= '9')) ? TRUE : FALSE;

}

extern unsigned char
ISSPACE ( int c )
{
    return (c == 0x20) ? TRUE : FALSE;
}



/*-----------------------------------------------------------------------*/

#define SignCheck(MinValue, MaxValue, Signed, Value, NextDigit, DataType)                        \
{                                                                                               \
    if (FALSE == Signed)                                                                        \
    {                                                                                           \
        if ((unsigned DataType)Value > ((unsigned DataType)MaxValue / 10))                      \
            return CONVERSION_ERROR_OVERFLOW;                                                   \
                                                                                                \
        if ((unsigned DataType)Value == ((unsigned DataType)MaxValue / 10))                     \
            if ((unsigned DataType)DIGIT_TO_CHAR(NextDigit) > ((unsigned DataType)MaxValue % 10)) \
                return CONVERSION_ERROR_OVERFLOW;                                               \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        if ((unsigned DataType)Value > ((unsigned DataType)MinValue / 10))                      \
            return CONVERSION_ERROR_UNDERFLOW;                                                  \
                                                                                                \
        if ((unsigned DataType)Value == ((unsigned DataType)MinValue / 10))                     \
            if ((unsigned DataType)DIGIT_TO_CHAR(NextDigit) > ((unsigned DataType)MinValue % 10)) \
                return CONVERSION_ERROR_UNDERFLOW;                                              \
    }                                                                                           \
}

/*-----------------------------------------------------------------------*/

#define UnsignCheck(MaxValue, Value, NextDigit, DataType)                                    \
{                                                                                           \
    if ((unsigned DataType)Value > ((unsigned DataType)MaxValue / 10))                      \
        return CONVERSION_ERROR_OVERFLOW;                                                   \
                                                                                            \
    if ((unsigned DataType)Value == ((unsigned DataType)MaxValue / 10))                     \
        if (DIGIT_TO_CHAR(NextDigit) > ((unsigned DataType)MaxValue % 10))                    \
            return CONVERSION_ERROR_OVERFLOW;                                               \
}

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/

int parseStrToByte(const char *pString, void *pChar, unsigned char convert)
{
    signed char     Char = 0;
    char           Digit;
    unsigned char         Signed = FALSE;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    if ('-' == *pString)
    {
        Signed = TRUE;
        pString++;
    }
    else if ('+' == *pString)
        pString++;

    if ('\0' == *pString)
        return GENERAL_ERROR_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        SignCheck(MIN_CHAR, MAX_CHAR, Signed, Char, Digit, char);

        Char *= 10;
        Char = (char)(Char + DIGIT_TO_CHAR(Digit));

        pString++;
    }

    if (convert)
        *((char *) pChar) = (char) ((Signed == TRUE) ? ((char)((~Char)+1)) : ((char)Char));

    return RT_OK;

}   /* parseStrToByte */

/*-----------------------------------------------------------------------*/

int parseStrToUbyte(const char *pString, void *pUchar, unsigned char convert)
{
    unsigned char       Uchar = 0;
    char               Digit;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        UnsignCheck(MAX_UCHAR, Uchar, Digit, char);

        Uchar *= 10;
        Uchar = (unsigned char)(Uchar + DIGIT_TO_CHAR(Digit));

        pString++;
    }

    if (convert)
    {
        if (NULL == pUchar)
            return GENERAL_ERROR_NULL_POINTER;

        *((unsigned char *) pUchar) = Uchar;        
    }

    return RT_OK;
}

/*-----------------------------------------------------------------------*/

/* sample conversion code */

int parseStrToIP(const char *pString, void *pIPaddr, unsigned char convert)
{
    unsigned char  Uchar = 0;
    char          Digit;
    unsigned int        IPdigit;
    unsigned int         IPaddr = 0;
    unsigned char        hasOctet;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    for (IPdigit = 0; IPdigit < 4; IPdigit++)
    {
        hasOctet = FALSE;
        while (('\0' != (Digit = *pString)) && ('.' != *pString))
        {
            if (FALSE == ISDIGIT(Digit))
                return CONVERSION_ERROR_INCORRECT_TYPE;

            /* check for overflow.  size containment problems... */
            UnsignCheck(MAX_UCHAR, Uchar, Digit, char);

            Uchar *= 10;
            Uchar += (unsigned char) DIGIT_TO_CHAR(Digit);

            pString++;
            hasOctet = TRUE;
        }
        if (FALSE == hasOctet)
            return CONVERSION_ERROR_INCORRECT_TYPE;

        IPaddr <<= 8;
        IPaddr  += Uchar;
        Uchar    = 0;

        if ('\0' == Digit)
        {
            if (3 != IPdigit)
                return CONVERSION_ERROR_INCORRECT_TYPE;
            else break;
        }

        /* extra garbage characters? */
        if ((3 == IPdigit) && ('\0' != Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        pString++;
    }

/* endian conversion may be necessary here */

    if (convert)
    {
        if (NULL == pIPaddr)
            return GENERAL_ERROR_NULL_POINTER;

        *((unsigned int *)pIPaddr) = HTON4(IPaddr);
    }

    return RT_OK;

}   /* parseStrToIP */

/*-----------------------------------------------------------------------*/
/* sample conversion code */

char parseHex2Dec(char HexDigit)
{
    if (('0' <= HexDigit) && (HexDigit <= '9'))
        return (HexDigit - '0');

    if (('a' <= HexDigit) && (HexDigit <= 'f'))
        return (HexDigit - 'a' + 10);

    if (('A' <= HexDigit) && (HexDigit <= 'F'))
        return (HexDigit - 'A' + 10);

    /* illegal digit */
    return -1;
}

/*-----------------------------------------------------------------------*/

char parseDec2Hex(char DecDigit) 
{
    int digit = (int) DecDigit;

    if ((0 <= digit) && (digit <= 9))
        return (digit + '0');

    if ((10 <= digit) && (digit <= 15))
        return (digit - 10 + 'a');

    /* illegal digit */
    return ' ';
}

/*-----------------------------------------------------------------------*/
/*
extern void parseHexString(int *hex, const char *pBuf)
{
    char   t     = 0;
    unsigned char zeros = TRUE;

    *(pBuf)++ = '0';
    *(pBuf)++ = 'x';

    while (*hex)
    {
        t = (0xf0000000 & *hex) >> 28;
        if (! zeros || (0 < t))
        {
            *pBuf = parseDec2Hex(t);
            pBuf++;
            zeros = FALSE;
        }
        *hex = *hex << 4;
    }
    if (zeros)
        *(pBuf++) = '0';

    *pBuf = kCHAR_NULL;
}
*/

/*-----------------------------------------------------------------------*/

/* Is string a valid numeric format? */

unsigned char parseIsNumber(const char *pBuf)
{
    if (NULL_STRING(pBuf))
        return FALSE;

    if (('-' == *pBuf) || ('+' == *pBuf))
        pBuf++;

    if (! ISDIGIT(*pBuf))
        return FALSE;

    for ( ; '\0' != *pBuf ; pBuf++)
    {
        if (! ISDIGIT(*pBuf))
            return FALSE;
    }

    return TRUE;
}

int parseStrToMacWildMask(const char *pString, void *pMacAddr, unsigned char convert)
{
	unsigned char *p = NULL, *pp1 = NULL, *pp2 = NULL;
	unsigned char *addr = (unsigned char *)pMacAddr;
	char *endptr = NULL; 
	int p1 = 0, p2 = 0, p3 = 0;
	unsigned char buf[32];
	pp2 = (unsigned char *)pString;

	if(strlen(pString) != 14)
		return PARAMETER_ERROR;
	//like ffff-00ff-ff00
	memset(buf, 0x00, sizeof(buf));
	p = (unsigned char *)strstr(pString, "-");
	if(((p - (unsigned char *)pString) != 4 )|| (NULL == p))
		return PARAMETER_ERROR;
	memcpy(buf, pString, p-(unsigned char *)pString);
	p1 = strtoul ((char *)buf, &endptr, 16); 
	addr[0] = (p1>>8)&0xff;
	addr[1] = p1&0xff;
	memset(buf, 0x00, sizeof(buf));

	pp1 = (unsigned char *)strstr((char *)++p, "-");
	if(((pp1 - p) != 4 )||(NULL == pp1))
		return PARAMETER_ERROR;
	pp1++;
	memcpy(buf, p, pp1-p-1);
	p2 = strtoul ((char *)buf, &endptr, 16); 
	addr[2] = (p2>>8)&0xff;
	addr[3] = p2&0xff;
	memset(buf, 0x00, sizeof(buf));

	if((strlen((char *)pp2) - (pp1-pp2)) != 4)
		return PARAMETER_ERROR;
	memcpy(buf, pp1, 4);
	p3 = strtoul ((char *)buf, &endptr, 16); 
	addr[4] = (p3>>8)&0xff;
	addr[5] = p3&0xff;
	
    return RT_OK;
}


int parseStrToMacAddr(const char *pString, void *pMacAddr, unsigned char convert)
{
    short         hiDigit;
    short         loDigit;
    short         tempDigit;
    unsigned int        index;
    char          MacAddr[MACADDR_OCTETS];

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    for (index = 0; index < MACADDR_OCTETS; index++)
    {
        hiDigit = parseHex2Dec(*(pString++));

        if (('\0' != *pString) && (':' != *pString))
            loDigit = parseHex2Dec(*(pString++));
        else
        {
            loDigit = hiDigit;
            hiDigit = 0;
        }
        
        if ((0 > hiDigit) || (0 > loDigit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        tempDigit = (hiDigit << 4) + loDigit;
        if ((0 > tempDigit) || (tempDigit > 255))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        if ((index < (MACADDR_OCTETS - 1)) && (':' != *pString))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        if (index < MACADDR_OCTETS - 1)
        {
            if (':' != *pString)
                return GENERAL_ERROR_ILLEGAL_VALUE;
        }
        else
        {
            if (0 != *pString)
                return GENERAL_ERROR_ILLEGAL_VALUE;
        }

        pString++;

        MacAddr[index] = (char) tempDigit;
    }

    /* endian conversion may be necessary here */

    if (convert)
    {
        if (NULL == pMacAddr)
            return GENERAL_ERROR_NULL_POINTER;

        memcpy(pMacAddr, MacAddr, MACADDR_OCTETS);
    }

    return RT_OK;

}   /* parseStrToMacAddr */
 
/*-----------------------------------------------------------------------*/

int parseStrToInt(const char *pString, void *pInt, unsigned char convert)
{
    unsigned int    Int = 0;
    char   Digit;
    unsigned char Signed = FALSE;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    if ('-' == *pString)
    {
        Signed = TRUE;
        pString++;
    }
    else if ('+' == *pString)
        pString++;

    if ('\0' == *pString)
        return GENERAL_ERROR_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        SignCheck(MIN_INT, MAX_INT, Signed, Int, Digit, int);

        Int *= 10;
        Int += DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pInt)
            return GENERAL_ERROR_NULL_POINTER;

        *((int *) pInt) = (Signed == TRUE) ? (int)((~Int)+1) : (int)Int;    
    }

    return RT_OK;

}   /* parseStrToInt */

/*-----------------------------------------------------------------------*/

int parseStrToUnsigned(const char *pString, void *pUnsigned, unsigned char convert)
{
    unsigned int    Unsigned = 0;
    char           Digit;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        UnsignCheck(MAX_UNSIGNED, Unsigned, Digit, int);

        Unsigned *= 10;
        Unsigned += DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pUnsigned)
            return GENERAL_ERROR_NULL_POINTER;

        *((unsigned int *) pUnsigned) = Unsigned;   
    }

    return RT_OK;
}

/*-----------------------------------------------------------------------*/

int parseStrToShort(const char *pString, void *pShort, unsigned char convert)
{
    unsigned short  Short = 0;
    char           Digit;
    unsigned char         Signed = FALSE;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    if ('-' == *pString)
    {
        Signed = TRUE;
        pString++;
    }
    else if ('+' == *pString)
        pString++;

    if ('\0' == *pString)
        return GENERAL_ERROR_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        SignCheck(MIN_SHORT, MAX_SHORT, Signed, Short, Digit, short);

        Short *= 10;
        Short += (short) DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pShort)
            return GENERAL_ERROR_NULL_POINTER;

        *((short *) pShort) = (short) ((Signed == TRUE) ? (~Short + 1) : Short);
    }

    return RT_OK;

}   /* parseStrToShort */

/*-----------------------------------------------------------------------*/

int parseStrToUshort(const char *pString, void *pUshort, unsigned char convert)
{
    unsigned short      Ushort = 0;
    char               Digit;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        UnsignCheck(MAX_USHORT, Ushort, Digit, short);

        Ushort *= 10;
        Ushort = (unsigned short)(Ushort + DIGIT_TO_CHAR(Digit));

        pString++;
    }

    if (convert)
    {
        if (NULL == pUshort) 
            return GENERAL_ERROR_NULL_POINTER;

        *((unsigned short *) pUshort) = Ushort;     
    }

    return RT_OK;
}

/*-----------------------------------------------------------------------*/

int parseStrToLong(const char *pString, void *pLong, unsigned char convert)
{
    unsigned long   Long = 0;
    char           Digit;
    unsigned char         Signed = FALSE;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    if ('-' == *pString)
    {
        Signed = TRUE;
        pString++;
    }
    else if ('+' == *pString)
        pString++;

    if ('\0' == *pString)
        return GENERAL_ERROR_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        SignCheck(MIN_LONG, MAX_LONG, Signed, Long, Digit, long);

        Long *= 10;
        Long += DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pLong)
            return GENERAL_ERROR_NULL_POINTER;

        *((long *) pLong) = (Signed == TRUE) ? (long)((~Long)+1) : (long)Long;  
    }

    return RT_OK;

}   /* parseStrToLong */

/*-----------------------------------------------------------------------*/

int parseStrToUlong(const char *pString, void *pUlong, unsigned char convert)
{
    unsigned long       Ulong = 0;
    char               Digit;

    if (NULL_STRING(pString))
        return GENERAL_ERROR_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return CONVERSION_ERROR_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        UnsignCheck(MAX_ULONG, Ulong, Digit, long);

        Ulong *= 10;
        Ulong += DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pUlong)
            return GENERAL_ERROR_NULL_POINTER;

        *((unsigned long *) pUlong) = Ulong;        
    }

    return RT_OK;
}

/*-----------------------------------------------------------------------*/

int parsePrefixIpv6Addr (const char *str, struct in6_addr *addr, int *pLen)
{
  char *pnt;
  char *cp;
  int ret;

  pnt = strchr (str, '/');

  /* If string doesn't contain `/' treat it as host route. */
  if (pnt == NULL) 
    {
      ret = inet_pton (AF_INET6, str, addr);
      if (ret == 0)
	        return RT_ERR;
      *pLen = IPV6_MAX_BITLEN;
    }
  else 
    {
      int plen;

      cp = (char *)malloc ((pnt - str) + 1);
      strncpy (cp, str, pnt - str);
      *(cp + (pnt - str)) = '\0';
      ret = inet_pton (AF_INET6, cp, addr);
      free (cp);
      if (ret == 0)
	    return RT_ERR;
      plen = (u_char) atoi (++pnt);
      if (plen > IPV6_MAX_BITLEN)
	    return RT_ERR;
      *pLen = plen;
    }

  return RT_OK;
}

char *parseParamErrorPrint(char *buf, int errcode)
{
    if(!buf)
        return buf;

    switch(errcode){
        case CONVERSION_ERROR_INCORRECT_TYPE:
            break;
        case CONVERSION_ERROR_OVERFLOW:
        case CONVERSION_ERROR_TOO_LONG:
            break;
        default:
            break;
    }
    sprintf(buf, " Parameter Error ");
    return buf;
}

#if 0

int parse_ulong(const char *para_string, int *value) 
{
    char *endptr = NULL; 
    errno = 0; 
    (*value) = strtoul ((para_string), &endptr, 0); 
    if (*(para_string) == '-' || *endptr != '\0' || errno || value < 0) 
    { 
        //vty_out (vty, "%% Invalid %s value%s", name, VTY_NEWLINE); 
        return RT_ERR; 
    } 
    return RT_RT_OK;
}

int parse_ushort(const char *para_string, unsigned short *value) 
{ 
    char *endptr = NULL; 
    int v = 0;
    errno = 0; 
    (v) = strtoul ((para_string), &endptr, 0); 
    if (*(para_string) == '-' || *endptr != '\0' || errno || value > 65535 || value < 0) 
    { 
        //vty_out (vty, "%% Invalid %s value%s", name, VTY_NEWLINE); 
        return RT_ERR; 
    } 
    *value = (unsigned short)v;
    return RT_RT_OK;
}

int parse_ushort(const char *para_string, unsigned char *value) 
{ 
    char *endptr = NULL; 
    int v = 0;
    errno = 0; 
    (value) = strtoul ((para_string), &endptr, 0); 
    if (*(para_string) == '-' || *endptr != '\0' || errno || value > 255 || value < 0) 
    { 
      //vty_out (vty, "%% Invalid "#name" value%s", VTY_NEWLINE); 
      return RT_ERR; 
    } 
    *value = (unsigned char)v;
    return RT_RT_OK;
}


#define PARSE_CHAR_STRING(para_string, value, name) \
do { \
  char *endptr = NULL; \
  errno = 0; \
  (value) = strtoul ((para_string), &endptr, 0); \
  if (*(para_string) == '-' || *endptr != '\0' || errno || value > 128 || value < -128) \
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
#endif


char *convertNumToStr(int num, char *buf, int hex)
{
    if(NULL == buf)
        return buf;

    if(hex == 10)
        sprintf(buf, "%d", num);
    else if(hex == 16)
        sprintf(buf, "%x", num);
    else
        sprintf(buf, "%d", num);
    
    return buf;
}







