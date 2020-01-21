/*
This file contains structs shared between both c++
and opencl, and thus some things have to be done in order to support both
hence the #ifdef __cplusplus that you will find
throughout the code
*/


#ifdef __cplusplus
#pragma once
#include <Core.h>
#include "CL/cl.hpp"

using float4 = cl_float4;
using float3 = cl_float3;
using float2 = cl_float2;

using int4 = cl_int4;
using int3 = cl_int3;
using int2 = cl_int2;


#endif

#define LEAF_TERMINATOR 0x8000000

typedef struct PathState {
	float4 throughput;
	int volume;
	int flags;
	int extra0;
	int extra1;
}PathState;

typedef struct BBox {
	float4 Min, Max;
}BBox;


typedef struct Vertice {
#ifdef __cplusplus
	Vertice() : VertexUVx{ 0.f }, NormalUVy{ 0. }, Material(0), Indicie(0){}
#endif
	float4 VertexUVx, NormalUVy;
	int Material;
	int Indicie;
	bool EndOfTriangle;
	char Padding[7];
}Vertice;

typedef struct Ray {
	float4 Origin;
	float4 Direction;
	int2 Extra;
	int2 actual;
}Ray;

typedef struct KernelTexture {
	int Width, Height;
	unsigned int Offset;
	unsigned short Format, Mips;

}KernelTexture;

typedef struct KernelMaterial {
	float Red, Green, Blue; 
	int Texture; 
}KernelMaterial;


#define NO_TEXTURE -1



typedef struct Material {
	float4 AlbedoColor; //w component stores metalness  
	short AlbedoMap, NormalMap;
	bool HasAlbedo, HasNormal, HasRoughness, HasMetalness;
	unsigned short Emmisive, Padding;
	float Roughness;
#ifdef __cplusplus
	Material() : AlbedoColor{ 1.f }, AlbedoMap(-1), NormalMap(-1), HasAlbedo(false), HasNormal(false), HasRoughness(false), HasMetalness(false) {}
#endif
}Material;



typedef struct Intersection {
#ifdef __cplusplus
	Intersection() : UVWT{ 10000.f }, Triangle(-1), Material(0), Mesh(0) {}
#endif
	float4 UVWT;
	int Triangle, Material, Mesh, Padding;
}Intersection;




typedef struct KernelMesh {

	unsigned int MemoryOffsetTriangles;
	unsigned int MemoryOffsetNodes;
	unsigned int MemoryOffsetMaterials;
	unsigned int Padding;

#ifdef __cplusplus
	KernelMesh(unsigned int MOT, unsigned int MON, unsigned int MOM) :
		MemoryOffsetTriangles(MOT), MemoryOffsetNodes(MON), MemoryOffsetMaterials(MOM), Padding(0) {}
#endif

}KernelMesh;



typedef struct KernelModel {

	float4 RotationMatrix1PositionX, RotationMatrix2PositionY, RotationMatrix3PositionZ;

	unsigned int Mesh; //what mesh it contains 

	//might be required for the future, and I cant store it as a float3
	float ScaleX, ScaleY, ScaleZ;
}KernelModel;

typedef struct SoundData {

	float SendGains[4]; 
	float ReflectivityRatios[4]; 
	float SharedAirSpace; 
	float Padding[3]; 

}SoundData;



typedef struct PostIntersection {

#ifdef __cplusplus
	PostIntersection() : WorldPosition{ -1. }, Normal{ -1. }, Indirect{ -1. }, ResultingColor{ -1. } {}
#endif
	float4 WorldPosition; //in alpha component - store X uv coordinate 
	float4 Normal; //in alpha component - store y uv coordinate
	float4 Indirect; //in alpha component - store roughness
	float4 ResultingColor; //in alpha component - store metalness
	float4 Albedo; //not quite sure what I want in alpha component	
}PostIntersection;

