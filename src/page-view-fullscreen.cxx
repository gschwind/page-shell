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

view_fullscreen_t::view_fullscreen_t(tree_t * ref, client_managed_p client) :
		view_rebased_t{ref, client}
{

}

view_fullscreen_t::~view_fullscreen_t()
{

}

auto view_fullscreen_t::shared_from_this() -> view_fullscreen_p
{
	return static_pointer_cast<view_fullscreen_t>(tree_t::shared_from_this());
}

void view_fullscreen_t::remove_this_view()
{
	view_t::remove_this_view();
}

void view_fullscreen_t::reconfigure()
{
	auto _ctx = _root->_ctx;
	auto _dpy = _root->_ctx->dpy();

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
