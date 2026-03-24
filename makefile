# ================================
# VBCC Amiga Makefile (Windows 11)
# Einfache Version mit Log
# ================================

CC      = vc
TARGET  = RC
LOGFILE = build.log

# Source
SRCS    = RC.c
OBJS    = $(SRCS:.c=.o)

# Flags
CFLAGS  = -c99 -O2
ARCH    = +aos68k
LIBS    = -lamiga -lauto -lreaction

# ================================
# Build
# ================================

all: $(TARGET)
	@echo.
	@echo ========================================
	@echo Build abgeschlossen!
	@echo Ausgabe siehe: $(LOGFILE)
	@echo ========================================

$(TARGET): $(OBJS)
	@echo [LINK] $(TARGET)
	@echo [LINK] $(TARGET) >> $(LOGFILE)
	$(CC) $(ARCH) $(OBJS) -o $(TARGET) $(LIBS) >> $(LOGFILE) 2>&1

%.o: %.c
	@echo [CC] $<
	@echo [CC] $< >> $(LOGFILE)
	$(CC) $(ARCH) $(CFLAGS) -c $< -o $@ >> $(LOGFILE) 2>&1

# ================================
# Clean
# ================================

clean:
	@echo Cleaning...
	@if exist *.o del /Q *.o 2>NUL
	@if exist $(TARGET) del /Q $(TARGET) 2>NUL
	@if exist $(LOGFILE) del /Q $(LOGFILE) 2>NUL
	@echo Clean done.

# ================================
# Rebuild
# ================================

rebuild: clean
	@echo ========================================= > $(LOGFILE)
	@echo RC Build Log >> $(LOGFILE)
	@echo ========================================= >> $(LOGFILE)
	@echo. >> $(LOGFILE)
	@$(MAKE) --no-print-directory all

# ================================
# Zeige Log
# ================================

showlog:
	@if exist $(LOGFILE) (type $(LOGFILE)) else (echo Keine Log-Datei vorhanden!)

# ================================
# Help
# ================================

help:
	@echo.
	@echo RC Makefile Commands:
	@echo.
	@echo   make         - Build RC (Output -^> build.log)
	@echo   make rebuild - Clean und build
	@echo   make showlog - Zeige build.log an
	@echo   make clean   - Loesche generierte Dateien
	@echo   make help    - Diese Hilfe
	@echo.

.PHONY: all clean rebuild showlog help 