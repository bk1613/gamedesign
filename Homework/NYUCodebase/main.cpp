#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
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
		std::cout << "Unable to load image. Make Sure the Path is corret0\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return retTexture;
}


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);
	//untexture
	ShaderProgram programuntextured;
	programuntextured.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	
	Matrix projectionMatrixuntext;
	Matrix modelMatrixuntext;
	Matrix viewMatrixuntext;

	modelMatrixuntext.Translate(2.0f, 0.0f, 0.0f);
	programuntextured.SetModelMatrix(modelMatrixuntext);

	projectionMatrixuntext.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(programuntextured.programID);

	//texture
	ShaderProgram programtextured;
	programtextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint boardgameTexture = LoadTexture(RESOURCE_FOLDER"pieceBlack_single15.png");
	GLuint spaceshooterTexture = LoadTexture(RESOURCE_FOLDER"playerShip1_red.png");
	GLuint exitTexture = LoadTexture(RESOURCE_FOLDER"exitW.png");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	modelMatrix.Translate(-2.0f, 0.0f, 0.0f);
	programtextured.SetModelMatrix(modelMatrix);



	Matrix modelMatrix2;

	modelMatrix.Identity();
	modelMatrix2.Translate(0.0f, 1.5f, 0.0f);
	programtextured.SetModelMatrix(modelMatrix2);


	Matrix modelMatrix3;

	modelMatrix.Identity();
	modelMatrix3.Translate(0.0f, -1.5f, 0.0f);
	programtextured.SetModelMatrix(modelMatrix3);

	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	
	glUseProgram(programtextured.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float lastFrameTicks = 1.0f;
	float angle = 0.0;
	//End Setup
	//Running program
	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) { // check if window close
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.25f, 0.6f, 0.4f, 1.0f);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		angle += elapsed;

		//Untextured
		programuntextured.SetModelMatrix(modelMatrixuntext);
		programuntextured.SetProjectionMatrix(projectionMatrixuntext);
		programuntextured.SetViewMatrix(viewMatrixuntext);
		float vertices1[] = { 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f };
		glVertexAttribPointer(programuntextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
		glEnableVertexAttribArray(programuntextured.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(programuntextured.positionAttribute);

		//Textured
		modelMatrix.Identity();
		modelMatrix.Rotate(9.0f * 2.0f);
		modelMatrix.Translate(0.0f, angle, 0.0f);

		programtextured.SetModelMatrix(modelMatrix);
		programtextured.SetProjectionMatrix(projectionMatrix);
		programtextured.SetViewMatrix(viewMatrix);
		glBindTexture(GL_TEXTURE_2D, spaceshooterTexture);

		float vertices2[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		glVertexAttribPointer(programtextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(programtextured.positionAttribute);

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(programtextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(programtextured.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		//textured2
		glBindTexture(GL_TEXTURE_2D, boardgameTexture);

		
		programtextured.SetModelMatrix(modelMatrix2);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		//textured3
		glBindTexture(GL_TEXTURE_2D, exitTexture);

		programtextured.SetModelMatrix(modelMatrix3);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(programtextured.positionAttribute);
		glDisableVertexAttribArray(programtextured.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}