CC = gcc
CFLAGS = -g -Wall -Wextra

all: create_obj snake snake_test client server


create_obj:
	@ if [ ! -d "obj" ]; then mkdir obj; echo "mkdir obj";fi


snake: obj/main.o obj/game.o obj/types.o obj/game.o obj/AI.o obj/queue.o
	$(CC) $(CFLAGS) obj/types.o obj/game.o obj/AI.o obj/main.o obj/queue.o -o snake -lm

obj/main.o: src/main.c src/game.h
	$(CC) $(CFLAGS) -c src/main.c -o $@

obj/AI.o: src/AI.c src/game.h src/types.h
	$(CC) $(CFLAGS) -c src/AI.c -o $@

obj/game.o: src/game.c src/types.h src/AI.h src/queue.h
	$(CC) $(CFLAGS) -c src/game.c -o $@

obj/game_with_no_display.o: src/game.c src/types.h src/AI.h src/queue.h
	$(CC) -c src/game.c -DDO_NOT_DISPLAY -o obj/game_with_no_display.o -o $@

obj/types.o: src/types.c
	$(CC) $(CFLAGS) -c src/types.c -o $@

obj/queue.o: src/queue.c
	$(CC) $(CFLAGS) -c src/queue.c -o $@



snake_test: obj/main_test.o obj/test_types.o obj/types.o obj/game_with_no_display.o obj/AI.o obj/test_AI.o
	$(CC) $(CFLAGS) obj/main_test.o obj/test_types.o obj/test_AI.o obj/types.o obj/game_with_no_display.o obj/AI.o obj/queue.o -o snake_test -lm

obj/main_test.o: src/main_test.c src/test_types.h
	$(CC) $(CFLAGS) -c src/main_test.c -o $@

obj/test_types.o: src/test_types.c src/test_types.h src/types.h
	$(CC) $(CFLAGS) -c src/test_types.c -o $@

obj/test_AI.o: src/test_AI.c src/test_AI.h src/types.h
	$(CC) $(CFLAGS) -c src/test_AI.c -o $@



client: src/client.c obj/types.o obj/game.o obj/queue.o obj/AI.o
	$(CC) $(CFLAGS) src/client.c obj/types.o obj/game.o obj/queue.o obj/AI.o -lm -o client



server: src/server.c obj/types.o obj/game_with_no_display.o obj/queue.o obj/AI.o
	$(CC) $(CFLAGS) src/server.c obj/types.o obj/game_with_no_display.o obj/queue.o obj/AI.o -lpthread -lm -o server



clean:
	@rm -f *.o
