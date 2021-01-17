#pragma once
#include <Component/SimpleScene.h>
#include <TextRenderer/TextRenderer.h>

namespace EngineComponents
{
	class Camera;
}

class Gameplay
{
public:
	GLboolean first_person_camera;
	GLint lives;
	GLboolean lifeRefuel, lifeReposition, freeze;
};

class Sphere
{
public:
	glm::vec3 position, angle, rotationSpeed;
	GLfloat movementSpeed, fuel, radius;
	GLboolean debug, jumping, falling, mapFalling, jammed;
};

class Platform
{
public:
	glm::vec3 position, color;
	GLfloat radius = 0.1f;
	GLboolean debug;
};

class Tema2 : public SimpleScene
{
public:
	Tema2();
	~Tema2();

	void Init() override;

	EngineComponents::Camera* camera;

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color = glm::vec3(1));

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;
	
	// Draw the hud
	void drawHud();

	// Window resolution
	glm::ivec2 resolution;

	// Draw primitives mode
	GLenum polygonMode;

	// The text renderer
	TextRenderer* Text;

protected:
	GLboolean intersects(Sphere sphere, Platform platform);

	Mesh* mesh;
	glm::mat4 modelMatrix;

	glm::vec3 cameraPosition;
	glm::vec3 lightPosition;
	glm::vec3 lightDirection;

	GLuint materialShininess;
	GLfloat materialKd;
	GLfloat materialKs;
	GLfloat cutOff;

	glm::vec3 color_white, color_blue, color_purple, color_red, color_yellow, color_orange, color_green, color_lightblue, color_gray, color_black;
	GLboolean red_trigger1, red_trigger2, yellow_trigger1, yellow_trigger2, orange_trigger1, orange_trigger2, green_trigger1, green_trigger2, white_trigger1, white_trigger2, gray_trigger1, gray_trigger2;
};
