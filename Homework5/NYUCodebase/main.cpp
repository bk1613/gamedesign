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
#include "FlareMap.h"
#include "SatCollision.h"
#include "Vector.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define TILE_SIZE 0.5f
#define LEVEL_HEIGHT 40
#define LEVEL_WIDTH 128
#else

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

void DrawSpriteSheetSprite(ShaderProgram &program, int index, int spriteCountX, int spriteCountY) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	float texCoords[] = { u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight };
	float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

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
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
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

enum EntityType { ENTITY_PLAYER, ENTITY_COIN };

class Entity {
public:
	Entity() {}
	float coinpos_x;
	float coinpos_y;
	float dir_x = sin(0.785398);
	float dir_y = 1.0f;
	float friction_x = 0.5f;
	float friction_y = 0.5f;
	float pistolpos_x;
	float pistolpos_y;
	float position_x;
	float position_y;
	float velocity_x = 0.0f;
	float velocity_y = 0.0f;
	float astpos_x;
	float astpos_y;
	float astvel_x = 0.0f;
	float astvel_y = 0.0f;
	float acceleration_x;
	float acceleration_y;
	float gravity_x = 0.0f;
	float gravity_y = -2.0f;

	vector<Vector> points;
	Matrix modelMatrix;
	Matrix modelMatrixguns;
	Matrix modelMatrixguns2;
	Matrix mapmodelMatrix;
	Matrix modelMatrixtext;
	Matrix modelMatrixasteriod;
	EntityType entityType;
	SheetSprite sprite;
};
float lerp(float v0, float v1, float t) { return (1.0 - t)*v0 + t * v1; }

class Gamestate {
public:
	vector<Entity> bullets;
	bool gamestop = false;
};

Entity coins;
Entity playerpos;
Entity metoer;
vector<Entity> coin;



Gamestate state;
FlareMap map;

void Update(float elapsed) {


	metoer.astpos_x = metoer.astvel_x * metoer.dir_x * elapsed;
	metoer.astpos_y = metoer.astvel_y * metoer.dir_y * elapsed;

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	playerpos.acceleration_x = 0.0f;
	playerpos.acceleration_x = 0.0f;

	if (keys[SDL_SCANCODE_RIGHT]) {
		playerpos.acceleration_x = 1.5f;
	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		playerpos.acceleration_x = -1.5f;
	}
	else if (keys[SDL_SCANCODE_UP]) {
		//playerpos.velocity_y = 2.0f;
		playerpos.acceleration_x = 1.5f;
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		//playerpos.velocity_y = -2.0f;
		playerpos.acceleration_x = -1.5f;
	}

	playerpos.velocity_x = lerp(playerpos.velocity_x, 0.0f, elapsed * playerpos.friction_x);
	playerpos.velocity_y = lerp(playerpos.velocity_y, 0.0f, elapsed * playerpos.friction_y);

	//playerpos.velocity_x += playerpos.gravity_x * elapsed;
	//playerpos.velocity_y += playerpos.gravity_y * elapsed;

	playerpos.velocity_x += playerpos.acceleration_x * elapsed;
	playerpos.velocity_y += playerpos.acceleration_y * elapsed;


}

Gamestate pistol;

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


	vector<float> vertexData;
	vector<float> texCoordData;
	int SPRITE_COUNT_X = 16;
	int SPRITE_COUNT_Y = 8;


	//textured
	ShaderProgram programtextured;
	programtextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint spritespaceshooter = LoadTexture("sheet.png");
	GLuint asteriod = LoadTexture("meteorBrown_big3.png");
	GLuint text = LoadTexture("font1.png");
	Entity entity;
	Entity entityparts1;
	Entity entityparts2;
	Entity entitymeteor;
	Entity entitymeteor2;
	Entity enntityMatrix;

	entity.sprite = SheetSprite(spritespaceshooter, 224.0f / 1024.0f, 832.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.25);
	entityparts1.sprite = SheetSprite(spritespaceshooter, 809.0f / 1024.0f, 796.0f / 1024.0f, 20.0f / 1024.0f, 41.0f / 1024.0f, 0.25);
	entityparts2.sprite = SheetSprite(spritespaceshooter, 809.0f / 1024.0f, 611.0f / 1024.0f, 20.0f / 1024.0f, 52.0f / 1024.0f, 0.25);
	entitymeteor.sprite = SheetSprite(spritespaceshooter, 224.0f / 1024.0f, 664.0f / 1024.0f, 101.0f / 1024.0f, 84.0f / 1024.0f, 0.25);
	entitymeteor2.sprite = SheetSprite(spritespaceshooter, 651.0f / 1024.0f, 447.0f / 1024.0f, 43.0f / 1024.0f, 43.0f / 1024.0f, 0.25);
	Matrix projectionMatrix;
	Matrix viewMatrix;

	/*entity.modelMatrix.Translate(0.0f, 0.0f, 0.0f);
	entity.modelMatrix.Scale(0.15f, 0.15f, 0.0f);
	programtextured.SetModelMatrix(entity.modelMatrix);

	entitymeteor.modelMatrixasteriod.Translate(0.0f, 0.0f, 0.0f);
	entitymeteor.modelMatrixasteriod.Scale(0.15f, 0.15f, 0.0f);
	programtextured.SetModelMatrix(entitymeteor.modelMatrixasteriod);

	entitymeteor2.modelMatrixasteriod.Translate(0.0f, 0.0f, 0.0f);
	entitymeteor2.modelMatrixasteriod.Scale(0.15f, 0.15f, 0.0f);
	programtextured.SetModelMatrix(entitymeteor2.modelMatrixasteriod);*/

	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	enum GameMode { TITLE_SCREEN, GAME };
	GameMode mode = TITLE_SCREEN;

	Vector v;
	Vector v2;
	Vector v3;
	Vector v4;

	//meteor1
	v.x = -0.5f *.25 * ((101.0f / 1024.0f)/(84.0f / 1024.0f));
	v.y = -0.5f * .25;
	v.z = 0;
	entitymeteor.points.push_back(v);

	v2.x = 0.5f *.25 * ((101.0f / 1024.0f) / (84.0f / 1024.0f));
	v2.y = 0.5f * .25;
	v2.z = 0;
	entitymeteor.points.push_back(v2);
	
	v3.x = -0.5f *.25 * ((101.0f / 1024.0f) / (84.0f / 1024.0f));
	v3.y = 0.5f * .25;
	v4.z = 0;
	entitymeteor.points.push_back(v3);

	v4.x = 0.5f *.25 * ((101.0f / 1024.0f) / (84.0f / 1024.0f));
	v4.y = -0.5f * .25;
	v4.z = 0;
	entitymeteor.points.push_back(v4);

	//meteor2
	v.x = -0.5f *.25 * ((43.0f / 1024.0f) / (43.0f / 1024.0f));
	v.y = -0.5f * .25;
	v.z = 0;
	entitymeteor2.points.push_back(v);

	v2.x = 0.5f *.25 * ((43.0f / 1024.0f) / (43.0f / 1024.0f));
	v2.y = 0.5f * .25;
	v2.z = 0;
	entitymeteor2.points.push_back(v2);

	v3.x = -0.5f *.25 * ((43.0f / 1024.0f) / (43.0f / 1024.0f));
	v3.y = 0.5f * .25;
	v4.z = 0;
	entitymeteor2.points.push_back(v3);
	
	v4.x = 0.5f *.25 * ((43.0f / 1024.0f) / (43.0f / 1024.0f));
	v4.y = -0.5f * .25;
	v4.z = 0;
	entitymeteor2.points.push_back(v4);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(programtextured.programID);

	float lastFrameTicks = 0.0f;

	bool begingame = false;
	float accumulator = 0.0f;

	const int runAnimation[] = { 1, 5, 9, 13 };
	const int runAnimation2[] = { 3, 7, 11, 15 };
	const int numFrames = 5;
	float animationElapsed = 0.0f;
	float framesPerSecond = 30.0f;
	int currentIndex = 0;
	int TOTAL_TITLES = LEVEL_HEIGHT * LEVEL_WIDTH;


	unsigned int levelData[LEVEL_HEIGHT][LEVEL_WIDTH]{};

	programtextured.SetProjectionMatrix(projectionMatrix);
	programtextured.SetViewMatrix(viewMatrix);


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

					}
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.4f, 0.3f, 2.25f, 1.0f);

		switch (mode)
		{
		case TITLE_SCREEN:
			entity.modelMatrixtext.Identity();
			entity.modelMatrixtext.Translate(-3.0f, 1.0f, 0.0f);
			programtextured.SetModelMatrix(entity.modelMatrixtext);
			DrawText(&programtextured, text, "Welcome to Adventure World", ((3.5 / 25.0f) * 2.0f), -0.03f);

			entity.modelMatrixtext.Identity();
			entity.modelMatrixtext.Translate(-1.16f, -1.0f, 0.0f);
			programtextured.SetModelMatrix(entity.modelMatrixtext);
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

			/*animationElapsed += elapsed;
			if (animationElapsed > 1.0 / framesPerSecond) {
			currentIndex++;
			animationElapsed = 0.0;
			if (currentIndex > numFrames - 1) {
			currentIndex = 0;
			}
			}*/

			std::pair<float, float> penetration;

			vector<std::pair<float, float>> e1Points;
			vector<std::pair<float, float>> e2Points;

			for (int i = 0; i < entitymeteor.points.size(); i++) {
				Vector point =  entitymeteor.modelMatrixasteriod * entitymeteor.points[i];
				e1Points.push_back(std::make_pair(point.x, point.y));
			}

			for (int i = 0; i < entitymeteor2.points.size(); i++) {
				Vector point = entitymeteor2.modelMatrixasteriod * entitymeteor2.points[i];
				e2Points.push_back(std::make_pair(point.x, point.y));
			}

			bool collided = CheckSATCollision(e1Points, e2Points, penetration);
			//metoers

			entitymeteor.modelMatrixasteriod.Identity();
			entitymeteor.modelMatrixasteriod.Translate(metoer.astpos_x, metoer.astpos_y, 0.0f);
			programtextured.SetModelMatrix(entitymeteor.modelMatrixasteriod);
			programtextured.SetProjectionMatrix(projectionMatrix);
			entitymeteor.sprite.Draw(&programtextured);

			entitymeteor2.modelMatrixasteriod.Identity();
			entitymeteor2.modelMatrixasteriod.Translate(metoer.astpos_x, metoer.astpos_y, 0.0f);
			programtextured.SetModelMatrix(entitymeteor2.modelMatrixasteriod);
			programtextured.SetProjectionMatrix(projectionMatrix);
			entitymeteor2.sprite.Draw(&programtextured);

			//player

			viewMatrix.Identity();
			viewMatrix.Translate(-playerpos.position_x, -playerpos.position_y, 0.0f);
			programtextured.SetViewMatrix(viewMatrix);
			enntityMatrix.modelMatrix.Identity();
			enntityMatrix.modelMatrix.Translate(playerpos.position_x, playerpos.position_y, 0.0f);
			

			enntityMatrix.modelMatrixguns.Identity();
			enntityMatrix.modelMatrixguns.Translate(playerpos.position_x+.1, playerpos.position_y+.1, 0.0f);
			

			enntityMatrix.modelMatrixguns2.Identity();
			enntityMatrix.modelMatrixguns2.Translate(playerpos.position_x-.1, playerpos.position_y+.1, 0.0f);
	

			programtextured.SetModelMatrix( enntityMatrix.modelMatrixguns*enntityMatrix.modelMatrix);
			programtextured.SetModelMatrix( enntityMatrix.modelMatrixguns2*enntityMatrix.modelMatrix);
			programtextured.SetProjectionMatrix(projectionMatrix);
			entity.sprite.Draw(&programtextured);
			entityparts1.sprite.Draw(&programtextured);
			entityparts2.sprite.Draw(&programtextured);
			//glBindTexture(GL_TEXTURE_2D, player);
			//DrawSpriteSheetSprite(programtextured, 3, 4, 4);

			break;
		}

		//End Drawing
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}