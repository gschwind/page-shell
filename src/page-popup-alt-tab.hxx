/*
 * popup_alt_tab.hxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */


#ifndef POPUP_ALT_TAB_HXX_
#define POPUP_ALT_TAB_HXX_

#include <cairo.h>

#include <memory>

#include "page-viewport.hxx"
#include "page-icon-handler.hxx"
#include "page-renderable-thumbnail.hxx"

#include "page-client-managed.hxx"


namespace page {

using namespace std;

class cycle_window_entry_t {
	view_w client;
	string title;
	shared_ptr<renderable_thumbnail_t> _thumbnail;

public:
	cycle_window_entry_t() { }

	cycle_window_entry_t(cycle_window_entry_t const & x) :
		client{x.client},
		title{x.title},
		_thumbnail{x._thumbnail}
	{ }

	friend class popup_alt_tab_t;

};

class popup_alt_tab_t : public tree_t {
	page_t * _ctx;
	xcb_window_t _wid;
	rect _position_extern;
	rect _position_intern;
	viewport_w _viewport;

	list<cycle_window_entry_t> _client_list;
	list<cycle_window_entry_t>::iterator _selected;

	bool _is_durty;
	bool _exposed;
	bool _damaged;

	ClutterActor * _actor;
	ClutterActor * _select_actor;

	void update_backbuffer();
	void paint_exposed();

	void _init();

	void _select_from_mouse(int x, int y);

	void _reconfigure();

	void _clear_selected();

public:

	popup_alt_tab_t(tree_t * ref, list<view_p> client_list, viewport_p viewport);

	template<typename ... Args>
	static shared_ptr<popup_alt_tab_t> create(Args ... args) {
		auto ths = make_shared<popup_alt_tab_t>(args...);
		ths->_init();
		return ths;
	}

	virtual ~popup_alt_tab_t();

	rect const & position();

	view_w selected(view_w c);
	view_w selected();

	void destroy_client(client_managed_t * c);

	void grab_button_press(ClutterEvent const * ev);
	void grab_button_motion(ClutterEvent const * ev);

	/**
	 * tree_t virtual API
	 **/

	virtual void hide() override;
	virtual void show() override;
	virtual auto get_node_name() const -> string;
	// virtual void remove(shared_ptr<tree_t> t);

	//virtual bool button_press(xcb_button_press_event_t const * ev);
	//virtual bool button_release(xcb_button_release_event_t const * ev);
	//virtual bool button_motion(xcb_motion_notify_event_t const * ev);
	//virtual bool leave(xcb_leave_notify_event_t const * ev);
	//virtual bool enter(xcb_enter_notify_event_t const * ev);
	virtual void expose(xcb_expose_event_t const * ev);

	//virtual auto get_toplevel_xid() const -> xcb_window_t;
	//virtual rect get_window_position() const;
	//virtual void queue_redraw();

};

using popup_alt_tab_p = shared_ptr<popup_alt_tab_t>;
using popup_alt_tab_w = weak_ptr<popup_alt_tab_t>;

}



#endif /* POPUP_ALT_TAB_HXX_ */
