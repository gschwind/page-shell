/*
 * popup_alt_tab.hxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */


#ifndef DROPDOWN_MENU_HXX_
#define DROPDOWN_MENU_HXX_

#include <cairo.h>
#include <cairo-xcb.h>

#include <string>
#include <memory>
#include <vector>

#include "page-region.hxx"
#include "page-page-types.hxx"
#include "page-theme.hxx"
#include "page-tree.hxx"

namespace page {

using namespace std;

class dropdown_menu_entry_t {
	friend class dropdown_menu_t;

	theme_dropdown_menu_entry_t _theme_data;
	function<void(xcb_timestamp_t time)> _on_click;

	dropdown_menu_entry_t(dropdown_menu_entry_t const &) = delete;
	dropdown_menu_entry_t & operator=(dropdown_menu_entry_t const &) = delete;

public:
	dropdown_menu_entry_t(shared_ptr<icon16> icon,
			string const & label, function<void(xcb_timestamp_t time)> on_click);
	~dropdown_menu_entry_t();
	shared_ptr<icon16> icon() const;
	string const & label() const;
	theme_dropdown_menu_entry_t const & get_theme_item();

};

struct dropdown_menu_overlay_t : public tree_t {
	page_t * _ctx;
	rect _position;
	bool _is_durty;

	ClutterContent * _canvas;
	ClutterActor * _default_view;

	signal_t<ClutterCanvas *, cairo_t *, int, int> on_draw;

	dropdown_menu_overlay_t(tree_t * root, rect position);
	~dropdown_menu_overlay_t();

	rect const & position();

	void invalidate();

	void draw(ClutterCanvas * canvas, cairo_t * cr, int width, int height);

};

class dropdown_menu_t :
		public grab_handler_t,
		public connectable_t
{

public:
	using item_t = dropdown_menu_entry_t;

protected:
	page_t * _ctx;
	vector<shared_ptr<item_t>> _items;
	int _selected;
	shared_ptr<dropdown_menu_overlay_t> pop;
	rect _start_position;
	xcb_button_t _button;
	xcb_timestamp_t _time;
	bool has_been_released;

public:

	dropdown_menu_t(tree_t * ref, vector<shared_ptr<item_t>> items,
			xcb_button_t button, int x, int y, int width, rect start_position);
	~dropdown_menu_t();

	int selected();
	xcb_timestamp_t time();

	void draw(ClutterCanvas * canvas, cairo_t * cr, int width, int height);

	void update_items_back_buffer(cairo_t * cr, int n);
	void set_selected(int s);
	void update_cursor_position(int x, int y);

	virtual void button_press(ClutterEvent const * e) override;
	virtual void button_motion(ClutterEvent const * e) override;
	virtual void button_release(ClutterEvent const * e) override;
	virtual void key_press(ClutterEvent const * ev) override;
	virtual void key_release(ClutterEvent const * ev) override;

};



}



#endif /* POPUP_ALT_TAB_HXX_ */
