#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <string.h>


typedef struct {
    char marker[3];
    char version[2];
    char flags;
    char length[4];
} headerID3V2;


typedef struct {
    char id[4];
    char length[4];
    char flags[2];
    char unicode;
    char* frameText;
} frameID3V2;


unsigned int calcSynchSafe(char length[4]) {
    unsigned int result = 0;
    for (int i = 0; i < 4; ++i)
        result += length[3 - i] << (i * 7);
    return result;
}


void calcInt(int x, char* length) {
    for (int i = 0; i < 4; ++i) 
        length[i] = (x >> (3 - i) * 7) & 127;
}


void setValue(FILE* inputFile, char* nameOfFile, headerID3V2* header, unsigned int allFramesLength, char* propName, char* propValue) {
    FILE* tempFile = fopen("temp.mp3", "wb");
    if (tempFile == NULL)
        printf("File error");
    fwrite(&header->marker, 3, 1, tempFile);
    fwrite(&header->version, 2, 1, tempFile);
    fwrite(&header->flags, 1, 1, tempFile);
    fwrite(&header->length, 4, 1, tempFile);

    int flagIfFound = 0;

    while (allFramesLength > 0) {
        frameID3V2* curFrame = (frameID3V2*)malloc(sizeof(frameID3V2));
        fread(&curFrame->id, 4, 1, inputFile);

        if ((int) curFrame->id[0] == 0) 
            break;
        
        fwrite(&curFrame->id, 4, 1, tempFile);
        fread(&curFrame->length, 4, 1, inputFile);
        fread(&curFrame->flags, 2, 1, inputFile);
        fread(&curFrame->unicode, 1, 1, inputFile);
        unsigned int curSize = calcSynchSafe(curFrame->length) - 1;

        curFrame->frameText = (char*) calloc(curSize, 1);
        fread(curFrame->frameText, curSize, 1, inputFile);

        if (propName[0] == curFrame->id[0] && propName[1] == curFrame->id[1] && 
            propName[2] == curFrame->id[2] && propName[3] == curFrame->id[3]) {

            flagIfFound = 1;
            long curPoint = ftell(inputFile);
            fseek(inputFile, 0, SEEK_END); 
            long neededSize = ftell(inputFile) - curPoint;
            char newSize[4];
            calcInt(strlen(propValue) + 1, newSize);
            
            fwrite(&newSize, 4, 1, tempFile);
            fwrite(&curFrame->flags, 2, 1, tempFile);
            fwrite(&curFrame->unicode, 1, 1, tempFile);

            fwrite(propValue, strlen(propValue), 1, tempFile);

            unsigned int* buf = (unsigned int*)calloc(neededSize, 1);
            fseek(inputFile, curPoint, SEEK_SET); 
            fread(buf, neededSize, 1, inputFile);    
            fwrite(buf, neededSize, 1, tempFile);

            fseek(tempFile, 6, SEEK_SET); 
            char newSizeTag[4];
            int newSizeTagInt = calcSynchSafe(header->length) - (curSize - strlen(propValue));
            calcInt(newSizeTagInt, newSizeTag);
            fwrite(&newSizeTag, 4, 1, tempFile);

            free(curFrame);
            fclose(inputFile);
            fclose(tempFile);
            remove(nameOfFile);                   
            rename("temp.mp3", nameOfFile);
            break;
            
        } else {
            fwrite(&curFrame->length, 4, 1, tempFile);
            fwrite(&curFrame->flags, 2, 1, tempFile);
            fwrite(&curFrame->unicode, 1, 1, tempFile);
            fwrite(curFrame->frameText, curSize, 1, tempFile);
        }
        free(curFrame);
        allFramesLength -= (11 + curSize);
    }
    if (flagIfFound == 0) {
        printf("Error! We can't find needed frame\n");
    } 
}


void show(FILE* inputFile, unsigned int allFramesLength, int flagGet, char* frameName) {
    while (allFramesLength > 0) {
        frameID3V2* curFrame = (frameID3V2*)malloc(sizeof(frameID3V2));
        fread(&curFrame->id, 4, 1, inputFile);

        if ((int) curFrame->id[0] == 0) 
            break;
        
        fread(&curFrame->length, 4, 1, inputFile);
        fread(&curFrame->flags, 2, 1, inputFile);
        fread(&curFrame->unicode, 1, 1, inputFile);
        unsigned int curSize  = calcSynchSafe(curFrame->length) - 1;

        curFrame->frameText = (char*) calloc(curSize, 1);
        fread(& *curFrame->frameText, curSize, 1, inputFile);

        if (flagGet) {
            if (frameName[0] == curFrame->id[0] && frameName[1] == curFrame->id[1] && 
            frameName[2] == curFrame->id[2] && frameName[3] == curFrame->id[3]) {
                printf("Information of %s frame is: ", frameName);
                for (int i = 0; i < curSize; ++i) 
                    printf("%c", curFrame->frameText[i]);
                printf("\n");
                break;
            }
        } else {
            printf("%c%c%c%c   ", curFrame->id[0], curFrame->id[1], curFrame->id[2], curFrame->id[3]);
            for (int i = 0; i < curSize; ++i) 
                printf("%c", curFrame->frameText[i]);
            printf("\n");
        }
        free(curFrame);
        allFramesLength -= (11 + curSize);
    }
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Sorry, I can't do anything. Try again");
        exit(1);
    }

    char* tmp;
    strtok_r(argv[1], "=", &tmp);
    char* nameOfFile = strtok_r(NULL, "", &tmp);

    FILE* inputFile = fopen(nameOfFile, "rb");
    
    if (inputFile == NULL) {
        printf("File error", stderr);
        exit(1);
    }

    headerID3V2* header = (headerID3V2*)malloc(sizeof(headerID3V2));
    fread(&header->marker, 3, 1, inputFile);
    fread(&header->version, 2, 1, inputFile);
    fread(&header->flags, 1, 1, inputFile);
    fread(&header->length, 4, 1, inputFile);
    unsigned int allFramesLength = calcSynchSafe(header->length);

    if (argc == 3) {
        if ((strcmp(argv[2], "--show") == 0)) {
            show(inputFile, allFramesLength, 0, "");
        } else {
            strtok_r(argv[2], "=", &tmp);
            char* frameName = strtok_r(NULL, "", &tmp);
            show(inputFile, allFramesLength, 1, frameName);
        }
    } else {
        strtok_r(argv[2], "=", &tmp);
        char* propName = strtok_r(NULL, "", &tmp);
        strtok_r(argv[3], "=", &tmp);
        char* propValue = strtok_r(NULL, "", &tmp);
        setValue(inputFile, nameOfFile, header, allFramesLength, propName, propValue);
    }
    
    fclose(inputFile);
    free(header);
    return 0;
}
