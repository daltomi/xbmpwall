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
#pragma once

#include "config.h"

#ifndef SH
#error "config.h: SH not defined."
#endif

#ifndef XSETROOT
#error "config.h: XSETROOT not defined."
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <sys/wait.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Dialog.h>

#include "data/xbmpwall.xbm"
#include "hexcolors.h"

#define APP_NAME "XBmpWall"
#define APP_TITLE APP_NAME " " VERSION

#define SCRIPT "xbmpwall.sh"
#define SCRIPT_HIDE "/."SCRIPT
#define SCRIPT_HEAD "#!" SH "\n"

#define SCRIPT_XSETROOT XSETROOT " -bitmap %s -bg '%s' -fg '%s'"

#define INFO_BITMAPS APP_TITLE "\nOpen: %d"
#define INFO_COLORS "Colors: %zu"

#define WIN_WIDTH 640
#define WIN_HEIGHT 600

#define ITEM_SIZE 38

#ifndef HAVE_LIMITS_H
#ifndef PATH_MAX
#if defined(_POSIX_PATH_MAX)
#define PATH_MAX _POSIX_PATH_MAX
#elif defined(MAXPATHLEN)
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 4096
#endif
#endif
#endif
