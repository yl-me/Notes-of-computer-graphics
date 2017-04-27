#ifndef image_h

#define image_h

struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char* data;
};

int ImageLoad(char* filename, Image* image);

#endif