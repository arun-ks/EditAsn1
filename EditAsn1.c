/********************************************************************
Prints like :
  T<TransferBatch x61> L<0 x80> 
   T<BatchControlInfo x64> L<0 x80> 
     T<Sender x5F8144> L<5 x05> V<AUTPT x4155545054>
     T<Recipient x5F8136> L<5 x05> V<EUR01 x4555523031>
     T<FileSequenceNumber x5F6D> L<5 x05> V<00001 x3030303031>
     T<FileCreationTimeStamp x7F6C> L<0 x80> 
       T<LocalTimeStamp x50> L<14 x0E> V<19981028020000 x3139393831303238303230303030>
       T<UtcTimeOffset x5F8167> L<5 x05> V<+0100 x2B30313030>

Bugs : Problem with Indentation of actual-length Nodes
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef USETAGARRAY
  #include "asn1TagArray.h"
#endif

#define XXXX_GOOD              0
#define XXXX_BAD               -1
#define XXXX_EOF               -1

#define XXXX_MAX_REC_LEN       18
#define XXXX_READ_BUFF_LEN     100
#define XXXX_PAD_CHAR          ' ' 
#define XXXX_MAX_CONTENT_LEN   300
#define XXXX_MAX_TAG_LEN       10
#define XXXX_MAX_LEN_LEN       10

typedef struct {
	unsigned int  tagValue;
	char tagBytes[XXXX_MAX_TAG_LEN];
	enum { XXXX_PRIMITIVE_TAG, XXXX_CONSTRUCTED_TAG } tagType;
	enum { XXXX_UNIVERSAL_TAG, XXXX_APPLICATION_TAG, XXXX_CONTEXT_SPEC_TAG, XXXX_PRIVATE_TAG } tagClass;
	unsigned int  tagInOneByteInd;
	int  tagBytesCount;
} TagInfo_t ;

typedef struct {
	unsigned int lenValue;
	char lenBytes[XXXX_MAX_LEN_LEN];
	enum { XXXX_UNDEF_LEN_LEN, XXXX_FINITE_LEN_LEN } lenType;
	int  lenBytesCount;
} LenInfo_t;

typedef struct {
	char contentValue[XXXX_MAX_CONTENT_LEN];
	char contentBytes[XXXX_MAX_CONTENT_LEN];
	int  contentBytesCount;
	enum { XXXX_PRINTABLE_CONTENT, XXXX_UNPRINTABLE_CONTENT }  contentPrintableInd;
} ContentInfo_t;

int            xxxx_EXECinit(int i_argc, char *i_argv[]);
int            xxxx_EXECterm();
unsigned char  xxxx_readTheNextByte();

int            xxxx_PROCdecodeAndPrint();
int            xxxx_GetTagValue(TagInfo_t *o_tagInfo);
int            xxxx_GetLengthValue(LenInfo_t *o_lenInfo) ;
int            xxxx_GetContentValue(int i_len, ContentInfo_t *o_contentInfo);

int            xxxx_PROCencodeAndWrite();

FILE *fpI, *fpO;
int   g_InpFileEofInd =0;
enum { XXXX_MODE_DECODE_PRINT,XXXX_MODE_ENCODE_WRITE } g_applicationMode;


int main(int argc, char *argv[])
{
	int  rc;

	rc = xxxx_EXECinit(argc, argv);
	if ( rc )
	{
		printf("Fatal Error in xxxx_EXECinit\n");
		return XXXX_BAD;
	}

	if( g_applicationMode == XXXX_MODE_DECODE_PRINT)
		rc = xxxx_PROCdecodeAndPrint();
	else
		rc = xxxx_PROCencodeAndWrite();

	if ( rc )
	{
		printf("Fatal Error in Processing\n");
		return XXXX_BAD;
	}

	rc = xxxx_EXECterm();
	if ( rc )
	{
		printf("Fatal Error in xxxx_EXECterm\n");
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
	char l_execUsageInfo[] = "\n\nUsage\n    %s -d <ASN.1 File>\n" \
                                 "    %s -e <Ascii TLV File> <ASN.1 File>\n\n";

	if ( i_argc < 2 )
	{
		printf(l_execUsageInfo, i_argv[0], i_argv[0]);
		return XXXX_BAD;
	}

	if ( !strcmp(i_argv[1],"-d") )
		g_applicationMode = XXXX_MODE_DECODE_PRINT;
	else
		if ( !strcmp(i_argv[1],"-e") )
			g_applicationMode = XXXX_MODE_ENCODE_WRITE;
		else 
		{
			printf(l_execUsageInfo, i_argv[0], i_argv[0]);
			return XXXX_BAD;
		}


	if( (g_applicationMode == XXXX_MODE_DECODE_PRINT  && i_argc < 3 ) ||
	    (g_applicationMode == XXXX_MODE_ENCODE_WRITE  && i_argc < 4 ))
	{
		printf(l_execUsageInfo, i_argv[0], i_argv[0]);
		return XXXX_BAD;
	}

	fpI = fopen(i_argv[2], "rb");
	if ( fpI == NULL )
		return XXXX_BAD;
	setvbuf(fpI, NULL,_IONBF, 0);

	if( g_applicationMode == XXXX_MODE_ENCODE_WRITE )
	{
		fpO = fopen(i_argv[3], "wb");
		if ( fpO == NULL )
			return XXXX_BAD;
	}

	return XXXX_GOOD;
}

/********************************************************************
int xxxx_EXECterm()
********************************************************************/
int xxxx_EXECterm()
{
	fclose(fpI);

	if(g_applicationMode == XXXX_MODE_ENCODE_WRITE )
		fclose(fpO);

	return XXXX_GOOD;
}


/********************************************************************
int xxxx_PROCdecodeAndPrint()
********************************************************************/
int xxxx_PROCdecodeAndPrint()
{
	int           l_unclosedConstructs =0;
	TagInfo_t     l_tagInfo;
	LenInfo_t     l_lenInfo;
	ContentInfo_t l_contentInfo;
	int           rc;

	while ( 1 )
	{
		rc = xxxx_GetTagValue(&l_tagInfo);
		if ( rc == XXXX_EOF )
			break;

		rc = xxxx_GetLengthValue(&l_lenInfo);

		if( l_tagInfo.tagType == 0 && l_lenInfo.lenValue == 0 )
		{
			printf ("\n%*.c N<00 x0000>", l_unclosedConstructs*2, XXXX_PAD_CHAR );
			l_unclosedConstructs --;
		}
		else
		{
			printf("\n%*.c T<%s x%s> L<%d x%s> ", l_unclosedConstructs*2, XXXX_PAD_CHAR,
#ifdef USETAGARRAY
				    ( l_tagInfo.tagValue  <= XXXX_MAX_TAG_NUMBER ) ?
		                       TagArray[l_tagInfo.tagValue]: "SomeThing Wrong",
#else
                                      "Tag", 
#endif
				    l_tagInfo.tagBytes,
				    l_lenInfo.lenValue, l_lenInfo.lenBytes);
			if( l_lenInfo.lenType == XXXX_UNDEF_LEN_LEN || l_tagInfo.tagType == XXXX_CONSTRUCTED_TAG  )
				l_unclosedConstructs++;
		}

		if( l_lenInfo.lenValue != 0 && l_lenInfo.lenType == XXXX_FINITE_LEN_LEN  && 
		    l_tagInfo.tagType == XXXX_PRIMITIVE_TAG)
		{
			rc = xxxx_GetContentValue(l_lenInfo.lenValue, &l_contentInfo );
			if( l_contentInfo.contentPrintableInd == XXXX_PRINTABLE_CONTENT )
				printf("V<%s x%s>", l_contentInfo.contentValue, l_contentInfo.contentBytes);
			else
				printf("Vx<x%s>", l_contentInfo.contentBytes );
		}
	}

	return XXXX_GOOD;
}

/********************************************************************
int  xxxx_GetTagValue(TagInfo_t *o_tagInfo)
********************************************************************/
int  xxxx_GetTagValue(TagInfo_t *o_tagInfo)
{
	unsigned char l_tagChar;

	o_tagInfo->tagBytesCount=0;
	o_tagInfo->tagValue=0;
	o_tagInfo->tagType=0;

	l_tagChar = xxxx_readTheNextByte();
	if( g_InpFileEofInd == XXXX_EOF )
		return XXXX_EOF ;

	sprintf(o_tagInfo->tagBytes+o_tagInfo->tagBytesCount*2, "%02X", l_tagChar);
	o_tagInfo->tagBytesCount++;

	o_tagInfo->tagType = (l_tagChar & 0x20)>>5 ? XXXX_CONSTRUCTED_TAG : XXXX_PRIMITIVE_TAG ;
	o_tagInfo->tagClass =  (l_tagChar & 0xC0)>>6;
	o_tagInfo->tagInOneByteInd = ( (l_tagChar & 0x1F) == 0x1F ) ? 0 : 1;

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

#ifdef USETAGARRAY
	if ( o_tagInfo->tagValue >= XXXX_MAX_TAG_NUMBER ) 
	   o_tagInfo->tagValue = XXXX_MAX_TAG_NUMBER;
#endif

	return o_tagInfo->tagValue;
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

	if ( l_lenChar == 0x80 )
	{
		o_lenInfo->lenValue = 0;        /* Undef */
		o_lenInfo->lenType = XXXX_UNDEF_LEN_LEN ;
	}
	else
	{
		o_lenInfo->lenType = XXXX_FINITE_LEN_LEN ;
		if ( (l_lenChar & 0x7F) != 0x80 )
			o_lenInfo->lenValue = (l_lenChar & 0x7F);
		else
		{
			l_bytesToRead=( l_lenChar & 0x7F ) ;
			for ( i=0; i<=l_bytesToRead;++i)
			{
				l_lenChar = xxxx_readTheNextByte();
				sprintf(o_lenInfo->lenBytes+o_lenInfo->lenBytesCount*2, "%02X", l_lenChar);
				o_lenInfo->lenBytesCount++;
				o_lenInfo->lenValue = ( o_lenInfo->lenValue <<7 ) | ( l_lenChar & 0x7F );
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
	int i, l_retvalBytePos=0;

	o_contentInfo->contentPrintableInd = XXXX_PRINTABLE_CONTENT;
	o_contentInfo->contentBytesCount = 0;
	for(i=1; i<=i_len; i++)
	{
		l_valueChar = xxxx_readTheNextByte();
		sprintf(o_contentInfo->contentBytes+o_contentInfo->contentBytesCount*2, "%02X", l_valueChar);
		o_contentInfo->contentBytesCount++;

		o_contentInfo->contentValue[l_retvalBytePos++] = l_valueChar;
		if (!isprint(l_valueChar) )
			o_contentInfo->contentPrintableInd = XXXX_UNPRINTABLE_CONTENT;

	}

	*(o_contentInfo->contentBytes+o_contentInfo->contentBytesCount*2)=0;
	o_contentInfo->contentValue[l_retvalBytePos] = 0;

	return XXXX_GOOD;
}


/********************************************************************
int     xxxx_PROCencodeAndWrite()
********************************************************************/
int     xxxx_PROCencodeAndWrite()
{
	return XXXX_GOOD;
}

/********************************************************************
unsigned char xxxx_readTheNextByte() 
********************************************************************/
unsigned char xxxx_readTheNextByte()
{
	unsigned char retval_byte;
	static unsigned char read_Buffer[XXXX_READ_BUFF_LEN +2 ];
	static int cur_pos=0,buff_len=0;

	if( g_InpFileEofInd == XXXX_EOF )
		return XXXX_EOF;

	fflush(fpI);

	if ( cur_pos == buff_len )
	{
		buff_len = fread (read_Buffer,1, XXXX_READ_BUFF_LEN, fpI);
		cur_pos=0;
	}

	retval_byte = read_Buffer[ cur_pos ] ;
	if( cur_pos == buff_len && buff_len != XXXX_READ_BUFF_LEN)
	{
		g_InpFileEofInd = XXXX_EOF;
		return XXXX_EOF;
	}

	cur_pos++;
	//	printf("%02X", retval_byte );
	return retval_byte;
}
