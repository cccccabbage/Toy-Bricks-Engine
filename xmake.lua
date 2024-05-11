add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "build" })

set_languages("c++20")

add_requires("vulkansdk") -- installed from the installer in https://vulkan.lunarg.com/
add_requires("imgui", { configs = { vulkan = true, glfw = true } })
add_requires(             -- 3rd libs managed by xrepo
	"spdlog",
	"glfw",
	"glm",
	"stb",
	"vcpkg::tinyobjloader"
)

target("Toy-Bricks-Engine")
set_kind("binary")
add_includedirs("SourceCode/")
add_files("SourceCode/TBEngine/**.cpp")
add_files("SourceCode/main.cpp")
add_packages("vulkansdk", "spdlog", "glfw", "glm", "stb", "imgui", "vcpkg::tinyobjloader")
