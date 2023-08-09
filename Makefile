CFLAGS=-Wall -Wextra -pedantic -ggdb
CFLAGS += $(shell pkg-config --cflags gdk-pixbuf-2.0)
CFLAGS += $(shell pkg-config --cflags libavformat)
LIBS = $(shell pkg-config --libs libmpdclient)
LIBS+= $(shell pkg-config --libs gdk-pixbuf-2.0)
LIBS+= $(shell pkg-config --libs libavformat)
FILES = $(shell find ./src -name "*.c")
VERSION=$(shell git describe --long --tags)

ASSETS=index.html favicon.png logo.svg build/bundle.js build/bundle.css

htmpd: src/*.c src/*.h src/version.h src/assets.c
	$(CC) -o htmpd $(FILES)  $(CFLAGS) $(LIBS)

src/assets.c: dist/index.html dist/build/bundle.js dist/build/bundle.css
	cd dist && python ./../../tools/bin2c.py ./../../src/assets.c $(ASSETS)

dist/build/bundle.js:
	cd web && pnpm run build

src/version.h: src/version.def.h
	sed 's/v0.0.0/$(VERSION)/' src/version.def.h > src/version.h

compile_commands:
	compiledb make
	compdb -p ./ list > compile_commands_with_headers.json 2>/dev/null
	rm compile_commands.json 
	mv compile_commands_with_headers.json compile_commands.json

clean:
	rm -f htmpd
	rm -f compile_commands.json
	rm -f ./src/version.h

