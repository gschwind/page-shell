/*
 * page.hxx
 *
 * copyright (2010-2015) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#ifndef PAGE_HXX_
#define PAGE_HXX_

extern "C" {
#include "shell-wm.h"
}

#include <memory>
#include <list>
#include <vector>
#include <string>
#include <map>
#include <array>

#include "page-time.hxx"

#include "page-config-handler.hxx"

#include "page-key-desc.hxx"

#include "page-theme.hxx"

#include "page-client-managed.hxx"

#include "page-popup-alt-tab.hxx"
#include "page-popup-split.hxx"

#include "page-dropdown-menu.hxx"

#include "page-page-component.hxx"
#include "page-notebook.hxx"
#include "page-split.hxx"
#include "page-viewport.hxx"
#include "page-workspace.hxx"

#include "page-page.hxx"

namespace page {

using namespace std;

struct fullscreen_data_t {
	weak_ptr<client_managed_t> client;
	weak_ptr<workspace_t> workspace;
	weak_ptr<viewport_t> viewport;
	managed_window_type_e revert_type;
	weak_ptr<notebook_t> revert_notebook;
};

/* process_mode_e define possible state of page */
enum process_mode_e {
	PROCESS_NORMAL,						// default evant processing
	PROCESS_SPLIT_GRAB,					// when split is moving
	PROCESS_NOTEBOOK_GRAB,				// when notebook tab is moved
	PROCESS_NOTEBOOK_BUTTON_PRESS,		// when click on close/unbind button
	PROCESS_FLOATING_MOVE,				// when a floating window is moved
	PROCESS_FLOATING_RESIZE,			// when resizing a floating window
	PROCESS_FLOATING_CLOSE,				// when clicking close button of floating window
	PROCESS_FLOATING_BIND,				// when clicking bind button
	PROCESS_FULLSCREEN_MOVE,			// when mod4+click to change fullscreen window screen
	PROCESS_FLOATING_MOVE_BY_CLIENT,	// when moving a floating window started by client himself
	PROCESS_FLOATING_RESIZE_BY_CLIENT,	// when resizing a floating window started by client himself
	PROCESS_NOTEBOOK_MENU,				// when notebook menu is shown
	PROCESS_NOTEBOOK_CLIENT_MENU,		// when switch workspace menu is shown
	PROCESS_ALT_TAB						// when alt-tab running
};

struct key_bind_cmd_t {
	key_desc_t key;
	string cmd;
};

class page_t:
		public connectable_t,
		public g_connectable_t
{
	static uint32_t const DEFAULT_BUTTON_EVENT_MASK = XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE|XCB_EVENT_MASK_BUTTON_MOTION|XCB_EVENT_MASK_POINTER_MOTION;
	static uint32_t const ROOT_EVENT_MASK = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_FOCUS_CHANGE;
	static time64_t const default_wait;

	/** define callback function type for event handler **/
	using callback_event_t = void (page_t::*) (xcb_generic_event_t const *);

	workspace_p _current_workspace;
	vector<workspace_p> _workspace_list;

	weak_ptr<client_managed_t> _net_active_window;

	view_w _actual_focussed_client;

	pair<uint32_t, uint32_t> _current_focuced_client;

public:
	rect _root_position;

	/** used to ignore map/unmap events **/
	set<xcb_window_t> _page_windows;

private:
	shared_ptr<grab_handler_t> _grab_handler;

public:

	/** window that handle page identity for others clients */
	xcb_window_t identity_window;

	MetaPlugin * _plugin;
	MetaDisplay * _display;
	MetaScreen * _screen;
	theme_t * _theme;

	ClutterActor * _viewport_group;
	ClutterActor * _overlay_group;

	page_configuration_t configuration;

	config_handler_t _conf;

	list<xcb_atom_t> supported_list;

	string page_base_dir;
	string _theme_engine;

	key_desc_t bind_page_quit;
	key_desc_t bind_toggle_fullscreen;
	key_desc_t bind_toggle_compositor;
	key_desc_t bind_close;

	key_desc_t bind_exposay_all;

	key_desc_t bind_right_workspace;
	key_desc_t bind_left_workspace;

	key_desc_t bind_bind_window;
	key_desc_t bind_fullscreen_window;
	key_desc_t bind_float_window;

	key_desc_t bind_debug_1;
	key_desc_t bind_debug_2;
	key_desc_t bind_debug_3;
	key_desc_t bind_debug_4;

	array<key_bind_cmd_t, 10> bind_cmd;

	bool _schedule_repaint;
	uint32_t frame_alarm;

private:

	xcb_timestamp_t _last_focus_time;
	xcb_timestamp_t _last_button_press;

	/** store all client in mapping order, older first **/
	list<client_managed_p> _net_client_list;
	list<view_w> _global_focus_history;

	int _left_most_border;
	int _top_most_border;

	using key_handler_func = void (page_t::*)(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);

	struct _handler_key_binding {
		page_t * target;
		key_handler_func func;

		_handler_key_binding(page_t * target, key_handler_func func) :
			target{target}, func{func} { }

		static void call(MetaDisplay * display, MetaScreen * screen,
				MetaWindow * window, ClutterKeyEvent * event,
				MetaKeyBinding * binding, gpointer user_data);
	};

	void add_keybinding_helper(GSettings * settings, char const * name, key_handler_func func);
	void set_keybinding_custom_helper(char const * name, key_handler_func func);

	void _handler_key_make_notebook_window(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_make_fullscreen_window(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_make_floating_window(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_page_quit(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_toggle_fullscreen(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_debug_1(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_debug_2(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_debug_3(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_debug_4(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_run_cmd_0(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_run_cmd_1(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_run_cmd_2(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_run_cmd_3(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);
	void _handler_key_run_cmd_4(MetaDisplay * display, MetaScreen * screen, MetaWindow * window, ClutterKeyEvent * event, MetaKeyBinding * binding);

private:
	/* do no allow copy */
	page_t(page_t const &);
	page_t &operator=(page_t const &);

public:
	page_t(MetaPlugin * cnx);
	virtual ~page_t();

	void set_default_pop(shared_ptr<notebook_t> x);


	// Plugin API

	void _handler_plugin_start();
	void _handler_plugin_minimize(ShellWM * wm, MetaWindowActor * actor);
	void _handler_plugin_unminimize(ShellWM * wm, MetaWindowActor * actor);
	void _handler_plugin_size_changed(ShellWM * wm, MetaWindowActor * window_actor);
	void _handler_plugin_size_change(ShellWM * wm, MetaWindowActor * window_actor, MetaSizeChange const which_change, MetaRectangle * old_frame_rect, MetaRectangle * old_buffer_rect);
	void _handler_plugin_map(ShellWM * wm, MetaWindowActor * window_actor);
	void _handler_plugin_destroy(ShellWM * wm, MetaWindowActor * actor);
	void _handler_plugin_switch_workspace(ShellWM * wm, gint from, gint to, MetaMotionDirection direction);
	void _handler_plugin_show_tile_preview(ShellWM * wm, MetaWindow * window, MetaRectangle *tile_rect, int tile_monitor_number);
	void _handler_plugin_hide_tile_preview(ShellWM * wm);
	void _handler_plugin_show_window_menu(ShellWM * wm, MetaWindow * window, MetaWindowMenuType menu, int x, int y);
	void _handler_plugin_show_window_menu_for_rect(ShellWM * wm, MetaWindow * window, MetaWindowMenuType menu, MetaRectangle * rect);
	void _handler_plugin_kill_window_effects(ShellWM * wm, MetaWindowActor * actor);
	void _handler_plugin_kill_switch_workspace(ShellWM * wm);
	auto _handler_plugin_xevent_filter(ShellWM * wm, XEvent * event) -> gboolean;
	auto _handler_plugin_keybinding_filter(ShellWM * wm, MetaKeyBinding * binding) -> gboolean;
	void _handler_plugin_confirm_display_change(ShellWM * wm);
	auto _handler_plugin_create_close_dialog(ShellWM * wm, MetaWindow * window) -> MetaCloseDialog *;
	auto _handler_plugin_create_inhibit_shortcuts_dialog(ShellWM * wm, MetaWindow * window) -> MetaInhibitShortcutsDialog *;


	/* scan current root window status, finding mapped windows */
//	void scan();

	auto _handler_stage_button_press_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;
	auto _handler_stage_button_release_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;
	auto _handler_stage_motion_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;
	auto _handler_stage_key_press_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;
	auto _handler_stage_key_release_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;

	void _handler_screen_in_fullscreen_changed(MetaScreen *metascreen);
	void _handler_screen_monitors_changed(MetaScreen * screen);
	void _handler_screen_restacked(MetaScreen * screen);
	void _handler_screen_startup_sequence_changed(MetaScreen * screen, gpointer arg1);
	void _handler_screen_window_entered_monitor(MetaScreen *metascreen, gint arg1, MetaWindow *arg2);
	void _handler_screen_window_left_monitor(MetaScreen *metascreen, gint arg1, MetaWindow *arg2);
	void _handler_screen_workareas_changed(MetaScreen * screen);
	void _handler_screen_workspace_added(MetaScreen * screen, gint arg1);
	void _handler_screen_workspace_removed(MetaScreen * screen, gint arg1);
	void _handler_screen_workspace_switched(MetaScreen * screen, gint arg1, gint arg2, MetaMotionDirection arg3);

	void _handler_meta_window_focus(MetaWindow * window);
	void _handler_window_unmanaged(MetaWindow * window);

	void _handler_meta_display_accelerator_activated(MetaDisplay * metadisplay, guint arg1, guint arg2, guint arg3);
	void _handler_meta_display_grab_op_begin(MetaDisplay * metadisplay, MetaScreen * arg1, MetaWindow * arg2, MetaGrabOp arg3);
	void _handler_meta_display_grab_op_end(MetaDisplay * metadisplay, MetaScreen * arg1, MetaWindow * arg2, MetaGrabOp arg3);
	auto _handler_meta_display_modifiers_accelerator_activated(MetaDisplay * display) -> gboolean;
	void _handler_meta_display_overlay_key(MetaDisplay * display);
	auto _handler_meta_display_restart(MetaDisplay * display) -> gboolean;
	void _handler_meta_display_window_created(MetaDisplay * display, MetaWindow * window);

	/* unmanage a managed window */
	void unmanage(client_managed_p mw);

	/* toggle fullscreen */
	void toggle_fullscreen(view_p c, xcb_timestamp_t time);

	void move_view_to_notebook(view_p v, notebook_p n, xcb_timestamp_t time);
	void move_notebook_to_notebook(view_notebook_p v, notebook_p n, xcb_timestamp_t time);
	void move_floating_to_notebook(view_floating_p v, notebook_p n, xcb_timestamp_t time);

	/* split a notebook into two notebook */
	void split(shared_ptr<notebook_t> nbk, split_type_e type);

	/* compute the allocation of viewport taking in account DOCKs */
	void compute_viewport_allocation(workspace_p d, viewport_p v);

	void cleanup_not_managed_client(client_managed_p c);

	void process_net_vm_state_client_message(xcb_window_t c, long type, xcb_atom_t state_properties);

	void insert_as_floating(client_managed_p c, xcb_timestamp_t time = XCB_CURRENT_TIME);
	void insert_as_fullscreen(client_managed_p c, xcb_timestamp_t time = XCB_CURRENT_TIME);
	void insert_as_notebook(client_managed_p c, xcb_timestamp_t time = XCB_CURRENT_TIME);

	client_managed_p get_transient_for(client_managed_p c);

	/* TODO: replace it,
	 * temporarly gather all tree_t */
	vector<shared_ptr<tree_t>> get_all_children() const;

	/* attach floating window in a notebook */

	void grab_pointer();
	/* if grab is linked to a given window remove this grab */
	void cleanup_grab();
	/* find a valid notebook, that is in subtree base and that is no nbk */
	notebook_p get_another_notebook(shared_ptr<tree_t> base, shared_ptr<tree_t> nbk);
	static viewport_p find_viewport_of(shared_ptr<tree_t> n);
	static workspace_p find_workspace_of(shared_ptr<tree_t> n);
	void set_window_cursor(xcb_window_t w, xcb_cursor_t c);
	void update_viewport_layout();
	void remove_viewport(shared_ptr<workspace_t> d, shared_ptr<viewport_t> v);

	void ackwoledge_configure_request(xcb_configure_request_event_t const * e);
	//void create_unmanaged_window(client_proxy_p proxy);
	void update_page_areas();
	void set_workspace_geometry(long width, long height);

	auto lookup_client_managed_with(MetaWindow * w) const -> client_managed_p;
	auto lookup_client_managed_with(MetaWindowActor * actor) const -> client_managed_p;
	auto lookup_workspace(MetaWorkspace * w) const -> workspace_p;

	void raise_child(shared_ptr<tree_t> t);
	void process_notebook_client_menu(shared_ptr<client_managed_t> c, int selected);

	void check_x11_extension();

	void render(cairo_t * cr, time64_t time);
	bool need_render(time64_t time);

	bool check_for_managed_window(xcb_window_t w);
	bool check_for_destroyed_window(xcb_window_t w);

	shared_ptr<client_managed_t> find_hidden_client_with(xcb_window_t w);

	void render();

	/** debug function that try to print the state of page in stdout **/
	void print_state() const;
	void switch_to_workspace(unsigned int workspace, xcb_timestamp_t time);
	void start_switch_to_workspace_animation(workspace_p workspace);
	void update_fullscreen_clients_position();
	void update_workspace_visibility(xcb_timestamp_t time);
	void process_error(xcb_generic_event_t const * e);
	void start_compositor();
	void stop_compositor();
	void run_cmd(string const & cmd_with_args);

	void start_alt_tab(xcb_timestamp_t time);

	void reconfigure_docks(shared_ptr<workspace_t> const & d);

	void mark_durty(shared_ptr<tree_t> t);

	void process_pending_events();

	bool global_focus_history_front(view_p & out);
	void global_focus_history_remove(view_p in);
	void global_focus_history_move_front(view_p in);
	bool global_focus_history_is_empty();

	/**
	 * page_t virtual API
	 **/

	auto conf() const -> page_configuration_t const &;
	auto theme() const -> theme_t const *;
	auto dpy() const -> MetaDisplay *;
	void overlay_add(shared_ptr<tree_t> x);
	auto current_workspace() const -> workspace_p const &;
	auto get_workspace(int id) const -> workspace_p const &;
	int  get_workspace_count() const;
	void create_workspace(guint time);
	void grab_start(shared_ptr<grab_handler_t> handler, guint32 time);
	void grab_stop(guint32 time);
	void insert_window_in_notebook(shared_ptr<client_managed_t> x, shared_ptr<notebook_t> n, bool prefer_activate);
	void move_fullscreen_to_viewport(view_fullscreen_p c, viewport_p v);
	void split_left(notebook_p nbk, view_p c, xcb_timestamp_t time);
	void split_right(notebook_p nbk, view_p c, xcb_timestamp_t time);
	void split_top(notebook_p nbk, view_p c, xcb_timestamp_t time);
	void split_bottom(notebook_p nbk, view_p c, xcb_timestamp_t time);
	void apply_focus(xcb_timestamp_t tfocus);
	void notebook_close(shared_ptr<notebook_t> nbk, xcb_timestamp_t time);
	int  left_most_border();
	int  top_most_border();
	auto global_client_focus_history() -> list<view_w>;
	auto net_client_list() -> list<client_managed_p> const &;
	void make_surface_stats(int & size, int & count);
	void schedule_repaint();
	void damage_all();

	void activate(view_p c, xcb_timestamp_t time);
	void sync_tree_view();

	bool has_grab_handler();

};


}



#endif /* PAGE_HXX_ */
