#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>
#define HIT_LIST_SIZE 100
#define PICK_SIZE 50

#define PULSATING_INTERVAL 100
#define POINT_SIZE 1.0
#define BACKGROUND_COLOR_R 0.0
#define BACKGROUND_COLOR_G 0.0
#define BACKGROUND_COLOR_B 0.0

#define HIGHLIGHT_POINT_SIZE 4.0
#define HIGHLIGHT_COLOR_R 1.0
#define HIGHLIGHT_COLOR_G 1.0
#define HIGHLIGHT_COLOR_B 1.0

#define START_COLOR_R 0.0
#define START_COLOR_G 0.0
#define START_COLOR_B 1.0
#define START_LINE_WIDTH 1
#define START_W 800
#define START_H 800
#define START_DRAWING_MODE GL_TRIANGLES


//----------
//data structures
//----------
typedef struct Color3f {
	float r, g, b;
} Color3f;

typedef struct Point {
	int x;
	int y;
} Point;

typedef struct Vertex {
	Point point;
	struct Vertex *nxt_vrtx;
} Vertex;

typedef struct Primitive {
	int name;
	int mode;
	Color3f color;
	float line_width;
	Vertex *nxt_vrtx;
	struct Primitive *nxt_prim;
} Primitive;

//----------
//functions
//----------


//Vertexes(data structure)
void addVrtx(Vertex *vrtx, int x, int y);
void editVrtx(Vertex *vrtx, int x, int y);
Vertex* findNearestVrtx(Primitive *prim, int x, int y);

//Primitives (data structures/ drawing)
void startNewPrim(int x, int y);
Primitive* findPrim(int name);
void destroyPrim(Primitive *prim);
void destroyPrims();
void printAllPrims();

void drawPrim(Primitive *prim);
void drawPrimInteract(Primitive *prim);
void drawDraggedPrim(Primitive *prim, Vertex *vrtx, int x, int y);
void drawPrims();

//helper functions
float calcDistance(int x1, int y1, int x2, int y2);
void randomizeColor();
void randomizeLineWidth();
void finishObject();
void processHits (GLint hits, GLuint buffer[], int x, int y);

//popup menu
void mainMenu(int value);
void drawingModeMenu(int new_drawing_mode);
void colorMenu(int color_id);
void lineWidthMenu(int size);
void buildPopupMenu();

//glut Callbacks ( display/interaction)
void display();
void reshape(int new_w, int new_h);
void idle();

void keyboard(unsigned char key, int x, int y);
void mouse(int btn, int state, int x, int y);
void motion(int x, int y);
void passiveMotion(int x, int y);

int main(int argc, char** argv);


//----------
//global variables
//----------

Primitive *head_prim = NULL; 
Primitive *last_prim = NULL;
Vertex *last_vrtx = NULL; //keep last point of last prim
int last_name=0;

Vertex *selected_vrtx = NULL;
Primitive *selected_prim = NULL;
int selected_x = 0;
int selected_y = 0;

int w=START_W;
int h=START_H;

int mouse_x = 0;
int mouse_y = 0;

int drawing_mode=START_DRAWING_MODE;
float line_width = START_LINE_WIDTH;
Color3f color = {START_COLOR_R, START_COLOR_G, START_COLOR_B};
int render_mode = GL_RENDER;
int pulsating_time = 0;




//----------
//Vertexes(data structure)
//----------

void addVrtx(Vertex *vrtx, int x, int y)
{
	Vertex *new_vrtx = malloc(sizeof(struct Vertex));
	new_vrtx->point.x = x;
	new_vrtx->point.y = y;
	new_vrtx->nxt_vrtx = NULL;
	vrtx->nxt_vrtx = new_vrtx;
	last_vrtx = new_vrtx;
}

void editVrtx(Vertex *vrtx, int x, int y)
{
	vrtx->point.x = x;
	vrtx->point.y = y;
}

Vertex* findNearestVrtx(Primitive *prim, int x, int y)
{
	float min_dist, current_dist;
	Vertex *min_vrtx = prim->nxt_vrtx;
	min_dist = calcDistance(min_vrtx->point.x, min_vrtx->point.y, x, y);
	Vertex *current_vrtx = min_vrtx->nxt_vrtx;
	while(current_vrtx != NULL){
		current_dist = calcDistance(current_vrtx->point.x, current_vrtx->point.y, x, y);
		if(current_dist < min_dist){
			min_vrtx = current_vrtx;
			min_dist = current_dist;
		}
		current_vrtx = current_vrtx->nxt_vrtx;
	}
	return min_vrtx;
}

//----------
//Primitives (data structures/ drawing)
//----------

void startNewPrim(int x, int y)
{
	//create new primitive
	Primitive *new_prim = malloc(sizeof(struct Primitive));
	new_prim->name = ++last_name;
	new_prim->mode = drawing_mode;
	new_prim->line_width = line_width;
	new_prim->color = color;
	Vertex *fst_vrtx = malloc(sizeof(struct Vertex));
	fst_vrtx->point.x = x;
	fst_vrtx->point.y = y;
	fst_vrtx->nxt_vrtx = NULL;
	new_prim->nxt_vrtx = fst_vrtx;
	new_prim->nxt_prim = NULL;

	//add this primitive to our data structure
	if(last_prim == NULL){
		head_prim = new_prim;
		last_prim = new_prim;
	}else{
		last_prim->nxt_prim = new_prim;
		last_prim = new_prim;
	}
	last_vrtx = fst_vrtx;
}

Primitive* findPrim(int name)
{
	Primitive *current=head_prim;
	while( current != NULL ){
		if( current->name == name)
			return current;
		current = current -> nxt_prim;
	}
	return NULL;
}

void destroyPrim(Primitive *prim)
{
	Vertex *vrtx = prim->nxt_vrtx;
    while(vrtx != NULL){
       	Vertex *tmp_vrtx = vrtx->nxt_vrtx;
       	free(vrtx);
        vrtx = tmp_vrtx;
    }
    free(prim);	
}

void destroyPrims()
{
	Primitive *prim = head_prim;
	while(prim != NULL){
		Primitive *tmp_prim = prim->nxt_prim;
		destroyPrim(prim);
		prim = tmp_prim;
	}
	last_vrtx = NULL;
	last_prim = NULL;
	head_prim = NULL;
	last_name = 0;
}

void printAllPrims()
{
	Primitive *prim = head_prim;
	printf("Primitives:\n");
	while(prim != NULL){
		printf("prim %d:", prim->name);
		Vertex *vrtx = prim->nxt_vrtx;
		while(vrtx != NULL){
        	printf("(%d, %d) ", vrtx->point.x, vrtx->point.y);
        	vrtx = vrtx->nxt_vrtx;
        }
        printf("\n\n");
        prim = prim->nxt_prim;
	}
}

//privimites drawing

void drawPrim(Primitive *prim)
{
	//draw prim, if SELECT mode add its name to stack
	Vertex *vrtx = prim->nxt_vrtx;
	glColor3f(prim->color.r, prim->color.g, prim->color.b);
	glLineWidth(prim->line_width);
	if(render_mode == GL_SELECT){ 
		glLoadName(prim->name);
	}
    glBegin(prim->mode);
    	while(vrtx != NULL){
        	glVertex2i(vrtx->point.x, vrtx->point.y);
        	vrtx = vrtx->nxt_vrtx;
        }
    glEnd();
    //highlight vertexes if we are in non-drawing mode
    if(drawing_mode == -1){
    	glColor3f(HIGHLIGHT_COLOR_R, HIGHLIGHT_COLOR_G, HIGHLIGHT_COLOR_B);
    	Vertex *vrtx = prim->nxt_vrtx;
    	glBegin(GL_POINTS);
    		while(vrtx != NULL){
    			glVertex2i(vrtx->point.x, vrtx->point.y);
    			vrtx = vrtx->nxt_vrtx;
    		}
    	glEnd();
    }
}

void drawPrimInteract(Primitive *prim)
{
	//erase part of primitive drawn so far
	//adds new vertex where mouse is pointing at
	//draws this primitive again
	//remove last vertex
	glLogicOp(GL_XOR);
	drawPrim(prim);
	Vertex *tmp_vrtx = last_vrtx;
	addVrtx(last_vrtx, mouse_x, mouse_y);
	drawPrim(prim);
	free(tmp_vrtx->nxt_vrtx);
	tmp_vrtx->nxt_vrtx = NULL;
	last_vrtx = tmp_vrtx;
	glLogicOp(GL_COPY);
}

void drawDraggedPrim(Primitive *prim, Vertex *vrtx, int x, int y)
{
	//drag one vertex 
	//draw primitive
	//drag back this vertex
	Point tmp_point= vrtx->point;
	editVrtx(vrtx, x, y);
	drawPrim(prim);
	editVrtx(vrtx, tmp_point.x, tmp_point.y);
}

void drawPrims()
{
	Primitive *prim = head_prim;
	while(prim != NULL){
		if(selected_prim!=prim)
			drawPrim(prim);
		else
			drawDraggedPrim(selected_prim, selected_vrtx, selected_x, selected_y);
		prim = prim->nxt_prim;
	}
}

//----------
//Helper functions
//----------

float calcDistance(int x1, int y1, int x2, int y2)
{
	return sqrt(pow(x2-x1, 2) + pow(y2 - y1, 2));
}

void randomizeColor()
{
	color.r = (float)rand()/(float)RAND_MAX;
	color.g = (float)rand()/(float)RAND_MAX;
	color.b = (float)rand()/(float)RAND_MAX;	
}

void randomizeLineWidth()
{
	line_width = rand()%10;
}

void finishObject()
{
	last_vrtx = NULL;
}

void processHits (GLint hits, GLuint buffer[], int x, int y)
{
   unsigned int i, j;
   GLuint names, *ptr;
   float difference;
   Primitive *prim;
   Vertex *vrtx;
   selected_prim = NULL;
   selected_vrtx = NULL;

   ptr = (GLuint *) buffer;
   for (i = 0; i < hits; i++) { /*  for each hit  */
      names = *ptr;
      ptr++; ptr++; ptr++;
      //find the nearest vertex from click:
      for (j = 0; j < names; j++) { 
   	     prim = findPrim(*ptr);
  	 	 vrtx = findNearestVrtx(prim, x, y); //finds the nearest vertex of specific primitve from click
  	 	 if(selected_vrtx==NULL){
  	 	 	selected_prim = prim;
  	 	 	selected_vrtx = vrtx;
  	 	 }else{
  	 	 	difference = calcDistance(vrtx->point.x, vrtx->point.y, x, y);
  	 	 	if(difference < calcDistance(selected_vrtx->point.x, selected_vrtx->point.y, x, y)){
  	 	 		selected_prim = prim;
  	 	 		selected_vrtx = vrtx;
  	 	 	}
  	 	 }
         ptr++;
      }
   }
   selected_x = x;
   selected_y = y;

} 

//----------
//Popup menu
//----------

void mainMenu(int value)
{
	switch(value){
		case -2:
			finishObject();
			drawing_mode = -2;
			break;
		case 1:
			destroyPrims();
			glutPostRedisplay();
			break;
		case 2: 
			exit(0);
			break;
	}
}

void drawingModeMenu(int new_drawing_mode)
{
	finishObject();
	drawing_mode = new_drawing_mode;
	glutPostRedisplay();
}

void colorMenu(int color_id)
{
	finishObject();
	switch(color_id){
		case 0:
			randomizeColor();
			break;
		case 1: //blue
			color.r = 0.0;
			color.g = 0.0;
			color.b = 1.0;
			break;
		case 2: //green
			color.r = 0.0;
			color.g = 1.0;
			color.b = 0.0;
			break;
		case 3: //purple
			color.r = 0.0;
			color.g = 1.0;
			color.b = 0.0;
			break;
		case 4: //red
			color.r = 1.0;
			color.g = 0.0;
			color.b = 0.0;
			break;
		case 5: //yellow
			color.r = 1.0;
			color.g = 1.0;
			color.b = 0.0;
			break;
	}
	glutPostRedisplay();
}

void lineWidthMenu(int size)
{
	finishObject();
	switch(size){
		case 0:
			randomizeLineWidth();
			break;
		default:
			line_width = size;
			break;
	}
	glutPostRedisplay();
}


void buildPopupMenu()
{
	int main_menu, color_menu, line_width_menu, drawing_mode_menu;
	drawing_mode_menu = glutCreateMenu(drawingModeMenu);
		glutAddMenuEntry("Points", GL_POINTS);
		glutAddMenuEntry("Lines", GL_LINES);
		glutAddMenuEntry("Line strip", GL_LINE_STRIP);
		glutAddMenuEntry("Line loop", GL_LINE_LOOP);
		glutAddMenuEntry("Polygon", GL_POLYGON);
		glutAddMenuEntry("Triangles", GL_TRIANGLES);
	color_menu = glutCreateMenu(colorMenu);
		glutAddMenuEntry("Randomize", 0);
		glutAddMenuEntry("Blue", 1);
		glutAddMenuEntry("Green", 2);
		glutAddMenuEntry("Purple", 3);
		glutAddMenuEntry("Red", 4);
		glutAddMenuEntry("Yellow", 5);
	line_width_menu = glutCreateMenu(lineWidthMenu);
		glutAddMenuEntry("Randomize", 0);
		glutAddMenuEntry("1", 1);
		glutAddMenuEntry("2", 2);
		glutAddMenuEntry("4", 4);
		glutAddMenuEntry("6", 6);
		glutAddMenuEntry("8", 8);
		glutAddMenuEntry("10", 10);
	main_menu = glutCreateMenu(mainMenu);
		glutAddMenuEntry("Select", -1);
		glutAddSubMenu("Draw", drawing_mode_menu);
		glutAddSubMenu("Colors", color_menu);
		glutAddSubMenu("Line width", line_width_menu);
		glutAddMenuEntry("Pulsating effect", -2);
    	glutAddMenuEntry("Clear screen", 1);
    	glutAddMenuEntry("Exit", 2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//----------
//Glut Callbacks (display/interaction)
//----------


void display() 
{
    glClearColor(BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    if(drawing_mode == -1)
    	glPointSize(HIGHLIGHT_POINT_SIZE);
    else
    	glPointSize(POINT_SIZE);
    if(pulsating_time < PULSATING_INTERVAL){
    	drawPrims();
    	if(last_prim != NULL && last_vrtx != NULL)
    		drawPrimInteract(last_prim);
    }
    glFlush();
}

void reshape(int new_w, int new_h)
{
  w = new_w;
  h = new_h;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, h, 0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_COLOR_LOGIC_OP);
}

void idle() 
{
	if(drawing_mode==-2){
		if(pulsating_time < PULSATING_INTERVAL*2)
			pulsating_time++;
		else
			pulsating_time=0;
		glutPostRedisplay();
	}
}

//inetraction with user

void keyboard(unsigned char key, int x, int y)
{
	int should_finish_object=1;
	switch(key){
		case 27: //ESCAPE
			exit(0);
			break;
		case 'p':
			drawing_mode = GL_POINTS;
			break;
		case 'l':
			drawing_mode = GL_LINES;
			break;
		case 's':
			drawing_mode = GL_LINE_STRIP;
			break;
		case 'o':
			drawing_mode = GL_LINE_LOOP;
			break;
		case 'y':
			drawing_mode = GL_POLYGON;
			break;
		case 't':
			drawing_mode = GL_TRIANGLES;
			break;
		case 'r':
			randomizeColor();
			break;
		case 'w':
			randomizeLineWidth();
			break;
		case 'd':
			drawing_mode = -1;
			break;
		case 'e':
			drawing_mode = -2;
			break;
		case 'c':
			destroyPrims();
			glutPostRedisplay();
			break;
		default:
			should_finish_object = 0;
			break;

	}
	if(should_finish_object || key == 32){ //user clicked space
		finishObject();
		glutPostRedisplay();
	}
	if(drawing_mode!=-2){
		pulsating_time=0;
	}
}

void mouse(int btn, int state, int x, int y)
{
	GLuint nameBuffer[HIT_LIST_SIZE];
	GLint hits;
	GLint viewport[4];
	int i;
	if(btn == GLUT_LEFT_BUTTON && state == GLUT_UP){
		//finish dragging object
		if(selected_prim && selected_vrtx){
			editVrtx(selected_vrtx, selected_x, selected_y);
			selected_x = 0;
			selected_y = 0;
			selected_vrtx = NULL;
			selected_prim = NULL;
		}
	}

    if(btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
    	if(drawing_mode == -1){
    		glSelectBuffer(HIT_LIST_SIZE, nameBuffer);
    		render_mode = GL_SELECT;
    		glRenderMode(render_mode);
    		glInitNames();
    		glPushName(0);

    		glGetIntegerv(GL_VIEWPORT, viewport);
    		glMatrixMode(GL_PROJECTION);

  			glPushMatrix();
  			glLoadIdentity();

  			gluPickMatrix( (GLdouble) x, (GLdouble) y, PICK_SIZE, PICK_SIZE, viewport);
  			gluOrtho2D(0, w, 0, h);

  			drawPrims();
  			glMatrixMode(GL_PROJECTION);

  			glPopMatrix();
  			glFlush();

  			render_mode = GL_RENDER;
    		hits = glRenderMode(render_mode);
    		processHits(hits, nameBuffer, x, y);
    	}else{
    		if(last_vrtx == NULL){
    			startNewPrim(mouse_x, mouse_y);
    		}else{
    			addVrtx(last_vrtx, mouse_x, mouse_y);
    		};
    	};	
    }
	glutPostRedisplay();
}

void motion(int x, int y)
{
	if(selected_prim != NULL){
		selected_x = x;
		selected_y = y;
		glutPostRedisplay();
	}
}

void passiveMotion(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
	glutPostRedisplay();
}

//------------
//Main program
//------------

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(w, h);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Sketchpad");
    buildPopupMenu();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display); 
    glutIdleFunc(idle);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passiveMotion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return EXIT_SUCCESS;
}