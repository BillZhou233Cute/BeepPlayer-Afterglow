args=-Wall -Wextra -O2

bp: beepplayer.o decode.o play.o
	cc -o bp beepplayer.o decode.o play.o -lm

beepplayer.o: beepplayer.c decode.h play.h
	cc -c $(args) beepplayer.c

decode.o: decode.c decode.h
	cc -c $(args) decode.c

play.o: play.c play.h
	cc -c $(args) play.c

chmod: bp
	sudo chown root bp
	sudo chmod 4774 bp

clean:
	-rm -f bp *.o
