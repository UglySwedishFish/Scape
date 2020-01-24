#include "Pipeline.h"

namespace Scape {




	void Pipeline::PreparePipeline(Camera& Camera, Window& Window)
	{

		glClearColor(0.0, 0.0, 0.0, 0.0);
		Core::PrepareHaltonSequence();
		PreparePostProcess();
		World.LightBaker.PrepareLightBakingSystem(Sky);
		World.PrepareWorldManager(Window);
		Sky.PrepareSkyRenderer(Window); 
		Direct.PrepareDirectLighting(Window); 

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); 

	}

	void Pipeline::RunPipeline(Camera& Camera, Window& Window)
	{
		bool Running = true;
		sf::Event Event;
		sf::Clock GameClock;
		int Frame = 0;
		float T = 0.;

		bool Lines = false;

		sf::Clock FrameRateCounter;

		int Frames = 0;

		float TimeOfDay = 0.0; 

		while (Running) {

			while (Window.GetRawWindow()->pollEvent(Event)) {
				switch (Event.type) {
				case sf::Event::KeyPressed:
					switch (Event.key.code) {
					case sf::Keyboard::Escape:
						return;
						break;

					}
				}
			}

			if (FrameRateCounter.getElapsedTime().asSeconds() >= 1.0) {

				std::cout << Frames << '\n';
				FrameRateCounter.restart();
				Frames = 0;


			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				TimeOfDay += 36000. * Window.GetFrameTime(); 
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				TimeOfDay -= 36000. * Window.GetFrameTime();

			TimeOfDay = glm::clamp(TimeOfDay, 0.0f, 86400.f / 2.f); 

			Sky.SetTimeOfDay(TimeOfDay); 
			World.SetSunDetail(Vector4f(Sky.SunColor, TimeOfDay)); 
			World.HandleWorldGeneration(Camera); 


			if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
				World.LightBaker.UpdateLightBaking(Sky,World);

			Frames++;
			Window.SetFrameTime(GameClock.getElapsedTime().asSeconds());
			GameClock.restart();
			T += Window.GetFrameTime();
			Frame++;
			Window.SetTimeOpened(T);
			Window.SetFrameCount(Frame);


			Camera.HandleInput(Window, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ? 10.f : 1.0f, 0.15f, true, sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle));

			Camera.PrevView = Camera.View;
			Camera.View = Core::ViewMatrix(Camera.Position, Camera.Rotation);
			Camera.PrevProject = Camera.Project;
			Camera.Project = Camera.RawProject;

			Sky.RenderSky(Window, Camera, World); 

			glEnable(GL_DEPTH_TEST); 

			World.DeferredFBO.Bind(); 
			World.RenderWorld(Camera, Sky.SkyCube); 
			World.DeferredFBO.UnBind();
			Direct.RenderDirectLighting(Window, Camera, World, Sky); 

			glFinish();


			Window.GetRawWindow()->display(); 

			//std::cout << "Frametime: " << Window.GetFrameTime() * 1000. << " (ms)\n"; 

		}




	}

}
