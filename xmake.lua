---@diagnostic disable: undefined-global, undefined-field

local VERSION = "0.1.2"

set_project("mprisctl")
set_version(VERSION)
set_languages("c++20")
set_warnings("allextra", "pedantic", "error")

add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.check", "mode.profile")
add_rules("plugin.compile_commands.autoupdate", { lsp = "clangd", outputdir = "build" })

add_requires("boost ^1.86.0", "dbus-1")
add_requireconfs("boost", { configs = { program_options = true } })

target("mprisctl", function()
	set_kind("binary")
	add_files("src/**.cpp")
	add_includedirs("include")
	add_packages("dbus-1", "boost")
	add_defines('VERSION = "' .. VERSION .. '"')
end)
