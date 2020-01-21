#include "Kernel.h"

namespace Scape {
	namespace Rendering {
		KernelGlobals::KernelGlobals(cl::Platform Platform, cl::Device Device) : Context(), Platform(Platform), Device(Device), Properties(new cl_context_properties[7]{ CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
				CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
				CL_CONTEXT_PLATFORM, (cl_context_properties)Platform(),
				0 }) {
			int Error;
			this->Context = cl::Context(this->Device, Properties, 0, 0, &Error);
			if (Error != CL_SUCCESS) {
				std::cout << "Failed to create opencl context (error code: " << Error << ")\n";
			}
			else {
				std::cout << "OpenCL context created succesfully!\n";
			}
			Created = true;
		};
		void Queue::Create(int Index, KernelGlobals& Globals) {
			int Error;
			std::cout << "Queue index: " << Index << '\n';

			if (!Globals.Created) {
				std::cout << "Failed to create queue (error: global kernel data not initialized)\n";
				std::cin.get();
			}

			CommandQueues = clCreateCommandQueue(Globals.Context(), Globals.Device(), 0Ui64, &Error);


			if (Error != CL_SUCCESS) {
				std::cout << "Failed to create queue (error code: " << Error << ")\n";
				std::cin.get();
			}
		}
		void Queue::Finish() {
			int Error = clFinish(CommandQueues);
			if (Error != CL_SUCCESS) {

				std::cout << "Failed to flush queue (error code: " << Error << ")\n";
				std::cin.get();
			}
		}



		void Kernel::SetArgumentMemory(unsigned char Index, cl_mem Memory) {
			int Error = clSetKernelArg(BaseKernel(), Index, sizeof(cl_mem), &Memory);
			if (Error != CL_SUCCESS) {
				std::cout << "Failed to set argument [csSetKernelArg] (eror code: " << Error << ")" << std::endl;
				std::cin.get();
			}
		}
		void Kernel::SetArgumentEmpty(unsigned char Index, unsigned int Size) {
			int Error = clSetKernelArg(BaseKernel(), Index, Size, nullptr);
			if (Error != CL_SUCCESS) {
				std::cout << "Failed to set argument [clSetKernelArgEmpty] (eror code: " << Error << ")" << std::endl;
				std::cin.get();
			}
		}
		void Kernel::LoadKernel(const std::string& Name, KernelGlobals& GlobalInfo, const std::string& KernelMain, bool StrictAliasing, bool EnableMad, bool NoSignedZeros, bool FastMath)
		{

			if (!GlobalInfo.Created) {
				std::cout << "Failed to create queue (error: global kernel data not initialized)\n";
				std::cin.get();
			}

			std::cout << "Loading kernel: " << Name << "\n (Kernel function name: " << KernelMain << ")\n";

			std::ifstream File(Name);
			std::string Source = "";

			if (!File) {
				std::cout << "Failed to load path tracing kernel: " << Name << " \n";
				std::cin.get();
				return;
			}

			while (!File.eof()) {
				char Line[500];
				File.getline(Line, 500);
				Source += std::string(Line) + "\n";
			}

			const char* CSource = Source.c_str();

			int Error = CL_SUCCESS;

			BaseProgram = cl::Program(GlobalInfo.Context, CSource, &Error);

			if (Error != CL_SUCCESS) {
				std::cout << "Failed to create program (error code: " << Error << ")\n";
			}

			std::string CompilerInput = "-I .";
			if (StrictAliasing)
				CompilerInput += " -cl-strict-aliasing";
			if (EnableMad)
				CompilerInput += " -cl-mad-enable";
			if (NoSignedZeros)
				CompilerInput += " -cl-no-signed-zeros";
			if (FastMath)
				CompilerInput += " -cl-fast-relaxed-math";




			int Result = BaseProgram.build({ GlobalInfo.Device }, CompilerInput.c_str());

			if (Result == CL_BUILD_PROGRAM_FAILURE) {
				std::string BuildLog = BaseProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(GlobalInfo.Device);
				std::cout << "Build log: " << '\n' << BuildLog << '\n';
			}
			else if (Result != CL_SUCCESS) {
				std::cout << "Internal OpenCL compiler error (error code:  " << Result << ")\n";
			}

			BaseKernel = cl::Kernel(BaseProgram, KernelMain.c_str(), &Error);

			if (Error != CL_SUCCESS) {
				std::cout << "Failed to create kernel (error code: " << Error << ")\n";
			}

		}

		KernelEvent Kernel::RunKernel(cl_command_queue& Queue, size_t GlobalWorkSize, size_t LocalWorkSize, bool Uniform) {
			cl_event Event;



			auto error = clEnqueueNDRangeKernel(Queue, BaseKernel(), 1, NULL, &GlobalWorkSize, &LocalWorkSize, 0, nullptr, &Event);
			if (error != CL_SUCCESS) {

				std::cout << "Failed to launch kernel (eror code: " << error << ")" << std::endl;
				std::cin.get();
			}
			return KernelEvent(Event);
		}

		KernelEvent Kernel::RunKernel2D(cl_command_queue& Queue, std::vector<size_t> GlobalWorkSize, std::vector<size_t> LocalWorkSize) {
			cl_event Event;
			auto error = clEnqueueNDRangeKernel(Queue, BaseKernel(), 2, NULL, &GlobalWorkSize[0], &LocalWorkSize[0], 0, nullptr, &Event);
			if (error != CL_SUCCESS) {
				std::cout << "Failed to launch kernel (eror code: " << error << ")" << std::endl;
				std::cin.get();
			}
			return KernelEvent(Event);
		}

		unsigned int KernelInterop2DImageList::CreateGLImage(cl::Context& Context, cl_int2 Resolution, bool HighPrecision, bool Alpha, bool Nearest, cl_mem_flags flags) {
			//create an empty image

			unsigned int ID = glImageList.size();

			glImageList.push_back(0);
			auto& Image = glImageList[ID];

			glGenTextures(1, &Image);
			glBindTexture(GL_TEXTURE_2D, Image);
			//create an empty texture
			glTexImage2D(GL_TEXTURE_2D, 0, HighPrecision ? GL_RGBA32F : GL_RGBA16F, Resolution.x, Resolution.y, 0, GL_RGBA, GL_FLOAT, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Nearest ? GL_NEAREST : GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Nearest ? GL_NEAREST : GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glBindTexture(GL_TEXTURE_2D, 0);


			//glFinish();

			int Error = CL_SUCCESS;



			clImageMemoryList.push_back(clCreateFromGLTexture(Context(), flags, GL_TEXTURE_2D, 0, Image, &Error));

			if (Error != CL_SUCCESS) {
				std::cout << "GL Image object: " << Image << '\n';
				std::cout << "Fatal error in cl::ImageGL creation, whatever you do; do NOT press enter. Close the application! (Error code: " << Error << ")\n";
				std::cin.get();
			}



			return Image;

		}

		void KernelInterop2DImageList::AssignGLImage(unsigned char Index, unsigned int Texture, cl::Context& Context, cl_mem_flags flags) {
			glImageList[Index] = Texture;
			int Error;
			clImageMemoryList[Index] = clCreateFromGLTexture(Context(), flags, GL_TEXTURE_2D, 0, Texture, &Error);

			if (Error != CL_SUCCESS) {
				std::cout << "Fatal error in cl::ImageGL creation, whatever you do; do NOT press enter. Close the application! (Error code: " << Error << "), index: " << int(Index) << '\n';
				std::cin.get();
				std::cin.get();
				std::cin.get();
				std::cin.get();

			}

			//clImageMemoryList[Index] = clImageList[Index]; 
		}
	}
}