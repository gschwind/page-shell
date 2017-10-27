/*
 * Copyright (2017) Benoit Gschwind
 *
 * view_floating.cxx is part of page-compositor.
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

#include "page-view-floating.hxx"
#include "page-workspace.hxx"
#include "page-page.hxx"
#include "page-grab-handlers.hxx"

namespace page {

view_floating_t::view_floating_t(tree_t * ref, client_managed_p client) :
		view_rebased_t{ref, client}
{
	_init();
}

view_floating_t::view_floating_t(view_rebased_t * src) :
	view_rebased_t{src}
{
	_init();
}

view_floating_t::~view_floating_t()
{

}

auto view_floating_t::shared_from_this() -> view_floating_p
{
	return static_pointer_cast<view_floating_t>(tree_t::shared_from_this());
}

void view_floating_t::_init()
{
	g_connect(_client->meta_window(), "position-changed", &view_floating_t::_handler_position_changed);
	g_connect(_client->meta_window(), "size-changed", &view_floating_t::_handler_size_changed);

	auto _ctx = _root->_ctx;

	g_object_set(G_OBJECT(_client->meta_window_actor()), "no-shadow", FALSE, NULL);

	MetaRectangle xrect;
	meta_window_get_frame_rect(_client->_meta_window, &xrect);
	_client->_floating_wished_position = rect(xrect.x, xrect.y, xrect.width,
			xrect.height);

	// if x == 0 then place window at center of the screen
	if (_client->_floating_wished_position.x == 0) {
		_client->_floating_wished_position.x =
				(_root->primary_viewport()->raw_area().w
						- _client->_floating_wished_position.w) / 2;
	}

	// if y == 0 then place window at center of the screen
	if (_client->_floating_wished_position.y == 0) {
		_client->_floating_wished_position.y =
				(_root->primary_viewport()->raw_area().h
						- _client->_floating_wished_position.h) / 2;
	}


	_client->_absolute_position = _client->_floating_wished_position;

}

void view_floating_t::_handler_position_changed(MetaWindow * window)
{
	MetaRectangle xrect;
	meta_window_get_frame_rect(_client->_meta_window, &xrect);
	_client->_floating_wished_position = rect(xrect.x, xrect.y, xrect.width,
			xrect.height);
}

void view_floating_t::_handler_size_changed(MetaWindow * window)
{
	MetaRectangle xrect;
	meta_window_get_frame_rect(_client->_meta_window, &xrect);
	_client->_floating_wished_position = rect(xrect.x, xrect.y, xrect.width,
			xrect.height);
}

void view_floating_t::remove_this_view()
{
	view_t::remove_this_view();
	_root->_ctx->schedule_repaint();
}

void view_floating_t::set_focus_state(bool is_focused)
{
	view_rebased_t::set_focus_state(is_focused);

	region r = _base_position;
	r -= _client->_absolute_position;
	_root->_ctx->schedule_repaint();
}

void view_floating_t::reconfigure() {
	//printf("call %s\n", __PRETTY_FUNCTION__);

	auto _ctx = _root->_ctx;
	auto _dpy = _root->_ctx->dpy();

	_client->_absolute_position = _client->_floating_wished_position;

	if (false) {
		_base_position.x = _client->_absolute_position.x
				- _ctx->theme()->floating.margin.left;
		_base_position.y = _client->_absolute_position.y - _ctx->theme()->floating.margin.top - _ctx->theme()->floating.title_height;
		_base_position.w = _client->_absolute_position.w + _ctx->theme()->floating.margin.left
				+ _ctx->theme()->floating.margin.right;
		_base_position.h = _client->_absolute_position.h + _ctx->theme()->floating.margin.top
				+ _ctx->theme()->floating.margin.bottom + _ctx->theme()->floating.title_height;

		_orig_position.x = _ctx->theme()->floating.margin.left;
		_orig_position.y = _ctx->theme()->floating.margin.top + _ctx->theme()->floating.title_height;
		_orig_position.w = _client->_absolute_position.w;
		_orig_position.h = _client->_absolute_position.h;

	} else {
		_base_position.x = _client->_absolute_position.x;
		_base_position.y = _client->_absolute_position.y;
		_base_position.w = _client->_absolute_position.w;
		_base_position.h = _client->_absolute_position.h;

		_orig_position.x = 0;
		_orig_position.y = 0;
		_orig_position.w = _client->_absolute_position.w;
		_orig_position.h = _client->_absolute_position.h;
	}

	if (meta_window_is_tiled_with_custom_position(_client->meta_window()))
		meta_window_unmake_tiled_with_custom_position(_client->meta_window());
	_reconfigure_windows();

}

} /* namespace page */
