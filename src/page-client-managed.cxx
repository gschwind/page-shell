/*
 * managed_window.cxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#include <cairo.h>
#include <cairo-xlib.h>
#include <cairo-xcb.h>

#include "page-page.hxx"
#include "page-client-managed.hxx"
#include "page-notebook.hxx"
#include "page-utils.hxx"
#include "page-grab-handlers.hxx"

namespace page {

using namespace std;

client_managed_t::client_managed_t(page_t * ctx, MetaWindowActor * actor) :
	_ctx{ctx},
	_meta_window_actor{actor},
	_meta_window{meta_window_actor_get_meta_window(actor)},
	_floating_wished_position{},
	_absolute_position{},
	_current_owner_view{nullptr}
{
	g_object_ref(_meta_window_actor);
	g_object_ref(_meta_window);
	//_update_title();
	rect pos{position()};

	//printf("window default position = %s\n", pos.to_string().c_str());

	_floating_wished_position = pos;
	_absolute_position = pos;

	g_connect(_meta_window, "focus", &client_managed_t::_handler_meta_window_focus);
	g_connect(_meta_window, "position-changed", &client_managed_t::_handler_meta_window_position_changed);
	g_connect(_meta_window, "raised", &client_managed_t::_handler_meta_window_raised);
	g_connect(_meta_window, "size-changed", &client_managed_t::_handler_meta_window_size_changed);
	g_connect(_meta_window, "unmanaged", &client_managed_t::_handler_meta_window_unmanaged);
	g_connect(_meta_window, "workspace-changed", &client_managed_t::_handler_meta_window_workspace_changed);
}

client_managed_t::~client_managed_t()
{
	on_destroy.signal(this);
	g_object_unref(_meta_window_actor);
	g_object_unref(_meta_window);
}

auto client_managed_t::meta_window() -> MetaWindow *
{
	return _meta_window;
}
auto client_managed_t::meta_window_actor() -> MetaWindowActor *
{
	return _meta_window_actor;
}

void client_managed_t::_handler_meta_window_focus(MetaWindow * metawindow)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
}

void client_managed_t::_handler_meta_window_position_changed(MetaWindow * window)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
}

void client_managed_t::_handler_meta_window_raised(MetaWindow * metawindow)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);

	/* ensure preservation of stack */
	_ctx->sync_tree_view();
}

void client_managed_t::_handler_meta_window_size_changed(MetaWindow * window)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
	on_configure_notify.signal(this);
}

void client_managed_t::_handler_meta_window_unmanaged(MetaWindow * metawindow)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
	on_unmanage.signal(this);
}

void client_managed_t::_handler_meta_window_workspace_changed(MetaWindow * metawindow)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
}


void client_managed_t::delete_window(guint32 t) {
	log(LOG_NONE, "request close for '%s'\n", title().c_str());
	meta_window_delete(_meta_window, t);
}

void client_managed_t::set_floating_wished_position(rect const & pos) {
	_floating_wished_position = pos;
}

rect const & client_managed_t::get_wished_position() {
	return _absolute_position;
}

rect const & client_managed_t::get_floating_wished_position() {
	return _floating_wished_position;
}

auto client_managed_t::current_owner_view() const -> view_t *
{
	return _current_owner_view;
}

void client_managed_t::acquire(view_t * v)
{
	_current_owner_view = v;
}
void client_managed_t::release(view_t * v)
{
	if(_current_owner_view == v)
		_current_owner_view = nullptr;
}

void client_managed_t::focus(guint32 timestamp)
{
	meta_window_focus(_meta_window, timestamp);
}

void client_managed_t::set_demands_attention()
{
	meta_window_set_demands_attention(_meta_window);
}

auto client_managed_t::title() const -> string
{
	return string{meta_window_get_title(_meta_window)};
}


auto client_managed_t::position() -> rect
{
	MetaRectangle xrect;
	meta_window_get_frame_rect(_meta_window, &xrect);
	return rect(xrect);
}

}

