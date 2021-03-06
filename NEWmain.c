#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "linkedList.h"
#include "dataTypes.h"
#include "auxMethods.h"

int firstPass(FILE *, lptr *);	/*this function contains all the logic of the first pass over the assembly program.
								 it uses the following parameters:
								 -pointer to source code file
								 -pointer to label linked list
								 */
int secondPass(FILE *, lptr);	/*this function contains all the logic of the second pass over the assembly program.
								it uses the following parameters:
								-pointer to source code file
								-pointer to label linked list
								*/
/*
The main function is used to handle the assembly source files.
it receives the source files names as command prompt parameters via argv (files names parameters should not contain a suffix).
it will iterate over the files and for each file it will run the first and second passes.
if no errors were found it will then create the object file and entries/externals files if needed.
*/

int main(int argc, char *argv[]){
	int i;
	FILE *fp;
	char fileName[MAX_FILE_NAME];
	lptr labelPtr;


	for (i=1 ; i < argc; i++) {
		strcpy(fileName, argv[i]);
		strcat(fileName, ASSEMBLER_SOURCE_FILENAME_SUFFIX);
		fp=fopen(fileName, "r");
		if(!fp){
			printf("File %s not found\n",fileName);
			continue;
		}

		DC = 0;IC = 0;labelPtr = NULL;
		printf("Starting assembly for source file %s , Output file will be writen in the source file folder.\n", fileName);

		if(firstPass(fp, &labelPtr)){
			printf("First pass failed, aborting assembly for current file\n");
			fclose(fp);
			freeLabelList(labelPtr);
			continue;
		}
		fclose(fp);
		fp=fopen(fileName, "r");
		if(secondPass(fp, labelPtr)){
			printf("Second pass failed, aborting assembly for current file\n");
			fclose(fp);
			freeLabelList(labelPtr);
			continue;
		}
		fclose(fp);

		strcpy(fileName, argv[i]);
		strcat(fileName, ASSEMBLER_OBJECT_FILENAME_SUFFIX);
		if(createObjectFile(fileName, MEMORY_OFFSET)){
			printf("%s file creation failed, aborting current assembly\n", fileName);
			remove(fileName);
			freeLabelList(labelPtr);
			continue;
		}
		strcpy(fileName, argv[i]);
		strcat(fileName, ASSEMBLER_ENTRIES_FILENAME_SUFFIX);
		if(createEntriesFile(fileName, labelPtr)){
			printf("%s file creation failed, aborting current assembly\n", fileName);
			remove(fileName);
			freeLabelList(labelPtr);
			continue;
		}
		strcpy(fileName, argv[i]);
		strcat(fileName, ASSEMBLER_EXTERNALS_FILENAME_SUFFIX);
		if(createExternalsFile(fileName, labelPtr, MEMORY_OFFSET)){
			printf("%s file creation failed, aborting current assembly\n", fileName);
			remove(fileName);
			freeLabelList(labelPtr);
			continue;
		}
		printf("Assembly for source file %s.as completed successfully\n", argv[i]);
		freeLabelList(labelPtr);
	}
	return 0;
}

int firstPass(FILE *fp, lptr *labelPtr){
	char buf[MAX_LINE_LENGTH+5];
	char auxbuf[MAX_LINE_LENGTH+5];
	char *token, *token2, *token3;
	char tempLabel[MAX_LABEL_SIZE+1];
	int labelSize, lineErrorFlag;
	int errorFlag = 0, hasLabel = 0;
	int typeFlag = 0;
	lineNumber = 0;

	while(fgets(buf,MAX_LINE_LENGTH,fp)){
		lineErrorFlag = 0;
		lineNumber++;

		if(buf[0] == ';'){
			continue;
		}
		strcpy(auxbuf, buf);
		removeRedundantSpaces(buf);
		token = strtok(buf," ");
		if(!token){
			continue;
		}
		hasLabel = isLabel(token);
		if(hasLabel){
			labelSize = isLegitLabelName(token);
			if(!labelSize){
				errorFlag = 1;
				continue;
			}
			strncpy(tempLabel, token, labelSize);
			tempLabel[labelSize] = '\0';
			token = strtok(NULL," ");
		}

		typeFlag = isGuidance(token);
		if(typeFlag >= 0){
			token = strtok(NULL," ");
			if(!token){
				printf("Error in line %d: Missing parameters\n", lineNumber);
				errorFlag = 1;
				continue;
			}
			if(typeFlag == 4){
				continue;
			}else if (typeFlag == 3){
				if(addLabelToList(labelPtr, token, EXTERNAL, 0)){
					printf("Error in line %d: Failed to add label %s to label list\n", lineNumber, token);
					errorFlag = 1;
					continue;
				}
			}else{
				if(hasLabel){
					if(addLabelToList(labelPtr, tempLabel, DATA, DC)){
						printf("Error in line %d: Failed to add label %s to label list\n", lineNumber, tempLabel);
						errorFlag = 1;
						continue;
					}
				}
				token2 = strtok(NULL," ");
				token3 = strtok(NULL," ");
				switch(typeFlag){
				case 0:
						lineErrorFlag |= storeData(token);
						if(token2){
							errorFlag = 1;
							printf("Error in line %d: Redundant parameters\n", lineNumber);
						}
						break;
				case 1:
						token = strtok(auxbuf, "\"");
						token2 = strtok(NULL, "\"");
						token3 = strtok(NULL, "\"");
						if(token && token2 && token3 && !strtok(token3, " 	\n")){
							lineErrorFlag |= storeString(token2);
						}else{
							errorFlag = 1;
							printf("Error in line %d: Incorrect parameter\n", lineNumber);
						}
						break;
				case 2:
						lineErrorFlag |= storeMat(token, token2);
						if(token3){
							errorFlag = 1;
							printf("Error in line %d: Redundant parameters\n", lineNumber);
						}
						break;
				}

				if(lineErrorFlag){
					printf("Error while processing line %d: Data image storage action failure\n", lineNumber);
					errorFlag = 1;
				}
			}
		}else{
			if(hasLabel){
				if(addLabelToList(labelPtr, tempLabel, CODE, (IC)+MEMORY_OFFSET)){
					printf("Error in line %d: Failed to add label %s to label list\n", lineNumber, tempLabel);
					lineErrorFlag = 1;
				}
			}

			typeFlag = isInstruction(token);
			if(typeFlag < 0){
				printf("Error in line %d: '%s' is not an instruction or guidance\n", lineNumber, token);
				lineErrorFlag = 1;
			}else{
				token = strtok(NULL," ");
				if(strtok(NULL," ")){
					printf("Error in line %d: Redundant parameters\n", lineNumber);
					errorFlag = 1;
					continue;
				}
				if(processInstruction(typeFlag, token)){
					lineErrorFlag = 1;
				}
			}
		}
		errorFlag |= lineErrorFlag;
	}
	updateDataLabels(*labelPtr, (IC)+MEMORY_OFFSET);
	return errorFlag;
}

int secondPass(FILE *fp, lptr labelPtr){
	int lineErrorFlag;
	char *token, *oper1, *oper2;
	char buf[MAX_LINE_LENGTH+5];
	int typeFlag = 0;
	int errorFlag = 0;
	IC = 0;
	lineNumber = 0;

	while(fgets(buf,MAX_LINE_LENGTH,fp)){
		lineErrorFlag = 0;
		lineNumber++;

		if(buf[0] == ';'){
			continue;
		}
		removeRedundantSpaces(buf);
		token = strtok(buf," ");
		if(!token){
			continue;
		}
		if(isLabel(token)){
			token = strtok(NULL," ");
		}
		typeFlag = isGuidance(token);
		if(typeFlag >=0 && typeFlag <=3){
			continue;
		}else if(typeFlag == 4){
			token = strtok(NULL," ");
			if(!markEntryLabels(labelPtr, token)){
				printf("Error in line %d: Entry label %s was not found in table or label is external\n", lineNumber, token);
				lineErrorFlag = 1;
			}
		}else{
			token = strtok(NULL," ");
			oper1 = strtok(token,",");
			if(!oper1){
				IC++;
				continue;
			}
			oper2 = strtok(NULL,",");
			if(oper2){
				if(encodeSourceAndDestOper(labelPtr, oper1, oper2)){
					printf("Error while processing line %d: Instruction source and destination operands encoding failure\n", lineNumber);
					lineErrorFlag = 1;
				}
			}else{
				if(encodeDestOper(labelPtr, oper1)){
					printf("Error while processing line %d: Instruction destination operand encoding failure\n", lineNumber);
					lineErrorFlag = 1;
				}
			}
		}
		errorFlag |= lineErrorFlag;
	}
	return errorFlag;
}
