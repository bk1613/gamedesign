#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
//STB_image loads images
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);// | SDL_INIT_JOYSTICK);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	SDL_Event event;
	bool done = false;
	glViewport(0, 0, 640, 360);
	//untextures
	ShaderProgram programuntextured;
	programuntextured.Load(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl");
	Matrix projectionMatrixuntext;
	Matrix modelMatrixuntext;
	Matrix viewMatrixuntext;

	modelMatrixuntext.Translate(0.0f, 0.0f, 0.0f);
	modelMatrixuntext.Scale(0.15f, 0.15f, 0.0f);
	programuntextured.SetModelMatrix(modelMatrixuntext);

	projectionMatrixuntext.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(programuntextured.programID);

	//textured
	ShaderProgram programtextured;
	programtextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint bar1 = LoadTexture(RESOURCE_FOLDER"pieceBlack_border02.png");
	GLuint bar2 = LoadTexture(RESOURCE_FOLDER"pieceBlack_border04.png");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	Matrix modelMatrix2;
	
	Matrix modelMatrix3;
	
	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(programtextured.programID);

	float lastFrameTicks = 0.0f;
	float velocity = 2.25f;

	float paddelposition1Y = 0.0f;
	float paddelposition2Y = 0.0f;

	
	float posballx = 0.0f;
	float posbally = 0.0f;
	float dir_x = sin(0.785398);
	float dir_y = cos(0.785398);
	float ballspeed = 3.0f;

	bool begingame = false;

	//End Setup
	//Running program
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) { // check if window close
				done = true;
			}else if (event.type == SDL_KEYDOWN) {
			 //Press enter key to begin game
			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
			    begingame = !begingame;
				lastFrameTicks = (float)SDL_GetTicks() / 1000.0f;
			}
		}
			
	}
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.4f, 0.3f, 0.25f, 1.0f);

		//Drawing
		if (begingame) {
			float ticks = (float)SDL_GetTicks() / 1000.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;
			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			//right paddel
			if (keys[SDL_SCANCODE_UP]) {
				if (!((paddelposition1Y + 0.5) > 2.0)) {
					paddelposition1Y += velocity * elapsed;
				}
				
			}
			else if (keys[SDL_SCANCODE_DOWN]) {
				if (!((paddelposition1Y - 0.5) < -2.0)) {
					paddelposition1Y -= velocity * elapsed;
				}
			}
			//left paddel
			if (keys[SDL_SCANCODE_W]) {
				if (!((paddelposition2Y + 0.5) > 2.0)) {
					paddelposition2Y += velocity * elapsed;
				}
			}
			else if (keys[SDL_SCANCODE_S]) {
				if (!((paddelposition2Y - 0.5) < -2.0)) {
					paddelposition2Y -= velocity * elapsed;
				}
			}
			posballx += dir_x * ballspeed * elapsed;
			posbally += dir_y * ballspeed * elapsed;

			//collisions
			if (((posballx + 0.075) > 3.55)) {
				posballx = 0;
				posbally = 0;
			}

			if (((posballx - 0.075) < -3.55)) {
				posballx = 0;
				posbally = 0;
			}
			//right collsion
			if (!((posballx - 0.075) > -3.05) &&
				!((posballx + 0.075) < -3.15) &&
				!((posbally + 0.075) < paddelposition2Y - .4) &&
				!((posbally - 0.075) > paddelposition2Y + .4)) {

				dir_x = -dir_x;

				
			}
			//left collision
			if (!((posballx + 0.075) < 3.05) &&
				!((posballx - 0.075) > 3.15) &&
				!((posbally + 0.075) < paddelposition1Y - .4) &&
				!((posbally - 0.075) > paddelposition1Y + .4)) {

				dir_x = -dir_x;

				
			}
			//top wall
			if ((posbally + 0.075) >= 2.0) {
				dir_y = -dir_y;
				posbally -= .01;
			}
			//bottom wall
			if ((posbally - 0.075) <= -2.0) {
				dir_y = -dir_y;
				posbally += .01;
			}
			
		}
		

		//ball
		modelMatrixuntext.Identity();
		modelMatrixuntext.Translate(posballx, posbally, 0.0f);
		programuntextured.SetModelMatrix(modelMatrixuntext);
		programuntextured.SetProjectionMatrix(projectionMatrixuntext);
		programuntextured.SetViewMatrix(viewMatrixuntext);
		float vertices[] = { -0.075, -0.075, 0.075, -0.075, 0.075, 0.075, -0.075, -0.075, 0.075, 0.075, -0.075, 0.075 };

		glVertexAttribPointer(programuntextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(programuntextured.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(programuntextured.positionAttribute);

		//left bar
		modelMatrix2.Identity();
		
		modelMatrix2.Translate(2.75f, paddelposition1Y, 0.0f);

		programtextured.SetModelMatrix(modelMatrix3);
		programtextured.SetProjectionMatrix(projectionMatrix);
		programtextured.SetViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, bar1);
		
		float verticesbar[] = { -0.6f, -0.4f, -0.2f, -0.4f, -0.2f, 0.4f, -0.6f, -0.4f, -0.2f, 0.4f, -0.6f, 0.4f };
		glVertexAttribPointer(programtextured.positionAttribute, 2, GL_FLOAT, false, 0, verticesbar);
		glEnableVertexAttribArray(programtextured.positionAttribute);

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(programtextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(programtextured.texCoordAttribute);
		
		glDrawArrays(GL_TRIANGLES, 0, 6);

		

		
		//right bar
		float verticesbar2[] = { 0.2f, -0.4f, 0.7f, -0.4f, 0.7f, 0.4f, 0.2f, -0.4f, 0.7f, 0.4f, 0.2f, 0.4f };
		glVertexAttribPointer(programtextured.positionAttribute, 2, GL_FLOAT, false, 0, verticesbar2);
		glBindTexture(GL_TEXTURE_2D, bar2);
		modelMatrix3.Identity();
		
		modelMatrix3.Translate(-2.75f, paddelposition2Y, 0.0f);
		programtextured.SetModelMatrix(modelMatrix2);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(programtextured.positionAttribute);
		glDisableVertexAttribArray(programtextured.texCoordAttribute);
		//End Drawing
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

