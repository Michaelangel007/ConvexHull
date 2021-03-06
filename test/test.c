#include <ccore/window.h>
#include <ccore/display.h>
#include <ccore/opengl.h>

#include <stdio.h>
#include <gl/GL.h>

#include <lodepng/lodepng.h>
#include <convexHull/convexHull.h>

#define TITLE "Convex Hull test case\n"
#define IMAGEDIR "test/source.png"
#define NODECOUNT_DEFAULT 16
#define PRECISION_DEFAULT 16.0f

unsigned char *pixels;
unsigned int nodeCount;
float precision;
unsigned int width, height;
convexHull hull;
GLuint texture;

void loadImage()
{
	// Load image from file using lodepng
	lodepng_decode32_file(&pixels, &width, &height, IMAGEDIR);
	printf("Loaded test image %dx%d\n", width, height);

	// Initialize openGL texture
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	nodeCount = NODECOUNT_DEFAULT;
	precision = PRECISION_DEFAULT;
	hull = convexHullCreate(pixels, width, height, (convexHullVector){ (float)(width >> 1), (float)(height >> 1) }, nodeCount, precision);
}

#define CROSS_RADIUS 0.05f

void render()
{
	unsigned int i;
	float scalex = 1.0f / (width >> 1);
	float scaley = 1.0f / (height >> 1);

	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texture);

	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);	glVertex2f(-1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);	glVertex2f(1.0f, 1.0f);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINES);
	for(i = 0; i < hull.nodeCount; i++) {
		convexHullVector position = (convexHullVector){ hull.nodes[i].x * scalex, -hull.nodes[i].y * scaley };
		convexHullVector nextPosition = (convexHullVector){ hull.nodes[(i + 1) % hull.nodeCount].x * scalex, -hull.nodes[(i + 1) % hull.nodeCount].y * scaley };

		glColor4f(1, 0, 0, 1);
		glVertex2f(position.x, position.y);
		glVertex2f(nextPosition.x, nextPosition.y);

		glColor4f(1, 1, 1, 0.2f);
		glVertex2f(0, 0);
		glVertex2fv((GLfloat*)&position);
		
		glColor4f(1, 1, 1, 1);
		glVertex2f(position.x - CROSS_RADIUS, position.y);
		glVertex2f(position.x + CROSS_RADIUS, position.y);
		glVertex2f(position.x, position.y - CROSS_RADIUS);
		glVertex2f(position.x, position.y + CROSS_RADIUS);
	}
	glEnd();
}

int run()
{
	// Poll events
	while(ccWindowEventPoll()) {
		ccEvent event = ccWindowEventGet();

		switch(event.type) {
		case CC_EVENT_KEY_DOWN:
			switch(event.keyCode) {
			case CC_KEY_ESCAPE:
				return 0;
				break;
			case CC_KEY_LEFT:
				precision -= 8.0f;
				if(precision < 8.0f) precision = 8.0f;
				hull = convexHullCreate(pixels, width, height, (convexHullVector){ (float)(width >> 1), (float)(height >> 1) }, nodeCount, precision);
				printf("Changed precision to %.2f\n", precision);
				break;
			case CC_KEY_RIGHT:
				precision += 8.0f;
				hull = convexHullCreate(pixels, width, height, (convexHullVector){ (float)(width >> 1), (float)(height >> 1) }, nodeCount, precision);
				printf("Changed precision to %.2f\n", precision);
				break;
			case CC_KEY_UP:
				nodeCount+=8;
				hull = convexHullCreate(pixels, width, height, (convexHullVector){ (float)(width >> 1), (float)(height >> 1) }, nodeCount, precision);
				printf("Changed spacing to %d\n", nodeCount);
				break;
			case CC_KEY_DOWN:
				nodeCount-=8;
				if(nodeCount == 0) nodeCount = 8;
				printf("Changed spacing to %d\n", nodeCount);
				hull = convexHullCreate(pixels, width, height, (convexHullVector){ (float)(width >> 1), (float)(height >> 1) }, nodeCount, precision);
				break;
			}
			break;
		case CC_EVENT_WINDOW_QUIT:
			return 0;
			break;
		}
	}

	// Render
	render();
	ccGLBuffersSwap();

	return 1;
}

int main(int argc, char **argv)
{
	printf(TITLE);

	// Fetch displays prior to window creation
	ccDisplayInitialize();

	// Create window
	ccWindowCreate((ccRect){ 0, 0, 512, 512 }, TITLE, CC_WINDOW_FLAG_NORESIZE);
	ccWindowSetCentered();

	// Bind openGL context
	ccGLContextBind();

	// Configure opengl
	glClearColor(0, 0, 0, 1.0f);

	// Initialize
	loadImage();

	while(run());

	// Free
	convexHullFree(hull);

	return 0;
}