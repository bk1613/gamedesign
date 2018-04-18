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
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <SDL_mixer.h>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
using namespace std;

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	stbi_image_free(image);
	return retTexture;
}


class SheetSprite {
public:
	SheetSprite() {}
	SheetSprite(unsigned int te, float givenU, float givenV, float givenWidth, float givenHeight, float givenSize) : size(givenSize), textureID(te), u(givenU), v(givenV),
		width(givenWidth), height(givenHeight) {}

	void Draw(ShaderProgram *program);

	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

void SheetSprite::Draw(ShaderProgram *program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat textCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size,
	};
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, textCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

}


void DrawText(ShaderProgram *program, int fontTexture, string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;

	vector<float> vertexData;
	vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {

		int spriteIndex = (int)text[i];

		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			});

		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y ,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y ,
			texture_x, texture_y + texture_size,
			});
	}
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}


class Entity {
public:
	Entity() {}
	float pistolpos_x;
	float pistolpos_y;
	float pistolvelocity_y;
	float dir_x = sin(0.785398);
	float dir_y = 1.0f;
	float position_x;
	float position_y;
	float shippositionX = 0.0f;
	float timeAlive = 0.0f;

	SheetSprite sprite;
};

class Gamestate {
public:
	vector<Entity> entities;
	vector<Entity> bullets;
	int numEnemies = 0;
	int score = 0;
	bool gamestop = false;
};


bool BoxBoxCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
	if (y1 + h1 / 2.0f < y2 - h2 / 2.0f || y1 - h1 / 2.0f > y2 + h2 / 2.0f || x1 - w1 / 2.0f > x2 + w2 / 2.0f || x1 + w1 / 2.0f < x2 - w2 / 2.0f) {
		return false;
	}
	else {
		return true;
	}
}

bool shouldREMOVEBullet(Entity bullet) {

	if (bullet.timeAlive > 0.5) {
		return true;
	}
	else {
		return false;
	}
}

Gamestate pistol;
Gamestate enemyentit;
Gamestate state;
Entity playerpos;
Mix_Chunk *laser;
Mix_Chunk *explosion;

void Update(float elapsed) {

	for (int i = 0; i < enemyentit.entities.size(); i++) {

		enemyentit.entities[i].position_x += enemyentit.entities[i].dir_x * 3.0f * elapsed;

		if (!((enemyentit.entities[i].position_x - 0.5) < -3.50)) {
			enemyentit.entities[i].dir_x = -enemyentit.entities[i].dir_x;

		}

		if (!((enemyentit.entities[i].position_x + 0.5) > 3.50)) {
			enemyentit.entities[i].dir_x = -enemyentit.entities[i].dir_x;
		}
	}

	pistol.bullets.erase(remove_if(pistol.bullets.begin(), pistol.bullets.end(), shouldREMOVEBullet), pistol.bullets.end());

	for (int i = 0; i < pistol.bullets.size(); i++)
	{
		pistol.bullets[i].pistolpos_y += pistol.bullets[i].pistolvelocity_y * elapsed;
		Entity backpos;
		for (int j = 0; j < enemyentit.entities.size(); j++) {
			if (BoxBoxCollision(enemyentit.entities[j].position_x, enemyentit.entities[j].position_y, 0.35, 0.35, pistol.bullets[i].pistolpos_x, pistol.bullets[i].pistolpos_y, 0.025 * 2, 0.085 * 2)) {
				pistol.bullets[i].pistolpos_x = 10.0; // backpos.shippositionX;
				enemyentit.entities[j].position_x = 4.76;;
				state.numEnemies--;
				state.score += 50;
				Mix_PlayChannel(-1, explosion, 0);
			}
		}
	}

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_RIGHT]) {
		if (!((playerpos.shippositionX + 0.5) > 3.55)) {
			playerpos.shippositionX += 2.25f * elapsed;
		}

	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		if (!((playerpos.shippositionX - 0.5) < -3.55)) {
			playerpos.shippositionX -= 2.25f * elapsed;
		}
	}


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

	ShaderProgram programuntextured;
	programuntextured.Load(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl");
	Matrix projectionMatrixuntext;
	Matrix modelMatrixuntext;
	Matrix viewMatrixuntext;
	programuntextured.SetModelMatrix(modelMatrixuntext);

	projectionMatrixuntext.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(programuntextured.programID);

	//textured
	ShaderProgram programtextured;
	programtextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint player = LoadTexture(RESOURCE_FOLDER"playerShip1_red.png");
	//GLuint explosion = LoadTexture(RESOURCE_FOLDER"explosion.png");
	GLuint spriteSheetTexture = LoadTexture("sheet.png");
	GLuint text = LoadTexture("font1.png");
	Entity entity;

	for (int i = 0; i < 5; i++) {
		Entity enemy;
		for (int j = 0; j < 10; j++)
		{
			enemy.sprite = SheetSprite(spriteSheetTexture, 423.0f / 1024.0f, 728.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.35);
			enemy.position_x = j * 0.5f - 3.0f;
			enemy.position_y = i * 0.4f - .15f;
			state.numEnemies += 1;
			enemyentit.entities.push_back(enemy);
		}
	}


	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix modelMatrix2;
	Matrix modelMatrixtext;
	Matrix viewMatrix;

	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	enum GameMode { TITLE_SCREEN, GAME };
	GameMode mode = TITLE_SCREEN;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(programtextured.programID);

	float lastFrameTicks = 0.0f;

	bool begingame = false;
	float accumulator = 0.0f;

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	
	laser = Mix_LoadWAV("explode.wav");

	
	explosion = Mix_LoadWAV("pistal.wav");

	Mix_Music *music;
	music = Mix_LoadMUS("Lobo_Loco_-_13_-_Funky_Machine_ID_874.mp3");
	Mix_VolumeMusic(25);
	Mix_PlayMusic(music, -1);

	programtextured.SetProjectionMatrix(projectionMatrix);
	programtextured.SetViewMatrix(viewMatrix);

	programuntextured.SetProjectionMatrix(projectionMatrixuntext);
	programuntextured.SetViewMatrix(viewMatrixuntext);

	//End Setup
	//Running program
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) { // check if window close
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				//Press enter key to begin game
				if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					if (mode == TITLE_SCREEN) {
						mode = GAME;
						lastFrameTicks = (float)SDL_GetTicks() / 1000.0f;
					}


				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					Entity newbullt;

					newbullt.pistolpos_x = playerpos.shippositionX;
					newbullt.pistolpos_y = -1.6;
					newbullt.pistolvelocity_y = 2.05f;
					newbullt.timeAlive = 0.0f;
					pistol.bullets.push_back(newbullt);
					Mix_PlayChannel(-1, laser, 0);
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.4f, 0.3f, 2.25f, 1.0f);

		switch (mode)
		{
		case TITLE_SCREEN:
			modelMatrixtext.Identity();
			modelMatrixtext.Translate(-3.0f, 1.0f, 0.0f);
			programtextured.SetModelMatrix(modelMatrixtext);
			DrawText(&programtextured, text, "Welcome to Space Invaders", ((3.5 / 25.0f) * 2.0f), -0.03f);

			modelMatrixtext.Identity();
			modelMatrixtext.Translate(-1.16f, -1.0f, 0.0f);
			programtextured.SetModelMatrix(modelMatrixtext);
			DrawText(&programtextured, text, "Press Enter", ((3.0 / 25.0f) * 2.0f), -0.05f);
			break;

		case GAME:

			float ticks = (float)SDL_GetTicks() / 1000.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;
			elapsed += accumulator;
			if (elapsed < FIXED_TIMESTEP) {
				accumulator = elapsed;
				continue;
			}

			while (elapsed >= FIXED_TIMESTEP) {
				Update(FIXED_TIMESTEP);
				elapsed -= FIXED_TIMESTEP;
			}

			accumulator = elapsed;
			Entity bullet;
			shouldREMOVEBullet(bullet);

			
			//pistol
			for (int b = 0; b < pistol.bullets.size(); b++) {
				modelMatrixuntext.Identity();
				modelMatrixuntext.Translate(pistol.bullets[b].pistolpos_x, pistol.bullets[b].pistolpos_y, 0.0f);
				programuntextured.SetModelMatrix(modelMatrixuntext);
				programuntextured.SetProjectionMatrix(projectionMatrixuntext);
				programuntextured.SetViewMatrix(viewMatrixuntext);
				float verticespistol[] = { -0.025, -0.085, 0.025, -0.085, 0.025, 0.085, -0.025, -0.085, 0.025, 0.085, -0.025, 0.085 };

				glVertexAttribPointer(programuntextured.positionAttribute, 2, GL_FLOAT, false, 0, verticespistol);
				glEnableVertexAttribArray(programuntextured.positionAttribute);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(programuntextured.positionAttribute);

			}

			//enemy ships
			for (int i = 0; i < enemyentit.entities.size(); i++) {
				modelMatrix2.Identity();
				modelMatrix2.Translate(enemyentit.entities[i].position_x, enemyentit.entities[i].position_y, 0.0f);
				programtextured.SetModelMatrix(modelMatrix2);
				enemyentit.entities[i].sprite.Draw(&programtextured);
			}


			//player ship
			modelMatrix.Identity();
			modelMatrix.Translate(playerpos.shippositionX, -1.65f, 0.0f);

			programtextured.SetModelMatrix(modelMatrix);
			programtextured.SetProjectionMatrix(projectionMatrix);
			programtextured.SetViewMatrix(viewMatrix);


			glBindTexture(GL_TEXTURE_2D, player);


			float verticesship[] = { -0.25, -0.25,
				0.25, -0.25,
				0.25, 0.25,
				-0.25, -0.25,
				0.25, 0.25,
				-0.25, 0.25 };
			glVertexAttribPointer(programtextured.positionAttribute, 2, GL_FLOAT, false, 0, verticesship);
			glEnableVertexAttribArray(programtextured.positionAttribute);

			float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			glVertexAttribPointer(programtextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(programtextured.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(programtextured.positionAttribute);
			glDisableVertexAttribArray(programtextured.texCoordAttribute);


			if (state.numEnemies == 0 || state.gamestop)
			{
				enemyentit.entities.clear();
				mode = TITLE_SCREEN;
			}

			modelMatrixtext.Identity();
			modelMatrixtext.Translate(-2.2f, 1.9f, 0.0f);
			programtextured.SetModelMatrix(modelMatrixtext);
			DrawText(&programtextured, text, to_string(state.score), ((3.5 / 25.0f) * 2.0f), -0.08f);


			break;
		}

		//End Drawing
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

