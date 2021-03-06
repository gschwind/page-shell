/*
 * tiny_theme.hxx
 *
 * copyright (2010-2014) Benoit Gschwind
 *
 * This code is licensed under the GPLv3. see COPYING file for more details.
 *
 */

#ifndef TINY_THEME_HXX_
#define TINY_THEME_HXX_

#include <pango/pangocairo.h>

#include <memory>
#include <cairo.h>
#include <cairo-xlib.h>

#include "page-utils.hxx"
#include "page-theme.hxx"
#include "page-simple2-theme.hxx"
#include "page-color.hxx"
#include "page-config-handler.hxx"

namespace page {

using namespace std;

class tiny_theme_t : public simple2_theme_t {

	rect compute_notebook_bookmark_position(rect const & allocation) const;
	rect compute_notebook_vsplit_position(rect const & allocation) const;
	rect compute_notebook_hsplit_position(rect const & allocation) const;
	rect compute_notebook_close_position(rect const & allocation) const;
	rect compute_notebook_menu_position(rect const & allocation) const;

	void render_notebook_selected(
			cairo_t * cr,
			theme_notebook_t const & n,
			theme_tab_t const & data,
			PangoFontDescription const * pango_font,
			color_t const & text_color,
			color_t const & outline_color,
			color_t const & border_color,
			color_t const & background_color,
			double border_width
	) const;

	void render_notebook_normal(
			cairo_t * cr,
			theme_tab_t const & data,
			PangoFontDescription const * pango_font,
			color_t const & text_color,
			color_t const & outline_color,
			color_t const & border_color,
			color_t const & background_color
	) const;

public:
	tiny_theme_t(config_handler_t & conf);
	virtual ~tiny_theme_t();

	virtual void render_notebook(cairo_t * cr, theme_notebook_t const * n) const;

	virtual void render_iconic_notebook(
			cairo_t * cr,
			vector<theme_tab_t> const & tabs
	) const;

};

}

#endif /* SIMPLE_THEME_HXX_ */
