LDFLAGS= -framework GLUT -framework OpenGL -framework Cocoa 

sketchpad : sketchpad.c
	gcc -o $@ $< $(LDFLAGS)
