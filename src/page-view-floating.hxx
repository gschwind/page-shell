/*
 * Copyright (2017) Benoit Gschwind
 *
 * view_floating.hxx is part of page-compositor.
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

#ifndef SRC_VIEW_FLOATING_HXX_
#define SRC_VIEW_FLOATING_HXX_

#include "page-page-types.hxx"
#include "page-view-rebased.hxx"
extern "C" {
#include "meta/meta-plugin.h"
}

namespace page {

struct view_floating_t : public view_rebased_t {

	view_floating_t(tree_t * ref, client_managed_p client);
	view_floating_t(view_rebased_t * src);
	virtual ~view_floating_t();

	auto shared_from_this() -> view_floating_p;

	void _init();

	void _handler_position_changed(MetaWindow * window);
	void _handler_size_changed(MetaWindow * window);
	/**
	 * view_t API
	 **/

	using view_t::xxactivate;
	virtual void remove_this_view() override;
	using view_rebased_t::acquire_client;
	using view_rebased_t::release_client;
	void set_focus_state(bool is_focused) override;

	/**
	 * tree_t virtual API
	 **/

	using view_t::hide;
	using view_t::show;
	//virtual auto get_node_name() const -> string;
	//virtual void remove(shared_ptr<tree_t> t);

	virtual void reconfigure() override;
	using view_rebased_t::on_workspace_enable;
	using view_rebased_t::on_workspace_disable;

	//virtual auto button_press(xcb_button_press_event_t const * ev)  -> button_action_e override;
	//virtual bool button_release(xcb_button_release_event_t const * ev);
	//virtual bool button_motion(xcb_motion_notify_event_t const * ev);
	//virtual bool leave(xcb_leave_notify_event_t const * ev);
	//virtual bool enter(xcb_enter_notify_event_t const * ev);
	//virtual void expose(xcb_expose_event_t const * ev) override;
	//virtual void trigger_redraw() override;

	//using view_rebased_t::get_toplevel_xid;
	//virtual rect get_window_position() const;
	//virtual void queue_redraw() override;
};

} /* namespace page */

#endif /* SRC_VIEW_FLOATING_HXX_ */
