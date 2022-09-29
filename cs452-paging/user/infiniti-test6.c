/* This is the solution to leetcode problem No.6, the zigzag conversion.
 * author: Jidong Xiao
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"

char * convert(char * s, int numRows){
    int i=0;
    int j=0;
    int jm=0;
    int len;
    int index=0;
    len=strlen(s);
    if(len==1 || numRows==1 || numRows>=len) return s;
    char *s1[960];

	/* we could just allocate 1000 bytes for each row, but below is just to satisfy leetcode's insane memory limit requirement. */
    /* we use s1[0] to store final result, thus it needs more memory. */
    s1[0]=(char *)infiniti_malloc(sizeof(char)*(len+numRows));
    if(numRows!=2){
        for(i=1;i<numRows;i++)
            s1[i]=(char *)infiniti_malloc(sizeof(char)*(len/(numRows-2)+50));
    }else{
        for(i=1;i<2;i++)
            s1[i]=(char *)infiniti_malloc(sizeof(char)*(len/2+50));   
    }

    while(index<len){
        s1[0][j]=s[index];
        index++;
        for(i=1;i<numRows-1 && index<len;i++){
            s1[i][jm]=s[index];
            index++;
            s1[i][jm+1]='\0';
        }
        s1[numRows-1][j]=s[index];
        index++;
        jm++;
        j++;
        s1[0][j]='\0';
        s1[numRows-1][j]='\0';
        for(i=numRows-2;i>0 && index<len;i--){
            s1[i][jm]=s[index];
            index++;
            s1[i][jm+1]='\0';
        }
        jm++;
    }
    for(i=1;i<numRows;i++){
        strcat(s1[0],s1[i]);
		infiniti_free(s1[i]);
	}
    return s1[0];
}

void main(){
	char *s="PAYPALISHIRING";
	//char *s="Apalindromeisaword,phrase,number,orothersequenceofunitsthatcanbereadthesamewayineitherdirection,withgeneralallowancesforadjustmentstopunctuationandworddividers.";
	char *s1;
	init_infiniti();
	//s1=convert(s, 5);
	s1=convert(s, 3);
	printf("s1 is %s\n",s1);
}

/* vim: set ts=4: */
