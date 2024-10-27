set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
	PUBLIC FILE_SET headers TYPE HEADERS FILES
		irTools.hpp
		nec.hpp
		nubert.hpp
		rc6.hpp
	PRIVATE
		irTools.cpp
		nec.cpp
		nubert.cpp
		rc6.cpp
)

if(${PLATFORM} STREQUAL "native")
	# native platform (Windows, MacOS, Linux)

elseif(${PLATFORM} STREQUAL "emu")
	target_sources(${PROJECT_NAME}
		PUBLIC FILE_SET platform_headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/emu FILES
			emu/coco/platform/IrReceiver_emu.hpp
		PRIVATE
			emu/coco/platform/IrReceiver_emu.cpp
	)
elseif(${PLATFORM} MATCHES "^nrf52")
#	target_sources(${PROJECT_NAME}
#		PUBLIC FILE_SET platform_headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/nrf52 FILES
#			nrf52/coco/platform/.hpp
#		PRIVATE
#			nrf52/coco/platform/.cpp
#	)

elseif(${PLATFORM} MATCHES "^stm32")
	target_sources(${PROJECT_NAME}
		PUBLIC FILE_SET platform_headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/stm32 FILES
			stm32/coco/platform/IrReceiver_TIM.hpp
		PRIVATE
			stm32/coco/platform/IrReceiver_TIM.cpp
	)
endif()

target_link_libraries(${PROJECT_NAME}
	coco::coco
	coco-loop::coco-loop
	coco-device::coco-device
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		..
)

# install the library
install(TARGETS ${PROJECT_NAME}
	FILE_SET headers DESTINATION include/coco
	FILE_SET platform_headers DESTINATION include
)
