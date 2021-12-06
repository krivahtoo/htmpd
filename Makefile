CFLAGS=-Wall -Wextra -pedantic -ggdb
LIBS=-lm
FILES=`find . -name "*.c"`

htmpd: src/main.c
	$(CC) -o htmpd $(FILES)  $(CFLAGS) $(LIBS)

compile_commands:
	compiledb make
	compdb -p ./ list > compile_commands_with_headers.json 2>/dev/null
	rm compile_commands.json 
	mv compile_commands_with_headers.json compile_commands.json
	rm compile_commands_with_headers.json

clean:
	rm -f htmpd
	rm -f compile_commands.json
