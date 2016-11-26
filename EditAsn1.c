/********************************************************************
                     ASN.1 File Viewer/Editor
	                                    Arun Sivanandan
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define XXXX_GOOD              0
#define XXXX_BAD               -1
#define XXXX_EOF               99
#define XXXX_SOF               XXXX_GOOD

#define XXXX_READ_BIN_BUFF_LEN 1000
#define XXXX_READ_TXT_BUFF_LEN 1000
#define XXXX_MAX_CONTENT_LEN   1000
#define XXXX_MAX_TAG_LEN       10
#define XXXX_MAX_LEN_LEN       10
#define XXXX_MAX_OPEN_NODES    30
#define XXXX_PAD_CHARS         4

typedef enum { XXXX_INFO_TYPE_TAG, XXXX_DO_NO_SKIP_NULLS } InfoType_t;
typedef enum { XXXX_PRIMITIVE_TAG, XXXX_CONSTRUCTED_TAG, XXXX_NULL_BYTE_TAG } TagType_t;
typedef enum { XXXX_UNIVERSAL_TAG, XXXX_APPLICATION_TAG, 
	       XXXX_CONTEXT_SPEC_TAG, XXXX_PRIVATE_TAG } TagClass_t;

typedef struct {
	char tagName[80];
	TagClass_t tagClass;
	TagType_t tagType;
} Asn1TagInfo_t;

#include "asn1TagInfo.h"     /* <== Definition of g_asn1TagInfoArray & XXXX_MAX_TAG_NUMBER */

typedef struct {
	unsigned int  tagValue;
	char tagBytes[XXXX_MAX_TAG_LEN];
	TagType_t  tagType;
	TagClass_t tagClass;
	unsigned int  tagInOneByteInd;
	int  tagBytesCount;
} TagInfo_t;

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

int     xxxx_EXECinit(int i_argc, char *i_argv[]);
int     xxxx_EXECterm();

int     xxxx_PROCdecodeAndPrint();
int     xxxx_readTheNextByte(unsigned char *o_nextByte, InfoType_t i_typeOfInfo );
int     xxxx_GetTagValue(TagInfo_t *o_tagInfo);
int     xxxx_GetLengthValue(LenInfo_t *o_lenInfo) ;
int     xxxx_GetContentValue(int i_len, ContentInfo_t *o_contentInfo);
int     xxxx_GetPaddingLevel(TagInfo_t i_tagInfo, LenInfo_t i_lenInfo, int *o_paddingLevel, char *o_paddingStr);

int     xxxx_PROCencodeAndWrite();
int     xxxx_readTheNextLine(char *o_lineRead, int *o_lineLen);
int     xxxx_GetTknPositionInString(char *i_string,int i_strt,char i_tkn, int *o_tknPos);
int     xxxx_GetTagInHexBytes(int i_tagValue, TagInfo_t *o_tagInfo);
int     xxxx_GetLengthInHexBytes(int i_lenValue, LenInfo_t *o_lenInfo);
int     xxxx_GetContentInHexBytes(char *i_contentValue,ContentPrintableInd_t i_contentPrintableInd,
			    ContentInfo_t *o_contentInfo);
int     xxxx_writeHexBytesToFile(char *i_hexByteString);

typedef enum { XXXX_MODE_SMART_DECODE_PRINT,XXXX_MODE_BASIC_DECODE_PRINT, 
       XXXX_MODE_SMART_ENCODE_WRITE,XXXX_MODE_BASIC_ENCODE_WRITE,
       XXXX_MODE_INVALID_MODE } ExecutionMode_t;

typedef struct {
   ExecutionMode_t executionMode;
   char     commandArg[4];
   char     modeDescription[50];
   int      minNumberOfParams;
   char     inputFileInd;
   char     outputFileInd;
   int      (*functionPtr)();
   char     usageArgListMessage[50];
} CommandLineParams_t;

CommandLineParams_t g_commandLineParams[] =
{
 { XXXX_MODE_SMART_DECODE_PRINT,"-d","Smart Decode for TAP3",
      3,'Y','N', xxxx_PROCdecodeAndPrint,"<ASN.1 File>"},
 { XXXX_MODE_BASIC_DECODE_PRINT,"-D","Basic/Primitive Decode",
      3,'Y','N', xxxx_PROCdecodeAndPrint,"<ASN.1 File>"},
 { XXXX_MODE_SMART_ENCODE_WRITE,"-e","Smart Encode for TAP3",
      4,'Y','Y', xxxx_PROCencodeAndWrite,"<Ascii TLV File> <ASN.1 File>"},
 { XXXX_MODE_BASIC_ENCODE_WRITE,"-E","Basic/Primitive Encode",
      4,'Y','Y', xxxx_PROCencodeAndWrite,"<Ascii TLV File> <ASN.1 File>"},
 { XXXX_MODE_INVALID_MODE,"","",
      0,'N','N', NULL,NULL}
};

FILE    *fpI, *fpO;
int     g_InpFileEofInd=XXXX_SOF;
float   g_InputBytePos=0,g_InputLinesCount=0;
int     g_cmdLineParamIndex;

int main(int argc, char *argv[])
{
	int  rc;

	rc = xxxx_EXECinit(argc, argv);
	if ( rc == XXXX_BAD)
		return XXXX_BAD;

	if( g_commandLineParams[g_cmdLineParamIndex].functionPtr != NULL )
	      rc = (*g_commandLineParams[g_cmdLineParamIndex].functionPtr)();

	if ( rc == XXXX_BAD)
	{
		printf("Fatal Error in Processing\n");
		return XXXX_BAD;
	}

	rc = xxxx_EXECterm();
	if ( rc == XXXX_BAD)
	{
		printf("Fatal Error in Program Termination \n");
		return XXXX_BAD;
	}
	return XXXX_GOOD;
}

/********************************************************************
int xxxx_EXECinit(int i_argc, char *i_argv[])
********************************************************************/
int xxxx_EXECinit(int i_argc, char *i_argv[])
{
        int i;

	g_cmdLineParamIndex = XXXX_MODE_INVALID_MODE;
	if ( i_argc >= 2 )
	{
            for(i=0;(g_commandLineParams[i].functionPtr!=NULL); ++i) 
            {
                 if(!strcmp(g_commandLineParams[i].commandArg,i_argv[1])) 
	             g_cmdLineParamIndex = i;
            }
	}

	if( ( g_cmdLineParamIndex == XXXX_MODE_INVALID_MODE ) ||
	    ( g_commandLineParams[g_cmdLineParamIndex].minNumberOfParams > i_argc ) )
        {
		printf("\nMissing/Wrong Arguments..\n\n Usage :\n");
                for(i=0;(g_commandLineParams[i].functionPtr!=NULL);++i)
                     printf("\t%-25s : %s %s %s\n", g_commandLineParams[i].modeDescription, 
			 i_argv[0], g_commandLineParams[i].commandArg, g_commandLineParams[i].usageArgListMessage);

		return XXXX_BAD;
	}

        if( g_commandLineParams[g_cmdLineParamIndex].inputFileInd == 'Y' )
        {
	    fpI = fopen(i_argv[2], "rb");
	    if ( fpI == NULL )
	    {
		printf("\nCannot Open File %s\n\n", i_argv[2] );
		return XXXX_BAD;
	    }
	    setvbuf(fpI, NULL,_IONBF, 0);
        }

        if( g_commandLineParams[g_cmdLineParamIndex].outputFileInd == 'Y' )
	{
		fpO = fopen(i_argv[3], "wb");
		if ( fpO == NULL )
		{
			printf("\nCannot Open File %s\n\n", i_argv[3] );
			return XXXX_BAD;
		}
	        setvbuf(fpO, NULL,_IONBF, 0);
	}

	return XXXX_GOOD;
}

/********************************************************************
int xxxx_EXECterm()
********************************************************************/
int xxxx_EXECterm()
{
	if( g_commandLineParams[g_cmdLineParamIndex].inputFileInd == 'Y' ) 
	        fclose(fpI);

	if( g_commandLineParams[g_cmdLineParamIndex].outputFileInd == 'Y' ) 
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
	int             rc=XXXX_GOOD;
	char            l_paddingString[XXXX_MAX_OPEN_NODES*XXXX_PAD_CHARS];
	int             l_paddingLevel;
	char            l_tagInBytes[10];

	while ( 1 )
	{
		rc = xxxx_GetTagValue(&l_tagInfo);
		if ( rc != XXXX_GOOD )
			break;

		rc = xxxx_GetLengthValue(&l_lenInfo);
                if ( rc != XXXX_GOOD )
                        break;

		rc = xxxx_GetPaddingLevel(l_tagInfo, l_lenInfo, &l_paddingLevel, l_paddingString);
		if ( rc == XXXX_BAD )
			break;

		if( l_tagInfo.tagType == XXXX_NULL_BYTE_TAG && 
		    l_lenInfo.lenType == XXXX_NULL_BYTE_LEN )
		{
			printf ("%sN<Null 00>", l_paddingString);
		}
		else
		{
			if ( g_commandLineParams[g_cmdLineParamIndex].executionMode == XXXX_MODE_SMART_DECODE_PRINT)
			{
  		            printf("%sT<%s %s> L<%d> ", l_paddingString,
			            g_asn1TagInfoArray[l_tagInfo.tagValue].tagName,
			            l_tagInfo.tagBytes, l_lenInfo.lenValue);
			}
			else
			{
			    printf("%sT<%d %s> L<%d> ", l_paddingString,
				    l_tagInfo.tagValue,
				    l_tagInfo.tagBytes, l_lenInfo.lenValue);
			}
		}

		if( l_tagInfo.tagType == XXXX_PRIMITIVE_TAG)
		{
			rc = xxxx_GetContentValue(l_lenInfo.lenValue, &l_contentInfo );
                        if ( rc == XXXX_BAD )
			      break;

			if( l_contentInfo.contentPrintableInd == XXXX_PRINTABLE_CONTENT )
			    printf("V<%s>", l_contentInfo.contentValue);
			else
			    printf("Vx<%s>", l_contentInfo.contentBytes );
		}
		printf("\n");
	}

	return rc;
}

/********************************************************************
Encoding of the Tag
~~~~~~~~~~~~~~~~~~~
The identifier octets encode the ASN.1 tag of the data value. 
Two possibilities exist:
1. single octet encoding for tag numbers from 0 to 30 (inclusive)
                8   7   6   5   4   3   2   1
              +-------+---+-------------------+
            1 | CLASS |P/C|  NUMBER of TAG    |
              +-------------------------------+

               Bits 8-7: Class identifier:
              +----------------------------+
              |                Bit: 8   7  |
              +----------------------------|
              | Universal           0   0  |
              | Application         0   1  |
              | Context-specific    1   0  |
              | Private             1   1  |
              +----------------------------+
               Bit  6  : Primitive (0) or
                         Constructed (1)
               Bits 5-1: binary integer with bit 5 as msb

2. Use of a leading octet for tagnumbers bigger than or equal to 31
The leading octet is encoded as follows:
                8   7   6   5   4   3   2   1
              +-------+---+---+---+---+---+---+
            1 | CLASS |P/C| 1 | 1 | 1 | 1 | 1 |
              +-------------------------------+

               Bits 8-7: Class identifier as for single octet id
               Bit  6  : Primitive (0) or
                         Constructed (1)
               Bits 5-1: all bits set to 1

Subsequent octets are encoded as:
                8   7   6   5   4   3   2   1
              +---+---------------------------| first
            2 | 1 |  NUMBER of TAG (msb)      | subsequent
              +-------------------------------|
              .                               .
              .                               .
              +-------------------------------| last
              | 0 |  NUMBER of TAG (lsb)      | subsequent
              +-------------------------------+

               Bits 8 :  set to 1 in all non-last subsequent
                         octets
               Bits 7-1: Bits  7-1 of all subsequent octets
                         encoded as a binary integer equal to
                         the tagnumber with bit 7 of the first
                         subsequent octet as most significant
                         bit.
********************************************************************/
int  xxxx_GetTagValue(TagInfo_t *o_tagInfo)
{
	unsigned char l_tagChar;
	int rc=XXXX_GOOD;

	o_tagInfo->tagBytesCount=0;
	o_tagInfo->tagValue=0;
	o_tagInfo->tagType=XXXX_PRIMITIVE_TAG;

	rc = xxxx_readTheNextByte( &l_tagChar,XXXX_INFO_TYPE_TAG );
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
	                rc = xxxx_readTheNextByte( &l_tagChar,XXXX_INFO_TYPE_TAG );
			sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, 
				    "%02X", l_tagChar);
			o_tagInfo->tagBytesCount++;
			o_tagInfo->tagValue = ( o_tagInfo->tagValue <<7 ) |( l_tagChar & 0x7F );
                        
                        if( o_tagInfo->tagBytesCount*2 > XXXX_MAX_TAG_LEN) 
                        {
			     printf("\nTag Too Long %d \n", o_tagInfo->tagValue);
			     rc=XXXX_BAD;
                        }
		} while (  (l_tagChar & 0x80 ) );

	}

	*(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2) =  0;

	if ( o_tagInfo->tagValue >= XXXX_MAX_TAG_NUMBER )
		o_tagInfo->tagValue = XXXX_MAX_TAG_NUMBER;

	return rc;
}

/********************************************************************
Encoding of the length
~~~~~~~~~~~~~~~~~~~~~~
The length octets encode the length of the following content of the data item. 
Three possibilities exist in ASN.1: short, long and indefinite. 
1. short length encoding for length from 0 to 127 (inclusive)
                8   7   6   5   4   3   2   1
              +-------------------------------|
            1 | 0   L   L   L   L   L   L   L |
              +-------------------------------+

	LLLLLLL represents the length of the content

2. long length encoding for length > 127
                8   7   6   5   4   3   2   1
              +---+---------------------------|
            1 | 1 |      0 < n < 127          |
              +-------------------------------+
              +-------------------------------|
            2 | L   L   L   L   L   L   L   L |
              +-------------------------------+
                           ...
              +-------------------------------|
           n+1| L   L   L   L   L   L   L   L |
              +-------------------------------+

	LLLLLLLL represents the length of the content

3. Indefinate length encoding
      The length is represnted with 1 byte containing the value 0x80
                8   7   6   5   4   3   2   1
              +---+---------------------------|
            1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
              +-------------------------------+

      In these cases, the content part is terminated with 2 consecutive NULLs like :
                8   7   6   5   4   3   2   1
              +---+---------------------------|
            1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
              +-------------------------------+
              +---+---------------------------|
            2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
              +-------------------------------+

********************************************************************/
int  xxxx_GetLengthValue(LenInfo_t *o_lenInfo )
{
	unsigned char l_lenChar;
	int  i, l_bytesToRead, rc=XXXX_GOOD;

	o_lenInfo->lenBytesCount=0;

	rc = xxxx_readTheNextByte(&l_lenChar, XXXX_DO_NO_SKIP_NULLS);
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
                                if( l_bytesToRead >XXXX_MAX_LEN_LEN )
                                { 
					printf("\nToo many bytes in the len %d\n",l_bytesToRead);
					rc=XXXX_BAD;
				}
				else
				{
					for ( i=0; i<l_bytesToRead;++i)
					{
	                                        rc = xxxx_readTheNextByte(&l_lenChar, 
                                                        XXXX_DO_NO_SKIP_NULLS);
						sprintf(o_lenInfo->lenBytes+
							o_lenInfo->lenBytesCount*2, 
						       "%02X", l_lenChar);
						o_lenInfo->lenBytesCount++;
						o_lenInfo->lenValue = (o_lenInfo->lenValue<<8)
							| ( l_lenChar & 0xFF );
					}
				}
			}
		}

	*(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2)=0;
	return rc;
}

/********************************************************************
Encoding of the content
~~~~~~~~~~~~~~~~~~~~~~~
The numbering of bits within one octet and the encoding of a binary
 value in an octet structure can be found in the following figure:
                8   7   6   5   4   3   2   1
              +-------------------------------+
              |  most significant byte        | octet 1
              +-------------------------------+
              |                               | octet 2
              +-------------------------------+
              .                               .
              .                               .
              +-------------------------------+
              | least significant byte        | octet n
              +-------------------------------+

bit 8 of octet 1 is the most significant bit (msb)
bit 1 of octet n is the least significant bit (lsb)

********************************************************************/
int  xxxx_GetContentValue(int i_len, ContentInfo_t *o_contentInfo)
{
	unsigned char l_valueChar;
	int i, l_contentValueBytePos=0;

	o_contentInfo->contentPrintableInd = XXXX_PRINTABLE_CONTENT;
	o_contentInfo->contentBytesCount = 0;

	if ( i_len > XXXX_MAX_CONTENT_LEN )
	{
		printf("Content Len Too Big its %d, max allowed is %d \n", i_len, XXXX_MAX_CONTENT_LEN);
		return XXXX_BAD;
	}

	for(i=1; i<=i_len; i++)
	{
		xxxx_readTheNextByte(&l_valueChar, XXXX_DO_NO_SKIP_NULLS);
		sprintf(o_contentInfo->contentBytes+o_contentInfo->contentBytesCount*2,
			   "%02X", l_valueChar);
		o_contentInfo->contentBytesCount++;

		o_contentInfo->contentValue[l_contentValueBytePos++] = l_valueChar;
		if (!isprint(l_valueChar) || l_valueChar == '<' || l_valueChar == '>' )
			o_contentInfo->contentPrintableInd = XXXX_UNPRINTABLE_CONTENT;

	}

	*(o_contentInfo->contentBytes+o_contentInfo->contentBytesCount*2)=0;
	o_contentInfo->contentValue[l_contentValueBytePos] = 0;

	return XXXX_GOOD;
}

/********************************************************************
int  xxxx_GetPaddingLevel(TagInfo_t i_tagInfo, LenInfo_t i_lenInfo, int *o_paddingLevel, char *o_paddingStr )
********************************************************************/
int  xxxx_GetPaddingLevel(TagInfo_t i_tagInfo, LenInfo_t i_lenInfo, int *o_paddingLevel, char *o_paddingStr )
{
	static UnclosedNodesInfo_t l_ucnl[XXXX_MAX_OPEN_NODES];
	static int l_unclosedNodesCount=0, rc= XXXX_GOOD;

	int  l_UndefUcn2Close=0, l_UndefUcnFound=0, i;
	char l_activityArray[28];

	static char l_paddingString[XXXX_MAX_OPEN_NODES*XXXX_PAD_CHARS]={0};
	char l_padSubStr[XXXX_PAD_CHARS+1];


	*o_paddingLevel = l_unclosedNodesCount;
	memset(o_paddingStr, 0, XXXX_MAX_OPEN_NODES*XXXX_PAD_CHARS);

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
		if ( l_unclosedNodesCount > XXXX_MAX_OPEN_NODES )
		{
		   printf("\nToo Many Levels of Nesting\n");
		   rc = XXXX_BAD;
		}
	}

#ifdef PRINTMORE
        //printf("%05.f %2d %2d :", g_InputBytePos,l_unclosedNodesCount, *o_paddingLevel );
	printf("%18s %05.f %2d %2d :",l_activityArray,
	  g_InputBytePos,l_unclosedNodesCount, *o_paddingLevel );
#endif

	/* Create the Padding Pattern String */
	if( l_paddingString[0]==0 )
	{
	     l_padSubStr[0]='|'; 
	     memset(l_padSubStr+1,' ', XXXX_PAD_CHARS-1); 
	     l_padSubStr[XXXX_PAD_CHARS]='\0';

	     memset(l_paddingString, ' ', XXXX_PAD_CHARS );
	     for ( i=1;i<XXXX_MAX_OPEN_NODES ; i++)
	        memcpy(l_paddingString + XXXX_PAD_CHARS*i, l_padSubStr, XXXX_PAD_CHARS );

	     l_padSubStr[0]='+';
             memset(l_padSubStr+1,'-',XXXX_PAD_CHARS-1);
	     l_padSubStr[XXXX_PAD_CHARS]='\0';

             memcpy(l_paddingString+XXXX_PAD_CHARS*(i-1),l_padSubStr, XXXX_PAD_CHARS);
	}

        /* Create the Padding String for the current Tag */
	memcpy(o_paddingStr, 
	    l_paddingString + strlen(l_paddingString) - XXXX_PAD_CHARS * (*o_paddingLevel) ,
	    XXXX_PAD_CHARS * (*o_paddingLevel) );
        if(i_tagInfo.tagType == XXXX_CONSTRUCTED_TAG || (*o_paddingLevel)==1)
            o_paddingStr[(*o_paddingLevel-1) * XXXX_PAD_CHARS] = '|';

	if ( l_unclosedNodesCount > XXXX_MAX_OPEN_NODES || l_unclosedNodesCount < 0 )
		rc = XXXX_BAD;

	return rc;
}

/********************************************************************
The function reads a big buffer from the file, each successive invocation,
the function returns the next byte from the buffer.
********************************************************************/
int   xxxx_readTheNextByte(unsigned char *o_nextByte, InfoType_t i_typeOfInfo )
{
	static unsigned char l_read_Buffer[XXXX_READ_BIN_BUFF_LEN +2 ];
	static int l_cur_pos=0,l_buff_len=0;
	static int buffsRead=0;

	if( g_InpFileEofInd == XXXX_EOF )
		return XXXX_EOF;

	fflush(fpI);

	if ( l_cur_pos == l_buff_len)
	{
		l_buff_len = fread (l_read_Buffer,1, XXXX_READ_BIN_BUFF_LEN, fpI);
		l_cur_pos=0;
	}

/*
        if ( i_typeOfInfo == XXXX_INFO_TYPE_TAG && 
             l_read_Buffer[l_cur_pos]==0x0 && l_read_Buffer[l_cur_pos+1]==0x0)
	{
		l_buff_len = fread (l_read_Buffer,1, XXXX_READ_BIN_BUFF_LEN, fpI);
                g_InputBytePos+=(l_buff_len-l_cur_pos);
		l_cur_pos=0;
	}
*/

	*o_nextByte = l_read_Buffer[l_cur_pos] ;
	if( l_cur_pos == l_buff_len && l_buff_len != XXXX_READ_BIN_BUFF_LEN)
	{
		g_InpFileEofInd = XXXX_EOF;
		return XXXX_EOF;
	}

	l_cur_pos++;
	g_InputBytePos++;

	return XXXX_GOOD;
}

/********************************************************************
Accepts lines like :
   N<...
   T<TagNameOrTagValue TagBytes> L<LenValue>
   T<TagNameOrTagValue TagBytes> L<LenValue> V<ContentValue>
   T<TagNameOrTagValue TagBytes> L<LenValue> Vx<ContentBytes>
********************************************************************/
int     xxxx_PROCencodeAndWrite()
{
	char  l_lineReadFromFile[XXXX_READ_TXT_BUFF_LEN+1];
	int   l_readLinePos,l_tagLevel;
	char  l_typeOfInfoInLine;
        int   l_readLineLen,l_startOfInfo;

	char  l_bufferToWriteToFile[XXXX_READ_BIN_BUFF_LEN+1];
	int   l_bufferPos;

	char  l_tempStr[XXXX_READ_TXT_BUFF_LEN];
	int   l_tempNum;

	int rc=XXXX_GOOD;
	ContentPrintableInd_t  l_typeOfContentInfo;
	TagInfo_t l_tagInfo;
	LenInfo_t l_lenInfo;
	ContentInfo_t l_contentInfo;

	do
	{
	       /* Get Level and Type */
	       l_tagLevel = 0; l_readLinePos = 0; l_bufferPos = 0;

	       rc=xxxx_readTheNextLine(l_lineReadFromFile, &l_readLineLen);
	       if(rc != XXXX_GOOD ) return rc;
  
#ifdef PRINTMORE
               printf("\nEncode read %s",l_lineReadFromFile );
#endif

	       rc = xxxx_GetTknPositionInString(l_lineReadFromFile,l_readLinePos,'<', &l_readLinePos);
	       if( rc != XXXX_GOOD ) return rc;

	       l_tagLevel = (int) (l_readLinePos / XXXX_PAD_CHARS);
	       l_typeOfInfoInLine =  l_lineReadFromFile[l_readLinePos - 1]; 

	       if( l_lineReadFromFile[l_readLinePos - 1]=='N' ) 
	       {
		   strcpy(l_tempStr,"0000");
		   rc = xxxx_writeHexBytesToFile(l_tempStr);
	           if(rc != XXXX_GOOD ) return rc;
               }

	       if( l_lineReadFromFile[l_readLinePos - 1]=='T' ) 
	       {
	           /* Get Tag */
	           rc = xxxx_GetTknPositionInString(l_lineReadFromFile,l_readLinePos,
				     ' ', &l_readLinePos);
	           l_startOfInfo=l_readLinePos+1;
	           rc = xxxx_GetTknPositionInString(l_lineReadFromFile,l_readLinePos,
				     '>', &l_readLinePos);

		   memcpy(l_tempStr,((char *) l_lineReadFromFile) + l_startOfInfo ,
				     l_readLinePos - l_startOfInfo);
                   l_tempStr[l_readLinePos - l_startOfInfo] = 0;

		   rc = xxxx_writeHexBytesToFile(l_tempStr);
	           if(rc != XXXX_GOOD ) return rc;

	           /* Get Length */
	           rc = xxxx_GetTknPositionInString(l_lineReadFromFile,l_readLinePos,
				     '<', &l_readLinePos);
	           l_startOfInfo=l_readLinePos+1;
	           rc = xxxx_GetTknPositionInString(l_lineReadFromFile,l_readLinePos,
				     '>', &l_readLinePos);
		   memcpy(l_tempStr,((char *) l_lineReadFromFile) + l_startOfInfo ,
				     l_readLinePos - l_startOfInfo);
                   l_tempStr[l_readLinePos - l_startOfInfo] = 0;
		   l_tempNum = atoi(l_tempStr);
		   rc=xxxx_GetLengthInHexBytes(l_tempNum, &l_lenInfo);
	           if(rc != XXXX_GOOD ) return rc;

		   rc = xxxx_writeHexBytesToFile(l_lenInfo.lenBytes);
	           if(rc != XXXX_GOOD ) return rc;

	           /* Get Value if any */
	           rc = xxxx_GetTknPositionInString(l_lineReadFromFile,l_readLinePos,
				     '<', &l_readLinePos);
		   if(rc == XXXX_GOOD )
		   {
			l_typeOfContentInfo = ( l_lineReadFromFile[l_readLinePos-1] == 'x' ) ?
			    XXXX_UNPRINTABLE_CONTENT : XXXX_PRINTABLE_CONTENT;
	                l_startOfInfo=l_readLinePos+1;
	                rc = xxxx_GetTknPositionInString(l_lineReadFromFile,l_readLinePos,
			     '>', &l_readLinePos);
		        memcpy(l_tempStr,((char *) l_lineReadFromFile) + l_startOfInfo ,
			     l_readLinePos - l_startOfInfo);
                        l_tempStr[l_readLinePos - l_startOfInfo] = 0;

			rc = xxxx_GetContentInHexBytes(l_tempStr, l_typeOfContentInfo, &l_contentInfo);
	                if(rc != XXXX_GOOD ) return rc;
			
		        rc = xxxx_writeHexBytesToFile(l_contentInfo.contentBytes);
	                if(rc != XXXX_GOOD ) return rc;

		  }
                }

	} while( rc = 1 );

	return XXXX_GOOD;
}

/********************************************************************
see Encoding logic in the comments for xxxx_GetTagValue function
This function uses the reverse logic.
********************************************************************/
int    xxxx_GetTagInHexBytes(int i_tagValue, TagInfo_t *o_tagInfo)
{
	int  i, l_bytesReq, l_128sMuliple;
	int  rc=XXXX_GOOD;

	unsigned char l_tagByte;

	o_tagInfo->tagValue = i_tagValue;
	switch( g_commandLineParams[g_cmdLineParamIndex].executionMode )
	{
	    case XXXX_MODE_SMART_ENCODE_WRITE:
	        o_tagInfo->tagType = g_asn1TagInfoArray[i_tagValue].tagType;
	        o_tagInfo->tagClass = g_asn1TagInfoArray[i_tagValue].tagClass;
	        l_tagByte = (unsigned char )g_asn1TagInfoArray[i_tagValue].tagClass <<6;
	        l_tagByte = l_tagByte | ((unsigned char )g_asn1TagInfoArray[i_tagValue].tagType <<5);
		break;

	    case XXXX_MODE_BASIC_ENCODE_WRITE:
	        o_tagInfo->tagType = XXXX_PRIMITIVE_TAG; 
	        o_tagInfo->tagClass = XXXX_APPLICATION_TAG;
	        l_tagByte = (unsigned char )o_tagInfo->tagClass  <<6;
		l_tagByte = l_tagByte | ((unsigned char )(o_tagInfo->tagType) <<5);
		break;
        }

	o_tagInfo->tagBytesCount=0;

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

		if( l_bytesReq > XXXX_MAX_TAG_LEN )
		{
			printf("\nToo Many bytes required for Tag %d\n",i_tagValue);
			rc = XXXX_BAD;
                }
		for(i=l_bytesReq;i>1;--i)
		{
			l_tagByte = i_tagValue >> (i-1)*7;
			l_tagByte |= 0x80;
			sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, 
				   "%02X", l_tagByte);
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
see Encoding logic in the comments for xxxx_GetLengthValue function
This function uses the reverse logic.
********************************************************************/
int  xxxx_GetLengthInHexBytes(int i_lenValue, LenInfo_t *o_lenInfo )
{
	unsigned char l_lenByte;
	int  i,rc = XXXX_GOOD ;
	int l_bytesReq, l_256sMultiple;

	o_lenInfo->lenValue = i_lenValue;
	o_lenInfo->lenBytesCount=0;

	switch ( i_lenValue )
	{

	case 0x00 :
		sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "%02X", 0x80);
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
			sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2,
				  "%02X", i_lenValue);
			o_lenInfo->lenBytesCount++;
			o_lenInfo->lenType = XXXX_FINITE_LEN_LEN;
		}
		else
		{
			l_256sMultiple = 1;
			for(l_bytesReq=0; l_256sMultiple < i_lenValue ; l_bytesReq++)
				l_256sMultiple *= 256;

                        if ( i_lenValue % 256 == 0 ) l_bytesReq++;
			l_lenByte = l_bytesReq;
			l_lenByte |= 0x80;
			sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2,
				 "%02X", l_lenByte);
			o_lenInfo->lenBytesCount++;

			if( l_bytesReq*2 > XXXX_MAX_LEN_LEN )
			{
			       rc = XXXX_BAD;
			       break;
                        }
			for(i=l_bytesReq;i>0;--i)
			{
				l_lenByte =  i_lenValue >> (i-1)*8;
				sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2,
				 	"%02X", l_lenByte);
				o_lenInfo->lenBytesCount++;
			}
		}
	}

	*(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2)=0;
	return rc;
}

/********************************************************************
see Encoding logic in the comments for xxxx_GetContentValue function
This function uses the reverse logic.
********************************************************************/
int   xxxx_GetContentInHexBytes(char *i_contentValue,ContentPrintableInd_t i_contentPrintableInd,
			    ContentInfo_t *o_contentInfo)
{
	int  i,rc = XXXX_GOOD ;

	o_contentInfo->contentPrintableInd = i_contentPrintableInd;
	if( i_contentPrintableInd == XXXX_PRINTABLE_CONTENT )
	{
	   strcpy(o_contentInfo->contentValue, i_contentValue);
	   o_contentInfo->contentBytesCount=strlen(i_contentValue)*2;
	   for(i=0;i<strlen(i_contentValue);++i)
		 sprintf(o_contentInfo->contentBytes+i*2,"%02X",i_contentValue[i]);
           *(o_contentInfo->contentBytes+i*2) = 0;
	}
	else
	{
	   /* TODO: Not populated : o_contentInfo->contentValue */
	   strcpy(o_contentInfo->contentBytes,i_contentValue);
	   o_contentInfo->contentBytesCount=strlen(i_contentValue);
	}

	return rc;
}


/********************************************************************
int            xxxx_readTheNextLine(char *o_lineRead, int *l_readLineLen);
********************************************************************/
int            xxxx_readTheNextLine(char *o_lineRead, int *o_readLineLen)
{
	int rc=XXXX_GOOD;

        fflush(fpI);
	memset(o_lineRead,0, XXXX_READ_TXT_BUFF_LEN);
	*o_readLineLen=0;

        if( fgets (o_lineRead,XXXX_READ_TXT_BUFF_LEN, fpI) == NULL )
	{
		rc = XXXX_BAD;
        }
	else
	{
		*o_readLineLen =  strlen(o_lineRead);
		g_InputLinesCount++;
	}

	if(feof(fpI)) 
		rc = g_InpFileEofInd = XXXX_EOF;

	return rc;
}
/********************************************************************
Returns the position of character <i_tkn> in the string <i_string> , 
  starting from the position <i_strt>.
If successful <o_tknPos> the the token's index, else its -1
********************************************************************/
int  xxxx_GetTknPositionInString(char *i_string,int i_strt,char i_tkn, int *o_tknPos)
{
 int rc=XXXX_GOOD;

 for(*o_tknPos=i_strt;*o_tknPos<=strlen(i_string) && i_string[*o_tknPos]!= i_tkn;++(*o_tknPos));

 if(i_string[*o_tknPos]!= i_tkn)
 {
  *o_tknPos=-1;
  rc = XXXX_BAD;
 }

 return rc;
}
/********************************************************************
int  xxxx_writeHexBytesToFile(char *i_hexByteString);
********************************************************************/
int  xxxx_writeHexBytesToFile(char *i_hexByteString)
{
    int i,rc = XXXX_GOOD;
    unsigned char ch;
    char  tmpStr[3];
    unsigned char l_byteStream[XXXX_READ_BIN_BUFF_LEN];
    int l_lenOfOutputBuffer;

    memset(tmpStr,0,sizeof(tmpStr));
    l_lenOfOutputBuffer =  strlen(i_hexByteString)/2;
    l_byteStream[0]=NULL;


    for(i=0;i< l_lenOfOutputBuffer;i++)
    {
	 tmpStr[0]=i_hexByteString[2*i]; 
	 tmpStr[1]=i_hexByteString[2*i+1]; 
	 l_byteStream[i] = (unsigned char) strtoul(tmpStr,NULL,16);
    }

#ifdef PRINTMORE
               printf("  Encode wrote %d of (%s) ",i,  i_hexByteString );
#endif

    rc = fwrite(l_byteStream, l_lenOfOutputBuffer,1, fpO);
    if ( rc != l_lenOfOutputBuffer ) 
      rc= XXXX_BAD;

    return XXXX_GOOD;
}
