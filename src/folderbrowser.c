#include "folderbrowser.h"
void create_tree(GtkTreeStore *pTree, gchar *sChemin, GtkTreeIter *iter, GtkTreeIter *iter2)
 {
GList *list;
GnomeVFSResult result;
result=gnome_vfs_directory_list_load (&list, sChemin, GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE | GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
if (result==GNOME_VFS_OK){
        GList *element;
	GnomeVFSFileInfo *info;
        for (element = list; element != NULL; element = element->next)
 	    {
           info = element->data;
           GdkPixbuf *p_file_image = NULL;
    if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
            if (info->name[0]!='.'){
                p_file_image =gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "folder", 16, 0, NULL);
 	       gtk_tree_store_insert_with_values(GTK_TREE_STORE(pTree), iter2, iter, 0, 0, p_file_image, 1, info->name,2, "inode/directory",-1);
               GtkTreeIter iter_new;
               gchar *next_dir = NULL;
               next_dir = g_build_path (G_DIR_SEPARATOR_S, sChemin, info->name, NULL);
               create_tree(GTK_TREE_STORE(pTree),next_dir,iter2,&iter_new);
            }
              } else {
           const char *mime;
           mime=gnome_vfs_get_mime_type_for_name (info->name);
 //          g_print("nombre: %s mime: %s\n",info->name,mime);
               //FIXME:avoid unwanted files
               /*avoid unwanted files*/
               if (gnome_vfs_mime_type_is_equal (mime,"application/x-trash") || gnome_vfs_mime_type_is_equal (mime,"application/x-archive") || gnome_vfs_mime_type_is_equal (mime,"application/x-object") || gnome_vfs_mime_type_is_equal (mime,"application/octet-stream") || gnome_vfs_mime_type_get_equivalence (mime,"image/*")==GNOME_VFS_MIME_PARENT|| gnome_vfs_mime_type_get_equivalence (mime," audio/*")==GNOME_VFS_MIME_PARENT || gnome_vfs_mime_type_is_equal (mime,"application/pdf") || gnome_vfs_mime_type_is_equal (mime,"application/zip") || gnome_vfs_mime_type_is_equal (mime,"application/rar")){
  //             g_print("skip back-up or object file\n");
               }else{
            //TODO: get right icon for mimetype
            p_file_image =gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "text-x-generic", 16, 0, NULL);
            gtk_tree_store_insert_with_values(GTK_TREE_STORE(pTree), iter2, iter, 0, 0, p_file_image, 1, info->name,2,mime,-1);
               }
 	      }
 	    }
} else {
g_print(_("error loading tree:%s\n"),gnome_vfs_result_to_string (result));
}
}

void tree_double_clicked(GtkTreeView *tree_view,GtkTreePath *path,GtkTreeViewColumn *column,gpointer user_data)
 {
 	GtkTreeModel *model;
 	GtkTreeSelection *select;
 	GtkTreeIter iter;
  	select = gtk_tree_view_get_selection(tree_view);
  	if(sChemin==NULL)
        sChemin=(gchar*)gtk_button_get_label(GTK_BUTTON(main_window.button_dialog));
  if(gtk_tree_selection_get_selected (select, &model, &iter))
  {
 	gchar *nfile;
        gchar *mime;
     gtk_tree_model_get (model, &iter,1, &nfile,2,&mime, -1);

 	GtkTreeIter* parentiter=(GtkTreeIter*)malloc(sizeof(GtkTreeIter));
 	while(gtk_tree_model_iter_parent(model,parentiter,&iter)){
 		gchar *rom;
     	gtk_tree_model_get (model, parentiter, 1, &rom, -1);
 		nfile = g_build_path (G_DIR_SEPARATOR_S, rom, nfile, NULL);
 		iter=*parentiter;
 		parentiter=(GtkTreeIter*)malloc(sizeof(GtkTreeIter));
 	}
     gchar* file_name = g_build_path (G_DIR_SEPARATOR_S, sChemin, nfile, NULL);
     if (!MIME_ISDIR(mime))
     	switch_to_file_or_open(file_name,0);
     else
     {
	if(gtk_tree_view_row_expanded (tree_view,path))
     		gtk_tree_view_collapse_row (tree_view,path);
     	else
     		gtk_tree_view_expand_row (tree_view,path,0);
     }  
   }
 }

/*
 * filebrowser_sort_func
 *
 * this function is the sort function, and has the following  features:
 * - directories go before files
 * - files are first sorted without extension, only equal names are sorted by extension
 *
 */
gint filebrowser_sort_func(GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b,
						   gpointer user_data)
{
	gchar *namea, *nameb, *mimea, *mimeb;
	gboolean isdira, isdirb;
	gint retval = 0;
	gtk_tree_model_get((GtkTreeModel *)model, a, 1, &namea,2, &mimea,-1);
	gtk_tree_model_get((GtkTreeModel *)model, b, 1, &nameb,2, &mimeb,-1);
        isdira = (mimea && MIME_ISDIR(mimea));
	isdirb = (mimeb && MIME_ISDIR(mimeb));
	/*g_print("isdira=%d, mimea=%s, isdirb=%d, mimeb=%s\n",isdira,mimea,isdirb,mimeb);*/
	if (isdira == isdirb) {		/* both files, or both directories */
		if (namea == nameb) {
			retval = 0;			/* both NULL */
		} else if (namea == NULL || nameb == NULL) {
			retval = (namea - nameb);
		} else {				/* sort by name, first without extension */
			gchar *dota, *dotb;
			dota = strrchr(namea, '.');
			dotb = strrchr(nameb, '.');
			if (dota)
				*dota = '\0';
			if (dotb)
				*dotb = '\0';
			retval = strcmp(namea, nameb);
			if (retval == 0) {
				if (dota)
					*dota = '.';
				if (dotb)
					*dotb = '.';
				retval = strcmp(namea, nameb);
			}
		}
	} else {					/* a directory and a file */
		retval = (isdirb - isdira);
	}
	g_free(namea);
	g_free(nameb);
	g_free(mimea);
	g_free(mimeb);
	return retval;
}


void folderbrowser_create(MainWindow *main_window)
 {
    	main_window->folder = gtk_vbox_new(FALSE, 0);

 	GtkTreeViewColumn *pColumn;
 	GtkCellRenderer  *pCellRenderer;
 	main_window->pTree = gtk_tree_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING,G_TYPE_STRING);
 	main_window->pListView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(main_window->pTree));

        pCellRenderer = gtk_cell_renderer_pixbuf_new();
        pColumn = gtk_tree_view_column_new_with_attributes("",pCellRenderer,"pixbuf",0,NULL);

 	gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->pListView), pColumn);
 	main_window->pScrollbar = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(main_window->pScrollbar),GTK_POLICY_ALWAYS,GTK_POLICY_ALWAYS);
        gtk_container_add(GTK_CONTAINER(main_window->pScrollbar), main_window->pListView);

 	//renderer for text
 	g_signal_connect(G_OBJECT(main_window->pListView), "row-activated", G_CALLBACK(tree_double_clicked), NULL);
 	pColumn=NULL;
 	pCellRenderer=NULL;
        pCellRenderer = gtk_cell_renderer_text_new();
        pColumn = gtk_tree_view_column_new_with_attributes(_("File"), pCellRenderer, "text",1,NULL);
 	gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->pListView), pColumn);

        pColumn=NULL;
 	pCellRenderer=NULL;
        pCellRenderer = gtk_cell_renderer_text_new();
        pColumn = gtk_tree_view_column_new_with_attributes(_("Mime"), pCellRenderer,"text",2,NULL);
  	gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->pListView), pColumn);
        gtk_tree_view_column_set_visible    (pColumn,FALSE);
 	gtk_widget_show(main_window->folder);
 	GtkWidget *label= gtk_label_new(_("Folder browser"));

 	if(sChemin!=NULL)
   		main_window->button_dialog = gtk_button_new_with_label (sChemin);
   	else {
		main_window->button_dialog = gtk_button_new_with_label (_("Workspace's directory"));
        }
 	g_signal_connect(G_OBJECT(main_window->button_dialog), "pressed", G_CALLBACK(pressed_button_file_chooser), NULL);

 	gtk_widget_set_usize(main_window->pListView,80,450);
        //Close button for the side bar
	GtkWidget *hbox;
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 0);
	main_window->close_image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_misc_set_padding(GTK_MISC(main_window->close_image), 0, 0);
	main_window->close_sidebar_button = gtk_button_new();
	gtk_widget_set_tooltip_text(main_window->close_sidebar_button, _("Close class Browser"));
	gtk_button_set_image(GTK_BUTTON(main_window->close_sidebar_button), main_window->close_image);
	gtk_button_set_relief(GTK_BUTTON(main_window->close_sidebar_button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(main_window->close_sidebar_button), FALSE);
	gtk_signal_connect(GTK_OBJECT(main_window->close_sidebar_button), "clicked", G_CALLBACK (classbrowser_show_hide),NULL);
	gtk_widget_show(main_window->close_image);
	gtk_widget_show(main_window->close_sidebar_button);
	gtk_box_pack_end(GTK_BOX(hbox), main_window->close_sidebar_button, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
        gtk_box_pack_start(GTK_BOX(main_window->folder), hbox, FALSE, TRUE, 2);
 	gtk_box_pack_start(GTK_BOX(main_window->folder), main_window->button_dialog, FALSE, TRUE, 2);
 	gtk_box_pack_start(GTK_BOX(main_window->folder), main_window->pScrollbar, FALSE, FALSE, 2);
        gtk_widget_show(main_window->button_dialog);
	gtk_widget_show_all(main_window->folder);
        gint pos;
       	pos=gtk_notebook_insert_page (GTK_NOTEBOOK(main_window->notebook_manager), main_window->folder, label, 1);
 			if(sChemin!=NULL){
 				GtkTreeIter iter2;
				GtkTreeIter* iter=NULL;
 				gtk_tree_store_clear(main_window->pTree);
                                gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(main_window->pTree), 1,
										filebrowser_sort_func, NULL, NULL);
                                gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(main_window->pTree), 1,
										 GTK_SORT_ASCENDING);
 				create_tree(GTK_TREE_STORE(main_window->pTree),sChemin,iter,&iter2);
   	}
}
