/*
 * workspace.cxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#include <algorithm>
#include <typeinfo>

#include "page-page.hxx"
#include "page-viewport.hxx"
#include "page-workspace.hxx"
#include "page-view.hxx"
#include "page-view-fullscreen.hxx"
#include "page-view-floating.hxx"
#include "page-view-notebook.hxx"

namespace page {

using namespace std;

time64_t const workspace_t::_switch_duration{0.5};

void workspace_t::_init()
{
	_stack_is_locked = true;

	_viewport_layer = make_shared<tree_t>(this);
	_floating_layer = make_shared<tree_t>(this);
	_fullscreen_layer = make_shared<tree_t>(this);
	_overlays_layer = make_shared<tree_t>(this);

	push_back(_viewport_layer);
	push_back(_floating_layer);
	push_back(_fullscreen_layer);
	push_back(_overlays_layer);

	set_to_default_name();
}

workspace_t::workspace_t(page_t * ctx, MetaWorkspace * workspace) :
	tree_t{this},
	_ctx{ctx},
	_meta_workspace{workspace},
	_default_pop{},
	_primary_viewport{},
	_switch_direction{WORKSPACE_SWITCH_LEFT},
	_is_enable{false}
{
	_init();
}

workspace_t::workspace_t(page_t * ctx, guint32 time) :
	tree_t{this},
	_ctx{ctx},
	_meta_workspace{nullptr},
	_default_pop{},
	_primary_viewport{},
	_switch_direction{WORKSPACE_SWITCH_LEFT},
	_is_enable{false}
{
	_meta_workspace = meta_screen_append_new_workspace(ctx->_screen, FALSE, time);
	_init();
}

workspace_t::~workspace_t()
{

}

auto workspace_t::shared_from_this() -> workspace_p
{
	return static_pointer_cast<workspace_t>(tree_t::shared_from_this());
}

string workspace_t::get_node_name() const {
	return _get_node_name<'D'>();
}

auto workspace_t::get_viewport_map() const -> vector<viewport_p> {
	return _viewport_outputs;
}

void workspace_t::update_viewports_layout()
{
	_viewport_layer->clear();

	auto screen = meta_plugin_get_screen(_ctx->_plugin);
	auto n_monitor = meta_screen_get_n_monitors(screen);

	vector<rect> viewport_allocation;
	region already_allocated;
	for(int monitor_id = 0; monitor_id < n_monitor; ++monitor_id) {
		MetaRectangle area;
		meta_workspace_get_work_area_for_monitor(_meta_workspace, monitor_id, &area);
		region region_to_alocate{rect{area}};
		region_to_alocate -= already_allocated;
		for (auto & r: region_to_alocate.rects()) {
			viewport_allocation.push_back(r);
		}
		already_allocated += region_to_alocate;
	}

	/** get old viewport_allocation to recycle old viewport, and keep unchanged outputs **/
	auto old_layout = _viewport_outputs;
	/** store the newer layout, to be able to cleanup obsolete viewports **/
	_viewport_outputs.clear();
	/** for each not overlaped rectangle **/
	for (unsigned i = 0; i < viewport_allocation.size(); ++i) {
		log::printf("%p: found viewport (%d,%d,%d,%d)\n", _meta_workspace,
				viewport_allocation[i].x, viewport_allocation[i].y,
				viewport_allocation[i].w, viewport_allocation[i].h);
		viewport_p vp;
		if (i < old_layout.size()) {
			vp = old_layout[i];
			vp->update_work_area(viewport_allocation[i]);
		} else {
			vp = make_shared<viewport_t>(this, viewport_allocation[i]);
		}
		_viewport_outputs.push_back(vp);
		_viewport_layer->push_back(vp);
	}

	/** clean up obsolete viewport_allocation **/
	for (unsigned i = _viewport_outputs.size(); i < old_layout.size(); ++i) {
		/** destroy this viewport **/
		remove_viewport(old_layout[i]);
		old_layout[i].reset();
	}

	_primary_viewport = _viewport_outputs[0];

	// update visibility
	if (_is_visible)
		show();
	else
		hide();

}

void workspace_t::remove_viewport(viewport_p v)
{

	/* Transfer clients to a valid notebook */
	for (auto x : v->gather_children_root_first<view_notebook_t>()) {
		ensure_default_notebook()->add_client_from_view(x, XCB_CURRENT_TIME);
	}

	for (auto x : v->gather_children_root_first<view_floating_t>()) {
		insert_as_floating(x->_client, XCB_CURRENT_TIME);
	}

}

auto workspace_t::get_any_viewport() const -> viewport_p {
	return _primary_viewport.lock();
}

auto workspace_t::get_viewports() const -> vector<viewport_p> {
	return _viewport_layer->gather_children_root_first<viewport_t>();
}

void workspace_t::set_default_pop(notebook_p n) {
	assert(n->_root == this);

	if (not _default_pop.expired()) {
		_default_pop.lock()->set_default(false);
	}

	if (n != nullptr) {
		_default_pop = n;
		_default_pop.lock()->set_default(true);
	}
}

auto workspace_t::ensure_default_notebook() -> notebook_p {
	auto notebooks = gather_children_root_first<notebook_t>();
	assert(notebooks.size() > 0); // workspace must have at less one notebook.

	if(_default_pop.expired()) {
		_default_pop = notebooks[0];
		notebooks[0]->set_default(true);
		return notebooks[0];
	}

	if(not has_key(notebooks, _default_pop.lock())) {
		_default_pop = notebooks[0];
		notebooks[0]->set_default(true);
		return notebooks[0];
	}

	return _default_pop.lock();

}

void workspace_t::attach(shared_ptr<client_managed_t> c) {
	assert(c != nullptr);

//	if(c->is(MANAGED_FULLSCREEN)) {
//		_fullscreen_layer->push_back(c);
//	} else {
//		_floating_layer->push_back(c);
//	}
//
//	if(_is_visible) {
//		c->show();
//	} else {
//		c->hide();
//	}
}

void workspace_t::enable(xcb_timestamp_t time)
{
	_is_enable = true;
	broadcast_on_workspace_enable();

	view_p focus;
	if(client_focus_history_front(focus)) {
		set_focus(focus, time);
	} else {
		set_focus(nullptr, time);
	}

}

void workspace_t::disable()
{
	_is_enable = false;
	broadcast_on_workspace_disable();
}

bool workspace_t::is_enable()
{
	return _is_enable;
}

void workspace_t::insert_as_floating(client_managed_p c, xcb_timestamp_t time)
{
	auto fv = make_shared<view_floating_t>(this, c);
	_insert_view_floating(fv, time);
}

void workspace_t::insert_as_fullscreen(client_managed_p mw, xcb_timestamp_t time)
{
	//printf("call %s\n", __PRETTY_FUNCTION__);
	auto v = get_any_viewport();
	insert_as_fullscreen(mw, v);
}

void workspace_t::insert_as_notebook(client_managed_p mw, xcb_timestamp_t time)
{
	//printf("call %s\n", __PRETTY_FUNCTION__);
	/** select if the client want to appear mapped or iconic **/
	bool activate = true;
	ensure_default_notebook()->add_client(mw, time);
	_ctx->sync_tree_view();

}

void workspace_t::insert_as_fullscreen(shared_ptr<client_managed_t> mw, shared_ptr<viewport_t> v) {
	//printf("call %s\n", __PRETTY_FUNCTION__);
	assert(v != nullptr);

	auto fv = make_shared<view_fullscreen_t>(this, mw);
	if(is_enable())
		fv->acquire_client();

	// unfullscreen client that already use this screen
	for (auto & x : gather_children_root_first<view_fullscreen_t>()) {
		switch_fullscreen_to_prefered_view_mode(x, XCB_CURRENT_TIME);
	}

	add_fullscreen(fv);
	fv->show();

	/* hide the viewport because he is covered by the fullscreen client */
	v->hide();
	_ctx->sync_tree_view();
}

void workspace_t::switch_view_to_fullscreen(view_p v, xcb_timestamp_t time)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
	auto vx = dynamic_pointer_cast<view_floating_t>(v);
	if(vx) {
		switch_floating_to_fullscreen(vx, time);
		return;
	}

	auto vn = dynamic_pointer_cast<view_notebook_t>(v);
	if(vn) {
		switch_notebook_to_fullscreen(vn, time);
		return;
	}
}

void workspace_t::switch_view_to_floating(view_p v, xcb_timestamp_t time)
{
	//printf("call %s\n", __PRETTY_FUNCTION__);
	auto vn = dynamic_pointer_cast<view_notebook_t>(v);
	if(vn) {
		switch_notebook_to_floating(vn, time);
		return;
	}

	auto vf = dynamic_pointer_cast<view_fullscreen_t>(v);
	if(vf) {
		switch_fullscreen_to_floating(vf, time);
		return;
	}
}

void workspace_t::switch_view_to_notebook(view_p v, xcb_timestamp_t time)
{
	//printf("call %s\n", __PRETTY_FUNCTION__);
	auto vx = dynamic_pointer_cast<view_floating_t>(v);
	if(vx) {
		switch_floating_to_notebook(vx, time);
		return;
	}

	auto vf = dynamic_pointer_cast<view_fullscreen_t>(v);
	if(vf) {
		switch_fullscreen_to_notebook(vf, time);
		return;
	}
}

void workspace_t::switch_notebook_to_floating(view_notebook_p vn, xcb_timestamp_t time)
{
	//printf("call %s\n", __PRETTY_FUNCTION__);
	vn->remove_this_view();
	meta_window_move_resize_frame(vn->_client->meta_window(), FALSE, vn->_client->_floating_wished_position.x, vn->_client->_floating_wished_position.y, vn->_client->_floating_wished_position.w, vn->_client->_floating_wished_position.h);
	auto vf = make_shared<view_floating_t>(vn.get());
	_insert_view_floating(vf, time);
}

void workspace_t::switch_notebook_to_fullscreen(view_notebook_p vn, xcb_timestamp_t time)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);

	auto client = vn->_client;
	auto nbk = vn->parent_notebook();
	auto vf = make_shared<view_fullscreen_t>(this, client);

	vf->revert_type = MANAGED_NOTEBOOK;
	vf->revert_notebook = nbk;

	nbk->remove_view_notebook(vn);

	vf->acquire_client();
	add_fullscreen(vf);
	_ctx->sync_tree_view();
}

void workspace_t::switch_floating_to_fullscreen(view_floating_p vx, xcb_timestamp_t time)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
	auto viewport = get_any_viewport();
	vx->remove_this_view();
	auto vf = make_shared<view_fullscreen_t>(this, vx->_client);
	if(is_enable())
		vf->acquire_client();
	vf->revert_type = MANAGED_FLOATING;
	add_fullscreen(vf);
}

void workspace_t::switch_floating_to_notebook(view_floating_p vf, xcb_timestamp_t time)
{
	vf->remove_this_view();
	ensure_default_notebook()->add_client_from_view(vf, time);
}

void workspace_t::switch_fullscreen_to_floating(view_fullscreen_p view, xcb_timestamp_t time)
{
	view->remove_this_view();

	//meta_window_unmake_fullscreen(view->_client->_meta_window);
	auto fv = make_shared<view_floating_t>(view.get());
	_insert_view_floating(fv, time);
}

void workspace_t::switch_fullscreen_to_notebook(view_fullscreen_p view, xcb_timestamp_t time)
{
	view->remove_this_view();

	auto n = ensure_default_notebook();
	if(not view->revert_notebook.expired()) {
		n = view->revert_notebook.lock();
	}

	//meta_window_unmake_fullscreen(view->_client->_meta_window);
	n->add_client_from_view(view, time);
	_ctx->sync_tree_view();
}

/* switch a fullscreened and managed window into floating or notebook window */
void workspace_t::switch_fullscreen_to_prefered_view_mode(view_p c, xcb_timestamp_t time)
{
	log::printf("call %s\n", __PRETTY_FUNCTION__);
	auto vf = dynamic_pointer_cast<view_fullscreen_t>(c);
	if(vf)
		switch_fullscreen_to_prefered_view_mode(vf, time);
}

void workspace_t::switch_fullscreen_to_prefered_view_mode(view_fullscreen_p view, xcb_timestamp_t time)
{
	view->remove_this_view();

	for(auto v: gather_children_root_first<viewport_t>()) {
		v->show();
	}

	if (view->revert_type == MANAGED_NOTEBOOK) {
		auto n = ensure_default_notebook();
		if(not view->revert_notebook.expired()) {
			n = view->revert_notebook.lock();
		}
		//meta_window_unmake_fullscreen(view->_client->_meta_window);
		n->add_client_from_view(view, time);
	} else {
		//meta_window_unmake_fullscreen(view->_client->_meta_window);
		auto vf = make_shared<view_floating_t>(view.get());
		_insert_view_floating(vf, time);
	}

	_ctx->sync_tree_view();
}

void workspace_t::add_floating(tree_p c)
{
	_floating_layer->push_back(c);
}

void workspace_t::add_fullscreen(tree_p c)
{
	_fullscreen_layer->push_back(c);
}

void workspace_t::add_overlay(tree_p t)
{
	_overlays_layer->push_back(t);
}

void workspace_t::set_name(string const & s) {
	_name = s;
}

auto workspace_t::name() -> string const & {
	return _name;
}

void workspace_t::set_to_default_name() {
	std::ostringstream os;
	os << "Workspace #" << meta_workspace_index(_meta_workspace);
	_name = os.str();
}

void workspace_t::set_primary_viewport(shared_ptr<viewport_t> v) {
	if(not has_key(_viewport_outputs, v))
		throw exception_t("invalid primary viewport");
	_primary_viewport = v;
}

shared_ptr<viewport_t> workspace_t::primary_viewport() const {
	return _primary_viewport.lock();
}

auto workspace_t::client_focus_history() -> list<view_w>
{
	return _client_focus_history;
}

bool workspace_t::client_focus_history_front(view_p & out) {
	if(not client_focus_history_is_empty()) {
		out = _client_focus_history.front().lock();
		return true;
	}
	return false;
}

void workspace_t::client_focus_history_remove(view_p in) {
	_client_focus_history.remove_if([in](view_w w) { return w.expired() or w.lock() == in; });
}

void workspace_t::client_focus_history_move_front(view_p in) {
	move_front(_client_focus_history, in);
}

bool workspace_t::client_focus_history_is_empty() {
	_client_focus_history.remove_if([](view_w const & w) { return w.expired(); });
	return _client_focus_history.empty();
}

auto workspace_t::lookup_view_for(client_managed_p c) const -> view_p
{
	for (auto & x: gather_children_root_first<view_t>()) {
		if (x->_client == c)
			return x;
	}
	return nullptr;
}

void workspace_t::set_focus(view_p new_focus, xcb_timestamp_t time) {
	if(new_focus) {
		client_focus_history_move_front(new_focus);
	}
	_net_active_window = new_focus;
	_ctx->apply_focus(time);
}

void workspace_t::unmanage(client_managed_p mw)
{
	auto v = lookup_view_for(mw);
	if (v == nullptr)
		return;

	bool has_focus = false;
	if (_net_active_window.lock() == v) {
		has_focus = true;
	}

	/* if managed window have active clients */
	log(LOG_MANAGE, "unmanaging : 0x%x '%s'\n", 0, mw->title().c_str());

	client_focus_history_remove(v);
	v->remove_this_view();

	if (dynamic_pointer_cast<view_rebased_t>(v) != nullptr) {
		if (_ctx->configuration._auto_refocus and has_focus) {
			view_p focus;
			if (client_focus_history_front(focus)) {
				set_focus(focus, XCB_CURRENT_TIME);
				focus->xxactivate(XCB_CURRENT_TIME);
			}
		}
	}

}

auto workspace_t::_find_viewport_of(tree_p t) -> viewport_p {
	while(t != nullptr) {
		auto ret = dynamic_pointer_cast<viewport_t>(t);
		if(ret != nullptr)
			return ret;
		t = t->parent()->shared_from_this();
	}

	return nullptr;
}

void workspace_t::_insert_view_floating(view_floating_p fv, xcb_timestamp_t time)
{
	auto c = fv->_client;

//	auto wid = c->ensure_workspace();
//	if (wid != ALL_DESKTOP) {
//		//c->set_net_wm_desktop(_id);
//	}

	if (is_enable())
		fv->acquire_client();

	add_floating(fv);
	fv->raise();
	fv->show();
	set_focus(fv, time);
	_ctx->sync_tree_view();
}

}
