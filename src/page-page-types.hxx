/*
 * page_context.hxx
 *
 *  Created on: 13 juin 2015
 *      Author: gschwind
 */

#ifndef SRC_PAGE_TYPES_HXX_
#define SRC_PAGE_TYPES_HXX_

#include <typeinfo>
#include <xcb/xcb.h>
#include <memory>

#include <clutter/clutter.h>

namespace page {

using namespace std;

class tree_t;
using tree_p = shared_ptr<tree_t>;
using tree_w = weak_ptr<tree_t>;

class workspace_t;
using workspace_p = shared_ptr<workspace_t>;
using workspace_w = weak_ptr<workspace_t>;

class viewport_t;
using viewport_p = shared_ptr<viewport_t>;
using viewport_w = weak_ptr<viewport_t>;

class split_t;
using split_p = shared_ptr<split_t>;
using split_w = weak_ptr<split_t>;

class notebook_t;
using notebook_p = shared_ptr<notebook_t>;
using notebook_w = weak_ptr<notebook_t>;

class view_t;
using view_p = shared_ptr<view_t>;
using view_w = weak_ptr<view_t>;

class view_fullscreen_t;
using view_fullscreen_p = shared_ptr<view_fullscreen_t>;
using view_fullscreen_w = weak_ptr<view_fullscreen_t>;

class view_notebook_t;
using view_notebook_p = shared_ptr<view_notebook_t>;
using view_notebook_w = weak_ptr<view_notebook_t>;

class view_floating_t;
using view_floating_p = shared_ptr<view_floating_t>;
using view_floating_w = weak_ptr<view_floating_t>;

class view_rebased_t;
using view_rebased_p = shared_ptr<view_rebased_t>;
using view_rebased_w = weak_ptr<view_rebased_t>;

class client_managed_t;
using client_managed_p = shared_ptr<client_managed_t>;
using client_managed_w = weak_ptr<client_managed_t>;

class page_t;
class theme_t;
class notebook_t;
class viewport_t;
class workspace_t;

struct grab_handler_t {
	virtual ~grab_handler_t() { }
	virtual void button_press(ClutterEvent const *) = 0;
	virtual void button_motion(ClutterEvent const *) = 0;
	virtual void button_release(ClutterEvent const *) = 0;
	virtual void key_press(ClutterEvent const * ev) = 0;
	virtual void key_release(ClutterEvent const * ev) = 0;
};

struct page_configuration_t {
	bool _replace_wm;
	bool _menu_drop_down_shadow;
	bool _auto_refocus;
	bool _mouse_focus;
	bool _enable_shade_windows;
	int64_t _fade_in_time;
};

}

#endif /* SRC_PAGE_TYPES_HXX_ */
