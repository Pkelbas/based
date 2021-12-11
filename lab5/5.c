#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    char id[2];
    uint32_t size;
    int32_t notRequired;
    uint32_t offset;
} headerBMP;


typedef struct {
    uint32_t infoSize;
    uint32_t width;
    uint32_t height;
    uint16_t colorplanes;
    uint16_t bitsperpixel;
    uint32_t compression;
    uint32_t imgSize;
    uint32_t notRequired[4];
} infoBMP;


typedef struct {
    uint32_t height;
    uint32_t width;
    int** cellsColor;
} Image;


Image readImage(FILE* inputFile, uint32_t height, uint32_t width) {
    Image picture;
    picture.height = height;
    picture.width = width;
    picture.cellsColor = (int**) malloc(width * sizeof(int*));

    for (int i = 0; i < width; ++i) 
        picture.cellsColor[i] = (int*) malloc(height * sizeof(int));

    char red, green, blue;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            char red, green, blue;
            fread(&red, 1, 1, inputFile);
            fread(&green, 1, 1, inputFile);
            fread(&blue, 1, 1, inputFile);
            picture.cellsColor[x][y] = (!red) && (!green) && (!blue);
        }
    }
    return picture;
}

void freeImage(Image picture) {
    for (int i = picture.height - 1; i >= 0; --i) 
        free(picture.cellsColor[i]);
    free(picture.cellsColor);
}

void createBMP(char* dirname, int iter, headerBMP header, infoBMP infoHeader, Image picture) {
    unsigned char color;
    char *path = (char *)malloc(sizeof(dirname) + 10);
    sprintf(path, "%s/%d.bmp", dirname, iter); 
    FILE* outputFile = fopen(path, "wb");

    fwrite(header.id, 2, 1, outputFile);
    fwrite(&header.size, 12, 1, outputFile);
    fwrite(&infoHeader, sizeof(infoHeader), 1, outputFile);
    for (int y = 0; y < picture.height; y++) {
        for (int x = 0; x < picture.width; x++) {
            color = picture.cellsColor[x][y] ? (unsigned char) 0 : (unsigned char) 255;
            for (int k = 0; k < 3; ++k)
                fwrite(&color, 1, 1, outputFile);
        }
        //printf("\n");
    }
    fclose(outputFile);
}


Image nextMove(Image board) {
    Image board2;
    board2.height = board.height;
    board2.width = board.width;
    board2.cellsColor = (int**) malloc(board.width * sizeof(int*));

    for (int i = 0; i < board.width; ++i) 
        board2.cellsColor[i] = (int*) malloc(board.height * sizeof(int));
    for (int i = 0; i < board.height; i++) 
        for (int j = 0; j < board.width; j++) 
            board2.cellsColor[j][i] = board.cellsColor[j][i];

    for (int row = 0; row < board.width; row++) {
        for (int col = 0; col < board.height; col++) {
            int countAlive = 0;
            for (int y = row - 1; y < row + 2; ++y) {
                for (int x = col - 1; x < col + 2; ++x) {
                    if ((y == row && x == col) || (y < 0 || x < 0) || (y >= board.width || x >= board.height))
                        continue;
                    if (board.cellsColor[y][x] == 1)
                        countAlive++;
                }
            }
            if (board.cellsColor[row][col]) { // if cell is alive
                if (countAlive < 2 || countAlive > 3) {
                    board2.cellsColor[row][col] = 0;
                }
            } else if (countAlive == 3) { // if cell is dead (inside)
                    board2.cellsColor[row][col] = 1;
            }
        }
    }
    for (int i = 0; i < board.height; i++) 
        for (int j = 0; j < board.width; j++) 
            board.cellsColor[j][i] = board2.cellsColor[j][i];     
    freeImage(board2);
    return board;
}


void openBMP(char* filename, char* dirName, int maxIter, int dumpFreq) {
    FILE* inputFile = fopen(filename, "rb");

    headerBMP mainHeader;
    infoBMP infoHeader;
    fread(mainHeader.id, 2, 1, inputFile);
    fread(&mainHeader.size, 12, 1, inputFile);
    fread(&infoHeader, 24, 1, inputFile);
    fseek(inputFile, mainHeader.offset, SEEK_SET);
    //printf("%d %d %d\n", infoHeader.height, infoHeader.width, infoHeader.imgSize);
    Image img = readImage(inputFile, infoHeader.height, infoHeader.width);
    Image nextMoveImg;
    for (int i = 0, j = 0; i < maxIter + 1; i++, j++) {
        nextMoveImg = nextMove(img);
        if (j == dumpFreq) {
            createBMP(dirName, i, mainHeader, infoHeader, nextMoveImg);
            j = 0;
        }    
    }
    freeImage(img);
    fclose(inputFile);
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Wrong input");
        return 1;
    }
    char* inputFile;
    char* dirName;
    int maxIter = 1, dumpFreq = 1;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--input"))
            inputFile = argv[i + 1];
        else if (!strcmp(argv[i], "--output")) 
            dirName = argv[i + 1];
        else if (!strcmp(argv[i], "--max_iter"))
            maxIter = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "--dump_freq"))
            dumpFreq = atoi(argv[i + 1]);
    }
    openBMP(inputFile, dirName, maxIter, dumpFreq);
    return 0;
}
