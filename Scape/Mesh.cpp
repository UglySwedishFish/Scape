#include "Mesh.h"
#include <iostream>

namespace Scape {

	namespace Rendering {

		using SimplifiedMaterial = Vector4f;

		std::vector<SimplifiedMaterial> SimplifiedMaterials = {};
		std::vector<sf::Image*> SFImages; 
		std::vector<unsigned int> Texture2DArrayContainer; 

		unsigned int ActualSimplifiedImages = 0; 
		unsigned int MaterialDataContainer = 0; 


		void Mesh::ClearVectors() {
			Vertices.clear();
			Normals.clear();
			TexCoords.clear();
			Indices.clear();
			Materials.clear();
			MeshEntries.clear();
		}

		Mesh::~Mesh() {
			//ClearVectors(); 
		}

		void LoadMeshData(const char* file, Mesh& Model, std::vector<Material>& Materials, std::unique_ptr<Assimp::Importer>& ObjectImporter, unsigned int* CombinedTextures) {

			const aiScene* Scene = ObjectImporter->ReadFile(file, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

			if (Scene) {

				Model.MeshEntries.resize(Scene->mNumMeshes);


				unsigned int NumVertices = 0;
				unsigned int NumIndices = 0;

				// Count the number of vertices and indices
				for (unsigned int i = 0; i < Model.MeshEntries.size(); i++) {
					Model.MeshEntries[i].MaterialIndex = Scene->mMeshes[i]->mMaterialIndex;
					Model.MeshEntries[i].NumIndices = Scene->mMeshes[i]->mNumFaces * 3;
					Model.MeshEntries[i].BaseVertex = NumVertices;
					Model.MeshEntries[i].BaseIndex = NumIndices;

					NumVertices += Scene->mMeshes[i]->mNumVertices;
					NumIndices += Model.MeshEntries[i].NumIndices;
				}


				for (unsigned int i = 0; i < Model.MeshEntries.size(); i++) {
					const aiMesh* aiMesh = Scene->mMeshes[i];
					InitMesh(aiMesh, Model);
				}
				InitMaterials(Scene, Materials, CombinedTextures);

				








			}
			else {
				std::cout << "Failed to load model: " << file << " (Error: " << ObjectImporter->GetErrorString() << ") \n";
			}
		}

		void LoadOnlyMeshData(const char* file, Mesh& Model, std::unique_ptr<Assimp::Importer>& ObjectImporter)
		{

			const aiScene* Scene = ObjectImporter->ReadFile(file, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

			if (Scene) {

				Model.MeshEntries.resize(Scene->mNumMeshes);


				unsigned int NumVertices = 0;
				unsigned int NumIndices = 0;

				// Count the number of vertices and indices
				for (unsigned int i = 0; i < Model.MeshEntries.size(); i++) {
					Model.MeshEntries[i].MaterialIndex = Scene->mMeshes[i]->mMaterialIndex;
					Model.MeshEntries[i].NumIndices = Scene->mMeshes[i]->mNumFaces * 3;
					Model.MeshEntries[i].BaseVertex = NumVertices;
					Model.MeshEntries[i].BaseIndex = NumIndices;

					NumVertices += Scene->mMeshes[i]->mNumVertices;
					NumIndices += Model.MeshEntries[i].NumIndices;
				}


				for (unsigned int i = 0; i < Model.MeshEntries.size(); i++) {
					const aiMesh* aiMesh = Scene->mMeshes[i];
					InitMesh(aiMesh, Model);
				}
			}

		}

		std::string GetModelName(std::string RawModelName)
		{
			
			std::string Name, FinalName = ""; 

			for (int Char = RawModelName.size() - 1; Char >= 0; Char--) {

				if (RawModelName[Char] == '/' || RawModelName[Char] == '\\') {

					break;

				}
				else
					Name += RawModelName[Char]; 


			}
			
			for (int Char = Name.size() - 1; Char >= 0; Char--) {
				FinalName += Name[Char];
			}

			return FinalName;

		}

		std::vector<float> ConvertToImage(std::vector<Material>& Materials)
		{
			auto Data = std::vector<float>(Materials.size() * 3, 0.);

			for (int Material = 0; Material < Materials.size(); Material++) {


				auto ThisMaterial = Materials[Material];

				Data[Material * 3] = glm::intBitsToFloat(65535 * ThisMaterial.TextureOffsetDiffuse + ThisMaterial.TextureOffsetEmissive);
				Data[Material * 3 + 1] = glm::uintBitsToFloat(glm::packHalf2x16(Vector2f(ThisMaterial.Albedo.x, ThisMaterial.Albedo.y)));
				Data[Material * 3 + 2] = glm::uintBitsToFloat(glm::packHalf2x16(Vector2f(ThisMaterial.Albedo.z, ThisMaterial.BaseRoughness)));

			}
			return Data;

		}

		std::vector<unsigned int> &GetCombinedTextures()
		{
			return Texture2DArrayContainer;
		}

		void ConstructCombinedTextures(unsigned int TEXTURE_RES)
		{
			if (Texture2DArrayContainer.size() == 0) {
				Texture2DArrayContainer.resize(10); // for technical reasons, max limit for simplified textures is 2560  

				for (int i = 0; i < 10; i++)
					Texture2DArrayContainer[i] = 0;

			}

			UpdateCombinedTextures(TEXTURE_RES);
		}

		void UpdateCombinedTextures(unsigned int TEXTURE_RES)
		{


			ActualSimplifiedImages = (SFImages.size() / 255) + 1;

			for (int i = 0; i < ActualSimplifiedImages; i++)
				if (Texture2DArrayContainer[i] == 0)
					glGenTextures(1, &Texture2DArrayContainer[i]);

			unsigned int CurrentTexture = 0; 


			for (int i = 0; i < ActualSimplifiedImages; i++) {

				unsigned short TextureArraySize = 255;

				if (SFImages.size() - CurrentTexture < 255) {
					TextureArraySize = SFImages.size() - CurrentTexture;
				}

				std::cout << "Creating texture: " << i << "\n"; 

				glBindTexture(GL_TEXTURE_2D_ARRAY, Texture2DArrayContainer[i]);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D_ARRAY,
					GL_TEXTURE_MIN_FILTER,
					GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY,
					GL_TEXTURE_MAG_FILTER,
					GL_LINEAR);
				glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, TEXTURE_RES, TEXTURE_RES, TextureArraySize, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

				for (int SubImage = 0; SubImage < TextureArraySize; SubImage++) {

					std::cout << "Creating subtexture: " << SubImage << "\n";


					std::vector<unsigned char> ImageData = std::vector<unsigned char>(TEXTURE_RES * TEXTURE_RES * 3);

					auto Pixels = SFImages[CurrentTexture]->getPixelsPtr();




					for (int x = 0; x < TEXTURE_RES; x++) {
						for (int y = 0; y < TEXTURE_RES; y++) {

							//grab the correct coords

							float TextureX = float(x) / float(TEXTURE_RES);
							float TextureY = float(y) / float(TEXTURE_RES);

							int TexelX = glm::clamp(int(TextureX * SFImages[CurrentTexture]->getSize().x), 0, (int)SFImages[CurrentTexture]->getSize().x);
							int TexelY = glm::clamp(int(TextureY * SFImages[CurrentTexture]->getSize().y), 0, (int)SFImages[CurrentTexture]->getSize().y);

							for (int i = 0; i < 3; i++) {
								ImageData[(y * TEXTURE_RES + x) * 3 + i] = Pixels[(TexelY * SFImages[CurrentTexture]->getSize().x + TexelX) * 4 + i];

							}



						}
					}

					glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
						0,
						0, 0, SubImage,
						TEXTURE_RES, TEXTURE_RES, 1,
						GL_RGB,
						GL_UNSIGNED_BYTE,
						ImageData.data());

					CurrentTexture++;

				}

				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

				//glFinish(); 

			}
		}

		void UpdateMaterialContainer()
		{

			if (SimplifiedMaterials.size() == 0)
				return; //something went very wrong here...

			if (MaterialDataContainer == 0) {
				glGenTextures(1, &MaterialDataContainer); 
			}

			/*for (auto& Mat : SimplifiedMaterials) {

				Mat = SimplifiedMaterials[16]; 

			}*/

			std::cout << "Material color: " << SimplifiedMaterials[18].x << ' ' << SimplifiedMaterials[18].y << ' ' << SimplifiedMaterials[18].z << '\n';


			glBindTexture(GL_TEXTURE_2D, MaterialDataContainer); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SimplifiedMaterials.size(), 1, 0, GL_RGBA, GL_FLOAT, SimplifiedMaterials.data()); 
			glBindTexture(GL_TEXTURE_2D, 0);


		}

		unsigned int GetMaterialContainer()
		{
			return MaterialDataContainer; 
		}

		bool InitMesh(const aiMesh* aiMesh, Mesh& mesh) {


			unsigned int MaterialOffset = SimplifiedMaterials.size(); 

			for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
				const aiVector3D* VertexPos = &(aiMesh->mVertices[i]);
				const aiVector3D* VertexNormal = &(aiMesh->mNormals[i]);
				const aiVector3D* VertexTextureCoordinate = aiMesh->HasTextureCoords(0) ? &(aiMesh->mTextureCoords[0][i]) : nullptr;
				const aiVector3D* VertexTangent = aiMesh->HasTangentsAndBitangents() ? &(aiMesh->mTangents[i]) : nullptr;

				//if (!aiMesh->HasTangentsAndBitangents())
				//	std::cout << "Warning: model does not have proper tangents!\n"; 



				mesh.Vertices.push_back(glm::vec3(VertexPos->x, VertexPos->y, VertexPos->z));
				mesh.Normals.push_back(glm::vec3(VertexNormal->x, VertexNormal->y, VertexNormal->z));


				if (VertexTextureCoordinate)
				{
					mesh.TexCoords.push_back(Vector3f(VertexTextureCoordinate->x, VertexTextureCoordinate->y, float(aiMesh->mMaterialIndex + SimplifiedMaterials.size()) + 0.1));
				}
				else
				{
					mesh.TexCoords.push_back(Vector3f(0., 0., float(aiMesh->mMaterialIndex + SimplifiedMaterials.size()) + 0.1f));
				}

				if (VertexTangent) {
					mesh.Tangents.push_back(glm::vec3(VertexTangent->x, VertexTangent->y, VertexTangent->z));
				}
				else {
					mesh.Tangents.push_back(glm::vec3(0.));

				}


				mesh.Materials.push_back(aiMesh->mMaterialIndex);
			}

			for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
				const aiFace& Face = aiMesh->mFaces[i];
				if (Face.mNumIndices == 3) { //if it isn't a triangle, skip this face
					mesh.Indices.push_back(Face.mIndices[0]);
					mesh.Indices.push_back(Face.mIndices[1]);
					mesh.Indices.push_back(Face.mIndices[2]);
				}
				else {
					mesh.Indices.push_back(Face.mIndices[0]);
					mesh.Indices.push_back(Face.mIndices[0]);
					mesh.Indices.push_back(Face.mIndices[0]);
				}
			}
			return true;
		}

		void InitMaterials(const aiScene* Scene, std::vector<Material>& Materials, unsigned int* CombinedTextures)
		{

			std::vector<sf::Image*> Images[2];

			if (CombinedTextures != nullptr) {


				glGenTextures(2, CombinedTextures);

			}

			unsigned short TextureOffsetDiffuseGlobal = 1;
			unsigned short TextureOffsetEmissiveGlobal = 2;

			std::cout << "Size: " << SimplifiedMaterials.size() << '\n';

			for (unsigned int i = 0; i < Scene->mNumMaterials; i++) {

				SimplifiedMaterial ThisMaterialSimplified; 

				//KernelMaterial KernelMat; 

				Material ThisMaterial;

				const aiMaterial* Material = Scene->mMaterials[i];

				aiColor3D Diffuse = aiColor3D(), Specular = aiColor3D();

				auto DiffuseResult = Material->Get(AI_MATKEY_COLOR_DIFFUSE, Diffuse);
				auto SpecularResult = Material->Get(AI_MATKEY_COLOR_SPECULAR, Specular);

				ThisMaterial.Albedo = Vector3f(1.0);
				ThisMaterial.BaseMetalness = 0.0;
				ThisMaterial.BaseRoughness = 0.0;

				if (DiffuseResult != aiReturn_FAILURE)
					ThisMaterial.Albedo = Vector3f(Diffuse.r, Diffuse.g, Diffuse.b);
				if (SpecularResult != aiReturn_FAILURE) {
					ThisMaterial.BaseRoughness = Specular.r;
					ThisMaterial.BaseMetalness = Specular.g;
					ThisMaterial.BaseEmissive = Specular.b; 
				}

				ThisMaterialSimplified = Vector4f(ThisMaterial.Albedo,ThisMaterial.BaseEmissive);

				aiTextureType loopElements[5] = { aiTextureType::aiTextureType_DIFFUSE,aiTextureType::aiTextureType_AMBIENT,aiTextureType::aiTextureType_DISPLACEMENT,aiTextureType::aiTextureType_SPECULAR, aiTextureType::aiTextureType_NORMALS };

				for (int Texture = 0; Texture < 5; Texture++) {
					if (Material->GetTextureCount(loopElements[Texture]) > 0) {

						aiString Path;

						if (Material->GetTexture(loopElements[Texture], 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
							std::string p(Path.data);

							std::string FullPath = p;

							sf::Image* Image = new sf::Image();

							ThisMaterial.Textures[Texture] = LoadTextureGLKeepSfImage(p, Image, TextureFormat[Texture]);

							if (ThisMaterial.Textures[Texture].Resolution.x == 0 || Texture != 0) {
								delete Image;
								Image = nullptr; 
							}
							if (Image && Texture == 0) {

								int Index = SFImages.size(); 

								SFImages.push_back(Image); 

								ThisMaterialSimplified.y = Index; 
								ThisMaterialSimplified.x = -1.0f; 

							}

						}

					}
				}

				Materials.push_back(ThisMaterial);
				SimplifiedMaterials.push_back(ThisMaterialSimplified); 
			}

			if (CombinedTextures != nullptr) {



				for (int TextureType = 0; TextureType < 2; TextureType++) {



					glBindTexture(GL_TEXTURE_2D_ARRAY, CombinedTextures[TextureType]);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D_ARRAY,
						GL_TEXTURE_MIN_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D_ARRAY,
						GL_TEXTURE_MAG_FILTER,
						GL_LINEAR);
					glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, 128, 128, Images[TextureType].size(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

					for (int SubImage = 0; SubImage < Images[TextureType].size(); SubImage++) {


						std::vector<unsigned char> ImageData = std::vector<unsigned char>(128 * 128 * 3);

						auto Pixels = Images[TextureType][SubImage]->getPixelsPtr();



						for (int x = 0; x < 128; x++) {
							for (int y = 0; y < 128; y++) {

								//grab the correct coords

								float TextureX = float(x) / 128.f;
								float TextureY = float(y) / 128.f;

								int TexelX = glm::clamp(int(TextureX * Images[TextureType][SubImage]->getSize().x), 0, (int)Images[TextureType][SubImage]->getSize().x);
								int TexelY = glm::clamp(int(TextureY * Images[TextureType][SubImage]->getSize().y), 0, (int)Images[TextureType][SubImage]->getSize().y);

								for (int i = 0; i < 3; i++) {
									ImageData[(y * 128 + x) * 3 + i] = Pixels[(TexelY * Images[TextureType][SubImage]->getSize().x + TexelX) * 4 + i];

								}



							}
						}

						glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
							0,
							0, 0, SubImage,
							128, 128, 1,
							GL_RGB,
							GL_UNSIGNED_BYTE,
							ImageData.data());

					}

					glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

					glBindTexture(GL_TEXTURE_2D_ARRAY, 0);




				}




			}

			/*for (int i = 0; i < 2; i++)
				for (auto Texture : Images[i])
					delete Texture;
					*/

		}

		void Model::PrepareForRendering() {


			//create temporary classes for the simplified model (i;e requires only one drawcall per mesh, instead of one per material) 


			

			int CurrentGlobalIndicie = 0;

			for (int MeshEntry = 0; MeshEntry < ModelData.MeshEntries.size(); MeshEntry++) {

				for (int Indicie = 0; Indicie < ModelData.MeshEntries[MeshEntry].NumIndices; Indicie++) {

					Vertices.push_back(ModelData.Vertices[ModelData.MeshEntries[MeshEntry].BaseIndex + Indicie]);
					TexCoords.push_back(ModelData.TexCoords[ModelData.MeshEntries[MeshEntry].BaseIndex + Indicie]);
					Normals.push_back(ModelData.Normals[ModelData.MeshEntries[MeshEntry].BaseIndex + Indicie]);
					Indicies.push_back(CurrentGlobalIndicie++);

				}


			}

			std::cout << "Vertices: " << Vertices.size() << '\n'; 

			VAOShadowVertCount = Vertices.size();

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(6, VBO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ModelData.Vertices[0]) * ModelData.Vertices.size(), &ModelData.Vertices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ModelData.TexCoords[0]) * ModelData.TexCoords.size(), &ModelData.TexCoords[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ModelDataLightMapUV.TexCoords[0]) * ModelDataLightMapUV.TexCoords.size(), &ModelDataLightMapUV.TexCoords[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ModelData.Normals[0]) * ModelData.Normals.size(), &ModelData.Normals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ModelData.Tangents[0]) * ModelData.Tangents.size(), &ModelData.Tangents[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);


			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[0]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ModelData.Indices[0]) * ModelData.Indices.size(), &ModelData.Indices[0], GL_STATIC_DRAW);



			glBindVertexArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


			glGenVertexArrays(1, &VAOShadow);
			glBindVertexArray(VAOShadow);
			glGenBuffers(4, VBOShadow);

			glBindBuffer(GL_ARRAY_BUFFER, VBOShadow[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBOShadow[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, VBOShadow[3]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);



			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOShadow[0]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indicies[0]) * Indicies.size(), &Indicies[0], GL_STATIC_DRAW);

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			glGenTextures(1, &MaterialDataTexture);

			glBindTexture(GL_TEXTURE_2D, MaterialDataTexture);



			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, Materials.size(), 1, 0, GL_RGB, GL_FLOAT, ConvertToImage(Materials).data());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


			glBindTexture(GL_TEXTURE_2D, 0);

		}

		void Model::Draw() {
			glBindVertexArray(VAOShadow);

			glDrawElements(GL_TRIANGLES, VAOShadowVertCount, GL_UNSIGNED_INT, nullptr);

			glBindVertexArray(0);
		}

		void Model::DrawInstanced(int InstanceCount)
		{
			glBindVertexArray(VAOShadow);

			glDrawElementsInstanced(GL_TRIANGLES, VAOShadowVertCount, GL_UNSIGNED_INT, nullptr, InstanceCount);

			glBindVertexArray(0);
		}

		void Model::DrawWithSimplifiedMaterialsInstanced(Shader& Shader, int BaseImage, int InstanceCount)
		{

			for (int i = 0; i < 2; i++) {
				glActiveTexture(GL_TEXTURE0 + BaseImage + i);
				glBindTexture(GL_TEXTURE_2D_ARRAY, CombinedTextures[i]);
			}

			glActiveTexture(GL_TEXTURE23);
			glBindTexture(GL_TEXTURE_2D, MaterialDataTexture);

			glBindVertexArray(VAOShadow);

			glDrawElementsInstanced(GL_TRIANGLES, VAOShadowVertCount, GL_UNSIGNED_INT, nullptr, InstanceCount);

			glBindVertexArray(0);


		}

		void Model::DrawWithMaterialsInstanced(const Shader& Shader, int BaseImage, int InstanceCount) const
		{
			const std::string HasNames[5] = { "HasAlbedo", "HasNormal", "HasRough", "HasMet", "HasEmissive" };

			glBindVertexArray(VAO);

			for (int i = 0; i < ModelData.MeshEntries.size(); i++) {

				unsigned int MaterialIndex = ModelData.MeshEntries[i].MaterialIndex;


				Shader.SetUniform("Albedo", Materials[MaterialIndex].Albedo);
				Shader.SetUniform("Metalness", Materials[MaterialIndex].BaseMetalness);
				Shader.SetUniform("Roughness", Materials[MaterialIndex].BaseRoughness);
				Shader.SetUniform("InputEmission", Materials[MaterialIndex].BaseEmissive);

				for (int i = 0; i < 5; i++) {
					if (Materials[MaterialIndex].Textures[i].Resolution.x != 0) {
						Materials[MaterialIndex].Textures[i].Bind(BaseImage + i);

					}
					Shader.SetUniform(HasNames[i], Materials[MaterialIndex].Textures[i].Resolution.x != 0);
				}

				glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
					ModelData.MeshEntries[i].NumIndices,
					GL_UNSIGNED_INT,
					(void*)(sizeof(unsigned int) * ModelData.MeshEntries[i].BaseIndex),
					InstanceCount,
					ModelData.MeshEntries[i].BaseVertex);



			}

			glBindVertexArray(0);
		}

		void Model::DrawWithMaterials(Shader& Shader, int BaseImage)
		{

			const std::string HasNames[5] = { "HasAlbedo", "HasNormal", "HasRough", "HasMet", "HasEmissive" };

			glBindVertexArray(VAO);

			for (int i = 0; i < ModelData.MeshEntries.size(); i++) {

				unsigned int MaterialIndex = ModelData.MeshEntries[i].MaterialIndex;


				Shader.SetUniform("Albedo", Materials[MaterialIndex].Albedo);
				Shader.SetUniform("Metalness", Materials[MaterialIndex].BaseMetalness);
				Shader.SetUniform("Roughness", Materials[MaterialIndex].BaseRoughness);

				for (int i = 0; i < 5; i++) {
					if (Materials[MaterialIndex].Textures[i].Resolution.x != 0) {
						Materials[MaterialIndex].Textures[i].Bind(BaseImage + i);

					}
					Shader.SetUniform(HasNames[i], Materials[MaterialIndex].Textures[i].Resolution.x != 0);
				}

				glDrawElementsBaseVertex(GL_TRIANGLES,
					ModelData.MeshEntries[i].NumIndices,
					GL_UNSIGNED_INT,
					(void*)(sizeof(unsigned int) * ModelData.MeshEntries[i].BaseIndex),
					ModelData.MeshEntries[i].BaseVertex);



			}

			glBindVertexArray(0);

		}

	}

}
