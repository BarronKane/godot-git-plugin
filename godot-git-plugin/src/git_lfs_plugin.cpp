#include "git_lfs_plugin.h"

#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/button_group.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/label.hpp"

#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/thread.hpp"

void GitLFSControl::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("_update_elements"), &GitLFSControl::_update_elements);
}

GitLFSControl::GitLFSControl()
{
    vContainer = memnew(godot::VBoxContainer);
    hStatusContainer = memnew(godot::HBoxContainer);
    statusText = memnew(godot::Label);
    statusDisplay = memnew(godot::Label);
    hLFSContainer = memnew(godot::HBoxContainer);
    title = memnew(godot::Label);
    GitLFSCheckoutButton = memnew(godot::Button);
}

GitLFSControl::~GitLFSControl()
{
}

void GitLFSControl::InitElements(const godot::String &assetPath)
{
    AssetPath = assetPath;
    ainfo = GetAssetInfo(assetPath);

    add_child(vContainer);

    vContainer->add_child(hStatusContainer);
    hStatusContainer->add_child(statusText);
    hStatusContainer->add_child(statusDisplay);

    vContainer->add_child(hLFSContainer);
    hLFSContainer->add_child(title);
    hLFSContainer->add_child(GitLFSCheckoutButton);

    hStatusContainer->set_alignment(godot::BoxContainer::ALIGNMENT_CENTER);
    hStatusContainer->set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);

    hLFSContainer->set_alignment(godot::BoxContainer::ALIGNMENT_CENTER);
    hLFSContainer->set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);

    statusText->set_text("LFS Status: ");

    statusDisplay->set_text("Checking...");

    title->set_text("Git LFS");
    title->set_modulate(godot::Color(255,255,255));
    title->set_horizontal_alignment(godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_LEFT);
    title->set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);

    update_elements_control = godot::Callable(this, "_update_elements");

    GitLFSCheckoutButton->set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);
    GitLFSCheckoutButton->set_text("Check Out Asset");
    GitLFSCheckoutButton->set_modulate(godot::Color(255, 255, 255));
    GitLFSCheckoutButton->set_disabled(true);

    GitLFSCheckoutButton->set_flat(true);
    GitLFSCheckoutButton->set_text_alignment(godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);
    
    GitLFSCheckoutButton->set_action_mode(godot::BaseButton::ACTION_MODE_BUTTON_PRESS);
    GitLFSCheckoutButton->connect("pressed", update_elements_control);
}

void GitLFSControl::_update_elements()
{
    title->set_text("Updated!");
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

        GitLFSControl* lfsControl = memnew(GitLFSControl);
        lfsControl->InitElements(resource_file);
        
        add_custom_control(lfsControl);
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
