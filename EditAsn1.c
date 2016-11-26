/********************************************************************
 Can do -d, -D with CORRECT Levels
   the -e basic routines also work
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define XXXX_GOOD              0
#define XXXX_BAD               -1
#define XXXX_EOF               99

#define XXXX_READ_BUFF_LEN     100
#define XXXX_PAD_CHAR          ' ' 
#define XXXX_MAX_CONTENT_LEN   1000
#define XXXX_MAX_TAG_LEN       10
#define XXXX_MAX_LEN_LEN       10
#define XXXX_MAX_OPEN_NODES    30

typedef enum { XXXX_PRIMITIVE_TAG, XXXX_CONSTRUCTED_TAG, XXXX_NULL_BYTE_TAG } TagType_t;
typedef enum { XXXX_UNIVERSAL_TAG, XXXX_APPLICATION_TAG, XXXX_CONTEXT_SPEC_TAG, XXXX_PRIVATE_TAG } TagClass_t;

typedef struct {
	char tagName[80];
	TagClass_t tagClass;
	TagType_t tagType;
} Asn1TagInfo_t ;

#include "asn1TagInfo.h"              /* <== Definition of g_asn1TagInfoArray & XXXX_MAX_TAG_NUMBER */

typedef struct {
	unsigned int  tagValue;
	char tagBytes[XXXX_MAX_TAG_LEN];
	TagType_t  tagType;
	TagClass_t tagClass;
	unsigned int  tagInOneByteInd;
	int  tagBytesCount;
} TagInfo_t ;

typedef enum { XXXX_UNDEF_LEN_LEN, XXXX_FINITE_LEN_LEN, XXXX_NULL_BYTE_LEN } LenType_t;
typedef struct {
	unsigned int lenValue;
	char lenBytes[XXXX_MAX_LEN_LEN];
	LenType_t lenType;
	int  lenBytesCount;
} LenInfo_t;

typedef enum { XXXX_PRINTABLE_CONTENT, XXXX_UNPRINTABLE_CONTENT } ContentPrintableInd_t;
typedef struct {
	char contentValue[XXXX_MAX_CONTENT_LEN];
	char contentBytes[XXXX_MAX_CONTENT_LEN];
	int  contentBytesCount;
	ContentPrintableInd_t  contentPrintableInd;
} ContentInfo_t;

typedef struct {
	unsigned int  ucnTagValue;
	LenType_t ucnLenType;
	float ucnEndPos;
} UnclosedNodesInfo_t;


int            xxxx_EXECinit(int i_argc, char *i_argv[]);
int            xxxx_EXECterm();

int            xxxx_PROCdecodeAndPrint();
unsigned char  xxxx_readTheNextByte();
int            xxxx_GetTagValue(TagInfo_t *o_tagInfo);
int            xxxx_GetLengthValue(LenInfo_t *o_lenInfo) ;
int            xxxx_GetContentValue(int i_len, ContentInfo_t *o_contentInfo);
int            xxxx_GetPaddingLevel(TagInfo_t i_tagInfo, LenInfo_t i_lenInfo, int *o_paddingLevel);

int            xxxx_PROCencodeAndWrite();
char *         xxxx_readTheNextLine();
int            xxxx_GetTagInBytes(int i_tagValue, TagInfo_t *o_tagInfo);
int            xxxx_GetLengthInBytes(int i_lenValue, LenInfo_t *o_lenInfo);

enum { XXXX_MODE_SMART_DECODE_PRINT,XXXX_MODE_DECODE_PRINT, 
       XXXX_MODE_ENCODE_WRITE, XXXX_MODE_INVALID_MODE } g_applicationMode;
FILE *fpI, *fpO;
int   g_InpFileEofInd =0;
float g_InputBytePos=0;


int main(int argc, char *argv[])
{
	int  rc;

	rc = xxxx_EXECinit(argc, argv);
	if ( rc )
		return XXXX_BAD;

	switch ( g_applicationMode ) 
	{
	  case XXXX_MODE_SMART_DECODE_PRINT :
	  case XXXX_MODE_DECODE_PRINT : 
		rc = xxxx_PROCdecodeAndPrint();
		break;
	  case XXXX_MODE_ENCODE_WRITE : 
		rc = xxxx_PROCencodeAndWrite();
		break;
        }

	if ( rc )
	{
		printf("Fatal Error in Processing\n");
		return XXXX_BAD;
	}

	rc = xxxx_EXECterm();
	if ( rc )
	{
		printf("Fatal Error in Program Termination \n");
		return XXXX_BAD;
	}

	printf("\n\n Thanks \n");
	return XXXX_GOOD;
}

/********************************************************************
int xxxx_EXECinit(int i_argc, char *i_argv[])
********************************************************************/
int xxxx_EXECinit(int i_argc, char *i_argv[])
{
#define XXXX_PRINT_USAGE(i_msg) { \
	printf("\n\t%s\n\n Usage" \
			 "\n     Smart Decode    :   %s -d <ASN.1 File>" \
                         "\n     Basic Decode    :   %s -D <ASN.1 File>" \
	                 "\n     Encoding        :   %s -e <Ascii TLV File> <ASN.1 File>\n\n", \
			  i_msg, i_argv[0], i_argv[0], i_argv[0]); \
        }

	if ( i_argc < 2 )
	{
		XXXX_PRINT_USAGE("Missing Arguments");
		return XXXX_BAD;
	}

	g_applicationMode = XXXX_MODE_INVALID_MODE;
	if ( !strcmp(i_argv[1],"-d") )
		g_applicationMode = XXXX_MODE_SMART_DECODE_PRINT;
	if ( !strcmp(i_argv[1],"-D") )
		g_applicationMode = XXXX_MODE_DECODE_PRINT;
	if ( !strcmp(i_argv[1],"-e") )
	       	g_applicationMode = XXXX_MODE_ENCODE_WRITE;
	if ( g_applicationMode == XXXX_MODE_INVALID_MODE )
	{ 
	       XXXX_PRINT_USAGE( "Invalid Mode");
	       return XXXX_BAD;
	}


	if( (( g_applicationMode == XXXX_MODE_SMART_DECODE_PRINT || 
	       g_applicationMode == XXXX_MODE_DECODE_PRINT )  && i_argc < 3 ) ||
	    (g_applicationMode == XXXX_MODE_ENCODE_WRITE  && i_argc < 4 ))
	{
		XXXX_PRINT_USAGE("Missing Arguments"); 
		return XXXX_BAD;
	}

	fpI = fopen(i_argv[2], "rb");
	if ( fpI == NULL )
	{
		printf("\nCannot Open File %s\n\n", i_argv[2] );
		return XXXX_BAD;
	}
	setvbuf(fpI, NULL,_IONBF, 0);

	if( g_applicationMode == XXXX_MODE_ENCODE_WRITE )
	{
		fpO = fopen(i_argv[3], "wb");
		if ( fpO == NULL )
		{
			printf("\nCannot Open File %s\n\n", i_argv[3] );
			return XXXX_BAD;
		}
	}

	return XXXX_GOOD;
}

/********************************************************************
int xxxx_EXECterm()
********************************************************************/
int xxxx_EXECterm()
{
	fclose(fpI);

	if( g_applicationMode == XXXX_MODE_ENCODE_WRITE )
		fclose(fpO);

	return XXXX_GOOD;
}


/********************************************************************
int xxxx_PROCdecodeAndPrint()
********************************************************************/
int xxxx_PROCdecodeAndPrint()
{
	TagInfo_t       l_tagInfo, lt_tagInfo;
	LenInfo_t       l_lenInfo;
	ContentInfo_t   l_contentInfo;
	int             rc= XXXX_GOOD, l_numOfPaddings;
	char            l_tagInBytes[10];

	while ( 1 )
	{
		rc = xxxx_GetTagValue(&l_tagInfo);
		if ( rc == XXXX_EOF )
		{
			rc = XXXX_GOOD;
			break;
		}

		rc = xxxx_GetLengthValue(&l_lenInfo);

		rc = xxxx_GetPaddingLevel(l_tagInfo, l_lenInfo, &l_numOfPaddings);
		if ( rc == XXXX_BAD )
			break;

		if( l_tagInfo.tagType == XXXX_NULL_BYTE_TAG && 
		                l_lenInfo.lenType == XXXX_NULL_BYTE_LEN )
		{
			printf ("%*.c N<00>", l_numOfPaddings*3, XXXX_PAD_CHAR );
		}
		else
		{
			xxxx_GetTagInBytes(l_tagInfo.tagValue, &lt_tagInfo);  /* @ */
			printf("%*.c T<%s %0X> L<%d :%s>  ", l_numOfPaddings*3, XXXX_PAD_CHAR,
			    ( g_applicationMode == XXXX_MODE_SMART_DECODE_PRINT ) ? 
			    g_asn1TagInfoArray[l_tagInfo.tagValue].tagName : l_tagInfo.tagBytes,
			    l_tagInfo.tagValue, l_lenInfo.lenValue, l_lenInfo.lenBytes);
		}

		if( l_tagInfo.tagType == XXXX_PRIMITIVE_TAG)
		{
			rc = xxxx_GetContentValue(l_lenInfo.lenValue, &l_contentInfo );
			if( l_contentInfo.contentPrintableInd == XXXX_PRINTABLE_CONTENT )
				printf("V<%s>", l_contentInfo.contentValue);
			else
				printf("Vx<x%s>", l_contentInfo.contentBytes );
		}
		printf("\n");
	}

	return rc;
}

/********************************************************************
int  xxxx_GetTagValue(TagInfo_t *o_tagInfo)
********************************************************************/
int  xxxx_GetTagValue(TagInfo_t *o_tagInfo)
{
	unsigned char l_tagChar;

	o_tagInfo->tagBytesCount=0;
	o_tagInfo->tagValue=0;
	o_tagInfo->tagType=XXXX_PRIMITIVE_TAG;

	l_tagChar = xxxx_readTheNextByte();
	if( g_InpFileEofInd == XXXX_EOF )
		return XXXX_EOF ;

	sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, "%02X", l_tagChar);
	o_tagInfo->tagBytesCount++;

	o_tagInfo->tagType = (l_tagChar & 0x20)>>5 ? XXXX_CONSTRUCTED_TAG : XXXX_PRIMITIVE_TAG ;
	o_tagInfo->tagClass =  (l_tagChar & 0xC0)>>6;
	o_tagInfo->tagInOneByteInd = ( (l_tagChar & 0x1F) == 0x1F ) ? 0 : 1;

	if( l_tagChar == 0x00 )
		o_tagInfo->tagType = XXXX_NULL_BYTE_TAG;

	if ( o_tagInfo->tagInOneByteInd )
		o_tagInfo->tagValue = (l_tagChar & 0x1F);
	else
	{
		do
		{
			l_tagChar = xxxx_readTheNextByte();
			sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, "%02X", l_tagChar);
			o_tagInfo->tagBytesCount++;
			o_tagInfo->tagValue = ( o_tagInfo->tagValue <<7 ) | ( l_tagChar & 0x7F );
		} while (  (l_tagChar & 0x80 ) ) ;

	}

	*(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2) =  0;

	if ( o_tagInfo->tagValue >= XXXX_MAX_TAG_NUMBER )
		o_tagInfo->tagValue = XXXX_MAX_TAG_NUMBER;

	return XXXX_GOOD;
}

/********************************************************************
int  xxxx_GetLengthValue(LenInfo_t *o_lenInfo )
********************************************************************/
int  xxxx_GetLengthValue(LenInfo_t *o_lenInfo )
{
	unsigned char l_lenChar;
	int  i, l_bytesToRead;

	o_lenInfo->lenBytesCount=0;

	l_lenChar = xxxx_readTheNextByte();
	sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "%02X", l_lenChar);
	o_lenInfo->lenBytesCount++;

	if( l_lenChar == 0x0 )
	{
		o_lenInfo->lenValue = 0;
		o_lenInfo->lenType =XXXX_NULL_BYTE_LEN;
	}
	else
		if ( l_lenChar == 0x80 )
		{
			o_lenInfo->lenValue = 0;
			o_lenInfo->lenType = XXXX_UNDEF_LEN_LEN ;
		}
		else
		{
			o_lenInfo->lenType = XXXX_FINITE_LEN_LEN ;
			if ( (l_lenChar & 0x80) != 0x80 )
				o_lenInfo->lenValue = (l_lenChar & 0x7F);
			else
			{
				l_bytesToRead=( l_lenChar & 0x7F ) ;
				o_lenInfo->lenValue = 0;
				for ( i=0; i<l_bytesToRead;++i)
				{
					l_lenChar = xxxx_readTheNextByte();
					sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "%02X", l_lenChar);
					o_lenInfo->lenBytesCount++;
					o_lenInfo->lenValue = ( o_lenInfo->lenValue <<8 ) | ( l_lenChar & 0xFF );
				}
			}
		}

	*(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2)=0;
	return o_lenInfo->lenValue;
}

/********************************************************************
int  xxxx_GetContentValue(int i_len, ContentInfo_t *o_contentInfo)
********************************************************************/
int  xxxx_GetContentValue(int i_len, ContentInfo_t *o_contentInfo)
{
	unsigned char l_valueChar;
	int i, l_contentValueBytePos=0;

	o_contentInfo->contentPrintableInd = XXXX_PRINTABLE_CONTENT;
	o_contentInfo->contentBytesCount = 0;
	for(i=1; i<=i_len; i++)
	{
		l_valueChar = xxxx_readTheNextByte();
		sprintf(o_contentInfo->contentBytes+o_contentInfo->contentBytesCount*2, "%02X", l_valueChar);
		o_contentInfo->contentBytesCount++;

		o_contentInfo->contentValue[l_contentValueBytePos++] = l_valueChar;
		if (!isprint(l_valueChar) )
			o_contentInfo->contentPrintableInd = XXXX_UNPRINTABLE_CONTENT;

	}

	*(o_contentInfo->contentBytes+o_contentInfo->contentBytesCount*2)=0;
	o_contentInfo->contentValue[l_contentValueBytePos] = 0;

	return XXXX_GOOD;
}

/********************************************************************
int  xxxx_GetPaddingLevel(TagInfo_t i_tagInfo, LenInfo_t i_lenInfo, int *o_paddingLevel)
********************************************************************/
int  xxxx_GetPaddingLevel(TagInfo_t i_tagInfo, LenInfo_t i_lenInfo, int *o_paddingLevel)
{
	static UnclosedNodesInfo_t l_ucnl[XXXX_MAX_OPEN_NODES];
	static int l_unclosedNodesCount=0, rc= XXXX_GOOD;
	char l_activityArray[28];
	int  l_UndefUcn2Close=0, l_UndefUcnFound=0;

	*o_paddingLevel = l_unclosedNodesCount;

	memset(l_activityArray,0, sizeof(l_activityArray));

	/* Pop */
        if( i_tagInfo.tagType == XXXX_NULL_BYTE_TAG && i_lenInfo.lenType == XXXX_NULL_BYTE_LEN )
             l_UndefUcn2Close = 1;

        while( l_unclosedNodesCount>0 ) 
        {
	    if ( l_ucnl[l_unclosedNodesCount-1].ucnLenType == XXXX_UNDEF_LEN_LEN )
	    {
               l_UndefUcnFound++;
               if ( l_UndefUcnFound <= l_UndefUcn2Close ) 
               {
	             sprintf(l_activityArray,"_%02X", 
			     l_ucnl[l_unclosedNodesCount-1].ucnTagValue);
		     l_unclosedNodesCount--;
               }
               else 
                  break;
	    }

	    if( l_unclosedNodesCount>0 && 
	       l_ucnl[l_unclosedNodesCount-1].ucnLenType == XXXX_FINITE_LEN_LEN )
	    {   
	       if( g_InputBytePos >=  l_ucnl[l_unclosedNodesCount-1].ucnEndPos )
	       {
			sprintf(l_activityArray,"%s-%02X", l_activityArray, 
			      l_ucnl[l_unclosedNodesCount-1].ucnTagValue);
			l_unclosedNodesCount--;
			(*o_paddingLevel)--;
	        }
	        else
	        	break;
	    }
	}

	/* Push */
	if( i_tagInfo.tagType == XXXX_CONSTRUCTED_TAG )
	{
		l_ucnl[l_unclosedNodesCount].ucnTagValue = i_tagInfo.tagValue ;
		l_ucnl[l_unclosedNodesCount].ucnLenType = i_lenInfo.lenType ;
		l_ucnl[l_unclosedNodesCount].ucnEndPos = g_InputBytePos + i_lenInfo.lenValue;
		sprintf(l_activityArray,"%s+%02X", l_activityArray, 
			 l_ucnl[l_unclosedNodesCount].ucnTagValue);
		l_unclosedNodesCount++;
	}

#ifdef PRINTMORE
	printf("%18s %05.f %2d %2d :",l_activityArray,
	    g_InputBytePos,l_unclosedNodesCount, *o_paddingLevel );
#endif

	if ( l_unclosedNodesCount > XXXX_MAX_OPEN_NODES || l_unclosedNodesCount < 0 )
		rc = XXXX_BAD;

	return rc;
}

/********************************************************************
unsigned char xxxx_readTheNextByte() 
********************************************************************/
unsigned char xxxx_readTheNextByte()
{
	unsigned char l_retval_byte;
	static unsigned char l_read_Buffer[XXXX_READ_BUFF_LEN +2 ];
	static int l_cur_pos=0,l_buff_len=0;
	static int buffsRead=0;

	if( g_InpFileEofInd == XXXX_EOF )
		return XXXX_EOF;

	fflush(fpI);

	if ( l_cur_pos == l_buff_len )
	{
		l_buff_len = fread (l_read_Buffer,1, XXXX_READ_BUFF_LEN, fpI);
		l_cur_pos=0;
	}

	l_retval_byte = l_read_Buffer[l_cur_pos] ;
	if( l_cur_pos == l_buff_len && l_buff_len != XXXX_READ_BUFF_LEN)
	{
		g_InpFileEofInd = XXXX_EOF;
		return XXXX_EOF;
	}

	l_cur_pos++;
	g_InputBytePos++;

	return l_retval_byte;
}

/********************************************************************
int     xxxx_PROCencodeAndWrite()
********************************************************************/
int     xxxx_PROCencodeAndWrite()
{
	return XXXX_GOOD;
}


/********************************************************************
int    xxxx_GetTagInBytes(int i_tagValue, TagInfo_t *o_tagInfo)
********************************************************************/
int    xxxx_GetTagInBytes(int i_tagValue, TagInfo_t *o_tagInfo)
{
	int  i, l_bytesReq, l_128sMuliple;

	unsigned char l_tagByte;

	o_tagInfo->tagValue = i_tagValue;
	o_tagInfo->tagType = g_asn1TagInfoArray[i_tagValue].tagType;
	o_tagInfo->tagClass = g_asn1TagInfoArray[i_tagValue].tagClass;

	o_tagInfo->tagBytesCount=0;

	l_tagByte = (unsigned char )g_asn1TagInfoArray[i_tagValue].tagClass <<6;
	l_tagByte = l_tagByte | (  (unsigned char )g_asn1TagInfoArray[i_tagValue].tagType <<5);

	if ( i_tagValue < 0x1f )
	{
		o_tagInfo->tagInOneByteInd = 1;
		l_tagByte |= i_tagValue;
		sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, "%02X", l_tagByte);
		o_tagInfo->tagBytesCount++;
	}
	else
	{
		o_tagInfo->tagInOneByteInd = 0;
		l_tagByte |= 0x1f;
		sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, "%02X", l_tagByte);
		o_tagInfo->tagBytesCount++;

		l_128sMuliple = 1;
		for(l_bytesReq=0; l_128sMuliple < i_tagValue ; l_bytesReq++)
		{
			l_128sMuliple *= 128;
		}

		for(i=l_bytesReq;i>1;--i)   
		{
		       l_tagByte = i_tagValue >> (i-1)*7;
		       l_tagByte |= 0x80;
		       sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, "%02X", l_tagByte);
		       o_tagInfo->tagBytesCount++;
		}

		l_tagByte = i_tagValue & 0x7f;

		sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, "%02X", l_tagByte);
		o_tagInfo->tagBytesCount++;
	}

	*(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2) = 0;

	return XXXX_GOOD;
}


/********************************************************************
int  xxxx_GetLengthInBytes(int i_lenValue, LenInfo_t *o_lenInfo )
********************************************************************/
int  xxxx_GetLengthInBytes(int i_lenValue, LenInfo_t *o_lenInfo )
{
	unsigned char l_lenByte;
	int  i ;
	int l_bytesReq, l_256sMuliple;

	o_lenInfo->lenValue = i_lenValue;
	o_lenInfo->lenBytesCount=0;

	switch ( i_lenValue )
	{

	case 0x00 :
		sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "%02X", i_lenValue);
		o_lenInfo->lenBytesCount++;
		o_lenInfo->lenType =XXXX_NULL_BYTE_LEN;
		break;

	case -1 :
		sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "80");
		o_lenInfo->lenBytesCount++;
		o_lenInfo->lenType = XXXX_UNDEF_LEN_LEN;
		break;

	default:
		if ( i_lenValue <= 127 )
		{
			sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "%02X", i_lenValue);
			o_lenInfo->lenBytesCount++;
			o_lenInfo->lenType = XXXX_FINITE_LEN_LEN;
		}
		else
		{
			l_256sMuliple = 1;
			for(l_bytesReq=0; l_256sMuliple < i_lenValue ; l_bytesReq++)
				l_256sMuliple *= 256;

			l_lenByte = l_bytesReq;
			l_lenByte |= 0x80;
			sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "%02X", l_lenByte);
			o_lenInfo->lenBytesCount++;

			printf("\n This Needs %d Bytes ", l_bytesReq);
			for(i=l_bytesReq;i>0;--i)
			{
				l_lenByte =  i_lenValue >> (i-1)*8;
				sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "%02X", l_lenByte);
				o_lenInfo->lenBytesCount++;
			}
		}
	}

	*(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2)=0;
	printf("\n Its (%s)\n", o_lenInfo->lenBytes );
	return o_lenInfo->lenValue;
}


/********************************************************************
char * xxxx_readTheNextLine()
********************************************************************/
char * xxxx_readTheNextLine()
{
 char retval [1000];

 return retval;

}
