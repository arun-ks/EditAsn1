/*
 Prints tags like :
  T<Notification x62> L<-1 x80> 
     T<Sender x5F8144> L<5 x05> V<AUTPT x4155545054>
     T<Recipient x5F8136> L<5 x05> V<EUR01 x4555523031>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "asn1TagArray.h"

#undef  MORE_PRINTFS
#define XXXX_GOOD 0
#define XXXX_BAD  -1
#define XXXX_EOF  -1

#define XXXX_MAX_REC_LEN       18
#define XXXX_READ_BUFF_LEN     100
#define XXXX_PAD_CHAR          ' ' 
#define XXXX_MAX_CONTENT_LEN   300

FILE *fpI, *fpO;
int   g_InpFileEofInd =0;
enum { XXXX_MODE_DECODE_PRINT,XXXX_MODE_ENCODE_WRITE } g_applicationMode;

int            xxxx_EXECinit(int i_argc, char *i_argv[]);
int            xxxx_EXECterm();
unsigned char  xxxx_readTheNextByte();

int            xxxx_PROCdecodeAndPrint();
int            xxxx_GetTagValue(char *o_tagBytes, int *o_tagType);
int            xxxx_GetLengthValue(char *o_lenBytes) ;
char          *xxxx_GetContentValue(int i_len, char *o_contentBytes, int *o_printableInd) ;

int            xxxx_PROCencodeAndWrite();


int main(int argc, char *argv[]) 
{
	int  rc;
	int  applMode;

	rc = xxxx_EXECinit(argc, argv);
	if ( rc ) 
	{
		printf("Fatal Error in xxxx_EXECinit\n");
		return XXXX_BAD;
	}

	if( applMode == XXXX_MODE_DECODE_PRINT)
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
	char temp_headerRec[XXXX_MAX_REC_LEN];
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
	int  l_len, l_tag;
	char l_content[XXXX_MAX_CONTENT_LEN];
	char l_tagBytes[10], l_lenBytes[10], l_contentBytes[XXXX_MAX_CONTENT_LEN];
	int unclosedConstructs =0;
	int l_tagType, l_contentPrintableInd;

	while ( g_InpFileEofInd != XXXX_EOF ) 
	{
		l_tag = xxxx_GetTagValue(l_tagBytes,  &l_tagType);
		if ( l_tag == XXXX_BAD )
			break;

		l_len = xxxx_GetLengthValue(l_lenBytes);

		if( l_tag == 0 && l_len == 0 ) 
		{
			printf ("\n%*.c N<00 x0000>", unclosedConstructs*2, XXXX_PAD_CHAR );
			unclosedConstructs --;
		}
		else
		{
			printf("\n%*.c T<%s x%s> L<%d x%s> ", unclosedConstructs*2, XXXX_PAD_CHAR,
			    TagArray[l_tag], l_tagBytes,  l_len, l_lenBytes);
			if( l_len == -1 || l_tagType == 1  ) 
				unclosedConstructs++;
		}

		if(l_len > 0 && l_tagType == 0) 
		{
			strcpy(l_content, 
			    xxxx_GetContentValue(l_len, l_contentBytes, &l_contentPrintableInd)) ;
			if( l_contentPrintableInd )
				printf("V<%s x%s>", l_content, l_contentBytes);
			else
				printf("Vx<x%s>", l_contentBytes );
		}
	}

	return XXXX_GOOD;
}

/********************************************************************
int  xxxx_GetTagValue(char *o_tagBytes, int *o_tagType) 
********************************************************************/
int  xxxx_GetTagValue(char *o_tagBytes, int *o_tagType)
{
	unsigned int retval_Tag;
	unsigned char l_tagChar;
	int  l_tagBytePos=0;
#ifdef MORE_PRINTFS
        static char TagClassIdentifierNameArray[4][20] = 
           { "Universal", "Application","Context Specific", "Private" };
#endif

	retval_Tag =0;
	*o_tagType=0;

	l_tagChar = xxxx_readTheNextByte();
	if( g_InpFileEofInd == XXXX_EOF )
		return -1 ;

	sprintf(o_tagBytes+l_tagBytePos, "%02X", l_tagChar);
	l_tagBytePos+=2;

	*o_tagType = (l_tagChar & 0x20)>>5 ? 1 :0 ;

#ifdef MORE_PRINTFS
	printf ( "\nTag Class Id : %s, P/C %s , Tags in %s   ",
	    TagClassIdentifierNameArray[ ( l_tagChar & 0xC0)>>6],
	    ((l_tagChar & 0x20)>>5    ? "Constructed" : "Primitive" ),
	    ((l_tagChar & 0x1F)==0x1F ? "Many Bytes " :"One Byte"   ));
#endif

	if ( (l_tagChar & 0x1F)!=0x1F )  
		retval_Tag = (l_tagChar & 0x1F);
	else
	{
		do 
		{
			l_tagChar = xxxx_readTheNextByte();
			sprintf(o_tagBytes+l_tagBytePos, "%02X", l_tagChar);
			l_tagBytePos+=2;
			retval_Tag = ( retval_Tag <<7 ) | ( l_tagChar & 0x7F );
		} while (  (l_tagChar & 0x80 ) ) ;

	}

	*(o_tagBytes+l_tagBytePos) =  0;

	return retval_Tag;
}

/********************************************************************
int  xxxx_GetLengthValue(char *o_lenBytes )
********************************************************************/
int  xxxx_GetLengthValue(char *o_lenBytes )
{
	unsigned int retval_Len;
	unsigned char l_lenChar;
	int  i, l_bytesToRead;
	int  l_lenBytePos=0;

	l_lenChar = xxxx_readTheNextByte();
	sprintf(o_lenBytes+l_lenBytePos, "%02X", l_lenChar);
	l_lenBytePos+=2;

	if ( l_lenChar == 0x80 )  
	{
		retval_Len = -1;        /* Undef */
	}
	else
		if ( (l_lenChar & 0x7F) != 0x80 )  
			retval_Len = (l_lenChar & 0x7F);
		else
		{
			l_bytesToRead=( l_lenChar & 0x7F ) ;
			for ( i=0; i<=l_bytesToRead;++i) 
			{
				l_lenChar = xxxx_readTheNextByte();
				sprintf(o_lenBytes+l_lenBytePos, "%02X", l_lenChar);
				l_lenBytePos+=2;
				retval_Len = ( retval_Len <<7 ) | ( l_lenChar & 0x7F );
			}
		}

	*(o_lenBytes+l_lenBytePos)=0;
	return retval_Len;
}

/********************************************************************
char *xxxx_GetContentValue(int i_len, char *o_contentBytes) 
********************************************************************/
char *xxxx_GetContentValue(int i_len, char *o_contentBytes, int *o_printableInd)
{
	unsigned char l_valueChar;
	static char retval_Content[XXXX_MAX_CONTENT_LEN];
	int  l_contentBytePos=0, l_retvalBytePos=0;
	int i;

	*o_printableInd = 1;
	for(i=1; i<=i_len; i++) 
	{
		l_valueChar = xxxx_readTheNextByte();
		sprintf(o_contentBytes+l_contentBytePos, "%02X", l_valueChar);
		l_contentBytePos+=2;
		retval_Content[l_retvalBytePos++] = l_valueChar;
		if (!isprint(l_valueChar) )
			*o_printableInd = 0;

	}
	*(o_contentBytes+l_contentBytePos)=0;
	retval_Content[l_retvalBytePos] = 0;

	return retval_Content;
}


/********************************************************************
int            xxxx_PROCencodeAndWrite()
********************************************************************/
int            xxxx_PROCencodeAndWrite()
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
