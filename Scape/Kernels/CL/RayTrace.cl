
/*************************************************************************
INCLUDES
**************************************************************************/

const sampler_t Sampler = CLK_ADDRESS_CLAMP_TO_EDGE; 

#define PI 3.14159265358979323846f
#define KERNEL __kernel
#define GLOBAL __global
#define INLINE __attribute__((always_inline))
#define HIT_MARKER 1
#define MISS_MARKER -1
#define INVALID_IDX -1

/*************************************************************************
EXTENSIONS
**************************************************************************/
#ifdef AMD_MEDIA_OPS
#pragma OPENCL EXTENSION cl_amd_media_ops2 : enable
#endif

/*************************************************************************
TYPES
**************************************************************************/

// Axis aligned bounding box
typedef struct
{
    float4 pmin;
    float4 pmax;
} bbox;

// Ray definition
typedef struct
{
    float4 o;
    float4 d;
    int2 extra;
    int2 actual;
} ray;

// Intersection definition
typedef struct
{
    float4 uvwt; 
	int Triangle, Material, Mesh, Padding; 
} Intersection;


/*************************************************************************
HELPER FUNCTIONS
**************************************************************************/
INLINE
int ray_get_mask(ray const* r)
{
    return r->extra.x;
}

INLINE
int ray_is_active(ray const* r)
{
    return r->extra.y;
}

INLINE
float ray_get_maxt(ray const* r)
{
    return r->o.w;
}

INLINE
float ray_get_time(ray const* r)
{
    return r->d.w;
}

/*************************************************************************
FUNCTIONS
**************************************************************************/
#ifndef APPLE
INLINE
float4 make_float4(float x, float y, float z, float w)
{
    float4 res;
    res.x = x;
    res.y = y;
    res.z = z;
    res.w = w;
    return res;
}
INLINE
float3 make_float3(float x, float y, float z)
{
    float3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}
INLINE
float2 make_float2(float x, float y)
{
    float2 res;
    res.x = x;
    res.y = y;
    return res;
}
INLINE
int2 make_int2(int x, int y)
{
    int2 res;
    res.x = x;
    res.y = y;
    return res;
}
INLINE
int3 make_int3(int x, int y, int z)
{
    int3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}
#endif

INLINE float min3(float a, float b, float c)
{
#ifdef AMD_MEDIA_OPS
    return amd_min3(a, b, c);
#else
    return min(min(a,b), c);
#endif
}

INLINE float max3(float a, float b, float c)
{
#ifdef AMD_MEDIA_OPS
    return amd_max3(a, b, c);
#else
    return max(max(a,b), c);
#endif
}


// Intersect ray against a triangle and return intersection interval value if it is in
// (0, t_max], return t_max otherwise.
INLINE
float fast_intersect_triangle(ray r, float3 v1, float3 v2, float3 v3, float t_max)
{
    float3 const e1 = v2 - v1;
    float3 const e2 = v3 - v1;
    float3 const s1 = cross(r.d.xyz, e2);
    float const invd = native_recip(dot(s1, e1));
    float3 const d = r.o.xyz - v1;
    float const b1 = dot(d, s1) * invd;
    float3 const s2 = cross(d, e1);
    float const b2 = dot(r.d.xyz, s2) * invd;
    float const temp = dot(e2, s2) * invd;

    if (b1 < 0.f || b1 > 1.f || b2 < 0.f || b1 + b2 > 1.f || temp < 0.f || temp > t_max)
    {
        return t_max;
    }
    else
    {
        return temp;
    }
}

float3 RayIntersectsTriangle(ray r, 
                           float3 v1, float3 v2, float3 v3,
                           float t_max)
{
    const float EPSILON = 0.0000001;
    float3 vertex0 = v1;
    float3 vertex1 = v2;  
    float3 vertex2 = v3;
    float3 edge1, edge2, h, s, q;
    float a,f,u,v;
    edge1  = vertex1 - vertex0;
    edge2  = vertex2 - vertex0;
    h = cross(r.d.xyz, edge2);
    a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return (float3)(0.0,0.0,t_max);;    // This ray is parallel to this triangle.
    f = 1.0/a;
    s = r.o.xyz - vertex0;
    u = f * (dot(s,h));
    if (u < 0.0 || u > 1.0)
        return (float3)(0.0,0.0,t_max);;
    q = cross(s,edge1);
    v = f * dot(r.d.xyz,q);
    if (v < 0.0 || u + v > 1.0)
        return (float3)(0.0,0.0,t_max);;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2, q);
    if (t > EPSILON && t < t_max) // ray intersection
    {
        return (float3)(u,v,t); 
    }
    else // This means that there is a line intersection but not a ray intersection.
        return (float3)(0.0,0.0,t_max);
}

INLINE
float3 safe_invdir(ray r)
{
#ifdef USE_SAFE_MATH
    float const dirx = r.d.x;
    float const diry = r.d.y;
    float const dirz = r.d.z;
    float const ooeps = exp2(-80.0f); // Avoid div by zero.
    float3 invdir;
    invdir.x = 1.0f / (fabs(dirx) > ooeps ? dirx : copysign(ooeps, dirx));
    invdir.y = 1.0f / (fabs(diry) > ooeps ? diry : copysign(ooeps, diry));
    invdir.z = 1.0f / (fabs(dirz) > ooeps ? dirz : copysign(ooeps, dirz));
    return invdir;
#else
    return native_recip(r.d.xyz);
#endif
}

// Intersect rays vs bbox and return intersection span. 
// Intersection criteria is ret.x <= ret.y
INLINE
float2 fast_intersect_bbox1(bbox box, float3 invdir, float3 oxinvdir, float t_max)
{
    float3 const f = mad(box.pmax.xyz, invdir, oxinvdir);
    float3 const n = mad(box.pmin.xyz, invdir, oxinvdir);
    float3 const tmax = max(f, n);
    float3 const tmin = min(f, n);
    float const t1 = min(min3(tmax.x, tmax.y, tmax.z), t_max);
    float const t0 = max(max3(tmin.x, tmin.y, tmin.z), 0.f);
    return make_float2(t0, t1);
}







// Given a point in triangle plane, calculate its barycentrics
INLINE
float2 triangle_calculate_barycentrics(float3 p, float3 v1, float3 v2, float3 v3)
{
    
    float3 const e1 = v2 - v1;
    float3 const e2 = v3 - v1;
    float3 const e = p - v1;
    float const d00 = dot(e1, e1);
    float const d01 = dot(e1, e2);
    float const d11 = dot(e2, e2);
    float const d20 = dot(e, e1);
    float const d21 = dot(e, e2);
    float const invdenom = native_recip(d00 * d11 - d01 * d01);
    float const b1 = (d11 * d20 - d01 * d21) * invdenom;
    float const b2 = (d00 * d21 - d01 * d20) * invdenom;
    return make_float2(b1, b2);
}


/*************************************************************************
TYPE DEFINITIONS
**************************************************************************/

#define LEAFNODE(x) (((x).child0) == -1)
#define GLOBAL_STACK_SIZE 32
#define SHORT_STACK_SIZE 16
#define WAVEFRONT_SIZE 64

// BVH node
typedef struct
{
    union 
    {
        

        struct
        {
            // If node is a leaf we keep vertex indices here
            int i0, i1, i2;
            // Address of a left child
            int child0;
            // Shape mask
            int shape_mask;
            // Shape ID
            int shape_id;
            // Primitive ID
            int prim_id;
            // Address of a right child
            int child1;

            int Padding[8]; 

        };

        struct
        {
            // Child bounds
            bbox bounds[2];
        };
    };

} bvh_node;

typedef struct {

    bbox bounds[2];

}smallnode; 

typedef struct KernelMesh {
	
	unsigned int MemoryOffsetTriangles; 
	unsigned int MemoryOffsetNodes; 
	unsigned int MemoryOffsetMaterials; 
	unsigned int Padding; 

}KernelMesh;

typedef struct KernelModel {

    float4 RotationMatrix1PositionX, RotationMatrix2PositionY, RotationMatrix3PositionZ; 

	unsigned int Mesh; //what mesh it contains 
	
	//might be required for the future, and I cant store it as a float3
	float ScaleX, ScaleY, ScaleZ; 
}KernelModel;

float4 MatrixVectorMultiplication(float4 Matrix1, float4 Matrix2, float4 Matrix3, float4 Matrix4, float4 Vector) {

	float Vector1 = Matrix1.x * Vector.x + Matrix1.y * Vector.y + Matrix1.z * Vector.z + Matrix1.w * Vector.w; 
	float Vector2 = Matrix2.x * Vector.x + Matrix2.y * Vector.y + Matrix2.z * Vector.z + Matrix2.w * Vector.w; 
	float Vector3 = Matrix3.x * Vector.x + Matrix3.y * Vector.y + Matrix3.z * Vector.z + Matrix3.w * Vector.w; 
	float Vector4 = Matrix4.x * Vector.x + Matrix4.y * Vector.y + Matrix4.z * Vector.z + Matrix4.w * Vector.w; 

	return (float4)(Vector1, Vector2, Vector3, Vector4); 
}



__attribute__((reqd_work_group_size(64, 1, 1)))
KERNEL void RayTrace(
    // Bvh nodes
    GLOBAL bvh_node * nodes,
    // Triangles vertices
    GLOBAL float4 const* restrict vertices,
    // Triangles UVs
    GLOBAL float2 const* restrict UVs,
    //Triangles normals (+ material index)
    GLOBAL float4 const* restrict Normals,
    // Number of rays in rays buffer
    int num_rays,
    
	GLOBAL KernelMesh * Meshes, 

	GLOBAL KernelModel * Models, 

    GLOBAL int* stack,
	
	int ModelCount,

    int width,

    //input / outpute image data: 
    __read_only image2d_t Direction, 
    __read_only image2d_t Origin, 
    __write_only image2d_t RayUVWM, //Texture Coordinate, Traversal, Material Index
    __write_only image2d_t RayNorm //Ray normal hit 
    
    )
{
    int global_id = get_global_id(0);
    int local_id = get_local_id(0);
    int group_id = get_group_id(0);
    int2 Coord = (int2)(global_id%width, global_id/width); 


    // Handle only working subset
    //if (global_id < num_rays)

    Intersection ThisHit; 


    // the ray which we are about to generate
    ray _r; 

    //Ray generation step
    {
        
        float4 DirectionSample = read_imagef(Direction, Sampler, Coord); 
        float4 OriginSample = read_imagef(Origin, Sampler, Coord); 

        
        _r.d.xyz = DirectionSample.xyz; 
        _r.o.xyz = OriginSample.xyz; 
        _r.o.w = 100000.f; 
        _r.d.w = 0.0f; 
        _r.extra = (int2)(0xFFFFFFFF); 
        _r.extra.x = 0; 

        if(DirectionSample.w < 0.0) {
            _r.extra.y = 0;
        }

    }

    


    //Traversal step
    if(true){

        


        ThisHit.Triangle = MISS_MARKER;
        ThisHit.uvwt = -1.f; 



		
        float t_max = 10000.f;

        

        if (ray_is_active(&_r))
        {

            __global int* gm_stack_base = stack + (group_id * WAVEFRONT_SIZE + local_id) * GLOBAL_STACK_SIZE;
            __global int* gm_stack = gm_stack_base;
            // Allocate stack in LDS
            __local int lds[SHORT_STACK_SIZE * WAVEFRONT_SIZE];
            int isect_idx = INVALID_IDX;

            
                      
			for(int mesh = 0; mesh < ModelCount; mesh++) {

            GLOBAL KernelModel * CurrentModel = Models + mesh; 


            // Current node address
            int addr = Meshes[CurrentModel->Mesh].MemoryOffsetNodes;

            int Safe = 0; 

			ray r = _r;

            float3 Position = (float3)(CurrentModel->RotationMatrix1PositionX.w,CurrentModel->RotationMatrix2PositionY.w,CurrentModel->RotationMatrix3PositionZ.w); 
            
			r.o.xyz -= Position; 

            float4 ModelMatrix1 = (float4)(CurrentModel->RotationMatrix1PositionX.xyz, 0.0); 
            float4 ModelMatrix2 = (float4)(CurrentModel->RotationMatrix2PositionY.xyz, 0.0); 
            float4 ModelMatrix3 = (float4)(CurrentModel->RotationMatrix3PositionZ.xyz, 0.0); 
            float4 ModelMatrix4 = (float4)(0.0,0.0,0.0,1.0); 

            //if(global_id == 0) {
                //printf("Position: %f %f %f\n", Position.x, Position.y, Position.z); 
          //  }


			//r.d.xyz = MatrixVectorMultiplication(ModelMatrix1,ModelMatrix2,ModelMatrix3,ModelMatrix4, (float4)(r.d.xyz,1.0)).xyz; 
			//r.o.xyz = MatrixVectorMultiplication(ModelMatrix1,ModelMatrix2,ModelMatrix3,ModelMatrix4, (float4)(r.o.xyz,1.0)).xyz; 

            //r.d.xyz = normalize(r.d.xyz); 


			// Precompute inverse direction and origin / dir for bbox testing
            float3 const invdir = safe_invdir(r);
            float3 const oxinvdir = -r.o.xyz * invdir;
            // Intersection parametric distance
            
            
            __local int* lm_stack_base = lds + local_id;
            __local int* lm_stack = lm_stack_base;

            *lm_stack = INVALID_IDX;
            lm_stack += WAVEFRONT_SIZE;

           while (addr != INVALID_IDX && Safe < 1000)
            {

                

                // Fetch next node
                bvh_node const node = nodes[addr];

                Safe++; 

                // Check if it is a leaf
                if (LEAFNODE(node))
                {

                    
                    

                    // Leafs directly store vertex indices
                    // so we load vertices directly
                    float3 const v1 = vertices[node.i0].xyz;
                    float3 const v2 = vertices[node.i1].xyz;
                    float3 const v3 = vertices[node.i2].xyz;
                    // Intersect triangle
                    float const f = fast_intersect_triangle(r, v1, v2, v3, t_max);
                    // If hit update closest hit distance and index
                    if (f < t_max)
                    {
                        t_max = f;
                        isect_idx = addr;
                    }
                }
                else
                {

                    if(global_id == 0) {
                        /*printf("Bounds: %f, %f, %f\n", node.bounds[1].pmin.x,node.bounds[1].pmin.y,node.bounds[1].pmin.z); 
                        printf("Bounds: %f, %f, %f\n", node.bounds[1].pmax.x,node.bounds[1].pmax.y,node.bounds[1].pmax.z); 
                        printf("Adress: %d\n", addr); 
                        printf("Ray direction: %f %f %f\n", r.d.x,r.d.y,r.d.z); 
                        printf("Ray Origin: %f %f %f\n", r.o.x,r.o.y,r.o.z); */
                    } 

                         

                    // It is internal node, so intersect vs both children bounds
                    float2 const s0 = fast_intersect_bbox1(node.bounds[0], invdir, oxinvdir, t_max);
                    float2 const s1 = fast_intersect_bbox1(node.bounds[1], invdir, oxinvdir, t_max);

                    // Determine which one to traverse
                    bool const traverse_c0 = (s0.x <= s0.y);
                    bool const traverse_c1 = (s1.x <= s1.y);
                    bool const c1first = traverse_c1 && (s0.x > s1.x);

                    


                    if (traverse_c0 || traverse_c1)
                    {
                        
                            
                        
                        

                        int deferred = -1;

                        // Determine which one to traverse first
                        if (c1first || !traverse_c0)
                        {
                            // Right one is closer or left one not travesed
                            addr = node.child1;
                            deferred = node.child0;
                        }
                        else
                        {
                            // Traverse left node otherwise
                            addr = node.child0;
                            deferred = node.child1;
                        }

                        // If we traverse both children we need to postpone the node
                        if (traverse_c0 && traverse_c1)
                        {
                            // If short stack is full, we offload it into global memory
                            if ( lm_stack - lm_stack_base >= SHORT_STACK_SIZE * WAVEFRONT_SIZE)
                            {
                                for (int i = 1; i < SHORT_STACK_SIZE; ++i)
                                {
                                    gm_stack[i] = lm_stack_base[i * WAVEFRONT_SIZE];
                                }

                                gm_stack += SHORT_STACK_SIZE;
                                lm_stack = lm_stack_base + WAVEFRONT_SIZE;
                            }

                            *lm_stack = deferred;
                            lm_stack += WAVEFRONT_SIZE;
                        }

                        // Continue traversal
                        continue;
                    }
                }

                // Try popping from local stack
                lm_stack -= WAVEFRONT_SIZE;
                addr = *(lm_stack);

                // If we popped INVALID_IDX then check global stack
                if (addr == INVALID_IDX && gm_stack > gm_stack_base)
                {
                    // Adjust stack pointer
                    gm_stack -= SHORT_STACK_SIZE;
                    // Copy data from global memory to LDS
                    for (int i = 1; i < SHORT_STACK_SIZE; ++i)
                    {
                        lm_stack_base[i * WAVEFRONT_SIZE] = gm_stack[i];
                    }
                    // Point local stack pointer to the end
                    lm_stack = lm_stack_base + (SHORT_STACK_SIZE - 1) * WAVEFRONT_SIZE;
                    addr = lm_stack_base[WAVEFRONT_SIZE * (SHORT_STACK_SIZE - 1)];
                }
            }

             if (isect_idx != INVALID_IDX)
            {
                // Fetch the node & vertices
                bvh_node const node = nodes[isect_idx];
                float3 const v1 = vertices[node.i0].xyz;
                float3 const v2 = vertices[node.i1].xyz;
                float3 const v3 = vertices[node.i2].xyz;
                // Calculate hit position
                float3 const p = r.o.xyz + r.d.xyz * t_max;
                // Calculte barycentric coordinates
                float2 const uv = triangle_calculate_barycentrics(p, v1, v2, v3);
                // Update hit information
                ThisHit.Triangle = node.i0;
                ThisHit.uvwt = make_float4(uv.x, uv.y, 0.f, t_max);
            }
            else
            {
                // Miss here
                ThisHit.Triangle = MISS_MARKER;
                ThisHit.uvwt.w = -1.0; 
            }

			}
           
        }
    }
    //*/
    //handle hit + store 

    {
        
        //Texture Coordinate, Traversal, Material Index
        //Ray normal hit 

        float4 Pixel1 = (float4)(0.0,0.0,-1.0,0.0);
        float4 Pixel2 = (float4)(0.0,1.0,0.0,0.0); 

        if(ThisHit.Triangle >= 0) {

            int const i0 = ThisHit.Triangle;
		    int const i1 = ThisHit.Triangle + 1;
		    int const i2 = ThisHit.Triangle + 2;

            float2 const uv0 = UVs[i0]; 
		    float2 const uv1 = UVs[i1]; 
		    float2 const uv2 = UVs[i2]; 

            float4 const n0 = Normals[i0]; 
            float4 const n1 = Normals[i1]; 
            float4 const n2 = Normals[i2]; 

            float2 BarycentricCoordinate = ThisHit.uvwt.xy; 

            float2 const TextureCoordinate = (1.f - BarycentricCoordinate.x - BarycentricCoordinate.y) * uv0 + BarycentricCoordinate.x * uv1 + BarycentricCoordinate.y * uv2; 
            float3 const Normal = normalize((1.f - BarycentricCoordinate.x - BarycentricCoordinate.y) * n0.xyz + BarycentricCoordinate.x * n1.xyz + BarycentricCoordinate.y * n2.xyz); 

            Pixel1 = (float4)(TextureCoordinate, ThisHit.uvwt.w, n0.w); 
            Pixel2 = (float4)(Normal, 0.0); 

        }  
        write_imagef(RayUVWM, Coord, Pixel1); 
        write_imagef(RayNorm, Coord, Pixel2); 



    }

}
