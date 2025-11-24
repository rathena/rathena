// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
// High-Performance Embedded Python Bridge Implementation
// Zero-overhead AI agent integration via CPython C API

#include "ai_bridge.hpp"

#include <cstring>
#include <chrono>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include "../map/npc.hpp"
#include "../map/unit.hpp"
#include "../map/map.hpp"

// AI service Python module path
#define AI_MODULE_PATH "../ai-autonomous-world/ai-service"
#define AI_MODULE_NAME "bridge.embedded_api"
#define AI_BRIDGE_CLASS "EmbeddedAIBridge"

// === Singleton Implementation ===

AIBridge& AIBridge::instance() noexcept {
    static AIBridge s_instance;
    return s_instance;
}

// === Initialization & Shutdown ===

bool AIBridge::initialize() noexcept {
    std::lock_guard<std::mutex> lock(m_init_mutex);
    
    // Already initialized
    if (m_initialized.load(std::memory_order_acquire)) {
        ShowWarning("AIBridge::initialize: Already initialized\n");
        return true;
    }
    
    ShowStatus("Initializing embedded Python AI bridge...\n");
    
    try {
        // Set Python paths before initialization
        wchar_t* python_home = Py_DecodeLocale(AI_MODULE_PATH, nullptr);
        if (python_home == nullptr) {
            ShowError("AIBridge::initialize: Failed to decode Python home path\n");
            return false;
        }
        Py_SetPythonHome(python_home);
        
        // Initialize Python interpreter
        Py_Initialize();
        
        // Enable threading support
        if (!PyEval_ThreadsInitialized()) {
            PyEval_InitThreads();
        }
        
        // Add AI service path to sys.path
        PyRun_SimpleString("import sys");
        char path_cmd[512];
        snprintf(path_cmd, sizeof(path_cmd), "sys.path.insert(0, '%s')", AI_MODULE_PATH);
        PyRun_SimpleString(path_cmd);
        
        // Import AI module
        PyObject* module_name = PyUnicode_DecodeFSDefault(AI_MODULE_NAME);
        if (module_name == nullptr) {
            ShowError("AIBridge::initialize: Failed to decode module name\n");
            handle_python_exception("decode module name");
            Py_Finalize();
            PyMem_RawFree(python_home);
            return false;
        }
        
        m_ai_module = PyImport_Import(module_name);
        Py_DECREF(module_name);
        
        if (m_ai_module == nullptr) {
            ShowError("AIBridge::initialize: Failed to import AI module '%s'\n", AI_MODULE_NAME);
            handle_python_exception("import AI module");
            Py_Finalize();
            PyMem_RawFree(python_home);
            return false;
        }
        
        // Get bridge class
        PyObject* bridge_class = PyObject_GetAttrString(m_ai_module, AI_BRIDGE_CLASS);
        if (bridge_class == nullptr) {
            ShowError("AIBridge::initialize: Failed to get bridge class '%s'\n", AI_BRIDGE_CLASS);
            handle_python_exception("get bridge class");
            Py_DECREF(m_ai_module);
            m_ai_module = nullptr;
            Py_Finalize();
            PyMem_RawFree(python_home);
            return false;
        }
        
        // Create orchestrator instance (assumes bridge has orchestrator attribute/parameter)
        // For now, we'll instantiate with no args and let Python handle orchestrator creation
        PyObject* args = PyTuple_New(0);
        m_orchestrator = PyObject_CallObject(bridge_class, args);
        Py_DECREF(args);
        Py_DECREF(bridge_class);
        
        if (m_orchestrator == nullptr) {
            ShowError("AIBridge::initialize: Failed to create orchestrator instance\n");
            handle_python_exception("create orchestrator");
            Py_DECREF(m_ai_module);
            m_ai_module = nullptr;
            Py_Finalize();
            PyMem_RawFree(python_home);
            return false;
        }
        
        // Cache function references for performance
        m_func_generate_dialogue = PyObject_GetAttrString(m_orchestrator, "generate_dialogue");
        m_func_get_npc_decision = PyObject_GetAttrString(m_orchestrator, "get_npc_decision");
        m_func_store_memory = PyObject_GetAttrString(m_orchestrator, "store_memory");
        m_func_generate_quest = PyObject_GetAttrString(m_orchestrator, "generate_quest");
        
        // Verify all functions are callable
        if (!PyCallable_Check(m_func_generate_dialogue) ||
            !PyCallable_Check(m_func_get_npc_decision) ||
            !PyCallable_Check(m_func_store_memory) ||
            !PyCallable_Check(m_func_generate_quest)) {
            ShowError("AIBridge::initialize: One or more AI functions are not callable\n");
            Py_XDECREF(m_func_generate_dialogue);
            Py_XDECREF(m_func_get_npc_decision);
            Py_XDECREF(m_func_store_memory);
            Py_XDECREF(m_func_generate_quest);
            Py_DECREF(m_orchestrator);
            Py_DECREF(m_ai_module);
            m_orchestrator = nullptr;
            m_ai_module = nullptr;
            Py_Finalize();
            PyMem_RawFree(python_home);
            return false;
        }
        
        // Release main thread state for other threads to use
        PyEval_SaveThread();
        
        PyMem_RawFree(python_home);
        
        m_initialized.store(true, std::memory_order_release);
        ShowStatus("Embedded Python AI bridge initialized successfully\n");
        return true;
        
    } catch (...) {
        ShowError("AIBridge::initialize: Unexpected exception during initialization\n");
        return false;
    }
}

void AIBridge::shutdown() noexcept {
    std::lock_guard<std::mutex> lock(m_init_mutex);
    
    if (!m_initialized.load(std::memory_order_acquire)) {
        return;
    }
    
    ShowStatus("Shutting down embedded Python AI bridge...\n");
    
    // Acquire GIL for cleanup
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    // Release function references
    Py_XDECREF(m_func_generate_quest);
    Py_XDECREF(m_func_store_memory);
    Py_XDECREF(m_func_get_npc_decision);
    Py_XDECREF(m_func_generate_dialogue);
    
    // Release orchestrator and module
    Py_XDECREF(m_orchestrator);
    Py_XDECREF(m_ai_module);
    
    m_func_generate_quest = nullptr;
    m_func_store_memory = nullptr;
    m_func_get_npc_decision = nullptr;
    m_func_generate_dialogue = nullptr;
    m_orchestrator = nullptr;
    m_ai_module = nullptr;
    
    // Release GIL and finalize
    PyGILState_Release(gstate);
    Py_Finalize();
    
    m_initialized.store(false, std::memory_order_release);
    ShowStatus("Embedded Python AI bridge shutdown complete\n");
}

// === Helper Functions ===

PyObject* AIBridge::to_python_string(const std::string& str) noexcept {
    return PyUnicode_DecodeUTF8(str.c_str(), str.length(), "replace");
}

std::string AIBridge::from_python_string(PyObject* obj) noexcept {
    if (obj == nullptr || !PyUnicode_Check(obj)) {
        return "";
    }
    
    Py_ssize_t size;
    const char* data = PyUnicode_AsUTF8AndSize(obj, &size);
    if (data == nullptr) {
        handle_python_exception("convert Python string to UTF-8");
        return "";
    }
    
    return std::string(data, size);
}

void AIBridge::handle_python_exception(const char* context) noexcept {
    if (PyErr_Occurred()) {
        PyObject *type, *value, *traceback;
        PyErr_Fetch(&type, &value, &traceback);
        PyErr_NormalizeException(&type, &value, &traceback);
        
        if (value != nullptr) {
            PyObject* str_obj = PyObject_Str(value);
            if (str_obj != nullptr) {
                std::string error_msg = from_python_string(str_obj);
                ShowError("AIBridge [%s]: Python exception: %s\n", context, error_msg.c_str());
                Py_DECREF(str_obj);
            }
        }
        
        Py_XDECREF(type);
        Py_XDECREF(value);
        Py_XDECREF(traceback);
    }
}

// === Template Specializations for call_python_function ===

template<>
std::string AIBridge::call_python_function<std::string>(
    const char* func_name, int npc_id, const std::string& arg1, int arg2) noexcept {
    
    if (!m_initialized.load(std::memory_order_acquire)) {
        ShowDebug("AIBridge::%s: Bridge not initialized\n", func_name);
        return "";
    }
    
    // Acquire GIL
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    try {
        // Get cached function reference
        PyObject* func = nullptr;
        if (strcmp(func_name, "generate_dialogue") == 0) {
            func = m_func_generate_dialogue;
        } else if (strcmp(func_name, "get_npc_decision") == 0) {
            func = m_func_get_npc_decision;
        } else if (strcmp(func_name, "generate_quest") == 0) {
            func = m_func_generate_quest;
        }
        
        if (func == nullptr) {
            ShowError("AIBridge::%s: Function not found\n", func_name);
            PyGILState_Release(gstate);
            return "";
        }
        
        // Build arguments based on function signature
        PyObject* args = nullptr;
        if (strcmp(func_name, "generate_dialogue") == 0 || strcmp(func_name, "get_npc_decision") == 0) {
            // (npc_id: int, message/context: str)
            args = PyTuple_New(2);
            PyTuple_SET_ITEM(args, 0, PyLong_FromLong(npc_id));
            PyTuple_SET_ITEM(args, 1, to_python_string(arg1));
        } else if (strcmp(func_name, "generate_quest") == 0) {
            // (npc_id: int, player_level: int)
            args = PyTuple_New(2);
            PyTuple_SET_ITEM(args, 0, PyLong_FromLong(npc_id));
            PyTuple_SET_ITEM(args, 1, PyLong_FromLong(arg2));
        }
        
        if (args == nullptr) {
            ShowError("AIBridge::%s: Failed to create arguments\n", func_name);
            PyGILState_Release(gstate);
            return "";
        }
        
        // Call Python function
        PyObject* result = PyObject_CallObject(func, args);
        Py_DECREF(args);
        
        if (result == nullptr) {
            handle_python_exception(func_name);
            PyGILState_Release(gstate);
            return "";
        }
        
        // Extract string result
        std::string ret = from_python_string(result);
        Py_DECREF(result);
        
        PyGILState_Release(gstate);
        return ret;
        
    } catch (...) {
        ShowError("AIBridge::%s: Unexpected C++ exception\n", func_name);
        PyGILState_Release(gstate);
        return "";
    }
}

template<>
bool AIBridge::call_python_function<bool>(
    const char* func_name, int npc_id, const std::string& arg1, int arg2) noexcept {
    
    if (!m_initialized.load(std::memory_order_acquire)) {
        ShowDebug("AIBridge::%s: Bridge not initialized\n", func_name);
        return false;
    }
    
    // Acquire GIL
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    try {
        PyObject* func = m_func_store_memory;
        
        if (func == nullptr) {
            ShowError("AIBridge::%s: Function not found\n", func_name);
            PyGILState_Release(gstate);
            return false;
        }
        
        // Build arguments: (npc_id: int, memory_data: str)
        PyObject* args = PyTuple_New(2);
        PyTuple_SET_ITEM(args, 0, PyLong_FromLong(npc_id));
        PyTuple_SET_ITEM(args, 1, to_python_string(arg1));
        
        // Call Python function
        PyObject* result = PyObject_CallObject(func, args);
        Py_DECREF(args);
        
        if (result == nullptr) {
            handle_python_exception(func_name);
            PyGILState_Release(gstate);
            return false;
        }
        
        // Check if result is truthy
        int is_true = PyObject_IsTrue(result);
        Py_DECREF(result);
        
        PyGILState_Release(gstate);
        return is_true == 1;
        
    } catch (...) {
        ShowError("AIBridge::%s: Unexpected C++ exception\n", func_name);
        PyGILState_Release(gstate);
        return false;
    }
}

// === Public AI Agent Functions ===

std::string AIBridge::generate_dialogue(int npc_id, const std::string& player_message) noexcept {
    return call_python_function<std::string>("generate_dialogue", npc_id, player_message, 0);
}

std::string AIBridge::get_npc_decision(int npc_id, const std::string& context_json) noexcept {
    return call_python_function<std::string>("get_npc_decision", npc_id, context_json, 0);
}

bool AIBridge::store_memory(int npc_id, const std::string& memory_data) noexcept {
    return call_python_function<bool>("store_memory", npc_id, memory_data, 0);
}

std::string AIBridge::generate_quest(int npc_id, int player_level) noexcept {
    return call_python_function<std::string>("generate_quest", npc_id, "", player_level);
}

// === Script Commands ===

BUILDIN_FUNC(ai_dialogue) {
    int npc_id = script_getnum(st, 2);
    const char* player_msg = script_getstr(st, 3);
    
    if (player_msg == nullptr) {
        ShowError("script:ai_dialogue: null player message\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    AIBridge& bridge = AIBridge::instance();
    if (!bridge.is_initialized()) {
        ShowWarning("script:ai_dialogue: AI bridge not initialized\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Generate AI dialogue
    std::string response = bridge.generate_dialogue(npc_id, player_msg);
    
    if (!response.empty()) {
        script_pushstrcopy(st, response.c_str());
    } else {
        script_pushconststr(st, "");
    }
    
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(ai_decision) {
    int npc_id = script_getnum(st, 2);
    const char* context = script_getstr(st, 3);
    
    if (context == nullptr) {
        ShowError("script:ai_decision: null context\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    AIBridge& bridge = AIBridge::instance();
    if (!bridge.is_initialized()) {
        ShowWarning("script:ai_decision: AI bridge not initialized\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Get AI decision
    std::string decision = bridge.get_npc_decision(npc_id, context);
    
    if (!decision.empty()) {
        script_pushstrcopy(st, decision.c_str());
    } else {
        script_pushconststr(st, "");
    }
    
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(ai_remember) {
    int npc_id = script_getnum(st, 2);
    const char* memory = script_getstr(st, 3);
    
    if (memory == nullptr) {
        ShowError("script:ai_remember: null memory data\n");
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    AIBridge& bridge = AIBridge::instance();
    if (!bridge.is_initialized()) {
        ShowWarning("script:ai_remember: AI bridge not initialized\n");
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    // Store memory
    bool success = bridge.store_memory(npc_id, memory);
    script_pushint(st, success ? 1 : 0);
    
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(ai_quest) {
    int npc_id = script_getnum(st, 2);
    int player_level = script_getnum(st, 3);
    
    AIBridge& bridge = AIBridge::instance();
    if (!bridge.is_initialized()) {
        ShowWarning("script:ai_quest: AI bridge not initialized\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Generate quest
    std::string quest = bridge.generate_quest(npc_id, player_level);
    
    if (!quest.empty()) {
        script_pushstrcopy(st, quest.c_str());
    } else {
        script_pushconststr(st, "");
    }
    
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(ai_walk) {
    int npc_id = script_getnum(st, 2);
    int x = script_getnum(st, 3);
    int y = script_getnum(st, 4);
    const char* map_name = script_getstr(st, 5);
    
    // Find NPC by ID
    npc_data* nd = map_id2nd(npc_id);
    if (nd == nullptr) {
        ShowWarning("script:ai_walk: NPC ID %d not found\n", npc_id);
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    // Validate map bounds
    if (nd->m < 0) {
        ShowError("script:ai_walk: NPC ID %d is not on a valid map\n", npc_id);
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    struct map_data* mapdata = map_getmapdata(nd->m);
    if (x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys) {
        ShowWarning("script:ai_walk: coordinates (%d,%d) out of bounds for map %s\n", 
                   x, y, mapdata->name);
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    // Get AI decision for movement
    AIBridge& bridge = AIBridge::instance();
    if (bridge.is_initialized()) {
        char context[512];
        snprintf(context, sizeof(context), 
                "{\"action\":\"walk\",\"x\":%d,\"y\":%d,\"map\":\"%s\"}", 
                x, y, map_name);
        
        std::string decision = bridge.get_npc_decision(npc_id, context);
        
        // Parse decision (in production, you'd parse JSON)
        // For now, we proceed with movement if we got a response
        if (!decision.empty()) {
            ShowDebug("script:ai_walk: AI approved movement for NPC %d\n", npc_id);
        }
    }
    
    // Initialize NPC status if needed
    if (!nd->status.hp) {
        status_calc_npc(nd, SCO_FIRST);
    } else {
        status_calc_npc(nd, SCO_NONE);
    }
    
    // Execute movement
    bool success = unit_walktoxy(nd, x, y, 0);
    
    script_pushint(st, success ? 1 : 0);
    return SCRIPT_CMD_SUCCESS;
}