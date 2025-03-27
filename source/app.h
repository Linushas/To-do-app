#ifndef UI_EXTRA_H
#define UI_EXTRA_H

#include "SDL_include.h"
#include "linUI.h"

#define PANEL_AMOUNT 4

typedef struct windowModel {
        SDL_Window *win;
        SDL_Renderer *rend;
        bool is_running;
        int w, h;

        int mouse_x, mouse_y;
        bool is_mouse_down;
} WM;

typedef struct Task {
        char name[256];
        int date;
        bool status;
} Task;

typedef struct TaskList {
        Task tasks[64];
        int task_count;
        Checklist cl;
} TaskList;

typedef struct UIResources {
        TTF_Font *russo_big;
        TTF_Font *russo_medium;
        TTF_Font *russo_small;
        TTF_Font *montserrat_big;
        TTF_Font *montserrat_medium;
        TTF_Font *montserrat_small;
        
        SDL_Color black, white, red, green, blue;
        SDL_Color grey, dim_grey, slate_grey, raisin_black, charcoal, spanish_blue;
} UIRes;

int UI_Init(UIRes *res);
int UI_EventHandler(WM *wm, Panel *panels, int *task_count, Task *task_list);

void setTab(Panel *panels, int tab);

int setupComponents(SDL_Renderer *rend, Panel *panels, UIRes ui_res);
int eventHandler(WM *wm, Panel *panels);

int parseTaskCSV(SDL_Renderer *rend, TaskList *tl);
int saveTasks(TaskList tl);

void render(SDL_Renderer *rend, Panel *panels);

void cleanUp(UIRes *ui_res, Panel *panels, WM *wm);

SDL_Rect createRect(int x, int y, int w, int h);
SDL_Color createColor(int r, int g, int b, int a);

#endif