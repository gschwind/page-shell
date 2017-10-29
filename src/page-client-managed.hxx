/*
 * managed_window.hxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#ifndef CLIENT_MANAGED_HXX_
#define CLIENT_MANAGED_HXX_

#include <string>
#include <vector>
#include <set>
#include <map>

#include <xcb/xcb.h>

extern "C" {
#include <meta/meta-plugin.h>
}

#include "page-icon-handler.hxx"
#include "page-theme.hxx"
#include "page-utils.hxx"

namespace page {

using namespace std;

enum managed_window_type_e {
	MANAGED_FLOATING,
	MANAGED_NOTEBOOK,
	MANAGED_FULLSCREEN,
	MANAGED_DOCK,
	MANAGED_POPUP
};

struct client_managed_t :
		public enable_shared_from_this<client_managed_t>,
		public connectable_t,
		public g_connectable_t
{

	page_t * _ctx;

	/* handle properties of client */
	MetaWindowActor * _meta_window_actor;
	MetaWindow * _meta_window;

	managed_window_type_e revert_type;

	/** prefered notebook to revert if available **/
	notebook_w revert_notebook;

	/**
	 * hold floating position of the client window relative to root window,
	 * even if rebased to another window. This used as default floating position
	 * when the user switch the window between fullscreen or notebook to
	 * floating.
	 **/
	rect _floating_wished_position;

	/**
	 * The position of the client window relative to root window, even if the
	 * client is rebased to another window.
	 **/
	rect _absolute_position;

	view_t * _current_owner_view;

	/* private to avoid copy */
	client_managed_t(client_managed_t const &) = delete;
	client_managed_t & operator=(client_managed_t const &) = delete;

	void init_managed_type(managed_window_type_e type);

	void set_wished_position(rect const & position);
	rect const & get_wished_position() const;

	auto current_owner_view() const -> view_t *;
	// this two functions is to ensure correct ownership.
	void acquire(view_t * v);
	void release(view_t * v);

	client_managed_t(page_t * ctx, MetaWindowActor * actor);
	~client_managed_t();

	auto meta_window() -> MetaWindow *;
	auto meta_window_actor() -> MetaWindowActor *;

	void _handler_meta_window_focus(MetaWindow * metawindow);
	void _handler_meta_window_position_changed(MetaWindow * window);
	void _handler_meta_window_raised(MetaWindow * metawindow);
	void _handler_meta_window_size_changed(MetaWindow * window);
	void _handler_meta_window_unmanaged(MetaWindow * metawindow);
	void _handler_meta_window_workspace_changed(MetaWindow * metawindow);

	signal_t<client_managed_t *> on_destroy;
	signal_t<client_managed_t *> on_title_change;
	signal_t<client_managed_t *> on_configure_notify;
	signal_t<client_managed_t *> on_unmanage;

	auto get_wished_position() -> rect const &;
	void set_floating_wished_position(rect const & pos);
	auto get_floating_wished_position() -> rect const & ;

	/* wrappers */
	void delete_window(guint32 t);
	void focus(guint32 timestamp);
	void set_demands_attention();
	auto title() const -> string;
	auto position() -> rect;

};

using client_managed_p = shared_ptr<client_managed_t>;
using client_managed_w = weak_ptr<client_managed_t>;

}


#endif /* MANAGED_WINDOW_HXX_ */
