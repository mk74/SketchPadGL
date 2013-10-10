#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>

//----------
//data structures
//----------
typedef struct Color3f {
	float x, y, z;
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

Primitive *head_prim = NULL; 
Primitive *last_prim = NULL;

Vertex *last_vrtx = NULL; //keep last point of last prim
int last_name=0;


//----------
//global variables
//----------

int w=800;
int h=800;

int mouse_x = 0;
int mouse_y = 0;

int drawing_mode=GL_POINTS;
float line_width = 1;
Color3f color = {1.0, 1.0, 1.0};
int render_mode = GL_RENDER;



//----------
//Managing data structure
//----------

void addVrtx(Vertex *vrtx, int x, int y){
	Vertex *new_vrtx = malloc(sizeof(struct Vertex));
	new_vrtx->point.x = x;
	new_vrtx->point.y = y;
	new_vrtx->nxt_vrtx = NULL;
	vrtx->nxt_vrtx = new_vrtx;
	last_vrtx = new_vrtx;
}

void editVrtx(Vertex *vrtx, int x, int y){
	vrtx->point.x = x;
	vrtx->point.y = y;
}

float calcDistance(int x1, int y1, int x2, int y2){
	return sqrt(pow(x2-x1, 2) + pow(y2 - y1, 2));
}

Vertex* findNearestVrtx(Primitive *prim, int x, int y){ //TODO do on list of primitives
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

void startNewPrim(int x, int y){
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

void printAll(){
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

//Displaying

void drawPrim(Primitive *prim){
	Vertex *vrtx = prim->nxt_vrtx;
	glColor3f(prim->color.x, prim->color.y, prim->color.z);
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
}

void drawPrims(){
	Primitive *prim = head_prim;
	while(prim != NULL){
		drawPrim(prim);
		prim = prim->nxt_prim;
	}
}

void drawPrimInteract(Primitive *prim){
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

void display() {
	//printAll();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawPrims();
    if(last_prim != NULL && last_vrtx != NULL){
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

//------------
//inetraction with user
//------------

void processHits (GLint hits, GLuint buffer[]){
   unsigned int i, j;
   GLuint names, *ptr;

   printf ("hits = %d\n", hits);
   ptr = (GLuint *) buffer;
   for (i = 0; i < hits; i++) { /*  for each hit  */
      names = *ptr;
      printf (" number of names for hit = %d\n", names); ptr++;
      printf("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
      printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
      printf ("   the name is ");
      for (j = 0; j < names; j++) {     /*  for each name */
         printf ("%d ", *ptr); ptr++;
      }
      printf ("\n");
   }
} 

void mouse(int btn, int state, int x, int y){
	GLuint nameBuffer[100];
	GLint hits;
	GLint viewport[4];
	int i;

    if(btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
    	if(drawing_mode == -1){
    		glSelectBuffer(100, nameBuffer);
    		render_mode = GL_SELECT;
    		glRenderMode(render_mode);
    		glInitNames();
    		glPushName(0);
    		glSelectBuffer(100, nameBuffer);

    		glGetIntegerv(GL_VIEWPORT, viewport);
    		glMatrixMode(GL_PROJECTION);

  			glPushMatrix();
  			glLoadIdentity();

  			gluPickMatrix( (GLdouble) x, (GLdouble) y, 100, 100, viewport);
  			gluOrtho2D(0, w, 0, h);

  			drawPrims();
  			glMatrixMode(GL_PROJECTION);

  			glPopMatrix();
  			glFlush();

  			render_mode = GL_RENDER;
    		hits = glRenderMode(render_mode);
    		processHits(hits, nameBuffer);
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

void passiveMotion(int x, int y){
	mouse_x = x;
	mouse_y = y;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y){
	switch(key){
		case 27: //ESCAPE
			exit(0);
			break;
		case 32:
			last_vrtx = NULL;
			glutPostRedisplay();
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
			color.x = (float)rand()/(float)RAND_MAX;
			color.y = (float)rand()/(float)RAND_MAX;
			color.z = (float)rand()/(float)RAND_MAX;
			break;
		case 'w':
			line_width = rand()%5;
			break;
		case 'd':
			drawing_mode = -1;
			break;
	}
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

    glutReshapeFunc(reshape);
    glutDisplayFunc(display); 
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(passiveMotion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return EXIT_SUCCESS;
}