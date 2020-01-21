#pragma once
#include <Window.h>
#include <CL/cl.hpp> 
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

namespace Scape {
	namespace Rendering {

		struct KernelGlobals {
			cl_context_properties* Properties;
			cl::Platform Platform;
			cl::Context Context; //OpenCL platform

			bool Created = false;

			cl::Device Device;
			KernelGlobals(cl::Platform Platform, cl::Device Device);

			inline KernelGlobals() {}
		};

		class Queue {
			cl_command_queue CommandQueues;
		public:
			inline cl_command_queue& Get(const unsigned char& Index) {
				return CommandQueues;
			}
			void Create(int Index, KernelGlobals& Globals);

			void Finish();

		};

		struct KernelEvent {
			cl_event Event;
			KernelEvent(cl_event& event) : Event(event) {

			}
			void Wait() {
				int Error = clWaitForEvents(1, &Event);
				if (Error != CL_SUCCESS) {
					std::cout << "Error in clWaitForEvents! (Error code: " << Error << ")\n";
					std::cin.get();
				}
			}
		};

		struct Kernel {
			cl::Kernel BaseKernel;
			cl::Program BaseProgram;

			template<typename T>
			void SetArgument(unsigned char Index, T Data) {
				auto error = clSetKernelArg(BaseKernel(), Index, sizeof(T), &Data);




				if (error != CL_SUCCESS) {
					std::cout << "Failed to set argument [setArg] (eror code: " << error << ")" << std::endl;


				}
			}

			void SetArgumentMemory(unsigned char Index, cl_mem Memory);

			void SetArgumentEmpty(unsigned char Index, unsigned int Size);


			void LoadKernel(const std::string& Name, KernelGlobals& GlobalInfo, const std::string& KernelMain, bool StrictAliasing = true, bool EnableMad = true, bool NoSignedZeros = true, bool FastMath = true);
			KernelEvent RunKernel(cl_command_queue& Queue, size_t GlobalWorkSize, size_t LocalWorkSize, bool Uniform = true);

			KernelEvent RunKernel2D(cl_command_queue& Queue, std::vector<size_t> GlobalWorkSize, std::vector<size_t> LocalWorkSize);

			void FlushQueue(cl::CommandQueue& Queue) {
				Queue.finish();
			}



		};

		struct KernelInterop2DImageList {
			std::vector<unsigned int> glImageList;
			std::vector<cl_mem> clImageMemoryList;

			unsigned int CreateGLImage(cl::Context& Context, cl_int2 Resolution, bool HighPrecision = false, bool Alpha = true, bool Nearest = false, cl_mem_flags flags = CL_MEM_READ_WRITE);

			void AssignGLImage(unsigned char Index, unsigned int Texture, cl::Context& Context, cl_mem_flags flags = CL_MEM_READ_WRITE);

			void UpdateMemoryList(unsigned char Min, unsigned char Max) {
				//for (int Index = Min; Index < min(Max, unsigned char(clImageMemoryList.size())); Index++)
					//clImageMemoryList[Index] = clImageList[Index]; 
			}

			void SetArgument(Kernel& Kernel, unsigned char ArgumentIndex, unsigned char Index) {
				Kernel.BaseKernel.setArg(ArgumentIndex, clImageMemoryList[Index]);
			}

			void ReserveImageList(unsigned char Size) {
				//clImageList.resize(Size); 
				glImageList.resize(Size);
				clImageMemoryList.resize(Size);
			}

			void QueueAcquire(cl_command_queue& Queue) {
				int Error = clEnqueueAcquireGLObjects(Queue, clImageMemoryList.size(), &clImageMemoryList[0], 0, nullptr, nullptr);
				clFinish(Queue);
				if (Error != CL_SUCCESS) {
					std::cout << "Failed to acquire GL objects (error code: " << Error << ")\n";
					std::cin.get();
				}

			}

			void QueueRelease(cl_command_queue& Queue) {

				int Error = clEnqueueReleaseGLObjects(Queue, clImageMemoryList.size(), &clImageMemoryList[0], 0, nullptr, nullptr);
				clFinish(Queue);

				if (Error != CL_SUCCESS) {
					std::cout << "Failed to release GL objects (error code: " << Error << ")\n";
					std::cin.get();
				}

			}



		};

		struct KernelBufferData {
			std::vector<size_t> Sizes;
			std::vector<cl::Buffer> Data;
			template<typename T>
			void CreateData(cl::Context& Context, size_t Size, T* Data, cl_mem_flags flags = CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR) {
				int Error;
				this->Data.push_back(cl::Buffer(Context, flags, Size * sizeof(T), Data, &Error));

				if (Error != CL_SUCCESS) {
					std::cout << "Fatal error in cl::Buffer creation, whatever you do; do NOT press enter. Close the application! (Error code: " << Error << ")\n";
					std::cin.get();
				}
				Sizes.push_back(Size);

			}
			template<typename T>
			void AssignData(unsigned char Index, cl::Context& Context, size_t Size, T* Data, cl_mem_flags flags = CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR) {
				int Error;
				this->Data[Index] = cl::Buffer(Context, flags, Size * sizeof(T), Data, &Error);
				Sizes[Index] = Size;
				if (Error != CL_SUCCESS) {
					std::cout << "Fatal error in cl::Buffer creation, whatever you do; do NOT press enter. Close the application! (Error code: " << Error << ")\n";
					std::cin.get();
				}
			}
			template<typename T>
			void MapData(unsigned char Index, cl::CommandQueue& Queue, std::size_t offset, std::size_t size, T** Data, cl_mem_flags flags = CL_MEM_READ_WRITE) {
				int Error;
				*Data = (T*)Queue.enqueueMapBuffer(this->Data[Index], false, flags, sizeof(T) * offset, sizeof(T) * size, nullptr, nullptr, &Error);
				if (Error != CL_SUCCESS) {
					std::cout << "Fatal error in buffer MapData, whatever you do; do NOT press enter. Close the application! (Error code: " << Error << ")\n";
					std::cin.get();
				}
			}
			template<typename T>
			void UnMapData(unsigned char Index, cl::CommandQueue& Queue, T* Data) {
				int Error;
				Error = Queue.enqueueUnmapMemObject(static_cast<cl::Memory>(this->Data[Index]), Data);
				if (Error != CL_SUCCESS) {
					std::cout << "Fatal error in buffer UnMapData, whatever you do; do NOT press enter. Close the application! (Error code: " << Error << ")\n";
					std::cin.get();
				}
			}
			template<typename T>
			void FillDeviceBuffer(unsigned char Index, cl::CommandQueue& Queue, T& Value, size_t ElementCount) {
				int Error;
				Error = clEnqueueFillBuffer(Queue(), Data[Index](), &Value, sizeof(T), 0, sizeof(T) * ElementCount, 0, nullptr, nullptr);
				if (Error != CL_SUCCESS) {
					std::cout << "Fatal error in buffer FillDeviceBuffer, whatever you do; do NOT press enter. Close the application! (Error code: " << Error << ")\n";
					std::cin.get();
				}
			}
			template<typename T>
			void AssignOrCreate(unsigned char Index, cl::Context& Context, size_t Size, T* Data, cl_mem_flags flags = CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR) {
				if (Index > this->Data.size())
					return;
				if (Index == this->Data.size())
					CreateData(Context, Size, Data, flags);
				else
					AssignData(Index, Context, Size, Data, flags);
			}



			void SetArgument(Kernel& Kernel, unsigned char ArgumentIndex, unsigned char Index) {
				Kernel.BaseKernel.setArg(ArgumentIndex, Data[Index]);
			}

			size_t GetSize(unsigned char Index) {
				if (Index >= Sizes.size())
					return 0;
				else
					return Sizes[Index];
			}

		};

		template<typename T> struct KernelBuffer {
			cl_mem Memory;
			std::size_t Size;

			KernelBuffer(cl_mem Memory = NULL, std::size_t Size = 0) : Memory(Memory), Size(Size) {

			}


			KernelEvent WriteDeviceBuffer(Queue& Queue, T const* Data, int Size = -1) {

				if (Size < 0)
					Size = this->Size;

				cl_event Event;

				int Error = clEnqueueWriteBuffer(Queue.Get(0), this->Memory, false, 0, sizeof(T) * Size, Data, 0, nullptr, &Event);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clEnqueueWriteBuffer (error code: " << Error << ")\n";
				}

				return KernelEvent(Event);

			}
			KernelEvent WriteDeviceBuffer(Queue& Queue, T const* Data, size_t Offset, size_t Size) {

				if (Size < 0)
					Size = this->Size;

				cl_event Event;

				int Error = clEnqueueWriteBuffer(Queue.Get(0), this->Memory, false, sizeof(T) * Offset, sizeof(T) * Size, Data, 0, nullptr, &Event);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clEnqueueWriteBuffer (error code: " << Error << ")\n";
				}

				return KernelEvent(Event);

			}
			KernelEvent FillDeviceBuffer(Queue& Queue, T const& Data, int Size = -1) {
				if (Size < 0)
					Size = this->Size;

				cl_event Event;

				int Error = clEnqueueFillBuffer(Queue.Get(0), this->Memory, &Data, sizeof(T), 0, sizeof(T) * Size, 0, nullptr, &Event);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clEnqueueFillBuffer (error code: " << Error << ")\n";
				}

				return KernelEvent(Event);

			}
			KernelEvent ReadDeviceBuffer(Queue& Queue, T* Data, int Size = -1) {

				if (Size < 0)
					Size = this->Size;

				cl_event Event;

				int Error = clEnqueueReadBuffer(Queue.Get(0), this->Memory, false, 0, sizeof(T) * Size, Data, 0, nullptr, &Event);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clEnqueueReadBuffer (error code: " << Error << ")\n";
				}

				return KernelEvent(Event);


			}
			KernelEvent ReadDeviceBuffer(Queue& Queue, T const* Data, size_t Offset, size_t Size) {
				if (Size < 0)
					Size = this->Size;

				cl_event Event;

				int Error = clEnqueueReadBuffer(Queue.Get(0), this->Memory, false, sizeof(T) * Offset, sizeof(T) * Size, Data, 0, nullptr, &Event);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clEnqueueReadBuffer (error code: " << Error << ")\n";
				}

				return KernelEvent(Event);

			}
			KernelEvent MapDeviceBuffer(Queue& Queue, cl_map_flags Flags, T** MappedData) {

				int Error = CL_SUCCESS;

				cl_event Event;

				T* Data = (T*)clEnqueueMapBuffer(Queue.Get(0), this->Memory, false, Flags, 0, sizeof(T) * Size, 0, nullptr, &Event, &Error);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clEnqueueMapBuffer (error code: " << Error << ")\n";
					std::cin.get();
				}

				*MappedData = Data;

				return KernelEvent(Event);

			}
			KernelEvent MapDeviceBuffer(Queue& Queue, cl_map_flags Flags, size_t Offset, size_t Size, T** MappedData) {

				int Error = CL_SUCCESS;

				cl_event Event;

				T* Data = (T*)clEnqueueMapBuffer(Queue.Get(0), this->Memory, false, Flags, sizeof(T) * Offset, sizeof(T) * Size, 0, nullptr, &Event, &Error);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clEnqueueMapBuffer (error code: " << Error << ")\n";
					std::cin.get();
				}


				*MappedData = Data;

				return KernelEvent(Event);

			}
			KernelEvent UnmapDeviceBuffer(Queue& Queue, T* MappedData) {

				cl_event Event;

				int Error = clEnqueueUnmapMemObject(Queue.Get(0), this->Memory, MappedData, 0, nullptr, &Event);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clEnqueueReadBuffer (error code: " << Error << ")\n";
				}

				return KernelEvent(Event);

			}

			static KernelBuffer<T> Create(KernelGlobals& GlobalKernelData, cl_mem_flags Flag, size_t Size) {
				int Error = CL_SUCCESS;
				cl_mem DeviceBuffer = clCreateBuffer(GlobalKernelData.Context(), Flag, Size * sizeof(T), nullptr, &Error);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clCreateBuffer (error code: " << Error << ")\n";
					std::cin.get();
					std::cin.get();
					std::cin.get();
					std::cin.get();
					std::cin.get();
					std::cin.get();
				}

				return KernelBuffer<T>(DeviceBuffer, Size);
			}
			static KernelBuffer<T> Create(KernelGlobals& GlobalKernelData, cl_mem_flags Flag, size_t Size, T* Data) {


				int Error = CL_SUCCESS;
				cl_mem DeviceBuffer = clCreateBuffer(GlobalKernelData.Context(), Flag | CL_MEM_COPY_HOST_PTR, Size * sizeof(T), (void*)Data, &Error);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clCreateBuffer (error code: " << Error << ")\n";
					std::cin.get();
					std::cin.get();
					std::cin.get();
					std::cin.get();
					std::cin.get();
					std::cin.get();
				}

				return KernelBuffer<T>(DeviceBuffer, Size);

			}
			static KernelBuffer<T> CreateFromOpenCLBuffer(cl_mem Memory) {
				size_t bufferSize = 0;
				int Error = clGetMemObjectInfo(Memory, CL_MEM_SIZE, sizeof(bufferSize), &bufferSize, nullptr);

				if (Error != CL_SUCCESS) {
					std::cout << "Error in clGetMemObjectInfo (error code: " << Error << ")\n";
				}

				return KernelBuffer<T>(Memory, bufferSize / sizeof(T));
			}


		};



	}
}