// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef YAMLUPGRADE_HPP
#define YAMLUPGRADE_HPP

#include "../common/core.hpp"

#include "yaml.hpp"

using namespace rathena::server_core;

namespace rathena{
	namespace tool_yamlupgrade{
		class YamlUpgradeTool : public Core{
			protected:
				bool initialize( int argc, char* argv[] ) override;

			public:
				YamlUpgradeTool() : Core( e_core_type::TOOL ){

				}
		};
	}
}

#endif /* YAMLUPGRADE_HPP */
