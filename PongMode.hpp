#include "ColorTextureProgram.hpp"

#include "Mode.hpp"
#include "GL.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

/*
 * PongMode is a game mode that implements a single-player game of Pong.
 */

struct PongMode : Mode {
	PongMode();
	virtual ~PongMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update_mouse_pos(glm::vec2 const& new_mouse_pos, glm::uvec2 const& window_size);
	virtual void update(float elapsed, Window_settings& window_settings) override;
	virtual void cycle_title(Mode::Window_settings &window_settings);
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- settings -----

	const float d_camera_bounds_per_bounce = 0.25f;
	const int d_window_size_per_bounce = 20;

	const char *title_cycle[4] = {
		"Skies of Pongora   <(^o^<)",
		"Skies of Pongora    <(^o^)>",
		"Skies of Pongora     (>^o^)>",
		"Skies of Pongora    <(^o^)>"
	};

	// POIs
	const float POI_radius = 5.0f;
	const float POI_opacity_radius_inner = 0.5f;
	const float POI_opacity_radius_outer = 2.0f;

	struct POI {
		POI(glm::vec2 const& Position_, float Radius_) :
			Position(Position_), Radius(Radius_) { }
		glm::vec2 Position;
		float Radius;
		bool flip = true;
		bool rainbow = false;
	};
	std::vector< POI > POIs;

	// brick settings
	const float brick_width = 1.5f;
	const float brick_height = 0.5f;
	const float brick_padding = 0.5f;
	const float brick_layer_height = brick_height * 7.0f + brick_padding * 7.0f;

	// Vector of bricks
	struct Brick {
		Brick(glm::vec2 const& Position_, glm::vec2 const& Radius_) :
			Position(Position_), Radius(Radius_) { }
		glm::vec2 Position;
		glm::vec2 Radius;
		bool deleted = false;
	};
	std::vector< Brick > bricks;

	//----- game state -----

	glm::vec2 base_court_radius = glm::vec2(8.0f, 6.0f);
	glm::vec2 extreme_radius = glm::vec2(40.0f, 30.0f);
	glm::vec2 vert_paddle_radius = glm::vec2(0.2f, 1.0f);
	glm::vec2 horiz_paddle_radius = glm::vec2(1.0f, 0.2f);
	glm::vec2 ball_radius = glm::vec2(0.2f, 0.2f);

	glm::vec2 left_paddle = glm::vec2(-base_court_radius.x + 0.5f, 0.0f);
	glm::vec2 right_paddle = glm::vec2(base_court_radius.x - 0.5f, 0.0f);
	glm::vec2 bottom_paddle = glm::vec2(0.0f, -base_court_radius.y + 0.5f);
	glm::vec2 top_paddle = glm::vec2(0.0f, base_court_radius.y - 0.5f);

	glm::vec2 ball = glm::vec2(0.0f, 0.0f);
	glm::vec2 ball_velocity = glm::vec2(-1.0f, 0.0f);

	glm::vec2 absolute_mouse_pos = glm::vec2(0.0f, 0.0f);
	glm::vec2 relative_mouse_pos = glm::vec2(0.0f, 0.0f);

	glm::vec2 camera_pos = glm::vec2(0.0f, 0.0f);
	glm::vec2 camera_bounds_min = glm::vec2(-8.0f, -6.0f);
	glm::vec2 camera_bounds_max = glm::vec2(8.0f, 6.0f);

	glm::vec2 camera_velo = glm::vec2(0.0f, 0.0f);

	uint32_t left_score = 0;
	uint32_t right_score = 0;

	/*float ai_offset = 0.0f;
	float ai_offset_update = 0.0f;*/

	bool state_flipped = false;

	//----- pretty rainbow trails -----

	float trail_length = 1.3f;
	std::deque< glm::vec3 > ball_trail; //stores (x,y,age), oldest elements first

	//----- opengl assets / helpers ------

	//draw functions will work on vectors of vertices, defined as follows:
	struct Vertex {
		Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4*3 + 1*4 + 4*2, "PongMode::Vertex should be packed");

	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint white_tex = 0;

	//matrix that maps from clip coordinates to court-space coordinates:
	glm::mat3x2 clip_to_court = glm::mat3x2(1.0f);
	// computed in draw() as the inverse of OBJECT_TO_CLIP
	// (stored here so that the mouse handling code can use it to position the paddle)

};
