/*
  XBmpWall (xbmpwall)

  Copyright (C) 2019-2022 by Daniel T. Borelli <danieltborelli@gmail.com>

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

#include "xbmpwall.h"

char const *const appResources[] = {
  "*.background: #ECE9D8",
  "*.foreground: blue",
  "*viewport.forceBars:True",
  "*viewport.allowVert:True",
  "*viewport.allowHoriz:True",
  "*viewport.useRight:True",
  "*viewport.useBottom:True",
  NULL
};


static Widget appWidget,
              boxColors;

static Atom atomDeleteWindow;

static XtAppContext appContext;

static Display *display = NULL;

static char *bitmapName = NULL,
            *colorFg = NULL,
            *colorBg = NULL,
            *bashcmd = NULL;

static Boolean activeColorFg = True;

static Cursor cursorUp = None,
              cursorDown = None;


#define Free(p) do {  \
  free(p);            \
  p = NULL;           \
}while(0);


#ifdef DEBUG

#define dbg_log(...) do {          \
  fprintf(stderr, ##__VA_ARGS__);  \
  fprintf(stderr, "\n");           \
} while(0)

#define dbg_warning(...)  dbg_log("Warning: " __VA_ARGS__)

#define dbg_error(...)    dbg_log("Error: "  __VA_ARGS__)

#define dbg_notice(...)   dbg_log("Notice: "  __VA_ARGS__)

#define dbg_assert(...)  do { \
  dbg_error(__VA_ARGS__);     \
  assert(0);                  \
} while(0);

#else
#define dbg_warning(...)
#define dbg_error(...)
#define dbg_notice(...)
#define dbg_assert(...)
#endif



static char *get_home_env(void)
{
  char const *const home_env = getenv("HOME");

  if (NULL == home_env) {
    fprintf(stderr, "The HOME environment variable not set.\n"
        "Abnormal termination.");

    exit(EXIT_FAILURE);
  }
  return (char*)home_env;
}


/* Intenta asignar un valor a bitmapName, sino NULL.
 * Sobrescribe colorFg y colorBg, sino default.
 * */
static void ParseBashScript(void)
{
  char buffer[PATH_MAX];
  char *const filename = buffer;

  strncpy(filename, get_home_env(), PATH_MAX - 1);
  strncat(filename, SCRIPT_HIDE, PATH_MAX - 1);

  FILE *const fd = fopen(filename, "r");

  if (!fd) {
    /* Ignora el error, puede que el archivo no exista la primera vez.*/
    dbg_notice("ParseBashScript: failed open file %s", filename);
    return;
  }

  errno = 0;

  /* discard first line */
  if (!fgets(buffer, PATH_MAX, fd) || !fgets(buffer, PATH_MAX, fd)) {
    perror("ParseBashScript: fgets failed");
    fclose(fd);
    /* Èste error no se ignora.
     * Si el archivo existe debe poder accederse a él.
     * */
    exit(EXIT_FAILURE);
  }

  fclose(fd);

  bitmapName = calloc(PATH_MAX, sizeof(*bitmapName));

  char bg[10]; /* '#000000'\0 */
  char fg[10];

  if (sscanf(buffer, "%*s -bitmap %s -bg %s -fg %s", bitmapName, bg, fg) == EOF) {
      Free(bitmapName);
      dbg_error("ParseBashScript: scanf() = EOF");
      return;
  }

  // ignoring the compiler warning: stringop-truncation
  strncpy(colorBg, &bg[1], 7); /* trim ' ' */
  strncpy(colorFg, &fg[1], 7);
  colorBg[7] = '\0';
  colorFg[7] = '\0';

  dbg_notice("ParseBashScript: colorFg:%s, colorBg:%s\n", colorFg, colorBg);
}


static void Quit(Widget w, XEvent *event, String *params , Cardinal *nparams)
{
  (void)w;      /*UNUSED*/
  (void)params; /*UNUSED*/
  (void)nparams;/*UNUSED*/

  if ((event->type == ClientMessage) &&
      (event->xclient.data.l[0] != (long int)atomDeleteWindow)) {
    return;
  }

  if (NULL == bashcmd) {
    dbg_notice("Quit: bashcmd == NULL");
    exit(EXIT_SUCCESS);
  }

  char filename[PATH_MAX];

  strncpy(filename, get_home_env(), PATH_MAX - 1);
  strncat(filename, SCRIPT_HIDE, PATH_MAX - 1);

  errno = 0;

  FILE *const file = fopen(filename, "w+");

  if (!file) {
    Free(bashcmd);
    fprintf(stderr, APP_NAME ": failed open file:%s\n", filename);
    perror(APP_NAME);
    exit(EXIT_FAILURE);
  }

  errno = 0;

  if (fwrite(SCRIPT_HEAD, strlen(SCRIPT_HEAD), 1, file) == 0 ||
      fwrite(bashcmd, strlen(bashcmd), 1, file) == 0) {
    Free(bashcmd);
    fprintf(stderr, APP_NAME ": failed to write file:%s\n", filename);
    perror(APP_NAME);
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fclose(file);
  fprintf(stdout, "%s\n", bashcmd);
  Free(bashcmd);
  chmod(filename, S_IRWXU);
  exit(EXIT_SUCCESS);
}


static void set_bashcmd(char const fmt[static 1], ...)
{
  va_list ap;
  int n = 0;

  va_start(ap, fmt);
  n = vsnprintf(&(char){0}, (size_t){0}, fmt, ap);
  va_end(ap);

  assert(n > 0);

  size_t const size = n + 1;

  Free(bashcmd);

  bashcmd = calloc(size, sizeof(*bashcmd));

  assert(bashcmd != NULL);

  va_start(ap, fmt);
  n = vsnprintf(bashcmd, size, fmt, ap);
  va_end(ap);

  if (n < 0) {
    Free(bashcmd);
    dbg_assert("set_bashcmd: (n<0): vsnprintf() return %d", n);
  }
}


static void XSetRoot(char const filename[static 1])
{
  int wpid    = 0,
      pid     = 0,
      ret     = 0;

  set_bashcmd(SCRIPT_XSETROOT, filename, colorBg, colorFg);

  if ((pid = fork()) == 0) {
    ret = execl(XSETROOT, "xsetroot", "-bitmap", filename,
                "-bg", colorBg, "-fg", colorFg, NULL);

    if (ret == -1) {
      fprintf(stderr, "There was a failure while "
              "executing the process %s\n", XSETROOT);

      perror(APP_NAME);
    }
    exit(EXIT_SUCCESS);
  }

  while ((wpid = wait(&(int){0})) > 0);
}


static void ChangeCursor(void)
{
  if (activeColorFg) {
    XDefineCursor(display, XtWindow(boxColors), cursorUp);
  } else {
    XDefineCursor(display, XtWindow(boxColors), cursorDown);
  }
}


static void ConmuteStateColor(Widget w, XEvent *event,
                              String *params, Cardinal *nparams)
{
  (void)w;       /*UNUSED*/
  (void)event;   /*UNUSED*/
  (void)params;  /*UNUSED*/
  (void)nparams; /*UNUSED*/

  activeColorFg = !activeColorFg;
  ChangeCursor();
}


static void SetWallpaper(Widget w, XtPointer clientData, XtPointer callData)
{
  (void)w;        /*UNUSED*/
  (void)callData; /*UNUSED*/

  bitmapName = (char*)clientData;
  assert(bitmapName != NULL);
  XtSetSensitive(appWidget, False);
  XSetRoot(bitmapName);
  XtSetSensitive(appWidget, True);
}


static void SetColor(Widget w, XtPointer clientData, XtPointer callData)
{
  (void)w;        /*UNUSED*/
  (void)callData; /*UNUSED*/

  assert(clientData != NULL);

  if (activeColorFg) {
    colorFg = (char*)clientData;
  } else {
    colorBg = (char*)clientData;
  }

  if (bitmapName) {
    XSetRoot(bitmapName);
  }
}


int main(int argc, char *argv[argc + 1])
{
  if (argc < 2) {
    fprintf(stderr, "Missing parameters: file name .xbm\n");
    exit(EXIT_FAILURE);
  }

  if (argc == 2) {
    if (argv[1][0] == '-' && argv[1][1] == 'v') {
      printf("%s\n", APP_TITLE);
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

  display = XtDisplay(appWidget);

  int const screenId     = DefaultScreen(display);
  Screen *const screen   = DefaultScreenOfDisplay(display);
  int const depth        = DefaultDepth(display, 0);
  Pixel const fg         = BlackPixelOfScreen(screen);
  Pixel const bg         = WhitePixelOfScreen(screen);
  Pixmap const icon      = XCreatePixmapFromBitmapData(display,
                            RootWindowOfScreen(screen), (char *)icon_bits,
                            icon_width, icon_height, fg, bg, depth);

  Dimension const x = (XDisplayWidth(display, screenId) - WIN_WIDTH) / 2;
  Dimension const y = (XDisplayHeight(display, screenId) - WIN_HEIGHT) / 2;

  Arg args[] = {
    {XtNiconPixmap, (XtArgVal)icon              },
    {XtNtitle,      (XtArgVal)(char*)APP_TITLE  },
    {XtNx,          (XtArgVal)x                 },
    {XtNy,          (XtArgVal)y                 },
    {XtNwidth,      (XtArgVal)WIN_WIDTH         },
    {XtNheight,     (XtArgVal)WIN_HEIGHT        },
    {XtNminWidth,   (XtArgVal)WIN_WIDTH /  2    },
    {XtNminHeight,  (XtArgVal)WIN_HEIGHT / 2    },
  };

  XtSetValues(appWidget, args, XtNumber(args));

  XtRealizeWidget(appWidget);

  XtActionsRec actions[] = {
    {"quit", Quit                           },
    {"conmuteStateColor", ConmuteStateColor },
  };

  XtAppAddActions(appContext, actions, XtNumber(actions));

  XtOverrideTranslations(appWidget,
      XtParseTranslationTable("<Message>WM_PROTOCOLS: quit()\n"));

  atomDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, XtWindow(appWidget), &atomDeleteWindow, 1);

  Widget const paned = XtVaCreateManagedWidget("paned", panedWidgetClass,
      appWidget,
      XtNwidth, WIN_WIDTH,
      XtNheight, WIN_HEIGHT,
      XtNmax, WIN_HEIGHT - 240,
      NULL);

  Widget const viewport = XtVaCreateManagedWidget("viewport", viewportWidgetClass,
        paned,
        XtNwidth, WIN_WIDTH,
        XtNheight, WIN_HEIGHT,
        XtNmax, (WIN_HEIGHT / 2) + 140,
        NULL);

  Widget const boxBitmaps = XtVaCreateManagedWidget("box", boxWidgetClass,
          viewport,
          XtNwidth, WIN_WIDTH,
          XtNheight, WIN_HEIGHT,
          NULL);

  Widget const infoBitmaps = XtVaCreateManagedWidget("info", labelWidgetClass,
             boxBitmaps,
             XtNlabel, INFO_BITMAPS,
             NULL);

  Widget const viewportColors = XtVaCreateManagedWidget("viewport", viewportWidgetClass,
              paned,
              XtNwidth, WIN_WIDTH - 2 ,
              XtNheight, WIN_HEIGHT - 2,
              NULL);

  boxColors = XtVaCreateManagedWidget("box", boxWidgetClass,
        viewportColors,
        XtNwidth, WIN_WIDTH,
        XtNheight, WIN_HEIGHT,
        NULL);

  Widget const infoColors = XtVaCreateManagedWidget("info", labelWidgetClass,
            boxColors,
            XtNlabel, INFO_COLORS,
            NULL);

  char translationTable[] =  "<Key>space: conmuteStateColor()\n";
  XtOverrideTranslations(paned, XtParseTranslationTable(translationTable));

  /* Load bitmaps */
  int nbitmaps = 0;

  for(int i = 1; i < argc; ++i) {

    char const *const filename = argv[i];

    unsigned int width, height;
    unsigned char *data = NULL;
    int hotX, hotY;

    if (XReadBitmapFileData(filename, &width, &height, &data, &hotX, &hotY)
        != BitmapSuccess) {
      fprintf(stderr, "Error reading the bitmap file: %s\n", filename);
      exit(EXIT_FAILURE);
    }

    if (!data) {
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

    XtAddCallback(widget, XtNcallback, SetWallpaper, (char*)filename);
    XFree(data);
    nbitmaps = i;
  }

  size_t const ncolors = sizeof(hexColors) / sizeof(hexColors[0]);
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

  cursorUp  = XCreateFontCursor(display, XC_based_arrow_up);
  cursorDown  = XCreateFontCursor(display, XC_based_arrow_down);

  ChangeCursor();

  XtAppMainLoop(appContext);
  return EXIT_SUCCESS;
}
