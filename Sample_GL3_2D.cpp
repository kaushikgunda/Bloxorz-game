#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
						  0,                  // attribute 0. Vertices
						  3,                  // size (x,y,z)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  1,                  // attribute 1. Color
						  3,                  // size (r,g,b)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

const float side = 1;
int game_progress; // -1 = lost; 0 = in progress, 1 = won
bool take_action = false;
int last_move = -1;

class cuboid {
	public:
		int state; // 0 = along x-axis, 1 = along y-axis, 2 = along z-axis
		glm::mat4 rotation;
		float x, y, z; // position of center
		float one_x, one_y, one_z; //one and two refer to the centers of two cubes that make up the cuboid
		float two_x, two_y, two_z;
		int moves;
		VAO *obj;

		cuboid()
		{
			state = 1;
			rotation = glm::mat4(1.0f);
			x = y = z = one_x = one_z = two_x = two_z = 0;
			one_y = -1*side/2;
			two_y = side/2;
			moves = 0;
		}

		void create()
		{
			static const GLfloat vertex_buffer_data [] = {
				// front
				-1*side/2, side, side/2,		// vertex 1
				side/2, side, side/2,			// vertex 2
				-1*side/2, -1*side, side/2,	// vertex 4

				side/2, side, side/2,			// vertex 2
				-1*side/2, -1*side, side/2,	// vertex 4
				side/2, -1*side, side/2,		// vertex 3

				// back
				-1*side/2, side, -1*side/2,	// vertex 5
				side/2, side, -1*side/2,		// vertex 6
				-1*side/2, -1*side, -1*side/2,	// vertex 8

				side/2, side, -1*side/2,		// vertex 6
				-1*side/2, -1*side, -1*side/2,	// vertex 8
				side/2, -1*side, -1*side/2,	// vertex 7

				// left
				-1*side/2, side, side/2,		// vertex 1
				-1*side/2, side, -1*side/2,	// vertex 5
				-1*side/2, -1*side, side/2,	// vertex 4

				-1*side/2, side, -1*side/2,	// vertex 5
				-1*side/2, -1*side, side/2,	// vertex 4
				-1*side/2, -1*side, -1*side/2,	// vertex 8

				// right
				side/2, side, side/2,			// vertex 2
				side/2, side, -1*side/2,		// vertex 6
				side/2, -1*side, side/2,		// vertex 3

				side/2, side, -1*side/2,		// vertex 6
				side/2, -1*side, side/2,		// vertex 3
				side/2, -1*side, -1*side/2,	// vertex 7

				// top
				-1*side/2, side, side/2,		// vertex 1
				side/2, side, side/2,			// vertex 2
				-1*side/2, side, -1*side/2,	// vertex 5

				side/2, side, side/2,			// vertex 2
				-1*side/2, side, -1*side/2,	// vertex 5
				side/2, side, -1*side/2,		// vertex 6


				// bottom
				-1*side/2, -1*side, side/2,	// vertex 4
				side/2, -1*side, side/2,		// vertex 3
				-1*side/2, -1*side, -1*side/2,	// vertex 8

				side/2, -1*side, side/2,		// vertex 3
				-1*side/2, -1*side, -1*side/2,	// vertex 8
				side/2, -1*side, -1*side/2,	// vertex 7
			};

			static const GLfloat color_buffer_data [] = {
				0,0,0,
				0,0,0,
				0,0,0,
				
				0,0,0,
				0,0,0,
				0,0,0,

				1,0,1,
				1,0,1,
				1,0,1,
				
				1,0,1,
				1,0,1,
				1,0,1,

				0,0,1,
				0,0,1,
				0,0,1,

				0,0,1,
				0,0,1,
				0,0,1,

				1,1,0,
				1,1,0,
				1,1,0,

				1,1,0,
				1,1,0,
				1,1,0,

				0,1,1,
				0,1,1,
				0,1,1,

				0,1,1,
				0,1,1,
				0,1,1,

				1,1,1,
				1,1,1,
				1,1,1,

				1,1,1,
				1,1,1,
				1,1,1,
			};

			// create3DObject creates and returns a handle to a VAO that can be used later
			obj = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);
		}

		void move(int dir) // 0=Left, 1=Right, 2=Up, 3=Down
		{
			last_move = dir;
			if(game_progress != 0)
				return;
			moves++;
			if (dir == 0) // LEFT
			{
				rotation = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)) * rotation;
				if (state == 0) // along x
				{
					state = 1;
					x -= (side + side/2);
					y += side/2;
				}
				else if (state == 1)
				{
					state = 0;
					x -= (side + side/2);
					y -= side/2;
				}
				else if (state == 2)
				{
					x -= side;
				}
			}
			else if (dir == 1) // RIGHT
			{
				rotation = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1)) * rotation;
				if (state == 0)
				{
					state = 1;
					x += (side + side/2);
					y += side/2;
				}
				else if (state == 1)
				{
					state = 0;
					x += (side + side/2);
					y -= side/2;
				}
				else if (state == 2)
				{
					x += side;
				}
			}
			else if (dir == 2) // UP
			{
				rotation = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(1,0,0)) * rotation;
				if (state == 0)
				{
					z -= side;
				}
				else if (state == 1)
				{
					state = 2;
					z -= (side + side/2);
					y -= side/2;
				}
				else if (state == 2)
				{
					state = 1;
					z -= (side + side/2);
					y += side/2;
				}
			}
			else if (dir == 3) // DOWN
			{
				rotation = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0)) * rotation;
				if (state == 0)
				{
					z += side;
				}
				else if (state == 1)
				{
					state = 2;
					z += (side + side/2);
					y -= side/2;
				}
				else if (state == 2)
				{
					state = 1;
					z += (side + side/2);
					y += side/2;
				}
			}

			if (state == 0) // along x
			{
				one_x = x - side/2;
				two_x = x + side/2;

				one_y = two_y = y;

				one_z = two_z = z;
			}
			else if (state == 1) // along y
			{
				one_x = two_x = x;

				one_y = y + side/2;
				two_y = y - side/2;

				one_z = two_z = z;
			}
			else if (state == 2) // along z
			{
				one_x = two_x = x;

				one_y = two_y = y;

				one_z = z + side/2;
				two_z = z - side/2;
			}
			// cout << cuboidState << endl;
		}
};
cuboid piece;

int mapInd = 0;
const int map_center_i = 5;
const int map_center_j = 5;
const int max_map_size = 10;
const int max_maps = 2;

// -1 indicates where the brick starts
const int maps[max_maps][max_map_size][max_map_size] =
{
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 0, 0, 1, 1, 5, 1},
		{ 1, 1, 4, 1, 0, 0, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
		{ 1,-1, 1, 1, 3, 3, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
		{ 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 0, 0, 1, 1, 5, 1},
		{ 1, 1, 4, 1, 0, 0, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
		{ 1,-1, 1, 1, 3, 3, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
};

// first two numbers indicate switch indices; the rest pairwise tell the bridge indices
const int max_switches = 2;
const int max_switch_size = 10;
const int switches[max_maps][max_switches][max_switch_size] =
{
	{
		{ 4, 2, 6, 4, 6, 5,-1,-1,-1,-1},
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
	},
	{
		{ 4, 2, 6, 4, 6, 5,-1,-1,-1,-1},
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
	},
};

class tiles {
	public:
		int state; //  0 = vacant, 1 = occupied
		int type; // 1 = regular, 2 = fragile, 3 = bridge, 4 = switch, 5 = goal
		int i, j;
		float x, y, z;
		int show;

		tiles(int a, int b, int c) // map matrix coordinates, type
		{
			i = a;
			j = b;

			x = (i - map_center_i)*side;
			y = -1*(side + side/10);
			z = -1*(j - map_center_j)*side;

			show = 1;
			if(c == 3)
				show = 0;

			type = c;
		}
};
vector<tiles> grid;

void init_grid()
{
	game_progress = 0;
	take_action = false;

	while(grid.size() > 0)
    {
        grid.erase(grid.begin());
    }
	for(int i = 0; i < max_map_size; i++)
	{
		for(int j= 0; j < max_map_size; j++)
		{
			if (maps[mapInd][i][j] != 0)
			{
				if (maps[mapInd][i][j] == -1)
				{
					tiles tile(i, j, 1);
					tile.state = 1;
					
					piece.x = (i - map_center_i) * side;
					piece.y = 0;
					piece.z = -1*(j - map_center_j) * side;
					piece.one_x = piece.two_x = piece.x;
					piece.one_y = piece.y + side/2;
					piece.two_y = piece.y - side/2;
					piece.one_z = piece.two_z = piece.z;
					piece.rotation = glm::mat4(1.0f);
					piece.state = 1;

					// cout << (tile.x == piece.x) << " " << (tile.z == piece.z) << endl;
					grid.push_back(tile);
				}
				else
				{
					tiles tile(i, j, maps[mapInd][i][j]);
					tile.state = 0;
					grid.push_back(tile);
				}
			}
		}
	}
}

bool is_occupied(int cube, float x, float z)
{
	if (cube == 1)
		if (piece.one_x == x)
			if (piece.one_z == z)
				return true;
	
	if (cube == 2)
		if (piece.two_x == x)
			if (piece.two_z == z)
				return true;

	return false;	
}

void toggle_bridge(int i, int j)
{
	for(int a = 0; a < max_switches; a++)
	{
		if(switches[mapInd][a][0] == i && switches[mapInd][a][1] == j)
		{
			for(int b = 2; b < max_switch_size; b+=2)
			{
				for(int c = 0; c < grid.size(); c++)
				{
					if(grid[c].i == switches[mapInd][a][b] && grid[c].j == switches[mapInd][a][b + 1])
					{
						grid[c].show = !grid[c].show;
					}
				}
			}
		}
	}
}

// Eye - Location of camera. Don't change unless you are sure!!
// glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
glm::vec3 eye (4, 4, 4);
// Target - Where is the camera looking at.  Don't change unless you are sure!!
glm::vec3 target (0, 0, 0);
// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
glm::vec3 up (0, 1, 0);

int view_mode = 0;
float r;
double theta; // angle against x on xy plane
double phi; // angle against z axis
void change_camera()
{
	// cout << view_mode << endl;
	switch(view_mode) {
		case 0: // Tower view
			r = 8;
			theta = 45; // angle against x on xy plane
			phi = 45; // angle against z axis

			eye[0] = -2;
			eye[1] = 5;
			eye[2] = 8;

			target[0] = 0;
			target[1] = 0;
			target[2] = 0;
			break;
		case 1: // Top view
			eye[0] = eye[2] = 0.1;
			eye[1] = 4;

			target[0] = 0;
			target[1] = 0;
			target[2] = 0;
			break;
		case 2: // Block view
			eye[0] = piece.x;
			eye[1] = piece.y + side*2;
			eye[2] = piece.z;

			for(int k = 0; k < grid.size(); k++)
			{
				if(grid[k].type == 5)
				{
					target[0] = grid[k].x;
					target[1] = 0;
					target[2] = grid[k].z;
					break;
				}
			}
			break;
		case 3: // Follow view
			eye[0] = piece.x;
			eye[1] = piece.y + side*2;
			eye[2] = piece.z + side*4;

			for(int k = 0; k < grid.size(); k++)
			{
				if(grid[k].type == 5)
				{
					target[0] = grid[k].x;
					target[1] = 0;
					target[2] = grid[k].z;
					break;
				}
			}
			break;
		case 4: // Helicopter view
			// cout << theta << " " << phi << endl;
			eye[0] = r*sin(phi)*cos(theta);
			eye[1] = r*sin(phi)*sin(theta);
			eye[2] = r*cos(phi);

			target[0] = 0;
			target[1] = 0;
			target[2] = 0;
			break;
		default:
			break;
	}
}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	 // Function is called first on GLFW_PRESS.
	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				view_mode = (view_mode + 1) % 5;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			case GLFW_KEY_R:
				if(mapInd != max_maps)
					init_grid();
			case GLFW_KEY_N:
				if(mapInd+1 != max_maps && game_progress == 1)
				{
					mapInd++;
					init_grid();
				}
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ENTER:
				mapInd = 0;
				piece.moves = 0;
				init_grid();
				break;
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_LEFT:
				piece.move(0);
				break;
			case GLFW_KEY_RIGHT:
				piece.move(1);
				break;
			case GLFW_KEY_UP:
				piece.move(2);
				break;
			case GLFW_KEY_DOWN:
				piece.move(3);
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
bool pan_drag = false;
float mouseX, mouseY;
float mousePanX, mousePanY;
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				triangle_rot_dir *= -1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
                if (pan_drag)
                	pan_drag = false;
            }
            if (action == GLFW_PRESS)
            {
                pan_drag = true;
                mousePanX = mouseX;
				mousePanY = mouseY;
            }
			break;
		default:
			break;
	}
}

void pan(int x, int y)
{
	// PAN += direction*0.1;
	// cout << x << " " << y << endl;
	theta -= y*0.01;
	phi += x*0.01;
}

void mousePos (GLFWwindow* window, double x, double y)
{
    // x = (x - 350) * 4 / 350.0;
    // x = (x + PAN)/ZOOM;

    // y = (y - 350) * -4 / 350.0;
    // y = (y + PAN)/ZOOM;

    mouseX = x;
    mouseY = y;
    
    // cout << x << " " << y << endl;
}

void zoom(int direction) // -1: out, 1: in
{
	r -= direction*1;
    cout <<  "ZOOM: x" << r <<endl;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoom(yoffset);
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	 is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	// Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;
VAO *reg, *frag, *bridge, *swch;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1.2,-1,0, // vertex 1
		1.2,-1,0, // vertex 2
		1.2, 1,0, // vertex 3

		1.2, 1,0, // vertex 3
		-1.2, 1,0, // vertex 4
		-1.2,-1,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// creates the tile objects
void createTiles()
{
	static const GLfloat vertex_buffer_data [] = {
		// front
		-1*side/2, side/10, side/2,		// vertex 1
		side/2, side/10, side/2,			// vertex 2
		-1*side/2, -1*side/10, side/2,	// vertex 4

		side/2, side/10, side/2,			// vertex 2
		-1*side/2, -1*side/10, side/2,	// vertex 4
		side/2, -1*side/10, side/2,		// vertex 3

		// back
		-1*side/2, side/10, -1*side/2,	// vertex 5
		side/2, side/10, -1*side/2,		// vertex 6
		-1*side/2, -1*side/10, -1*side/2,	// vertex 8

		side/2, side/10, -1*side/2,		// vertex 6
		-1*side/2, -1*side/10, -1*side/2,	// vertex 8
		side/2, -1*side/10, -1*side/2,	// vertex 7

		// left
		-1*side/2, side/10, side/2,		// vertex 1
		-1*side/2, side/10, -1*side/2,	// vertex 5
		-1*side/2, -1*side/10, side/2,	// vertex 4

		-1*side/2, side/10, -1*side/2,	// vertex 5
		-1*side/2, -1*side/10, side/2,	// vertex 4
		-1*side/2, -1*side/10, -1*side/2,	// vertex 8

		// right
		side/2, side/10, side/2,			// vertex 2
		side/2, side/10, -1*side/2,		// vertex 6
		side/2, -1*side/10, side/2,		// vertex 3

		side/2, side/10, -1*side/2,		// vertex 6
		side/2, -1*side/10, side/2,		// vertex 3
		side/2, -1*side/10, -1*side/2,	// vertex 7

		// top
		-1*side/2, side/10, side/2,		// vertex 1
		side/2, side/10, side/2,			// vertex 2
		-1*side/2, side/10, -1*side/2,	// vertex 5

		side/2, side/10, side/2,			// vertex 2
		-1*side/2, side/10, -1*side/2,	// vertex 5
		side/2, side/10, -1*side/2,		// vertex 6


		// bottom
		-1*side/2, -1*side/10, side/2,	// vertex 4
		side/2, -1*side/10, side/2,		// vertex 3
		-1*side/2, -1*side/10, -1*side/2,	// vertex 8

		side/2, -1*side/10, side/2,		// vertex 3
		-1*side/2, -1*side/10, -1*side/2,	// vertex 8
		side/2, -1*side/10, -1*side/2,	// vertex 7
	};

	static const GLfloat reg_color_buffer_data [] = {
		0,0,0,
		0,0,0,
		0,0,0,
		
		0,0,0,
		0,0,0,
		0,0,0,

		1,0,1,
		1,0,1,
		1,0,1,
		
		1,0,1,
		1,0,1,
		1,0,1,

		0,0,1,
		0,0,1,
		0,0,1,

		0,0,1,
		0,0,1,
		0,0,1,

		1,1,0,
		1,1,0,
		1,1,0,

		1,1,0,
		1,1,0,
		1,1,0,

		0,1,1,
		0,1,1,
		0,1,1,

		0,1,1,
		0,1,1,
		0,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,
	};

	static const GLfloat frag_color_buffer_data [] = {
		0,0,0,
		0,0,0,
		0,0,0,
		
		0,0,0,
		0,0,0,
		0,0,0,

		1,0,1,
		1,0,1,
		1,0,1,
		
		1,0,1,
		1,0,1,
		1,0,1,

		0,0,1,
		0,0,1,
		0,0,1,

		0,0,1,
		0,0,1,
		0,0,1,

		1,1,0,
		1,1,0,
		1,1,0,

		1,1,0,
		1,1,0,
		1,1,0,

		0,0.5,1,
		0,0.5,1,
		0,0.5,1,

		0,0.5,1,
		0,0.5,1,
		0,0.5,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,
	};

	static const GLfloat bridge_color_buffer_data [] = {
		0,0,0,
		0,0,0,
		0,0,0,
		
		0,0,0,
		0,0,0,
		0,0,0,

		1,0,1,
		1,0,1,
		1,0,1,
		
		1,0,1,
		1,0,1,
		1,0,1,

		0,0,1,
		0,0,1,
		0,0,1,

		0,0,1,
		0,0,1,
		0,0,1,

		1,1,0,
		1,1,0,
		1,1,0,

		1,1,0,
		1,1,0,
		1,1,0,

		0.8,1,1,
		0.8,1,1,
		0.8,1,1,

		0.8,1,1,
		0.8,1,1,
		0.8,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,
	};

	static const GLfloat swch_color_buffer_data [] = {
		0,0,0,
		0,0,0,
		0,0,0,
		
		0,0,0,
		0,0,0,
		0,0,0,

		1,0,1,
		1,0,1,
		1,0,1,
		
		1,0,1,
		1,0,1,
		1,0,1,

		0,0,1,
		0,0,1,
		0,0,1,

		0,0,1,
		0,0,1,
		0,0,1,

		1,1,0,
		1,1,0,
		1,1,0,

		1,1,0,
		1,1,0,
		1,1,0,

		1,1,0.5,
		1,1,0.5,
		1,1,0.5,

		1,1,0.5,
		1,1,0.5,
		1,1,0.5,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	reg = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, reg_color_buffer_data, GL_FILL);
	frag = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, frag_color_buffer_data, GL_FILL);
	bridge = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, bridge_color_buffer_data, GL_FILL);
	swch = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, swch_color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

float fall_speed;
const float gravity = 10;

void init_game()
{
	fall_speed = 300;
	if(game_progress == 1)
	{
		// mapInd++;
		if(mapInd+1 >= max_maps)
		{
			cout << "All Levels Completed!" << endl;
			cout << "Total moves used: " << piece.moves << endl;
			cout << "Press ENTER to play from Level 1, or press Q to quit" << endl;
		}
		else
		{
			cout << "LEVEL PASSED!" << endl;
			cout << "Total moves used: " << piece.moves << endl;
			cout << "Press N to  go to next level, press ENTER to play from Level 1, or press Q to quit" << endl;
		}
	}
	else if(game_progress == -1)
	{
		cout << "LEVEL FAILED!" << endl;
		cout << "Total moves used: " << piece.moves << endl;
		cout << "Press R to repeat current level, press ENTER to play from Level 1, or press Q to quit" << endl;
	}
	else
	{
		game_progress = 0;
		init_grid();
	}
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Compute Camera matrix (view)
	change_camera();
	Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	// Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	// PANNING
	if (pan_drag)
	{
		// cout << mouseX << " " << mouseY << endl;
		if(mouseX != mousePanX || mouseY != mousePanY)
		{	
			pan(mousePanX-mouseX, mousePanY-mouseY);
			mousePanX = mouseX;
			mousePanY = mouseY;
		}
	}

	if(game_progress != 0)
	{
		if(piece.y < -8)
		{
			if(!take_action)
			{
				take_action = true;
				init_game();
			}
		}
		else
		{
			fall_speed += gravity*0.5;
			piece.y -= (fall_speed + (gravity)*0.5*0.5/2)/10000;
		}
	}

	// CUBOID
	glm::mat4 translateCuboid = glm::translate (glm::vec3(piece.x, piece.y, piece.z)); // glTranslatef
	glm::mat4 cuboidTransform = translateCuboid * piece.rotation;
	Matrices.model *= cuboidTransform; 
	MVP = VP * Matrices.model; // MVP = p * V * M
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if(game_progress == 0 || !take_action)
		draw3DObject(piece.obj);
	Matrices.model = glm::mat4(1.0f);

	// GRID
	bool off_grid_1 = true, off_grid_2 = true;
	for(int i = 0; i < grid.size(); i++)
	{
		glm::mat4 translateTile = glm::translate (glm::vec3(grid[i].x, grid[i].y, grid[i].z)); // glTranslatef
		glm::mat4 tileTransform = translateTile;
		Matrices.model *= tileTransform; 
		MVP = VP * Matrices.model; // MVP = p * V * M
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		bool occupied1 = is_occupied(1, grid[i].x, grid[i].z);
		bool occupied2 = is_occupied(2, grid[i].x, grid[i].z);
		
		if(occupied1)
			off_grid_1 = false;
		if(occupied2)
			off_grid_2 = false;

		switch(grid[i].type) {
			case 1: // regular tile
				grid[i].state = (occupied1 || occupied2);
				draw3DObject(reg);
				break;
			case 2: // fragile
				grid[i].state = (occupied1 || occupied2);
				if (occupied1 && occupied2) // breaking condition
				{
					grid[i].show = 0;
					off_grid_1 = off_grid_2 = true;
					// cout << "fragile tile broken" << endl;
				}
				if(grid[i].show)
					draw3DObject(frag);
				break;
			case 3: // bridge
				grid[i].state = (occupied1 || occupied2);
				if (grid[i].show == 1)
					draw3DObject(bridge);
				if (grid[i].state == 1 && grid[i].show == 0)
					off_grid_1 = off_grid_2 = true;
				break;
			case 4: // switch
				if (occupied1 || occupied2)
				{
					if(grid[i].state == 0)
						toggle_bridge(grid[i].i, grid[i].j);
				}
				grid[i].state = (occupied1 || occupied2);
				draw3DObject(swch);
				break;
			case 5: // goal
				grid[i].state = (occupied1 || occupied2);
				if (occupied1 && occupied2)
				{
					game_progress = 1;
					// cout << "goal" << endl;
				}
				break;
			default:
				break;
		}
		Matrices.model = glm::mat4(1.0f);
	}

	if (off_grid_1 || off_grid_2)
	{
		piece.move(last_move);
		game_progress = -1;
		// cout << "OFF GRID!" << endl;
	}

	// TRIANGLE (DEFAULT)
	glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	Matrices.model *= triangleTransform; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	// draw3DObject(triangle);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	// draw3DObject(rectangle);

	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		// exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		// exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	 is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetCursorPosCallback(window, mousePos);
    glfwSetScrollCallback(window, scrollCallback);

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();

	piece.create();
	createTiles();
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	// glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	init_game();

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	// exit(EXIT_SUCCESS);
}
