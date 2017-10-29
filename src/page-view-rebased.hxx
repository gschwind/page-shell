/*
 * Copyright (2017) Benoit Gschwind
 *
 * view_rebased.hxx is part of page-compositor.
 *
 * page-compositor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * page-compositor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with page-compositor.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SRC_VIEW_REBASED_HXX_
#define SRC_VIEW_REBASED_HXX_

#include "page-view.hxx"

namespace page {

struct view_rebased_t :
		public view_t
{

public:
	view_rebased_t(tree_t * ref, client_managed_p client);
	view_rebased_t(view_rebased_t * src);
	virtual ~view_rebased_t();

	auto shared_from_this() -> view_rebased_p;
	void _create_base_windows();
	void _reconfigure_windows();
	void _on_focus_change(client_managed_t * c);
	/**
	 * view_t API
	 **/

	using view_t::xxactivate;
	using view_t::remove_this_view;
	using view_t::acquire_client;
	using view_t::release_client;
	virtual void set_focus_state(bool is_focused) override;

	/**
	 * tree_t virtual API
	 **/

	using view_t::hide;
	using view_t::show;
	using view_t::get_node_name;
	using view_t::remove;

	virtual void reconfigure() = 0;
	virtual void on_workspace_enable() override;
	virtual void on_workspace_disable() override;

	//virtual auto button_press(xcb_button_press_event_t const * ev) -> button_action_e override;
	//virtual bool button_release(xcb_button_release_event_t const * ev);
	//virtual bool button_motion(xcb_motion_notify_event_t const * ev);
	//virtual bool leave(xcb_leave_notify_event_t const * ev);
	//virtual bool enter(xcb_enter_notify_event_t const * ev);
	//virtual void expose(xcb_expose_event_t const * ev);
	//virtual void trigger_redraw();

	//virtual auto get_toplevel_xid() const -> xcb_window_t override;
	//virtual rect get_window_position() const;
	//virtual void queue_redraw();

	virtual auto get_default_view() const -> ClutterActor *;

};

} /* namespace page */

#endif /* SRC_VIEW_REBASED_HXX_ */
