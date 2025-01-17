#pragma once

#include "godot_cpp/classes/editor_plugin.hpp"
#include "godot_cpp/classes/editor_inspector_plugin.hpp"
#include "godot_cpp/classes/editor_property.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/rich_text_label.hpp"
#include "godot_cpp/classes/button_group.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/label.hpp"

#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/classes/thread.hpp"
#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/worker_thread_pool.hpp"

// CONSOLE RUNNING
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
// CONSOLE RUNNING

enum LFSState
{
    CheckedIn,
    CheckedOut,
    FailedCheckOut,

    NotLFS,

    Unknown
};

inline const char* LFSStateToString(LFSState state)
{
    switch(state)
    {
        case CheckedIn: return "CheckedIn";
        case CheckedOut: return "CheckedOut";
        case FailedCheckOut: return "FailedCheckout";
        case NotLFS: return "NotLFS";
        case Unknown: return "Unknown";
        default: return "No State";
    }
}

struct AssetInfo
{
    godot::String asset_path;
    LFSState asset_state;
};

struct CommandResult
{
    int exitcode;
    std::string output;
};

namespace
{

CommandResult exec_command_with_cwd(std::string cmd)
{
    int exitstatus = 255;
    //std::array<char, 1048576> buffer {};
    char buffer[8192];
    std::string result;

#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
    #define WEXITSTATUS
#endif

    godot::ProjectSettings* project_settings = godot::ProjectSettings::get_singleton();
    godot::String project_path = project_settings->globalize_path("res://");
    std::string cwd = project_path.utf8().get_data();

    std::string cwd_command = "cd \"" + cwd + "\" && ";
    std::string command = cwd_command + cmd;

    FILE *pipe = popen(command.c_str(), "r");

    if (pipe == nullptr) 
    {
        godot::UtilityFunctions::push_error("popen() failed!");

        return CommandResult
        {
            1,
            std::string("popen() failed!")
        };
    }

    char* line = NULL;
    size_t len = 0;

    while (fgets(buffer, 8192, pipe))
    {
        puts(buffer);
    }
    
    /*
    // For some reason I only get a single line back here
    std::size_t bytesread;
    while ((bytesread = fread(buffer.data(), sizeof(buffer.at(0)), sizeof(buffer), pipe)) != 0)
    {
        result += std::string(buffer.data(), bytesread);        
    }
    */

    result = buffer;
    exitstatus = WEXITSTATUS(pclose(pipe));

    return CommandResult
    {
        exitstatus,
        result
    };
}

std::string godotConvert(godot::String from)
{
    std::string converted = from.utf8().get_data();
    return converted;
}

godot::String stdConvert(std::string from)
{
    godot::String converted = from.c_str();
    return converted;
}

void CheckAssetLock(AssetInfo &ainfo)
{
    std::string command = "git lfs locks -p \"" + godotConvert(ainfo.asset_path) + "\"";
    CommandResult result = exec_command_with_cwd(command);
    godot::UtilityFunctions::print("Running command: ", stdConvert(command));
    godot::UtilityFunctions::print("Command result: ", stdConvert(result.output));
    /*
    if (result.exitcode != 0)
    {
        godot::String _result = result.output.c_str();
        godot::UtilityFunctions::push_error("Git command returned with error: ", _result);
        ainfo.asset_state = LFSState::Unknown;
    }
    */

    if (result.output.find("ID") != std::string::npos)
    {
        ainfo.asset_state = LFSState::CheckedOut;
        godot::UtilityFunctions::print("Asset is LFS Checked Out");
    }
    else
    {
        ainfo.asset_state = LFSState::CheckedIn;
        godot::UtilityFunctions::print("Asset is not checked out.");
    }

    godot::UtilityFunctions::print("Result: ", stdConvert(result.output));
}

void GetAssetInfo(AssetInfo &ainfo)
{
    godot::UtilityFunctions::print("Running GetAssetInfo");
    std::string _cmd = "git check-attr filter ";
    std::string _asset = ainfo.asset_path.utf8().get_data();
    std::string command = _cmd + "\"" + _asset + "\"";
    
    CommandResult result = exec_command_with_cwd(command);

    /*
    if (result.exitcode != 0)
    {
        godot::String _result = result.output.c_str();
        godot::UtilityFunctions::push_error("Git command returned with error: ", _result);
    }
    */

    if (result.output.find("lfs") == std::string::npos)
    {
        godot::UtilityFunctions::print("File is not LFS tracked: ", ainfo.asset_path);
        ainfo.asset_state = LFSState::NotLFS;
    }
    godot::UtilityFunctions::print("File is LFS tracked: ", ainfo.asset_path);

    CheckAssetLock(ainfo);

    godot::UtilityFunctions::print("Result: ", stdConvert(result.output));
}

void CheckOutAsset(AssetInfo &ainfo)
{
    godot::UtilityFunctions::print("Running CheckOutAsset");
    std::string command = "git lfs lock \"" + godotConvert(ainfo.asset_path) + "\"";
    CommandResult result = exec_command_with_cwd(command);

    /*
    if (result.exitcode != 0)
    {
        godot::String _result = result.output.c_str();
        godot::UtilityFunctions::push_error("Git command returned with error: ", _result);
    }
    */

    if (result.output.find("Locked") != std::string::npos)
    {
        godot::UtilityFunctions::print("State changed to CheckedOut");
        ainfo.asset_state = LFSState::CheckedOut;
    }
    else if (result.output.find("exists") != std::string::npos)
    {
        ainfo.asset_state = LFSState::CheckedIn;
        godot::UtilityFunctions::print("Failed to check out asset. Does someone else have a lock?", ainfo.asset_path);
    }

    godot::UtilityFunctions::print("Result: ", stdConvert(result.output));
}

void CheckInAsset(AssetInfo &ainfo)
{
    std::string command = "git lfs unlock \"" + godotConvert(ainfo.asset_path) + "\"";
    CommandResult result = exec_command_with_cwd(command);

    /*
    if (result.exitcode != 0)
    {
        godot::String _result = result.output.c_str();
        godot::UtilityFunctions::push_error("Git command returned with error: ", _result);
    }
    */

    if (result.output.find("Locked") != std::string::npos)
    {
        ainfo.asset_state = LFSState::FailedCheckOut;
        godot::UtilityFunctions::push_warning("Failed to check out asset. Is your asset modified?", ainfo.asset_path);
    }
    else
    {
        ainfo.asset_state = LFSState::CheckedIn;
    }

    godot::UtilityFunctions::print("Result: ", stdConvert(result.output));
}

}

class GitLFSControl : public godot::VBoxContainer
{
    GDCLASS(GitLFSControl, godot::VBoxContainer)

public:

    static void _bind_methods();

    GitLFSControl();
    ~GitLFSControl();

    void InitElements(const godot::String &assetPath);

    void _update_elements();
    void update_elements_impl();

    void _check_out_asset();
    void _check_out_asset_impl();

    void _check_in_asset();
    void _check_in_asset_impl();

    void _thread_pool_runner();

    godot::String AssetPath;

    godot::VBoxContainer* vContainer;
    godot::HBoxContainer* hStatusContainer;
    godot::Label* statusText;
    godot::Label* statusDisplay;
    godot::HBoxContainer* hLFSContainer;
    godot::Label* title;
    godot::Button* GitLFSCheckoutButton;
    godot::Button* GitLFSCheckinButton;

    godot::Callable update_elements_runner;
    godot::Callable update_runner_runner_impl;

    godot::Callable check_out_runner;
    godot::Callable check_out_runner_impl;

    godot::Callable check_in_runner;
    godot::Callable check_in_runner_impl;

    godot::Callable thread_pool_runner;

    AssetInfo ainfo;
    godot::Mutex* ainfo_mutex;

    godot::Thread* thread_pool_loop;
    std::vector<int64_t> thread_ids;
    bool b_pool_shutdown;

    godot::WorkerThreadPool* thread_pool;
};

class GitLFSInspectorPlugin : public godot::EditorInspectorPlugin
{
    GDCLASS(GitLFSInspectorPlugin, godot::EditorInspectorPlugin);

public:

    GitLFSInspectorPlugin();
    ~GitLFSInspectorPlugin();

    static void _bind_methods();

    bool _can_handle(Object* object) const;


    //bool _parse_property(Object *object, godot::Variant::Type type, const godot::String &name, godot::PropertyHint hint_type, const godot::String &hint_string, godot::BitField<godot::PropertyUsageFlags> usage_flags, bool wide) override;
    void _parse_category(Object *p_object, const godot::String &p_category) override;
    void _parse_begin(Object* object) override;
    void _parse_end(Object* object) override;

    godot::ProjectSettings* project_settings;
};

class GitLFSPlugin : public godot::EditorPlugin
{
    GDCLASS(GitLFSPlugin, godot::EditorPlugin);

public:

    GitLFSPlugin();
    ~GitLFSPlugin();
    godot::String _get_plugin_name() const override { return "GitLFSPlugin"; }

    static void _bind_methods();

    void _enter_tree() override;
    void _exit_tree() override;

    godot::Ref<GitLFSInspectorPlugin> InspectorPlugin;
    //godot::Control* LFSControl;
    //godot::Button* LFSCheckOutButton;
};
