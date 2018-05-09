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
#include <random>
#include <time.h>
#include "FlareMap.h"

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
void DrawSpriteSheetSpritedragon(ShaderProgram &program, int index, int spriteCountX, int spriteCountY) {
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
	float vertices[] = { -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f };

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
	float friction_x = 0.5f;
	float friction_y = 0.5f;
	float position_x;
	float position_y;
	float velocity_x = 0.0f;
	float velocity_y = 0.0f;
	float acceleration_x;
	float acceleration_y;
	float timeAlive = 0.0f;
	bool isStatic;

	EntityType entityType;
	SheetSprite sprite;
};
float lerp(float v0, float v1, float t) { return (1.0 - t)*v0 + t * v1; }

class Gamestate {
public:
	int score = 0;
	bool gamestop = false;
};

Entity coins;
Entity enemy;
Entity dragon;
Entity dragon2;
Entity wizard;
Entity solidar;
Entity forestenemy;
Entity knight;
Entity playerpos;
vector<Entity> coin;
vector<Entity> enemys;
vector<Entity> bullets;
vector<Entity> dragons;
vector<Entity> dragon2s;
vector<Entity> soliders;
vector<Entity> forests;
vector<Entity> knights;
vector<Entity> wizards;

void PlaceEntity(std::string type, float x, float y) {
	// place your entity at x, y based on type string
	if (type == "player") {
		playerpos.position_x = x;
		playerpos.position_y = y;
	}
	else if (type == "coin") {

		coins.position_x = x;
		coins.position_y = y;
		coin.push_back(coins);
	}
	else if (type == "enemy") {
		enemy.position_x = x;
		enemy.position_y = y;
		enemys.push_back(enemy);
	}
	else if (type == "dragon") {
		dragon.position_x = x;
		dragon.position_y = y;
		dragons.push_back(dragon);
	}
	else if (type == "dragon2") {
		dragon2.position_x = x;
		dragon2.position_y = y;
		dragon2s.push_back(dragon2);
	}

	else if (type == "solidar") {
		solidar.position_x = x;
		solidar.position_y = y;
		soliders.push_back(solidar);
	}
	else if (type == "wizard") {
		wizard.position_x = x;
		wizard.position_y = y;
		wizards.push_back(wizard);
	}
	else if (type == "knight") {
		knight.position_x = x;
		knight.position_y = y;
		knights.push_back(knight);
	}
	else if (type == "forest") {
		forestenemy.position_x = x;
		forestenemy.position_y = y;
		forests.push_back(forestenemy);
	}
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}

bool BoxBoxCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
	if (y1 + h1 / 2.0f < y2 - h2 / 2.0f || y1 - h1 / 2.0f > y2 + h2 / 2.0f || x1 - w1 / 2.0f > x2 + w2 / 2.0f || x1 + w1 / 2.0f < x2 - w2 / 2.0f) {
		return false;
	}
	else {
		return true;
	}

}

bool issolid(int tileindex) {
	return tileindex != 0 && tileindex != 170;
}

bool isexit(int tileindex) {
	return tileindex == 170;
}

bool shouldREMOVEBullet(Entity bullet) {

	if (bullet.timeAlive > 0.5) {
		return true;
	}
	else {
		return false;
	}
}

float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
	float retVal = dstMin + ((value - srcMin) / (srcMax - srcMin) * (dstMax - dstMin));
	if (retVal < dstMin) {
		retVal = dstMin;
	}
	if (retVal > dstMax) {
		retVal = dstMax;
	}
	return retVal;
}

float easeOut(float from, float to, float time) {
	float oneMinusT = 1.0f - time;
	float tVal = 1.0f - (oneMinusT * oneMinusT * oneMinusT * oneMinusT * oneMinusT);
	return (1.0f - tVal)*from + tVal * to;
}
class Particle { 
public: 
	Particle() {
		position_x = 0;
		position_y = 0;
		velocity_x = 0;
		velocity_y = 0;
		lifetime = 0;
	}
	float position_x; 
	float position_y;
	float velocity_y; 
	float velocity_x;
	float lifetime; 
};
bool particleDrawn = false;
class ParticleEmitter { 
public:         
	ParticleEmitter(unsigned int particleCount) : numparticle(particleCount){

		for (int i = 0; i < particleCount; ++i) {
			Particle exp;
			exp.position_x = 0;
			exp.position_y = 0;
			exp.velocity_x = 0;
			exp.velocity_y = 0;
			exp.lifetime = 0;
			maxLifetime = 5.5f;
			particles.push_back(exp);
		}
	}
	void render(ShaderProgram *programuntextured) {
		vector<float> vertices;
		for (int i = 0; i < particles.size(); i++)
		{
			//if (particles[i].lifetime < maxLifetime) {
				vertices.push_back(particles[i].position_x);
				vertices.push_back(particles[i].position_y);

			glVertexAttribPointer(programuntextured->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
			glEnableVertexAttribArray(programuntextured->positionAttribute);
			glDrawArrays(GL_POINTS, 0, particles.size());
		}
		

	}
	void update(float elapsed) {
		for (int i = 0; i < numparticle; ++i) {
			if (particleDrawn) {
				particles[i].lifetime += elapsed;
			}
			particles[i].velocity_x += gravity_x * elapsed;
			particles[i].velocity_y += gravity_y * elapsed;
			particles[i].position_x += particles[i].velocity_x * elapsed;
			particles[i].position_y += particles[i].velocity_y * elapsed;

		}

	}
	void reset() {
		for (int i = 0; i < numparticle; ++i) {
			particles[i].position_x = 0;
			particles[i].position_y = 0;
			particles[i].velocity_x = -1.0 + ((float)rand()/(float)RAND_MAX * 2.0);
			particles[i].velocity_y = (float)rand() / (float)RAND_MAX;
			particles[i].lifetime = 0;
		}
	}

	unsigned int numparticle;
	float position_x;
	float position_y;
	float velocity_x;
	float velocity_y;
	float gravity_x; 
	float gravity_y;
	float maxLifetime;              
	vector<Particle> particles; 
};

Gamestate state;
FlareMap *map = NULL;
Mix_Chunk *laser;
Mix_Chunk *explosion;
Mix_Chunk *coinscol;
Matrix projectionMatrix;
Matrix modelMatrix;
Matrix mapmodelMatrix;
Matrix modelMatrixtext;
Matrix modelMatrixuntext;
Matrix viewMatrix;
ParticleEmitter part(12);
ShaderProgram programuntextured;
int currentMap = 0;
int playerdirection;
float rad;
enum GameMode { TITLE_SCREEN, GAME, GAME_OVER };
GameMode mode = TITLE_SCREEN;


void LoadNewMap(const std::string &fileName) {

	delete map;

	map = new FlareMap();

	map->Load(fileName);
	coin.clear();
	enemys.clear();
	bullets.clear();
	
	soliders.clear();
	forests.clear();
	knights.clear();
	wizards.clear();

	dragons.clear();
	for (int i = 0; i < map->entities.size(); i++) {

		PlaceEntity(map->entities[i].type, map->entities[i].x * TILE_SIZE, map->entities[i].y * -TILE_SIZE);

	}
	

}

Matrix projectionMatrixuntext;
void Update(float elapsed) {
	
	part.update(elapsed);
	
	
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	playerpos.acceleration_x = 0.0f;
	playerpos.acceleration_x = 0.0f;

	

	if (keys[SDL_SCANCODE_RIGHT]) {
		playerpos.acceleration_x = 1.5f;
		playerdirection = 3;

	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		playerpos.acceleration_x = -1.5f;
		playerdirection = 1;
	}
	else if (keys[SDL_SCANCODE_UP]) {
		playerpos.velocity_y = 1.5f;
		playerdirection = 2;
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		playerpos.velocity_y = -1.5f;
		playerdirection = 0;
	}

	playerpos.velocity_x = lerp(playerpos.velocity_x, 0.0f, elapsed * playerpos.friction_x);
	playerpos.velocity_y = lerp(playerpos.velocity_y, 0.0f, elapsed * playerpos.friction_y);

	playerpos.velocity_x += playerpos.acceleration_x * elapsed;
	playerpos.velocity_y += playerpos.acceleration_y * elapsed;

	
	for (int i = 0; i < enemys.size(); i++) {
		rad = sqrt(powf(enemys[i].position_x - playerpos.position_x, 2) + powf(enemys[i].position_y - playerpos.position_y, 2));
		enemys[i].acceleration_x = 0.0f;
		enemys[i].acceleration_y = 0.0f;
	
		if (rad < 4.0) {
			enemys[i].acceleration_x = (playerpos.position_x - enemys[i].position_x)/rad * .5;
			enemys[i].acceleration_y = (playerpos.position_y - enemys[i].position_y)/rad * .5;
		}
		enemys[i].velocity_x = lerp(enemys[i].velocity_x, 0.0f, elapsed * enemys[i].friction_x);
		enemys[i].velocity_y = lerp(enemys[i].velocity_y, 0.0f, elapsed * enemys[i].friction_y);

		enemys[i].velocity_x += enemys[i].acceleration_x * elapsed;
		enemys[i].velocity_y += enemys[i].acceleration_y * elapsed;

		enemys[i].position_x += enemys[i].velocity_x * elapsed;
		enemys[i].position_y += enemys[i].velocity_y * elapsed;

	}
	for (int i = 0; i < soliders.size(); i++) {
		rad = sqrt(powf(soliders[i].position_x - playerpos.position_x, 2) + powf(soliders[i].position_y - playerpos.position_y, 2));
		soliders[i].acceleration_x = 0.0f;
		soliders[i].acceleration_y = 0.0f;

		if (rad < 4.0) {
			soliders[i].acceleration_x = (playerpos.position_x - soliders[i].position_x) / rad * .5;
			soliders[i].acceleration_y = (playerpos.position_y - soliders[i].position_y) / rad * .5;
		}
		soliders[i].velocity_x = lerp(soliders[i].velocity_x, 0.0f, elapsed * soliders[i].friction_x);
		soliders[i].velocity_y = lerp(soliders[i].velocity_y, 0.0f, elapsed * soliders[i].friction_y);

		soliders[i].velocity_x += soliders[i].acceleration_x * elapsed;
		soliders[i].velocity_y += soliders[i].acceleration_y * elapsed;

		soliders[i].position_x += soliders[i].velocity_x * elapsed;
		soliders[i].position_y += soliders[i].velocity_y * elapsed;

	}
	for (int i = 0; i < knights.size(); i++) {
		rad = sqrt(powf(knights[i].position_x - playerpos.position_x, 2) + powf(knights[i].position_y - playerpos.position_y, 2));
		knights[i].acceleration_x = 0.0f;
		knights[i].acceleration_y = 0.0f;

		if (rad < 4.0) {
			knights[i].acceleration_x = (playerpos.position_x - knights[i].position_x) / rad * .5;
			knights[i].acceleration_y = (playerpos.position_y - knights[i].position_y) / rad * .5;
		}
		knights[i].velocity_x = lerp(knights[i].velocity_x, 0.0f, elapsed * knights[i].friction_x);
		knights[i].velocity_y = lerp(knights[i].velocity_y, 0.0f, elapsed * knights[i].friction_y);

		knights[i].velocity_x += knights[i].acceleration_x * elapsed;
		knights[i].velocity_y += knights[i].acceleration_y * elapsed;

		knights[i].position_x += knights[i].velocity_x * elapsed;
		knights[i].position_y += knights[i].velocity_y * elapsed;

	}

	for (int i = 0; i < forests.size(); i++) {
		rad = sqrt(powf(forests[i].position_x - playerpos.position_x, 2) + powf(forests[i].position_y - playerpos.position_y, 2));
		forests[i].acceleration_x = 0.0f;
		forests[i].acceleration_y = 0.0f;

		if (rad < 4.0) {
			forests[i].acceleration_x = (playerpos.position_x - forests[i].position_x) / rad * .5;
			forests[i].acceleration_y = (playerpos.position_y - forests[i].position_y) / rad * .5;
		}
		forests[i].velocity_x = lerp(forests[i].velocity_x, 0.0f, elapsed * forests[i].friction_x);
		forests[i].velocity_y = lerp(forests[i].velocity_y, 0.0f, elapsed * forests[i].friction_y);

		forests[i].velocity_x += forests[i].acceleration_x * elapsed;
		forests[i].velocity_y += forests[i].acceleration_y * elapsed;

		forests[i].position_x += forests[i].velocity_x * elapsed;
		forests[i].position_y += forests[i].velocity_y * elapsed;

	}

	for (int i = 0; i < wizards.size(); i++) {
		rad = sqrt(powf(wizards[i].position_x - playerpos.position_x, 2) + powf(wizards[i].position_y - playerpos.position_y, 2));
		wizards[i].acceleration_x = 0.0f;
		wizards[i].acceleration_y = 0.0f;

		if (rad < 4.0) {
			wizards[i].acceleration_x = (playerpos.position_x - wizards[i].position_x) / rad * .5;
			wizards[i].acceleration_y = (playerpos.position_y - wizards[i].position_y) / rad * .5;
		}
		wizards[i].velocity_x = lerp(wizards[i].velocity_x, 0.0f, elapsed * wizards[i].friction_x);
		wizards[i].velocity_y = lerp(wizards[i].velocity_y, 0.0f, elapsed * wizards[i].friction_y);

		wizards[i].velocity_x += wizards[i].acceleration_x * elapsed;
		wizards[i].velocity_y += wizards[i].acceleration_y * elapsed;

		wizards[i].position_x += wizards[i].velocity_x * elapsed;
		wizards[i].position_y += wizards[i].velocity_y * elapsed;

	}



	for (int i = 0; i < dragons.size(); i++) {
		rad = sqrt(powf(dragons[i].position_x - playerpos.position_x, 2) + powf(dragons[i].position_y - playerpos.position_y, 2));
		dragons[i].acceleration_x = 0.0f;
		dragons[i].acceleration_y = 0.0f;

		if (rad < 3.5) {
			dragons[i].acceleration_x = (playerpos.position_x - dragons[i].position_x) / rad;
			dragons[i].acceleration_y = (playerpos.position_y - dragons[i].position_y) / rad;
		}
		dragons[i].velocity_x = lerp(dragons[i].velocity_x, 0.0f, elapsed * dragons[i].friction_x);
		dragons[i].velocity_y = lerp(dragons[i].velocity_y, 0.0f, elapsed * dragons[i].friction_y);

		dragons[i].velocity_x += dragons[i].acceleration_x * elapsed;
		dragons[i].velocity_y += dragons[i].acceleration_y * elapsed;

		dragons[i].position_x += dragons[i].velocity_x * elapsed;
		dragons[i].position_y += dragons[i].velocity_y * elapsed;

	}

	for (int i = 0; i < dragon2s.size(); i++) {
		rad = sqrt(powf(dragon2s[i].position_x - playerpos.position_x, 2) + powf(dragon2s[i].position_y - playerpos.position_y, 2));
		dragon2s[i].acceleration_x = 0.0f;
		dragon2s[i].acceleration_y = 0.0f;

		if (rad < 3.5) {
			dragon2s[i].acceleration_x = (playerpos.position_x - dragon2s[i].position_x) / rad;
			dragon2s[i].acceleration_y = (playerpos.position_y - dragon2s[i].position_y) / rad;
		}
		dragon2s[i].velocity_x = lerp(dragon2s[i].velocity_x, 0.0f, elapsed * dragon2s[i].friction_x);
		dragon2s[i].velocity_y = lerp(dragon2s[i].velocity_y, 0.0f, elapsed * dragon2s[i].friction_y);

		dragon2s[i].velocity_x += dragon2s[i].acceleration_x * elapsed;
		dragon2s[i].velocity_y += dragon2s[i].acceleration_y * elapsed;

		dragon2s[i].position_x += dragon2s[i].velocity_x * elapsed;
		dragon2s[i].position_y += dragon2s[i].velocity_y * elapsed;

	}

	for (int i = 0; i < coin.size(); i++) {
		if (BoxBoxCollision(playerpos.position_x, playerpos.position_y, 0.5, 0.5, coin[i].position_x, coin[i].position_y, 0.5, 0.5)) {
			coin[i].position_x = 1000.0f;
			coin[i].position_y = 1000.0f;
			state.score += 100;
			Mix_PlayChannel(-1, coinscol, 0);
		}
	}

	bullets.erase(remove_if(bullets.begin(), bullets.end(), shouldREMOVEBullet), bullets.end());

	for (int i = 0; i < bullets.size(); i++)
	{
		bullets[i].position_x += bullets[i].velocity_x * elapsed;
		bullets[i].position_y += bullets[i].velocity_y * elapsed;

		Entity backpos;
		for (int j = 0; j < enemys.size(); j++) {
			if (BoxBoxCollision(enemys[j].position_x, enemys[j].position_y, 0.5, 0.5, bullets[i].position_x, bullets[i].position_y, 0.025 * 2, 0.085 * 2)) {
				bullets[i].position_x = 10.0; // backpos.shippositionX;
				bullets[i].position_y = 10.0;
				part.position_x = enemys[j].position_x;
				part.position_y = enemys[j].position_y;
				//adjustparticlepostion
				part.gravity_x = 0;
				part.gravity_y = -2.0f;
				part.velocity_x = 2.0f;
				part.velocity_y = 2.5f;
				//render particaleemitter
				enemys[j].position_x = 4.76;
				enemys[j].position_y = 4.76;
				Mix_PlayChannel(-1, explosion, 0);
				part.reset();
			}
			
		}
		for (int j = 0; j < soliders.size(); j++) {
			if (BoxBoxCollision(soliders[j].position_x, soliders[j].position_y, 0.5, 0.5, bullets[i].position_x, bullets[i].position_y, 0.025 * 2, 0.085 * 2)) {
				bullets[i].position_x = 10.0; // backpos.shippositionX;
				bullets[i].position_y = 10.0;
				part.position_x = soliders[j].position_x;
				part.position_y = soliders[j].position_y;
				//adjustparticlepostion
				part.gravity_x = 0;
				part.gravity_y = -2.0f;
				part.velocity_x = 2.0f;
				part.velocity_y = 2.5f;
				//render particaleemitter
				soliders[j].position_x = 4.76;
				soliders[j].position_y = 4.76;
				Mix_PlayChannel(-1, explosion, 0);
				part.reset();
			}

		}
		for (int j = 0; j < wizards.size(); j++) {
			if (BoxBoxCollision(wizards[j].position_x, wizards[j].position_y, 0.5, 0.5, bullets[i].position_x, bullets[i].position_y, 0.025 * 2, 0.085 * 2)) {
				bullets[i].position_x = 10.0; // backpos.shippositionX;
				bullets[i].position_y = 10.0;
				
				part.position_x = wizards[j].position_x;
				part.position_y = wizards[j].position_y;
				//adjustparticlepostion
				part.gravity_x = 0;
				part.gravity_y = -2.0f;
				part.velocity_x = 2.0f;
				part.velocity_y = 2.5f;
				//render particaleemitter
				
				wizards[j].position_x = 4.76;
				wizards[j].position_y = 4.76;
				
				Mix_PlayChannel(-1, explosion, 0);
				part.reset();
			}

		}
		for (int j = 0; j < forests.size(); j++) {
			if (BoxBoxCollision(forests[j].position_x, forests[j].position_y, 0.5, 0.5, bullets[i].position_x, bullets[i].position_y, 0.025 * 2, 0.085 * 2)) {
				bullets[i].position_x = 10.0; // backpos.shippositionX;
				bullets[i].position_y = 10.0;
				
				part.position_x = forests[j].position_x;
				part.position_y = forests[j].position_y;
				//adjustparticlepostion
				part.gravity_x = 0;
				part.gravity_y = -2.0f;
				part.velocity_x = 2.0f;
				part.velocity_y = 2.5f;
				//render particaleemitter
				forests[j].position_x = 4.76;
				forests[j].position_y = 4.76;
				Mix_PlayChannel(-1, explosion, 0);
				part.reset();
			}

		}
		for (int j = 0; j < knights.size(); j++) {
			if (BoxBoxCollision(knights[j].position_x, knights[j].position_y, 0.5, 0.5, bullets[i].position_x, bullets[i].position_y, 0.025 * 2, 0.085 * 2)) {
				bullets[i].position_x = 10.0; // backpos.shippositionX;
				bullets[i].position_y = 10.0;
				
				part.position_x = knights[j].position_x;
				part.position_y = knights[j].position_y;
				//adjustparticlepostion
				part.gravity_x = 0;
				part.gravity_y = -2.0f;
				part.velocity_x = 2.0f;
				part.velocity_y = 2.5f;
				//render particaleemitter
				
				knights[j].position_x = 4.76;
				knights[j].position_y = 4.76;
				Mix_PlayChannel(-1, explosion, 0);
				part.reset();
			}

		}

	}
	
		for (int i = 0; i < enemys.size(); i++) {
			if (BoxBoxCollision(playerpos.position_x, playerpos.position_y, 0.5, 0.5, enemys[i].position_x, enemys[i].position_y, 0.5, 0.5)) {
				mode = GAME_OVER;
			}
		}

		for (int i = 0; i < soliders.size(); i++) {
			if (BoxBoxCollision(playerpos.position_x, playerpos.position_y, 0.5, 0.5, soliders[i].position_x, soliders[i].position_y, 0.5, 0.5)) {
				mode = GAME_OVER;
			}
		}
		for (int i = 0; i < wizards.size(); i++) {
			if (BoxBoxCollision(playerpos.position_x, playerpos.position_y, 0.5, 0.5, wizards[i].position_x, wizards[i].position_y, 0.5, 0.5)) {
				mode = GAME_OVER;
			}
		}
		for (int i = 0; i < forests.size(); i++) {
			if (BoxBoxCollision(playerpos.position_x, playerpos.position_y, 0.5, 0.5, forests[i].position_x, forests[i].position_y, 0.5, 0.5)) {
				mode = GAME_OVER;
			}
		}
		for (int i = 0; i < knights.size(); i++) {
			if (BoxBoxCollision(playerpos.position_x, playerpos.position_y, 0.5, 0.5, knights[i].position_x, knights[i].position_y, 0.5, 0.5)) {
				mode = GAME_OVER;
			}
		}

		for (int i = 0; i < dragons.size(); i++) {
			if (BoxBoxCollision(playerpos.position_x, playerpos.position_y, 0.5, 0.5, dragons[i].position_x, dragons[i].position_y, 1.0, 1.0)) {
				mode = GAME_OVER;
			}
		}

		for (int i = 0; i < dragon2s.size(); i++) {
			if (BoxBoxCollision(playerpos.position_x, playerpos.position_y, 0.5, 0.5, dragon2s[i].position_x, dragon2s[i].position_y, 1.0, 1.0)) {
				mode = GAME_OVER;
			}
		}

	int tileX1 = 0;
	int tileY2 = 0;
	float penetration = 0;
	playerpos.position_y += playerpos.velocity_y * elapsed;

	worldToTileCoordinates(playerpos.position_x, playerpos.position_y + 0.5f, &tileX1, &tileY2);
	penetration = fabs((-TILE_SIZE * tileY2) - TILE_SIZE -(playerpos.position_y + 0.5f));

	if (tileX1 > 0 && tileX1 < map->mapWidth && tileY2 > 0 && tileY2 < map->mapHeight) {

		if (issolid(map->mapData[tileY2][tileX1])) {

			playerpos.position_y -= penetration + 0.001f;
			playerpos.velocity_y = 0.0f;
		}
		else if (isexit(map->mapData[tileY2][tileX1]) && state.score >= 3000) {
			if (currentMap == 0) {

				LoadNewMap("cave.txt");

			}

			else if (currentMap == 1) {

				LoadNewMap("castlemap.txt");

			}
		}
	}
		worldToTileCoordinates(playerpos.position_x, playerpos.position_y - 0.5f, &tileX1, &tileY2);
		penetration = fabs((-TILE_SIZE * tileY2) - (playerpos.position_y - 0.5f));
		if (tileX1 > 0 && tileX1 < map->mapWidth && tileY2 > 0 && tileY2 < map->mapHeight) {
		if (issolid(map->mapData[tileY2][tileX1])) {

			playerpos.position_y += penetration + 0.001f;
			playerpos.velocity_y = 0.0f;


		}
		else if (isexit(map->mapData[tileY2][tileX1]) && state.score >= 3000) {
			if (currentMap == 0) {

				LoadNewMap("cave.txt");

			}

			else if (currentMap == 1) {

				LoadNewMap("castlemap.txt");

			}
		}


	}

	int tileX = 0;
	int tileY = 0;

	playerpos.position_x += playerpos.velocity_x * elapsed;
	worldToTileCoordinates(playerpos.position_x + 0.5f, playerpos.position_y, &tileX, &tileY);
	penetration = fabs((-TILE_SIZE * tileX) - (playerpos.position_x + 0.5f));

	if (tileY > 0 && tileY < map->mapHeight && tileX > 0 && tileX < map->mapWidth) {
		if (issolid(map->mapData[tileY][tileX])) {

			playerpos.velocity_x = 0.0f;
			playerpos.position_x -= penetration * 0.001f;
		}
		else if (isexit(map->mapData[tileY][tileX]) && state.score >= 3000) {
			if (currentMap == 0) {

				LoadNewMap("cave.txt");

			}

			else if (currentMap == 1) {

				LoadNewMap("castlemap.txt");

			}
		}

	}

		worldToTileCoordinates(playerpos.position_x - 0.5f, playerpos.position_y, &tileX, &tileY);
	
		penetration = fabs((-TILE_SIZE * tileX + TILE_SIZE) - (playerpos.position_x - 0.5f));
		if (tileY > 0 && tileY < map->mapHeight && tileX > 0 && tileX < map->mapWidth) {
		if (issolid(map->mapData[tileY][tileX])) {

			playerpos.velocity_x = 0.0f;
			playerpos.position_x += penetration * 0.001f;

		}
		else if (isexit(map->mapData[tileY][tileX]) && state.score >= 3000) {
			if (currentMap == 0) {

				LoadNewMap("cave.txt");

			}

			else if (currentMap == 1) {

				LoadNewMap("castlemap.txt");

			}
		}

	}


}
float animationTime = 0;
float textpos_y = 0;
float textpos_x = 0;
void updatetitle(float elapsed) {
	animationTime = animationTime + elapsed;

	float animationValuey = mapValue(animationTime,
		3.0, 6.0, 0.0, 1.0);
	float animationValuex = mapValue(animationTime,
		1.0, 4.0, 0.0, 1.0);
	textpos_y = easeOut(2.55, 1.0, animationValuey);
	textpos_x = easeOut(-3.55, -1.16f, animationValuex);
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

	LoadNewMap("adventureforest.txt");
	vector<float> vertexData;
	vector<float> texCoordData;
	int SPRITE_COUNT_X = 24;
	int SPRITE_COUNT_Y = 16;
	
	programuntextured.Load(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl");
	
	
	Matrix viewMatrixuntext;
	programuntextured.SetModelMatrix(modelMatrixuntext);

	projectionMatrixuntext.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(programuntextured.programID);


	//textured
	ShaderProgram programtextured;
	programtextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint player = LoadTexture("george_0.png");
	GLuint backgroun1 = LoadTexture("dirt-tiles.png");
	
	GLuint backgroun3 = LoadTexture("Wii - Final Fantasy 4 The After Years - Castle Interior Tiles.png");
	GLuint backgroun4 = LoadTexture("arne_sprites.png");
	GLuint dragon = LoadTexture("dragonsprite.png");
	GLuint dragon2 = LoadTexture("dragon2.png");
	GLuint forestenemy = LoadTexture("enemyforrest.png");
	GLuint wizardenemy = LoadTexture("enemywizard.png");
	GLuint soliderenemy = LoadTexture("enemysolider.png");
	GLuint knightrenemy = LoadTexture("enemyknight.png");
	GLuint text = LoadTexture("font1.png");
	Entity entity;


	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(programtextured.programID);

	float lastFrameTicks = 0.0f;

	bool begingame = false;
	float accumulator = 0.0f;

	const int runAnimation[] = { 1, 5, 9, 13 };
	const int runAnimation2[] = { 3, 7, 11, 15 };
	const int runAmimation3[] = { 2, 6, 10, 14 };
	const int runAmimation4[] = { 0, 4, 8, 12 };
	const int indexdragon[] = {1, 11, 20};
	const int numFrames = 4;
	float animationElapsed = 0.0f;
	float framesPerSecond = 30.0f;
	int currentIndex = 0;
	int TOTAL_TITLES = LEVEL_HEIGHT * LEVEL_WIDTH;


	unsigned int levelData[LEVEL_HEIGHT][LEVEL_WIDTH]{};

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);


	laser = Mix_LoadWAV("pistal.wav");
	coinscol = Mix_LoadWAV("coin.wav");

	explosion = Mix_LoadWAV("explode.wav");

	Mix_Music *music;
	music = Mix_LoadMUS("Lobo_Loco_-_01_-_Streetworker_Jack_ID_844.mp3");
	Mix_VolumeMusic(25);
	Mix_PlayMusic(music, -1);


	programtextured.SetProjectionMatrix(projectionMatrix);
	programtextured.SetViewMatrix(viewMatrix);

	srand(time(NULL));
	glPointSize(5.0);
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
						//reset player and enemy position
						LoadNewMap("adventureforest.txt");
						currentMap = 0;
						mode = GAME;
					}
					else if (mode == GAME_OVER){
						
						mode = TITLE_SCREEN;
					}
				}
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				Entity newbullt;

				newbullt.position_x = playerpos.position_x;
				newbullt.position_y = playerpos.position_y;
			
				float bulletVelocity = 1.0f;
				// set x and y velocity based on player direction
				switch (playerdirection)
				{
				case 0:
					newbullt.velocity_x = 0;
					newbullt.velocity_y = -bulletVelocity * 2.5;
					break;
				case 2:
					newbullt.velocity_x = 0;
					newbullt.velocity_y = bulletVelocity * 2.5;
					break;
				case 3:
					newbullt.velocity_x = bulletVelocity * 2.5;
					newbullt.velocity_y = 0;
					break;
				case 1:
					newbullt.velocity_x = -bulletVelocity * 2.5;
					newbullt.velocity_y = 0;
					break;
				}

				newbullt.timeAlive = 0.0f;
				bullets.push_back(newbullt);
				Mix_PlayChannel(-1, laser, 0);
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.4f, 0.3f, 2.25f, 1.0f);
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		switch (mode)
		{
		case TITLE_SCREEN:
			
			updatetitle(elapsed);

			modelMatrixtext.Identity();
			modelMatrixtext.Translate(-3.0f, textpos_y, 0.0f);
			programtextured.SetModelMatrix(modelMatrixtext);
			DrawText(&programtextured, text, "Welcome to Adventure World", ((3.5 / 25.0f) * 2.0f), -0.03f);
			
			modelMatrixtext.Identity();
			modelMatrixtext.Translate(textpos_x, -1.0f, 0.0f);
			programtextured.SetModelMatrix(modelMatrixtext);
			DrawText(&programtextured, text, "Press Enter", ((3.0 / 25.0f) * 2.0f), -0.05f);
			break;

		case GAME:
			
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

			animationElapsed += elapsed;
			if (animationElapsed > 1.0 / framesPerSecond) {
			currentIndex++;
			animationElapsed = 0.0;
			if (currentIndex > numFrames - 1) {
			currentIndex = 0;
			}
			}

			

			//pistol
			for (int b = 0; b < bullets.size(); b++) {
				modelMatrixuntext.Identity();
				modelMatrixuntext.Translate(bullets[b].position_x, bullets[b].position_y, 0.0f);
				programuntextured.SetModelMatrix(modelMatrixuntext);
				programuntextured.SetProjectionMatrix(projectionMatrixuntext);
				programuntextured.SetViewMatrix(viewMatrix);
				float verticespistol[] = { -0.05, -0.05, 0.05, -0.05, 0.05, 0.05, -0.05, -0.05, 0.05, 0.05, -0.05, 0.05 };

				glVertexAttribPointer(programuntextured.positionAttribute, 2, GL_FLOAT, false, 0, verticespistol);
				glEnableVertexAttribArray(programuntextured.positionAttribute);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(programuntextured.positionAttribute);

			}

			//player
			viewMatrix.Identity();
			viewMatrix.Translate(-playerpos.position_x, -playerpos.position_y, 0.0f);
			programtextured.SetViewMatrix(viewMatrix);
			

			//Rendermap1
			modelMatrix.Identity();
			programtextured.SetModelMatrix(mapmodelMatrix);

			vertexData.clear();
			texCoordData.clear();

			for (int y = 0; y < map->mapHeight; y++) {
				for (int x = 0; x < map->mapWidth; x++) {

					if (map->mapData[y][x] != 0) {
						float u = (float)(((int)map->mapData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
						float v = (float)(((int)map->mapData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

						float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
						float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;

						vertexData.insert(vertexData.end(), { TILE_SIZE * x, -TILE_SIZE * y,
							TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
							(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
							TILE_SIZE * x, -TILE_SIZE * y,
							(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
							(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y });

						texCoordData.insert(texCoordData.end(), { u, v,
							u, v + (spriteHeight),
							u + spriteWidth, v + (spriteHeight),
							u, v,
							u + spriteWidth, v + (spriteHeight),
							u + spriteWidth, v
							}
						);
					}
				}
			}
			switch (currentMap) {

			case 0:
				glBindTexture(GL_TEXTURE_2D, backgroun1);
				// bind level 1 texture

				break;

			case 1:
				glBindTexture(GL_TEXTURE_2D, backgroun1);
				//bind level 2 texture

				break;
			case 2:
				glBindTexture(GL_TEXTURE_2D, backgroun3);
				break;
			}
			
			glUseProgram(programtextured.programID);

			glVertexAttribPointer(programtextured.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
			glEnableVertexAttribArray(programtextured.positionAttribute);

			glVertexAttribPointer(programtextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
			glEnableVertexAttribArray(programtextured.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);

			glDisableVertexAttribArray(programtextured.positionAttribute);
			glDisableVertexAttribArray(programtextured.texCoordAttribute);

			
			modelMatrix.Identity();
			modelMatrix.Translate(playerpos.position_x, playerpos.position_y, 0.0f);
			programtextured.SetModelMatrix(modelMatrix);
			programtextured.SetProjectionMatrix(projectionMatrix);
			glBindTexture(GL_TEXTURE_2D, player);
			switch (playerdirection) {
			case 0:
				
				DrawSpriteSheetSprite(programtextured, runAmimation4[currentIndex], 4, 4);
				break;
			case 1:
				
				DrawSpriteSheetSprite(programtextured, runAnimation[currentIndex], 4, 4);
				break;
			case 2:
				
				DrawSpriteSheetSprite(programtextured, runAmimation3[currentIndex], 4, 4);
				break;
			case 3:
				
				DrawSpriteSheetSprite(programtextured, runAnimation2[currentIndex], 4, 4);
				break;
			}
			
			
			//coins
			for (int b = 0; b < coin.size(); b++) {
				modelMatrix.Identity();
				modelMatrix.Translate(coin[b].position_x, coin[b].position_y, 0.0f);
				programtextured.SetModelMatrix(modelMatrix);
				programtextured.SetProjectionMatrix(projectionMatrix);

				glBindTexture(GL_TEXTURE_2D, backgroun4);
				DrawSpriteSheetSprite(programtextured, 51, 16, 8);

			}
			//dragon
			for (int b = 0; b < dragons.size(); b++) {
				modelMatrix.Identity();
				modelMatrix.Translate(dragons[b].position_x, dragons[b].position_y, 0.0f);
				programtextured.SetModelMatrix(modelMatrix);
				programtextured.SetProjectionMatrix(projectionMatrix);

				glBindTexture(GL_TEXTURE_2D, dragon);
				if (indexdragon[1]) {
					DrawSpriteSheetSpritedragon(programtextured, 1, 16, 6);
				}
				else if (indexdragon[11]) {
					DrawSpriteSheetSpritedragon(programtextured, 11, 16, 6);
				}
				else if (indexdragon[20]) {
					DrawSpriteSheetSpritedragon(programtextured, 20, 16, 6);
				}
			}
				for (int b = 0; b < dragon2s.size(); b++) {
				modelMatrix.Identity();
				modelMatrix.Translate(dragon2s[b].position_x, dragon2s[b].position_y, 0.0f);
				programtextured.SetModelMatrix(modelMatrix);
				programtextured.SetProjectionMatrix(projectionMatrix);

				glBindTexture(GL_TEXTURE_2D, dragon2);
				DrawSpriteSheetSpritedragon(programtextured, 8, 10, 8);

			}

			//enemy
			for (int b = 0; b < enemys.size(); b++) {
				modelMatrix.Identity();
				modelMatrix.Translate(enemys[b].position_x, enemys[b].position_y, 0.0f);
				programtextured.SetModelMatrix(modelMatrix);
				programtextured.SetProjectionMatrix(projectionMatrix);

				glBindTexture(GL_TEXTURE_2D, backgroun4);
				DrawSpriteSheetSprite(programtextured, 98, 16, 8);
			}

				
			for (int b = 0; b < forests.size(); b++) {
					modelMatrix.Identity();
					modelMatrix.Translate(forests[b].position_x, forests[b].position_y, 0.0f);
					programtextured.SetModelMatrix(modelMatrix);
					programtextured.SetProjectionMatrix(projectionMatrix);

					glBindTexture(GL_TEXTURE_2D, forestenemy);
					DrawSpriteSheetSprite(programtextured, 1, 4, 1);
				}
			
			for (int b = 0; b < wizards.size(); b++) {
				modelMatrix.Identity();
				modelMatrix.Translate(wizards[b].position_x, wizards[b].position_y, 0.0f);
				programtextured.SetModelMatrix(modelMatrix);
				programtextured.SetProjectionMatrix(projectionMatrix);
				glBindTexture(GL_TEXTURE_2D, wizardenemy);
				DrawSpriteSheetSprite(programtextured, 1, 4, 1);
				
			}
			
			for (int b = 0; b < soliders.size(); b++) {
				modelMatrix.Identity();
				modelMatrix.Translate(soliders[b].position_x, soliders[b].position_y, 0.0f);
				programtextured.SetModelMatrix(modelMatrix);
				programtextured.SetProjectionMatrix(projectionMatrix);
				glBindTexture(GL_TEXTURE_2D, soliderenemy);
				DrawSpriteSheetSprite(programtextured, 2, 4, 1);
				
			}
				for (int b = 0; b < knights.size(); b++) {
					modelMatrix.Identity();
					modelMatrix.Translate(knights[b].position_x, knights[b].position_y, 0.0f);
					programtextured.SetModelMatrix(modelMatrix);
					programtextured.SetProjectionMatrix(projectionMatrix);
					glBindTexture(GL_TEXTURE_2D, knightrenemy);
					DrawSpriteSheetSprite(programtextured, 1, 4, 1);
					
				}
		
			
			modelMatrixuntext.Identity();
			modelMatrixuntext.Translate(part.position_x, part.position_y, 0);
			programuntextured.SetModelMatrix(modelMatrixuntext);
			programuntextured.SetProjectionMatrix(projectionMatrixuntext);
			programuntextured.SetViewMatrix(viewMatrix);
			part.render(&programuntextured);
			break;

		case GAME_OVER :
			viewMatrix.Identity();
			modelMatrixtext.Identity();
			modelMatrixtext.Translate(-2.0f, 1.0f, 0.0f);
			programtextured.SetModelMatrix(modelMatrixtext);
			programtextured.SetViewMatrix(viewMatrix);
			DrawText(&programtextured, text, "GAME OVER", ((3.5 / 25.0f) * 2.0f), -0.03f);
			break;
		}

		//End Drawing
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}