/*
* 	panger123 
*
*  	2014/01/06-?
*/

#ifndef __TF_TYPE_H__
#define __TF_TYPE_H__


/*
** General types
*/
//typedef unsigned int               UINT;	/* unsigned int (size depend on platform)*/
//typedef unsigned char              UINT8;	/* 8-bit  value*/
//typedef unsigned short             UINT16;	/* 16-bit value*/
//typedef unsigned int               UINT32;	/* 32-bit value*/
//typedef unsigned char              BOOL;	/* 8-bit*/
typedef unsigned char              BYTE;
//typedef int                        INT;		/* int (size depend on platform)*/
//typedef char                       INT8;   /* 8-bit  int*/
//typedef short                      INT16;  /* 16-bit int*/
//typedef long                       INT32;  /* 32-bit int*/


#define	PERM	(S_IRUSR | S_IWUSR)


#define TF_VALID 					1
#define TF_INVALID 				0

typedef enum
{
	SYS_IMAGE_TYPE_FILE=0,
	ONU_IMAGE_TYPE_FILE
}TF_FILE_TYPE;


/* Add by steven.tian 2014.09.05 ****begin */
#ifndef NELEMENTS
#define NELEMENTS(array) ((sizeof(array)/sizeof((array)[0])))
#endif
#ifndef OFFSETOFOBJ
#define OFFSETOFOBJ(type, field) ((unsigned long)&(((type*)0)->field))
#endif
#ifndef MIN
#define MIN(a, b) ((a) > (b) ? b : a)
#endif
/* Add by steven.tian 2014.09.05 ****end */

#endif

