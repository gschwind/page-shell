/*
 * viewport.cxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#include <algorithm>
#include <typeinfo>

#include "page-page.hxx"
#include "page-notebook.hxx"
#include "page-viewport.hxx"
#include "page-workspace.hxx"

namespace page {

using namespace std;

viewport_t::viewport_t(tree_t * ref, rect const & area) :
		page_component_t{ref},
		_raw_aera{area},
		_effective_area{area},
		_subtree{nullptr}
{
	_page_area = rect{0, 0, _effective_area.w, _effective_area.h};
	auto n = make_shared<notebook_t>(this);
	_subtree = static_pointer_cast<page_component_t>(n);
	push_back(_subtree);

	_canvas = clutter_canvas_new();
	g_object_ref_sink(_canvas);
	clutter_canvas_set_size(CLUTTER_CANVAS(_canvas), _effective_area.w, _effective_area.h);

	_default_view = clutter_actor_new();
	g_object_ref_sink(_default_view);
	clutter_actor_set_content(_default_view, _canvas);
	clutter_actor_set_content_scaling_filters(_default_view,
			CLUTTER_SCALING_FILTER_NEAREST, CLUTTER_SCALING_FILTER_NEAREST);
	clutter_actor_set_reactive (_default_view, TRUE);

	g_connect(CLUTTER_CANVAS(_canvas), "draw", &viewport_t::draw);
	g_connect(CLUTTER_ACTOR(_default_view), "button-press-event", &viewport_t::_handler_button_press_event);
	g_connect(CLUTTER_ACTOR(_default_view), "button-release-event", &viewport_t::_handler_button_release_event);
	g_connect(CLUTTER_ACTOR(_default_view), "motion-event", &viewport_t::_handler_motion_event);
	g_connect(CLUTTER_ACTOR(_default_view), "enter-event", &viewport_t::_handler_enter_event);
	g_connect(CLUTTER_ACTOR(_default_view), "leave-event", &viewport_t::_handler_leave_event);

	clutter_content_invalidate(_canvas);
	clutter_actor_set_position(_default_view, _effective_area.x, _effective_area.y);
	clutter_actor_set_size(_default_view, _effective_area.w, _effective_area.h);

	_subtree->set_allocation(_page_area);
}

viewport_t::~viewport_t() {
	g_object_unref(_canvas);
	g_object_unref(_default_view);
}

void viewport_t::replace(shared_ptr<page_component_t> src, shared_ptr<page_component_t> by) {
	//printf("replace %p by %p\n", src, by);

	assert(has_key(_children, src));

	if (_subtree == src) {
		remove(_subtree);
		_subtree = by;
		push_back(_subtree);
		_subtree->set_allocation(_page_area);

		if(_is_visible)
			_subtree->show();
		else
			_subtree->hide();

	} else {
		throw std::runtime_error("viewport: bad child replacement!");
	}
}

void viewport_t::remove(shared_ptr<tree_t> src) {
	assert(has_key(_children, src));
	tree_t::remove(_subtree);
	_subtree.reset();
}

void viewport_t::set_allocation(rect const & area) {
	_effective_area = area;
	_page_area = rect{0, 0, _effective_area.w, _effective_area.h};
	if(_subtree != nullptr)
		_subtree->set_allocation(_page_area);
	queue_redraw();
}

void viewport_t::set_raw_area(rect const & area) {
	_raw_aera = area;
}

rect const & viewport_t::raw_area() const {
	return _raw_aera;
}

string viewport_t::get_node_name() const {
	return _get_node_name<'V'>();
}

void viewport_t::reconfigure()
{
	if(not _root->is_enable())
		return;

	auto _ctx = _root->_ctx;
	auto _dpy = _root->_ctx->dpy();

}

void viewport_t::on_workspace_enable()
{
	auto _ctx = _root->_ctx;
	auto _dpy = _root->_ctx->dpy();
	reconfigure();
}

void viewport_t::on_workspace_disable()
{
	auto _ctx = _root->_ctx;
	auto _dpy = _root->_ctx->dpy();

}

rect viewport_t::allocation() const {
	return _effective_area;
}

rect const & viewport_t::page_area() const {
	return _page_area;
}

void viewport_t::hide() {
	tree_t::hide();
	reconfigure();
}

void viewport_t::show() {
	tree_t::show();
	reconfigure();
}

void viewport_t::draw(ClutterCanvas * _, cairo_t * cr, int width, int height) {
	log::printf("call %s\n", __PRETTY_FUNCTION__);

	cairo_save(cr);
	cairo_identity_matrix(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
	cairo_paint(cr);

	auto splits = gather_children_root_first<split_t>();
	for (auto x : splits) {
		x->render_legacy(cr);
	}

	auto notebooks = gather_children_root_first<notebook_t>();
	for (auto x : notebooks) {
		x->render_legacy(cr);
	}

	cairo_restore(cr);

}

/* mark renderable_page for redraw */
void viewport_t::queue_redraw()
{
	_root->_ctx->schedule_repaint();
	if(_canvas)
		clutter_content_invalidate(_canvas);
}

auto viewport_t::get_default_view() const -> ClutterActor *
{
	return _default_view;
}

auto viewport_t::_handler_button_press_event(ClutterActor * actor, ClutterEvent * event) -> gboolean
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);

	if (not _root->_ctx->has_grab_handler())
		broadcast_button_press(event);

	return FALSE;
}

auto viewport_t::_handler_button_release_event(ClutterActor * actor, ClutterEvent * event) -> gboolean
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);

	if (not _root->_ctx->has_grab_handler())
		broadcast_button_release(event);

	return FALSE;
}

auto viewport_t::_handler_motion_event(ClutterActor * actor, ClutterEvent * event) -> gboolean
{
	//log::printf("call %s\n", __PRETTY_FUNCTION__);

	broadcast_button_motion(event);

	return FALSE;
}

auto viewport_t::_handler_enter_event(ClutterActor * actor, ClutterEvent * event) -> gboolean
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);

	broadcast_enter(event);

	return FALSE;
}

auto viewport_t::_handler_leave_event(ClutterActor * actor, ClutterEvent * event) -> gboolean
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);

	broadcast_leave(event);

	return FALSE;
}


rect viewport_t::get_window_position() const {
	return _effective_area;
}

void viewport_t::get_min_allocation(int & width, int & height) const {
	width = 0;
	height = 0;

	if(_subtree != nullptr) {
		_subtree->get_min_allocation(width, height);
	}

}

}
