#include <atomic>
#include <cache.h>
#include <frp/push/map.h>
#include <frp/push/source.h>
#include <frp/push/transform.h>
#include <geometry.h>
#include <cstdlib>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <SDL.h>
#include <SDL_image.h>
#include <unordered_map>

#define ASSERT_SDL(expression) { if (!expression) { \
	std::cout << __FILE__ << "(" << __LINE__ << "): SDL Error: " << SDL_GetError() << std::endl; \
	exit(1); } }

typedef std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> sdl_window_type;
typedef std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> sdl_renderer_type;
typedef std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> sdl_texture_type;
typedef std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> sdl_surface_type;

struct resource_type {
	std::string sprite_filename;
	point_type size;
};

const resource_type main_character{ "star.png", { 64, 64 } };

struct instance_type {
	std::atomic<point_type> position;
	const resource_type &resource;

	instance_type(const point_type &position, const resource_type &resource)
		: position(position), resource(resource) {}
};

struct sprite_type {
	std::shared_ptr<instance_type> instance;
	std::shared_ptr<sdl_texture_type> texture;

	bool operator==(const sprite_type &s) const {
		return instance == s.instance && texture == s.texture;
	}
};

void draw(sdl_renderer_type &ren, const sprite_type &sprite) {
	point_type position(sprite.instance->position);
	const SDL_Rect rect{ int(position.x), int(position.y), int(sprite.instance->resource.size.x),
		int(sprite.instance->resource.size.y) };
	SDL_RenderCopy(ren.get(), sprite.texture->get(), nullptr, &rect);
}

point_type texture_size(sdl_texture_type &texture) {
	int x, y;
	SDL_QueryTexture(texture.get(), nullptr, nullptr, &x, &y);
	return{ numeric_type(x), numeric_type(y) };
}

constexpr point_type gravity{ 0, 60 };
constexpr numeric_type walk_speed(30), jump_velocity(120);

struct dynamic_body_type {
	std::shared_ptr<instance_type> character;
	std::atomic<point_type> velocity;

	dynamic_body_type(const std::shared_ptr<instance_type> &character, const point_type &velocity = { 0, 0 })
		: character(character), velocity(velocity) {}

	void add_velocity(const point_type &v) {
		point_type current_velocity(velocity.load());
		while (!velocity.compare_exchange_weak(current_velocity, v + current_velocity));
	}

	void integrate(uint32_t delta) {
		auto d(numeric_type(delta) / 1000);
		add_velocity(gravity * d);
		point_type current_position(character->position.load());
		auto delta_velocity(velocity.load() * d);
		while (!character->position.compare_exchange_weak(current_position,
			current_position + delta_velocity));
	}
};

template<typename T>
struct sink_type {

	std::shared_ptr<T> current; // atomic
	frp::push::repository_type<void> receiver;
	typedef std::function<void(const T &)> function_type;
	function_type function;

	template<typename F, typename Supplier>
	sink_type(F &&function, Supplier supplier)
		: function(std::forward<F>(function))
		, receiver(frp::push::transform([this](auto value) {
			std::atomic_store(&current, std::make_shared<T>(value));
		}, supplier)) {}

	void operator()() const {
		auto current(std::atomic_load(&current));
		if (current) {
			function(*current);
		}
	}

	operator bool() const {
		return !!std::atomic_load(&current);
	}

	T &get() const {
		return *std::atomic_load(&current);
	}
};

int main(int argc, char *argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	atexit(&SDL_Quit);

	auto win = sdl_window_type(
		SDL_CreateWindow("Hello World!", 100, 100, 1024, 768, SDL_WINDOW_SHOWN),
		&SDL_DestroyWindow);
	ASSERT_SDL(win);

	auto ren = sdl_renderer_type(
		SDL_CreateRenderer(win.get(), -1, SDL_RENDERER_ACCELERATED),
		&SDL_DestroyRenderer);
	ASSERT_SDL(win);

	cache_type<std::string, sdl_texture_type> image_cache([&](const std::string &value) {
		return sdl_texture_type(IMG_LoadTexture(ren.get(), value.c_str()), &SDL_DestroyTexture);
	});

	typedef std::vector<std::shared_ptr<instance_type>> character_container_type;

	auto character(frp::push::source(std::make_shared<instance_type>(point_type{ 5, 5 },
		main_character)));

	auto dynamic(frp::push::transform(
		[](auto character) {
			return std::make_shared<dynamic_body_type>(character);
		},
		std::ref(character)));

	sink_type<std::shared_ptr<dynamic_body_type>> controller(
		[](auto dynamic) {}, std::ref(dynamic));

	auto arrows(frp::push::source(std::vector<std::shared_ptr<instance_type>>()));

	// TODO(gardell): This must be a cached map where we reuse previous instances so we don't reset the velocity.
	auto dynamic_arrows(frp::push::map([](auto arrow) {
		return std::make_shared<dynamic_body_type>(arrow, point_type{ 20, 0 });
	}, std::ref(arrows)));

	auto dynamics(frp::push::transform(
		[](auto dynamic, auto dynamics) {
			std::vector<std::shared_ptr<dynamic_body_type>> vector{ dynamics };
			vector.push_back(dynamic);
			return vector;
		},
		std::ref(dynamic), std::ref(dynamic_arrows)));

	sink_type<std::vector<std::shared_ptr<dynamic_body_type>>> integrator(
		[&, previous = SDL_GetTicks()](auto dynamics) mutable {
			auto current(SDL_GetTicks());
			auto delta(current - previous);
			previous = current;
			for (const auto dynamic : dynamics) {
				dynamic->integrate(delta);
			}
		}, std::ref(dynamics));

	auto characters(frp::push::transform(
		[](auto character, auto arrows) {
			std::vector<std::shared_ptr<instance_type>> instances(arrows);
			instances.push_back(character);
			return instances;
		},
		std::ref(character), std::ref(arrows)));

	auto sprites(frp::push::map(
		[&](auto character) {
			return sprite_type{ character, image_cache(character->resource.sprite_filename) };
		},
		std::ref(characters)));

	sink_type<std::vector<sprite_type>> renderer([&ren](auto sprites) {
		for (const auto sprite : sprites) {
			draw(ren, sprite);
		}
	}, std::ref(sprites));

	SDL_Event e;
	while (true) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				return 0;
			case SDL_KEYDOWN:
				if (controller && !e.key.repeat) {
					switch (e.key.keysym.sym) {
					case SDLK_SPACE:
						controller.get()->add_velocity({ 0, -jump_velocity });
						break;
					case SDLK_LEFT:
						controller.get()->add_velocity({ -walk_speed, 0 });
						break;
					case SDLK_RIGHT:
						controller.get()->add_velocity({ walk_speed, 0 });
						break;
					case SDLK_LCTRL:
					case SDLK_RCTRL:
					{
						auto temp(*arrows);
						temp.push_back(std::make_shared<instance_type>(controller.get()->character->position, main_character));
						arrows = temp;
					}
						break;
					}
				}
				break;
			case SDL_KEYUP:
				break;
			}
		}

		integrator();
		SDL_RenderClear(ren.get());
		renderer();
		SDL_RenderPresent(ren.get());
	}

	return 0;
}
