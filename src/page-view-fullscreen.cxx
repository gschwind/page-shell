/*
 * Copyright (2017) Benoit Gschwind
 *
 * view_fullscreen.cxx is part of page-compositor.
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

#include "page-view-fullscreen.hxx"

#include "page-client-managed.hxx"
#include "page-viewport.hxx"
#include "page-workspace.hxx"
#include "page-page.hxx"
#include "page-grab-handlers.hxx"

namespace page {

view_fullscreen_t::view_fullscreen_t(client_managed_p client, viewport_p viewport) :
		view_rebased_t{viewport.get(), client},
		revert_type{MANAGED_FLOATING},
		_viewport{viewport}
{

}

view_fullscreen_t::view_fullscreen_t(view_rebased_t * src, viewport_p viewport) :
	view_rebased_t{src},
	revert_type{MANAGED_FLOATING},
	_viewport{viewport}
{

}

view_fullscreen_t::~view_fullscreen_t()
{

}

auto view_fullscreen_t::shared_from_this() -> view_fullscreen_p
{
	return static_pointer_cast<view_fullscreen_t>(tree_t::shared_from_this());
}

void view_fullscreen_t::_on_configure_request(client_managed_t * c, xcb_configure_request_event_t const * e)
{
	if (_root->is_enable())
		reconfigure();
}

void view_fullscreen_t::remove_this_view()
{
	view_t::remove_this_view();
	if (not _viewport.expired()) {
		auto viewport = _viewport.lock();
		viewport->show();
		_root->_ctx->schedule_repaint();
	}
}

void view_fullscreen_t::reconfigure()
{
	auto _ctx = _root->_ctx;
	auto _dpy = _root->_ctx->dpy();


//	if(not _viewport.expired())
//		_client->_absolute_position = _viewport.lock()->raw_area();

	_base_position.x = _client->_absolute_position.x;
	_base_position.y = _client->_absolute_position.y;
	_base_position.w = _client->_absolute_position.w;
	_base_position.h = _client->_absolute_position.h;

	_orig_position.x = 0;
	_orig_position.y = 0;
	_orig_position.w = _client->_absolute_position.w;
	_orig_position.h = _client->_absolute_position.h;

	if (not _is_client_owner())
		return;

	if (_root->is_enable() and _is_visible) {
		if (not meta_window_is_fullscreen(_client->meta_window()))
			meta_window_make_fullscreen(_client->meta_window());
	} else {
		log::printf("minimize %p\n", _client->meta_window());
		meta_window_minimize(_client->meta_window());
	}

}

} /* namespace page */
