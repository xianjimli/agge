cmake_minimum_required(VERSION 2.8)

include(strmd)

set(SAMPLES_COMMON_SOURCES
	font_loader.cpp
	truetype.cpp
)

if (WIN32)
	set(SAMPLES_COMMON_SOURCES ${SAMPLES_COMMON_SOURCES}
		platform/win32/dc.cpp
		platform/win32/font.cpp
		platform/win32/shell.cpp
		platform/win32/timing.cpp
	)
elseif(APPLE)
	enable_language(OBJCXX)
	
	set(SAMPLES_COMMON_SOURCES ${SAMPLES_COMMON_SOURCES}
		platform/apple/main.mm
		platform/apple/ShellView.mm
	)
endif()

add_library(samples.common STATIC ${SAMPLES_COMMON_SOURCES})

target_link_libraries(samples.common agge agge.text)

if(APPLE)
	target_link_libraries(samples.common "-framework Foundation -framework Cocoa")
endif()
