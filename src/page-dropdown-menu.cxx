/*
 * popup_alt_tab.cxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#include "page-dropdown-menu.hxx"

#include "page-page.hxx"
#include "page-tree.hxx"
#include "page-workspace.hxx"

namespace page {

using namespace std;

dropdown_menu_entry_t::dropdown_menu_entry_t(shared_ptr<icon16> icon,
		string const & label, function<void(xcb_timestamp_t time)> on_click) :
	_theme_data{icon, label},
	_on_click{on_click}
{

}

dropdown_menu_entry_t::~dropdown_menu_entry_t()
{

}

shared_ptr<icon16> dropdown_menu_entry_t::icon() const
{
	return _theme_data.icon;
}

string const & dropdown_menu_entry_t::label() const
{
	return _theme_data.label;
}

theme_dropdown_menu_entry_t const & dropdown_menu_entry_t::get_theme_item()
{
	return _theme_data;
}

dropdown_menu_overlay_t::dropdown_menu_overlay_t(tree_t * ref, rect position) :
	tree_t{ref->_root},
	_ctx{ref->_root->_ctx},
	_position{position}
{
	_is_durty = true;

	_canvas = clutter_canvas_new();
	g_object_ref_sink(_canvas);
	clutter_canvas_set_size(CLUTTER_CANVAS(_canvas), _position.w, _position.h);
	g_connect(CLUTTER_CANVAS(_canvas), "draw", &dropdown_menu_overlay_t::draw);

	_default_view = clutter_actor_new();
	g_object_ref_sink(_default_view);
	clutter_actor_set_content(_default_view, _canvas);
	clutter_actor_set_content_scaling_filters(_default_view,
			CLUTTER_SCALING_FILTER_NEAREST, CLUTTER_SCALING_FILTER_NEAREST);
	clutter_actor_set_reactive (_default_view, TRUE);

	clutter_actor_set_position(_default_view, _position.x, _position.y);
	clutter_actor_set_size(_default_view, _position.w, _position.h);
	clutter_actor_add_child(_ctx->_overlay_group, _default_view);
	clutter_actor_show(_default_view);

}

dropdown_menu_overlay_t::~dropdown_menu_overlay_t()
{
	clutter_actor_remove_child(_ctx->_overlay_group, _default_view);
	g_object_unref(_canvas);
	g_object_unref(_default_view);
}

rect const & dropdown_menu_overlay_t::position()
{
	return _position;
}

void dropdown_menu_overlay_t::invalidate()
{
	clutter_content_invalidate(_canvas);
}

void dropdown_menu_overlay_t::draw(ClutterCanvas * canvas, cairo_t * cr, int width, int height)
{
	on_draw.signal(canvas, cr, width, height);
}

dropdown_menu_t::dropdown_menu_t(tree_t * ref,
		vector<shared_ptr<item_t>> items, xcb_button_t button, int x, int y,
		int width, rect start_position) :
	_ctx{ref->_root->_ctx},
	_start_position{start_position},
	_button{button},
	_time{XCB_CURRENT_TIME}
{

	_selected = -1;
	_items = items;
	has_been_released = false;

	rect _position;
	_position.x = x;
	_position.y = y;
	_position.w = width;
	_position.h = 24*_items.size();

	pop = make_shared<dropdown_menu_overlay_t>(ref, _position);
	connect(pop->on_draw, this, &dropdown_menu_t::draw);
	pop->invalidate();

}

dropdown_menu_t::~dropdown_menu_t()
{
//	_ctx->schedule_repaint();
//	//pop->detach_myself();
	pop = nullptr;
}

int dropdown_menu_t::selected()
{
	return _selected;
}

xcb_timestamp_t dropdown_menu_t::time()
{
	return _time;
}

void dropdown_menu_t::draw(ClutterCanvas * canvas, cairo_t * cr, int width, int height)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
	for (unsigned k = 0; k < _items.size(); ++k) {
		update_items_back_buffer(cr, k);
	}
}

void dropdown_menu_t::update_items_back_buffer(cairo_t * cr, int n)
{
	if (n >= 0 and n < _items.size()) {
		rect area(0, 24 * n, pop->_position.w, 24);
		_ctx->theme()->render_menuentry(cr, _items[n]->get_theme_item(), area, n == _selected);
	}
}

void dropdown_menu_t::set_selected(int s)
{
	if(s >= 0 and s < _items.size() and s != _selected) {
		std::swap(_selected, s);
		pop->invalidate();
	}
}

void dropdown_menu_t::update_cursor_position(int x, int y)
{
	if (pop->_position.is_inside(x, y)) {
		int s = (int) floor((y - pop->_position.y) / 24.0);
		set_selected(s);
	}
}

void dropdown_menu_t::button_press(ClutterEvent const * e)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
}


void dropdown_menu_t::button_motion(ClutterEvent const * e)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
	gfloat x, y;
	clutter_event_get_coords(e, &x, &y);
	update_cursor_position(x, y);
}

void dropdown_menu_t::button_release(ClutterEvent const * e)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
	gfloat x, y;
	clutter_event_get_coords(e, &x, &y);
	auto button = clutter_event_get_button(e);
	auto time = clutter_event_get_time(e);

	log::printf("button=%d\n", button);

	if(not pop->_position.is_inside(x, y)) {
		if(has_been_released) {
			_ctx->grab_stop(time);
		} else {
			has_been_released = true;
		}
	} else {
		update_cursor_position(x, y);
		_time = time;
		_items[_selected]->_on_click(time);
		_ctx->grab_stop(time);
	}
}

void dropdown_menu_t::key_press(ClutterEvent const * ev) { }
void dropdown_menu_t::key_release(ClutterEvent const * ev) {
	auto key = clutter_event_get_key_symbol(ev);
	auto time = clutter_event_get_time(ev);

	log::printf("key release = %d, %d\n", key, XK_Escape);

	if (key == XK_Escape) {
		_ctx->grab_stop(time);
	}

}

}

