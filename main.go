package main

import (
	"strconv"
	"os"
	"github.com/rthornton128/goncurses"
)

type DirContents struct {
	path	string
	files	[]os.DirEntry
	win		*goncurses.Window
	men		goncurses.Menu
	menlist	[]*goncurses.MenuItem
}

func getFiles(dir string) []os.DirEntry {
	entries, err := os.ReadDir(dir)
	if err != nil {
		panic(err)
	}

	return entries
}

func initWindow() (*goncurses.Window, *goncurses.Window, *goncurses.Window) {
	_, err := goncurses.Init()
	if err != nil {
		panic(err)
	}

	win1, err := goncurses.NewWindow(30, 50, 0, 0)
	if err != nil {
		panic(err)
	}

	win1.Border('|', '|', '-', '-', 'o', 'o', 'o', 'o')

	win2 := win1.Sub(20, 25, 1, 1)
	win3 := win1.Sub(20, 24, 1, 25)

	goncurses.Echo(false)
	goncurses.Cursor(0)

	return win1, win2, win3
}

func initMenu(ContentMod, ContentNoMod DirContents) (DirContents, DirContents) {
	test1 := make([]*goncurses.MenuItem, len(ContentMod.files))
	test2 := make([]*goncurses.MenuItem, len(ContentNoMod.files))

	for i, e := range ContentMod.files {
		item, err := goncurses.NewItem(e.Name(), strconv.Itoa(i))
		if err != nil {
			panic(err)
		}
		test1[i] = item
	}

	for i, e := range ContentNoMod.files {
		item, err := goncurses.NewItem(e.Name(), strconv.Itoa(i))
		if err != nil {
			panic(err)
		}
		test2[i] = item
	}

		ContentMod.men.SetItems(test1)
		ContentNoMod.men.SetItems(test2)

		ContentMod.men.SetWindow(ContentMod.win)
		ContentNoMod.men.SetWindow(ContentNoMod.win)

		return ContentMod, ContentNoMod
}

func main() {
	var dirOne string = "/home/em/repos/godie/mod/"
	var dirTwo string = "/home/em/repos/godie/no_mod/"

	winBack, winMod, winNoMod := initWindow()

	menuMod := goncurses.Menu{}
	menuModList := []*goncurses.MenuItem{}

	menuNotMod := goncurses.Menu{}
	menuNotModList := []*goncurses.MenuItem{}

	ContentMod := DirContents{dirOne, getFiles(dirOne), winMod, menuMod, menuModList}
	ContentNoMod := DirContents{dirTwo, getFiles(dirTwo), winNoMod, menuNotMod, menuNotModList}

	ContentMod, ContentNoMod = initMenu(ContentMod, ContentNoMod)

	winMod.MovePrintln(0, 1, "Active Mods:")
	winNoMod.MovePrintln(0, 1, "Inactive Mods:")

	for i, e := range ContentMod.menlist {
		winMod.MovePrintln(i + 1, 1, e.Name())	
	}

	ContentMod.men.Post()
//	if err != nil {
//		panic(err)
//	}
	ContentNoMod.men.Post()


	/*
	for i, e := range filesOne {
		winMod.MovePrintln(i + 1, 1, e.Name())
	}
	for i, e := range filesTwo {
		winNoMod.MovePrintln(i + 1, 1, e.Name())
	}
	*/
	winBack.GetChar()

	defer goncurses.End()
}
