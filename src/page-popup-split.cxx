/*
 * Copyright (2017) Benoit Gschwind
 *
 * popup_split.cxx is part of page-compositor.
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

#include "page-popup-split.hxx"

#include "page-page.hxx"


namespace page {

void popup_split_t::show() {
	tree_t::show();
}

rect const & popup_split_t::position() {
	return _position;
}

popup_split_t::popup_split_t(tree_t * ref, shared_ptr<split_t> split) :
	tree_t{ref->_root},
	_ctx{ref->_root->_ctx},
	_s_base{split},
	_current_split{split->ratio()},
	_position{split->to_root_position(split->allocation())},
	_exposed{true}
{
	ClutterColor c{128u, 128u, 128u, 128u};
	_actor[0] = clutter_actor_new();
	clutter_actor_set_background_color(_actor[0], &c);

	_actor[1] = clutter_actor_new();
	clutter_actor_set_background_color(_actor[1], &c);

	clutter_actor_add_child(_ctx->_overlay_group, _actor[0]);
	clutter_actor_add_child(_ctx->_overlay_group, _actor[1]);

	clutter_actor_show(_actor[0]);
	clutter_actor_show(_actor[1]);

}

popup_split_t::~popup_split_t() {
//	xcb_destroy_window(_ctx->dpy()->xcb(), _wid);
//	_root->_ctx->_page_windows.erase(_wid);
	clutter_actor_remove_child(_ctx->_overlay_group, _actor[0]);
	clutter_actor_remove_child(_ctx->_overlay_group, _actor[1]);

	// Not needed ??
	//clutter_actor_destroy(_actor[0]);
	//clutter_actor_destroy(_actor[1]);
}

void popup_split_t::update_layout() {

	rect rect0;
	rect rect1;

	_s_base.lock()->compute_children_allocation(_current_split, rect0, rect1);

	array<xcb_rectangle_t, 8> rects;

	rects[0].x = rect0.x;
	rects[0].y = rect0.y;
	rects[0].width = rect0.w;
	rects[0].height = border_width;

	rects[1].x = rect0.x;
	rects[1].y = rect0.y;
	rects[1].width = border_width;
	rects[1].height = rect0.h;

	rects[2].x = rect0.x;
	rects[2].y = rect0.y+rect0.h-border_width;
	rects[2].width = rect0.w;
	rects[2].height = border_width;

	rects[3].x = rect0.x+rect0.w-border_width;
	rects[3].y = rect0.y;
	rects[3].width = border_width;
	rects[3].height = rect0.h;

	rects[4].x = rect1.x;
	rects[4].y = rect1.y;
	rects[4].width = rect1.w;
	rects[4].height = border_width;

	rects[5].x = rect1.x;
	rects[5].y = rect1.y;
	rects[5].width = border_width;
	rects[5].height = rect1.h;

	rects[6].x = rect1.x;
	rects[6].y = rect1.y+rect1.h-border_width;
	rects[6].width = rect1.w;
	rects[6].height = border_width;

	rects[7].x = rect1.x+rect1.w-border_width;
	rects[7].y = rect1.y;
	rects[7].width = border_width;
	rects[7].height = rect1.h;

	rect inner0(rect0.x+20, rect0.y+20, rect0.w-40, rect0.h-40);
	rect inner1(rect1.x+20, rect1.y+20, rect1.w-40, rect1.h-40);

//	if (inner0.w > 0 and inner0.h > 0)
//		_visible_region -= inner0;
//	if (inner1.w > 0 and inner1.h > 0)
//		_visible_region -= inner1;

	clutter_actor_set_position(_actor[0], rect0.x, rect0.y);
	clutter_actor_set_size(_actor[0], rect0.w, rect0.h);
	clutter_actor_set_position(_actor[1], rect1.x, rect1.y);
	clutter_actor_set_size(_actor[1], rect1.w, rect1.h);

}

void popup_split_t::set_position(double pos) {
	// avoid too mutch updates
	if(std::fabs(_current_split - pos) < 10e-4)
		return;
	_current_split = pos;
	update_layout();
}

}


