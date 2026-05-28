// Single entry point loaded by the C++ scripting host
// (src/map/scripting/script_host.cpp). Side-effect imports walk the
// tree; every register*() call below the import graph runs at
// module-evaluation time and populates the host's NPC registry.
//
// To add new content: drop a file under the appropriate subtree and
// add a matching `import "./newfile";` line in the nearest index.ts.
// No C++-side changes required.

import "./npcs";
