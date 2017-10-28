/*
 * Copyright (2017) Benoit Gschwind
 *
 * page-init.cxx is part of page-compositor.
 *
 * page-compositor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * page-compositor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with page-compositor.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "page-init.h"

#include "page-page.hxx"

extern "C" {
#include "shell-global.h"
#include "shell-global-private.h"
#include "shell-wm.h"
}


void page_init(MetaPlugin * plugin)
{
  auto x = new page::page_t(plugin);

  auto global = shell_global_get();
  auto wm = shell_global_get_wm(global);

  x->g_connectable_t::g_connect(wm, "minimize",
      &page::page_t::_handler_plugin_minimize);
  x->g_connectable_t::g_connect(wm, "unminimize",
      &page::page_t::_handler_plugin_unminimize);
  x->g_connectable_t::g_connect(wm, "size-changed",
      &page::page_t::_handler_plugin_size_changed);
  x->g_connectable_t::g_connect(wm, "size-change",
      &page::page_t::_handler_plugin_size_change);
  x->g_connectable_t::g_connect(wm, "map",
      &page::page_t::_handler_plugin_map);
  x->g_connectable_t::g_connect(wm, "destroy",
      &page::page_t::_handler_plugin_destroy);
  x->g_connectable_t::g_connect(wm, "switch-workspace",
      &page::page_t::_handler_plugin_switch_workspace);
  x->g_connectable_t::g_connect(wm, "kill-switch-workspace",
      &page::page_t::_handler_plugin_kill_switch_workspace);
  x->g_connectable_t::g_connect(wm, "kill-window-effects",
      &page::page_t::_handler_plugin_kill_window_effects);
  x->g_connectable_t::g_connect(wm, "show-tile-preview",
      &page::page_t::_handler_plugin_show_tile_preview);
  x->g_connectable_t::g_connect(wm, "hide-tile-preview",
      &page::page_t::_handler_plugin_hide_tile_preview);
  x->g_connectable_t::g_connect(wm, "show-window-menu",
      &page::page_t::_handler_plugin_show_window_menu);
  x->g_connectable_t::g_connect(wm, "filter-keybinding",
      &page::page_t::_handler_plugin_keybinding_filter);
  x->g_connectable_t::g_connect(wm, "confirm-display-change",
      &page::page_t::_handler_plugin_confirm_display_change);
  x->g_connectable_t::g_connect(wm, "create-close-dialog",
      &page::page_t::_handler_plugin_create_close_dialog);
  x->g_connectable_t::g_connect(wm, "create-inhibit-shortcuts-dialog",
      &page::page_t::_handler_plugin_create_inhibit_shortcuts_dialog);

  x->_handler_plugin_start();

}

