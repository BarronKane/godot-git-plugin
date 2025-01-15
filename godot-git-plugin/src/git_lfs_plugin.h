#pragma once

#include "godot_cpp/classes/editor_plugin.hpp"
#include "godot_cpp/classes/editor_inspector_plugin.hpp"
#include "godot_cpp/classes/editor_property.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/button_group.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/label.hpp"

#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/classes/project_settings.hpp"

class GitLFSHBoxContainer : public godot::HBoxContainer
{
    GDCLASS(GitLFSHBoxContainer, godot::HBoxContainer)

public:

    static void _bind_methods();

    GitLFSHBoxContainer();
    ~GitLFSHBoxContainer();

    void InitElements(const godot::String &assetPath);

    void _update_elements();

    godot::String AssetPath;

    godot::Label* title;
    godot::Button* GitLFSCheckoutButton;

    godot::Callable update_elements_control;
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
