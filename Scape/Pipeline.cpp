#include "Pipeline.h"

namespace Scape {




	void Pipeline::PreparePipeline(Camera& Camera, Window& Window)
	{

		glClearColor(0.0, 0.0, 0.0, 0.0);
		Core::PrepareHaltonSequence();
		PreparePostProcess();
		World.PrepareWorldManager(Window);
		Sky.PrepareSkyRenderer(Window); 

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
			Sky.SetTimeOfDay(TimeOfDay); 



			World.HandleWorldGeneration(Camera); 

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

			glBindFramebuffer(GL_FRAMEBUFFER, NULL);
			World.RenderWorld(Window, Camera, Sky.SkyCube); 

			glFinish();


			Window.GetRawWindow()->display(); 

			//std::cout << "Frametime: " << Window.GetFrameTime() * 1000. << " (ms)\n"; 

		}




	}

}