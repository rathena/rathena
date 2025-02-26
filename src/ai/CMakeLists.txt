# AI Module CMakeLists.txt
# This file defines the build configuration for the AI module

# Set minimum required CMake version
cmake_minimum_required(VERSION 3.10)

# Define the AI module library
set(AI_MODULE_SOURCES
    ai_module.cpp
    common/ai_request.cpp
    common/ai_response.cpp
    providers/azure_openai_provider.cpp
    providers/openai_provider.cpp
    providers/deepseek_provider.cpp
    providers/local_provider.cpp
    agents/world_evolution_agent.cpp
    agents/legend_bloodlines_agent.cpp
    agents/cross_class_synthesis_agent.cpp
    agents/quest_generation_agent.cpp
    agents/economic_ecosystem_agent.cpp
    agents/social_dynamics_agent.cpp
    agents/combat_mechanics_agent.cpp
    agents/housing_system_agent.cpp
    agents/time_manipulation_agent.cpp
    agents/npc_intelligence_agent.cpp
    agents/guild_evolution_agent.cpp
    agents/dimensional_warfare_agent.cpp
)

# Define the AI module headers
set(AI_MODULE_HEADERS
    ai_module.hpp
    common/ai_agent.hpp
    common/ai_provider.hpp
    common/ai_request.hpp
    common/ai_response.hpp
    providers/azure_openai_provider.hpp
    providers/openai_provider.hpp
    providers/deepseek_provider.hpp
    providers/local_provider.hpp
    agents/world_evolution_agent.hpp
    agents/legend_bloodlines_agent.hpp
    agents/cross_class_synthesis_agent.hpp
    agents/quest_generation_agent.hpp
    agents/economic_ecosystem_agent.hpp
    agents/social_dynamics_agent.hpp
    agents/combat_mechanics_agent.hpp
    agents/housing_system_agent.hpp
    agents/time_manipulation_agent.hpp
    agents/npc_intelligence_agent.hpp
    agents/guild_evolution_agent.hpp
    agents/dimensional_warfare_agent.hpp
)

# Create the AI module library
add_library(ai_module STATIC ${AI_MODULE_SOURCES} ${AI_MODULE_HEADERS})

# Set include directories
target_include_directories(ai_module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Set compiler flags
target_compile_definitions(ai_module PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Set compiler options
target_compile_options(ai_module PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

# Set C++ standard
target_compile_features(ai_module PRIVATE cxx_std_17)

# Link dependencies
target_link_libraries(ai_module PRIVATE
    common
    config
    database
)

# Add dependencies for third-party libraries
find_package(CURL REQUIRED)
target_link_libraries(ai_module PRIVATE ${CURL_LIBRARIES})
target_include_directories(ai_module PRIVATE ${CURL_INCLUDE_DIRS})

find_package(nlohmann_json REQUIRED)
target_link_libraries(ai_module PRIVATE nlohmann_json::nlohmann_json)

# Install the AI module library
install(TARGETS ai_module
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the AI module headers
install(FILES ${AI_MODULE_HEADERS}
    DESTINATION include/ai
)

# Add subdirectories for tests if testing is enabled
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()