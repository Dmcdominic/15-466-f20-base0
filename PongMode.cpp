#include "PongMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

PongMode::PongMode() {

	//set up trail as if ball has been here for 'forever':
	ball_trail.clear();
	ball_trail.emplace_back(ball, trail_length);
	ball_trail.emplace_back(ball, 0.0f);

	// Set up POIs
	POIs.emplace_back(glm::vec2(0.0f, 10.0f), POI_radius);

	// TODO - more POIs?

	// Set up Brick vector
	float x, y;
	// Top & Bot
	for (x = brick_width / 2.0f + brick_padding / 2.0f; x <= base_court_radius.x + brick_layer_height; x += brick_width + brick_padding) {
		for (y = base_court_radius.y + brick_height; y <= base_court_radius.y + brick_height + brick_layer_height; y += brick_height + brick_padding) {
			bricks.emplace_back(glm::vec2( x, y), glm::vec2(brick_width / 2.0f, brick_height / 2.0f));
			bricks.emplace_back(glm::vec2(-x, y), glm::vec2(brick_width / 2.0f, brick_height / 2.0f));
			bricks.emplace_back(glm::vec2( x,-y), glm::vec2(brick_width / 2.0f, brick_height / 2.0f));
			bricks.emplace_back(glm::vec2(-x,-y), glm::vec2(brick_width / 2.0f, brick_height / 2.0f));
		}
	}
	// Left & Right
	for (y = brick_width / 2.0f + brick_padding / 2.0f; y <= base_court_radius.y + brick_layer_height; y += brick_width + brick_padding) {
		for (x = base_court_radius.x + brick_height; x <= base_court_radius.x + brick_height + brick_layer_height; x += brick_height + brick_padding) {
			bricks.emplace_back(glm::vec2(x, y), glm::vec2(brick_height / 2.0f, brick_width / 2.0f));
			bricks.emplace_back(glm::vec2(-x, y), glm::vec2(brick_height / 2.0f, brick_width / 2.0f));
			bricks.emplace_back(glm::vec2(x, -y), glm::vec2(brick_height / 2.0f, brick_width / 2.0f));
			bricks.emplace_back(glm::vec2(-x, -y), glm::vec2(brick_height / 2.0f, brick_width / 2.0f));
		}
	}

	// TODO - more bricks?

	
	//----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//set up the vertex array object to describe arrays of PongMode::Vertex:
		glVertexAttribPointer(
			color_texture_program.Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 0 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Color_vec4);

		glVertexAttribPointer(
			color_texture_program.TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 + 4*1 //offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		glm::uvec2 size = glm::uvec2(1,1);
		std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}
}

PongMode::~PongMode() {

	//----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;

	glDeleteTextures(1, &white_tex);
	white_tex = 0;
}

bool PongMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_MOUSEMOTION) {
		update_mouse_pos(glm::uvec2(evt.motion.x, evt.motion.y), window_size);
	}
	return false;
}

void PongMode::update_mouse_pos(glm::vec2 const& new_mouse_pos, glm::uvec2 const& window_size) {
	//convert mouse from window pixels (top-left origin, +y is down) to clip space ([-1,1]x[-1,1], +y is up):
	glm::vec2 clip_mouse = glm::vec2(
		(new_mouse_pos.x + 0.5f) / window_size.x * 2.0f - 1.0f,
		(new_mouse_pos.y + 0.5f) / window_size.y * -2.0f + 1.0f
	);
	//compute and save mouse position
	absolute_mouse_pos = clip_to_court * glm::vec3(clip_mouse, 1.0f);
}

void PongMode::update(float elapsed, Window_settings& window_settings) {

	static std::mt19937 mt; //mersenne twister pseudo-random number generator

	// Update camera position based on camera velocity 
	// TODO - implement list of camera targets, and default to ball as the target
	camera_pos += camera_velo * elapsed;
	camera_pos = ball;

	// Update relative mouse position, and move paddles accordingly
	relative_mouse_pos = absolute_mouse_pos + camera_pos;
	left_paddle.y = relative_mouse_pos.y;
	right_paddle.y = relative_mouse_pos.y;
	bottom_paddle.x = relative_mouse_pos.x;
	top_paddle.x = relative_mouse_pos.x;

	//----- paddle update -----

	//{ //right player ai:
	//	ai_offset_update -= elapsed;
	//	if (ai_offset_update < elapsed) {
	//		//update again in [0.5,1.0) seconds:
	//		ai_offset_update = (mt() / float(mt.max())) * 0.5f + 0.5f;
	//		ai_offset = (mt() / float(mt.max())) * 2.5f - 1.25f;
	//	}
	//	if (right_paddle.y < ball.y + ai_offset) {
	//		right_paddle.y = std::min(ball.y + ai_offset, right_paddle.y + 2.0f * elapsed);
	//	} else {
	//		right_paddle.y = std::max(ball.y + ai_offset, right_paddle.y - 2.0f * elapsed);
	//	}
	//}

	//clamp paddles to court:
	left_paddle.y = std::max(left_paddle.y, -base_court_radius.y + vert_paddle_radius.y * 1.5f);
	left_paddle.y = std::min(left_paddle.y, base_court_radius.y - vert_paddle_radius.y * 1.5f);

	right_paddle.y = std::max(right_paddle.y, -base_court_radius.y + vert_paddle_radius.y * 1.5f);
	right_paddle.y = std::min(right_paddle.y, base_court_radius.y - vert_paddle_radius.y * 1.5f);

	bottom_paddle.x = std::max(bottom_paddle.x, -base_court_radius.x + horiz_paddle_radius.x * 1.5f);
	bottom_paddle.x = std::min(bottom_paddle.x, base_court_radius.x - horiz_paddle_radius.x * 1.5f);

	top_paddle.x = std::max(top_paddle.x, -base_court_radius.x + horiz_paddle_radius.x * 1.5f);
	top_paddle.x = std::min(top_paddle.x, base_court_radius.x - horiz_paddle_radius.x * 1.5f);

	//----- ball update -----

	//speed of ball doubles every four points:
	float speed_multiplier = 4.0f * std::pow(2.0f, (left_score + right_score) / 4.0f);

	//velocity cap, though (otherwise ball can pass through paddles):
	speed_multiplier = std::min(speed_multiplier, 10.0f);

	ball += elapsed * speed_multiplier * ball_velocity;

	//---- collision handling ----

	// lambda function to check for collisions with a rectangle
	auto rect_vs_ball = [this,&window_settings](glm::vec2 const &rect, glm::vec2 const &radius, bool velo_warp) {
		//compute area of overlap:
		glm::vec2 min = glm::max(rect - radius, ball - ball_radius);
		glm::vec2 max = glm::min(rect + radius, ball + ball_radius);

		//if no overlap, no collision:
		if (min.x > max.x || min.y > max.y) return false;

		if (max.x - min.x > max.y - min.y) {
			//wider overlap in x => bounce in y direction:
			if (ball.y > rect.y) {
				ball.y = rect.y + radius.y + ball_radius.y;
				ball_velocity.y = std::abs(ball_velocity.y);
			} else {
				ball.y = rect.y - radius.y - ball_radius.y;
				ball_velocity.y = -std::abs(ball_velocity.y);
			}
		} else {
			//wider overlap in y => bounce in x direction:
			if (ball.x > rect.x) {
				ball.x = rect.x + radius.x + ball_radius.x;
				ball_velocity.x = std::abs(ball_velocity.x);
			} else {
				ball.x = rect.x - radius.x - ball_radius.x;
				ball_velocity.x = -std::abs(ball_velocity.x);
			}
			//warp y velocity based on offset from paddle center:
			if (velo_warp) {
				float og_speed = sqrt(ball_velocity.x * ball_velocity.x + ball_velocity.y * ball_velocity.y);
				float vel = (ball.y - rect.y) / (radius.y + ball_radius.y);
				ball_velocity.y = glm::mix(ball_velocity.y, vel, 0.75f);
				ball_velocity /= sqrt(ball_velocity.x * ball_velocity.x + ball_velocity.y * ball_velocity.y);
				ball_velocity *= og_speed;
			}
		}

		cycle_title(window_settings);
		return true;
	};

	// lambda function to check for collisions with a circle
	auto circle_vs_ball = [this, &window_settings](glm::vec2 const& circle, float radius, POI *poi) {
		glm::vec2 displac = circle - ball;
		float dist = sqrt(displac.x * displac.x + displac.y * displac.y);
		if (poi != NULL && dist <= radius + POI_opacity_radius_outer) {
			window_settings.opacity = std::min(window_settings.opacity, (dist - (radius + POI_opacity_radius_inner)) / (POI_opacity_radius_outer - POI_opacity_radius_inner));
		}
		if (dist > radius) {
			return false;
		}
		glm::vec2 displac_norm = displac / dist;
		// Update velocity
		float dot_product = ball_velocity.x * displac_norm.x + ball_velocity.y * displac_norm.y;
		glm::vec2 projection = dot_product * displac_norm;
		ball_velocity -= projection * 2.0f;
		ball = circle - displac_norm * radius;
		if (poi != NULL && poi->flip) {
			state_flipped = !state_flipped;
		}
		cycle_title(window_settings);
		return true;
	};

	//paddles:
	rect_vs_ball(left_paddle, vert_paddle_radius, true);
	rect_vs_ball(right_paddle, vert_paddle_radius, true);
	rect_vs_ball(bottom_paddle, horiz_paddle_radius, true);
	rect_vs_ball(top_paddle, horiz_paddle_radius, true);

	// Check brick collisions with ball
	for (auto bricks_iter = bricks.begin(); bricks_iter != bricks.end(); bricks_iter++) {
		if (!(*bricks_iter).deleted) {
			if (rect_vs_ball((*bricks_iter).Position, (*bricks_iter).Radius, false)) {
				(*bricks_iter).deleted = true;
			}
		}
	}

	// Check POI collisions with ball
	window_settings.opacity = 1.0f;
	for (auto POI_iter = POIs.begin(); POI_iter != POIs.end(); POI_iter++) {
		circle_vs_ball((*POI_iter).Position, (*POI_iter).Radius, &(*POI_iter));
	}

	//court extremeties:
	if (ball.y > extreme_radius.y - ball_radius.y) {
		ball.y = extreme_radius.y - ball_radius.y;
		if (ball_velocity.y > 0.0f) {
			ball_velocity.y = -ball_velocity.y;
		}
		//extend wall upward.
		camera_bounds_max.y += d_camera_bounds_per_bounce;
		camera_pos.y += d_camera_bounds_per_bounce * 0.5f;
		window_settings.size.y += d_window_size_per_bounce;
		window_settings.position.y -= d_window_size_per_bounce;
		cycle_title(window_settings);
	}
	if (ball.y < -extreme_radius.y + ball_radius.y) {
		ball.y = -extreme_radius.y + ball_radius.y;
		if (ball_velocity.y < 0.0f) {
			ball_velocity.y = -ball_velocity.y;
		}
		//extend wall downward.
		camera_bounds_min.y -= d_camera_bounds_per_bounce;
		camera_pos.y -= d_camera_bounds_per_bounce * 0.5f;
		window_settings.size.y += d_window_size_per_bounce;
		cycle_title(window_settings);
	}

	if (ball.x > extreme_radius.x - ball_radius.x) {
		ball.x = extreme_radius.x - ball_radius.x;
		if (ball_velocity.x > 0.0f) {
			ball_velocity.x = -ball_velocity.x;
		}
		//extend wall right.
		camera_bounds_max.x += d_camera_bounds_per_bounce;
		camera_pos.x += d_camera_bounds_per_bounce * 0.5f;
		window_settings.size.x += d_window_size_per_bounce;
		cycle_title(window_settings);
	}
	if (ball.x < -extreme_radius.x + ball_radius.x) {
		ball.x = -extreme_radius.x + ball_radius.x;
		if (ball_velocity.x < 0.0f) {
			ball_velocity.x = -ball_velocity.x;
		}
		//extend wall left.
		camera_bounds_min.x -= d_camera_bounds_per_bounce;
		camera_pos.x -= d_camera_bounds_per_bounce * 0.5f;
		window_settings.size.x += d_window_size_per_bounce;
		window_settings.position.x -= d_window_size_per_bounce;
		cycle_title(window_settings);
	}

	//----- rainbow trails -----

	//age up all locations in ball trail:
	for (auto &t : ball_trail) {
		t.z += elapsed;
	}
	//store fresh location at back of ball trail:
	ball_trail.emplace_back(ball, 0.0f);

	//trim any too-old locations from back of trail:
	//NOTE: since trail drawing interpolates between points, only removes back element if second-to-back element is too old:
	while (ball_trail.size() >= 2 && ball_trail[1].z > trail_length) {
		ball_trail.pop_front();
	}
}

void PongMode::cycle_title(Mode::Window_settings &window_settings) {
	//update window name
	if (window_settings.title == NULL) {
		window_settings.title = &title_cycle[0];
		return;
	}
	int num_titles = sizeof(title_cycle)/sizeof(title_cycle[0]);
	for (int n = 0; n < num_titles; n++) {
		if (*window_settings.title == title_cycle[n]) {
			window_settings.title = &title_cycle[(n + 1) % num_titles];
			return;
		}
	}
	window_settings.title = &title_cycle[0];
}

void PongMode::draw(glm::uvec2 const &drawable_size) {
	//some nice colors from the course web page:
	#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
	const glm::u8vec4 bg_color = HEX_TO_U8VEC4(0x171714ff);
	const glm::u8vec4 fg_color = HEX_TO_U8VEC4(0xd1bb54ff);
	const glm::u8vec4 brick_color = HEX_TO_U8VEC4(0x03fca1ff);
	const glm::u8vec4 gold_color = HEX_TO_U8VEC4(0xd1bf1bff);
	const glm::u8vec4 red_color = HEX_TO_U8VEC4(0x8f071bff);
	const glm::u8vec4 shadow_color = HEX_TO_U8VEC4(0x604d29ff);
	const std::vector< glm::u8vec4 > rainbow_colors = {
		HEX_TO_U8VEC4(0x604d29ff), HEX_TO_U8VEC4(0x624f29fc), HEX_TO_U8VEC4(0x69542df2),
		HEX_TO_U8VEC4(0x6a552df1), HEX_TO_U8VEC4(0x6b562ef0), HEX_TO_U8VEC4(0x6b562ef0),
		HEX_TO_U8VEC4(0x6d572eed), HEX_TO_U8VEC4(0x6f592feb), HEX_TO_U8VEC4(0x725b31e7),
		HEX_TO_U8VEC4(0x745d31e3), HEX_TO_U8VEC4(0x755e32e0), HEX_TO_U8VEC4(0x765f33de),
		HEX_TO_U8VEC4(0x7a6234d8), HEX_TO_U8VEC4(0x826838ca), HEX_TO_U8VEC4(0x977840a4),
		HEX_TO_U8VEC4(0x96773fa5), HEX_TO_U8VEC4(0xa07f4493), HEX_TO_U8VEC4(0xa1814590),
		HEX_TO_U8VEC4(0x9e7e4496), HEX_TO_U8VEC4(0xa6844887), HEX_TO_U8VEC4(0xa9864884),
		HEX_TO_U8VEC4(0xad8a4a7c),
	};
	#undef HEX_TO_U8VEC4

	//other useful drawing constants:
	const float wall_radius = 0.05f;
	const float shadow_offset = 0.07f;
	const float padding = 0.14f; //padding between outside of walls and edge of window

	//---- compute vertices to draw ----

	//vertices will be accumulated into this list and then uploaded+drawn at the end of this function:
	std::vector< Vertex > vertices; // Triangle vertices

	//inline helper function for rectangle drawing:
	auto draw_rectangle = [&vertices,this](glm::vec2 const &center, glm::vec2 const &radius, glm::u8vec4 const &color) {
		//draw rectangle as two CCW-oriented triangles:
		vertices.emplace_back(glm::vec3(center.x-radius.x - camera_pos.x, center.y-radius.y - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x - camera_pos.x, center.y-radius.y - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x - camera_pos.x, center.y+radius.y - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));

		vertices.emplace_back(glm::vec3(center.x-radius.x - camera_pos.x, center.y-radius.y - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x - camera_pos.x, center.y+radius.y - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x-radius.x - camera_pos.x, center.y+radius.y - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
	};
	
	//inline helper function for circle drawing:
	auto draw_filled_circle = [&vertices, this](glm::vec2 const& center, glm::vec2 const& radius, glm::u8vec4 const& color) {
		uint16_t points = 100;
		float radians1 = 0;
		float x1 = cos(radians1);
		float y1 = sin(radians1);
		float radians0, x0, y0;
		for (uint16_t i = 1; i <= points + 1; i++) {
			radians0 = radians1;
			x0 = x1;
			y0 = y1;
			radians1 = i / float(points + 1) * 2.0f * float(M_PI);
			x1 = cos(radians1);
			y1 = sin(radians1);
			vertices.emplace_back(glm::vec3(center.x - camera_pos.x, center.y - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
			vertices.emplace_back(glm::vec3(center.x + radius.x * x0 - camera_pos.x, center.y + radius.y * y0 - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
			vertices.emplace_back(glm::vec3(center.x + radius.x * x1 - camera_pos.x, center.y + radius.y * y1 - camera_pos.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		}
		//TODO - use triangle fan instead?
	};

	// POIs
	for (auto POI_iter = POIs.begin(); POI_iter != POIs.end(); POI_iter++) {
		float radius = (*POI_iter).Radius;
		draw_filled_circle((*POI_iter).Position, glm::vec2(radius, radius), fg_color);
	}

	//shadows for everything (except the trail):

	/*glm::vec2 s = glm::vec2(shadow_offset,-shadow_offset);

	draw_rectangle(glm::vec2(-court_radius.x-wall_radius, 0.0f)+s, glm::vec2(wall_radius, court_radius.y + 2.0f * wall_radius), shadow_color);
	draw_rectangle(glm::vec2( court_radius.x+wall_radius, 0.0f)+s, glm::vec2(wall_radius, court_radius.y + 2.0f * wall_radius), shadow_color);
	draw_rectangle(glm::vec2( 0.0f,-court_radius.y-wall_radius)+s, glm::vec2(court_radius.x, wall_radius), shadow_color);
	draw_rectangle(glm::vec2( 0.0f, court_radius.y+wall_radius)+s, glm::vec2(court_radius.x, wall_radius), shadow_color);
	draw_rectangle(left_paddle+s, paddle_radius, shadow_color);
	draw_rectangle(right_paddle+s, paddle_radius, shadow_color);
	draw_rectangle(ball+s, ball_radius, shadow_color);*/

	//ball's trail:
	if (ball_trail.size() >= 2) {
		//start ti at second element so there is always something before it to interpolate from:
		std::deque< glm::vec3 >::iterator ti = ball_trail.begin() + 1;
		//draw trail from oldest-to-newest:
		for (uint32_t i = uint32_t(rainbow_colors.size())-1; i < rainbow_colors.size(); --i) {
			//time at which to draw the trail element:
			float t = (i + 1) / float(rainbow_colors.size()) * trail_length;
			//advance ti until 'just before' t:
			while (ti != ball_trail.end() && ti->z > t) ++ti;
			//if we ran out of tail, stop drawing:
			if (ti == ball_trail.end()) break;
			//interpolate between previous and current trail point to the correct time:
			glm::vec3 a = *(ti-1);
			glm::vec3 b = *(ti);
			glm::vec2 at = (t - a.z) / (b.z - a.z) * (glm::vec2(b) - glm::vec2(a)) + glm::vec2(a);
			//draw:
			draw_rectangle(at, ball_radius, rainbow_colors[i]);
		}
	}

	//solid objects:

	//walls:
	draw_rectangle(glm::vec2(-extreme_radius.x-wall_radius, 0.0f), glm::vec2(wall_radius, extreme_radius.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(glm::vec2(extreme_radius.x+wall_radius, 0.0f), glm::vec2(wall_radius, extreme_radius.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(glm::vec2( 0.0f,-extreme_radius.y-wall_radius), glm::vec2(extreme_radius.x, wall_radius), fg_color);
	draw_rectangle(glm::vec2( 0.0f, extreme_radius.y+wall_radius), glm::vec2(extreme_radius.x, wall_radius), fg_color);

	//paddles:
	draw_rectangle(left_paddle, vert_paddle_radius, fg_color);
	draw_rectangle(right_paddle, vert_paddle_radius, fg_color);
	draw_rectangle(bottom_paddle, horiz_paddle_radius, fg_color);
	draw_rectangle(top_paddle, horiz_paddle_radius, fg_color);
	
	//ball:
	draw_rectangle(ball, ball_radius, fg_color);

	//scores:
	/*glm::vec2 score_radius = glm::vec2(0.1f, 0.1f);
	for (uint32_t i = 0; i < left_score; ++i) {
		draw_rectangle(glm::vec2( -court_radius.x + (2.0f + 3.0f * i) * score_radius.x, court_radius.y + 2.0f * wall_radius + 2.0f * score_radius.y), score_radius, fg_color);
	}
	for (uint32_t i = 0; i < right_score; ++i) {
		draw_rectangle(glm::vec2( court_radius.x - (2.0f + 3.0f * i) * score_radius.x, court_radius.y + 2.0f * wall_radius + 2.0f * score_radius.y), score_radius, fg_color);
	}*/

	//bricks:
	for (auto bricks_iter = bricks.begin(); bricks_iter != bricks.end(); bricks_iter++) {
		if (!(*bricks_iter).deleted) {
			draw_rectangle((*bricks_iter).Position, (*bricks_iter).Radius, brick_color);
		}
	}



	//------ compute court-to-window transform ------

	//compute area that should be visible:
	glm::vec2 scene_min = glm::vec2(
		/*-court_radius.x - 2.0f * wall_radius - padding,
		-court_radius.y - 2.0f * wall_radius - padding*/
		camera_bounds_min.x,
		camera_bounds_min.y
	);
	glm::vec2 scene_max = glm::vec2(
		/*court_radius.x + 2.0f * wall_radius + padding,
		court_radius.y + 2.0f * wall_radius + 3.0f * score_radius.y + padding*/
		camera_bounds_max.x,
		camera_bounds_max.y
	);

	//compute window aspect ratio:
	float aspect = drawable_size.x / float(drawable_size.y);
	//we'll scale the x coordinate by 1.0 / aspect to make sure things stay square.

	//compute scale factor for court given that...
	float scale = std::min(
		(2.0f * aspect) / (scene_max.x - scene_min.x), //... x must fit in [-aspect,aspect] ...
		(2.0f) / (scene_max.y - scene_min.y) //... y must fit in [-1,1].
	);

	if (state_flipped) {
		scale = -scale;
	}

	glm::vec2 center = 0.5f * (scene_max + scene_min);

	//build matrix that scales and translates appropriately:
	glm::mat4 court_to_clip = glm::mat4(
		glm::vec4(scale / aspect, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-center.x * (scale / aspect), -center.y * scale, 0.0f, 1.0f)
	);
	//NOTE: glm matrices are specified in *Column-Major* order,
	// so each line above is specifying a *column* of the matrix(!)

	//also build the matrix that takes clip coordinates to court coordinates (used for mouse handling):
	clip_to_court = glm::mat3x2(
		glm::vec2(aspect / scale, 0.0f),
		glm::vec2(0.0f, 1.0f / scale),
		glm::vec2(center.x, center.y)
	);

	//---- actual drawing ----

	//clear the color buffer:
	glClearColor(bg_color.r / 255.0f, bg_color.g / 255.0f, bg_color.b / 255.0f, bg_color.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	//upload vertices to vertex_buffer:
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); //set vertex_buffer as current
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//set color_texture_program as current program:
	glUseProgram(color_texture_program.program);

	//upload OBJECT_TO_CLIP to the proper uniform location:
	glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(court_to_clip));

	//use the mapping vertex_buffer_for_color_texture_program to fetch vertex data:
	glBindVertexArray(vertex_buffer_for_color_texture_program);

	//bind the solid white texture to location zero so things will be drawn just with their colors:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	//run the OpenGL pipeline:
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

	//unbind the solid white texture:
	glBindTexture(GL_TEXTURE_2D, 0);

	//reset vertex array to none:
	glBindVertexArray(0);

	//reset current program to none:
	glUseProgram(0);
	

	GL_ERRORS(); //PARANOIA: print errors just in case we did something wrong.

}
