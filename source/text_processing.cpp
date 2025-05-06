#include "text_processing.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "hash_table.h"


// static ----------------------------------------------------------------------

#define ADDITIONAL_SPACE 64

// error messages

const char* FILE_OPENING_ERROR      = "Error while opening text file";
const char* MEMORY_ALLOCATION_ERROR = "Error while allocating memory for text";
const char* FILE_READING_ERROR      = "Error while reading input file";
const char* NULL_TEXT_POINTER_ERROR = "No pointer given to destroy text";
const char* NULL_TEXT_ERROR         = "No pointer on text";


size_t getFileSize(FILE* file);


// public ----------------------------------------------------------------------


TextState textLoad(Text* text, const char* filename)
{
    assert(text     != NULL);
    assert(filename != NULL);

    FILE* text_file = fopen(filename, "rb");
    if (!text_file)
    {
        fprintf(stderr, "%s\n", FILE_OPENING_ERROR);
        return TextState_FILE_OPENING_ERROR;
    }

    size_t size_of_file = getFileSize(text_file);
    text->data = (char*)calloc(size_of_file + ADDITIONAL_SPACE, 
                               sizeof(char));
    if (!text->data)
    {
        fclose(text_file);
        fprintf(stderr, "%s\n", MEMORY_ALLOCATION_ERROR);
        return TextState_ALLOCATION_ERROR;
    }

    size_t bytes_writen = fread(text->data,
                                sizeof(char),
                                size_of_file,
                                text_file);
    if (bytes_writen != size_of_file)
    {
        fclose(text_file);
        free(text->data);
        fprintf(stderr, "%s\n", FILE_READING_ERROR);
        return TextState_FILE_READING_ERROR;
    }

    text->word_count = 0;
    text->text_size  = bytes_writen;

    fclose(text_file);

    return TextState_OK;
}


TextState textDtor(Text* text)
{
    if (!text)
    {
        fprintf(stderr, "%s\n", NULL_TEXT_POINTER_ERROR);
        return TextState_ERROR;
    }

    if (!text->data)
    {
        fprintf(stderr, "%s\n", NULL_TEXT_ERROR);
        return TextState_ERROR;
    }

    free(text->data);
    memset(text, 0, sizeof(Text));

    return TextState_OK;
}




int textPutNextWordToBuffer(Text* text, char* buffer, size_t buffer_size)
{
    assert(text   != NULL);
    assert(buffer != NULL);
    assert(buffer_size > 0);

    char* current_text = text->data + text->current_position;
    
    while (isspace((unsigned char)*current_text))
    {
        current_text++;
    }

    if (*current_text == '\0')
    {
        buffer[0] = '\0';
        return 0;
    }
    
    size_t i = 0;
    while (i < buffer_size - 1
        && *current_text != '\0'
        && !isspace((unsigned char)*current_text))
    {
        buffer[i++] = *current_text;
        current_text++;
    }

    buffer[i] = '\0';
    text->current_position = current_text - text->data;

    return i;
}


int textGetRandomWord(Text* text, char** pointer) 
{
    assert(text != NULL);
    assert(pointer != NULL);

    if (text->data == NULL || text->text_size == 0) {
        *pointer = NULL;
        return 0;
    }

    size_t random_pos = rand() % text->text_size;
    size_t start, pos;

    // Поиск начала слова, в котором находится random_pos
    start = random_pos;
    while (start > 0 && isalnum((unsigned char)text->data[start - 1])) {
        start--;
    }

    // Проверяем, является ли найденная позиция началом слова
    if (isalnum((unsigned char)text->data[start])) {
        pos = start;
        while (pos < text->text_size && isalnum((unsigned char)text->data[pos])) {
            pos++;
        }
        *pointer = &text->data[start];
        return pos - start;
    }

    // Поиск следующего слова после random_pos
    for (pos = random_pos; pos < text->text_size; pos++) {
        if (isalnum((unsigned char)text->data[pos])) {
            start = pos;
            while (pos < text->text_size && isalnum((unsigned char)text->data[pos])) {
                pos++;
            }
            *pointer = &text->data[start];
            return pos - start;
        }
    }

    // Поиск первого слова с начала текста
    for (pos = 0; pos < random_pos; pos++) {
        if (isalnum((unsigned char)text->data[pos])) {
            start = pos;
            while (pos < text->text_size && isalnum((unsigned char)text->data[pos])) {
                pos++;
            }
            *pointer = &text->data[start];
            return pos - start;
        }
    }

    // Слов не найдено
    *pointer = NULL;
    return 0;
}


int textNextWordPointer(Text* text, char** pointer) 
{
    assert(text    != NULL);
    assert(pointer != NULL);

    char* current_text = text->data + text->current_position;

    while (isspace((unsigned char)*current_text)) 
    {
        current_text++;
    }

    if (*current_text == '\0') 
    {
        text->current_position = current_text - text->data;
        return 0;
    }

    while (*current_text != '\0' && !isalpha((unsigned char)*current_text)) 
    {
        current_text++;
    }

    *pointer = current_text;
    size_t length = 0;

    while (isalpha((unsigned char)*current_text) && *current_text != '\0') 
    {
        current_text++;
        length++;
    }

    text->current_position = current_text - text->data;
    text->word_count++;

    return length;
}


TextState textMoveToBegin(Text* text)
{
    assert(text != NULL);

    text->current_position = 0;
    return TextState_OK;
}



// static ----------------------------------------------------------------------


size_t getFileSize(FILE* file)
{
    assert(file != NULL);

    fseek(file, 0L, SEEK_END);
    size_t size_of_file = ftell(file);
    fseek(file, 0L, SEEK_SET);

    return size_of_file;
}
