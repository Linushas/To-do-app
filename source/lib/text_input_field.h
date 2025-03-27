#ifndef TEXT_INPUT_FIELD_H
#define TEXT_INPUT_FIELD_H

#include <stdbool.h>
#include "../SDL_include.h"

enum InputStates{INPUT_NONE, INPUT_TEXT, INPUT_BACKSPACE, INPUT_ENTER};

typedef struct TextInputField *TextInputField;

TextInputField createTextInputField(SDL_Renderer *rend, SDL_Rect rect, SDL_Color bg, SDL_Color fg, TTF_Font *font);
int destroyTextInputField(TextInputField tif);

void textInputField_update(SDL_Renderer *rend, TextInputField tif); // updates text texture
void textInputField_render(SDL_Renderer *rend, TextInputField tif); 
void textInputField_moveCursor(TextInputField tif, int direction); // 0 : left, 1 : right
bool textInputField_getFocus(TextInputField tif);
bool textInputField_updateFocus(TextInputField tif, int mouse_x, int mouse_y, bool is_mouse_down); // controls SDL_StartTextInput() and SDL_StopTextInput()
void textInputField_updateBuffer(TextInputField tif, int input_state, char *input);
void textInputField_getText(TextInputField tif, char *output_buffer);

#endif