/*
 * Copyright (2017) Benoit Gschwind
 *
 * view_rebased.cxx is part of page-compositor.
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

#include "page-view-rebased.hxx"

#include "page-page.hxx"
#include "page-workspace.hxx"

namespace page {

view_rebased_t::view_rebased_t(tree_t * ref, client_managed_p client) :
	view_t{ref, client}
{
//	_client->_client_proxy->set_border_width(0);
//	_base = std::unique_ptr<_base_frame_t>{new _base_frame_t(_root->_ctx, _client->_client_proxy->visualid(), _client->_client_proxy->visual_depth())};
//	_base->_window->select_input(MANAGED_BASE_WINDOW_EVENT_MASK);
//	_grab_button_unsafe();
//	xcb_flush(_root->_ctx->_dpy->xcb());
}

view_rebased_t::view_rebased_t(view_rebased_t * src) :
	view_t{src->_root, src->_client}
{

}

view_rebased_t::~view_rebased_t()
{
	release_client();
}

auto view_rebased_t::shared_from_this() -> view_rebased_p
{
	return static_pointer_cast<view_rebased_t>(tree_t::shared_from_this());
}

void view_rebased_t::_reconfigure_windows()
{
	auto _ctx = _root->_ctx;
	auto _dpy = _ctx->_display;

	if(not _is_client_owner())
		return;

	meta_window_change_workspace(_client->meta_window(), _root->_meta_workspace);

	if (_is_visible and _root->is_enable()) {
		if (meta_window_is_fullscreen(_client->meta_window()))
			meta_window_unmake_fullscreen(_client->meta_window());
		meta_window_unminimize(_client->meta_window());
		meta_window_move_resize_frame(_client->_meta_window, FALSE, _client->_absolute_position.x, _client->_absolute_position.y, _client->_absolute_position.w, _client->_absolute_position.h);
		//clutter_actor_show(CLUTTER_ACTOR(_client->meta_window_actor()));
		log::printf("%s\n", _client->_absolute_position.to_string().c_str());
	} else {
		log::printf("minimize %p\n", _client->meta_window());
		meta_window_minimize(_client->meta_window());
	}

}

void view_rebased_t::_on_focus_change(client_managed_t * c)
{
//	if (_client->_has_focus) {
//		_client->net_wm_state_add(_NET_WM_STATE_FOCUSED);
//		_ungrab_button_unsafe();
//	} else {
//		_client->net_wm_state_remove(_NET_WM_STATE_FOCUSED);
//		_grab_button_unsafe();
//	}
}

void view_rebased_t::set_focus_state(bool is_focused)
{
//	view_t::set_focus_state(is_focused);
//	if (_client->_has_focus) {
//		_ungrab_button_unsafe();
//	} else {
//		_grab_button_unsafe();
//	}
}

void view_rebased_t::on_workspace_enable()
{
	acquire_client();
	reconfigure();
}

void view_rebased_t::on_workspace_disable()
{
	auto _ctx = _root->_ctx;
	auto _dpy = _root->_ctx->dpy();
	if (_is_client_owner()) {
		log::printf("minimize %p\n", _client->meta_window());
		meta_window_minimize(_client->meta_window());
	}
}

auto view_rebased_t::get_default_view() const -> ClutterActor *
{
	return CLUTTER_ACTOR(_client->_meta_window_actor);
}

} /* namespace page */
