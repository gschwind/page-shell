/*
 * grab_handlers.hxx
 *
 *  Created on: 24 juin 2015
 *      Author: gschwind
 */

#ifndef SRC_GRAB_HANDLERS_HXX_
#define SRC_GRAB_HANDLERS_HXX_

#include "page-split.hxx"
#include "page-workspace.hxx"
#include "page-popup-split.hxx"


namespace page {

using namespace std;

enum notebook_area_e {
	NOTEBOOK_AREA_NONE,
	NOTEBOOK_AREA_TAB,
	NOTEBOOK_AREA_TOP,
	NOTEBOOK_AREA_BOTTOM,
	NOTEBOOK_AREA_LEFT,
	NOTEBOOK_AREA_RIGHT,
	NOTEBOOK_AREA_CENTER
};

struct grab_default_t : public grab_handler_t {
	page_t * _ctx;

	grab_default_t(page_t * c);

	virtual ~grab_default_t();
	virtual void button_press(ClutterEvent const * e) override;
	virtual void button_motion(ClutterEvent const * e) override;
	virtual void button_release(ClutterEvent const * e) override;
	virtual void key_press(ClutterEvent const * ev) override;
	virtual void key_release(ClutterEvent const * ev) override;
};

class grab_split_t : public grab_default_t {
	weak_ptr<split_t> _split;
	rect _slider_area;
	rect _split_root_allocation;
	double _split_ratio;
	shared_ptr<popup_split_t> _ps;

public:
	grab_split_t(page_t * ctx, shared_ptr<split_t> s);

	virtual ~grab_split_t();
	virtual void button_press(ClutterEvent const * e) override;
	virtual void button_motion(ClutterEvent const * e) override;
	virtual void button_release(ClutterEvent const * e) override;
	using grab_handler_t::key_press;
	using grab_handler_t::key_release;

};

class grab_bind_view_notebook_t : public grab_default_t {
	workspace_p workspace;
	view_notebook_w c;

	rect start_position;
	xcb_button_t _button;
	notebook_area_e zone;
	notebook_w target_notebook;
	ClutterActor * pn0;

	void _find_target_notebook(int x, int y, notebook_p & target, notebook_area_e & zone);

public:

	grab_bind_view_notebook_t(page_t * ctx, view_notebook_p c, xcb_button_t button, rect const & pos);

	virtual ~grab_bind_view_notebook_t();
	virtual void button_press(ClutterEvent const * e) override;
	virtual void button_motion(ClutterEvent const * e) override;
	virtual void button_release(ClutterEvent const * e) override;
	using grab_handler_t::key_press;
	using grab_handler_t::key_release;
};

class grab_bind_view_floating_t : public grab_default_t {
	view_floating_w c;

	rect start_position;
	xcb_button_t _button;
	notebook_area_e zone;
	notebook_w target_notebook;

	void _find_target_notebook(int x, int y, notebook_p & target, notebook_area_e & zone);

public:

	grab_bind_view_floating_t(page_t * ctx, view_floating_p c, xcb_button_t button, rect const & pos);

	virtual ~grab_bind_view_floating_t();
	virtual void button_press(ClutterEvent const * e) override;
	virtual void button_motion(ClutterEvent const * e) override;
	virtual void button_release(ClutterEvent const * e) override;
	using grab_handler_t::key_press;
	using grab_handler_t::key_release;
};

struct mode_data_notebook_client_menu_t  : public grab_default_t {
	weak_ptr<notebook_t> from;
	weak_ptr<client_managed_t> client;
	bool active_grab;
	rect b;

	mode_data_notebook_client_menu_t(page_t * ctx) : grab_default_t{ctx} {
		reset();
	}

	void reset() {
		from.reset();
		client.reset();
		active_grab = false;
	}

};

enum resize_mode_e {
	RESIZE_NONE,
	RESIZE_TOP_LEFT,
	RESIZE_TOP,
	RESIZE_TOP_RIGHT,
	RESIZE_LEFT,
	RESIZE_RIGHT,
	RESIZE_BOTTOM_LEFT,
	RESIZE_BOTTOM,
	RESIZE_BOTTOM_RIGHT
};

}


#endif /* SRC_GRAB_HANDLERS_HXX_ */
