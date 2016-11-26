/*
  Equivalent to ossDecode , does not do anything else
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "asn1TagArray.h"

#define XXXX_GOOD 0
#define XXXX_BAD  -1
#define XXXX_EOF  -1

#define XXXX_MAX_REC_LEN 180

FILE *fpI, *fpO;
int   g_InpFileEofInd =0;

int  xxxx_EXECinit(int i_argc, char *i_argv[]);
int  xxxx_EXECterm();
int  xxxx_AnalyseTagValue(int *o_tagType);
int  xxxx_AnalyseLengthValue() ;
int  xxxx_AnalyseContentValue(int i_len) ;

unsigned char  xxxx_readTheNextByte();


int main(int argc, char *argv[]) {
	char outfile_buffer[ XXXX_MAX_REC_LEN ];
	int  outfile_buffLen;
	int  rc;
	int  l_len, l_tag;
	int unclosedConstructs =0;
	int l_tagType;

	rc = xxxx_EXECinit(argc, argv);
	if ( rc ) {
		printf("Fatal Error in xxxx_EXECinit\n");
		return XXXX_BAD;
	}

	while ( !feof(fpI) ) {
		l_tag = xxxx_AnalyseTagValue(&l_tagType);
		if ( l_tag == XXXX_BAD ) break;

		l_len = xxxx_AnalyseLengthValue();

		if( l_tag == 0 && l_len == 0 ) {
			printf ("\n%*.s <Null Value> %02X %02X", unclosedConstructs*2, " ", l_tag, l_len);
			unclosedConstructs --;
		}
		else
			if( l_len == -1 || l_tagType == 1  ) {
				printf("\n%*.s <%s> Len x80 ", unclosedConstructs*2, " ",TagArray[l_tag] );
				unclosedConstructs++;
			}
			else
				printf("\n%*.s <%s> Len %02X , Value : ", unclosedConstructs*2, 
                                       " ", TagArray[l_tag], l_len);

		if(l_len > 0 && l_tagType == 0) {
			xxxx_AnalyseContentValue(l_len) ;
		}
	}

	rc = xxxx_EXECterm();
	if ( rc ) {
		printf("Fatal Error in xxxx_EXECterm\n");
		return XXXX_BAD;
	}

	printf("\n\n Thanks \n");
	return XXXX_GOOD;
}

int xxxx_EXECinit(int i_argc, char *i_argv[])
{
	char temp_headerRec[XXXX_MAX_REC_LEN];

	/* if ( i_argc != 3 ) */
	if ( i_argc != 2 )
		return XXXX_BAD;

	fpI = fopen(i_argv[1], "rb");
	if ( fpI == NULL )
		return XXXX_BAD;
	setvbuf(fpI, NULL,_IONBF, 0);

        /*
	fpO = fopen(i_argv[2], "wb");
	if ( fpO == NULL )
		return XXXX_BAD;
        */

	return XXXX_GOOD;
}

int xxxx_EXECterm()
{
	fclose(fpI);
        /*
	fclose(fpO);
        */

	return XXXX_GOOD;
}

char TagClassIdentifierNameArray[4][20] = { 
	"Universal", "Application","Context Specific", "Private" };

int  xxxx_AnalyseTagValue(int *o_tagType) {
	unsigned int retval_Tag;
	unsigned char l_tagChar;

	retval_Tag =0;
	*o_tagType=0;

	l_tagChar = xxxx_readTheNextByte();
	if( g_InpFileEofInd == XXXX_EOF )
		return -1 ;

	*o_tagType = (l_tagChar & 0x20)>>5 ? 1 :0 ;

	/*
        printf ( "\nTag Class Id : %s, P/C %s , Tags in %s   ", 
                 TagClassIdentifierNameArray[ ( l_tagChar & 0xC0)>>6],  
                 ((l_tagChar & 0x20)>>5    ? "Constructed" : "Primitive" ),  
                 ((l_tagChar & 0x1F)==0x1F ? "Many Bytes " :"One Byte"   ));
       */

	if ( (l_tagChar & 0x1F)!=0x1F )  {
		retval_Tag = (l_tagChar & 0x1F);
	}
	else
	{
		do {
			l_tagChar = xxxx_readTheNextByte();
			retval_Tag = ( retval_Tag <<7 ) | ( l_tagChar & 0x7F );
		} while (  (l_tagChar & 0x80 ) ) ;

	}

	return retval_Tag;
}


int  xxxx_AnalyseLengthValue( ) {
	unsigned int retval_Len;
	unsigned char l_lenChar;
	int i, l_bytesToRead;

	l_lenChar = xxxx_readTheNextByte();

	if ( l_lenChar == 0x80 )  {
		retval_Len = -1;        /* Undef */
	}
	else
		if ( (l_lenChar & 0x7F)!=0x80 )  {
			retval_Len = (l_lenChar & 0x7F);
		}
		else
		{
			l_bytesToRead=( l_lenChar & 0x7F ) ;
			for ( i=0; i<=l_bytesToRead;++i) {
				l_lenChar = xxxx_readTheNextByte();
				retval_Len = ( retval_Len <<7 ) | ( l_lenChar & 0x7F );
			}
		}

	return retval_Len;
}


int  xxxx_AnalyseContentValue(int i_len)  {
	unsigned char l_value;
	char l_valuePrintMask[5];
	int i;

	for(i=1; i<=i_len; i++) {
		l_value = xxxx_readTheNextByte();
		strcpy (l_valuePrintMask, isprint(l_value) ? "%c" : " @%02x ");
		printf(l_valuePrintMask, l_value);
	}
}

unsigned char xxxx_readTheNextByte()
{
	unsigned char retval_byte;

	if( g_InpFileEofInd == XXXX_EOF ) {
		return XXXX_EOF;
	}

	fflush(fpI);
	retval_byte = (unsigned char )fgetc(fpI);

	if( feof( fpI ) ) {
		g_InpFileEofInd = XXXX_EOF;
		return XXXX_EOF;
	}

	// printf("%02x", retval_byte );
	return retval_byte;
}
