all: rpg


rpg.o: rpg.c
	gcc -Wall -c rpg.c
	
game.o: Game.c Game.h
	gcc -Wall -c Game.c
	
player.o: Player.c Player.h
	gcc -Wall -c Player.c
	
rpg: rpg.o Game.o Player.o UNIXBOARD.h
	gcc -Wall rpg.o Game.o Player.o -o rpg
	
clean: 
	rm *.o
	rm rpg