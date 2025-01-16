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

// CONSOLE RUNNING
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <thread>
// CONSOLE RUNNING

enum LFSState
{
    CheckedIn,
    CheckedOut,
    Modified,

    NotLFS,

    Unknown
};

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
    std::array<char, 1048576> buffer {};
    std::string result;

#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
    #define WEXITSTATUS
#endif

    godot::ProjectSettings* project_settings = godot::ProjectSettings::get_singleton();
    godot::String project_path = project_settings->globalize_path("res://");
    std::string cwd = project_path.utf8().get_data();

    std::string cwd_command = "cd " + cwd + " && ";
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
    
    std::size_t bytesread;
    while ((bytesread = fread(buffer.data(), sizeof(buffer.at(0)), sizeof(buffer), pipe)) != 0)
    {
        result += std::string(buffer.data(), bytesread);        
    }

    exitstatus = WEXITSTATUS(pclose(pipe));

    return CommandResult
    {
        exitstatus,
        result
    };
}

AssetInfo GetAssetInfo(const godot::String &Asset)
{
    std::string _cmd = "git check-attr -a ";
    std::string _asset = Asset.utf8().get_data();
    std::string command = _cmd + _asset;
    
    CommandResult result = exec_command_with_cwd(command);

    if (result.exitcode != 0)
    {
        godot::String _result = result.output.c_str();
        godot::UtilityFunctions::push_error("Git command returned with error: ", _result);
    }
    
    LFSState state;
    godot::UtilityFunctions::print("GetAssetInfo: ", godot::String(result.output.c_str()));

    if (result.output.find("lfs") == std::string::npos)
    {
        godot::UtilityFunctions::print("File is not LFS tracked: ", Asset);
        state = LFSState::NotLFS;

        return AssetInfo 
        {
            Asset,
            state
        };
    }
    godot::UtilityFunctions::print("File is LFS tracked: ", Asset);

    return AssetInfo
    {
        Asset,
        LFSState::Unknown
    };
}

}

class GitLFSControl : public godot::Control
{
    GDCLASS(GitLFSControl, godot::Control)

public:

    static void _bind_methods();

    GitLFSControl();
    ~GitLFSControl();

    void InitElements(const godot::String &assetPath);

    void _update_elements();
    void update_elements_impl(AssetInfo ainfo);

    godot::String AssetPath;

    godot::VBoxContainer* vContainer;
    godot::HBoxContainer* hStatusContainer;
    godot::Label* statusText;
    godot::Label* statusDisplay;
    godot::HBoxContainer* hLFSContainer;
    godot::Label* title;
    godot::Button* GitLFSCheckoutButton;

    godot::Callable update_elements_control;

    AssetInfo ainfo;
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
