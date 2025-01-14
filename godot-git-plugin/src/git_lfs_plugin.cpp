#include "git_lfs_plugin.h"

#include "godot_cpp/classes/button_group.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/label.hpp"

#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

void GitLFSHBoxContainer::_bind_methods()
{
}

GitLFSHBoxContainer::GitLFSHBoxContainer()
{
    set_alignment(AlignmentMode::ALIGNMENT_CENTER);
    set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);

    GitLFSCheckoutButton = memnew(godot::Button);
    GitLFSCheckoutButton->set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);
    GitLFSCheckoutButton->set_text("Git LFS Checkout");
    GitLFSCheckoutButton->set_modulate(godot::Color(255, 255, 255));
    GitLFSCheckoutButton->set_disabled(false);
    GitLFSCheckoutButton->set_action_mode(godot::BaseButton::ActionMode::ACTION_MODE_BUTTON_PRESS);

    GitLFSCheckoutButton->set_flat(false);
    GitLFSCheckoutButton->set_text_alignment(godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);
}

GitLFSHBoxContainer::~GitLFSHBoxContainer()
{
}

void GitLFSHBoxContainer::InitElements(const godot::String &assetPath)
{
    AssetPath = assetPath;

    title = memnew(godot::Label);
    title->set_text("Git LFS Controls");
    title->set_modulate(godot::Color(255,255,255));
    title->set_horizontal_alignment(godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_LEFT);
    title->set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);

    GitLFSCheckoutButton = memnew(godot::Button);
    GitLFSCheckoutButton->set_text("Checkout Asset");

    add_child(title);
    add_child(GitLFSCheckoutButton);
}

/////////////////////////////////////////////////////////////

GitLFSInspectorPlugin::GitLFSInspectorPlugin()
{
    project_settings = godot::ProjectSettings::get_singleton();
    godot::UtilityFunctions::print("GitLFSInspectorPlugin initialized.");
    //LFSProperty = memnew(godot::Control);
}

GitLFSInspectorPlugin::~GitLFSInspectorPlugin()
{
    godot::UtilityFunctions::print("GitLFSInspectorPlugin deinitialized.");
}

void GitLFSInspectorPlugin::_bind_methods()
{
}

bool GitLFSInspectorPlugin::_can_handle(Object* object) const
{
    godot::Variant::Type type = static_cast<godot::Variant::Type>(object->get("resource_path").get_type());
    if (type != godot::Variant::NIL)
    {
        return true;       
    }

    return false;
}

/*
bool GitLFSInspectorPlugin::_parse_property(godot::Object *object, godot::Variant::Type type, const godot::String &name, godot::PropertyHint hint_type, const godot::String &hint_string, godot::BitField<godot::PropertyUsageFlags> usage_flags, bool wide)
{
    godot::UtilityFunctions::print("Parsing Property");
    return true;
}
*/

void GitLFSInspectorPlugin::_parse_category(Object *p_object, const godot::String &p_category)
{
    if (p_category == "Resource")
    {
        const godot::String resource_path = p_object->get("resource_path");
        const godot::String resource_file = project_settings->globalize_path(resource_path);

        GitLFSHBoxContainer* lfs_hbox_container = memnew(GitLFSHBoxContainer);
        lfs_hbox_container->set_alignment(godot::BoxContainer::AlignmentMode::ALIGNMENT_CENTER);
        lfs_hbox_container->InitElements(resource_file);
        
        add_custom_control(lfs_hbox_container);
        /*
        godot::Button* pAddButton = memnew(godot::Button);
        pAddButton->set_text("Git LFS Checkout");
        pAddButton->set_action_mode(godot::BaseButton::ACTION_MODE_BUTTON_PRESS);
        add_custom_control(pAddButton);
        */
    }
}


void GitLFSInspectorPlugin::_parse_begin(Object* object)
{
    //godot::UtilityFunctions::print("GitLFSInspectorPlugin Parsing");
    //object->set("LFS Checked Out", godot::Variant::BOOL);

    
}

void GitLFSInspectorPlugin::_parse_end(Object* object)
{
    //godot::UtilityFunctions::print("GitLFSInspectorPlugin Finished Parsing");
}

/////////////////////////////////////////////////////////////

GitLFSPlugin::GitLFSPlugin()
{
    //add_inspector_plugin(InspectorPlugin);
    //LFSControl = new godot::Control;
    godot::UtilityFunctions::print("GitLFSPlugin initialized.");
}

GitLFSPlugin::~GitLFSPlugin()
{
    //remove_inspector_plugin(InspectorPlugin);
    //delete LFSControl;
    //delete LFSCheckOutButton;

    godot::UtilityFunctions::print("GitLFSPlugin deinitialized.");
}

void GitLFSPlugin::_bind_methods()
{
}

void GitLFSPlugin::_enter_tree()
{
    godot::UtilityFunctions::print("GitLFSPlugin entering tree.");
    InspectorPlugin.instantiate();
    add_inspector_plugin(InspectorPlugin);
    //LFSControl->set_name("Blank");
    //LFSCheckOutButton = add_control_to_bottom_panel(LFSControl, "Blank2");
}

void GitLFSPlugin::_exit_tree()
{
    //if (LFSControl != nullptr)
    //{
    //    remove_control_from_bottom_panel(LFSControl);
    //    LFSControl->queue_free();
    //}
    remove_inspector_plugin(InspectorPlugin);
    godot::UtilityFunctions::print("GitLFSPlugin exiting tree.");
}
