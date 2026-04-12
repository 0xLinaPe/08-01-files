/*********************************
* Class: MAGSHIMIM C2			 *
* Thesis Project         		 *
* Name: Yair Shafran             *
* Credits:                       *
**********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define ARGS_AMOUNT 3
#define FULL_FILE_STR_LEN 150

#define APP_LOC 0
#define FOLDER_LOC 1
#define SIGNATURE_LOC 2

#define NORM_SCAN '0'

#define LOG_FILE_NAME "AntiVirusLog.txt"
#define LOG_BEGINING_CONTENT "Anti-virus began! Welcome!\n\n"

int normalScan(FILE* file, FILE* signature);
int quickScan(FILE* file, FILE* signature);
int filesAmountCheck(char* pathForDIR, struct dirent* directory);
void addBeginingToFile(FILE* logFile, char* folderPath, char* signaturePath, char userOptionChoice);

int main(int argc, char** argv)
{
	// Create Log File To Print Output:
	FILE* logFile = 0;
	char logPath[FULL_FILE_STR_LEN] = { 0 };

	// Create Variables For The Arguments:
	DIR* filesDirectory = 0;
	struct dirent* dir = 0;
	struct dirent* dirCounter = 0;
	int amountOfFiles = 0;
	char** fullFileNames = 0;
	FILE* signature = 0;

	// Create Variables For The Scan Arguments:
	FILE* currentFile = 0;
	int infectedAnswer = 0;

	// Create Other Variables:
	char userOptionChoice = ' ';


	// Check That There Are Enough Arguments:
	if (argc < ARGS_AMOUNT || argc > ARGS_AMOUNT)
	{
		printf("Invalid amount of arguments! (Only %d provided)\nUsage: %s <DIR> <VIRUS SIGNATURE>", argc, argv[APP_LOC]);
		return 1;
	}

	// Print Welcome And Arguments:
	printf("Welcome to my Virus Scan!\n\n");
	printf("Folder to scan: %s\n", argv[FOLDER_LOC]);
	printf("Virus signature: %s\n\n", argv[SIGNATURE_LOC]);

	// Ask User For Option (Quick / Normal Scan):
	printf("Press 0 for a normal scan or any other key for a quick scan: ");
	userOptionChoice = getchar();

	// Tell User Scan Is Starting:
	printf("Scanning began...\n");
	printf("This process may take several minutes...\n\n");

	// Start Scan:
	printf("Scanning:\n");

	strcpy(logPath, argv[FOLDER_LOC]);
	strcat(logPath, "/");
	strcat(logPath, LOG_FILE_NAME);
	logFile = fopen(logPath, "w");

	addBeginingToFile(logFile, argv[FOLDER_LOC], argv[SIGNATURE_LOC], userOptionChoice);

	filesDirectory = opendir(argv[FOLDER_LOC]);
	signature = fopen(argv[SIGNATURE_LOC], "rb");

	amountOfFiles = filesAmountCheck(argv[FOLDER_LOC], dirCounter);
	fullFileNames = malloc(sizeof(char*) * amountOfFiles);
	int filePathIndex = 0;
	// Start While Loop And Go Over All Files In DIR:
	while ((dir = readdir(filesDirectory)) != NULL)
	{
		// Check for non use files:
		if (strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..") && dir->d_type != DT_DIR)
		{
			fullFileNames[filePathIndex] = malloc(FULL_FILE_STR_LEN * sizeof(char));
			// Create File Path:
			strcpy(fullFileNames[filePathIndex], argv[FOLDER_LOC]);
			strcat(fullFileNames[filePathIndex], "/");
			strcat(fullFileNames[filePathIndex], dir->d_name);

			// Open Current File:
			currentFile = fopen(fullFileNames[filePathIndex], "rb");
			if (currentFile == NULL)
			{
				return 1;
			}

			// Normal Scan:
			if (userOptionChoice == NORM_SCAN)
			{
				infectedAnswer = normalScan(currentFile, signature);
				if (infectedAnswer == 0)
				{
					strcat(fullFileNames[filePathIndex], " - Clean");
				}
				else if (infectedAnswer == 4)
				{
					strcat(fullFileNames[filePathIndex], " - Error Occured!");
				}
				else
				{
					strcat(fullFileNames[filePathIndex], " - Infected!");
				}
			}
			// Quick Scan:
			else
			{
				infectedAnswer = quickScan(currentFile, signature);
				if (infectedAnswer == 0)
				{
					infectedAnswer = normalScan(currentFile, signature);
					if (infectedAnswer == 0)
					{
						strcat(fullFileNames[filePathIndex], " - Clean");
					}
					else if (infectedAnswer == 4)
					{
						strcat(fullFileNames[filePathIndex], " - Error Occured!");
					}
					else
					{
						strcat(fullFileNames[filePathIndex], " - Infected!");
					}
				}
				else if (infectedAnswer == 2)
				{
					strcat(fullFileNames[filePathIndex], " - Infected! (first 20%)");
				}
				else if (infectedAnswer == 4)
				{
					strcat(fullFileNames[filePathIndex], " - Error Occured!");
				}
				else
				{
					strcat(fullFileNames[filePathIndex], " - Infected! (last 20%)");
				}
			}
			filePathIndex++;
			fclose(currentFile);
		}
	}

	if (userOptionChoice == '0')
	{
		getchar();
	}

	fclose(signature);


	int i = 0, j = 0;
	char* temp = 0;

	// Start Sort For Files:
	for (i = 0; i < amountOfFiles - 1; i++)
	{
		for (j = 0; j < amountOfFiles - i - 1; j++)
		{
			if (strcmp(fullFileNames[j], fullFileNames[j + 1]) > 0)
			{
				temp = fullFileNames[j];
				fullFileNames[j] = fullFileNames[j + 1];
				fullFileNames[j + 1] = temp;
			}
		}
	}
	fprintf(logFile, "Results:\n");
	// Add to file:
	for (i = 0; i < amountOfFiles; i++)
	{
		fprintf(logFile, fullFileNames[i]);
		fprintf(logFile, "\n");
		printf("%s\n", fullFileNames[i]);
		free(fullFileNames[i]);
	}

	printf("Scan Completed\n");
	printf("See log path for results: %s\n", logPath);

	free(fullFileNames);
	fclose(logFile);

	closedir(filesDirectory);
	getchar();
	return 0;
}

/*
Function will check for a specific signature in a file.
Inputs: file and signature.
Output: Found or not Found.
*/

int normalScan(FILE* file, FILE* signature)
{
	int infectedVal = 0;
	int fileSize = 0;
	int signatureSize = 0;

	char* fileData = 0;
	char* signatureData = 0;

	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	fileData = (char*)malloc(fileSize);
	if (fileData == NULL)
	{
		fclose(file);
		return 4;
	}

	fread(fileData, fileSize, 1, file);


	fseek(signature, 0, SEEK_END);
	signatureSize = ftell(signature);
	fseek(signature, 0, SEEK_SET);

	signatureData = (char*)malloc(signatureSize);
	if (signatureData == NULL)
	{
		fclose(signature);
		free(fileData);
		return 4;
	}

	fread(signatureData, signatureSize, 1, signature);

	int i = 0;
	for (i = 0; i < fileSize - signatureSize + 1; i++)
	{
		if (memcmp(fileData + i, signatureData, signatureSize) == 0)
		{
			infectedVal = 1;
			break;
		}
	}

	free(fileData);
	free(signatureData);

	return infectedVal;

}

/*
Function will scan a files first and last 20% to check for a signature.
Inputs: file to check and signature to check for.
Output: infectedVal - Found signature or not and if yes where.
*/

int quickScan(FILE* file, FILE* signature)
{
	int infectedVal = 0;
	int fileSize = 0;
	int signatureSize = 0;

	char* fileData = 0;
	char* signatureData = 0;

	int firstTwentyPercent = 0;
	int lastTwentyPercent = 0;

	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	fileData = (char*)malloc(fileSize);
	if (fileData == NULL)
	{
		fclose(file);
		return 4;
	}

	fread(fileData, fileSize, 1, file);


	fseek(signature, 0, SEEK_END);
	signatureSize = ftell(signature);
	fseek(signature, 0, SEEK_SET);

	signatureData = (char*)malloc(signatureSize);
	if (signatureData == NULL)
	{
		fclose(signature);
		free(fileData);
		return 4;
	}

	fread(signatureData, signatureSize, 1, signature);

	firstTwentyPercent = fileSize * 0.2;
	lastTwentyPercent = fileSize * 0.8;

	int i = 0;

	for (i = 0; i < firstTwentyPercent; i++)
	{
		if (memcmp(fileData + i, signatureData, signatureSize) == 0)
		{
			infectedVal = 2;
			break;
		}
	}

	if (!infectedVal) {
		for (i = lastTwentyPercent; i < fileSize - signatureSize + 1; i++)
		{
			if (memcmp(fileData + i, signatureData, signatureSize) == 0)
			{
				infectedVal = 3;
				break;
			}
		}
	}
	free(fileData);
	free(signatureData);
	return infectedVal;
}

/*
Function will check for amount of files in dir
Inputs: Directory and dir struct
Output: Amount of files.
*/
int filesAmountCheck(char* pathForDIR, struct dirent* directory)
{
	int amount = 0;
	DIR* dir = opendir(pathForDIR);
	while ((directory = readdir(dir)) != NULL)
	{
		if (strcmp(directory->d_name, ".") && strcmp(directory->d_name, "..") && directory->d_type != DT_DIR)
		{
			amount++;
		}
	}
	closedir(dir);
	return amount;
}

/*
Function will print begining to file
Inputs: File and variables needed to print.
Output: None.
*/
void addBeginingToFile(FILE* logFile, char* folderPath, char* signaturePath, char userOptionChoice)
{
	fprintf(logFile, LOG_BEGINING_CONTENT);
	fprintf(logFile, "Folder to scan:\n");
	fprintf(logFile, folderPath);
	fprintf(logFile, "\n");
	fprintf(logFile, "Virus signature:\n");
	fprintf(logFile, signaturePath);
	fprintf(logFile, "\n\n");
	fprintf(logFile, "Scanning option:\n");
	fprintf(logFile, userOptionChoice == '0' ? "Normal Scan\n\n" : "Quick Scan\n\n");
}