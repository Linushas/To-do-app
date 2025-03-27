#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "lib/linUI.h"
#include "app.h"

int startUp(WM *wm, Panel *panels, UIRes *ui_res, TaskList *tl);
int close(WM *wm, Panel *panels, UIRes *ui_res, TaskList *tl);

int main(int argc, char **argv) {
        WM wm = {.is_running = true, .w = 600*16/9, .h = 600};
        UIRes ui_res;
        Panel panels[PANEL_AMOUNT];
        TaskList tl = {.task_count = 0};
        startUp(&wm, panels, &ui_res, &tl);
        
        while(wm.is_running) {
                eventHandler(&wm, panels);
                UI_EventHandler(&wm, panels, &tl.task_count, tl.tasks);
                render(wm.rend, panels);
        }
        
        close(&wm, panels, &ui_res, &tl);
        return 0;
}

int close(WM *wm, Panel *panels, UIRes *ui_res, TaskList *tl) {
        saveTasks(*tl);

        cleanUp(ui_res, panels, wm);
        TTF_Quit();
        SDL_Quit();
}

int startUp(WM *wm, Panel *panels, UIRes *ui_res, TaskList *tl) {
        SDL_Init(SDL_INIT_EVERYTHING);
        TTF_Init();

        wm->win = SDL_CreateWindow("To Do", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wm->w, wm->h, 0);
        wm->rend = SDL_CreateRenderer(wm->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        UI_Init(ui_res);

        int nav_bar_height = 30;
        panels[0] = createPanel(createRect(0, 0, wm->w, nav_bar_height), ui_res->raisin_black, ui_res->charcoal);

        panels[1] = createPanel(createRect(0, nav_bar_height, wm->w, wm->h - nav_bar_height), ui_res->raisin_black, ui_res->raisin_black);
        panels[2] = createPanel(createRect(0, nav_bar_height, wm->w, wm->h - nav_bar_height), ui_res->raisin_black, ui_res->raisin_black);
        panels[3] = createPanel(createRect(0, nav_bar_height, wm->w, wm->h - nav_bar_height), ui_res->raisin_black, ui_res->raisin_black);
        setupComponents(wm->rend, panels, *ui_res);
        setTab(panels, 0);

        tl->cl = panel_getComponent(panels[1], "task_checklist");
        if(parseTaskCSV(wm->rend, tl) != 0)
                printf("Error reading tasks");
}