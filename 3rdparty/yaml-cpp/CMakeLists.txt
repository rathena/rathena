set(YAML_HEADERS
	"yaml.h"
	"traits.h"
	"stlemitter.h"
	"parser.h"
	"ostream_wrapper.h"
	"null.h"
	"noncopyable.h"
	"noexcept.h"
	"mark.h"
	"exceptions.h"
	"eventhandler.h"
	"emitterstyle.h"
	"emittermanip.h"
	"emitter.h"
	"emitterdef.h"
	"emitfromevents.h"
	"dll.h"
	"depthguard.h"
	"binary.h"
	"anchor.h"
	"node/type.h"
	"node/ptr.h"
	"node/parse.h"
	"node/node.h"
	"node/iterator.h"
	"node/impl.h"
	"node/emit.h"
	"node/convert.h"
	"contrib/graphbuilder.h"
	"contrib/anchordict.h"
)
list(TRANSFORM YAML_HEADERS PREPEND "include/yaml-cpp/")

set(YAML_SOURCES
	"binary.cpp"
	"convert.cpp"
	"depthguard.cpp"
	"directives.cpp"
	"emit.cpp"
	"emitfromevents.cpp"
	"emitter.cpp"
	"emitterstate.cpp"
	"emitterutils.cpp"
	"exceptions.cpp"
	"exp.cpp"
	"memory.cpp"
	"nodebuilder.cpp"
	"node.cpp"
	"node_data.cpp"
	"nodeevents.cpp"
	"null.cpp"
	"ostream_wrapper.cpp"
	"parse.cpp"
	"parser.cpp"
	"regex_yaml.cpp"
	"scanner.cpp"
	"scanscalar.cpp"
	"scantag.cpp"
	"scantoken.cpp"
	"simplekey.cpp"
	"singledocparser.cpp"
	"stream.cpp"
	"tag.cpp"
	"contrib/graphbuilderadapter.cpp"
	"contrib/graphbuilder.cpp"
)
list(TRANSFORM YAML_SOURCES PREPEND "src/")

add_library(yaml-cpp STATIC EXCLUDE_FROM_ALL ${YAML_SOURCES})

target_include_directories(yaml-cpp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_definitions(yaml-cpp PUBLIC "-DYAML_CPP_STATIC_DEFINE")

if(WIN32)
	set_target_properties(yaml-cpp PROPERTIES FOLDER "3rdparty")
endif()
