//
//  main.cpp
//  Image
//
//  Created by Ajay Raj Nelapudi on 22/12/18.
//  Copyright © 2018 Ajay Raj Nelapudi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#define RATE 44100

#pragma pack(push)
#pragma pack(1)

typedef struct {
    unsigned short type;                 /* Magic identifier            */
    unsigned int size;                   /* File size in bytes          */
    unsigned int reserved;
    unsigned int offset;                 /* Offset to image data, bytes */
} HEADER;

typedef struct {
    unsigned int size;               /* Header size in bytes      */
    int width, height;               /* Width and height of image */
    unsigned short planes;           /* Number of colour planes   */
    unsigned short bits;             /* Bits per pixel            */
    unsigned int compression;        /* Compression type          */
    unsigned int imagesize;          /* Image size in bytes       */
    int xresolution,yresolution;     /* Pixels per meter          */
    unsigned int ncolours;           /* Number of colours         */
    unsigned int importantcolours;   /* Important colours         */
} INFOHEADER;

typedef struct {
    unsigned char r, g, b;
} COLOURINDEX;

#pragma pack(pop)

COLOURINDEX *readBMP(FILE *image, HEADER *header, INFOHEADER *infoHeader, int *width, int *height) {
    if (image == NULL || header == NULL || infoHeader == NULL || width == NULL || height == NULL) {
        return NULL;
    }
    
    /*HEADER head; INFOHEADER info;
    fread(&head, sizeof(HEADER), 1, image);
    fread(&info, sizeof(INFOHEADER), 1, image);
    
    printf("%u\n%d\n%d\n%u\n%u\n%u\n%u\n%d\n%d\n%u\n%u\n", info.size, info.width, info.height, info.planes, info.bits, info.compression, info.imagesize, info.xresolution, info.yresolution, info.ncolours, info.importantcolours);*/

    fseek(image, 18, SEEK_SET);
    fread(width, sizeof(int), 1, image);
    
    fseek(image, 22, SEEK_SET);
    fread(height, sizeof(int), 1, image);
    
    COLOURINDEX *imagePixels = (COLOURINDEX *)malloc(sizeof(COLOURINDEX) * (*width) * (*height));
    
    fseek(image, 54, SEEK_SET);
    fread(imagePixels, sizeof(COLOURINDEX) * (*width) * (*height), 1, image);
    
    return imagePixels;
}

COLOURINDEX *generateMask(COLOURINDEX *image1Pixels, COLOURINDEX *image2Pixels, int width, int height) {
    if (image1Pixels == NULL || image2Pixels == NULL) {
        return NULL;
    }
    
    COLOURINDEX *mask = (COLOURINDEX *)malloc(sizeof(COLOURINDEX) * width * height);
    
    for(int i = 0; i < width * height; i++) {
        if (image1Pixels[i].r == image2Pixels[i].r &&
            image1Pixels[i].g == image2Pixels[i].g &&
            image1Pixels[i].b == image2Pixels[i].b) {
            mask[i].r = 0;
            mask[i].g = 0;
            mask[i].b = 0;
        } else {
            mask[i].r = image1Pixels[i].r;
            mask[i].g = image1Pixels[i].g;
            mask[i].b = image1Pixels[i].b;
        }
    }
    
    return mask;
}

COLOURINDEX *pasteImage(COLOURINDEX *image, COLOURINDEX *mask, int width, int height) {
    if (image == NULL || mask == NULL) {
        return NULL;
    }
    
    COLOURINDEX *newImage = (COLOURINDEX *)malloc(sizeof(COLOURINDEX) * width * height);
    
    for(int i = 0; i < width * height; i++) {
        if (mask[i].r + mask[i].g + mask[i].b == 0) {
            newImage[i].r = image[i].r;
            newImage[i].g = image[i].g;
            newImage[i].b = image[i].b;
        } else {
            newImage[i].r = mask[i].r;
            newImage[i].g = mask[i].g;
            newImage[i].b = mask[i].b;
        }
    }
    
    return newImage;
}

HEADER *makeHeader() {
    HEADER *header = (HEADER *)malloc(sizeof(HEADER));
    header->type = 19778;
    header->size = 30054;
    header->reserved = 0;
    header->offset = 54;
    
    return header;
}

INFOHEADER *makeInfoHeader(int width, int height) {
    INFOHEADER *infoHeader = (INFOHEADER *)malloc(sizeof(INFOHEADER));
    infoHeader->size = 40;
    infoHeader->width = width;
    infoHeader->height = height;
    infoHeader->planes = 1;
    infoHeader->bits = 24;
    infoHeader->compression = 0;
    infoHeader->imagesize = 0;
    infoHeader->xresolution = 0;
    infoHeader->yresolution = 0;
    infoHeader->ncolours = 0;
    infoHeader->importantcolours = 0;
    
    return infoHeader;
}

void maskAndPaste(FILE *image1, FILE *image2, FILE *image3, FILE *target) {
    if (image1 == NULL || image2 == NULL || image3 == NULL || target == NULL) {
        return;
    }
    
    HEADER header1, header2, header3;
    INFOHEADER infoHeader1, infoHeader2, infoHeader3;
    COLOURINDEX *image1Pixels, *image2Pixels, *image3Pixels;
    int width1, width2, width3;
    int height1, height2, height3;
    
    image1Pixels = readBMP(image1, &header1, &infoHeader1, &width1, &height1);
    image2Pixels = readBMP(image2, &header2, &infoHeader2, &width2, &height2);
    image3Pixels = readBMP(image3, &header3, &infoHeader3, &width3, &height3);
    
    COLOURINDEX *mask = generateMask(image1Pixels, image2Pixels, width1, height1);
    COLOURINDEX *newImage = pasteImage(image3Pixels, mask, width1, height1);
    
    /*for(int i=0; i<width1 * height1; i++) {
        printf("%d %d %d\n", newImage[i].r, newImage[i].g, newImage[i].b);
    }*/
    
    HEADER *newHeader = makeHeader();
    INFOHEADER *newInfoHeader = makeInfoHeader(width1, height1);
    
    fwrite(newHeader, sizeof(HEADER), 1, target); //Solution: create new header
    fwrite(newInfoHeader, sizeof(INFOHEADER), 1, target); //Solution: create new infoheade
    fwrite(newImage, sizeof(COLOURINDEX)  * width1 * height1, 1, target);
    
    free(image1Pixels);
    free(image2Pixels);
    free(image3Pixels);
    free(mask);
    free(newImage);
    free(newHeader);
    free(newInfoHeader);
}

int main(int argc, char *argv[]) {
    FILE *image1, *image2, *image3;

    if (argc == 4) {
        image1 = fopen(argv[1], "rb");
        image2 = fopen(argv[2], "rb");
        image3 = fopen(argv[3], "rb");
    } else { 
        image1 = fopen("image1.bmp", "rb");
        image2 = fopen("image2.bmp", "rb");
        image3 = fopen("image3.bmp", "rb");
    }
    FILE *target = fopen("TragetImage.bmp", "wb");
    
    maskAndPaste(image1, image2, image3, target);
    
    printf("Image Generated in TragetImage.bmp\n");
    fclose(image1);
    fclose(image2);
    fclose(image3);
    fclose(target);
    
    return 0;
}
