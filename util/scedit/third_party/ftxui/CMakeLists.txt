# FTXUI v5.0.0 — scedit-owned build glue for a PRUNED vendored tree.
#
# This file is NOT upstream's. Upstream's top-level CMakeLists.txt ends with an
# unconditional `add_subdirectory(examples)` / `add_subdirectory(doc)`, so it
# cannot be used against a tree with those directories removed; and its cmake/
# helpers pull in install/export/coverage/fuzzer machinery scedit does not want.
# What IS upstream is the SOURCE LISTS below: copied verbatim from
# FTXUI-5.0.0/CMakeLists.txt lines 31-133, minus the header entries (they are
# listed there only so IDEs show them). Explicit lists, never a glob: a file
# that disappears from the vendored tree must be a build error, not a silently
# dropped translation unit.
#
# Provenance, pin and the pruning manifest: util/scedit/README.md § Vendoring.
#
# Targets: ftxui::screen, ftxui::dom, ftxui::component (upstream's alias
# spellings, so the consumer code reads like any FTXUI program).

add_library(ftxui_screen STATIC
	src/ftxui/screen/box.cpp
	src/ftxui/screen/color.cpp
	src/ftxui/screen/color_info.cpp
	src/ftxui/screen/screen.cpp
	src/ftxui/screen/string.cpp
	src/ftxui/screen/terminal.cpp)

add_library(ftxui_dom STATIC
	src/ftxui/dom/automerge.cpp
	src/ftxui/dom/blink.cpp
	src/ftxui/dom/bold.cpp
	src/ftxui/dom/hyperlink.cpp
	src/ftxui/dom/border.cpp
	src/ftxui/dom/box_helper.cpp
	src/ftxui/dom/canvas.cpp
	src/ftxui/dom/clear_under.cpp
	src/ftxui/dom/color.cpp
	src/ftxui/dom/composite_decorator.cpp
	src/ftxui/dom/dbox.cpp
	src/ftxui/dom/dim.cpp
	src/ftxui/dom/flex.cpp
	src/ftxui/dom/flexbox.cpp
	src/ftxui/dom/flexbox_config.cpp
	src/ftxui/dom/flexbox_helper.cpp
	src/ftxui/dom/focus.cpp
	src/ftxui/dom/frame.cpp
	src/ftxui/dom/gauge.cpp
	src/ftxui/dom/graph.cpp
	src/ftxui/dom/gridbox.cpp
	src/ftxui/dom/hbox.cpp
	src/ftxui/dom/inverted.cpp
	src/ftxui/dom/linear_gradient.cpp
	src/ftxui/dom/node.cpp
	src/ftxui/dom/node_decorator.cpp
	src/ftxui/dom/paragraph.cpp
	src/ftxui/dom/reflect.cpp
	src/ftxui/dom/scroll_indicator.cpp
	src/ftxui/dom/separator.cpp
	src/ftxui/dom/size.cpp
	src/ftxui/dom/spinner.cpp
	src/ftxui/dom/strikethrough.cpp
	src/ftxui/dom/table.cpp
	src/ftxui/dom/text.cpp
	src/ftxui/dom/underlined.cpp
	src/ftxui/dom/underlined_double.cpp
	src/ftxui/dom/util.cpp
	src/ftxui/dom/vbox.cpp)

add_library(ftxui_component STATIC
	src/ftxui/component/animation.cpp
	src/ftxui/component/button.cpp
	src/ftxui/component/catch_event.cpp
	src/ftxui/component/checkbox.cpp
	src/ftxui/component/collapsible.cpp
	src/ftxui/component/component.cpp
	src/ftxui/component/component_options.cpp
	src/ftxui/component/container.cpp
	src/ftxui/component/dropdown.cpp
	src/ftxui/component/event.cpp
	src/ftxui/component/hoverable.cpp
	src/ftxui/component/input.cpp
	src/ftxui/component/loop.cpp
	src/ftxui/component/maybe.cpp
	src/ftxui/component/menu.cpp
	src/ftxui/component/modal.cpp
	src/ftxui/component/radiobox.cpp
	src/ftxui/component/renderer.cpp
	src/ftxui/component/resizable_split.cpp
	src/ftxui/component/screen_interactive.cpp
	src/ftxui/component/slider.cpp
	src/ftxui/component/terminal_input_parser.cpp
	src/ftxui/component/util.cpp
	src/ftxui/component/window.cpp)

foreach(lib ftxui_screen ftxui_dom ftxui_component)
	# SYSTEM on the interface: FTXUI's headers are not scedit's to warn about.
	target_include_directories(${lib} SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_include_directories(${lib} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
	target_compile_features(${lib} PUBLIC cxx_std_17)
	set_target_properties(${lib} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endforeach()

target_link_libraries(ftxui_dom PUBLIC ftxui_screen)
target_link_libraries(ftxui_component PUBLIC ftxui_dom)

find_package(Threads REQUIRED)
target_link_libraries(ftxui_component PUBLIC Threads::Threads)

add_library(ftxui::screen ALIAS ftxui_screen)
add_library(ftxui::dom ALIAS ftxui_dom)
add_library(ftxui::component ALIAS ftxui_component)
