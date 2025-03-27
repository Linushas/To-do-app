#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app.h"

SDL_Rect createRect(int x, int y, int w, int h) {
        SDL_Rect r = {x,y,w,h};
        return r;
}

SDL_Color createColor(int r, int g, int b, int a) {
        SDL_Color c = {r,g,b,a};
        return c;
}

void setTab(Panel *panels, int tab) {
        for(int i = 1; i < PANEL_AMOUNT; i++) {
                if(i == tab+1) {
                        panel_hide(panels[i], false);
                }
                else {
                        panel_hide(panels[i], true);
                }
        }
}

int UI_Init(UIRes *res) {
        res->russo_big = TTF_OpenFont("res/RussoOne-Regular.ttf", 24);
        res->russo_medium = TTF_OpenFont("res/RussoOne-Regular.ttf", 16);
        res->russo_small = TTF_OpenFont("res/RussoOne-Regular.ttf", 12);

        res->montserrat_big = TTF_OpenFont("res/Montserrat-Regular.ttf", 24);
        res->montserrat_medium = TTF_OpenFont("res/Montserrat-Regular.ttf", 16);
        res->montserrat_small = TTF_OpenFont("res/Montserrat-Regular.ttf", 12);

        res->black = createColor(0, 0, 0, 255);
        res->white = createColor(255, 255, 255, 255);
        res->red = createColor(255, 0, 0, 255);
        res->green = createColor(0, 255, 0, 255);
        res->blue = createColor(0, 0, 255, 255);
        res->spanish_blue = createColor(0, 112, 184, 255);
        res->grey = createColor(160, 160, 160, 255);
        res->charcoal = createColor(51, 51, 51, 255);
        res->dim_grey = createColor(105, 105, 105, 255);
        res->raisin_black = createColor(36, 33, 36, 255);
        res->slate_grey = createColor(112, 128, 144, 255);
        return 1;
}

int UI_EventHandler(WM *wm, Panel *panels, int *task_count, Task *task_list) {
        UI_Event ui_event;
        char *tab_keys[3] = {"tab_tasks", "tab_stats", "tab_settings"};
        static int current_tab = 0;
        static int tab_count = 3;

        SDL_GetMouseState(&wm->mouse_x, &wm->mouse_y);
        for(int i = 0; i < PANEL_AMOUNT; i++) {
                panel_update(wm->rend, panels[i], &ui_event, wm->mouse_x, wm->mouse_y, wm->is_mouse_down);
        
                switch(ui_event.event_type) {
                        case BUTTON_CLICKED:
                                for(int i = 0; i < tab_count; i++) {
                                        if(strcmp(ui_event.component_key, tab_keys[i]) == 0) {
                                                current_tab = i;
                                                setTab(panels, current_tab);
                                                break;
                                        }
                                }
                                if(strcmp(ui_event.component_key, "add_tasks_button") == 0) {
                                        panel_hideComponent(panels[1], "new_tasks_input", false);
                                        panel_hideComponent(panels[1], "confirm_task_button", false);
                                }
                                else if(strcmp(ui_event.component_key, "confirm_task_button") == 0) {
                                        char str[256];
                                        Checklist cl = panel_getComponent(panels[1], "task_checklist");
                                        TextInputField tif = panel_getComponent(panels[1], "new_tasks_input");
                                        textInputField_getText(tif, str);
                                        checklist_addItem(wm->rend, cl, str);
                                        strcpy(task_list[*task_count].name, str);
                                        task_list[*task_count].status = 0;
                                        (*task_count)++;
                                        panel_hideComponent(panels[1], "new_tasks_input", true);
                                        panel_hideComponent(panels[1], "confirm_task_button", true);
                                }
                                break;
                }
        }
}

int eventHandler(WM *wm, Panel *panels) {
        static SDL_Event event;

        TextInputField tif = panel_getComponent(panels[1], "new_tasks_input");

        while(SDL_PollEvent(&event)) {
                switch(event.type) {
                        case SDL_QUIT:
                                wm->is_running = false;
                                break;

                        case SDL_MOUSEBUTTONDOWN:
                                if(event.button.button == SDL_BUTTON_LEFT)
                                        wm->is_mouse_down = true;
                                break;
                        case SDL_MOUSEBUTTONUP:
                                if(event.button.button == SDL_BUTTON_LEFT)
                                        wm->is_mouse_down = false;
                                break;
                        
                        case SDL_KEYDOWN:
                                if(textInputField_getFocus(tif)) {
                                        if(event.key.keysym.scancode == SDL_SCANCODE_LEFT)
                                                textInputField_moveCursor(tif, 0);
                                        else if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                                                textInputField_moveCursor(tif, 1);
                                        if(event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE)
                                                textInputField_updateBuffer(tif, INPUT_BACKSPACE, "");
                                }
                                break;
                        
                        case SDL_TEXTINPUT:
                                if(textInputField_getFocus(tif)) {
                                        textInputField_updateBuffer(tif, INPUT_TEXT, event.text.text);
                                }
                                break;
                }
        }
}

int saveTasks(TaskList tl) {
        bool status_list[64];
        checklist_getStatus(tl.cl, status_list);
        for(int i = 0; i < tl.task_count; i++) {
                tl.tasks[i].status = status_list[i];
        }

        FILE *f_ptr = fopen("data/tasks.csv", "w");
        if (!f_ptr) {
            perror("Error opening file for writing");
            return -1;
        }
    
        for (int i = 0; i < tl.task_count; i++) {
            fprintf(f_ptr, "%s,%d\n", tl.tasks[i].name, tl.tasks[i].status);
        }
    
        fclose(f_ptr);
        return 0; 
}

int parseTaskCSV(SDL_Renderer *rend, TaskList *tl) {
        FILE *f_ptr = fopen("data/tasks.csv", "r");
        if (!f_ptr) {
            perror("Error opening file");
            return -1;
        }
    
        char line[1024];
        int count = 0;
    
        while (fgets(line, sizeof(line), f_ptr)) {
            line[strcspn(line, "\n")] = '\0';
    
            char *name = strtok(line, ",");
            char *status_str = strtok(NULL, ",");
    
            if (name && status_str) {
                strncpy(tl->tasks[count].name, name, sizeof(tl->tasks[count].name) - 1);
                tl->tasks[count].name[sizeof(tl->tasks[count].name) - 1] = '\0'; 
                tl->tasks[count].status = atoi(status_str);
    
                count++;
            } else {
                fprintf(stderr, "Invalid line in CSV: %s\n", line);
            }
        }
    
        fclose(f_ptr);

        tl->task_count = count;

        for(int i = 0; i < tl->task_count; i++) {
                checklist_addItem(rend, tl->cl, tl->tasks[i].name);
                checklist_setItemStatus(tl->cl, i, tl->tasks[i].status);
        }
        return 0; 
}

int setupComponents(SDL_Renderer *rend, Panel *panels, UIRes ui_res) {
        SDL_Rect nav_bar = panel_getRect(panels[0]);

        Button tabs[3];
        tabs[0] = createButton(rend, "Tasks", createRect(0, 0, nav_bar.w/3, nav_bar.h), ui_res.charcoal, ui_res.grey, ui_res.montserrat_medium);
        panel_addComponent(panels[0], COMPONENT_BUTTON, tabs[0], "tab_tasks");
        button_setColorsHovered(rend, tabs[0], ui_res.slate_grey, ui_res.white);

        tabs[1] = createButton(rend, "Stats", createRect(nav_bar.w/3, 0, nav_bar.w/3, nav_bar.h), ui_res.charcoal, ui_res.grey, ui_res.montserrat_medium);
        panel_addComponent(panels[0], COMPONENT_BUTTON, tabs[1], "tab_stats");
        button_setColorsHovered(rend, tabs[1], ui_res.slate_grey, ui_res.white);

        tabs[2] = createButton(rend, "Settings", createRect(nav_bar.w*2/3, 0, nav_bar.w/3, nav_bar.h), ui_res.charcoal, ui_res.grey, ui_res.montserrat_medium);
        panel_addComponent(panels[0], COMPONENT_BUTTON, tabs[2], "tab_settings");
        button_setColorsHovered(rend, tabs[2], ui_res.slate_grey, ui_res.white);

        // TASKS TAB
        Label label = createLabel(rend, nav_bar.w/2 - 200, 50, "Tasks", ui_res.white, ui_res.montserrat_big);
        panel_addComponent(panels[1], COMPONENT_LABEL, label, "task_label");

        Checklist checklist = createChecklist(nav_bar.w/2 - 180, 100, 40, ui_res.white, ui_res.montserrat_medium);
        panel_addComponent(panels[1], COMPONENT_CHECKLIST, checklist, "task_checklist");

        Button add_task = createButton(rend, "+", createRect(nav_bar.w/2 - 100, 56, 20, 20), ui_res.slate_grey, ui_res.white, ui_res.montserrat_medium);
        panel_addComponent(panels[1], COMPONENT_BUTTON, add_task, "add_tasks_button");
        button_setColorsHovered(rend, add_task, ui_res.grey, ui_res.black);

        TextInputField new_task = createTextInputField(rend, createRect(nav_bar.w/2 - 60, 56, 140, 24), ui_res.charcoal, ui_res.grey, ui_res.montserrat_small);
        panel_addComponent(panels[1], COMPONENT_TEXT_INPUT_FIELD, new_task, "new_tasks_input");
        panel_hideComponent(panels[1], "new_tasks_input", true);

        Button confirm_task = createButton(rend, "Confirm", createRect(nav_bar.w/2 + 81, 55, 70, 26), ui_res.slate_grey, ui_res.white, ui_res.montserrat_small);
        panel_addComponent(panels[1], COMPONENT_BUTTON, confirm_task, "confirm_task_button");
        button_setColorsHovered(rend, confirm_task, ui_res.grey, ui_res.black);
        panel_hideComponent(panels[1], "confirm_task_button", true);

        // STATS TAB
        Label label3 = createLabel(rend, 80, 50, "Stats", ui_res.white, ui_res.montserrat_big);
        panel_addComponent(panels[2], COMPONENT_LABEL, label3, "stats_label");

        // SETTINGS TAB
        Label label4 = createLabel(rend, 80, 50, "Settings", ui_res.white, ui_res.montserrat_big);
        panel_addComponent(panels[3], COMPONENT_LABEL, label4, "settings_label");
        
        return true;
}

void cleanUp(UIRes *ui_res, Panel *panels, WM *wm) {
        TTF_CloseFont(ui_res->russo_big);
        TTF_CloseFont(ui_res->russo_small);
        TTF_CloseFont(ui_res->russo_medium);

        TTF_CloseFont(ui_res->montserrat_big);
        TTF_CloseFont(ui_res->montserrat_small);
        TTF_CloseFont(ui_res->montserrat_medium);

        for(int i = 0; i < PANEL_AMOUNT; i++) {
                destroyPanel(panels[i]);
        }

        SDL_DestroyRenderer(wm->rend);
        SDL_DestroyWindow(wm->win);
}

void render(SDL_Renderer *rend, Panel *panels) {
        SDL_SetRenderDrawColor(rend, 0,0,10,0);
        SDL_RenderClear(rend);
        for(int i = 0; i < PANEL_AMOUNT; i++) {
                panel_render(rend, panels[i]);
        }
        SDL_RenderPresent(rend);
}