#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ncurses.h>
#include <string.h>

// File definitions
#define MAX_FILE_LENGTH 64

// Window definitions
#define BORDER_Y 30
#define BORDER_X 60
#define BORDER_STARTX 1
#define BORDER_STARTY 1

#define NOMOD_Y 20
#define NOMOD_X 29
#define NOMOD_STARTY 3
#define NOMOD_STARTX 2

#define MOD_Y 20
#define MOD_X  29
#define MOD_STARTY 3
#define MOD_STARTX 31

WINDOW* border_window;
WINDOW* mod_window;
WINDOW* nomod_window;;

// Directory structs

typedef struct DirContents {
	char*	path;
	int 	size;
	int	highlight;
	char** 	files;
	WINDOW*	win;
} DirContents;

DIR *dir;
struct dirent *entry;

void init_window(DirContents* nomod, DirContents* mod) {
	// init options
	initscr();
	noecho();
	curs_set(0);
//	start_color();

	// init windows
	border_window = newwin(
			BORDER_Y, BORDER_X, BORDER_STARTY, BORDER_STARTX
	);	
	box(border_window, '|', '-');
	keypad(border_window, TRUE);

	mod_window = newwin(
			MOD_Y, MOD_X, MOD_STARTY, MOD_STARTX
	);
	box(mod_window, '|', '-');

	nomod_window = newwin(
			NOMOD_Y, NOMOD_X, NOMOD_STARTY, NOMOD_STARTX
	);
	box(nomod_window, '|', '-');

	mvwprintw(
		border_window,
		1, 1,
		"\t Inactive Mods\t\t       Active Mods"
	);

	nomod->win = nomod_window;
	mod->win = mod_window;

	wrefresh(border_window);
	wrefresh(mod_window);
	wrefresh(nomod_window);
}

void init_dircontents(DirContents* folder, int count) {
	folder->size = count;
	folder->files = malloc((count + 1) * sizeof(char*));
	if (folder->files == NULL) {
		endwin();
		printf("Error: Unable to allocate memory for struct\n"); 
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < count; ++i) {
		folder->files[i] = malloc((MAX_FILE_LENGTH + 1) * sizeof(char*));
		if (folder->files[i] == NULL) {
			endwin();
			printf("Error: Unable to allocate memory for struct members\n");
			exit(EXIT_FAILURE);
		}
	}
}

int get_files(DirContents* folder) {
	int count = 0;
	dir = opendir(folder->path);
	if (dir == NULL) {
		endwin();
		printf("Error: Unable to open file for count\n"); 
		exit(EXIT_FAILURE);
	}
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		++count;
    	}
	closedir(dir);

	init_dircontents(folder, count);

	dir = opendir(folder->path);
	if (dir == NULL) {
		endwin(); 
		printf("Error: Unable to open file for copy\n"); 
		exit(EXIT_FAILURE);
	}
	int i = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		strcpy(folder->files[i++], entry->d_name);
	}
	closedir(dir);

	return count;
};

void display(int choice, DirContents* folder) {
	werase(folder->win);
	if (choice > folder->size - 1) return;
	for (int i = 0; i < folder->size; ++i) {
		if (folder->highlight) {
			wattron(folder->win, A_REVERSE);
			mvwprintw(folder->win, choice + 1, 1, folder->files[choice]);
			wattroff(folder->win, A_REVERSE);
		} else {

			mvwprintw(folder->win, choice + 1, 1, folder->files[choice]);
		}
		if (choice == i) continue;
		mvwprintw(folder->win, i + 1, 1, folder->files[i]);
	};	
	wrefresh(folder->win);
}

DirContents nomod = {
	.path = "/home/em/repos/modman/nomods/",
	.highlight = 1,
};
DirContents mod = {
	.path = "/home/em/repos/modman/mods/",
	.highlight = 0,
};

int main() {
	int choice = 0;

	init_window(&nomod, &mod);

	int count = get_files(&nomod);
	get_files(&mod);

	int active = 1;
	while (active) {
		display(choice, &nomod);
		display(choice, &mod);
		mvwprintw(border_window, 20, 1, "Count: %d", count);
		mvwprintw(border_window, 21, 1, "Choice: %d", choice);
		wrefresh(border_window);
		int response = wgetch(border_window);
		switch (response) {
			case 'j':
			case KEY_DOWN:
				if (choice < count - 1) ++choice;
				break;
			case 'l':
			case KEY_RIGHT:
				if (nomod.highlight == 1) {
					--nomod.highlight;
					++mod.highlight;
					count = get_files(&mod);
					if (choice > mod.size - 1) choice = nomod.size - 1;
				}
				break;
			case 'h':
			case KEY_LEFT:
				if (mod.highlight == 1) {
					++nomod.highlight;
					--mod.highlight;
					count = get_files(&nomod);
					if (choice > nomod.size - 1) choice = nomod.size - 1;
				}
				break;
			case 'k':
			case KEY_UP:
				if (choice > 0) --choice;
				break;
				
			case 'q':
				active = false;
				break;
		}
	};
	endwin();
	exit(EXIT_SUCCESS);
}
