Name: Erico Royce Bayani
Collaborated with: No one, but I did go to office hours


This lab, we had to make a "game" (there's no way to win, just not to die). We would do this by leveraging the f* functions in the stdio.h file to navigate
through given room files and display its contents on a terminal, which we had to do by getting familiar with vt100 commands. The vt100 commands would make it
so that words and backgrounds can be displayed. The actual game would be played in an event loop, where a switch statement would continuously check for the 
inputted char using getchar(), and we would only check for the characters n,s,e,and w, and q to end the game. The file i/o functions come in when making
Game.c, the library we had to make to actually run the game. There's a smaller library Player.h that just handled adding things to an inventory, and 
finding things in the inventory. The Game.c functions would handle going 1 of 4 cardinal directions, based on whether or not they can be traversed,
initializing the starting room of the game, and accessing the various pieces of room information such as the title, the description, and the exits.
The GameGo functions would open the file, load the data it parses from the file to a module-level struct, and then close the file. This prevents leaks,
and is also worth -4 points if we don't do this. Actual file traversal, we had to learn, but we were helped by the fact that certain aspects of the
file have a set size, so we can fread() and fseek() as much as we need. Also, the files are encrypted, and we have to decrypt the data before we can
properly use it according to a base key added by the room number.

I read the manual but was still a little confused, especially with the vt100 commands and file i/o, so I first played around with the vt100 commands a little bit
until I learned about certain other commands not given in the lab manual (the link to the full list were given in the manual). The specific command that made 
things much better was the ^[[H command that puts the cursor to the top left corner of the screen, since the clear option makes the cursor go down too much. 
After I solved that, I tried to wrap my head around how to use the file i/o functions. After a while of reading the documenation, I eventually figured out
how to read a file, and from there, I gradually got the grasp of things, especially after I went to office hours. After a bit of debugging, I was able to get 
the GameGoNorth function to work as intented, so I would just copy that function over to the other GameGo* functions and change the direction, and from there,
after more debugging, my lab was finished. I didn't work with anyone else on the lab, but I did go to office hours. Max confirmed how I was supposed to run through
the file, so I was relieved that I wasn't crazy.

I enjoyed this lab a lot. It works pretty well. I'm kind of miffed that we don't actually have a way to win the game, now that I notice the functions we had make
didn't say anything about a win condition. It ended up taking me about 8 hours in total to do the lab over 2 days. I enjoyed playing around with the vt100 commands
and the prospect of actually making a game. It was fun to see the final product working as intented. I'm not sure what exactly I dislike other than again, no winning
the game. I also think this is a very important lab because now I'm comfortable with having a cursor in my head running through a file, and I think being comfortable
with the file i/o functions would open up a lot more possibilities. I would definitely have an actual way to end the game added. The hardest part was learning how to use
the file i/o functions correctly. I think the points are fair. The manual is a little clear, it's just that the concepts take a while to get used to. Also whatever we need
to look up we're already told what or where. I think the demonstrations of the file i/o functions in class were nice to have.