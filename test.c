#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*get the input source and copies it to a new string without double space*/
void clearDoubleSpace(const char *source, char *copy)
{
	 int i, j, tmp;
	 for(i = j = 0; source[i]; i++, j++)
	 {
	 	source[i] == '\t' ? i++ : 0;
	 	if(source[i] == ' ') {
	 		tmp = i+1;
	 		if(source[tmp] != '\0') {
	 			for(; source[tmp] == ' ' && source[tmp]; tmp++)
	 				i++;
	 		}
	 	}
	 	copy[j] = source[i];
	 }
	
	 copy[j] = '\0';
}
/*copies one word from source to word without any space befor and after*/
void getWord(const char *source, char *word)
{
	int i;
	
	for(; *source == ' ' && *source != '\0'; source++);
	
	for(i = 0; *source != ' ' && *source != '\0'; i++, source++) 
		word[i] = *source;
	
	word[i] = '\0';
}

void printBits(int x, FILE *fd)
{
	unsigned short mask;
	for(mask = 1 << 11; mask; mask >>= 1)
	{
		if(x & mask)
			fprintf(fd, "/");
		else 
			fprintf(fd, ".");
	}
	fprintf(fd, "\n");
}

int isComment(char *buf)
{
	for(; *buf == ' ' && *buf != '\0'; buf++);
	
	return (*buf == ';');
}
int isEmpty(char *buf)
{
	for(; isspace(*buf) && *buf != '\0'; buf++);
	
	return (*buf == '\0');
}

int main (int argc, char *argv[])
{
	char str[] = " ;	    noam   , shu   shan   8  ! !   -5    0 ", copy[80], word[80];
	int i;
	
	clearDoubleSpace(str, copy);
	printf("source =%s\ncopy =%s\n", str, copy);
	for(i = 0; copy[i]; i++)
	{
		getWord(copy+i, word);
		i += strlen(word);
		puts(word);
	}
	
	for(i = 0; copy[i]; i++)
	{
		if(isdigit(copy[i]))
			printBits(atoi(copy+i), stdout);
	}
	
	printf("isComment(copy) ? %s\n", isComment(copy) ? "true" : "false");
	
	fgets(word, sizeof(word), stdin);
	printf("isEmpty(word) ? %s\n", isEmpty(word) ? "empty" : "full");
	
	
	return 0;
}