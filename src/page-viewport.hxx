/*
 * viewport.hxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#ifndef VIEWPORT_HXX_
#define VIEWPORT_HXX_

#include <memory>
#include <vector>

#include "page-split.hxx"
#include "page-theme.hxx"
#include "page-page-component.hxx"
#include "page-notebook.hxx"
#include "page-page-types.hxx"

namespace page {

using namespace std;

class viewport_t:
		public page_component_t
{

	/** rendering tabs is time consuming, thus use back buffer **/
	ClutterContent * _canvas;
	ClutterActor * _default_view;

	/** area without considering dock windows **/
	rect _raw_aera;

	/** area considering dock windows **/
	rect _effective_area;
	rect _page_area;

	shared_ptr<page_component_t> _subtree;

	viewport_t(viewport_t const & v) = delete;
	viewport_t & operator= (viewport_t const &) = delete;

	auto get_nearest_notebook() -> shared_ptr<notebook_t>;
	void set_effective_area(rect const & area);
	auto page_area() const -> rect const &;

	void update_renderable();
	void draw(ClutterCanvas * _, cairo_t * cr, int width, int height);
	void paint_expose();

	auto _handler_button_press_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;
	auto _handler_button_release_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;
	auto _handler_motion_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;
	auto _handler_enter_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;
	auto _handler_leave_event(ClutterActor * actor, ClutterEvent * event) -> gboolean;

public:

	viewport_t(tree_t * ref, rect const & area);
	virtual ~viewport_t();

	auto raw_area() const -> rect const &;
	void set_raw_area(rect const & area);

	/**
	 * tree_t virtual API
	 **/

	virtual void hide() override;
	virtual void show() override;
	virtual auto get_node_name() const -> string override;
	virtual void remove(tree_p t) override;

	virtual void reconfigure() override;
	virtual void on_workspace_enable() override;
	virtual void on_workspace_disable() override;

	//virtual bool button_press(xcb_button_press_event_t const * ev);
	//virtual bool button_release(xcb_button_release_event_t const * ev);
	//virtual bool button_motion(xcb_motion_notify_event_t const * ev);
	//virtual bool leave(xcb_leave_notify_event_t const * ev);
	//virtual bool enter(xcb_enter_notify_event_t const * ev);

	//virtual auto get_toplevel_xid() const -> xcb_window_t;
	virtual rect get_window_position() const;
	virtual void queue_redraw();

	virtual auto get_default_view() const -> ClutterActor *;

	/**
	 * page_component_t virtual API
	 **/

	virtual void set_allocation(rect const & area);
	virtual rect allocation() const;
	virtual void replace(shared_ptr<page_component_t> src, shared_ptr<page_component_t> by);
	virtual void get_min_allocation(int & width, int & height) const;


};

using viewport_p = shared_ptr<viewport_t>;
using viewport_w = weak_ptr<viewport_t>;

}

#endif /* VIEWPORT_HXX_ */
