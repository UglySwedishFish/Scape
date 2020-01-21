#pragma once

#include "DependenciesMath.h"
#include <string>
#include "Shader.h"
#include "Window.h"

const Matrix4f CubeProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
const Matrix4f CubeViews[] =
{
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};


namespace Scape {
	namespace Rendering {

		struct TextureGL {
			unsigned int ID;
			Vector2i Resolution;
			TextureGL(unsigned int id = 4294967294U, Vector2i Resolution = Vector2i(0));
			void Bind(unsigned int target) const;
		};

		struct TextureGL3D {
			unsigned int ID;
			Vector3i Resolution;
			TextureGL3D(unsigned int ID = 0, Vector3i Resolution = Vector3i(0)) :
				ID(ID), Resolution(Resolution) {

			}
			void Bind(unsigned int Target) {
				glActiveTexture(GL_TEXTURE0 + Target);
				glBindTexture(GL_TEXTURE_3D, ID);
			}
		};

		struct TextureCubeMap {
			unsigned int ID;
			glm::ivec2 Resolution;
			TextureCubeMap(unsigned int id = 4294967295, glm::ivec2 res = glm::ivec2(0));
			void Bind(unsigned int target);
		};

		TextureGL LoadTextureGL(const std::string& Path, int PixelAccess = GL_RGBA);
		TextureGL LoadTextureGLKeepSfImage(const std::string& Path, sf::Image* Image, int PixelAccess = GL_RGBA);

		TextureGL3D LoadTextureGL3D(const std::string& Path, Vector3i Resolution);
		TextureCubeMap LoadHDRI(const char* name, bool linear, bool mipmaps, Shader& EquirectangularToCubeMapShader);
		TextureCubeMap LoadCubeMap(const char* name, const char* type);


		struct TexturePT {

			char* Data; 
			//int2 Size; 

			size_t GetSizeInBytes() {
				return 4; 
			}

			TexturePT() : 
				Data(new char[16])
				{
				Data[0] = Data[1] = Data[2] = Data[3] = (char)0xFF;
				Data[4] = Data[5] = Data[6] = Data[7] = (char)0xFF;
				Data[8] = Data[9] = Data[10] = Data[11] = (char)0xFF;
				Data[12] = Data[13] = Data[14] = Data[15] = (char)0xFF;
			}

			TexturePT(bool Empty) {}

			

		};

		TexturePT LoadFromSF(sf::Image* Image);

	}
}
