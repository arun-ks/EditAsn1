/* 
  This one expects INFILE & OUTFILE as its 2 arguments 
  INFILE is the output of osstlv
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



char triplet_name[5][10] = { "Tag", "Length", "Value", "Null", "Oops !"  };
int xxxx_ParseAndBinarify(char *i_inRec, char *o_outBuf, int * o_bufLen)
{
	int  i;
        int l_inpStrLen, l_outStrLen;
	int  l_tripletCounter;
	char l_next2BytesStr[3]={ 0,0,0	};
        unsigned long l_next2BytesLong;

        memset(l_next2BytesStr, 0, sizeof(l_next2BytesStr));

	l_inpStrLen = strlen( i_inRec);
	*o_bufLen = 0;

	printf("\nRead Record : (%.*s) of %d\n", l_inpStrLen-1, i_inRec, l_inpStrLen );

	/* Skip leading Blanks */
	i=0;
	while(i_inRec[i]==' ' && i < l_inpStrLen-1 ) i++;
	if (i== l_inpStrLen)
		return XXXX_GOOD;

        if ( i_inRec[i]=='0' &&  i_inRec[i+1]=='0' && i_inRec[i+3] == '0' && i_inRec[i+4] == '0' ) 
                  l_tripletCounter = 3;
        else
	          l_tripletCounter = 0;

	printf("  Triplets [%s] : ", triplet_name[l_tripletCounter]);

	while( i< l_inpStrLen -1)
	{
		l_next2BytesStr [0]= i_inRec[i];
		l_next2BytesStr [1]= i_inRec[i+1];
                l_next2BytesLong = strtoul(l_next2BytesStr, (char **)NULL, 16)  ; 
                o_outBuf[*o_bufLen] = l_next2BytesLong ; 

               printf("%s(%x)%d,", l_next2BytesStr, l_next2BytesLong, *o_bufLen );
               (*o_bufLen)++;

		i+=2;

		if ( i_inRec[i]==' ')
		{
                        l_tripletCounter = ( l_tripletCounter !=3 ? l_tripletCounter+1 : 3 );
                        if( l_tripletCounter < 3 )
			  printf("  [%s] : ", triplet_name[l_tripletCounter]);

			i++;
			while(i_inRec[i]==' ' && i <= l_inpStrLen ) i++;
		}

	}

	return XXXX_GOOD;
}
