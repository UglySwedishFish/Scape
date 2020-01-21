//idea: 
//rendering hyperrealistic terrain WITHOUT utilizing any form of geometry-bound raytracing. Thus, we use only more traditional techniques! 
//what can we utilize? 
//start with the terrain itself. utilize techniques similar to the ones used in the previous engine. draw as few vertices as possible! 
//instead of previous method - we utilize LOD and greedy meshing to reduce amount of vertices called to a minimum 
//for AO, we utilize a mixture of pre-baked AO (for terrain) mixed with other techniques (screen-space + cubemap-space RTAO) 
//utilize compact LODs with instancing to remove overhead when rendering radiance probes. batch the computation of them into one drawcall per chunk. 
//any fragment may utilize up to 4 probes at the same time. 
//(perhaps) utilize hemispherical shadowing solution for hemispherical lighting. (albeit unlikely) 

//for reflections the proposed solution (which has also been proven in the past) is to utilize camera-centric cubemap 

//nevermind with that. we need a superfast GI baker. each entity can have either 64x64, 32x32,16x16 or 8x8 resolution lightmaps. terrain (i;e chunk itself) has 128x128 resolution lightmap. Progressively update chunk until EACH lightmap (there are 4 in total) is at 16 samples per pixel (focus on lightmap that would currently be used). When lightmap is being updated -> stop updating of shadow cascades 

//larger grass -> 8x8, trees / larger rocks -> 64x64, medium sized rocks / bushes -> 32x32, smaller rocks -> 16x16, terrain -> 128 x 128 
//smallest grass (not sure how that will be rendered) will share the lightmap of the terrain 

//STICK TO OPENGL 3.3

#include "Pipeline.h"

int main() {
	using namespace Scape; 
	
	Window Screen = Window(Vector2i(1920, 1080), false);
	Camera Camera = Scape::Camera(45.0f, 0.01, 250., Vector3f(0.0, 0.0, 0.0), Vector3f(0.), Screen);

	Pipeline Engine;

	Engine.PreparePipeline(Camera, Screen);
	Engine.RunPipeline(Camera, Screen);
}