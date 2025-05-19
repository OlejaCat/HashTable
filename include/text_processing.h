#ifndef TEXT_PROCESSING_H
#define TEXT_PROCESSING_H

#include <stdlib.h>

typedef enum TextState
{
    TextState_OK                 = 0,
    TextState_ERROR              = 1,
    TextState_ALLOCATION_ERROR   = 2,
    TextState_FILE_OPENING_ERROR = 3,
    TextState_FILE_READING_ERROR = 4,
} TextState;

typedef struct Text
{
    char*  data;
    size_t current_position;
    size_t word_count;
    size_t text_size;
} Text;

TextState textLoad(Text* text, const char* filename);
TextState textDtor(Text* text);

int textPutNextWordToBuffer(Text* text, char* buffer, size_t buffer_size);
int textNextWordPointer(Text* text, char** pointer);
int textGetRandomWord(Text* text, char** pointer);
int textGetNextLine(Text* text, char** pointer);

TextState textMoveToBegin(Text* text);

#endif // TEXT_PROCESSING_H
