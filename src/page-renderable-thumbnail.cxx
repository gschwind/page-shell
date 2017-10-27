/*
 * Copyright (2017) Benoit Gschwind
 *
 * renderable_thumbnail.cxx is part of page-compositor.
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

#include "page-renderable-thumbnail.hxx"

#include "page-page.hxx"
#include "page-workspace.hxx"
#include "page-view.hxx"

namespace page {

renderable_thumbnail_t::renderable_thumbnail_t(tree_t * ref, view_p c, rect const & target_position, thumnail_anchor_e target_anchor) :
	tree_t{ref->_root},
	_ctx{ref->_root->_ctx},
	_c{c},
	_title_width{0},
	_is_mouse_over{false},
	_ratio{1.0},
	_target_position{target_position},
	_target_anchor{target_anchor},
	_client_view{nullptr}
{

	auto meta_window_actor = c->_client->meta_window_actor();
	_client_view = clutter_clone_new(CLUTTER_ACTOR(meta_window_actor));
	clutter_actor_add_child(_ctx->_overlay_group, _client_view);
	update_layout();

}

renderable_thumbnail_t::~renderable_thumbnail_t() {
	clutter_actor_remove_child(_ctx->_overlay_group, _client_view);
	_ctx->schedule_repaint();
}

/** @return scale factor */
double renderable_thumbnail_t::fit_to(double target_width, double target_height, double src_width, double src_height) {
	double x_ratio = target_width / src_width;
	double y_ratio = target_height / src_height;
	if (x_ratio < y_ratio) {
		return x_ratio;
	} else {
		return y_ratio;
	}
}

auto renderable_thumbnail_t::target_position() -> rect const &
{
	return _target_position;
}

rect renderable_thumbnail_t::get_real_position() {
	return rect{_thumbnail_position.x, _thumbnail_position.y, _thumbnail_position.w, _thumbnail_position.h + 20};
}

void renderable_thumbnail_t::renderable_thumbnail_t::set_mouse_over(bool x) {
	_is_mouse_over = x;
}

void renderable_thumbnail_t::update_title() {
//	_tt.title = make_shared<pixmap_t>(_ctx->dpy(), PIXMAP_RGB, _thumbnail_position.w, 20);
//	cairo_t * cr = cairo_create(_tt.title->get_cairo_surface());
//	_ctx->theme()->render_thumbnail_title(cr, rect{0 + 3, 0, _thumbnail_position.w - 6, 20}, _c.lock()->_client->title());
//	cairo_destroy(cr);
}


void renderable_thumbnail_t::move_to(rect const & target_position) {
	_target_position = target_position;
	clutter_actor_set_position(_client_view, target_position.x, target_position.y);
	update_title();
}

void renderable_thumbnail_t::update_layout() {
	if(_c.expired())
		return;

	auto c = _c.lock();

	int src_width = clutter_actor_get_width(CLUTTER_ACTOR(c->_client->meta_window_actor()));
	int src_height = clutter_actor_get_height(CLUTTER_ACTOR(c->_client->meta_window_actor()));

	_ratio = fit_to(_target_position.w, _target_position.h, src_width, src_height);

	_thumbnail_position = rect(0, 0, src_width  * _ratio, src_height * _ratio);

	switch(_target_anchor) {
	case ANCHOR_TOP:
	case ANCHOR_TOP_LEFT:
	case ANCHOR_TOP_RIGHT:
		_thumbnail_position.y = _target_position.y;
		break;
	case ANCHOR_LEFT:
	case ANCHOR_CENTER:
	case ANCHOR_RIGHT:
		_thumbnail_position.y = _target_position.y + ((_target_position.h - 20) - src_height * _ratio) / 2.0;
		break;
	case ANCHOR_BOTTOM:
	case ANCHOR_BOTTOM_LEFT:
	case ANCHOR_BOTTOM_RIGHT:
		_thumbnail_position.y = _target_position.y + (_target_position.h - 20) - src_height * _ratio;
		break;
	}

	switch(_target_anchor) {
	case ANCHOR_LEFT:
	case ANCHOR_TOP_LEFT:
	case ANCHOR_BOTTOM_LEFT:
		_thumbnail_position.x = _target_position.x;
		break;
	case ANCHOR_TOP:
	case ANCHOR_BOTTOM:
	case ANCHOR_CENTER:
		_thumbnail_position.x = _target_position.x + (_target_position.w - src_width * _ratio) / 2.0;
		break;
	case ANCHOR_RIGHT:
	case ANCHOR_TOP_RIGHT:
	case ANCHOR_BOTTOM_RIGHT:
		_thumbnail_position.x = _target_position.x + _target_position.w - src_width * _ratio;
		break;
	}

	clutter_actor_set_position(_client_view, _thumbnail_position.x, _thumbnail_position.y);
	clutter_actor_set_scale(_client_view, _ratio, _ratio);

}

void renderable_thumbnail_t::show() {
	tree_t::show();
//	if (not _c.expired() and _client_view == nullptr) {
//		_client_view = _c.lock()->create_surface();
//	}
}

void renderable_thumbnail_t::hide() {
	tree_t::hide();
	_ctx->schedule_repaint();
	_client_view = nullptr;
	_tt.pix = nullptr;
	_tt.title = nullptr;
}

}


