#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ncurses.h>
#include <string.h>

// file definitions
#define MAX_FILENAME_LENGTH 64

// window size and position assignments
#define BORDER_Y 30
#define BORDER_X 60
#define BORDER_STARTX 1
#define BORDER_STARTY 1

#define INACTIVE_Y 20
#define INACTIVE_X 29
#define INACTIVE_STARTY 3
#define INACTIVE_STARTX 2

#define ACTIVE_Y 20
#define ACTIVE_X 29
#define ACTIVE_STARTY 3
#define ACTIVE_STARTX 31

WINDOW* border_window;
WINDOW* active_window;
WINDOW* inactive_window;

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

DirContents inactive = {
	.path = "/home/em/repos/modman/inactive_mods/",
	.highlight = 0,
};
DirContents active = {
	.path = "/home/em/repos/modman/mods/",
	.highlight = 0,
};

// function declarations
void init_window(DirContents* inactivefolder, DirContents* activefolder);
void init_dircontents(DirContents* folder, int count);
void free_elements(DirContents* folder);
int get_files(DirContents* folder);
void refresh_files(DirContents* inactivefolder, DirContents* activefolder);
int switch_pane(int choice, DirContents* inactivefolder, DirContents* activefolder);
int change_index(int choice, char* direction, DirContents* inactivefolder, DirContents* activefolder);
void display_panes(int choice, DirContents* folder);
void display_metadata(int choice, WINDOW* win, DirContents* inactivefolder, DirContents* activefolder);

// initializes three ncurses windows, one for the background/border
// and two for the active/inactive active folders respectively
void init_window(DirContents* inactivefolder, DirContents* activefolder) {
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

	active_window = newwin(
			ACTIVE_Y, ACTIVE_X, ACTIVE_STARTY, ACTIVE_STARTX
	);

	inactive_window = newwin(
			INACTIVE_Y, INACTIVE_X, INACTIVE_STARTY, INACTIVE_STARTX
	);

	// static text placeholder
	mvwprintw(
		border_window,
		1, 1,
		"\t Inactive Mods\t\t       Active Mods"
	);

	// DirContents type window assignment
	inactivefolder->win = inactive_window;
	activefolder->win = active_window;

	wrefresh(border_window);
	wrefresh(active_window);
	wrefresh(inactive_window);
}

// allocates memory for the DirContents files array and each individual
// element tallied by count
void init_dircontents(DirContents* folder, int count) {
	folder->size = count;
	folder->files = malloc((count + 1) * sizeof(char*));
	if (folder->files == NULL) {
		endwin();
		fprintf(stderr, "Error: Unable to allocate memory for struct\n"); 
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < count; ++i) {
		folder->files[i] = malloc((MAX_FILENAME_LENGTH + 1) * sizeof(char*));
		if (folder->files[i] == NULL) {
			endwin();
			fprintf(stderr, "Error: Unable to allocate memory for struct members\n");
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
		fprintf(stderr, "Error: Unable to open file for count\n"); 
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
		fprintf(stderr, "Error: Unable to open file for copy\n"); 
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

void refresh_files(DirContents* inactivefolder, DirContents* activefolder) {
	get_files(inactivefolder);
	get_files(activefolder);
}

// switches from left to right pane, updates directory list to ensure the choice
// isn't greater than the selected directory index
int switch_pane(int choice, DirContents* inactivefolder, DirContents* activefolder) {
	if (inactivefolder->highlight == 1) {
		activefolder->size = get_files(activefolder);
		if (activefolder->size == 0) return 0;
		--inactivefolder->highlight;
		++activefolder->highlight;
		if (choice > activefolder->size - 1) {
			choice = activefolder->size - 1;
		}
	} else if (activefolder->highlight == 1) {
		inactivefolder->size = get_files(inactivefolder);
		if (inactivefolder->size == 0) return 0;
		++inactivefolder->highlight;
		--activefolder->highlight;
		if (choice > inactivefolder->size - 1) {
			choice = inactivefolder->size - 1;
		}
	}
	return choice;
}

// moves selected index up or down depending on the string given, making sure to
// stay within the bounds of the array
int change_index(int choice, char* direction, DirContents* inactivefolder, DirContents* activefolder) {
	if (inactivefolder->highlight == 1) {
		if (strcmp(direction, "up")) {
			if (choice < inactivefolder->size - 1) ++choice;	
		} else if (strcmp(direction, "down")) {
			if (choice > 0) --choice;	
		}
	} else if (activefolder ->highlight == 1) {
		if (strcmp(direction, "up")) {
			if (choice < activefolder->size - 1) ++choice;	
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

	if (originfolder->size == 0) return 0;

	size_t originpathlength = (strlen(originfolder->path) + strlen(originfolder->files[choice]) + 1);
	char* originfullpath = (char*)malloc(originpathlength);
	sprintf(originfullpath, "%s%s", originfolder->path, originfolder->files[choice]);

	size_t destpathlength = (strlen(destfolder->path) + strlen(originfolder->files[choice]) + 1);
	char* destfullpath = (char*)malloc(destpathlength);
	sprintf(destfullpath, "%s%s", destfolder->path, originfolder->files[choice]);

	rename(originfullpath, destfullpath);

	originfolder->size = get_files(originfolder);
	destfolder->size = get_files(destfolder);

	if ((choice > originfolder->size - 1) && (choice > 0)) --choice;

	if (originfolder->size == 0) switch_pane(choice, originfolder, destfolder);

	return choice;
}

// prints contents of the file array, highlighting the item that's currently
// being selected
void display_panes(int choice, DirContents* folder) {
	folder->size = get_files(folder);
	werase(folder->win);
	if (folder->size == 0) {
		wrefresh(folder->win);
		return;
	}
	if (choice > folder->size - 1) {
		choice = folder->size - 1;
	}
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

void display_metadata(int choice, WINDOW* win, DirContents* inactivefolder, DirContents* activefolder) {
	mvwprintw(win, 21, 2, "inactive_mods: %d", inactivefolder->size);
	mvwprintw(win, 21, 31, "mods: %d", activefolder->size);
	mvwprintw(win, 25, 2, "choice: %d", choice);
}

int main() {
	// choice is what index is currently selected
	int choice = 0;

	init_window(&inactive, &active);

	refresh_files(&inactive, &active);

	if (inactive.size > 0) inactive.highlight++;
	else active.highlight++;

	int running = 1;
	while (running) {
		refresh_files(&inactive, &active);
		display_metadata(choice, border_window, &inactive, &active);
		display_panes(choice, &inactive);
		display_panes(choice, &active);
		int response = wgetch(border_window);
		switch (response) {

			case 'j':
			case KEY_DOWN:
				choice = change_index(choice, "down", &inactive, &active);
				break;
			case 'k':
			case KEY_UP:
				choice = change_index(choice, "up", &inactive, &active);
				break;
			case 'h':
			case KEY_LEFT:
				choice = switch_pane(choice, &inactive, &active);
				break;
			case 'l':
			case KEY_RIGHT:
				choice = switch_pane(choice, &inactive, &active);
				break;
			case ' ':
				choice = move_file(choice, &inactive, &active);	
				break;
			case 'r':
				refresh_files(&inactive, &active);
				break;
			case 'q':
				running = 0;
				break;
		}
	};
	endwin();
	free_elements(&inactive);
	free_elements(&active);
	exit(EXIT_SUCCESS);
}
