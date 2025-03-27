#ifndef CHECKLIST_H
#define CHECKLIST_H

#include <stdbool.h>
#include "SDL_include.h"

typedef struct Checklist *Checklist;

Checklist createChecklist(int x, int y, int size, SDL_Color fg, TTF_Font *font);
int checklist_addItem(SDL_Renderer *rend, Checklist cb, char *text);
int destroyChecklist(Checklist cb);
int checklist_event(Checklist cb, int mouse_x, int mouse_y, bool is_mouse_down);
void checklist_render(SDL_Renderer *rend, Checklist cb);
void checklist_getStatus(Checklist cb, bool *status_list);
void checklist_setItemStatus(Checklist cb, int item_idx, bool status);

#endif