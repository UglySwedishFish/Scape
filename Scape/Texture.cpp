#include "Texture.h"
#include "DependenciesRendering.h"
#include "Framebuffer.h"
#include <iostream>

namespace Scape {
	namespace Rendering {

		TextureGL::TextureGL(unsigned int id, Vector2i Resolution) :
			ID(id), Resolution(Resolution) {

		}
		void TextureGL::Bind(unsigned int target) const {
			glActiveTexture(GL_TEXTURE0 + target);
			glBindTexture(GL_TEXTURE_2D, ID);
		}

		int GetFormat(int X) {
			switch (X) {
			case GL_RGBA:
				return GL_RGBA8;
				break;
			case GL_RGB:
				return GL_RGB8;
				break;
			case GL_RG:
				return GL_RG8;
				break;
			case GL_RED:
				return GL_R8;
				break;
			}
		}

		TextureGL LoadTextureGL(const std::string& Path, int PixelAccess) {
			unsigned int id;
			sf::Image Image;

			if (Image.loadFromFile(Path)) {


				sf::Uint8* Pixels = nullptr;
				auto BasePixels = Image.getPixelsPtr();
				Image.flipVertically();

				if (PixelAccess == GL_RGBA)
					Pixels = const_cast<sf::Uint8*>(Image.getPixelsPtr());
				else {

					int PixelByteSize = 3;
					if (PixelAccess == GL_RG)
						PixelByteSize = 2;
					else if (PixelAccess == GL_RED)
						PixelByteSize = 1;

					Pixels = new sf::Uint8[(int)(Image.getSize().x) * (int)(Image.getSize().y) * PixelByteSize];

					for (int Pixel = 0; Pixel < (int)(Image.getSize().x) * (int)(Image.getSize().y); Pixel++)
						for (int i = 0; i < PixelByteSize; i++) {
							Pixels[(Pixel)* PixelByteSize + i] = BasePixels[(Pixel) * 4 + i];
						}



				}


				glGenTextures(1, &id);
				glBindTexture(GL_TEXTURE_2D, id);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -2);
				glTexImage2D(GL_TEXTURE_2D, 0, GetFormat(PixelAccess), Image.getSize().x, Image.getSize().y, 0, PixelAccess, GL_UNSIGNED_BYTE, Pixels);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);

				std::cout << "Format1: " << GetFormatText(PixelAccess) << " Format2: " << GetFormatText(GetFormat(PixelAccess)) << '\n';


				return TextureGL(id, glm::ivec2(Image.getSize().x, Image.getSize().y));
			}
			else {
				return TextureGL();
			}
		}

		TextureGL LoadTextureGLKeepSfImage(const std::string& Path, sf::Image* Image, int PixelAccess)
		{
			unsigned int id;

			if (Image->loadFromFile(Path)) {
				

				sf::Uint8* Pixels = nullptr;
				auto BasePixels = Image->getPixelsPtr();
				Image->flipVertically();

				if (PixelAccess == GL_RGBA)
					Pixels = const_cast<sf::Uint8*>(Image->getPixelsPtr());
				else {

					int PixelByteSize = 3;
					if (PixelAccess == GL_RG)
						PixelByteSize = 2;
					else if (PixelAccess == GL_RED)
						PixelByteSize = 1;

					Pixels = new sf::Uint8[(int)(Image->getSize().x) * (int)(Image->getSize().y) * PixelByteSize];

					for (int Pixel = 0; Pixel < (int)(Image->getSize().x) * (int)(Image->getSize().y); Pixel++)
						for (int i = 0; i < PixelByteSize; i++) {
							Pixels[(Pixel)* PixelByteSize + i] = BasePixels[(Pixel) * 4 + i];
						}



				}


				glGenTextures(1, &id);
				glBindTexture(GL_TEXTURE_2D, id);
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -2);

				glTexImage2D(GL_TEXTURE_2D, 0, GetFormat(PixelAccess), Image->getSize().x, Image->getSize().y, 0, PixelAccess, GL_UNSIGNED_BYTE, Pixels); 

				glGenerateMipmap(GL_TEXTURE_2D); 

				glBindTexture(GL_TEXTURE_2D, 0);

				//std::cout << "Format1: " << GetFormatText(PixelAccess) << " Format2: " << GetFormatText(GetFormat(PixelAccess)) << '\n';


				return TextureGL(id, glm::ivec2(Image->getSize().x, Image->getSize().y));
			}
			else {
				return TextureGL();
			}
		}

		Vector2i ConvertTo2D(Vector3i Pixel, Vector3i Resolution) {

			int X = Pixel.x % Resolution.x + Pixel.y * Resolution.x;
			int Y = Pixel.z;

			return Vector2i(X, Y);

		}

		TextureGL3D LoadTextureGL3D(const std::string& Path, Vector3i Resolution)
		{

			sf::Image RawImage;

			if (RawImage.loadFromFile(Path))
				if (Resolution.x * Resolution.y * Resolution.z <= RawImage.getSize().x * RawImage.getSize().y) {

					//utilize format 


					std::vector<unsigned char> PixelData = std::vector<unsigned char>(Resolution.x * Resolution.y * Resolution.z * 4);


					for (int X = 0; X < Resolution.x; X++) {
						for (int Y = 0; Y < Resolution.y; Y++) {
							for (int Z = 0; Z < Resolution.z; Z++) {

								Vector2i PixelCoordinate = ConvertTo2D(Vector3i(X, Y, Z), Resolution);
								sf::Color SfColor = RawImage.getPixel(PixelCoordinate.x, PixelCoordinate.y);


								PixelData[(X * Resolution.y * Resolution.z + Y * Resolution.z + Z) * 4] = SfColor.r;
								PixelData[(X * Resolution.y * Resolution.z + Y * Resolution.z + Z) * 4 + 1] = SfColor.g;
								PixelData[(X * Resolution.y * Resolution.z + Y * Resolution.z + Z) * 4 + 2] = SfColor.b;
								PixelData[(X * Resolution.y * Resolution.z + Y * Resolution.z + Z) * 4 + 3] = SfColor.a;



							}
						}
					}

					unsigned int ID;

					glGenTextures(1, &ID);

					glBindTexture(GL_TEXTURE_3D, ID);
					glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, Resolution.x, Resolution.y, Resolution.z, 0, GL_RGBA, GL_UNSIGNED_BYTE, &PixelData[0]);
					glBindTexture(GL_TEXTURE_3D, 0);



					return TextureGL3D(ID, Resolution);




				}

			throw std::exception(("Failed to load texture: " + Path).c_str());

			return TextureGL3D();




		}

		TextureCubeMap LoadHDRI(const char* name, bool linear, bool mipmaps, Shader& EquirectangularToCubeMapShader) {


			TextureGL RawHDRIMap = LoadTextureGL(name);
			//get a somewhat accurate resolution to use for cubemap: 
			unsigned int PixelCount = RawHDRIMap.Resolution.x * RawHDRIMap.Resolution.y;

			unsigned int Resolution = sqrt(static_cast<float>(PixelCount) / 6.); //obtain a resolution
			unsigned int ResolutionPower2 = pow(2, round(log2(Resolution) + .25)); //force it to be a power of 2

			unsigned int TemporaryFrameBuffer, TemporaryRenderBuffer, FinalImage;

			glGenFramebuffers(1, &TemporaryFrameBuffer);
			glGenRenderbuffers(1, &TemporaryRenderBuffer);

			glBindFramebuffer(GL_FRAMEBUFFER, TemporaryFrameBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, TemporaryRenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ResolutionPower2, ResolutionPower2);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, TemporaryRenderBuffer);

			glGenTextures(1, &FinalImage);
			glBindTexture(GL_TEXTURE_CUBE_MAP, FinalImage);
			for (unsigned int i = 0; i < 6; ++i)
			{
				// note that we store each face with 16 bit floating point values
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
					ResolutionPower2, ResolutionPower2, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);

			glViewport(0, 0, ResolutionPower2, ResolutionPower2); // don't forget to configure the viewport to the capture dimensions.
			glBindFramebuffer(GL_FRAMEBUFFER, TemporaryFrameBuffer);

			EquirectangularToCubeMapShader.Bind();

			glUniformMatrix4fv(glGetUniformLocation(EquirectangularToCubeMapShader.ShaderID, "ProjectionMatrix"), 1, false, glm::value_ptr(CubeProjection));
			glUniform1i(glGetUniformLocation(EquirectangularToCubeMapShader.ShaderID, "EquirectangularMap"), 0);
			RawHDRIMap.Bind(0);

			for (unsigned int i = 0; i < 6; ++i)
			{
				glUniformMatrix4fv(glGetUniformLocation(EquirectangularToCubeMapShader.ShaderID, "ViewMatrix"), 1, false, glm::value_ptr(CubeViews[i]));
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, FinalImage, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				DrawPostProcessCube(); // renders a 1x1 cube
			}

			EquirectangularToCubeMapShader.UnBind();

			return TextureCubeMap(FinalImage, Vector2i(ResolutionPower2));
		}

		std::string CubeMapSides[6] = {
	"RT.",
	"LF.",
	"DN.",
	"UP.",
	"FT.",
	"BK."
		};
		TextureCubeMap LoadCubeMap(const char* name, const char* filetype)
		{
			unsigned int ID;
			std::string _filenames[6];
			for (int i = 0; i < 6; i++)
				_filenames[i] = std::string(name) + CubeMapSides[i] + filetype;
			glGenTextures(1, &ID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
			for (int i = 0; i < 6; i++) {
				sf::Image image;
				image.loadFromFile(_filenames[i]);
				image.flipVertically();
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			}
			return TextureCubeMap(ID);
		}




		TextureCubeMap::TextureCubeMap(unsigned int id, glm::ivec2 res) :
			ID(id), Resolution(res) {
		}

		void TextureCubeMap::Bind(unsigned int target) {
			glActiveTexture(GL_TEXTURE0 + target);
			glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
		}

		TexturePT LoadFromSF(sf::Image* Image)
		{

			if (Image) {

				TexturePT Texture = TexturePT(true); 

				int XSize = std::min(256u, Image->getSize().x); 
				int YSize = std::min(256u, Image->getSize().y); 
				
				Texture.Data = new char[XSize * YSize * 4]; 
				//Texture.Size = { XSize, YSize }; 


				for (int x = 0; x < XSize; x++) {
					for (int y = 0; y < YSize; y++) {

						float TCx = float(x) / float(XSize); 
						float TCy = float(y) / float(YSize); 

						int PixelX = glm::clamp(int(TCx * Image->getSize().x), 0, int(Image->getSize().x)-1);
						int PixelY = glm::clamp(int(TCy * Image->getSize().y), 0, int(Image->getSize().y)-1);

						auto Color = Image->getPixel(PixelX, PixelY); 

						Texture.Data[(x * YSize + y) * 4] = Color.r; 
						Texture.Data[(x * YSize + y) * 4 + 1] = Color.g;
						Texture.Data[(x * YSize + y) * 4 + 2] = Color.b;
						Texture.Data[(x * YSize + y) * 4 + 3] = Color.a;

					}
				}

				return Texture; 

			}
			else {
				return TexturePT(); 
			}
		}

}
}
