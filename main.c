#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ncurses.h>
#include <string.h>

// file definitions
#define MAX_FILE_LENGTH 64

// window size and position assignments
#define BORDER_Y 30
#define BORDER_X 60
#define BORDER_STARTX 1
#define BORDER_STARTY 1

#define NOMOD_Y 20
#define NOMOD_X 29
#define NOMOD_STARTY 3
#define NOMOD_STARTX 2

#define MOD_Y 20
#define MOD_X 29
#define MOD_STARTY 3
#define MOD_STARTX 31

WINDOW* border_window;
WINDOW* mod_window;
WINDOW* nomod_window;

// directory content struct
typedef struct DirContents {
	char*	path;
	char**	files;
	int 	size;
	int	highlight;
	WINDOW*	win;
} DirContents;

// global assignments
DIR *dir;
struct dirent *entry;

DirContents nomod = {
	.path = "/home/em/repos/modman/nomods/",
	.highlight = 1,
};
DirContents mod = {
	.path = "/home/em/repos/modman/mods/",
	.highlight = 0,
};


// initializes three ncurses windows, one for the background/border
// and two for the mod/inactive mod folders respectively
void init_window(DirContents* nomodfolder, DirContents* modfolder) {
	// init options
	initscr();
	noecho();
	curs_set(0);

	// init windows
	border_window = newwin(
			BORDER_Y, BORDER_X, BORDER_STARTY, BORDER_STARTX
	);	
	box(border_window, '|', '-');
	keypad(border_window, TRUE);

	mod_window = newwin(
			MOD_Y, MOD_X, MOD_STARTY, MOD_STARTX
	);

	nomod_window = newwin(
			NOMOD_Y, NOMOD_X, NOMOD_STARTY, NOMOD_STARTX
	);

	// static text placeholder
	mvwprintw(
		border_window,
		1, 1,
		"\t Inactive Mods\t\t       Active Mods"
	);

	// DirContents type window assignment
	nomodfolder->win = nomod_window;
	modfolder->win = mod_window;

	wrefresh(border_window);
	wrefresh(mod_window);
	wrefresh(nomod_window);
}

// allocates memory for the DirContents files array and each individual
// element tallied by count
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

void free_elements(DirContents* folder) {
	for (int i = 0; i < folder->size; ++i) {
		free(folder->files[i]);	
	}
	free(folder->files);
}

// first retrieves number of files in the directory, then copies all the
// file names into an initialized struct
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

// switches from left to right pane, updates directory list to ensure the choice
// isn't greater than the selected directory index
int switch_pane(int choice, DirContents* nomodfolder, DirContents* modfolder) {
	if (nomodfolder->highlight == 1) {
		modfolder->size = get_files(modfolder);
		if (modfolder->size == 0) return 0;
		--nomodfolder->highlight;
		++modfolder->highlight;
		if (choice > modfolder->size - 1) {
			choice = nomodfolder->size - 1;
		}
	} else if (modfolder->highlight == 1) {
		nomodfolder->size = get_files(nomodfolder);
		if (nomodfolder->size == 0) return 0;
		++nomodfolder->highlight;
		--modfolder->highlight;
		if (choice > nomodfolder->size - 1) {
			choice = nomodfolder->size - 1;
		}
	}
	return choice;
}

// moves selected index up or down depending on the string given, making sure to
// stay within the bounds of the array
int change_index(int choice, char* direction, DirContents* nomodfolder, DirContents* modfolder) {
	if (nomodfolder->highlight == 1) {
		if (strcmp(direction, "up")) {
			if (choice < nomodfolder->size - 1) ++choice;	
		} else if (strcmp(direction, "down")) {
			if (choice > 0) --choice;	
		}
	} else if (modfolder ->highlight == 1) {
		if (strcmp(direction, "up")) {
			if (choice < modfolder->size - 1) ++choice;	
		} else if (strcmp(direction, "down")) {
			if (choice > 0) --choice;	
		}
	}
	return choice;
}

// moves the file indicated by the index to the other directory/pane
int move_file(int choice, DirContents* folder1, DirContents* folder2) {
	DirContents* originfolder;
	DirContents* destfolder;

	if (folder1->highlight == 1) {
	       	originfolder = folder1;
	       	destfolder = folder2;
	} else {
		destfolder = folder1;
		originfolder = folder2;
	}

	size_t originpathlength = (strlen(originfolder->path) + strlen(originfolder->files[choice]) + 1);
	char* originfullpath = (char*)malloc(originpathlength);
	sprintf(originfullpath, "%s%s", originfolder->path, originfolder->files[choice]);

	size_t destpathlength = (strlen(destfolder->path) + strlen(originfolder->files[choice]) + 1);
	char* destfullpath = (char*)malloc(destpathlength);
	sprintf(destfullpath, "%s%s", destfolder->path, originfolder->files[choice]);

	rename(originfullpath, destfullpath);

	originfolder->size = get_files(originfolder);
	destfolder->size = get_files(destfolder);

	if (choice > originfolder->size - 1) return --choice;

	return choice;
}

// prints contents of the file array, highlighting the item that's currently
// being selected
void display_panes(int choice, DirContents* folder) {
	werase(folder->win);
	if (folder->size == 0) return;
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
	mvwprintw(border_window, 21, 2, "nomods: %d", folder->size);
	wrefresh(folder->win);
}

void display_metadata(int choice, WINDOW* win, DirContents* nomodfolder, DirContents* modfolder) {
	mvwprintw(win, 21, 2, "nomods: %d", nomodfolder->size);
	mvwprintw(win, 21, 31, "mods: %d", modfolder->size);
	mvwprintw(win, 25, 2, "choice: %d", choice);
}

int main() {
	// choice is what index is currently selected
	int choice = 0;

	init_window(&nomod, &mod);

	// count is the number of files in the directory
	get_files(&nomod);
	get_files(&mod);

	int active = 1;
	while (active) {
		display_panes(choice, &nomod);
		display_panes(choice, &mod);
		display_metadata(choice, border_window, &nomod, &mod);
		get_files(&nomod);
		get_files(&mod);
		int response = wgetch(border_window);
		switch (response) {
			case 'j':
			case KEY_DOWN:
				choice = change_index(choice, "down", &nomod, &mod);
				break;
			case 'l':
			case KEY_RIGHT:
				choice = switch_pane(choice, &nomod, &mod);
				break;
			case 'h':
			case KEY_LEFT:
				choice = switch_pane(choice, &nomod, &mod);
				break;
			case 'k':
			case KEY_UP:
				choice = change_index(choice, "up", &nomod, &mod);
				break;
			case ' ':
				choice = move_file(choice, &nomod, &mod);	
				break;
				
			case 'q':
				active = false;
				break;
		}
	};
	endwin();
	free_elements(&nomod);
	free_elements(&mod);
	exit(EXIT_SUCCESS);
}
