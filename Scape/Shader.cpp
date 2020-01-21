#include "Shader.h"
#include "GL/glad.h"
#include <string> 
#include <fstream> 
#include <iostream> 

#define throwerror(s) std::cout << s

#define GLOBAL_SHADER_DEFINITIONS "#define double highp float\n"


unsigned int Scape::Rendering::LoadShader(unsigned int ShaderType, const std::string& File, unsigned int& Buffer, unsigned int BaseID, bool ReCreate) {
	std::string source = ""; //all the shader code
	std::ifstream file(File);

	if (!file.is_open()) {
		throwerror(static_cast<std::string>("Failed to open file: ") + File);
		return 0; //-1 hurr durr
	}

	int LineCnt = 0; 

	while (!file.eof()) {
		LineCnt++;
		char line[50000];
		file.getline(line, 50000);
		std::string Line = line;

		if (source.size() == 0) {
			source += Line + '\n';
			source += GLOBAL_SHADER_DEFINITIONS;
		}
		else if (Line[0] == '#' && Line[1] == 'i' && Line[2] == 'n') {
			char Id[100];
			int i = sscanf(Line.c_str(), "#include %s", Id);
			if (i != 0) { //probably shouldn't be copying any code from here, this is a strictly "it works" brute force way to do it 
				std::string Dir = "shaders/" + static_cast<std::string>(Id);

				std::ifstream IncludeFile(Dir);

				if (!IncludeFile.is_open())
					std::cout << "Failed to open file (" + Dir << ") in line: " << Line << ", line count: " << LineCnt << "\n";
				else {

					std::cout << "Opened file!\n";

					while (!IncludeFile.eof()) {
						char IncludeLine[500];

						IncludeFile.getline(IncludeLine, 500);

						std::string IncLine = IncludeLine;

						source += IncLine + '\n';

					}
				}
			}
		}
		else {
			source += Line + '\n';
		}




	}


	//std::cout << source << '\n'; 

	if (ReCreate)
		BaseID = glCreateShader(ShaderType); //compile the shader 

	std::cout << "Shader created!\n";

	const char* csource = source.c_str();



	if (ReCreate)
		glGenBuffers(1, &Buffer);
	glShaderSource(BaseID, 1, &csource, NULL);
	glCompileShader(BaseID);
	char error[15000];
	glGetShaderInfoLog(BaseID, 15000, NULL, error);

	std::cout << "Hmm\n";


	throwerror(error);
	std::string s = error;



	if (s.length() > 3)
		std::cout << "Shader: " << File << " compiled with either errors or warnings\n";

	std::cout << "BaseID: " << BaseID << '\n';

	return BaseID;
}

Scape::Rendering::Shader::Shader(const std::string& vertex, const std::string& fragment) :
	VertexShader(LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader)),
	FragmentShader(LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader)),
	ShaderID(glCreateProgram()) {
	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);
}
Scape::Rendering::Shader::Shader(const std::string& vertex, const std::string& geometry, const std::string& fragment) :
	VertexShader(LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader)),
	GeometryShader(LoadShader(GL_GEOMETRY_SHADER, geometry, GeometryBuffer, GeometryShader)),
	FragmentShader(LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader)),
	ShaderID(glCreateProgram()) {
	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, GeometryShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);
}

Scape::Rendering::Shader::Shader(const std::string& Directory, bool HasGeometryShader) {
	if (HasGeometryShader)
		* this = Shader(Directory + "/shader.vert", Directory + "/shader.geom", Directory + "/shader.frag");
	else
		*this = Shader(Directory + "/shader.vert", Directory + "/shader.frag");
}

Scape::Rendering::Shader::Shader() :
	VertexShader(NULL),
	FragmentShader(NULL),
	GeometryShader(NULL),
	ShaderID(NULL) {
}

void Scape::Rendering::Shader::Bind() {
	glUseProgram(ShaderID);
}

void Scape::Rendering::Shader::UnBind() {
	glUseProgram(NULL);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, int Value) const {
	glUniform1i(glGetUniformLocation(ShaderID, Name.c_str()), Value);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, size_t Value) const {
	glUniform1i(glGetUniformLocation(ShaderID, Name.c_str()), Value);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, float Value) const {
	glUniform1f(glGetUniformLocation(ShaderID, Name.c_str()), Value);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, bool Value) const {
	glUniform1i(glGetUniformLocation(ShaderID, Name.c_str()), Value);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, Vector2f Value) const {
	glUniform2f(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, Vector3f Value) const {
	glUniform3f(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y, Value.z);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, Vector4f Value) const {
	glUniform4f(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y, Value.z, Value.w);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, Vector2i Value) const {
	glUniform2i(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, Vector3i Value) const {
	glUniform3i(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y, Value.z);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, Vector4i Value) const {
	glUniform4i(glGetUniformLocation(ShaderID, Name.c_str()), Value.x, Value.y, Value.z, Value.w);
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, Matrix4f Value, bool Transpose) const {
	glUniformMatrix4fv(glGetUniformLocation(ShaderID, Name.c_str()), 1, Transpose, glm::value_ptr(Value));
}

void Scape::Rendering::Shader::SetUniform(const std::string& Name, Matrix3f Value, bool Transpose) const {
	glUniformMatrix3fv(glGetUniformLocation(ShaderID, Name.c_str()), 1, Transpose, glm::value_ptr(Value));
}

void Scape::Rendering::Shader::Reload(const std::string& vertex, const std::string& fragment) {


	glDetachShader(ShaderID, VertexShader);
	glDetachShader(ShaderID, FragmentShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	glDeleteProgram(ShaderID);

	ShaderID = glCreateProgram();


	VertexShader = LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader, true);
	FragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader, true);

	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);

}

void Scape::Rendering::Shader::Reload(const std::string& vertex, const std::string& fragment, const std::string& geometry) {
	glDetachShader(ShaderID, VertexShader);
	glDetachShader(ShaderID, FragmentShader);
	glDetachShader(ShaderID, GeometryShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
	glDeleteShader(GeometryShader);

	glDeleteProgram(ShaderID);

	ShaderID = glCreateProgram();


	VertexShader = LoadShader(GL_VERTEX_SHADER, vertex, VertexBuffer, VertexShader, true);
	GeometryShader = LoadShader(GL_GEOMETRY_SHADER, geometry, GeometryBuffer, GeometryShader, true);
	FragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragment, FragmentBuffer, FragmentShader, true);

	glAttachShader(ShaderID, VertexShader);
	glAttachShader(ShaderID, GeometryShader);
	glAttachShader(ShaderID, FragmentShader);
	glLinkProgram(ShaderID);
	glUseProgram(ShaderID);
}

void Scape::Rendering::Shader::Reload(const std::string& Directory) {
	Reload(Directory + "/shader.vert", Directory + "/shader.frag");
}

void Scape::Rendering::Shader::Reload(const std::string& Directory, bool HasGeometryShader) {
	if (HasGeometryShader)
		Reload(Directory + "/vert.glsl", Directory + "/frag.glsl", Directory + "/geom.glsl");
	else
		Reload(Directory + "/vert.glsl", Directory + "/frag.glsl", Directory + "/geom.glsl");

}

Scape::Rendering::Shader::~Shader() {
	//TODO: free up shader
}

