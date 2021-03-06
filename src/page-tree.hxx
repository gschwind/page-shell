/*
 * tree.hxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#ifndef TREE_HXX_
#define TREE_HXX_

#include <typeinfo>
#include <memory>
#include <iostream>
#include <map>

#include <clutter/clutter.h>

#include "page-page-types.hxx"
#include "page-utils.hxx"
#include "page-time.hxx"

enum button_action_e {
	BUTTON_ACTION_CONTINUE,
	BUTTON_ACTION_REPLAY,
	BUTTON_ACTION_GRAB_SYNC,
	BUTTON_ACTION_GRAB_ASYNC,
	BUTTON_ACTION_HAS_ACTIVE_GRAB
};

namespace page {

using namespace std;

/**
 * tree_t is the base of the hierarchy of workspace, viewports,
 * client_managed and unmanaged, etc...
 * It define the stack order of each component drawn within page.
 **/
class tree_t :
		public connectable_t,
		public g_connectable_t,
		public enable_shared_from_this<tree_t>
{

protected:
	template<typename ... T>
	bool _broadcast_root_first(bool (tree_t::* f)(T ... args), T ... args) {
		if((this->*f)(args...))
			return true;
		for(auto x: weak(get_all_children_root_first())) {
			if((x->*f)(args...))
				return true;
		}
		return false;
	}

	template<typename ... T>
	void _broadcast_root_first(void (tree_t::* f)(T ... args), T ... args) {
		(this->*f)(args...);
		for(auto x: weak(get_all_children_root_first())) {
			if(not x.expired())
				(x.lock().get()->*f)(args...);
		}
		(this->*f)(args...);
	}

	template<typename ... T>
	bool _broadcast_deep_first(bool (tree_t::* f)(T ... args), T ... args) {
		for(auto x: weak(get_all_children_deep_first())) {
			if(not x.expired()) {
				if((x.lock().get()->*f)(args...))
					return true;
			}
		}
		if((this->*f)(args...))
			return true;
		return false;
	}

	template<typename ... T>
	void _broadcast_deep_first(void (tree_t::* f)(T ... args), T ... args) {
		for(auto x: weak(get_all_children_deep_first())) {
			if(not x.expired())
				(x.lock().get()->*f)(args...);
		}
		(this->*f)(args...);
	}

	auto _broadcast_deep_first(button_action_e (tree_t::* f)(ClutterEvent const * ev), ClutterEvent const * ev) -> button_action_e {
		for(auto x: weak(get_all_children_deep_first())) {
			if(not x.expired()) {
				auto ret = (x.lock().get()->*f)(ev);
				if(ret != BUTTON_ACTION_CONTINUE)
					return ret;
			}
		}
		auto ret = (this->*f)(ev);
		if(ret != BUTTON_ACTION_CONTINUE)
			return ret;
		return BUTTON_ACTION_CONTINUE;
	}

	template<char const c>
	string _get_node_name() const {
		return xformat("%c(%ld) #%016lx #%016lx", c, shared_from_this().use_count(), _parent, (uintptr_t) this);
	}

	template<typename T>
	void _gather_children_root_first(vector<shared_ptr<T>> & out) const
	{
		for (auto const & x : _children) {
			auto t = dynamic_pointer_cast<T>(x);
			if(t != nullptr)
				out.push_back(t);
			x->_gather_children_root_first<T>(out);
		}
	}

	/**
	 * Parent must exist or beeing NULL, when a node is destroyed, he must
	 * clear children _parent.
	 **/
	tree_t * _parent;
	list<tree_p> _children;

	bool _is_visible;
	bool _stack_is_locked; // define wether childdren can be raised.

private:
	tree_t(tree_t const &) = delete;
	tree_t & operator=(tree_t const &) = delete;

public:
	workspace_t * _root;

	void set_parent(tree_t * parent);
	void clear_parent();

public:
	tree_t(workspace_t * root);

	auto parent() const -> shared_ptr<tree_t>;

	bool is_visible() const;

	void print_tree(int level = 0) const;

	auto children() const -> vector<tree_p>;
	auto get_all_children() const -> vector<tree_p>;
	auto get_all_children_deep_first() const -> vector<tree_p>;
	auto get_all_children_root_first() const -> vector<tree_p>;

	void get_all_children_deep_first(vector<tree_p> & out) const;
	void get_all_children_root_first(vector<tree_p> & out) const;

	template<typename T>
	auto gather_children_root_first() const -> vector<shared_ptr<T>>
	{
		vector<shared_ptr<T>> ret;
		_gather_children_root_first<T>(ret);
		return ret;
	}

	auto broadcast_button_press(ClutterEvent const * ev) -> button_action_e;
	bool broadcast_button_release(ClutterEvent const * ev);
	bool broadcast_button_motion(ClutterEvent const * ev);
	bool broadcast_leave(ClutterEvent const * ev);
	bool broadcast_enter(ClutterEvent const * ev);

	void broadcast_on_workspace_enable();
	void broadcast_on_workspace_disable();

	rect to_root_position(rect const & r) const;

	void raise(shared_ptr<tree_t> t = nullptr);
	auto workspace() const -> workspace_p;
	auto lookup_view_for(client_managed_p c) const -> view_p;
	void gather_children(vector<tree_p> & out) const;

	void detach_myself();
	void push_back(tree_p t);
	void push_front(tree_p t);

	/**
	 * tree_t virtual API
	 **/

	virtual ~tree_t();
	virtual void hide();
	virtual void show();
	virtual auto get_node_name() const -> string;
	virtual void remove(tree_p t);
	virtual void clear();

	virtual void reconfigure(); // used to place all windows taking in account the current tree state
	virtual void on_workspace_enable();
	virtual void on_workspace_disable();

	virtual auto button_press(ClutterEvent const * ev) -> button_action_e;
	virtual bool button_release(ClutterEvent const * ev);
	virtual bool button_motion(ClutterEvent const * ev);
	virtual bool leave(ClutterEvent const * ev);
	virtual bool enter(ClutterEvent const * ev);

	virtual rect get_window_position() const;
	virtual void queue_redraw();

	virtual auto get_default_view() const -> ClutterActor *;

};


}

#endif /* TREE_HXX_ */
