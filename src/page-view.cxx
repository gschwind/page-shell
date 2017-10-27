/*
 * Copyright (2017) Benoit Gschwind
 *
 * view.cxx is part of page-compositor.
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

#include "page-view.hxx"

#include "page-tree.hxx"
#include "page-client-managed.hxx"
#include "page-workspace.hxx"
#include "page-page.hxx"
#include "page-view-floating.hxx"

namespace page {

view_t::view_t(tree_t * ref, client_managed_p client) :
	tree_t{ref->_root},
	_client{client}
{
	//printf("create %s\n", __PRETTY_FUNCTION__);

	_stack_is_locked = true;

//	_client->net_wm_state_remove(_NET_WM_STATE_FOCUSED);
	//_grab_button_unfocused_unsafe();

}

view_t::~view_t()
{
	release_client();
}

auto view_t::shared_from_this() -> view_p
{
	return static_pointer_cast<view_t>(tree_t::shared_from_this());
}

void view_t::focus(xcb_timestamp_t t) {
	_client->focus(t);
}

void view_t::move_all_window()
{
	if (not _is_client_owner())
		return;

	auto _ctx = _root->_ctx;
	auto _dpy = _root->_ctx->dpy();

	if (_root->is_enable() and _is_visible) {
		meta_window_unminimize(_client->_meta_window);
		meta_window_move_resize_frame(_client->_meta_window, FALSE,
				_client->_absolute_position.x, _client->_absolute_position.y,
				_client->_absolute_position.w, _client->_absolute_position.h);
	} else {
		log::printf("minimize %p\n", _client->meta_window());
		meta_window_minimize(_client->_meta_window);
	}

}

bool view_t::_is_client_owner()
{
	return _client->current_owner_view() == this;
}

void view_t::xxactivate(xcb_timestamp_t time)
{
	raise();
	_root->set_focus(shared_from_this(), time);
}

void view_t::remove_this_view()
{
	assert(_parent != nullptr);
	_parent->remove(shared_from_this());
}

void view_t::acquire_client()
{
	_client->acquire(this);
}

void view_t::release_client()
{
	_client->release(this);
}

void view_t::set_focus_state(bool is_focused)
{
//	_client->_has_focus = is_focused;
//	if (_client->_has_focus) {
//		_client->net_wm_state_add(_NET_WM_STATE_FOCUSED);
//	} else {
//		_client->net_wm_state_remove(_NET_WM_STATE_FOCUSED);
//	}
}

void view_t::reconfigure()
{
	//printf("call %s\n", __PRETTY_FUNCTION__);
	auto _ctx = _root->_ctx;

	move_all_window();

}

void view_t::on_workspace_enable()
{
	acquire_client();
	reconfigure();
}

void view_t::on_workspace_disable()
{
	if (_is_client_owner()) {
		log::printf("minimize %p\n", _client->meta_window());
		meta_window_minimize(_client->meta_window());
	}
}

void view_t::hide()
{
	tree_t::hide();
	reconfigure();
}

void view_t::show()
{
	tree_t::show();
	reconfigure();
}

auto view_t::get_node_name() const -> string {
	string s = _get_node_name<'M'>();
	ostringstream oss;

	auto id = meta_window_get_xwindow(_client->_meta_window);
	MetaRectangle rect;
	meta_window_get_frame_rect(_client->_meta_window, &rect);

	oss << s << " " << id << " " << meta_window_get_title(_client->_meta_window);

	oss << " " << rect.width << "x" << rect.height << "+" << rect.x << "+"
			<< rect.y;

	return oss.str();
}



} /* namespace page */
