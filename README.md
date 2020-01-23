# Scape
![image](https://github.com/UglySwedishFish/Scape/blob/master/Scape/Media/Thumbnail.png?raw=true)

Scape is a (currently early prototype) landscape rendering engine that is designed to render very realistic landscape-y environments in real-time. 
Think of it kind of like proland, except its more focused on the rendering from the ground and up (rather than the other way around) 

# What is implemented right now? 

Because of the incredibly early stage of the engine, there isn't that much that is implemented. But still, there are a few things 

* **Chunk system with procedurally placed entities, utilizing instancing for the highest possible performance** 
* **Very fast GPU accelerated monte-carlo light baker for hemispherical lighting and ambient occlusion.**
* **Atmospheric sky rendering** 
* **Basic direct lighting + global illumination for direct lighting (1 bounce)**

# What are the future plans? 

* **Implemented support for physically-correct directional lighting (with contact hardening)** 
* **Implement meshing for terrain, as well as approximation for terrain tracing for the path-tracer** 
* **Implement support for 1 bounce GI from directional light source** 
* **Shift lightmapping to O(log N) solution for lightbaking as well as implement 5+ bounce GI for both hemispherical and directional light source**
* **Support for multiple chunks** 
* **Volumetric height fog** 
* **Support for screen-space GI + AO for more dynamic objects as well as camera-centric cubemap solution for indirect specular lighting**
* **Support for baked radiance probes for chunks for indirect specular lighting fallback as well as indirect diffuse fallback for more dynamic objects**
* **Support for more complicated materials such as skin (with approximatory subsurface scattering), glass (with proper refraction) and leaves (with translucency)**
* **Basic skeletal animation support** 
* **Basic networking support. Let people explore together!**
* **Look into potential of using lightfields for microgeometry such as grass** 
* **Decal support for smaller geometry such as branches or leaves that have falled on the ground** 
* **Add support for multiple biomes** 
* **Tesselation / POM support for high quality displacement mapping** 
* **Entirely dynamic weather system with rain and snow (with procedural coverage for both of these)**
* **Area, IES, Point and Spot light sources** 
* **Very realistic ocean rendering with physically accurate water volume** 
* **TAA** 
* **Implement system to allow for more interesting ways of exploration! Skiing, biking, travelling by boat!**

# More media:

![image](https://github.com/UglySwedishFish/Scape/blob/master/Scape/Media/Comparasion.png?raw=true)

**No lighting vs baked lighting**
