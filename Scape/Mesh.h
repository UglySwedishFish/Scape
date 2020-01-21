#pragma once

#include "Shader.h"
#include "Texture.h"
#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Scape {
	namespace Rendering {

		enum class TextureType : unsigned char { AlbedoMap = 0, NormalMap, RoughnessMap, MetalnessMap, EmissiveMap, TextureCount };

		const int TextureFormat[5] = { GL_RGBA, GL_RGB, GL_RED, GL_RED, GL_RED };
		
		void UpdateMaterialContainer(); 
		unsigned int GetMaterialContainer(); 
		
		struct Mesh { //note; this is NOT a model. It is only a collection of the data required to construct a model 

			struct MeshEntry {
				unsigned int NumIndices;
				unsigned int BaseVertex;
				unsigned int BaseIndex;
				unsigned int MaterialIndex;
				inline MeshEntry() :
					NumIndices(0),
					BaseVertex(0),
					BaseIndex(0),
					MaterialIndex(0)
				{}
			};

			std::vector<MeshEntry> MeshEntries;
			std::vector<unsigned int> Materials;

			std::vector<Vector3f> Vertices;
			std::vector<Vector3f> Normals;
			std::vector<Vector3f> Tangents;

			std::vector<Vector3f> TexCoords;
			std::vector<unsigned int> Indices;

			std::vector<unsigned int> SingleMeshIndicies;
			void ClearVectors();
			~Mesh();

		};

		bool InitMesh(const aiMesh* aiMesh, Mesh& mesh);

		struct Material {
			TextureGL Textures[static_cast<unsigned long long>(TextureType::TextureCount)] = { TextureGL(), TextureGL(), TextureGL(), TextureGL(),TextureGL() };
			float BaseMetalness = 0., BaseRoughness = 0., BaseEmissive = 0.0;
			unsigned short TextureOffsetDiffuse = 0, TextureOffsetEmissive = 0;
			Vector3f Albedo = Vector3f(0.);
		};

		void InitMaterials(const aiScene* Scene, std::vector<Material>& Materials, unsigned int* CombinedTextures);
		void LoadMeshData(const char* file, Mesh& Model, std::vector<Material>& Materials, std::unique_ptr<Assimp::Importer>& ObjectImporter, unsigned int* CombinedTextures);
		void LoadOnlyMeshData(const char* file, Mesh& Model, std::unique_ptr<Assimp::Importer>& ObjectImporter); 
		std::string GetModelName(std::string RawModelName); 

		std::vector<float> ConvertToImage(std::vector<Material>& Materials);

		struct Model {
			Mesh ModelData, ModelDataLightMapUV;
			std::vector<Material> Materials;
			std::string ModelName; 
			unsigned int VAO, VBO[6];
			unsigned int VAOShadow, VBOShadow[4];
			unsigned int VAOShadowVertCount;

			unsigned int CombinedTextures[3];
			unsigned int MaterialDataTexture;

			std::vector<Vector3f> Normals, TexCoords, Vertices;
			std::vector<unsigned int> Indicies;

			inline Model(const char* Model, const char * LightMapUVModel, std::unique_ptr<Assimp::Importer>& ObjectImporter) :
				VAO(),
				VBO{},
				ModelName(GetModelName(Model)),
				ModelData(Mesh()) {

				LoadMeshData(Model, ModelData, Materials, ObjectImporter, CombinedTextures);
				LoadOnlyMeshData(Model, ModelDataLightMapUV, ObjectImporter); 
				PrepareForRendering();
			}

			inline Model() :
				VAO(),
				VBO{},
				ModelData(Mesh()) {
			}

			void PrepareForRendering();
			void Draw();
			void DrawInstanced(int InstanceCount);
			void DrawWithSimplifiedMaterialsInstanced(Shader& Shader, int BaseImage, int InstanceCount);


			void DrawWithMaterialsInstanced(const Shader& Shader, int BaseImage, int InstanceCount) const;
			void DrawWithMaterials(Shader& Shader, int BaseImage);

		};


		std::vector<unsigned int> &GetCombinedTextures(); 
		void ConstructCombinedTextures(unsigned int TEXTURE_RES = 128); 
		void UpdateCombinedTextures(unsigned int TEXTURE_RES = 128); 
		


	}
}
