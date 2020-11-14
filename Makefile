APP := xbmpwall


# Xaw
TARGET_LIB="-lXaw"

# Xaw3d
#TARGET_LIB="-lXaw3d"

# neXtaw
#TARGET_LIB="-lneXtaw"

CLIBS := ${TARGET_LIB} -lX11 -lXt -lX11 -lXpm
CLIBS_RELEASE := -Wl,-s,--sort-common,--as-needed,-z,relro

CFLAGS := -std=gnu99 -D_GNU_SOURCE
CFLAGS_RELEASE := -O3 -flto -fstack-protector-strong -Wno-unused-parameter
CFLAGS_DEBUG := -Wall -Wextra -pedantic -O0 -ggdb -DDEBUG

CC := gcc

SOURCE := $(wildcard *.c)

OBJ := $(patsubst %.c,%.o,$(SOURCE))

release: CFLAGS += $(CFLAGS_RELEASE)
release: CLIBS += $(CLIBS_RELEASE)
release: $(APP)

debug: CFLAGS += $(CFLAGS_DEBUG)
debug: $(APP)

$(APP): $(OBJ)
		$(CC) $(OBJ) $(CLIBS) -o $(APP)

.o:
		$(CC) $(CFLAGS) -c $<

clean:
		rm $(APP) *.o

#  vim: set ts=4 sw=4 tw=500 noet :
