#include <glm/gtx/euler_angles.hpp>
// Create the world matrix for the robot

// function to create world matrix from assignment 7
glm::mat4 eulerWM(glm::vec3 pos, glm::vec3 YPR)
{
	glm::mat4 MEa = glm::eulerAngleYXZ(glm::radians(YPR.y), glm::radians(YPR.x), glm::radians(YPR.z));
	return glm::translate(glm::mat4(1.0), pos) * MEa;
}

glm::mat4 getRobotWorldMatrix(GLFWwindow *window)
{
	// just setting deltaT to check how much time has passed
	// between getRobotWorldMatrix calls
	static auto startTime = std::chrono::high_resolution_clock::now();
	static float lastTime = 0.0f;
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	float deltaT = time - lastTime;
	lastTime = time;

	float XSPEED = 1.1;							// > 1 -> faster, < 1 -> slower
	float YSPEED = 1.8;							// > 1 -> faster, < 1 -> slower
	float DIAGSPEED = 0.8;						// > 1 -> faster, < 1 -> slower
	static glm::vec3 pos = glm::vec3(-3, 1, 2); // player position

	std::unordered_map<std::string, glm::vec3> CamDir;
	CamDir["east"] = glm::vec3(-1, 0, 0);
	CamDir["west"] = glm::vec3(1, 0, 0);
	CamDir["north"] = glm::vec3(0, 0, 1);
	CamDir["south"] = glm::vec3(0, 0, -1);

	std::unordered_map<std::string, float> yawAngles;
	yawAngles["east"] = 0.0f;
	yawAngles["nort-east"] = 45.0f;
	yawAngles["north"] = 90.0f;
	yawAngles["north-west"] = 135.0f;
	yawAngles["west"] = 180.0f;
	yawAngles["south-west"] = 225.0f;
	yawAngles["south"] = 270.0f;
	yawAngles["south-east"] = 315.0f;

	float angle = 90.0f;
	float oscillation = 0.0f;

	pos += YSPEED * CamDir["north"] * deltaT;

	// common WASD input control
	// west
	if (glfwGetKey(window, GLFW_KEY_A))
	{
		pos += XSPEED * CamDir["west"] * deltaT;
		angle = yawAngles["north-west"];
	}
	// east
	else if (glfwGetKey(window, GLFW_KEY_D))
	{
		pos += XSPEED * CamDir["east"] * deltaT;
		angle = yawAngles["north-east"];
	}
	// north
	else if (glfwGetKey(window, GLFW_KEY_W))
	{
		pos += YSPEED * CamDir["north"] * (deltaT / 2);
		angle = yawAngles["north"];
	}
	// south
	else if (glfwGetKey(window, GLFW_KEY_S))
	{
		pos += YSPEED * CamDir["south"] * (deltaT / 2);
		angle = yawAngles["north"];
	}
	// north-west
	else if (glfwGetKey(window, GLFW_KEY_A) && glfwGetKey(window, GLFW_KEY_W))
	{
		pos += YSPEED * CamDir["north"] * (deltaT / 2);
		pos += XSPEED * CamDir["west"] * deltaT;
		angle = yawAngles["north-west"];
	}
	// north-east
	else if (glfwGetKey(window, GLFW_KEY_W) && glfwGetKey(window, GLFW_KEY_D))
	{
		pos += YSPEED * CamDir["north"] * (deltaT / 2);
		pos += XSPEED * CamDir["east"] * deltaT;
		angle = yawAngles["north-east"];
	}
	// south-east
	else if (glfwGetKey(window, GLFW_KEY_D) && glfwGetKey(window, GLFW_KEY_S))
	{
		pos += YSPEED * CamDir["south"] * (deltaT / 2);
		pos += XSPEED * CamDir["east"] * deltaT;
		angle = yawAngles["north-east"];
	}
	// south-west
	else if (glfwGetKey(window, GLFW_KEY_A) && glfwGetKey(window, GLFW_KEY_S))
	{
		pos += YSPEED * CamDir["south"] * (deltaT / 2);
		pos += XSPEED * CamDir["east"] * deltaT;
		angle = yawAngles["north-west"];
	}

	oscillation = sin(time);
	// world matrix to give as output
	return eulerWM(pos, glm::vec3(oscillation * 10, angle, 0.0f));
}