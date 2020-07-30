/*
	XBmpWall (xbmpwall)

	Copyright (C) 2019 by daltomi <daltomi@disroot.org> <danieltborelli@gmail.com>

	This file is part of xbmpwall.

	xbmpwall is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xbmpwall is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xbmpwall. If not, see <http://www.gnu.org/licenses/>.
*/


/* Inspired by: https://github.com/dkeg/bitmap-walls.git */


/*
	- libXaw, libX11, etc. see makefile.
	- GNU99
	- xsetroot
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Dialog.h>
#include <X11/cursorfont.h>

#include "desktop/xbmpwall.xbm"
#include "hexcolors.h"

#define VERSION			"1.14"
#define APP_NAME		"XBmpWall"
#define APP_VERSION		APP_NAME " v." VERSION
#define INFO_BITMAPS	APP_VERSION "\nOpen: %d"
#define INFO_COLORS		"Colors: %zu"
#define XBMPWALL_BASH	"/.xbmpwall.sh"
#define XSETROOT		"/usr/bin/xsetroot"
#define WIN_WIDTH		640
#define WIN_HEIGHT		600
#define ITEM_SIZE		48

static const char* execArgs =  XSETROOT " -bitmap %s -bg '%s' -fg '%s'";

const char* appResources[] = {
	"*.background: #ECE9D8",
	"*.foreground: blue",
	"*viewport.forceBars:True",
	"*viewport.allowVert:True",
	"*viewport.allowHoriz:True",
	"*viewport.useRight:True",
	"*viewport.useBottom:True",
	NULL
};


static Widget		appWidget,
					boxBitmaps,
					boxColors,
					paned,
					viewport;

static Atom			atomDeleteWindow;

static XtAppContext	appContext;

static Display		*display	= NULL;

static Screen		*screen		= NULL;

static int			screenId,
					depth;

static char			*bitmapName	= NULL,
					*colorFg	= NULL,
					*colorBg	= NULL,
					*bashcmd	= NULL;

static Boolean		activeColorFg	= True;

static Cursor		cursorUp,
					cursorDown;


#define Free(p) do {	\
	free(p);			\
	p = NULL;			\
}while(0);


/* Intenta asignar un valor a bitmapName, sino NULL.
 * Sobrescribe colorFg y colorBg, sino default.
 * */
static void ParseBashScript(void)
{
	char buffer[PATH_MAX];
	char *filename = buffer;
	char *home = getenv("HOME");

	assert(home);

	strncpy(filename, home, PATH_MAX - 1);
	strncat(filename, XBMPWALL_BASH, PATH_MAX - 1);

	errno = 0;

	FILE* fd = fopen(filename, "r");

	if(!fd) {
		/* Ignora el error, puede que el archivo no exista la primera vez.*/
		return;
	}

	/* discard first line */
	if(!fgets(buffer, PATH_MAX, fd) || !fgets(buffer, PATH_MAX, fd)) {
		fclose(fd);
		perror("ParseBashScript: fgets failed");
		/* Èste error no se ignora.
		 * Si el archivo existe debe poder accederse a él.
		 * */
		exit(EXIT_FAILURE);
	}

	fclose(fd);

	bitmapName = malloc(PATH_MAX);
	char bg[10]; /* '#000000'\0 */
	char fg[10];

	if(sscanf(buffer, "%*s -bitmap %s -bg %s -fg %s", bitmapName, bg, fg) == EOF) {
		Free(bitmapName);
		return;
	}

	strncpy(colorBg, &bg[1], 7); /* trim ' ' */
	strncpy(colorFg, &fg[1], 7);
	colorBg[7] = '\0';
	colorFg[7] = '\0';
}


static void Quit(Widget w, XEvent *event, String *params , Cardinal *nparams)
{
	if((event->type == ClientMessage) &&
	    (event->xclient.data.l[0] != (long int)atomDeleteWindow)) {
		return;
	}

	if(!bashcmd) {
		exit(EXIT_SUCCESS);
	}

	char filename[PATH_MAX];
	char *home = getenv("HOME");

	assert(home);

	strncpy(filename, home, PATH_MAX - 1);
	strncat(filename, XBMPWALL_BASH, PATH_MAX - 1);

	FILE *file = fopen(filename, "w+");

	if(!file) {
		Free(bashcmd);
		fprintf(stderr, "xbmpwall: failed open file:%s\n", filename);
		perror("xbmpwall");
		exit(EXIT_FAILURE);
	}

	if(fwrite("#!/bin/sh\n", 10, 1, file) == 0 ||
	    fwrite(bashcmd, strlen(bashcmd), 1, file) == 0) {
		fclose(file);
		Free(bashcmd);
		fprintf(stderr, "xbmpwall: failed to write file:%s\n", filename);
		perror("xbmpwall");
		exit(EXIT_FAILURE);
	}

	fclose(file);
	fprintf(stdout, "%s\n", bashcmd);
	Free(bashcmd);
	chmod(filename, S_IRWXU);
	exit(EXIT_SUCCESS);
}


static void XSetRoot(char const *filename)
{
	int wpid	= 0,
	    status	= 0,
	    pid		= 0,
	    ret		= 0;

	Free(bashcmd);

	ret = asprintf(&bashcmd, execArgs, filename, colorBg, colorFg);

	if((pid = fork()) == 0) {
		ret = execl(XSETROOT, "xsetroot", "-bitmap", filename,  "-bg", colorBg, "-fg", colorFg, NULL);
		if(ret == -1) {
			fprintf(stderr, "There was a failure while executing the process %s\n", XSETROOT);
			perror("xbmpwall:");
		}
		exit(EXIT_SUCCESS);
	}

	while ((wpid = wait(&status)) > 0);
}


static void ChangeCursor(void)
{
	if(activeColorFg) {
		XDefineCursor(display, XtWindow(boxColors), cursorUp);
	} else {
		XDefineCursor(display, XtWindow(boxColors), cursorDown);
	}
}


static void ConmuteStateColor(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
	activeColorFg = !activeColorFg;
	ChangeCursor();
}


static void SetWallpaper(Widget w, XtPointer clientData, XtPointer callData)
{
	bitmapName = (char*)clientData;
	assert(bitmapName);
	XtSetSensitive(appWidget, False);
	XSetRoot(bitmapName);
	XtSetSensitive(appWidget, True);
}

static void SetColor(Widget w, XtPointer clientData, XtPointer callData)
{
	assert(clientData);

	if(activeColorFg) {
		colorFg = (char*)clientData;
	} else {
		colorBg = (char*)clientData;
	}

	if (bitmapName) {
		XSetRoot(bitmapName);
	}
}



int main(int argc, char *argv[])
{
	if(argc < 2) {
		fprintf(stderr, "Missing parameters: file name .xbm\n");
		exit(EXIT_FAILURE);
	}

	if(argc == 2) {
		if(argv[1][0] == '-' && argv[1][1] == 'v') {
			fprintf(stdout, APP_VERSION);
			exit(EXIT_SUCCESS);
		}
	}

	/* Default colors. */
	colorFg = strdup("#000000"); /* 7 + 1 */
	colorBg = strdup("#FFFFFF");

	ParseBashScript();

	XtSetLanguageProc(NULL, NULL, NULL);

	appWidget = XtVaAppInitialize(&appContext, (char*)APP_NAME,
				NULL, 0,
				&argc, argv,
				(char**)appResources,
				NULL);

	display		= XtDisplay(appWidget);
	screenId	= DefaultScreen(display);
	screen		= DefaultScreenOfDisplay(display);
	depth		= DefaultDepth(display, 0);
	Pixel fg	= BlackPixelOfScreen(screen);
	Pixel bg	= WhitePixelOfScreen(screen);
	Pixmap icon	= XCreatePixmapFromBitmapData(display, RootWindowOfScreen(screen),
					(char *)icon_bits, icon_width, icon_height,
					fg, bg, depth);

	const Dimension x = (XDisplayWidth(display, screenId) - WIN_WIDTH) / 2;
	const Dimension y = (XDisplayHeight(display, screenId) - WIN_HEIGHT) / 2;

	Arg args[] = {
		{XtNiconPixmap,	(XtArgVal)icon					},
		{XtNtitle,		(XtArgVal)(char*)APP_VERSION	},
		{XtNx,			(XtArgVal)x						},
		{XtNy,			(XtArgVal)y						},
		{XtNwidth,		(XtArgVal)WIN_WIDTH				},
		{XtNheight,		(XtArgVal)WIN_HEIGHT			},
		{XtNminWidth,	(XtArgVal)WIN_WIDTH /  2		},
		{XtNminHeight,	(XtArgVal)WIN_HEIGHT / 2		},
	};
	XtSetValues(appWidget, args, XtNumber(args));

	XtRealizeWidget(appWidget);

	XtActionsRec actions[] = {
		{"quit", Quit							},
		{"conmuteStateColor", ConmuteStateColor	},
	};

	XtAppAddActions(appContext, actions, XtNumber(actions));
	XtOverrideTranslations(appWidget, XtParseTranslationTable("<Message>WM_PROTOCOLS: quit()\n"));
	atomDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, XtWindow(appWidget), &atomDeleteWindow, 1);

	paned = XtVaCreateManagedWidget("paned", panedWidgetClass,
			appWidget,
			XtNwidth, WIN_WIDTH,
			XtNheight, WIN_HEIGHT,
			XtNmax, WIN_HEIGHT - 240,
			NULL);

	viewport = XtVaCreateManagedWidget("viewport", viewportWidgetClass,
				paned,
				XtNwidth, WIN_WIDTH,
				XtNheight, WIN_HEIGHT,
				XtNmax, (WIN_HEIGHT / 2) + 140,
				NULL);

	boxBitmaps = XtVaCreateManagedWidget("box", boxWidgetClass,
					viewport,
					XtNwidth, WIN_WIDTH,
					XtNheight, WIN_HEIGHT,
					NULL);

	Widget infoBitmaps = XtVaCreateManagedWidget("info", labelWidgetClass,
						 boxBitmaps,
						 XtNlabel, INFO_BITMAPS,
						 NULL);

	Widget viewportColors = XtVaCreateManagedWidget("viewport", viewportWidgetClass,
							paned,
							XtNwidth, WIN_WIDTH - 2 ,
							XtNheight, WIN_HEIGHT - 2,
							NULL);

	boxColors = XtVaCreateManagedWidget("box", boxWidgetClass,
				viewportColors,
				XtNwidth, WIN_WIDTH,
				XtNheight, WIN_HEIGHT,
				NULL);

	Widget infoColors = XtVaCreateManagedWidget("info", labelWidgetClass,
						boxColors,
						XtNlabel, INFO_COLORS,
						NULL);

	char translationTable[] =  "<Key>space: conmuteStateColor()\n";
	XtOverrideTranslations(paned, XtParseTranslationTable(translationTable));

	/* Load bitmaps */
	int nbitmaps = 0;

	for(int i = 1; i < argc; ++i) {
		char *filename = strdup(argv[i]);
		assert(filename != NULL);

		unsigned int width, height;
		unsigned char *data = NULL;
		int hotX, hotY;

		if(XReadBitmapFileData(filename, &width, &height, &data, &hotX, &hotY) != BitmapSuccess) {
			fprintf(stderr, "Error reading the bitmap file: %s\n", filename);
			exit(EXIT_FAILURE);
		}

		if(!data) {
			continue;
		}

		Pixmap pixmap = XCreatePixmapFromBitmapData(display,
						RootWindowOfScreen(screen),
						(char *)data, width, height,
						fg, bg, depth);

		Widget widget = XtVaCreateManagedWidget(NULL,
						commandWidgetClass,
						boxBitmaps,
						XtNbackgroundPixmap, pixmap,
						XtNwidth, ITEM_SIZE,
						XtNheight, ITEM_SIZE,
						NULL);

		XtAddCallback(widget, XtNcallback, SetWallpaper, filename);
		XFree(data);
		++nbitmaps;
	}

	const size_t ncolors = sizeof(hexColors) / sizeof(hexColors[0]);
	char buffer[40];

	snprintf(buffer, sizeof(buffer), INFO_COLORS, ncolors);
	XtSetValues(infoColors, &(Arg){XtNlabel, (XtArgVal)buffer}, 1);

	Colormap colormap = XDefaultColormap(display, screenId);

	for(size_t i = 0; i < ncolors; i++) {
		XColor scrDef, exactDef;
		XAllocNamedColor(display, colormap, hexColors[i], &scrDef, &exactDef);

		Widget widget = XtVaCreateManagedWidget(NULL,
						commandWidgetClass,
						boxColors,
						XtNbackground, scrDef.pixel,
						XtNwidth, ITEM_SIZE / 2,
						XtNheight, ITEM_SIZE / 2,
						NULL);

		XtAddCallback(widget, XtNcallback, SetColor, (XtPointer)hexColors[i]);
	}

	snprintf(buffer, sizeof(buffer), INFO_BITMAPS, nbitmaps);
	XtSetValues(infoBitmaps, &(Arg){XtNlabel, (XtArgVal)buffer}, 1);

	cursorUp	= XCreateFontCursor(display, XC_based_arrow_up);
	cursorDown	= XCreateFontCursor(display, XC_based_arrow_down);

	ChangeCursor();

	XtAppMainLoop(appContext);
	return EXIT_SUCCESS;
}
