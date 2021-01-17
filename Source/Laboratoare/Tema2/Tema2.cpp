#include "Tema2.h"

#include <iostream>
#include <ctime>

#include <Core/Engine.h>
#include <Component/Camera/Camera.h>
#include "Transform3D.h"
#include "Object2D.h"

/*
	BIOLETE Alexandru-Eusebiu - Grupa 334CC

	Tema 2 - Skyroads
*/

// Physics-related values
#define ACCELERATION_COEFFICIENT		0.05f
#define DECELERATION_COEFFICIENT		0.025f
#define BRAKING_COEFFICIENT				0.25f
#define MIN_MOVEMENT_SPEED				0			// Player can't go backwards
#define MAX_MOVEMENT_SPEED				0.25f
#define DIRECTION_SPEED					4
#define MAX_HEIGHT						2.5f

// Map-related values
#define CENTER_X_COORDINATE				4.5f
#define SURFACE_Y_OFFSET				0.5f
#define MAP_FALLING_DISTANCE			-10

// Object-related values
#define SPHERE_RADIUS					0.5f
#define PLATFORM_COUNT					100
#define PLATFORM_SIZE_X					2
#define PLATFORM_SIZE_Y					0.25f
#define PLATFORM_SIZE_Z					4
#define PLATFORM_OFFSET_X				0.5f
#define PLATFORM_OFFSET_Z				0.5f

// Time-related values
#define PRINT_FUEL_DELTA				1
#define RESET_TRIGGER_DELTA				3
#define ORANGE_SPECIAL_DELTA			15

// Gameplay-related values
#define STARTING_LIFE_COUNT				3
#define MAX_LIFE_COUNT					5
#define FUEL_CAPACITY					200

// HUD-related values
#define HUD_BAR_X						1
#define HUD_BAR_Y						0.25f
#define HUD_INDICATOR_RADIUS			0.1f

using namespace std;
using namespace EngineComponents;

// Object-related variables
Sphere sphere;
Platform platform[PLATFORM_COUNT];

// Time-related variables
clock_t this_time = clock();
clock_t last_time = this_time;
GLfloat time_counter1 = 0, time_counter2 = 0, time_counter3 = 0;

// Gameplay-related variables
Gameplay game;

Tema2::Tema2()
{
}

Tema2::~Tema2()
{
}

void Tema2::Init()
{
	camera = GetSceneCamera();
	cameraPosition = glm::vec3(CENTER_X_COORDINATE, 2.5f, -1.5f);
	camera->SetPosition(cameraPosition);
	camera->SetRotation(glm::vec3(RADIANS(330), RADIANS(180), 0));
	camera->Update();
	GetCameraInput()->SetActive(false);

	// Cursor visibility
	ShowCursor(false);

	// Polygon mode
	polygonMode = GL_FILL;

	// Text renderer
	resolution = window->GetResolution();
	Text = new TextRenderer(resolution.x, resolution.y);
	Text->Load("Source/TextRenderer/Fonts/Arial.ttf", 18);

	glm::vec3 corner = glm::vec3(0.001, 0.001, 0);

	// Color values
	{
		color_white = glm::vec3(1, 1, 1);
		color_blue = glm::vec3(0, 0, 1);
		color_purple = glm::vec3(1, 0, 1);
		color_red = glm::vec3(1, 0, 0);
		color_yellow = glm::vec3(1, 1, 0);
		color_orange = glm::vec3(1, 0.5f, 0);
		color_green = glm::vec3(0, 1, 0);
		color_lightblue = glm::vec3(0, 0.5f, 1);
		color_gray = glm::vec3(0.5f, 0.5f, 0.5f);
		color_black = glm::vec3(0, 0, 0);
	}

	// Light & material properties
	{
		lightDirection = glm::vec3(0, 0, RADIANS(180));
		materialShininess = 30;
		materialKd = 0.5;
		materialKs = 0.5;
		cutOff = RADIANS(60);
	}

	// Initialization of triggers for special platform effect activation
	{
		red_trigger1 = false;
		red_trigger2 = true;
		yellow_trigger1 = false;
		yellow_trigger2 = true;
		orange_trigger1 = false;
		orange_trigger2 = true;
		green_trigger1 = false;
		green_trigger2 = true;
		white_trigger1 = false;
		white_trigger2 = true;
		gray_trigger1 = false;
		gray_trigger2 = true;
	}

	// Game settings
	{
		game.first_person_camera = false;
	}

	// Player initialization
	{
		game.lives = STARTING_LIFE_COUNT;
		game.lifeRefuel = false;
		game.lifeReposition = false;
	}

	// HUD
	{
		Mesh* staticBar = Object2D::CreateRectangle("staticBar", corner, HUD_BAR_X, HUD_BAR_Y, color_white, false);
		AddMeshToList(staticBar);
		Mesh* fuelBar = Object2D::CreateRectangle("fuelBar", corner, HUD_BAR_X, HUD_BAR_Y, color_orange, true);
		AddMeshToList(fuelBar);
		Mesh* speedBar = Object2D::CreateRectangle("speedBar", corner, HUD_BAR_X, HUD_BAR_Y, color_lightblue, true);
		AddMeshToList(speedBar);
		Mesh* speedBarJammed = Object2D::CreateRectangle("speedBarJammed", corner, HUD_BAR_X, HUD_BAR_Y, color_green, true);
		AddMeshToList(speedBarJammed);
		Mesh* life = Object2D::CreateCircle("life", corner, HUD_INDICATOR_RADIUS, color_red, true);
		AddMeshToList(life);
	}

	// Platform object
	{
		mesh = new Mesh("box");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		meshes[mesh->GetMeshID()] = mesh;

		// Create a shader program for drawing face polygon with the color of the normal
		{
			Shader* shader = new Shader("PlatformShader");
			shader->AddShader("Source/Laboratoare/Tema2/Shaders/Platform/VertexShader.glsl", GL_VERTEX_SHADER);
			shader->AddShader("Source/Laboratoare/Tema2/Shaders/Platform/FragmentShader.glsl", GL_FRAGMENT_SHADER);
			shader->CreateAndLink();
			shaders[shader->GetName()] = shader;
		}

		// Initialization of the first row of platforms
		for (int i = 0; i < 5; i++) {
			platform[i].position.x = PLATFORM_OFFSET_X + i * 2;
			platform[i].position.z = PLATFORM_OFFSET_Z;
		}

		// Initialization of the rest of the platforms
		for (int i = 5; i < PLATFORM_COUNT; i++) {
			srand(time(NULL) + i);
			platform[i].position.x = PLATFORM_OFFSET_X + PLATFORM_SIZE_X * (rand() % 5);
			platform[i].position.z = PLATFORM_OFFSET_Z + PLATFORM_SIZE_Z * (rand() % (PLATFORM_COUNT / 5));
		}
	}

	// Player object
	{
		mesh = new Mesh("sphere");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;

		// Create a shader program for drawing face polygon with the color of the normal
		{
			Shader* shader = new Shader("SphereShader");
			shader->AddShader("Source/Laboratoare/Tema2/Shaders/Sphere/VertexShader.glsl", GL_VERTEX_SHADER);
			shader->AddShader("Source/Laboratoare/Tema2/Shaders/Sphere/FragmentShader.glsl", GL_FRAGMENT_SHADER);
			shader->CreateAndLink();
			shaders[shader->GetName()] = shader;
		}

		// Create a shader program for drawing face polygon with the color of the normal
		{
			Shader* shader = new Shader("SpecialSphereShader");
			shader->AddShader("Source/Laboratoare/Tema2/Shaders/Sphere/SpecialVertexShader.glsl", GL_VERTEX_SHADER);
			shader->AddShader("Source/Laboratoare/Tema2/Shaders/Sphere/SpecialFragmentShader.glsl", GL_FRAGMENT_SHADER);
			shader->CreateAndLink();
			shaders[shader->GetName()] = shader;
		}

		// Initialization of the player object
		{
			sphere.position.x = platform[2].position.x;
			sphere.position.y = platform[2].position.y + SURFACE_Y_OFFSET + PLATFORM_SIZE_Y + SPHERE_RADIUS;
			//sphere.position.y = 7.5f;
			sphere.position.z = platform[2].position.z;
			sphere.jammed = false;
			sphere.jumping = false;
			sphere.falling = true;
			sphere.mapFalling = false;
			sphere.fuel = FUEL_CAPACITY;
		}
	}
}

void Tema2::drawHud()
{
	if (game.first_person_camera) {
		// Fuel bar
		{
			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(cameraPosition.x / 15, cameraPosition.y / 1.125f, cameraPosition.z + 5);

			RenderSimpleMesh(meshes["staticBar"], shaders["VertexColor"], modelMatrix);

			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(cameraPosition.x / 15 + 0.01f, cameraPosition.y / 1.125f, cameraPosition.z + 4.99f);
			modelMatrix *= Transform3D::Scale(sphere.fuel * 0.00495f, 0.99f, 1);

			RenderSimpleMesh(meshes["fuelBar"], shaders["VertexColor"], modelMatrix);
		}

		// Life count indicators
		{
			for (int i = 0; i < game.lives; i++) {
				modelMatrix = glm::mat4(1);

				modelMatrix *= Transform3D::Translate(cameraPosition.x / 15 + i * 0.25f + 0.075f, cameraPosition.y / 1.3f, cameraPosition.z + 5.01f);

				RenderSimpleMesh(meshes["life"], shaders["VertexColor"], modelMatrix);
			}
		}

		// Speed bar
		{
			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(cameraPosition.x / 0.585f, cameraPosition.y / 1.125f, cameraPosition.z + 5);

			RenderSimpleMesh(meshes["staticBar"], shaders["VertexColor"], modelMatrix);

			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(cameraPosition.x / 0.585f - 0.01f, cameraPosition.y / 1.125f, cameraPosition.z + +4.99f);
			modelMatrix *= Transform3D::Scale(sphere.movementSpeed * 4, 1, 1);

			if (sphere.jammed) {
				RenderSimpleMesh(meshes["speedBarJammed"], shaders["VertexColor"], modelMatrix);
			}
			else {
				RenderSimpleMesh(meshes["speedBar"], shaders["VertexColor"], modelMatrix);
			}
		}
	}
	else {
		// Fuel bar
		{
			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(cameraPosition.x / 15, cameraPosition.y / 1.125f, cameraPosition.z + 5);
			modelMatrix *= Transform3D::RotateOX(RADIANS(30));

			RenderSimpleMesh(meshes["staticBar"], shaders["VertexColor"], modelMatrix);

			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(cameraPosition.x / 15 + 0.01f, cameraPosition.y / 1.125f, cameraPosition.z + 4.99f);
			modelMatrix *= Transform3D::RotateOX(RADIANS(30));
			modelMatrix *= Transform3D::Scale(sphere.fuel * 0.00495f, 0.99f, 1);

			RenderSimpleMesh(meshes["fuelBar"], shaders["VertexColor"], modelMatrix);
		}

		// Life count indicators
		{
			for (int i = 0; i < game.lives; i++) {
				modelMatrix = glm::mat4(1);

				modelMatrix *= Transform3D::Translate(cameraPosition.x / 15 + i * 0.25f, cameraPosition.y / 1.175f, cameraPosition.z + 5.01f);
				modelMatrix *= Transform3D::RotateOX(RADIANS(30));

				RenderSimpleMesh(meshes["life"], shaders["VertexColor"], modelMatrix);
			}
		}

		// Speed bar
		{
			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(cameraPosition.x / 0.585f, cameraPosition.y / 1.125f, cameraPosition.z + 5);
			modelMatrix *= Transform3D::RotateOX(RADIANS(30));

			RenderSimpleMesh(meshes["staticBar"], shaders["VertexColor"], modelMatrix);

			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(cameraPosition.x / 0.51805f, cameraPosition.y / 1.125f, cameraPosition.z + +4.99f);
			modelMatrix *= Transform3D::RotateOX(RADIANS(30));
			modelMatrix *= Transform3D::RotateOY(RADIANS(180));
			modelMatrix *= Transform3D::Scale(sphere.movementSpeed * 4, 1, 1);

			if (sphere.jammed) {
				RenderSimpleMesh(meshes["speedBarJammed"], shaders["VertexColor"], modelMatrix);
			}
			else {
				RenderSimpleMesh(meshes["speedBar"], shaders["VertexColor"], modelMatrix);
			}
		}
	}

	// Text
	{
		if (sphere.fuel == 0 && !sphere.mapFalling && game.lives > 0 && !game.lifeRefuel) {
			Text->RenderText("YOU RAN OUT OF GAS!", resolution.x / 2.35f, resolution.y / 2.2f, 1.0f, glm::vec3(1, 1, 1));
			Text->RenderText("Press ENTER to refuel in exchange of a life.", resolution.x / 2.75f, resolution.y / 2, 1.0f, color_white);
		}
		else if (sphere.mapFalling && game.lives > 0 && !game.lifeReposition) {
			Text->RenderText("YOU FELL!", resolution.x / 2.25f, resolution.y / 2.2f, 1.0f, glm::vec3(1, 1, 1));
			Text->RenderText("Press ENTER to continue in exchange of a life.", resolution.x / 2.95f, resolution.y / 2, 1.0f, color_white);
		}
		else if (((sphere.fuel == 0 && sphere.movementSpeed <= 0) || sphere.mapFalling) && game.lives <= 0) {
			Text->RenderText("GAME OVER!", resolution.x / 2.25f, resolution.y / 2.2f, 1.0f, glm::vec3(1, 1, 1));
			Text->RenderText("Press ENTER to start again.", resolution.x / 2.5f, resolution.y / 2, 1.0f, color_white);
		}
	}
}

void Tema2::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::Update(float deltaTimeSeconds)
{
	glLineWidth(3);
	glPointSize(5);
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	// Real-time related tasks
	// https://stackoverflow.com/questions/10807681/loop-every-10-second
	{
		this_time = clock();
		time_counter1 += (GLdouble)(this_time - last_time);
		time_counter2 += (GLdouble)(this_time - last_time);
		time_counter3 += (GLdouble)(this_time - last_time);
		last_time = this_time;
	
		if (time_counter1 > (GLdouble)(PRINT_FUEL_DELTA * CLOCKS_PER_SEC)) {
			printf("Fuel: %f\n", sphere.fuel);

			time_counter1 -= (GLdouble)(PRINT_FUEL_DELTA * CLOCKS_PER_SEC);
		}

		if (time_counter2 > (GLdouble)(RESET_TRIGGER_DELTA * CLOCKS_PER_SEC)) {
			red_trigger2 = true;
			yellow_trigger2 = true;
			orange_trigger2 = true;
			green_trigger2 = true;
			white_trigger2 = true;
			gray_trigger2 = true;

			time_counter2 -= (GLdouble)(RESET_TRIGGER_DELTA * CLOCKS_PER_SEC);
		}

		if (time_counter3 > (GLdouble)(ORANGE_SPECIAL_DELTA * CLOCKS_PER_SEC)) {
			sphere.jammed = false;

			time_counter3 -= (GLdouble)(ORANGE_SPECIAL_DELTA * CLOCKS_PER_SEC);
		}
	}

	// Revive / Restart
	{
		if (sphere.fuel == 0 && !sphere.mapFalling && game.lives >= 0 && game.lifeRefuel) {
			sphere.fuel = FUEL_CAPACITY;
			game.lives--;
			game.lifeRefuel = false;
		}

		if (sphere.mapFalling && game.lives >= 0 && game.lifeReposition) {
			sphere.position.x = platform[2].position.x;
			sphere.position.y = platform[2].position.y + SURFACE_Y_OFFSET + PLATFORM_SIZE_Y + SPHERE_RADIUS;
			sphere.position.z = platform[2].position.z;
			sphere.movementSpeed = 0;
			sphere.rotationSpeed.x = 0;
			sphere.rotationSpeed.z = 0;
			sphere.jammed = false;
			sphere.mapFalling = false;
			sphere.fuel = FUEL_CAPACITY;
			game.lives--;
			game.lifeReposition = false;
		}
	}

	// Camera mode
	{
		if (game.first_person_camera) {
			cameraPosition = glm::vec3(sphere.position.x, sphere.position.y + 0.5f, sphere.position.z + 1);

			camera->SetPosition(cameraPosition);
			camera->SetRotation(glm::vec3(0, RADIANS(180), 0));
			camera->Update();

			lightPosition = sphere.position + glm::vec3(0, 0, 2);
		}
		else {
			cameraPosition = glm::vec3(CENTER_X_COORDINATE, 4, -3 + sphere.position.z);
			camera->SetPosition(cameraPosition);
			camera->SetRotation(glm::vec3(RADIANS(330), RADIANS(180), 0));
			camera->Update();

			// lightPosition = sphere.position + glm::vec3(0, 0, 0.25f);
			lightPosition = CENTER_X_COORDINATE - glm::vec3(0, 0, 3);
		}
	}

	// Spot-light formula
	{
		lightDirection /= sqrt(pow(lightDirection.x, 2) + pow(lightDirection.y, 2) + pow(lightDirection.z, 2));
	}

	// Sphere
	{
		if (sphere.debug) {
			printf("SphereX: %f\n", sphere.position.x);
			printf("SphereY: %f\n", sphere.position.y);
			printf("SphereZ: %f\n", sphere.position.z);

			sphere.debug = false;
		}

		if (sphere.fuel <= 0) {
			sphere.fuel = 0;
		}

		// Movement
		if (sphere.movementSpeed > MIN_MOVEMENT_SPEED && sphere.fuel > 0) {
			sphere.position.z += sphere.movementSpeed;
			sphere.angle.x += sphere.rotationSpeed.z;
			sphere.angle.z += sphere.rotationSpeed.x;
			sphere.fuel -= sphere.movementSpeed;
		}
		else if (sphere.movementSpeed > MIN_MOVEMENT_SPEED && !sphere.jammed) {
			sphere.movementSpeed -= DECELERATION_COEFFICIENT * deltaTimeSeconds;
			sphere.position.z += sphere.movementSpeed;
			sphere.rotationSpeed.z -= DECELERATION_COEFFICIENT * deltaTimeSeconds;
			sphere.angle.x += sphere.rotationSpeed.z;
			sphere.angle.z += sphere.rotationSpeed.x;
		}

		// Jumping
		if (sphere.jumping && sphere.position.y >= SURFACE_Y_OFFSET + PLATFORM_SIZE_Y + SPHERE_RADIUS) {
			GLfloat jumping_speed;
			if (sphere.position.y < MAX_HEIGHT) {
				jumping_speed = 5;
				sphere.position.y += deltaTimeSeconds * jumping_speed;
				jumping_speed -= deltaTimeSeconds;
			}
			else {
				sphere.jumping = false;
				sphere.falling = true;
			}
		}

		// Falling
		if (sphere.falling) {
			GLfloat falling_speed;
			if (sphere.position.y > SURFACE_Y_OFFSET + PLATFORM_SIZE_Y + SPHERE_RADIUS) {
				falling_speed = 2.5f;
				sphere.position.y -= deltaTimeSeconds * falling_speed;
				falling_speed += deltaTimeSeconds;
			}
			else {
				sphere.position.y = SURFACE_Y_OFFSET + PLATFORM_SIZE_Y + SPHERE_RADIUS;
				sphere.falling = false;
			}
		}

		// Map falling trigger
		{
			GLint counter = 0;
			for (int i = 0; i < PLATFORM_COUNT; i++) {
				if (!intersects(sphere, platform[i])) {
					counter++;
				}
			}

			if (counter == PLATFORM_COUNT && sphere.position.y == platform[0].position.y + SURFACE_Y_OFFSET + PLATFORM_SIZE_Y + SPHERE_RADIUS) {
				sphere.mapFalling = true;
			}
		}

		// Map falling
		if (sphere.mapFalling) {
			GLfloat falling_speed;
			if (sphere.position.y > MAP_FALLING_DISTANCE) {
				sphere.fuel = 0;
				falling_speed = 2.5f;
				sphere.position.y -= deltaTimeSeconds * falling_speed;
				falling_speed += deltaTimeSeconds;
			}
			else {
				falling_speed = 0;
			}
		}

		// Model
		{
			modelMatrix = glm::mat4(1);

			modelMatrix *= Transform3D::Translate(sphere.position.x, sphere.position.y, sphere.position.z);
			modelMatrix *= Transform3D::RotateOX(sphere.angle.x);
			modelMatrix *= Transform3D::RotateOY(0);
			modelMatrix *= Transform3D::RotateOZ(sphere.angle.z);
			modelMatrix *= Transform3D::Scale(SPHERE_RADIUS * 2, SPHERE_RADIUS * 2, SPHERE_RADIUS * 2);

			if (!red_trigger2 || !yellow_trigger2 || !orange_trigger2 || !green_trigger2 || !white_trigger2 || !gray_trigger2 || sphere.jammed) {
				RenderSimpleMesh(meshes["sphere"], shaders["SpecialSphereShader"], modelMatrix);
				if (game.first_person_camera)
					lightPosition = sphere.position + glm::vec3(0, 0, 3);
				else
					lightPosition = sphere.position + glm::vec3(0, 0, 0.25f);
			}
			else {
				RenderSimpleMesh(meshes["sphere"], shaders["SphereShader"], modelMatrix);
			}
		}
	}

	// Platforms
	{
		for (int i = 0; i < PLATFORM_COUNT; i++) {
			if (platform[0].debug) {
				printf("PlatformX: %f\n", platform[i].position.x);
				printf("PlatformY: %f\n", platform[i].position.y);
				printf("PlatformZ: %f\n", platform[i].position.z);

				platform[0].debug = false;
			}

			platform[i].color = color_blue;

			// Change current platform color
			if (intersects(sphere, platform[i])) {
				platform[i].color = color_purple;
			}

			// Red platform logic - Empty gas tank
			{
				if (i % 20 == 10) {
					platform[i % 20].color = color_red;

					if (intersects(sphere, platform[i % 20]) && red_trigger2) {
						red_trigger1 = true;
					}
				}

				if (red_trigger1) {
					sphere.fuel = 0;
					sphere.jammed = false;

					printf("Your fuel has been stolen!\n");

					red_trigger1 = false;
					red_trigger2 = false;
				}
			}

			// Yellow platform logic - 25% fuel loss
			{
				if (i % 20 == 11) {
					platform[i % 20].color = color_yellow;

					if (intersects(sphere, platform[i % 20]) && yellow_trigger2) {
						yellow_trigger1 = true;
					}
				}

				if (yellow_trigger1) {
					sphere.fuel -= 0.25 * sphere.fuel;

					printf("A part of your fuel has been stolen!\n");

					yellow_trigger1 = false;
					yellow_trigger2 = false;
				}
			}

			// Orange platform logic - Player locked at high-speed (acceleration and braking are disabled)
			{
				if (i % 20 == 12) {
					platform[i % 20].color = color_orange;

					if (intersects(sphere, platform[i % 20]) && orange_trigger2) {
						orange_trigger1 = true;
					}
				}

				if (orange_trigger1) {
					sphere.jammed = true;

					sphere.movementSpeed = MAX_MOVEMENT_SPEED;
					sphere.position.z += sphere.movementSpeed;
					sphere.rotationSpeed.z += ACCELERATION_COEFFICIENT * deltaTimeSeconds;
					sphere.angle.x += sphere.rotationSpeed.z;
					sphere.angle.z += sphere.rotationSpeed.x;

					printf("Your pedals are jammed!\n");

					orange_trigger1 = false;
					orange_trigger2 = false;
				}
			}

			// Green platform logic - Refuel
			{
				if (i % 20 == 13) {
					platform[i % 20].color = color_green;

					if (intersects(sphere, platform[i % 20]) && green_trigger2) {
						green_trigger1 = true;
					}
				}

				if (green_trigger1) {
					sphere.fuel = FUEL_CAPACITY;

					printf("You've been refueled!\n");

					green_trigger1 = false;
					green_trigger2 = false;
				}
			}

			// White platform logic - Add life
			{
				if (i % 20 == 14) {
					platform[i % 20].color = color_white;

					if (intersects(sphere, platform[i % 20]) && white_trigger2) {
						white_trigger1 = true;
					}
				}

				if (white_trigger1) {
					if (game.lives < MAX_LIFE_COUNT) {
						game.lives++;
						printf("You got one more life!\n");
					}
					else {
						printf("Too many lives, can't get more...\n");
					}

					white_trigger1 = false;
					white_trigger2 = false;
				}
			}

			// Gray platform logic - Remove life
			{
				if (i % 20 == 15) {
					platform[i % 20].color = color_gray;

					if (intersects(sphere, platform[i % 20]) && gray_trigger2) {
						gray_trigger1 = true;
					}
				}

				if (gray_trigger1) {
					if (game.lives >= 0) {
						game.lives--;
						printf("You lost one life!\n");
					}
					else {
						printf("Game over! You lost all lives...\n");
						sphere.fuel = 0;
						sphere.movementSpeed = 0;
					}

					gray_trigger1 = false;
					gray_trigger2 = false;
				}
			}

			// Move the platforms that are behind the camera at the end of the map
			if (platform[i].position.z + PLATFORM_SIZE_Z < cameraPosition.z) {
				srand(time(NULL) + i);
				platform[i].position.x = PLATFORM_OFFSET_X + PLATFORM_SIZE_X * (rand() % 5);
				platform[i].position.z += PLATFORM_SIZE_Z * PLATFORM_COUNT / 5;
			}

			// Model
			{
				modelMatrix = glm::mat4(1);

				modelMatrix *= Transform3D::Translate(platform[i].position.x, SURFACE_Y_OFFSET, platform[i].position.z);
				modelMatrix *= Transform3D::Scale(PLATFORM_SIZE_X, PLATFORM_SIZE_Y, PLATFORM_SIZE_Z);

				RenderSimpleMesh(meshes["box"], shaders["PlatformShader"], modelMatrix, platform[i].color);
			}
		}
	}
}

void Tema2::FrameEnd()
{
	drawHud();
	// DrawCoordinatSystem();
}

void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	int loc_cut_off = glGetUniformLocation(shader->program, "cut_off");
	glUniform1f(loc_cut_off, cutOff);

	// Set shader uniforms for light & material properties
	// TODO: Set light position uniform
	int light_position = glGetUniformLocation(shader->program, "light_position");
	glUniform3f(light_position, lightPosition.x, lightPosition.y, lightPosition.z);

	int light_direction = glGetUniformLocation(shader->program, "light_direction");
	glUniform3f(light_direction, lightDirection.x, lightDirection.y, lightDirection.z);

	int loc_resolution = glGetUniformLocation(shader->program, "resolution");
	glUniform2i(loc_resolution, resolution.x, resolution.y);

	// TODO: Set eye position (camera position) uniform
	glm::vec3 eyePosition = cameraPosition;
	int eye_position = glGetUniformLocation(shader->program, "eye_position");
	glUniform3f(eye_position, eyePosition.x, eyePosition.y, eyePosition.z);

	// TODO: Set material property uniforms (shininess, kd, ks, object color)
	int material_shininess = glGetUniformLocation(shader->program, "material_shininess");
	glUniform1i(material_shininess, materialShininess);

	int material_kd = glGetUniformLocation(shader->program, "material_kd");
	glUniform1f(material_kd, materialKd);

	int material_ks = glGetUniformLocation(shader->program, "material_ks");
	glUniform1f(material_ks, materialKs);

	int object_color = glGetUniformLocation(shader->program, "object_color");
	glUniform3f(object_color, color.r, color.g, color.b);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

// Documentation for the input functions can be found in: "/Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/Window/InputController.h

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		if (window->KeyHold(GLFW_KEY_W) && !sphere.jammed && !sphere.mapFalling)
		{
			if (sphere.fuel > 0) {
				if (sphere.movementSpeed < MAX_MOVEMENT_SPEED)
					sphere.movementSpeed += ACCELERATION_COEFFICIENT * deltaTime;
				if (sphere.movementSpeed > 0 && sphere.movementSpeed < MAX_MOVEMENT_SPEED)
					sphere.rotationSpeed.z += ACCELERATION_COEFFICIENT * deltaTime;
			}
			else {
				printf("You're out of fuel!\n");
			}
		}
		else {
			if (sphere.movementSpeed > MIN_MOVEMENT_SPEED && sphere.movementSpeed < MAX_MOVEMENT_SPEED)
				sphere.movementSpeed -= DECELERATION_COEFFICIENT * deltaTime;
			if (sphere.rotationSpeed.z > 0)
				sphere.rotationSpeed.z -= DECELERATION_COEFFICIENT * deltaTime;
		}

		if (window->KeyHold(GLFW_KEY_S) && !sphere.jammed && !sphere.mapFalling)
		{
			if (sphere.movementSpeed > MIN_MOVEMENT_SPEED)
				sphere.movementSpeed -= BRAKING_COEFFICIENT * deltaTime;
			if (sphere.rotationSpeed.z > 0)
				sphere.rotationSpeed.z -= BRAKING_COEFFICIENT * deltaTime;
		}

		if (window->KeyHold(GLFW_KEY_A) && !sphere.mapFalling)
		{
			sphere.position.x += DIRECTION_SPEED * deltaTime;
			sphere.rotationSpeed.x -= ACCELERATION_COEFFICIENT * deltaTime;
		}

		if (window->KeyHold(GLFW_KEY_D) && !sphere.mapFalling)
		{
			sphere.position.x -= DIRECTION_SPEED * deltaTime;
			sphere.rotationSpeed.x += ACCELERATION_COEFFICIENT * deltaTime;
		}
	}
}

void Tema2::OnKeyPress(int key, int mods)
{
	// add key press event
	if (key == GLFW_KEY_SPACE && !sphere.jumping && !sphere.falling && !sphere.mapFalling)
		sphere.jumping = true;

	if (key == GLFW_KEY_ENTER && sphere.fuel == 0) {
		game.lifeRefuel = true;
	}
	if (key == GLFW_KEY_ENTER && sphere.mapFalling) {
		game.lifeReposition = true;
	}
	if (key == GLFW_KEY_ENTER && ((sphere.fuel == 0 && sphere.movementSpeed <= 0) || sphere.mapFalling) && game.lives <= 0) {
		Tema2::Init();
	}

	if (key == GLFW_KEY_C && !sphere.mapFalling)
	{
		if (game.first_person_camera)
			game.first_person_camera = false;
		else
			game.first_person_camera = true;
	}

	if (key == GLFW_KEY_P) platform[0].debug = true;

	if (key == GLFW_KEY_O) sphere.debug = true;
}

void Tema2::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
}

// Collision function
GLboolean Tema2::intersects(Sphere sphere, Platform platform)
{
	float platform_min_x = platform.position.x - PLATFORM_SIZE_X / 2;
	float platform_max_x = platform.position.x + PLATFORM_SIZE_X / 2;
	// float platform_min_y = platform.position.y - PLATFORM_SIZE_Y / 2;
	// float platform_max_y = platform.position.y + PLATFORM_SIZE_Y / 2;
	float platform_min_z = platform.position.z - PLATFORM_SIZE_Z / 2;
	float platform_max_z = platform.position.z + PLATFORM_SIZE_Z / 2;

	return platform_max_z > sphere.position.z
		&& platform_min_z < sphere.position.z
		&& platform_max_x > sphere.position.x
		&& platform_min_x < sphere.position.x
		&& platform.position.y + SURFACE_Y_OFFSET + PLATFORM_SIZE_Y + SPHERE_RADIUS == sphere.position.y;
	
	/* 
	 *** Sphere vs. AABB
	 * https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
		float x = max(platform_min_x, min(sphere.position.x, platform_max_x));
		float y = max(platform_min_y, min(sphere.position.y, platform_max_y));
		float z = max(platform_min_z, min(sphere.position.z, platform_max_z));

		float distance = sqrt(pow((x - sphere.position.x), 2) + pow((y - sphere.position.y), 2) + pow((z - sphere.position.z), 2));
	
		return distance < SPHERE_RADIUS;
	 */
}
