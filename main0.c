#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define VIRUS_FOUND 1
#define VIRUS_NOT_FOUND 0
#define INFECTED "Infected!"
#define CLEAN "Clean"
#define MAX_LEN 200
#define NORMAL_SCAN 0
#define LOG_SIG "/AntiVirusLog.txt"

int searchString(FILE* virusFile,long fileSize, FILE* searchFile);
int compare(const void* a, const void* b);
void writeLogBasicDetail(FILE* writeFile, char* folder,FILE* virusFile, int scanningO);
void writeLogResultsNormal(FILE* writeFile, char** fileNames, char** fileS, int fileNumber);
void freeMemory(char** file1, char** file2, int len);
int main(int argc, char** argv)
{
    FILE* searchFile = NULL;
    FILE* virusFile = NULL;
    FILE* writeFile = NULL;
    DIR* dir;
    char dirName[MAX_LEN] = { 0 };
    char virusPath[MAX_LEN] = { 0 };
    int virusStatus;
    char** fileNames = NULL;
    char** fileStatus = NULL;
    int fileCount = 0;
    struct dirent* entry;
    int scanningO = 0;
    char logPath[MAX_LEN] = { 0 };
    char searchFilePath[MAX_LEN] = { 0 };
    if (argc < 3)
    {
        printf("Missing arguments.\nUsage: <program> <folder> <virus_signature_file> <scanning_option>\n");
        return 1;
    }
    printf("Folder to scan: ");
    strcpy(dirName, argv[1]);
    printf("%s\n", dirName);
    printf("Virus signature: ");
    strcpy(virusPath, argv[2]);
    printf("%s\n", virusPath);
    printf("press 0 for a normal Scan or any other key for a quick scan:  ");
    scanf("%d", &scanningO);
    printf("%d\n", scanningO);

    printf("Scanning began...\nThis process may take several minutes.\n");

    strcpy(logPath, dirName);
    strcat(logPath, LOG_SIG);

    writeFile = fopen(logPath, "w");
    if (writeFile == NULL) {
        printf("Failed to create log file.\n");
        return 1;
    }
    virusFile = fopen(virusPath, "rb");
    if (!virusFile) {
        printf("Failed to open the virus signature file.\n");
        fclose(writeFile);
        return 1;
    }
    fseek(virusFile, 0, SEEK_END);
    long fileSize = ftell(virusFile);
    rewind(virusFile);

    // Allocate memory to store the file contents
    


    dir = opendir(dirName);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (!(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, "AntiVirusLog.txt") == 0 || entry->d_type == DT_DIR)) {
                strcpy(searchFilePath, dirName);
                strcat(searchFilePath, "/");
                strcat(searchFilePath, entry->d_name);
                searchFile = fopen(searchFilePath, "rb");
                if (!searchFile) {
                    printf("Failed to open the file: %s\n", searchFilePath);
                    fclose(writeFile);
                    fclose(virusFile);
                    closedir(dir);
                    return 1;
                }

                fileCount++;
                fileNames = (char**)realloc(fileNames, sizeof(char*) * fileCount);
                fileNames[fileCount - 1] = (char*)malloc((strlen(entry->d_name) + 1) * sizeof(char));
                fileStatus = (char**)realloc(fileStatus, sizeof(char*) * fileCount);

                if (fileNames == NULL || fileStatus == NULL || fileNames[fileCount - 1] == NULL) {
                    freeMemory(fileNames, fileStatus, fileCount);
                    fclose(virusFile);
                    fclose(writeFile);
                    closedir(dir);
                    return 1;
                }

                strcpy(fileNames[fileCount - 1], entry->d_name);

                if (scanningO == NORMAL_SCAN) {
                    virusStatus = searchString(virusFile, fileSize, searchFile);
                    fclose(searchFile); // Close the file after searching
                    if (virusStatus) {
                        fileStatus[fileCount - 1] = (char*)malloc(sizeof(char) * (strlen(INFECTED) + 1));
                        strcpy(fileStatus[fileCount - 1], INFECTED);
                    }
                    else {
                        fileStatus[fileCount - 1] = (char*)malloc(sizeof(char) * (strlen(CLEAN) + 1));
                        strcpy(fileStatus[fileCount - 1], CLEAN);
                    }
                    if (fileStatus[fileCount - 1] == NULL) {
                        freeMemory(fileNames, fileStatus, fileCount);
                        fclose(virusFile);
                        fclose(writeFile);
                        closedir(dir);
                        return 1;
                    }
                }
                else {
                    // QUICK SCAN
                    fclose(searchFile); // Close the file after scanning
                }
            }
        }

        fclose(virusFile);
        closedir(dir);

        qsort(fileNames, fileCount, sizeof(char*), compare);

        writeLogBasicDetail(writeFile, dirName, virusFile ,scanningO);
        writeLogResultsNormal(writeFile, fileNames, fileStatus, fileCount);
        printf("Scan Completed\nSee log path for results: %s\n", logPath);

        fclose(writeFile);

        freeMemory(fileNames, fileStatus, fileCount);
    }

    getchar();
    return 0;
}

int searchString(FILE* virusFile,long fileSize ,FILE* searchFile)
{
    int virusFound = VIRUS_NOT_FOUND;
    int ch;
    int filech=fgetc(virusFile);
    int location = 0;
    while ((ch = fgetc(searchFile)) != EOF) {
        if (ch == filech) {
            filech = fgetc(virusFile);
            location++;
            if (location == fileSize) {
                virusFound = VIRUS_FOUND;
                break;
            }
        }
        else {
            location = 0;
            fseek(virusFile,0,SEEK_SET);
            filech = fgetc(virusFile);
        }
    }
    rewind(virusFile);

    return virusFound;
}

int compare(const void* a, const void* b)
{
    const char* file1 = *(const char**)a;
    const char* file2 = *(const char**)b;

    return strcmp(file1, file2);
}

void writeLogBasicDetail(FILE* writeFile, char* folder, FILE* virusFile, int scanningO)
{
    int ch;
    fprintf(writeFile, "Scanning Folder: %s\n\n", folder);
    fprintf(writeFile, "virus signature: ");
    while ((ch = fgetc(virusFile)) != EOF)
    {
        fprintf(writeFile, "%c", ch);
    }

    fprintf(writeFile, "\n\nScanning Option: \n");
    if (scanningO == NORMAL_SCAN) {
        fprintf(writeFile, "Normal Scan\n");
    }
    else {
        fprintf(writeFile, "Quick Scan\n");
    }
}



void writeLogResultsNormal(FILE* writeFile, char** fileNames, char** fileS, int fileNumber)
{
    int i;
    printf("\nScanning:\n");
    fprintf(writeFile,"Results:\n");
    for (i = 0; i < fileNumber; i++) {
        fprintf(writeFile, "File Name: %s %s\n", fileNames[i], fileS[i]);
        printf("File Name: %s -- %s\n", fileNames[i], fileS[i]);
    }
}

void freeMemory(char** file1, char** file2, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        free(file1[i]);
        free(file2[i]);
    }
    free(file1);
    free(file2);
}
