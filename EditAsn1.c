/*
  This one expects INFILE & OUTFILE as its 2 arguments
  INFILE is the output of osstlv
  it also has code to Analyse Tag & Value fields 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XXXX_GOOD 0
#define XXXX_BAD  -1

#define XXXX_MAX_REC_LEN 180

FILE *fpI, *fpO;

int  xxxx_EXECinit(int i_argc, char *i_argv[]);
int  xxxx_ParseAndBinarify(char *i_inRec, char *o_outBuf, int * o_bufLen);
int  xxxx_EXECterm();
int  xxxx_AnalyseTagValue( unsigned char *i_tagStr);
int  xxxx_AnalyseLengthValue( unsigned char *i_lenStr);
        


int main(int argc, char *argv[]) {
	char infile_record [ XXXX_MAX_REC_LEN ];
	char outfile_buffer[ XXXX_MAX_REC_LEN ];
	int  outfile_buffLen;
	int rc;

	rc = xxxx_EXECinit(argc, argv);
        if ( rc ) {
            printf("Fatal Error in xxxx_EXECinit\n");
            return XXXX_BAD;
        }
        
	while ( !feof(fpI)) {
		fgets(infile_record, XXXX_MAX_REC_LEN, fpI);
		rc = xxxx_ParseAndBinarify(infile_record, outfile_buffer, &outfile_buffLen);
                if ( rc ) {
                     printf("Fatal Error in xxxx_ParseAndBinarify\n");
                     return XXXX_BAD;
                }

  		fwrite(outfile_buffer, outfile_buffLen, 1, fpO);
                memset(infile_record,0, 10);
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

	if ( i_argc != 3 )
		return XXXX_BAD;

	fpO = fopen(i_argv[2], "wb");
	if ( fpO == NULL )
		return XXXX_BAD;

	fpI = fopen(i_argv[1], "rt");
	if ( fpI == NULL )
		return XXXX_BAD;
        
        /** Skip the 1st 4 Informational/Blank Lines **/
        fgets(temp_headerRec, XXXX_MAX_REC_LEN, fpI);
        fgets(temp_headerRec, XXXX_MAX_REC_LEN, fpI);
        fgets(temp_headerRec, XXXX_MAX_REC_LEN, fpI);
        fgets(temp_headerRec, XXXX_MAX_REC_LEN, fpI);

	return XXXX_GOOD;
}
        
int xxxx_EXECterm()
{
	fclose(fpI);
	fclose(fpO);

	return XXXX_GOOD;
}

#define PRINT_UNTILL_SPACE_OR_EOLN  { \
               l_thisFieldIndex = 0; \
               while ( i< l_inpStrLen -1  && i_inRec[i]!=' ' ) { \
                  memcpy(l_next2BytesStr, i_inRec +i  , 2 ); \
                  l_next2BytesLong = strtoul(l_next2BytesStr, (char **)NULL, 16)  ; \
                  o_outBuf[*o_bufLen] = l_next2BytesLong ; \
                  l_thisField[l_thisFieldIndex]= l_next2BytesLong; \
                  printf("%s ", l_next2BytesStr );  \
                  (*o_bufLen)++; \
                  l_thisFieldIndex++; \
                   i+=2; \
               } \
             }

int xxxx_ParseAndBinarify(char *i_inRec, char *o_outBuf, int * o_bufLen)
{
	int  i;
        int l_inpStrLen, l_outStrLen;
	char l_next2BytesStr[3]={ 0,0,0	};
        unsigned long l_next2BytesLong;

        int l_thisFieldIndex; 
        unsigned char l_thisField[XXXX_MAX_REC_LEN];

        memset(l_next2BytesStr, 0, sizeof(l_next2BytesStr));
	l_inpStrLen = strlen( i_inRec);
	*o_bufLen = 0;

        if(l_inpStrLen <=1 )  return XXXX_GOOD;

	printf("\nRead Record : (%.*s) of %d\n", l_inpStrLen-1, i_inRec, l_inpStrLen );

	/* Skip leading Blanks */
	for(i=0; i_inRec[i]==' ' && i < l_inpStrLen-1 ;i++);
	if (i==l_inpStrLen) return XXXX_GOOD;

        if ( i_inRec[i]=='0' &&  i_inRec[i+1]=='0' && i_inRec[i+3] == '0' && i_inRec[i+4] == '0' ) {
              printf("  [NULL] : ") ;

              PRINT_UNTILL_SPACE_OR_EOLN ; 
              i++;
              PRINT_UNTILL_SPACE_OR_EOLN ; 

              return XXXX_GOOD;
        }


        printf("  Tag : ") ; 
        PRINT_UNTILL_SPACE_OR_EOLN ; 
        //xxxx_AnalyseTagValue( l_thisField );
        i++;

        printf("  Len : ") ;
        PRINT_UNTILL_SPACE_OR_EOLN;
        xxxx_AnalyseLengthValue( l_thisField );

        return XXXX_GOOD;

        if( i == l_inpStrLen-1  ) return XXXX_GOOD;           
        i++;

        printf("  Val : ") ;
        PRINT_UNTILL_SPACE_OR_EOLN;

        return XXXX_GOOD;
}

char TagClassIdentifierNameArray[4][20] = { "Universal", "Application","Context Specific", "Private" };

int  xxxx_AnalyseTagValue( unsigned char *i_tagStr) {
  unsigned int retval_Tag;
 
  retval_Tag =0;

  printf ( "\tTag Class Id : %s, P/C %s , Tags in %s   ", 
                 TagClassIdentifierNameArray[ ( *i_tagStr & 0xC0)>>6],  
                 ((*i_tagStr & 0x20)>>5    ? "Constructed" : "Primitive" ),  
                 ((*i_tagStr & 0x1F)==0x1F ? "Many Bytes " :"One Byte"   ));

 if ( (*i_tagStr & 0x1F)!=0x1F )  {
    retval_Tag = (*i_tagStr & 0x1F);
 }
 else
 {
   do {
       i_tagStr ++;
       retval_Tag = ( retval_Tag <<7 ) | ( *i_tagStr & 0x7F );
   } while (  (*i_tagStr & 0x80 ) ) ;

 }

 return retval_Tag;
}


int  xxxx_AnalyseLengthValue( unsigned char *i_lenStr) {
  unsigned int retval_Len; 
  int i, l_bytesToRead;
  
 if ( *i_lenStr == 0x80 )  {
    retval_Len = -1;
 }
 else
   if ( (*i_lenStr & 0x7F)!=0x80 )  {
      retval_Len = (*i_lenStr & 0x7F);
   }
   else
   {
     l_bytesToRead=( *i_lenStr & 0x7F ) ;
     i_lenStr ++;

     for ( i=0; i<l_bytesToRead;++i) {
          retval_Len = ( retval_Len <<7 ) | ( *i_lenStr & 0x7F );
     }
   
   }
   
 return retval_Len;
}
