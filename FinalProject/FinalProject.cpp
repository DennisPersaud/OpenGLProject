/* Header Inclusions*/
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL Image loader Inclusion
#include "SOIL2/SOIL2.h"

using namespace std; // Standard namespace

#define WINDOW_TITLE "Modern OpenGL" // Window title Macro

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif


/*Variable declarations for shaders and window size initialization, buffer and vertex array objects*/
GLint stoolShaderProgram, lampShaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, EBO, texture, stoolVAO, LightVAO;
GLfloat degrees = glm::radians(-45.0f); // Converts float to degrees

//Subject position and scale
glm::vec3 stoolPosition(0.0f, 0.0f, 0.0f);
glm::vec3 stoolScale(2.0f);

//stool and light color
glm::vec3 objectColor(0.6f, 0.5f, 0.75f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

//Light position and scale
glm::vec3 lightPosition(0.5f, 0.5f, -3.0f);
glm::vec3 lightScale(0.3f);

GLfloat cameraSpeed = 0.0005f; // Movement speed per frame


GLfloat lastMouseX = 400, lastMouseY = 300; // Locks mouse cursor at the center of the screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f; // mouse offset, yaw, and pitch variables
GLfloat sensitivity = 0.5f; // Used for mouse / camera rotation sensitivity
bool mouseDetected = true; // Initially true when mouse movement is detected

//Global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Initial camera position. Placed 5 units in Z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); //Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); // Temporary z unit vector
glm::vec3 front; // Temporary z unit vector for mouse

/*Function prototypes*/
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UGenerateTexture(void);
void UMouseMove(int x, int y);

/* stool Vertex Shader Source Code */
const GLchar * vertexShaderSource = GLSL(330,

		layout (location = 0) in vec3 position; // VAP position 0 for vertex position data
		layout (location = 1) in vec3 normal; // VAP position 1 for normals

		out vec3 mobileColor; // variable to transfer color data to the fragment shader

		// Uniform Global variables for the transform matrices
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

void main(){

	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
	mobileColor = normal; // references incoming color data
	}
);


/* stool Fragment Shader Source Code */
const GLchar * fragmentShaderSource = GLSL(330,

		in vec3 Normal; // For incoming normals
		in vec3 FragmentPos; // For incoming fragment position

		out vec4 stoolColor; // For outgoing stool color to the GPU

		// Uniform / Global variables for object color, light color, light position, and camera/view position
		uniform vec3 objectColor;
		uniform vec3 lightColor;
		uniform vec3 lightPos;
		uniform vec3 viewPosition;

	void main(){

		/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

		/*Calculate Ambient lighting*/
		float ambientStrength = 0.1f; // Set ambient or global lighting strength
		vec3 ambient = ambientStrength * lightColor;


		/*Calculate Diffuse lighting*/
		vec3 norm = normalize(Normal); // Normalize vectors to 1 unit
		vec3 lightDirection = normalize(lightPos - FragmentPos); // Calculate distance (light direction) between light source and fragment
		float impact = max(dot(norm, lightDirection), 0.0); // Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse = impact * lightColor; // Generate diffuse light color


		/*Calculate specular lighting*/
		float specularIntensity = 0.8f; // Set specular light strength
		float highlightSize = 16.0f; // Set specular highlight size
		vec3 viewDir = normalize(viewPosition - FragmentPos); // Calculate view direction
		vec3 reflectDir = reflect(-lightDirection, norm); // Calculate reflection vector
		//Calculate specular component
		float specularComponent - pow(max(dot(viewDir, reflectDir), 0.0), hightlightSize);
		vec3 specular = specularIntensity * specularComponent * lightColor;

		//Calculate phong result
		vec3 phong = (ambient + diffuse + specular) * objectColor;

		stoolColor = vec4(phong, 1.0f); // Send lighting results to GPU
	}
);


/* Lamp Shader Source Code*/
const GLchar * lampVertexShaderSource = GLSL(330,

		layout (location = 0) in vec3 position; // VAP position 0 for vertex position data

		//Uniform / Global variables for the transform matrices
		uniform mat4 model;
		uniform mat4 view;
		unifirm mat4 projection;

		void main()
		{
			gl_Position = projection * view * model * vec4(position, 1.0f); // Tranforms vertices into clip coordinates
		}
);


/*Fragment Shader Source Code*/
const GLchar * lampFragmentShaderSource = GLSL(330,

		out vec4 color; // For outgoing lamp color (smaller stool) to the GPU

		void main()
		{
			color = vec4(1.0f) //Set color to white (1.0f, 1.0f, 1.0f) with alpha 1.0

		}
);


/*Main Program*/
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);


	glewExperimental =  GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW" << std::endl;
			return -1;
		}

	UCreateShader();

	UCreateBuffers();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color

	glutDisplayFunc(URenderGraphics);


	glutMainLoop();

	// Destroys Buffer objects once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	return 0;

}

/* Resizes the window*/
void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

/* Renders graphics*/
void URenderGraphics(void){

	glEnable(GL_DEPTH_TEST); // Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the screen

	GLint modelLoc, viewLoc, projLoc, objectColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc;

	CameraForwardZ = front; // Replaces camera forward vector with Radians normalized as a unit vector


	/******Use the stool shader and activate the stool vertex array object for rendering and transforming******/
	glUseProgram(stoolShaderProgram);
	glBindVertexArray(stoolVAO);

	// Transforms the stool
	glm::mat4 model;
	glm::mat4 projection;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Place the object at the center of the viewport
	model = glm::rotate(model, 45.0f, glm::vec3(0.0, 1.0f, 0.0f)); // Rotate the object 45 degrees on the X
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); // Increase the object size by a scale of 2

	// Transforms the camera
	glm::mat4 view;
	view = glm::lookAt(CameraForwardZ, cameraPosition, CameraUpY);

	// Set the camera projection to perspective
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);

	// Reference matrix uniforms from the stool shader program
	modelLoc = glGetUniformLocation(stoolShaderProgram, "model");
	viewLoc = glGetUniformLocation(stoolShaderProgram, "view");
	projLoc = glGetUniformLocation(stoolShaderProgram, "projection");

	// Pass matrix data to the stool shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Reference matrix uniforms from the stool shader program for the stool color, light color, light position, and camera position
	objectColorLoc = glGetUniformLocation(stoolShaderProgram, "objectColor");
	lightColorLoc = glGetUniformLocation(stoolShaderProgram, "lightColor");
	lightPositionLoc = glGetUniformLocation(stoolShaderProgram, "lightPos");
	viewPositionLoc = glGetUniformLocation(stoolShaderProgram, "viewPosition");

	// Pass color, light, and camera data to the stool shader program's corresponding uniforms
	glUniform3f(objectColorLoc, objectColor.r, objectColor.g, objectColor.b);
	glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	glDrawArrays(GL_TRIANGLES, 0, 36); //Draw the primitives / stool

	glBindVertexArray(0); // Deactivate the stool vertex Array Object


	/******Use the lamp shader and activate the lamp vertex array object for rendering and transforming******/
	glUseProgram(lampShaderProgram);
	glBindVertexArray(LightVAO);

	/*Transform the smaller stool used as a visual que for the light source*/
	model = glm::translate(model, lightPosition);
	model = glm::scale(model,lightScale);

	//Reference matrix uniforms from the Lamp Shader program
	modelLoc = glGetUniformLocation(lampShaderProgram, "model");
	viewLoc = glGetUniformLocation(lampShaderProgram, "view");
	projLoc = glGetUniformLocation(lampShaderProgram, "projection");

	// Pass matrix data to the Lamp Shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glDrawArrays(GL_TRIANGLES, 0, 144); // Draw the primitives / small stool(lamp)

	glBindVertexArray(0); // Deactivate the Lamp vertex Array object

	glutPostRedisplay(); // marks the current window to be redisplayed
	glutSwapBuffers(); // Flips the back buffer with the front buffer every frame. Similar to GL Flush


}

/*Creates the Shader program*/
void UCreateShader()
{

	//stool Vertex Shader
	GLint stoolVertexShader = glCreateShader(GL_VERTEX_SHADER); //Creates the vertex shader
	glShaderSource(stoolVertexShader, 1, &vertexShaderSource, NULL); // Attaches the vertex shader to the source code
	glCompileShader(stoolVertexShader); //Compiled the vertex shader

	//stool Fragment Shader
	GLint stoolFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create the fragment shader
	glShaderSource(stoolFragmentShader, 1, &fragmentShaderSource, NULL); // Attaches the Fragment shader to the source code
	glCompileShader(stoolFragmentShader); // Compiles the Fragment Shader

	//stool Shader program
	stoolShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
	glAttachShader(stoolShaderProgram, stoolVertexShader);// Attach Vertex shader to the shader program
	glAttachShader(stoolShaderProgram, stoolFragmentShader); // Attach FragmentShader to the Shader program
	glLinkProgram(stoolShaderProgram); // Link Vertex and Fragment shaders to shader program

	//Delete the stool shaders once linked
	glDeleteShader(stoolVertexShader);
	glDeleteShader(stoolFragmentShader);

	// Lamp vertex shader
	GLint lampVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the vertex shader
	glShaderSource(lampVertexShader, 1, &lampVertexShaderSource, NULL); // Attaches the vertex shader to the source code
	glCompileShader(lampVertexShader); // Compiles the vertex shader

	// Lamp Fragment shader
	GLint lampFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(lampFragmentShader, 1, &lampFragmentShaderSource, NULL); //Attaches the Fragment shader to the source code
	glCompileShader(lampFragmentShader); // Compiles the Fragment shader

	//Lamp Shader program
	lampShaderProgram = glCreateProgram(); //Creates the shader program and returns an id
	glAttachShader(lampShaderProgram, lampVertexShader); // Attach Vertex shader to the shader program
	glAttachShader(lampShaderProgram, lampFragmentShader); // Attach Fragment shader to the shader program
	glLinkProgram(lampShaderProgram); // Link Vertex and Fragment shaders to shader program

	//Delete the lamp shaders once linked
	glDeleteShader(lampVertexShader);
	glDeleteShader(lampFragmentShader);

}

/*Creates the Buffer and Array Objects*/
void UCreateBuffers()
{
	// Positions and Color data
	GLfloat verticies[] = {
			// Vertex Positions		// Colors
			0.0f, 0.5f, -0.5f,	1.0f, 0.0f, 0.0f,	// Top Right vertex 0
			0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f,	// Bottom Right vertex 1
			-0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f,	// Bottom Left Vertex 2
			0.0f, 0.5f, -0.5f,	1.0f, 0.0f, 1.0f,	// Top left Vertex 3

			0.5f, -0.5f, -1.0f,	0.5f, 0.5f, 1.0f,	// 4 br right
			0.5f, 0.5f, -1.0f,	1.0f, 1.0f, 0.5f,	// 5 tl right
			0.0f, 0.5f, -0.5f,	0.2f, 0.2f, 0.5f,	// 6 tl top
			-0.5f, -0.5f, -1.0f, 1.0f, 0.0f, 1.0f	// 7 bl back
							};

	// Index data to share position data
		GLuint indices[] = {
								1, 2, 3, // Triangle 1 front
								0, 1, 4, // Triangle 2 right
								4, 6, 7, // Triangle 3 back
								2, 6, 7, // Triangle 4 left
								1, 4, 7, // Triangle 5 bottom back
								1, 2, 7 // Triangle 6 bottom front
							};

	//Generate buffer id's
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Activate the Vertex Array Object before binding and setting any VBOs and Vertex Attribute POinters.
	glBindVertexArray(VAO);

	// Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW); // Copy vertices to VBO

	// Activate the Element Buffer Object / Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Copy indices to EBO

	// Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Set attribute pointer 1 to hold Color data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); // Enables vertex attribute

	glBindVertexArray(0); // Deactivates the VAO which is good practice

	// Generate buffer ids for lamp(smaller stool)
	glGenVertexArrays(1, &LightVAO); // Vertex Array object for stool vertex copes to serve as light source

	// Activate the Vertex Array object before binding and setting any VBOs and vertex Attribute Pointers
	glBindVertexArray(LightVAO);

	//Referencing the same VBO for its vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Set attribute pointer 0 to hold Position data (used for the lamp)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

//Implements the UMouseMove function
void UMouseMove(int x, int y)
{
	// Immediately replaces center locked coordinates with new mouse coordinates
	if(mouseDetected)
	{
		lastMouseX = x;
		lastMouseY = y;
		mouseDetected = false;
	}

	// Gets the direction the mouse was moved in x and y
	mouseXOffset = x - lastMouseX;
	mouseYOffset = lastMouseY - y;

	// Updates with new mouse coordinates
	lastMouseX = x;
	lastMouseY = y;

	// Applies sensitivity to mouse direction
	mouseXOffset *= sensitivity;
	mouseYOffset *= sensitivity;

	// Accumulates the yaw and pitch variables
	yaw += mouseXOffset;
	pitch += mouseYOffset;

	// Orbits around the center
	front.x = 10.0f * cos(yaw);
	front.y = 10.0f * sin(pitch);
	front.z = sin(yaw) * cos(pitch) * 10.0f;
}


/*Generate and load the texture */
void UGenerateTexture()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height;

	unsigned char* image = SOIL_load_image("wood.jpg", &width, &height, 0, SOIL_LOAD_RGB); // Loads texture file

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}
