/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2010 José Rostagno (for vijona.com.ar) 

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/

#include "pluginmenu.h"
#include "pluginmanager.h"
#include "main_window.h"
#include <gdk/gdkkeysyms.h>

struct _GtkPluginManagerMenuPrivate
{
  /* the recent manager object */
  PluginManager *plugmg;
  
  GtkWidget *placeholder;

  GtkAccelGroup *accel_group;
};

#define GTK_PLUGIN_MANAGER_MENU_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_PLUGIN_MANAGER_MENU, GtkPluginManagerMenuPrivate))
static void gtk_plugin_manager_menu_class_init (GtkPluginManagerMenuClass *klass);
static void gtk_plugin_manager_menu_init (GtkPluginManagerMenu *menu);
static void gtk_plugin_manager_menu_finalize (GObject *object);

static void gtk_plugin_manager_menu_populate (GtkPluginManagerMenu *menu);
static void plugin_exec (GtkWidget *widget, gpointer user_data);

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(GtkPluginManagerMenu, gtk_plugin_manager_menu, GTK_TYPE_MENU);  

static void
gtk_plugin_manager_menu_class_init (GtkPluginManagerMenuClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gtk_plugin_manager_menu_finalize;
  g_type_class_add_private (klass, sizeof (GtkPluginManagerMenuPrivate));
}

static void
gtk_plugin_manager_menu_init (GtkPluginManagerMenu *menu)
{
  GtkPluginManagerMenuPrivate *priv;
  
  priv = GTK_PLUGIN_MANAGER_MENU_GET_PRIVATE (menu);
  
  menu->priv = priv;

  priv->plugmg = plugin_manager_new ();

  /* we create a placeholder menuitem, to be used in case
   * the menu is empty. this placeholder will stay around
   * for the entire lifetime of the menu, and we just hide it
   * when it's not used. we have to do this, and do it here,
   * because we need a marker for the beginning of the recent
   * items list, so that we can insert the new items at the
   * right place when idly populating the menu in case the
   * user appended or prepended custom menu items to the
   * recent chooser menu widget.
   */
  priv->placeholder = gtk_menu_item_new_with_label (_("No items found"));
  gtk_widget_set_sensitive (priv->placeholder, FALSE);
  g_object_set_data (G_OBJECT (priv->placeholder),
                     "gtk-recent-menu-placeholder",
                     GINT_TO_POINTER (TRUE));

  gtk_menu_shell_insert (GTK_MENU_SHELL (menu), priv->placeholder, 0);
  gtk_widget_set_no_show_all (priv->placeholder, TRUE);
  gtk_widget_show (priv->placeholder);
}

static void
gtk_plugin_manager_menu_finalize (GObject *object)
{
  GtkPluginManagerMenu *menu = GTK_PLUGIN_MANAGER_MENU (object);
  GtkPluginManagerMenuPrivate *priv = menu->priv;
  
  if (priv->plugmg) priv->plugmg=NULL;  
  
  G_OBJECT_CLASS (gtk_plugin_manager_menu_parent_class)->finalize (object);
}

/*
* transform a number into it's corresponding keysym
*/
static gint parse_shortcut(gint accel_number){ 
  switch (accel_number) {
   case 0: return GDK_KEY_0;
    break;
    case 1: return GDK_KEY_1;
      break;
    case 2: return GDK_KEY_2;
      break;
    case 3: return GDK_KEY_3;
      break;
    case 4: return GDK_KEY_4;
      break;
    case 5: return GDK_KEY_5;
      break;
    case 6: return GDK_KEY_6;
      break;
    case 7: return GDK_KEY_7;
      break;
    case 8: return GDK_KEY_8;
      break;
    case 9: return GDK_KEY_9;
      break;
  }
 return GDK_KEY_0;
}

/* removes the items we own from the menu */
static void
gtk_plugin_manager_menu_dispose_items (GtkPluginManagerMenu *menu)
{
  GList *children, *l;
 
  children = gtk_container_get_children (GTK_CONTAINER (menu));
  for (l = children; l != NULL; l = l->next)
    {
      GtkWidget *menu_item = GTK_WIDGET (l->data);
      gboolean has_mark = FALSE;
      
      /* check for our mark, in order to remove just the items we own */
      has_mark =
        GPOINTER_TO_INT (g_object_get_data (G_OBJECT (menu_item), "gtk-recent-menu-mark"));

      if (has_mark)
        {
          GtkRecentInfo *info;
          
          /* destroy the attached RecentInfo struct, if found */
          info = g_object_get_data (G_OBJECT (menu_item), "gtk-recent-info");
          if (info)
            g_object_set_data_full (G_OBJECT (menu_item), "gtk-recent-info",
            			    NULL, NULL);
          
          /* and finally remove the item from the menu */
          gtk_container_remove (GTK_CONTAINER (menu), menu_item);
        }
    }

  g_list_free (children);
}

/* Creates a new GtkImageMenuItem with a stock image and a custom label.*/
GtkWidget *gtk_menu_item_new_image_item(const gchar *stock_id, const gchar *label)
{
	GtkWidget *item = gtk_image_menu_item_new_with_mnemonic(label);
	GtkWidget *image = gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU);

	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	gtk_widget_show(image);
	return item;
}

void add_plugin_to_menu (gpointer data, gpointer user_data)
{
    static int i=1;
    GtkPluginManagerMenu *menu=(GtkPluginManagerMenu *) user_data;
    GtkWidget *item;
    Plugin *plugin;
    plugin = PLUGIN(data);
    /*
    * Syntax plugins are automatically incorporate to the syntax check system so don't show that plugins here.
    * TODO: maybe configure this in preferences??
    */
    if (get_plugin_syntax_type(plugin)==-1){ 
    item =  gtk_menu_item_new_image_item(GTK_STOCK_EXECUTE, get_plugin_name(plugin));
    gtk_widget_show(item);
    install_menu_hint(item, (gchar *)get_plugin_description(plugin));
    g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(plugin_exec), (gpointer) menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    if (i<10) gtk_widget_add_accelerator(item, "activate", menu->priv->accel_group, parse_shortcut(i), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    i++;
    }
}

static void
gtk_plugin_manager_menu_populate (GtkPluginManagerMenu *menu)
{
  GtkPluginManagerMenuPrivate *priv = menu->priv;
  gtk_plugin_manager_menu_dispose_items (menu);

  guint plug_count= get_plugin_manager_items_count(priv->plugmg);
  if (plug_count!=0) gtk_widget_hide(priv->placeholder);
  GList *plugins= get_plugin_manager_items(priv->plugmg);
  g_list_foreach (plugins, add_plugin_to_menu,menu);
  g_list_free(plugins);
}

static void plugin_exec (GtkWidget *widget, gpointer user_data)
{
  Plugin *plugin;
  Documentable *doc = document_manager_get_current_documentable(main_window.docmg);
  if (!doc) return;
  GtkPluginManagerMenu *menu= GTK_PLUGIN_MANAGER_MENU (user_data);
  
  plugin = get_plugin_by_name(menu->priv->plugmg, (gchar *) gtk_menu_item_get_label (GTK_MENU_ITEM(widget)));
  plugin_run(plugin, doc);
}

void plugin_exec_with_num(GtkWidget *widget, gint num){
  Plugin *plugin;
  Documentable *doc = document_manager_get_current_documentable(main_window.docmg); 
  if (!doc) return;
  GtkPluginManagerMenu *menu= GTK_PLUGIN_MANAGER_MENU (widget);
  if (get_plugin_manager_items_count(menu->priv->plugmg) < num) return;
  plugin = get_plugin_by_num(menu->priv->plugmg, num);
  plugin_run(plugin, doc);
}
/*
 * Public API
 */

GtkWidget *
gtk_plugin_manager_menu_new (GtkAccelGroup *accel_grup)
{
  GtkPluginManagerMenu *menu = g_object_new (GTK_TYPE_PLUGIN_MANAGER_MENU, NULL);
  /* (re)populate the menu */
  GtkPluginManagerMenuPrivate *priv = menu->priv;
  priv->accel_group= accel_grup;
  gtk_plugin_manager_menu_populate (menu);
  return GTK_WIDGET(menu);
}
