#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "linUI.h"
#include "ui_extra.h"

#define PANEL_AMOUNT 5

typedef struct Task {
        char name[256];
        int date;
        bool status;
} Task;

typedef struct windowModel {
        SDL_Window *win;
        SDL_Renderer *rend;
        bool is_running;
        int w, h;

        int mouse_x, mouse_y;
        bool is_mouse_down;
} WM;

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
void cleanUp(UIRes *ui_res, Panel *panels, WM *wm);
void render(SDL_Renderer *rend, Panel *panels);
int setupComponents(SDL_Renderer *rend, Panel *panels, UIRes ui_res);
int eventHandler(WM *wm, Panel *panels);
void setTab(Panel *panels, int tab);
int parseTaskCSV(Task *task_list, int *task_count);
int saveTasks(Task *task_list, Checklist cl, int task_count);

int main(int argc, char **argv) {
        SDL_Init(SDL_INIT_EVERYTHING);
        TTF_Init();

        WM wm = {.is_running = true, .w = 600*16/9, .h = 600};
        wm.win = SDL_CreateWindow("UI test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wm.w, wm.h, 0);
        wm.rend = SDL_CreateRenderer(wm.win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        UIRes ui_res;
        UI_Init(&ui_res);
        Panel panels[PANEL_AMOUNT];
        panels[0] = createPanel(createRect(0, 0, wm.w, 20), ui_res.raisin_black, ui_res.charcoal);
        panels[1] = createPanel(createRect(0, 20, wm.w, wm.h - 20), ui_res.raisin_black, ui_res.raisin_black);
        panels[2] = createPanel(createRect(0, 20, wm.w, wm.h - 20), ui_res.raisin_black, ui_res.raisin_black);
        panels[3] = createPanel(createRect(0, 20, wm.w, wm.h - 20), ui_res.raisin_black, ui_res.raisin_black);
        panels[4] = createPanel(createRect(0, 20, wm.w, wm.h - 20), ui_res.raisin_black, ui_res.raisin_black);
        UI_Event ui_event;
        setupComponents(wm.rend, panels, ui_res);
        setTab(panels, 0);

        int task_count = 0;
        Task task_list[64];
        if(parseTaskCSV(task_list, &task_count) != 0)
                printf("Error reading tasks");

        Checklist cl = panel_getComponent(panels[1], "task_checklist");
        for(int i = 0; i < task_count; i++) {
                checklist_addItem(wm.rend, cl, task_list[i].name);
                checklist_setItemStatus(cl, i, task_list[i].status);
        }
        
        int current_tab = 0;
        int tab_count = 4;
        char *tab_keys[4] = {"tab_tasks", "tab_categories", "tab_stats", "tab_settings"};
        while(wm.is_running) {
                eventHandler(&wm, panels);

                SDL_GetMouseState(&wm.mouse_x, &wm.mouse_y);
                for(int i = 0; i < PANEL_AMOUNT; i++) {
                        panel_update(wm.rend, panels[i], &ui_event, wm.mouse_x, wm.mouse_y, wm.is_mouse_down);
                
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
                                                TextInputField tif = panel_getComponent(panels[1], "new_tasks_input");
                                                textInputField_getText(tif, str);
                                                checklist_addItem(wm.rend, cl, str);
                                                strcpy(task_list[task_count].name, str);
                                                task_list[task_count].status = 0;
                                                task_count++;
                                                panel_hideComponent(panels[1], "new_tasks_input", true);
                                                panel_hideComponent(panels[1], "confirm_task_button", true);
                                        }
                                        break;
                        }
                }
                
                render(wm.rend, panels);
        }
        saveTasks(task_list, cl, task_count);

        cleanUp(&ui_res, panels, &wm);
        TTF_Quit();
        SDL_Quit();
        return 0;
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

int saveTasks(Task *task_list, Checklist cl, int task_count) {
        bool status_list[64];
        checklist_getStatus(cl, status_list);
        for(int i = 0; i < task_count; i++) {
                task_list[i].status = status_list[i];
        }

        FILE *f_ptr = fopen("data/tasks.csv", "w");
        if (!f_ptr) {
            perror("Error opening file for writing");
            return -1;
        }
    
        for (int i = 0; i < task_count; i++) {
            fprintf(f_ptr, "%s,%d\n", task_list[i].name, task_list[i].status);
        }
    
        fclose(f_ptr);
        return 0; 
}

int parseTaskCSV(Task *task_list, int *task_count) {
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
                strncpy(task_list[count].name, name, sizeof(task_list[count].name) - 1);
                task_list[count].name[sizeof(task_list[count].name) - 1] = '\0'; 
                task_list[count].status = atoi(status_str);
    
                count++;
            } else {
                fprintf(stderr, "Invalid line in CSV: %s\n", line);
            }
        }
    
        fclose(f_ptr);

        *task_count = count;
        return 0; 
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

int setupComponents(SDL_Renderer *rend, Panel *panels, UIRes ui_res) {
        Button tabs[4];
        tabs[0] = createButton(rend, "Tasks", createRect(0, 0, 100, 20), ui_res.charcoal, ui_res.grey, ui_res.montserrat_small);
        panel_addComponent(panels[0], COMPONENT_BUTTON, tabs[0], "tab_tasks");
        button_setColorsHovered(rend, tabs[0], ui_res.slate_grey, ui_res.white);

        tabs[1] = createButton(rend, "Categories", createRect(100, 0, 100, 20), ui_res.charcoal, ui_res.grey, ui_res.montserrat_small);
        panel_addComponent(panels[0], COMPONENT_BUTTON, tabs[1], "tab_categories");
        button_setColorsHovered(rend, tabs[1], ui_res.slate_grey, ui_res.white);

        tabs[2] = createButton(rend, "Stats", createRect(200, 0, 100, 20), ui_res.charcoal, ui_res.grey, ui_res.montserrat_small);
        panel_addComponent(panels[0], COMPONENT_BUTTON, tabs[2], "tab_stats");
        button_setColorsHovered(rend, tabs[2], ui_res.slate_grey, ui_res.white);

        tabs[3] = createButton(rend, "Settings", createRect(300, 0, 100, 20), ui_res.charcoal, ui_res.grey, ui_res.montserrat_small);
        panel_addComponent(panels[0], COMPONENT_BUTTON, tabs[3], "tab_settings");
        button_setColorsHovered(rend, tabs[3], ui_res.slate_grey, ui_res.white);

        // TASKS TAB
        Label label = createLabel(rend, 80, 50, "Tasks", ui_res.white, ui_res.montserrat_big);
        panel_addComponent(panels[1], COMPONENT_LABEL, label, "task_label");

        Checklist checklist = createChecklist(100, 100, 40, ui_res.white, ui_res.montserrat_medium);
        panel_addComponent(panels[1], COMPONENT_CHECKLIST, checklist, "task_checklist");

        Button add_task = createButton(rend, "+", createRect(180, 56, 20, 20), ui_res.slate_grey, ui_res.white, ui_res.montserrat_medium);
        panel_addComponent(panels[1], COMPONENT_BUTTON, add_task, "add_tasks_button");
        button_setColorsHovered(rend, add_task, ui_res.grey, ui_res.black);

        TextInputField new_task = createTextInputField(rend, createRect(220, 56, 140, 24), ui_res.charcoal, ui_res.grey, ui_res.montserrat_small);
        panel_addComponent(panels[1], COMPONENT_TEXT_INPUT_FIELD, new_task, "new_tasks_input");
        panel_hideComponent(panels[1], "new_tasks_input", true);

        Button confirm_task = createButton(rend, "Confirm", createRect(361, 55, 70, 26), ui_res.slate_grey, ui_res.white, ui_res.montserrat_small);
        panel_addComponent(panels[1], COMPONENT_BUTTON, confirm_task, "confirm_task_button");
        button_setColorsHovered(rend, confirm_task, ui_res.grey, ui_res.black);
        panel_hideComponent(panels[1], "confirm_task_button", true);

        // CATEGORIES TAB
        Label label2 = createLabel(rend, 80, 50, "Categories", ui_res.white, ui_res.montserrat_big);
        panel_addComponent(panels[2], COMPONENT_LABEL, label2, "categories_label");

        // STATS TAB
        Label label3 = createLabel(rend, 80, 50, "Stats", ui_res.white, ui_res.montserrat_big);
        panel_addComponent(panels[3], COMPONENT_LABEL, label3, "stats_label");

        // SETTINGS TAB
        Label label4 = createLabel(rend, 80, 50, "Settings", ui_res.white, ui_res.montserrat_big);
        panel_addComponent(panels[4], COMPONENT_LABEL, label4, "settings_label");
        
        return true;
}

void render(SDL_Renderer *rend, Panel *panels) {
        SDL_SetRenderDrawColor(rend, 0,0,10,0);
        SDL_RenderClear(rend);
        for(int i = 0; i < PANEL_AMOUNT; i++) {
                panel_render(rend, panels[i]);
        }
        SDL_RenderPresent(rend);
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