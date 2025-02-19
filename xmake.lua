local VERSION = "1.0.0"

set_project("mprisctl")
set_version(VERSION)
set_languages("cxx23")
set_warnings("allextra", "pedantic")

set_toolchains("clang")
set_runtimes("c++_shared")

add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.check", "mode.profile")
add_rules("plugin.compile_commands.autoupdate", { lsp = "clangd", outputdir = "build" })

add_requires("dbus")

target("mprisctl", function()
	set_kind("binary")
	add_files("src/**.cpp")
	add_includedirs("include")
	add_packages("dbus")
	add_defines('VERSION = "' .. VERSION .. '"')
	set_policy("build.c++.modules", true)
end)
