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
#include "godot_cpp/classes/mutex.hpp"

void GitLFSControl::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("_update_elements"), &GitLFSControl::_update_elements);
    godot::ClassDB::bind_method(godot::D_METHOD("update_elements_impl"), &GitLFSControl::update_elements_impl);
    
    godot::ClassDB::bind_method(godot::D_METHOD("_check_out_asset"), &GitLFSControl::_check_out_asset);
    godot::ClassDB::bind_method(godot::D_METHOD("_check_out_asset_impl"), &GitLFSControl::_check_out_asset_impl);

    godot::ClassDB::bind_method(godot::D_METHOD("_check_in_asset"), &GitLFSControl::_check_in_asset);
    godot::ClassDB::bind_method(godot::D_METHOD("_check_in_asset_impl"), &GitLFSControl::_check_in_asset_impl);
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
    GitLFSCheckinButton = memnew(godot::Button);

    update_thread = memnew(godot::Thread);
    checkout_thread = memnew(godot::Thread);
    checkin_thread = memnew(godot::Thread);

    update_runner_caller = godot::Callable(this, "update_elements_impl");
    update_elements_control = godot::Callable(this, "_update_elements");

    check_out_runner = godot::Callable(this, "_check_out_asset");
    check_out_runner_impl = godot::Callable(this, "_check_out_asset_impl");

    check_in_runner = godot::Callable(this, "_check_in_asset");
    check_out_runner_impl = godot::Callable(this, "_check_in_asset_impl");

    ainfo_mutex = memnew(godot::Mutex);
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
    hLFSContainer->add_child(GitLFSCheckinButton);

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

    GitLFSCheckoutButton->set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);
    GitLFSCheckoutButton->set_text("Check Out Asset");
    //GitLFSCheckoutButton->(godot::Color(255, 255, 255));
    GitLFSCheckoutButton->set_disabled(true);

    GitLFSCheckoutButton->set_flat(true);
    GitLFSCheckoutButton->set_text_alignment(godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);
    
    GitLFSCheckoutButton->set_action_mode(godot::BaseButton::ACTION_MODE_BUTTON_PRESS);
    GitLFSCheckoutButton->connect("pressed", check_out_runner);


    GitLFSCheckinButton->set_h_size_flags(godot::Control::SizeFlags::SIZE_EXPAND_FILL);
    GitLFSCheckinButton->set_text("Check In Asset");
    //GitLFSCheckinButton->(godot::Color(255, 255, 255));
    GitLFSCheckinButton->set_disabled(true);

    GitLFSCheckinButton->set_flat(true);
    GitLFSCheckinButton->set_text_alignment(godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_CENTER);
    
    GitLFSCheckinButton->set_action_mode(godot::BaseButton::ACTION_MODE_BUTTON_PRESS);
    GitLFSCheckinButton->connect("pressed", check_in_runner);

    _update_elements();
}

void GitLFSControl::_update_elements()
{
    godot::UtilityFunctions::print("Updating elements.");
    ainfo_mutex->lock();
    if (update_thread->is_alive())
    {
        update_thread->wait_to_finish();
    }
    update_thread->call_deferred("start", update_runner_caller);
    //update_runner->start(update_runner_caller, godot::Thread::PRIORITY_LOW);
}

void GitLFSControl::_check_out_asset()
{
    godot::UtilityFunctions::print("Checking out asset.");
    ainfo_mutex->lock();
    update_thread->wait_to_finish();
    update_thread->call_deferred("start", check_out_runner_impl);
    //update_runner->start(check_out_runner_impl, godot::Thread::PRIORITY_LOW);
}

void GitLFSControl::_check_out_asset_impl()
{
    godot::UtilityFunctions::print("Checking out IMPL");
    CheckOutAsset(ainfo);
    update_elements_impl();
}

void GitLFSControl::_check_in_asset()
{
    godot::UtilityFunctions::print("Checking in asset.");
    ainfo_mutex->lock();
    update_thread->wait_to_finish();
    update_thread->call_deferred("start", check_in_runner_impl);
}

void GitLFSControl::_check_in_asset_impl()
{
    CheckInAsset(ainfo);
    update_elements_impl();
}

void GitLFSControl::update_elements_impl()
{
    godot::String asset = ainfo.asset_path;
    ainfo = GetAssetInfo(asset);

    godot::UtilityFunctions::print("CHECKING STATUS:");
    godot::UtilityFunctions::print("status precheck: ", godot::String(LFSStateToString(ainfo.asset_state)));
    if (ainfo.asset_state == LFSState::CheckedIn)
    {
        godot::UtilityFunctions::print("Status is CheckedIn");
        GitLFSCheckoutButton->call_deferred("set_disabled", false);
        statusDisplay->call_deferred("set_text", "Checked In");
        GitLFSCheckoutButton->call_deferred("set_flat", false);

        GitLFSCheckinButton->call_deferred("set_disabled", true);
        GitLFSCheckinButton->call_deferred("set_flat", true);
    }
    if (ainfo.asset_state == LFSState::CheckedOut)
    {
        godot::UtilityFunctions::print("Status is CheckedOut");
        GitLFSCheckoutButton->call_deferred("set_disabled", true);
        statusDisplay->call_deferred("set_text", "Checked Out");
        GitLFSCheckoutButton->call_deferred("set_flat", true);

        GitLFSCheckinButton->call_deferred("set_disabled", false);
        GitLFSCheckinButton->call_deferred("set_flat", false);
    }
    if (ainfo.asset_state == LFSState::NotLFS)
    {
        godot::UtilityFunctions::print("Status is NotLFS");
        GitLFSCheckoutButton->call_deferred("set_disabled", true);
        statusDisplay->call_deferred("set_text", "Not LFS Tracked");
        GitLFSCheckoutButton->call_deferred("set_flat", true);

        GitLFSCheckinButton->call_deferred("set_disabled", true);
        GitLFSCheckinButton->call_deferred("set_flat", true);
    }
    if (ainfo.asset_state == LFSState::Unknown)
    {
        godot::UtilityFunctions::print("Status is Unknown");
        GitLFSCheckoutButton->call_deferred("set_disabled", true);
        statusDisplay->call_deferred("set_text", "Unknown");
        GitLFSCheckoutButton->call_deferred("set_flat", true);

        GitLFSCheckinButton->call_deferred("set_disabled", true);
        GitLFSCheckinButton->call_deferred("set_flat", true);
    }
    if (ainfo.asset_state == LFSState::FailedCheckOut)
    {
        godot::UtilityFunctions::print("Status is CheckedIn");
        GitLFSCheckoutButton->call_deferred("set_disabled", false);
        statusDisplay->call_deferred("set_text", "Checkout Failed");
        GitLFSCheckoutButton->call_deferred("set_flat", false);

        GitLFSCheckinButton->call_deferred("set_disabled", true);
        GitLFSCheckinButton->call_deferred("set_flat", true);
    }
    
    ainfo_mutex->unlock();
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
