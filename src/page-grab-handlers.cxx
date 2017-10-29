/*
 * grab_handlers.cxx
 *
 *  Created on: 24 juin 2015
 *      Author: gschwind
 */


#include <iostream>

#include "page-page-types.hxx"
#include "page-page.hxx"
#include "page-grab-handlers.hxx"
#include "page-view-notebook.hxx"
#include "page-view-floating.hxx"
#include "page-view-fullscreen.hxx"

namespace page {

using namespace std;

grab_default_t::grab_default_t(page_t * c) :
	_ctx{c}
{

}

grab_default_t::~grab_default_t()
{

}

void grab_default_t::button_press(ClutterEvent const * e)
{

}

void grab_default_t::button_motion(ClutterEvent const * e)
{

}

void grab_default_t::button_release(ClutterEvent const * e)
{

}

void grab_default_t::key_press(ClutterEvent const * ev)
{

}

void grab_default_t::key_release(ClutterEvent const * e)
{
//	/* get KeyCode for Unmodified Key */
//	xcb_keysym_t k = _ctx->keymap()->get(e->detail);
//
//	if (k == 0)
//		return;
//
//	if (XK_Escape == k) {
//		_ctx->grab_stop(e->time);
//	}

}

grab_split_t::grab_split_t(page_t * ctx, shared_ptr<split_t> s) : grab_default_t{ctx}, _split{s} {
	_slider_area = s->to_root_position(s->get_split_bar_area());
	_split_ratio = s->ratio();
	_split_root_allocation = s->root_location();
	_ps = make_shared<popup_split_t>(s.get(), s);
	_ctx->overlay_add(_ps);
	_ps->show();
}

grab_split_t::~grab_split_t() {
	if(_ps != nullptr) {
		_ctx->schedule_repaint();
		_ps->detach_myself();
	}
}

void grab_split_t::button_press(ClutterEvent const * e) {
	/* ignore */
}

void grab_split_t::button_motion(ClutterEvent const * e)
{
	gfloat x, y;
	clutter_event_get_coords(e, &x, &y);
	auto button = clutter_event_get_button(e);
	auto time = clutter_event_get_time(e);

	if(_split.expired()) {
		_ctx->grab_stop(time);
		return;
	}

	if (_split.lock()->type() == VERTICAL_SPLIT) {
		_split_ratio = (x
				- _split_root_allocation.x)
				/ (double) (_split_root_allocation.w);
	} else {
		_split_ratio = (y
				- _split_root_allocation.y)
				/ (double) (_split_root_allocation.h);
	}

	_split_ratio = _split.lock()->compute_split_constaint(_split_ratio);

	_ps->set_position(_split_ratio);
	_ctx->schedule_repaint();

}

void grab_split_t::button_release(ClutterEvent const * e)
{
	gfloat x, y;
	clutter_event_get_coords(e, &x, &y);
	auto button = clutter_event_get_button(e);
	auto time = clutter_event_get_time(e);

	if(_split.expired()) {
		_ctx->grab_stop(time);
		return;
	}

	if (button == 1) {

		if (_split.lock()->type() == VERTICAL_SPLIT) {
			_split_ratio = (x
					- _split_root_allocation.x)
					/ (double) (_split_root_allocation.w);
		} else {
			_split_ratio = (y
					- _split_root_allocation.y)
					/ (double) (_split_root_allocation.h);
		}

		if (_split_ratio > 0.95)
			_split_ratio = 0.95;
		if (_split_ratio < 0.05)
			_split_ratio = 0.05;

		_split_ratio = _split.lock()->compute_split_constaint(_split_ratio);

		_split.lock()->queue_redraw();
		_split.lock()->set_split(_split_ratio);
		_ctx->grab_stop(time);
	}
}

grab_bind_view_notebook_t::grab_bind_view_notebook_t(page_t * ctx,
		view_notebook_p x, xcb_button_t button, rect const & pos) :
		grab_default_t{ctx},
		workspace{x->workspace()},
		c{x},
		start_position{pos},
		target_notebook{},
		zone{NOTEBOOK_AREA_NONE},
		pn0{},
		_button{button}
{
	pn0 = clutter_actor_new();
	ClutterColor c{128u, 128u, 128u, 128u};
	clutter_actor_set_background_color(pn0, &c);
	g_object_ref(pn0);
}

grab_bind_view_notebook_t::~grab_bind_view_notebook_t() {
	if(pn0 != nullptr) {
		if (clutter_actor_get_parent(pn0) != NULL)
			clutter_actor_remove_child(clutter_actor_get_parent(pn0), pn0);
		g_object_unref(pn0);
	}
}

void grab_bind_view_notebook_t::_find_target_notebook(int x, int y,
		notebook_p & target, notebook_area_e & zone) {

	target = nullptr;
	zone = NOTEBOOK_AREA_NONE;

	/* place the popup */
	auto ln = workspace->gather_children_root_first<notebook_t>();
	for (auto i : ln) {
		if (i->_area.tab.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_TAB;
			target = i;
			break;
		} else if (i->_area.right.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_RIGHT;
			target = i;
			break;
		} else if (i->_area.top.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_TOP;
			target = i;
			break;
		} else if (i->_area.bottom.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_BOTTOM;
			target = i;
			break;
		} else if (i->_area.left.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_LEFT;
			target = i;
			break;
		} else if (i->_area.popup_center.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_CENTER;
			target = i;
			break;
		}
	}
}

void grab_bind_view_notebook_t::button_press(ClutterEvent const * e) {

}

void grab_bind_view_notebook_t::button_motion(ClutterEvent const * e)
{
	gfloat x, y;
	clutter_event_get_coords(e, &x, &y);
	auto time = clutter_event_get_time(e);

	if(c.expired()) {
		_ctx->grab_stop(time);
		return;
	}

	/* do not start drag&drop for small move */
	if (not start_position.is_inside(x, y) and clutter_actor_get_parent(pn0) == NULL) {
		clutter_actor_insert_child_above(_ctx->_overlay_group, pn0, NULL);
		clutter_actor_show(pn0);
	}

	if (pn0 == nullptr)
		return;

	notebook_p new_target;
	notebook_area_e new_zone;
	_find_target_notebook(x, y, new_target, new_zone);


	if((new_target != target_notebook.lock() or new_zone != zone) and new_zone != NOTEBOOK_AREA_NONE) {
		target_notebook = new_target;
		zone = new_zone;
		ClutterGeometry geo{0, 0, 0u, 0u};
		switch(zone) {
		case NOTEBOOK_AREA_TAB:
			geo = new_target->_area.tab;
			break;
		case NOTEBOOK_AREA_RIGHT:
			geo = new_target->_area.popup_right;
			break;
		case NOTEBOOK_AREA_TOP:
			geo = new_target->_area.popup_top;
			break;
		case NOTEBOOK_AREA_BOTTOM:
			geo = new_target->_area.popup_bottom;
			break;
		case NOTEBOOK_AREA_LEFT:
			geo = new_target->_area.popup_left;
			break;
		case NOTEBOOK_AREA_CENTER:
			geo = new_target->_area.popup_center;
			break;
		}
		clutter_actor_save_easing_state(pn0);
		clutter_actor_set_easing_mode(pn0, CLUTTER_EASE_IN_CUBIC);
		clutter_actor_set_easing_duration(pn0, 100);
		clutter_actor_set_position(pn0, geo.x, geo.y);
		clutter_actor_set_size(pn0, geo.width, geo.height);
		clutter_actor_restore_easing_state(pn0);

	}


}

void grab_bind_view_notebook_t::button_release(ClutterEvent const * e)
{
	gfloat x, y;
	clutter_event_get_coords(e, &x, &y);
	auto button = clutter_event_get_button(e);
	auto time = clutter_event_get_time(e);

	if(c.expired()) {
		_ctx->grab_stop(time);
		return;
	}

	auto c = this->c.lock();

	if (button == _button) {
		notebook_p new_target;
		notebook_area_e new_zone;
		_find_target_notebook(x, y, new_target, new_zone);

		/* if the mouse is no where, keep old location */
		if ((new_target == nullptr or new_zone == NOTEBOOK_AREA_NONE)
				and not target_notebook.expired()) {
			new_zone = zone;
			new_target = target_notebook.lock();
		}

		if (start_position.is_inside(x, y)) {
			c->xxactivate(time);
			_ctx->grab_stop(time);
			return;
		}

		switch(zone) {
		case NOTEBOOK_AREA_TAB:
		case NOTEBOOK_AREA_CENTER:
			if(new_target != c->parent_notebook()) {
				_ctx->move_notebook_to_notebook(c, new_target, time);
			}
			break;
		case NOTEBOOK_AREA_TOP:
			_ctx->split_top(new_target, c, time);
			break;
		case NOTEBOOK_AREA_LEFT:
			_ctx->split_left(new_target, c, time);
			break;
		case NOTEBOOK_AREA_BOTTOM:
			_ctx->split_bottom(new_target, c, time);
			break;
		case NOTEBOOK_AREA_RIGHT:
			_ctx->split_right(new_target, c, time);
			break;
		default:
			c->xxactivate(time);
		}

		_ctx->grab_stop(time);

	}
}


grab_bind_view_floating_t::grab_bind_view_floating_t(page_t * ctx, view_floating_p x, xcb_button_t button, rect const & pos) :
		grab_default_t{ctx},
		c{x},
		start_position{pos},
		target_notebook{},
		zone{NOTEBOOK_AREA_NONE},
		_button{button}
{

}

grab_bind_view_floating_t::~grab_bind_view_floating_t() {

}

void grab_bind_view_floating_t::_find_target_notebook(int x, int y,
		notebook_p & target, notebook_area_e & zone) {

	target = nullptr;
	zone = NOTEBOOK_AREA_NONE;

	/* place the popup */
	auto ln = _ctx->current_workspace()->gather_children_root_first<notebook_t>();
	for (auto i : ln) {
		if (i->_area.tab.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_TAB;
			target = i;
			break;
		} else if (i->_area.right.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_RIGHT;
			target = i;
			break;
		} else if (i->_area.top.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_TOP;
			target = i;
			break;
		} else if (i->_area.bottom.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_BOTTOM;
			target = i;
			break;
		} else if (i->_area.left.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_LEFT;
			target = i;
			break;
		} else if (i->_area.popup_center.is_inside(x, y)) {
			zone = NOTEBOOK_AREA_CENTER;
			target = i;
			break;
		}
	}
}

void grab_bind_view_floating_t::button_press(ClutterEvent const * e) {

}

void grab_bind_view_floating_t::button_motion(ClutterEvent const * e)
{
	gfloat x, y;
	clutter_event_get_coords(e, &x, &y);
	auto time = clutter_event_get_time(e);

	if(c.expired()) {
		_ctx->grab_stop(time);
		return;
	}

	/* do not start drag&drop for small move */
//	if (not start_position.is_inside(x, y) and pn0->parent() == nullptr) {
//		_ctx->overlay_add(pn0);
//		pn0->show();
//	}
//
//	if (pn0 == nullptr)
//		return;

	shared_ptr<notebook_t> new_target;
	notebook_area_e new_zone;
	_find_target_notebook(x, y, new_target, new_zone);

	if((new_target != target_notebook.lock() or new_zone != zone) and new_zone != NOTEBOOK_AREA_NONE) {
		target_notebook = new_target;
		zone = new_zone;
		switch(zone) {
		case NOTEBOOK_AREA_TAB:
//			pn0->move_resize(new_target->_area.tab);
			break;
		case NOTEBOOK_AREA_RIGHT:
//			pn0->move_resize(new_target->_area.popup_right);
			break;
		case NOTEBOOK_AREA_TOP:
//			pn0->move_resize(new_target->_area.popup_top);
			break;
		case NOTEBOOK_AREA_BOTTOM:
//			pn0->move_resize(new_target->_area.popup_bottom);
			break;
		case NOTEBOOK_AREA_LEFT:
//			pn0->move_resize(new_target->_area.popup_left);
			break;
		case NOTEBOOK_AREA_CENTER:
//			pn0->move_resize(new_target->_area.popup_center);
			break;
		}
	}
}

void grab_bind_view_floating_t::button_release(ClutterEvent const * e)
{
	gfloat x, y;
	clutter_event_get_coords(e, &x, &y);
	auto button = clutter_event_get_button(e);
	auto time = clutter_event_get_time(e);

	if(c.expired()) {
		_ctx->grab_stop(time);
		return;
	}

	auto c = this->c.lock();

	if (button == _button) {
		notebook_p new_target;
		notebook_area_e new_zone;
		_find_target_notebook(x, y, new_target, new_zone);

		/* if the mouse is no where, keep old location */
		if((new_target == nullptr or new_zone == NOTEBOOK_AREA_NONE) and not target_notebook.expired()) {
			new_zone = zone;
			new_target = target_notebook.lock();
		}

		if(start_position.is_inside(x, y)) {
			c->workspace()->switch_floating_to_notebook(c, time);
			_ctx->grab_stop(time);
			return;
		}

		switch(zone) {
		case NOTEBOOK_AREA_TAB:
		case NOTEBOOK_AREA_CENTER:
			_ctx->move_floating_to_notebook(c, new_target, time);
			break;
		case NOTEBOOK_AREA_TOP:
			_ctx->split_top(new_target, c, time);
			break;
		case NOTEBOOK_AREA_LEFT:
			_ctx->split_left(new_target, c, time);
			break;
		case NOTEBOOK_AREA_BOTTOM:
			_ctx->split_bottom(new_target, c, time);
			break;
		case NOTEBOOK_AREA_RIGHT:
			_ctx->split_right(new_target, c, time);
			break;
		default:
			c->raise();
			c->workspace()->set_focus(c, time);
		}

		_ctx->grab_stop(time);

	}
}

}
