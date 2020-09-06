#pragma once

#include <SDL.h>
#include <glm/glm.hpp>

#include <memory>

struct Mode : std::enable_shared_from_this< Mode > {
	virtual ~Mode() { }

	//struct for the mode to tell main how to update the window, if desired
	struct Window_settings {
		Window_settings(glm::uvec2 size_, glm::uvec2 position_, float opacity_, const char **title_) :
			size(size_), position(position_), opacity(opacity_), title(title_) { }
		glm::uvec2 size;
		glm::uvec2 position;
		float opacity;
		const char **title;
	};


	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) { return false; }

	//update is called at the start of a new frame, after events are handled:
	// 'elapsed' is time in seconds since the last call to 'update'
	virtual void update(float elapsed, Window_settings &window_settings) { }

	//draw is called after update:
	virtual void draw(glm::uvec2 const &drawable_size) = 0;

	//Mode::current is the Mode to which events are dispatched.
	// use 'set_current' to change the current Mode (e.g., to switch to a menu)
	static std::shared_ptr< Mode > current;
	static void set_current(std::shared_ptr< Mode > const &);
};

